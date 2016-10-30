// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "study_limbo.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#define USE_TBB
#define USE_NLOPT

#include "nlopt.hpp"

#include <limbo/limbo.hpp>

using namespace limbo;

struct Params {
    // we use the default parameters for acqui_ucb
    struct acqui_ucb : public defaults::acqui_ucb {
    };

    struct mean_constant : public defaults::mean_constant {
    };

    // Exponential covariance function
    struct kernel_exp : public defaults::kernel_exp {
    };
    // Squared exponential covariance function with automatic relevance detection
    struct kernel_squared_exp_ard : public defaults::kernel_squared_exp_ard {
    };
    struct kernel_maternfivehalves : public defaults::kernel_maternfivehalves {
    };

    // acquisition function
    struct acqui_gpucb : public defaults::acqui_gpucb {
    };
    struct acqui_ei {
        BO_PARAM(double, jitter, 0.0);
    };

    // optimizer
    struct opt_parallelrepeater : public defaults::opt_parallelrepeater {
    };
    struct opt_rprop : public defaults::opt_rprop {
    };

    // internal optimizer
    // NLOpt
    struct opt_nloptnograd : public defaults::opt_nloptnograd {};
    // Default
    // struct opt_gridsearch : public defaults::opt_gridsearch {};

    struct bayes_opt_bobase {
        BO_PARAM(bool, stats_enabled, false)
        BO_PARAM(bool, bounded, true)
    };

    struct bayes_opt_boptimizer {
        BO_DYN_PARAM(double, noise)
        // BO_DYN_PARAM(int, hp_period, -1)
        BO_DYN_PARAM(int, hp_period)
    };

    struct init_randomsampling {
        BO_DYN_PARAM(int, samples)
    };

    struct stop_maxiterations {
        BO_DYN_PARAM(int, iterations)
    };
};

// declare dynamic parameters
BO_DECLARE_DYN_PARAM(int, Params::init_randomsampling, samples);
BO_DECLARE_DYN_PARAM(int, Params::stop_maxiterations, iterations);
BO_DECLARE_DYN_PARAM(double, Params::bayes_opt_boptimizer, noise);
BO_DECLARE_DYN_PARAM(int, Params::bayes_opt_boptimizer, hp_period);

// covariance function
using kernel_t = kernel::SquaredExpARD<Params>;
// using kernel_t = kernel::MaternFiveHalves<Params>;
// using kernel_t = kernel::MaternThreeHalves<Params>;
// using kernel_t = kernel::Exp<Params>;

// mean
// using mean_t = mean::Data<Params>;
// using mean_t = mean::Constant<Params>;
using mean_t = mean::FunctionARD<Params, mean::Constant<Params> >;

// Gaussian Process - hyperparameter optimizer
using gp_opt_t = model::gp::KernelLFOpt<Params>;
// using gp_opt_t = model::gp::MeanLFOpt<Params>;
// using gp_opt_t = model::gp::KernelMeanLFOpt<Params>;
// using gp_opt_t = model::gp::NoLFOpt<Params>;

// Gaussian Process
using GP_t = model::GP<Params, kernel_t, mean_t, gp_opt_t>;

// acquisition function
using acqui_t = acqui::GP_UCB<Params, GP_t>;
// using acqui_t = acqui::EI<Params, GP_t>;
// using acqui_t = acqui::UCB<Params, GP_t>;

// optimizer
using acqui_opt_t = opt::NLOptNoGrad<Params>;

struct StateEval
{
    // number of input dimension (x.size())
    static size_t dim_in;
    // number of dimenions of the result (res.size())
    static size_t dim_out;

    StateEval(StudyLimbo *study,
              bayes_opt::BOptimizer<Params, modelfun<GP_t>, acquifun<acqui_t>, acquiopt<acqui_opt_t> > *opt)
        : m_study(study), m_opt(opt)
    {
        // static variables
        StateEval::dim_in = m_study->parameters().count();
        StateEval::dim_out = 1;
    }

    Eigen::VectorXd operator()(const Eigen::VectorXd& x) const
    {
        // add set
        if (m_study->computationSets().count() == 1
                && m_study->computations(0).count() == m_study->value(Study::LIMBO_init_randomsampling_samples).toInt())
            m_study->addComputationSet(QObject::tr("Steps"));

        if (m_study->isAborted())
        {
            // opt.set_force_stop(1);
            // return numeric_limits<double>::max();
        }

        // qDebug() << "opt.get_population()" << opt.get_population();

        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);        

        // set parameters
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            // scale parameter from interval (0, 1)
            computation->config()->parameters()->set(parameter.name(), parameter.lowerBound() + x(i) * (parameter.upperBound() - parameter.lowerBound()));
            // qDebug() << x(i) << parameter.lowerBound() + x(i) * (parameter.upperBound() - parameter.lowerBound());
        }

        // compute uncertainty
        SolutionUncertainty solutionUncertainty;
        if (m_study->computationSets().count() > 1)
        {
            Eigen::VectorXd mu = m_opt->model().mu(x);
            double sigma = m_opt->model().sigma(x);

            solutionUncertainty.uncertainty = -mu(0); // -evaluateCriteria(query);
            solutionUncertainty.lowerBound = -mu(0) - 2.0*sqrt(sigma);
            solutionUncertainty.upperBound = -mu(0) + 2.0*sqrt(sigma);
        }

        // evaluate step
        m_study->evaluateStep(computation, solutionUncertainty);
        /*
        QList<double> values = m_study->evaluateMultiGoal(computation);

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        Eigen::VectorXd res(m_study->parameters().count());
        for (int i = 0; i < values.count(); i++)
            res[i] = values[i];
        */

        double value = m_study->evaluateSingleGoal(computation);

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        // penalty
        double totalPenalty = 0.0;
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            if (parameter.penaltyEnabled())
                totalPenalty += parameter.penalty(x[i]);
        }

        // Eigen::VectorXd res(1);
        // res(0) = -value;
        // return res;

        m_study->addComputation(computation);

        // we return a 1-dimensional vector
        return tools::make_vector(-(value + totalPenalty));
    }

private:
    StudyLimbo *m_study;
    bayes_opt::BOptimizer<Params, modelfun<GP_t>, acquifun<acqui_t>, acquiopt<acqui_opt_t> > *m_opt;
};

size_t StateEval::dim_in = -1;
size_t StateEval::dim_out = -1;

struct Average
{
    typedef double result_type;
    double operator()(const Eigen::VectorXd& x) const
    {
        return x.sum() / x.size();
    }
};

StudyLimbo::StudyLimbo() : Study()
{
}

int StudyLimbo::estimatedNumberOfSteps() const
{
    return value(LIMBO_stop_maxiterations_iterations).toInt();
}

void StudyLimbo::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    addComputationSet(tr("Initialization"));

    // init - random sampling
    Params::init_randomsampling::set_samples(value(LIMBO_init_randomsampling_samples).toInt());
    // max iterations
    Params::stop_maxiterations::set_iterations(value(LIMBO_stop_maxiterations_iterations).toInt());
    // Bayes Optimizer
    Params::bayes_opt_boptimizer::set_noise(value(LIMBO_bayes_opt_boptimizer_noise).toDouble());
    Params::bayes_opt_boptimizer::set_hp_period(value(LIMBO_bayes_opt_boptimizer_hp_period).toInt());
    // Params::bayes_opt_boptimizer::set_hp_period(-1);

    // Bayes Optimizer
    bayes_opt::BOptimizer<Params, modelfun<GP_t>, acquifun<acqui_t>, acquiopt<acqui_opt_t> > opt;

    // optimize
    opt.optimize(StateEval(this, &opt), FirstElem());
    // std::cout << "best obs based on Average aggregator: " << opt.best_observation(Average()) << " res  " << opt.best_sample(Average()).transpose() << std::endl;

    m_isSolving = false;
    emit solved();
}

void StudyLimbo::setDefaultValues()
{    
    Study::setDefaultValues();

    m_settingDefault[LIMBO_init_randomsampling_samples] = 10;
    m_settingDefault[LIMBO_stop_maxiterations_iterations] = 20;
    m_settingDefault[LIMBO_bayes_opt_boptimizer_noise] = 1e-10;
    m_settingDefault[LIMBO_bayes_opt_boptimizer_hp_period] = 10;

    /*
    m_settingDefault[LIMBO_xtol_rel] = 1e-6;
    m_settingDefault[LIMBO_xtol_abs] = 1e-12;
    m_settingDefault[LIMBO_ftol_rel] = 1e-6;
    m_settingDefault[LIMBO_ftol_abs] = 1e-12;

    m_settingDefault[LIMBO_algorithm] = LIMBO::LN_BOBYQA;
    */
}

void StudyLimbo::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[LIMBO_init_randomsampling_samples] = "LIMBO_init_randomsampling_samples";
    m_settingKey[LIMBO_stop_maxiterations_iterations] = "LIMBO_stop_maxiterations_iterations";
    m_settingKey[LIMBO_bayes_opt_boptimizer_noise] = "LIMBO_bayes_opt_boptimizer_noise";
    m_settingKey[LIMBO_bayes_opt_boptimizer_hp_period] = "LIMBO_bayes_opt_boptimizer_hp_period";

    /*
    m_settingKey[LIMBO_xtol_rel] = "LIMBO_xtol_rel";
    m_settingKey[LIMBO_xtol_abs] = "LIMBO_xtol_abs";
    m_settingKey[LIMBO_ftol_rel] = "LIMBO_ftol_rel";
    m_settingKey[LIMBO_ftol_abs] = "LIMBO_ftol_abs";

    m_settingKey[LIMBO_algorithm] = "LIMBO_algorithm";
    */
}

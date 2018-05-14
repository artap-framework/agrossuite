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

#include <limbo/experimental/bayes_opt/ehvi.hpp>
#include <limbo/experimental/bayes_opt/nsbo.hpp>
#include <limbo/experimental/bayes_opt/parego.hpp>
#include <limbo/experimental/stat/pareto_front.hpp>
#include <limbo/experimental/stat/hyper_volume.hpp>

using namespace limbo;

struct Params {
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
    struct kernel_maternthreehalves : public defaults::kernel_maternthreehalves {
    };

    // acquisition function
    struct acqui_ucb : public defaults::acqui_ucb {
    };
    struct acqui_gpucb : public defaults::acqui_gpucb {
    };
    struct acqui_ei {
        BO_PARAM(double, jitter, 0.0)
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

    struct kernel : public defaults::kernel {
        BO_DYN_PARAM(double, noise)
        BO_PARAM(bool, optimize_noise, false);
    };

    struct bayes_opt_bobase {
        BO_PARAM(bool, stats_enabled, false)
        BO_PARAM(bool, bounded, true)
    };

    struct bayes_opt_boptimizer {
        BO_DYN_PARAM(int, hp_period)
    };

    struct init_randomsampling {
        BO_DYN_PARAM(int, samples)
    };

    struct stop_maxiterations {
        BO_DYN_PARAM(int, iterations)
    };

    /*
    // ehvi
    struct bayes_opt_ehvi : public defaults::bayes_opt_ehvi {
        BO_PARAM(double, x_ref, -11);
        BO_PARAM(double, y_ref, -11);
    };

    // parego
    struct model_gp_parego : public experimental::defaults::model_gp_parego {
    };

    struct stat_hyper_volume {
        BO_PARAM_ARRAY(double, ref, 10, 10);
    };
    */
};

// declare dynamic parameters
BO_DECLARE_DYN_PARAM(int, Params::init_randomsampling, samples);
BO_DECLARE_DYN_PARAM(int, Params::stop_maxiterations, iterations);
BO_DECLARE_DYN_PARAM(double, Params::kernel, noise);
BO_DECLARE_DYN_PARAM(int, Params::bayes_opt_boptimizer, hp_period);

/*
Eigen::VectorXd getOptMu(const Eigen::VectorXd& x)
{
    if (m_opt_gpucb)
        return m_opt_gpucb->model().mu(x);
    else if (m_opt_ei)
        return m_opt_ei->model().mu(x);
    else
        assert(0);
}

double getOptSigma(const Eigen::VectorXd& x)
{
    if (m_opt_gpucb)
        return m_opt_gpucb->model().sigma(x);
    else if (m_opt_ei)
        return m_opt_ei->model().sigma(x);
    else
        assert(0);
}
*/
struct StateEval
{
    // number of input dimension (x.size())
    static size_t dimIn;
    static size_t dim_in() { return dimIn; }
    // number of dimenions of the result (res.size())
    static size_t dimOut;
    static size_t dim_out() { return dimOut; }

    StateEval(StudyLimbo *study)
        : m_study(study), m_steps(0)
    {
        // static variables
        // input
        StateEval::dimIn = m_study->parameters().count();
        // output
        StateEval::dimOut = 0;
        foreach(Functional functional, m_study->functionals())
            if (functional.weight() > 0.0)
                StateEval::dimOut++;
        if (StateEval::dimOut == 0) StateEval::dimOut = 1;
    }

    Eigen::VectorXd operator()(const Eigen::VectorXd& x) const
    {
        // add set
        if (m_study->computationSets().count() == 1
                && m_study->computations(0).count() == m_study->value(Study::LIMBO_init_randomsampling_samples).toInt())
            m_study->addComputationSet(QObject::tr("Steps"));

        if (m_study->isAborted())
        {
            // max iterations
            Params::stop_maxiterations::set_iterations(Params::stop_maxiterations::iterations());

            Eigen::VectorXd res(StateEval::dimOut);
            for (int i = 0; i < StateEval::dimOut; i++)
                res[i] = -numeric_limits<double>::max();

            return res;
        }

        // qDebug() << "opt.get_population()" << opt.get_population();

        // computation
        QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

        // set parameters
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            // scale parameter from interval (0, 1)
            computation->config()->parameters()->set(parameter.name(), parameter.lowerBound() + x(i) * (parameter.upperBound() - parameter.lowerBound()));
            // qDebug() << x(i) << parameter.lowerBound() + x(i) * (parameter.upperBound() - parameter.lowerBound());
        }

        // check geometry
        if (m_study->value(Study::General_SolveProblem).toBool())
        {
            try
            {
                // invalidate scene (parameter update)
                computation->scene()->invalidate();
                computation->scene()->invalidate();

                computation->scene()->checkGeometryResult();
            }
            catch (AgrosGeometryException& e)
            {
                qDebug() << e.toString();
                return tools::make_vector(-numeric_limits<double>::max());
            }
        }

        // compute uncertainty
        SolutionUncertainty solutionUncertainty;
        if (m_study->computationSets().count() > 1)
        {
            Eigen::VectorXd mu = Eigen::VectorXd();
            double sigma = 0.0;

            // solutionUncertainty.uncertainty = mu(0);
            // solutionUncertainty.lowerBound = mu(0) - 2.0*sqrt(sigma);
            // solutionUncertainty.upperBound = mu(0) + 2.0*sqrt(sigma);
        }

        // evaluate step
        m_study->evaluateStep(computation, solutionUncertainty);
        double value = m_study->evaluateSingleGoal(computation);

        // design of experiments
        if (m_study->value(Study::General_DoE).toBool())
        {
            // base point for DoE
            QVector<double> init(m_study->parameters().count());
            for (int i = 0; i < m_study->parameters().count(); i++)
            {
                Parameter parameter = m_study->parameters()[i];
                init[i] = parameter.lowerBound() + x(i) * (parameter.upperBound() - parameter.lowerBound());
            }

            // DoE
            m_study->doeCompute(computation, init, value);
        }

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        // add computation
        m_study->addComputation(computation);
        
        // output
        Eigen::VectorXd res(StateEval::dimOut);
        if (StateEval::dimOut == 1)
        {
            // single objective
            double value = m_study->evaluateSingleGoal(computation);
            res(0) = -value;
        }
        else
        {
            // multi objective
            QList<double> values = m_study->evaluateMultiGoal(computation);

            for (int i = 0; i < values.count(); i++)
                res[i] = values[i];
        }

        m_steps++;
        qInfo() << "Limbo: step " << m_steps << "/" << m_study->estimatedNumberOfSteps();

        return res;
    }

private:
    StudyLimbo *m_study;
    mutable int m_steps;
};

size_t StateEval::dimIn = -1;
size_t StateEval::dimOut = -1;

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
    // mean
    meanList.append("data");
    meanList.append("constant");
    meanList.append("function_ard");

    // gp
    gpList.append("kernel_lf");
    gpList.append("mean_lf");
    gpList.append("kernel_mean_lf");
    gpList.append("no_opt");

    // acqui
    acquiList.append("gpucb");
    acquiList.append("ei");
}

int StudyLimbo::estimatedNumberOfSteps() const
{
    return value(LIMBO_stop_maxiterations_iterations).toInt() + value(LIMBO_init_randomsampling_samples).toInt();
}

// covariance function
// using kernel_xt = kernel::SquaredExpARD<Params>;
// using kernel_xt = kernel::MaternFiveHalves<Params>;
// using kernel_xt = kernel::MaternThreeHalves<Params>;
// using kernel_xt = kernel::Exp<Params>;

// mean
// using mean_data_xt = mean::Data<Params>;
// using mean_constant_xt = mean::Constant<Params>;
// using mean_function_ard_xt = mean::FunctionARD<Params, mean::Constant<Params> >;

// Gaussian Process - hyperparameter optimizer
// using gp_kernel_lf_xt = model::gp::KernelLFOpt<Params>;
// using gp_mean_lf_opt_xt = model::gp::MeanLFOpt<Params>;
// using gp_kernel_mean_lf_opt_xt = model::gp::KernelMeanLFOpt<Params>;
// using gp_no_opt_xt = model::gp::NoLFOpt<Params>;

// acquisition function
// using acqui_gpucb_xt = acqui::GP_UCB<Params, GP_t>;
// using acqui_ei_xt = acqui::EI<Params, GP_t>;
// using acqui_ucb_xt = acqui::UCB<Params, GP_t>;

// optimizer
// using acqui_opt_t = opt::NLOptNoGrad<Params>;

// using GP_t = model::GP<Params, kernel::SquaredExpARD<Params>, mean::mean_t<Params>, model::gp::gp_opt_t<Params> >;
#define LIMBO_OPTIMIZE(mean_t, gp_opt_t, acqui_t) \
{ \
    using GP_t = model::GP<Params, kernel::SquaredExpARD<Params>, mean::mean_t<Params>, model::gp::gp_opt_t<Params> >; \
    bayes_opt::BOptimizer<Params, modelfun<GP_t>, acquifun<acqui::acqui_t<Params, GP_t> >, acquiopt<opt::NLOptNoGrad<Params> > > opt; \
    opt.optimize(StateEval(this), Average()); \
    } \

#define LIMBO_OPTIMIZE_MEAN_ARD(gp_opt_t, acqui_t) \
{ \
    using GP_t = model::GP<Params, kernel::SquaredExpARD<Params>, mean::FunctionARD<Params, mean::Constant<Params> >, model::gp::gp_opt_t<Params> >; \
    bayes_opt::BOptimizer<Params, modelfun<GP_t>, acquifun<acqui::acqui_t<Params, GP_t> >, acquiopt<opt::NLOptNoGrad<Params> > > opt; \
    opt.optimize(StateEval(this), Average()); \
    } \

void StudyLimbo::solve()
{
    m_computationSets.clear();

    QString mean = meanToStringKey(value(LIMBO_mean).toString());
    QString gp = gpToStringKey(value(LIMBO_gp).toString());
    QString acqui = acquiToStringKey(value(LIMBO_acqui).toString());

    // unsupported combinations
    if (((mean == "data") && (gp == "mean_lf") && (acqui == "gpucb"))
            || ((mean == "data") && (gp == "mean_lf") && (acqui == "ei"))
            || ((mean == "data") && (gp == "kernel_mean_lf") && (acqui == "gpucb"))
            || ((mean == "data") && (gp == "kernel_mean_lf") && (acqui == "ei"))
            || ((mean == "constant") && (gp == "mean_lf") && (acqui == "gpucb"))
            || ((mean == "constant") && (gp == "mean_lf") && (acqui == "ei"))
            || ((mean == "constant") && (gp == "kernel_mean_lf") && (acqui == "gpucb"))
            || ((mean == "constant") && (gp == "kernel_mean_lf") && (acqui == "ei")))
    {
        Agros::log()->printError(tr("OptiLab"), tr("Unsupported combination: mean = %1, gp = %2, acqui = %3 ").arg(mean).arg(gp).arg(acqui));

        return;
    }

    m_isSolving = true;

    addComputationSet(tr("Initialization"));

    // noise
    Params::kernel::set_noise(value(LIMBO_bayes_opt_boptimizer_noise).toDouble());
    // init - random sampling
    Params::init_randomsampling::set_samples(value(LIMBO_init_randomsampling_samples).toInt());
    // max iterations
    Params::stop_maxiterations::set_iterations(value(LIMBO_stop_maxiterations_iterations).toInt());
    // Bayes Optimizer
    if (gp == "no_opt")
    {
        // no optimizer
        Params::bayes_opt_boptimizer::set_hp_period(-1);
    }
    else
    {
        // lf optimizer
        Params::bayes_opt_boptimizer::set_hp_period(value(LIMBO_bayes_opt_boptimizer_hp_period).toInt());
    }

    // Bayes Optimizer
    if ((mean == "data") && (gp == "kernel_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Data, KernelLFOpt, GP_UCB)
            else if ((mean == "data") && (gp == "kernel_lf") && (acqui == "ei")) LIMBO_OPTIMIZE(Data, KernelLFOpt, EI)
            // else if ((mean == "data") && (gp == "mean_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Data, MeanLFOpt, GP_UCB)
            // else if ((mean == "data") && (gp == "mean_lf") && (acqui == "ei")) LIMBO_OPTIMIZE(Data, MeanLFOpt, EI)
            // else if ((mean == "data") && (gp == "kernel_mean_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Data, KernelMeanLFOpt, GP_UCB)
            // else if ((mean == "data") && (gp == "kernel_mean_lf") && (acqui == "ei")) LIMBO_OPTIMIZE(Data, KernelMeanLFOpt, EI)
            else if ((mean == "data") && (gp == "no_opt") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Data, NoLFOpt, GP_UCB)
            else if ((mean == "data") && (gp == "no_opt") && (acqui == "ei")) LIMBO_OPTIMIZE(Data, NoLFOpt, EI)
            else if ((mean == "constant") && (gp == "kernel_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Constant, KernelLFOpt, GP_UCB)
            else if ((mean == "constant") && (gp == "kernel_lf") && (acqui == "ei")) LIMBO_OPTIMIZE(Constant, KernelLFOpt, EI)
            // else if ((mean == "constant") && (gp == "mean_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Constant, MeanLFOpt, GP_UCB)
            // else if ((mean == "constant") && (gp == "mean_lf") && (acqui == "ei")) LIMBO_OPTIMIZE(Constant, MeanLFOpt, EI)
            // else if ((mean == "constant") && (gp == "kernel_mean_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Constant, KernelMeanLFOpt, GP_UCB)
            // else if ((mean == "constant") && (gp == "kernel_mean_lf") && (acqui == "ei")) LIMBO_OPTIMIZE(Constant, KernelMeanLFOpt, EI)
            else if ((mean == "constant") && (gp == "no_opt") && (acqui == "gpucb")) LIMBO_OPTIMIZE(Constant, NoLFOpt, GP_UCB)
            else if ((mean == "constant") && (gp == "no_opt") && (acqui == "ei")) LIMBO_OPTIMIZE(Constant, NoLFOpt, EI)
            else if ((mean == "function_ard") && (gp == "kernel_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE_MEAN_ARD(KernelLFOpt, GP_UCB)
            else if ((mean == "function_ard") && (gp == "kernel_lf") && (acqui == "ei")) LIMBO_OPTIMIZE_MEAN_ARD(KernelLFOpt, EI)
            else if ((mean == "function_ard") && (gp == "mean_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE_MEAN_ARD(MeanLFOpt, GP_UCB)
            else if ((mean == "function_ard") && (gp == "mean_lf") && (acqui == "ei")) LIMBO_OPTIMIZE_MEAN_ARD(MeanLFOpt, EI)
            else if ((mean == "function_ard") && (gp == "kernel_mean_lf") && (acqui == "gpucb")) LIMBO_OPTIMIZE_MEAN_ARD(KernelMeanLFOpt, GP_UCB)
            else if ((mean == "function_ard") && (gp == "kernel_mean_lf") && (acqui == "ei")) LIMBO_OPTIMIZE_MEAN_ARD(KernelMeanLFOpt, EI)
            else if ((mean == "function_ard") && (gp == "no_opt") && (acqui == "gpucb")) LIMBO_OPTIMIZE_MEAN_ARD(NoLFOpt, GP_UCB)
            else if ((mean == "function_ard") && (gp == "no_opt") && (acqui == "ei")) LIMBO_OPTIMIZE_MEAN_ARD(NoLFOpt, EI)
            else assert(0);

    /*
    using stat_t = boost::fusion::vector<experimental::stat::ParetoFront<Params>,
    experimental::stat::HyperVolume<Params>, stat::ConsoleSummary<Params>>;
    */
    /*
    using GP_t = model::GP<Params, kernel::SquaredExpARD<Params>, mean::FunctionARD<Params, mean::Constant<Params> >, model::gp::KernelMeanLFOpt<Params> >;
    experimental::bayes_opt::Parego<Params, GP_t > opt;
    opt.optimize(StateEval(this), Average());
    */
    /*
    using stat_t = boost::fusion::vector<experimental::stat::ParetoFront<Params>, experimental::stat::HyperVolume<Params>, stat::ConsoleSummary<Params>>;
    experimental::bayes_opt::Ehvi<Params, statsfun<stat_t>> opt;
    opt.optimize(StateEval(this));
    */
    m_isSolving = false;
}

QString StudyLimbo::meanString(const QString &meanType) const
{
    if (meanType == "data")
        return QObject::tr("Data");
    else if (meanType == "constant")
        return QObject::tr("Constant");
    else if (meanType == "function_ard")
        return QObject::tr("FunctionARD");
    else
    {
        std::cerr << "mean '" + meanType.toStdString() + "' is not implemented. meanType(const QString &meanType)" << endl;
        throw;
    }
}

QString StudyLimbo::gpString(const QString &gpType) const
{
    if (gpType == "kernel_lf")
        return QObject::tr("Kernel LF");
    else if (gpType == "mean_lf")
        return QObject::tr("Mean LF");
    else if (gpType == "kernel_mean_lf")
        return QObject::tr("Kernel and mean LF");
    else if (gpType == "no_opt")
        return QObject::tr("No Optimization");
    else
    {
        std::cerr << "mean '" + gpType.toStdString() + "' is not implemented. gpString(const QString &gpType)" << endl;
        throw;
    }
}

QString StudyLimbo::acquiString(const QString &acquiType) const
{
    if (acquiType == "gpucb")
        return QObject::tr("GP-UCB (Upper Confidence Bound)");
    else if (acquiType == "ei")
        return QObject::tr("Classic EI (Expected Improvement)");
    else
    {
        std::cerr << "mean '" + acquiType.toStdString() + "' is not implemented. acquiString(const QString &acquiType)" << endl;
        throw;
    }
}

void StudyLimbo::setDefaultValues()
{    
    Study::setDefaultValues();

    m_settingDefault[LIMBO_init_randomsampling_samples] = 10;
    m_settingDefault[LIMBO_stop_maxiterations_iterations] = 20;
    m_settingDefault[LIMBO_bayes_opt_boptimizer_noise] = 1e-10;
    m_settingDefault[LIMBO_bayes_opt_boptimizer_hp_period] = 10;
    m_settingDefault[LIMBO_mean] = meanToStringKey("data");
    m_settingDefault[LIMBO_gp] = gpToStringKey("kernel_lf");
    m_settingDefault[LIMBO_acqui] = acquiToStringKey("gpucb");
}

void StudyLimbo::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[LIMBO_init_randomsampling_samples] = "LIMBO_init_randomsampling_samples";
    m_settingKey[LIMBO_stop_maxiterations_iterations] = "LIMBO_stop_maxiterations_iterations";
    m_settingKey[LIMBO_bayes_opt_boptimizer_noise] = "LIMBO_bayes_opt_boptimizer_noise";
    m_settingKey[LIMBO_bayes_opt_boptimizer_hp_period] = "LIMBO_bayes_opt_boptimizer_hp_period";
    m_settingKey[LIMBO_mean] = "LIMBO_mean";
    m_settingKey[LIMBO_gp] = "LIMBO_gp";
    m_settingKey[LIMBO_acqui] = "LIMBO_acqui";
}

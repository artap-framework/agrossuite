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

#include "study_nsga3.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include "alg_nsgaiii.h"
#include "alg_individual.h"
#include "alg_population.h"
#include "problem_base.h"
/*
#include "../../3rdparty/Eigen/Core"
#include "limbo/model/gp.hpp"
#include "limbo/model/gp/kernel_mean_lf_opt.hpp"
#include "limbo/model/gp/kernel_lf_opt.hpp"
#include "limbo/model/gp/mean_lf_opt.hpp"
#include "limbo/model/gp/no_lf_opt.hpp"
#include "limbo/kernel/squared_exp_ard.hpp"
#include "limbo/mean/data.hpp"
#include "limbo/mean/constant.hpp"
#include "limbo/mean/function_ard.hpp"
*/

/*
struct Params {
    // struct opt_rprop : public limbo::defaults::opt_rprop {};
    struct opt_rprop { BO_PARAM(int, iterations, 100); }; // default 300
    // struct opt_parallelrepeater : public limbo::defaults::opt_parallelrepeater {};
    struct opt_parallelrepeater { BO_PARAM(int, repeats, 10); }; // default 10
};
*/
class NSGA3Problem : public NSGA3ProblemBase
{
public:
    NSGA3Problem(StudyNSGA3 *study) : NSGA3ProblemBase("NSGA3"), m_study(study), relearn(30), countComputation(0), countSurrogate(0), m_steps(0)
    {
        lbs_.resize(m_study->parameters().count());
        ubs_.resize(m_study->parameters().count());

        // set bounding box
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];

            lbs_[i] = parameter.lowerBound();
            ubs_[i] = parameter.upperBound();
        }
    }

    virtual std::size_t num_variables() const
    {
        return m_study->parameters().count();
    }

    virtual std::size_t num_objectives() const
    {
        return m_study->functionals().count();
    }

    virtual bool Evaluate(CIndividual *indv) const
    {
        CIndividual::TDecVec &x = indv->vars();
        CIndividual::TObjVec &f = indv->objs();

        if (m_study->isAborted())
            return false;

        // computation
        QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);
        // m_study->addComputation(computation);

        // training patterns
        // Eigen::VectorXd samples(m_study->parameters().count());
        // Gaussian process parameters
        // Eigen::VectorXd mu;
        double sigma;

        // set parameters
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            qInfo() << parameter.name() << computation->config()->parameters()->items().values()[i].value() << x[i];
            computation->config()->parameters()->set(parameter.name(), x[i]);

            // training patterns
            // samples[i] = x[i];
        }
        computation->clearSolution();
        computation->scene()->invalidate();

        // evaluate step
        try
        {
            bool useSurrogate = false; // m_study->value(Study::NSGA3_use_surrogate).toBool() && (gp.nb_samples() > relearn - 1);

            if (useSurrogate)
            {
                /*
                // estimate values
                std::tie(mu, sigma) = gp.query(samples);
                double lik = gp.get_lik();
                qDebug() << "sigma = " << sigma << ", lik = " << lik;

                // qDebug() << "surrogate function";
                // for (int i = 0; i < m_study->functionals().count(); i++)
                //     qDebug() << i << " : surr : mu = " << mu[i] << ", sigma = " << sigma << ", lik = " << gp.get_lik() << ", use = " << (sigma < 1e-4);
                // surrogate function
                if (sigma < 1e-4)
                {
                    // weight functionals
                    int totalWeight = 0;
                    foreach (Functional functional, m_study->functionals())
                        totalWeight += functional.weight();

                    QList<double> values;
                    double totalValue = 0.0;

                    // set objective functions
                    for (int i = 0; i < m_study->functionals().count(); i++)
                    {
                        f[i] = mu[i];
                        values.append(mu[i]);

                        totalValue += ((double) m_study->functionals()[i].weight() / totalWeight) * mu[i];
                    }

                    Agros::computations().remove(computation->problemDir());

                    countSurrogate++;

                    // update chart
                    emit m_study->updateChart(values, totalValue, SolutionUncertainty());
                }
                else
                {
                    // force computation
                    useSurrogate = false;
                }
                */
            }
            else
            {
                // real computation and model improvement
                if (!useSurrogate)
                {
                    // qDebug() << "real computation";

                    // design of experiments
                    if (m_study->value(Study::General_DoE).toBool())
                    {
                        // base point for DoE
                        QVector<double> init(m_study->parameters().count());
                        for (int i = 0; i < m_study->parameters().count(); i++)
                            init[i] = x[i];

                        // DoE
                        m_study->doeCompute(computation, init);
                    }

                    m_study->evaluateStep(computation);
                    QList<double> values = m_study->evaluateMultiGoal(computation);

                    if (m_study->value(Study::General_ClearSolution).toBool())
                        computation->clearSolution();

                    // add computation
                    m_study->addComputation(computation);

                    // penalty
                    double totalPenalty = 0.0;
                    for (int i = 0; i < m_study->parameters().count(); i++)
                    {
                        Parameter parameter = m_study->parameters()[i];
                        if (parameter.penaltyEnabled())
                            totalPenalty += parameter.penalty(x[i]);
                    }

                    // set objective functions
                    countComputation++;
                    for (int i = 0; i < values.count(); i++)
                        f[i] = values[i] + totalPenalty;

                    /*
                    if (m_study->value(Study::NSGA3_use_surrogate).toBool())
                    {
                        // estimate values
                        if (gp.nb_samples() > 1)
                        {
                            std::tie(mu, sigma) = gp.query(samples);
                             for (int i = 0; i < values.count(); i++)
                                 qDebug() << i << " : value = " << values[i] << ", mu = " << mu[i] << ", sigma = " << sigma;
                        }

                        // add samples and observations
                        Eigen::VectorXd observations(m_study->functionals().count());
                        for (int i = 0; i < values.count(); i++)
                            observations[i] = values[i];

                        Eigen::VectorXd noises(m_study->functionals().count());
                        for (int i = 0; i < values.count(); i++)
                            noises[i] = 0.0;

                        m_samples.push_back(samples);
                        m_observations.push_back(observations);

                        // refresh model
                        if (m_observations.size() % relearn == 0)
                            gp.compute(m_samples, m_observations, noises, 1e-10);
                    }
                    */
                }
            }

            // add set
            int computationCount = 0;
            foreach (ComputationSet computationSet, m_study->computationSets())
                computationCount += computationSet.computations().count();

            if (computationCount > 1 && computationCount < m_study->estimatedNumberOfSteps())
            {
                if (computationCount % m_study->currentPopulationSize() == 0)
                    m_study->addComputationSet(QObject::tr("Step %1").arg(computationCount / m_study->currentPopulationSize()));
            }

            m_steps++;
            qInfo() << "NSGA-III: step " << m_steps << "/" << m_study->estimatedNumberOfSteps();

            return true;
        }
        catch (AgrosSolverException &e)
        {
            qDebug() << e.toString();

            // opt.set_force_stop(2);
        }

        return false;
    }

    mutable int countComputation;
    mutable int countSurrogate;
private:
    StudyNSGA3 *m_study;

    // mutable limbo::model::GP<Params, limbo::kernel::SquaredExpARD<Params>, limbo::mean::Data<Params>, limbo::model::gp::KernelLFOpt<Params> > gp;
    // X mutable limbo::model::GP<Params, limbo::kernel::SquaredExpARD<Params>, limbo::mean::Data<Params>, limbo::model::gp::NoLFOpt<Params> > gp;
    // X mutable limbo::model::GP<Params, limbo::kernel::SquaredExpARD<Params>, limbo::mean::FunctionARD<Params, limbo::mean::Data<Params> >, limbo::model::gp::MeanLFOpt<Params> > gp;

    // parameters
    int relearn;
    // mutable std::vector<Eigen::VectorXd> m_samples;
    // mutable std::vector<Eigen::VectorXd> m_observations;

    mutable int m_steps;
};

StudyNSGA3::StudyNSGA3() : Study()
{
}

int StudyNSGA3::estimatedNumberOfSteps() const
{
    return currentPopulationSize() * (value(NSGA3_ngen).toInt() + 1);
}

int StudyNSGA3::currentPopulationSize() const
{
    int estSize = (value(NSGA3_popsize).toInt() + m_functionals.count() - 1);
    while (estSize % 4) estSize++;

    return estSize;
}

void StudyNSGA3::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    addComputationSet(QObject::tr("Initilization"));

    // nsga3
    NSGA3Problem nsga3Problem(this);

    // solutions
    CPopulation solutions;

    NSGA3 nsga3(value(NSGA3_pcross).toDouble(),
                value(NSGA3_pmut).toDouble(),
                value(NSGA3_eta_c).toDouble(),
                value(NSGA3_eta_m).toDouble(),
                value(NSGA3_popsize).toInt(),
                value(NSGA3_ngen).toInt());
    nsga3.Solve(&solutions, nsga3Problem);

    qDebug() << "countComputation = " << nsga3Problem.countComputation << ", countSurrogate = " << nsga3Problem.countSurrogate << " (" <<
                100.0 * (double) nsga3Problem.countSurrogate / (nsga3Problem.countComputation + nsga3Problem.countSurrogate) << " %)";

    m_isSolving = false;

    // sort computations
    // QString parameterName = m_functionals[0].name();
    // m_computationSets.last().sort(parameterName);
}

void StudyNSGA3::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[NSGA3_popsize] = 4;
    m_settingDefault[NSGA3_ngen] = 3;
    m_settingDefault[NSGA3_pcross] = 1.0;
    // m_settingDefault[NSGA3_pmut] = 0.2;
    m_settingDefault[NSGA3_eta_c] = 30.0;
    m_settingDefault[NSGA3_eta_m] = 20.0;
    m_settingDefault[NSGA3_use_surrogate] = false;
}

void StudyNSGA3::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[NSGA3_popsize] = "NSGA3_popsize";
    m_settingKey[NSGA3_ngen] = "NSGA3_ngen";
    m_settingKey[NSGA3_pcross] = "NSGA3_pcross";
    // m_settingKey[NSGA3_pmut] = "NSGA3_pmut";
    m_settingKey[NSGA3_eta_c] = "NSGA3_eta_c";
    m_settingKey[NSGA3_eta_m] = "NSGA3_eta_m";
    m_settingKey[NSGA3_use_surrogate] = "NSGA3_use_surrogate";
}

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

#include "study_sweep.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

const QString N_INIT_SAMPLES = "n_init_samples";
const QString N_ITERATIONS = "n_iterations";
const QString N_ITER_RELEARN = "n_iter_relearn";
const QString INIT_METHOD = "init_method";

SweepProblem::SweepProblem(StudySweep *study, bayesopt::Parameters par)
    : ContinuousModel(study->parameters().count(), par), m_study(study), m_steps(0)
{
    vectord lowerBound(m_study->parameters().count());
    vectord upperBound(m_study->parameters().count());

    // set bounding box
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];

        lowerBound.insert_element(i, parameter.lowerBound());
        upperBound.insert_element(i, parameter.upperBound());
    }

    setBoundingBox(lowerBound, upperBound);
}

double SweepProblem::evaluateSample(const vectord& x)
{
    // computation
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

    // set parameters
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->parameters()->set(parameter.name(), x[i]);
    }

    // evaluate step
    try
    {
        m_study->evaluateStep(computation);
        double value = m_study->evaluateSingleGoal(computation);

        // design of experiments
        if (m_study->value(Study::General_DoE).toBool())
        {
            // base point for DoE
            QVector<double> init(m_study->parameters().count());
            for (int i = 0; i < m_study->parameters().count(); i++)
                init[i] = x[i];

            // DoE
            m_study->doeCompute(computation, init, value);
        }

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        // add computation
        m_study->addComputation(computation);

        m_steps++;
        qInfo() << "Sweep: step " << m_steps << "/" << m_study->estimatedNumberOfSteps();

        return value;
    }
    catch (AgrosSolverException &e)
    {
        qDebug() << e.toString();

        return numeric_limits<double>::max();
    }
}

StudySweep::StudySweep() : Study()
{
    // init method
    initMethodList.insert(1, "lhs");
    initMethodList.insert(2, "sobol");
}

int StudySweep::estimatedNumberOfSteps() const
{
    return value(Study::Sweep_num_samples).toInt();
}

QString StudySweep::initMethodString(int method) const
{
    if (method == 1)
        return QObject::tr("Latin Hypercube Sampling (LHS)");
    else if (method == 2)
        return QObject::tr("Sobol Sequences");
    else
    {
        std::cerr << "init method '" + QString::number(method).toStdString() + "' is not implemented. initMethodString(int method)" << endl;
        throw;
    }
}

void StudySweep::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    // parameters
    bayesopt::Parameters par = initialize_parameters_to_default();
    par.n_init_samples = value(Sweep_num_samples).toInt();
    par.n_iterations = 0;
    par.n_iter_relearn = 1000000;
    par.init_method = initMethodFromStringKey(value(Sweep_init_method).toString());
    par.noise = 0;
    par.random_seed = 0;
    par.verbose_level = 0;

    SweepProblem sweepProblem(this, par);

    // init BayesOpt problem
    addComputationSet(tr("Sweep"));
    sweepProblem.initializeOptimization();

    m_isSolving = false;
}

void StudySweep::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[Sweep_num_samples] = 20;
    m_settingDefault[Sweep_init_method] = initMethodToStringKey(1); // 1-LHS, 2-Sobol
}

void StudySweep::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[Sweep_num_samples] = "Sweep_num_samples";
    m_settingKey[Sweep_init_method] = "Sweep_init_method";
}

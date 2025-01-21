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

const QString N_INIT_SAMPLES = "n_init_samples";
const QString N_ITERATIONS = "n_iterations";
const QString N_ITER_RELEARN = "n_iter_relearn";
const QString INIT_METHOD = "init_method";

SweepProblem::SweepProblem(StudySweep *study) : m_study(study), m_steps(0)
{

}

double SweepProblem::evaluate(const vectord& x)
{
    if (m_study->isAborted())
        return numeric_limits<double>::max();

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

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        // add computation
        m_study->addComputation(computation);

        m_steps++;

        return value;
    }
    catch (AgrosOptilabEvaluationException &e)
    {
        computation.clear();
        return numeric_limits<double>::max();
    }
}

SweepProblemRandom::SweepProblemRandom(StudySweep *study) : SweepProblem(study)
{
}

void SweepProblemRandom::evaluateRandom()
{
    int numSamples = m_study->estimatedNumberOfSteps();

    for (int i = 0; i < numSamples; i++)
    {
        vectord x(m_study->parameters().count());

        for (int j = 0; j < m_study->parameters().count(); j++)
        {
            Parameter parameter = m_study->parameters()[j];

            x.insert_element(j, parameter.lowerBound() + (parameter.upperBound() - parameter.lowerBound()) * (double)rand() / RAND_MAX);
        }

        evaluate(x);
    }
}

SweepProblemUniform::SweepProblemUniform(StudySweep *study) : SweepProblem(study)
{
}

void SweepProblemUniform::evaluateUniform()
{
    // TODO: more options for uniform sampling
    int numSamples = m_study->estimatedNumberOfSteps();

    for (int i = 0; i < numSamples; i++)
    {
        vectord x(m_study->parameters().count());

        for (int j = 0; j < m_study->parameters().count(); j++)
        {
            Parameter parameter = m_study->parameters()[j];

            double step = (parameter.upperBound() - parameter.lowerBound()) / (numSamples - 1);

            x.insert_element(j, parameter.lowerBound() + i * step);
        }

        evaluate(x);
    }
}

// *************************************************************************************************

SweepProblemBayesOpt::SweepProblemBayesOpt(StudySweep *study, bayesopt::Parameters par)
    : SweepProblem(study), ContinuousModel(study->parameters().count(), par)
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

StudySweep::StudySweep() : Study()
{
    // init method
    initMethodList.insert(0, "uniform");
    initMethodList.insert(1, "lhs");
    initMethodList.insert(2, "sobol");
    initMethodList.insert(3, "random");
}

int StudySweep::estimatedNumberOfSteps() const
{
    int numSamples = value(Study::Sweep_num_samples).toInt();

    if (numSamples < 2)
        numSamples = 2;

    return numSamples;
}

QString StudySweep::initMethodString(int method) const
{
    if (method == 0)
        return QObject::tr("Uniform Sampling");
    else if (method == 1)
        return QObject::tr("Latin Hypercube Sampling (LHS)");
    else if (method == 2)
        return QObject::tr("Sobol Sequences");
    else if (method == 3)
        return QObject::tr("Random Sampling");
    else
    {
        std::cerr << "init method '" + QString::number(method).toStdString() + "' is not implemented. initMethodString(int method)" << endl;
        throw;
    }
}

void StudySweep::solve()
{
    // start computation
    Study::solve();

    m_isSolving = true;

    addComputationSet(tr("Sweep"));

    int method = initMethodFromStringKey(value(Sweep_init_method).toString());

    // uniform
    if (method == 0)
    {
        // uniform sampling
        SweepProblemUniform sweepProblem(this);
        sweepProblem.evaluateUniform();
    }
    else if ((method == 1) || (method == 2))
    {
        // use bayesopt
        // parameters
        bayesopt::Parameters par = initialize_parameters_to_default();
        par.n_init_samples = estimatedNumberOfSteps();
        par.n_iterations = 0;
        par.n_iter_relearn = 1000000;
        par.init_method = method;
        par.noise = 0;
        par.random_seed = 0;
        par.verbose_level = 0;

        SweepProblemBayesOpt sweepProblem(this, par);

        // init BayesOpt problem
        sweepProblem.initializeOptimization();
    }
    else if (method == 3)
    {
        // random sampling
        SweepProblemRandom sweepProblem(this);
        sweepProblem.evaluateRandom();
    }
    else
    {
        std::cerr << "init method '" + QString::number(method).toStdString() + "' is not implemented. solve()" << endl;
        throw;
    }

    // remove empty computation sets
    this->removeEmptyComputationSets();

    m_isSolving = false;
    m_abort = false;
}

void StudySweep::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[Sweep_num_samples] = 20;
    // 0-Uniform
    // 1-LHS
    // 2-Sobol
    m_settingDefault[Sweep_init_method] = initMethodToStringKey(0);
}

void StudySweep::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[Sweep_num_samples] = "Sweep_num_samples";
    m_settingKey[Sweep_init_method] = "Sweep_init_method";
}

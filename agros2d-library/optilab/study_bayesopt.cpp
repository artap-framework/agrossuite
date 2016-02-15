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

#include "study_bayesopt.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"


BayesOptProblem::BayesOptProblem(StudyBayesOptAnalysis *study, bayesopt::Parameters par)
    : ContinuousModel(study->parameters().count(), par), m_study(study)
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

double BayesOptProblem::evaluateSample(const vectord& x)
{   
    // computation
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
    m_study->addComputation(computation);

    // set parameters
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->setParameter(parameter.name(), x[i]);
    }

    // solve
    computation->solve();

    // TODO: better error handling
    if (!computation->isSolved())
    {
        return numeric_limits<double>::max();
    }

    // evaluate functionals
    m_study->evaluateFunctionals(computation);

    // TODO: more functionals !!!
    assert(m_study->functionals().size() == 1);
    QString parameterName = m_study->functionals()[0].name();

    double value = computation->results()->resultValue(parameterName);
    computation->saveResults();

    m_study->updateParameters(m_study->parameters(), computation.data());
    m_study->updateChart();

    return value;
}

StudyBayesOptAnalysis::StudyBayesOptAnalysis() : Study()
{    
}

void StudyBayesOptAnalysis::solve()
{
    m_isSolving = true;

    // parameters
    bayesopt::Parameters par = initialize_parameters_to_default();
    par.n_init_samples = 2;
    par.n_iterations = 3;
    par.init_method = 1; // 1-LHS, 2-Sobol
    par.noise = 1e-10;
    par.random_seed = 0;
    par.verbose_level = 1;

    BayesOptProblem bayesOptProblem(this, par);

    // init BayesOpt problem
    addComputationSet(tr("Initialization"));
    bayesOptProblem.initializeOptimization();

    // steps   
    addComputationSet(tr("Steps"));
    for (int i = 0; i < bayesOptProblem.getParameters()->n_iterations; i++)
    {
        // step
        bayesOptProblem.stepOptimization();
    }

    // sort computations
    QString parameterName = m_functionals[0].name();
    m_computationSets.last().sort(parameterName);

    // vectord result = bayesOptProblem.getFinalResult();

    m_isSolving = false;

    emit solved();
}

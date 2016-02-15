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

#include "study_nlopt.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include "nlopt.hpp"

class NLoptProblem
{
public:
    NLoptProblem(StudyNLoptAnalysis *study) :  m_study(study)
    {

    }

    double objectiveFunction(const std::vector<double> &x, std::vector<double> &grad, void *data)
    {
        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
        m_study->addComputation(computation);

        // static BayesOptPhase currentPhase = m_phase;

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

private:
    StudyNLoptAnalysis *m_study;
};

double objFunctionWrapper(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
    NLoptProblem *obj = static_cast<NLoptProblem *>(data);
    return obj->objectiveFunction(x, grad, data);
}

StudyNLoptAnalysis::StudyNLoptAnalysis() : Study()
{
}

void StudyNLoptAnalysis::solve()
{
    std::vector<double> initialGuess(m_parameters.count());
    std::vector<double> lowerBound(m_parameters.count());
    std::vector<double> upperBound(m_parameters.count());

    // set bounding box
    for (int i = 0; i < m_parameters.count(); i++)
    {
        Parameter parameter = m_parameters[i];

        lowerBound[i] = parameter.lowerBound();
        upperBound[i] = parameter.upperBound();
        initialGuess[i] = (parameter.lowerBound() + parameter.upperBound()) / 2.0;
    }

    NLoptProblem nLoptProblem(this);

    nlopt::opt opt(nlopt::LN_BOBYQA, m_parameters.count());
    opt.set_min_objective(objFunctionWrapper, &nLoptProblem);
    opt.set_lower_bounds(lowerBound);
    opt.set_upper_bounds(upperBound);
    opt.set_xtol_rel(1e-4);
    opt.set_ftol_rel(1e-4);

    double minimum = 0.0; // numeric_limits<double>::max();
    nlopt::result result = opt.optimize(initialGuess, minimum);

    if (result == nlopt::SUCCESS)
    {
        qDebug() << "optimized variant: " << minimum;
        for (int i = 0; i < m_parameters.count(); i++)
            qDebug() << initialGuess[i];
    }
    else
    {
        qDebug() << "err ";
    }
}

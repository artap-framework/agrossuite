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

#include "study.h"
#include "parameter.h"

#include "optilab.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

Study::Study()
{

}

Study::~Study()
{
    clear();
}

void Study::clear()
{
    m_resultExpressions.clear();
}

// ********************************************************************************

StudySweepAnalysis::StudySweepAnalysis() : Study()
{
}

void StudySweepAnalysis::solve()
{
    // only one parameter
    assert(m_parameters.size() == 1);

    Parameter parameter = m_parameters.first();

    foreach (double value, parameter.values())
    {
        // set parameter
        Agros2D::preprocessor()->config()->setParameter(parameter.name(), value);

        // create computation
        QSharedPointer<ProblemComputation> computation = Agros2D::preprocessor()->createComputation(true, false);
        // store computation
        m_computations.append(computation);

        // solve
        computation->solve();

        // postprocessor
        // TODO: script !!!
        FieldInfo *fieldInfo = computation->fieldInfo("electrostatic");
        computation->scene()->labels->setSelected(true);
        std::shared_ptr<IntegralValue> values = fieldInfo->plugin()->volumeIntegral(computation.data(),
                                                                                    fieldInfo,
                                                                                    0,
                                                                                    0);

        QMap<QString, double> integrals = values->values();
        computation->result()->setResult("We", integrals["electrostatic_energy"]);
        computation->result()->setResult("V", integrals["electrostatic_volume"]);
        // TODO: script !!!

        computation->saveResults();
    }
}

// ********************************************************************************

StudyGoldenSectionSearch::StudyGoldenSectionSearch(double tolerance) : Study(),
    m_tolerance(tolerance)
{
}

double StudyGoldenSectionSearch::valueForParameter(const QString &name, double value)
{
    // set parameter
    Agros2D::preprocessor()->config()->setParameter(name, value);
    // create computation
    QSharedPointer<ProblemComputation> computation = Agros2D::preprocessor()->createComputation(true, false);
    // store computation
    m_computations.append(computation);

    // solve
    computation->solve();

    // postprocessor
    // TODO: script !!!
    FieldInfo *fieldInfo = computation->fieldInfo("electrostatic");
    computation->scene()->labels->setSelected(true);
    std::shared_ptr<IntegralValue> values = fieldInfo->plugin()->volumeIntegral(computation.data(),
                                                                                fieldInfo,
                                                                                0,
                                                                                0);

    QMap<QString, double> integrals = values->values();
    computation->result()->setResult("We", integrals["electrostatic_energy"]);

    // "functional"
    double functional = integrals["electrostatic_energy"] / integrals["electrostatic_volume"];
    computation->result()->setResult("fct", functional);

    computation->saveResults();

    return functional;
    // TODO: script !!!
}

void StudyGoldenSectionSearch::solve()
{
    // only one parameter
    assert(m_parameters.size() == 1);

    Parameter parameter = m_parameters.first();

    double goldenRate = (sqrt(5) - 1) / 2;

    double a = parameter.lowerBound();
    double b = parameter.upperBound();

    double xL = b - goldenRate*(b - a);
    double xR = a + goldenRate*(b - a);

    while ((b - a) > m_tolerance)
    {
        double fc = valueForParameter(parameter.name(), xL);
        double fd = valueForParameter(parameter.name(), xR);

        if (fc < fd)
        {
            b = xR;
            xR = xL;
            xL = b - goldenRate * (b - a);
        }
        else
        {
            a = xL;
            xL = xR;
            xR = a + goldenRate * (b - a);
        }

        // qDebug() << fabs(xL-xR);
    }
}

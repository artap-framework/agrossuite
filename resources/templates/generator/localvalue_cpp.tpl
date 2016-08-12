// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

// #include "{{ID}}_extfunction.h"
#include "{{ID}}_localvalue.h"
#include "{{ID}}_interface.h"

#include "util/global.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/field.h"
#include "solver/solutionstore.h"

#include "solver/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>


{{CLASS}}LocalValue::{{CLASS}}LocalValue(Computation *computation,
                                         const FieldInfo *fieldInfo,
                                         int timeStep,
                                         int adaptivityStep,
                                         const Point &point)
    : LocalValue(computation, fieldInfo, timeStep, adaptivityStep, point)
{
    calculate();
}

void {{CLASS}}LocalValue::calculate()
{
    int numberOfSolutions = m_fieldInfo->numberOfSolutions();

    m_values.clear();

    if (m_computation->isSolved())
    {
        double frequency = m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number();

        FieldSolutionID fsid(m_fieldInfo->fieldId(), m_timeStep, m_adaptivityStep);
        // check existence
        if (!m_computation->solutionStore()->contains(fsid))
            return;

        MultiArray ma = m_computation->solutionStore()->multiArray(fsid);

        // find marker
        SceneLabel *label = SceneLabel::findLabelAtPoint(m_computation->scene(), m_point);
        if (label && label->hasMarker(m_fieldInfo))
        {
            SceneMaterial *material = label->marker(m_fieldInfo);
            if (!material->isNone())
            {
                {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
                {{/VARIABLE_MATERIAL}}

                int k = 0; // only one point
                std::vector<dealii::Vector<double> > solution_values(1, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
                std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(1, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

                dealii::Point<2> p(m_point.x, m_point.y);

                for (int i = 0; i < numberOfSolutions; i++)
                {
                    if (m_fieldInfo->analysisType() == AnalysisType_Transient && m_timeStep == 0)
                    {
                        // set variables
                        solution_values[k][i] = m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble();
                        solution_grads[k][i][0] = 0;
                        solution_grads[k][i][1] = 0;
                    }
                    else
                    {
                        // point values
                        dealii::Functions::FEFieldFunction<2, dealii::hp::DoFHandler<2> > localvalues(ma.doFHandler(), ma.solution());

                        // set variables
                        solution_values[k][i] = localvalues.value(p, i);
                        solution_grads[k][i] = localvalues.gradient(p, i);
                    }
                }

                // expressions
                {{#VARIABLE_SOURCE}}
                if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                        && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
                    m_values[QLatin1String("{{VARIABLE}}")] = LocalPointValue({{EXPRESSION_SCALAR}}, Point({{EXPRESSION_VECTORX}}, {{EXPRESSION_VECTORY}}), material);
                {{/VARIABLE_SOURCE}}
            }
        }
    }
}

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
#include "{{ID}}_force.h"

#include "util/global.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"
#include "solver/solver_utils.h"

#include "solver/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

{{CLASS}}ForceValue::{{CLASS}}ForceValue(Computation *computation,
                                         const FieldInfo *fieldInfo,
                                         int timeStep,
                                         int adaptivityStep)
    : ForceValue(computation, fieldInfo, timeStep, adaptivityStep)
{
    FieldSolutionID fsid(m_fieldInfo->fieldId(), m_timeStep, m_adaptivityStep);
    ma = m_computation->solutionStore()->multiArray(fsid);

    // point values
    localvalues = std::shared_ptr<dealii::Functions::FEFieldFunction<2, dealii::hp::DoFHandler<2> > >
            (new dealii::Functions::FEFieldFunction<2, dealii::hp::DoFHandler<2> >(ma.doFHandler(), ma.solution()));
}

bool {{CLASS}}ForceValue::hasForce()
{
    {{#VARIABLE_SOURCE}}
    if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
    {
        return true;
    }
    {{/VARIABLE_SOURCE}}

    return false;
}

Point3 {{CLASS}}ForceValue::force(const Point3 &point, const Point3 &velocity)
{
    Point3 res;

    if (m_computation->isSolved())
    {
        int numberOfSolutions = m_fieldInfo->numberOfSolutions();

        // set variables
        double x = point.x;
        double y = point.y;
        dealii::Point<2> p(point.x, point.y);

        /*
        try
        {
            std::pair<TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Point<2> > cell
                    = dealii::GridTools::find_active_cell_around_point(*ProblemSolver::mappingCollection(m_fieldInfo), *ma.doFHandler(), p);
            currentCell = cell.first;

            localvalues->set_active_cell(currentCell);
        }
        catch (const TYPENAME dealii::GridTools::ExcPointNotFound<2> &e)
        {
            // do nothing - another cell
        }
        */

        // find material
        SceneMaterial *material = nullptr;
        SceneLabel *label = SceneLabel::findLabelAtPoint(m_computation->scene(), Point(point.x, point.y));
        if (label && label->hasMarker(m_fieldInfo))
        {
            material = label->marker(m_fieldInfo);
            if (material->isNone())
                return Point3();
        }
        else
        {
            // point not found
            return Point3();
        }

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}

        int k = 0; // only one point
        std::vector<dealii::Vector<double> > solution_values(1, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
        std::vector<std::vector<dealii::Tensor<1,2> > >  solution_gradients(1, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

        for (int i = 0; i < numberOfSolutions; i++)
        {
            // point values
            try
            {
                if (m_fieldInfo->analysisType() == AnalysisType_Transient && m_timeStep == 0)
                {
                    // set variables
                    solution_values[k][i] = m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble();
                    solution_gradients[k][i][0] = 0;
                    solution_gradients[k][i][1] = 0;
                }
                else
                {
                    // set variables
                    solution_values[k][i] = localvalues->value(p, i);
                    solution_gradients[k][i] = localvalues->gradient(p, i);
                }
            }
            catch (...) // (const TYPENAME dealii::GridTools::ExcPointNotFound<2> &e)
            {
                throw AgrosException(QObject::tr("Point [%1, %2] does not lie in any element").arg(x).arg(y));

                return res;
            }
        }

        {{#VARIABLE_SOURCE}}
        if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
         && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
        {
            res.x = {{EXPRESSION_X}};
            res.y = {{EXPRESSION_Y}};
            res.z = {{EXPRESSION_Z}};
        }
        {{/VARIABLE_SOURCE}}
    }

    return res;
}

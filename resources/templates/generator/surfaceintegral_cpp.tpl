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
#include "{{ID}}_surfaceintegral.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/field.h"
#include "solver/solutionstore.h"

#include "solver/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

#include <deal.II/numerics/vector_tools.h>

#include <deal.II/base/work_stream.h>

{{CLASS}}SurfaceIntegral::{{CLASS}}SurfaceIntegral(Computation *computation,
                                                   const FieldInfo *fieldInfo,
                                                   int timeStep,
                                                   int adaptivityStep)
    : IntegralValue(computation, fieldInfo, timeStep, adaptivityStep)
{
    m_values.clear();

    if (m_computation->isSolved())
    {
        FieldSolutionID fsid(m_fieldInfo->fieldId(), m_timeStep, m_adaptivityStep);
        ma = m_computation->solutionStore()->multiArray(fsid);

        // Gauss quadrature - volume
        dealii::hp::QCollection<2> quadratureFormulas;
        for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
            quadratureFormulas.push_back(dealii::QGauss<2>(degree + 1));

        // Gauss quadrature - surface
        dealii::hp::QCollection<2-1> faceQuadratureFormulas;
        for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
            faceQuadratureFormulas.push_back(dealii::QGauss<2-1>(degree + 1));

        // update time functions
        if (!m_computation->isSolving() && m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            Module::updateTimeFunctions(m_computation, m_computation->timeStepToTotalTime(m_timeStep));
        }

        dealii::WorkStream::run(ma.doFHandler()->begin_active(),
                                ma.doFHandler()->end(),
                                *this,
                                &{{CLASS}}SurfaceIntegral::localAssembleSystem,
                                &{{CLASS}}SurfaceIntegral::copyLocalToGlobal,
                                IntegralScratchData(ma.doFHandler()->get_fe(),
                                                    quadratureFormulas,
                                                    faceQuadratureFormulas),
                                IntegralCopyData());
    }
}

void {{CLASS}}SurfaceIntegral::localAssembleSystem(const typename dealii::hp::DoFHandler<2>::active_cell_iterator &cell_int,
                                 IntegralScratchData &scratch_data,
                                 IntegralCopyData &copy_data)
{
    double frequency = m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number();

    for (int iFace = 0; iFace < m_computation->scene()->faces->count(); iFace++)
    {
        SceneFace *edge = m_computation->scene()->faces->at(iFace);
        if (!edge->isSelected())
            continue;

        SceneLabel *label = m_computation->scene()->labels->at(cell_int->material_id() - 1);
        SceneMaterial *material = label->marker(m_fieldInfo);

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}

        // surface integration
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if (cell_int->face(face)->user_index() - 1 == iFace)
            {
                scratch_data.hp_fe_face_values.reinit(cell_int, face);

                const bool atBoundary = cell_int->face(face)->at_boundary();

                const dealii::FEFaceValues<2> &fe_values = scratch_data.hp_fe_face_values.get_present_fe_values();
                const unsigned int n_face_q_points = fe_values.n_quadrature_points;

                std::vector<dealii::Vector<double> > solution_values(n_face_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
                std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(n_face_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

                fe_values.get_function_values(ma.solution(), solution_values);
                fe_values.get_function_gradients(ma.solution(), solution_grads);

                {{#VARIABLE_SOURCE}}
                if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
                {
                    double res = 0.0;
                    for (unsigned int k = 0; k < n_face_q_points; ++k)
                    {
                        const dealii::Point<2> p = fe_values.quadrature_point(k);
                        const dealii::Tensor<1,2> normal = fe_values.normal_vector(k);

                        res += (atBoundary ? 1.0 : 0.5) * fe_values.JxW(k) * ({{EXPRESSION}});
                    }
                    copy_data.results[QLatin1String("{{VARIABLE}}")] += res;
                }
                {{/VARIABLE_SOURCE}}
            }
        }
    }
}

void {{CLASS}}SurfaceIntegral::copyLocalToGlobal(const IntegralCopyData &copy_data)
{
    // expressions
    {{#VARIABLE_SOURCE}}
    if ((m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_computation->config()->coordinateType() == {{COORDINATE_TYPE}}))
    {
        m_values[QLatin1String("{{VARIABLE}}")] += copy_data.results[QLatin1String("{{VARIABLE}}")];
    }
    {{/VARIABLE_SOURCE}}
}

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
#include "{{ID}}_filter.h"
#include "{{ID}}_interface.h"

#include "util/global.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/fe/mapping_q1.h>
#include <deal.II/numerics/fe_field_function.h>

{{CLASS}}ViewScalarFilter::{{CLASS}}ViewScalarFilter(Computation *computation,
                                                     const FieldInfo *fieldInfo,
                                                     int timeStep,
                                                     int adaptivityStep,
                                                     const QString &variable,
                                                     PhysicFieldVariableComp physicFieldVariableComp)
    : dealii::DataPostprocessorScalar<2>("Field",  dealii::update_values | dealii::update_gradients | dealii::update_hessians | dealii::update_q_points),
      m_computation(computation), m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep),
      m_variable(variable), m_physicFieldVariableComp(physicFieldVariableComp)
{
    FieldSolutionID fsid(fieldInfo->fieldId(), timeStep, adaptivityStep);
    m_ma = m_computation->solutionStore()->multiArray(fsid);

    m_variableHash = qHash(m_variable);
    /*
    {{#SPECIAL_FUNCTION_SOURCE}}
    if(m_fieldInfo->functionUsedInAnalysis("{{SPECIAL_FUNCTION_ID}}"))
    {{SPECIAL_FUNCTION_NAME}} = QSharedPointer<{{SPECIAL_EXT_FUNCTION_FULL_NAME}}>(new {{SPECIAL_EXT_FUNCTION_FULL_NAME}}(m_fieldInfo, 0));
    {{/SPECIAL_FUNCTION_SOURCE}}
    */
    m_coordinateType = m_computation->config()->coordinateType();
    m_labels = m_computation->scene()->labels;
    m_noneMarker = m_computation->scene()->materials->getNone(m_fieldInfo);
}

{{CLASS}}ViewScalarFilter::~{{CLASS}}ViewScalarFilter()
{
}

// only one component
void {{CLASS}}ViewScalarFilter::compute_derived_quantities_scalar (const std::vector<double> &uh,
                                                                   const std::vector<dealii::Tensor<1,2> > &duh,
                                                                   const std::vector<dealii::Tensor<2,2> > &dduh,
                                                                   const std::vector<dealii::Point<2> > &normals,
                                                                   const std::vector<dealii::Point<2> > &evaluation_points,
                                                                   const dealii::types::material_id mat_id,
                                                                   std::vector<dealii::Vector<double> > &computed_quantities) const
{
    int numberOfSolutions = m_fieldInfo->numberOfSolutions();
    double frequency = m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number();

    // find marker
    SceneLabel *label = m_labels->at(mat_id - 1);
    SceneMaterial *material = label->marker(m_fieldInfo);
    if(material == m_noneMarker)
        return;

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}

    std::vector<dealii::Vector<double> > solution_values(computed_quantities.size(), dealii::Vector<double>(numberOfSolutions));
    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(computed_quantities.size(), std::vector<dealii::Tensor<1,2> >(numberOfSolutions));
    std::vector<std::vector<dealii::Tensor<2,2> > >  solution_hessian(computed_quantities.size(), std::vector<dealii::Tensor<2,2> >(numberOfSolutions));

    for (unsigned int k = 0; k < computed_quantities.size(); k++)
    {
        dealii::Point<2> p = evaluation_points[k];

        solution_values[k][0] = uh[k];
        solution_grads[k][0] = duh[k];
        solution_hessian[k][0] = dduh[k];

        {{#VARIABLE_SOURCE}}
        if ((m_variableHash == {{VARIABLE_HASH}})
                && (m_coordinateType == {{COORDINATE_TYPE}})
                && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                && (m_physicFieldVariableComp == {{PHYSICFIELDVARIABLECOMP_TYPE}}))
            computed_quantities[k](0) = {{EXPRESSION}};
        {{/VARIABLE_SOURCE}}
    }
}

// multiple components
void {{CLASS}}ViewScalarFilter::compute_derived_quantities_vector (const std::vector<dealii::Vector<double> > &uh,
                                                                   const std::vector<std::vector<dealii::Tensor<1,2> > > &duh,
                                                                   const std::vector<std::vector<dealii::Tensor<2,2> > > &dduh,
                                                                   const std::vector<dealii::Point<2> > &normals,
                                                                   const std::vector<dealii::Point<2> > &evaluation_points,
                                                                   const dealii::types::material_id mat_id,
                                                                   std::vector<dealii::Vector<double> > &computed_quantities) const
{
    int numberOfSolutions = m_fieldInfo->numberOfSolutions();
    double frequency = m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number();

    // find marker
    // SceneLabel *label = m_labels->at(current_cell.first->material_id() - 1);
    SceneLabel *label = m_labels->at(mat_id - 1);
    SceneMaterial *material = label->marker(m_fieldInfo);
    if(material == m_computation->scene()->materials->getNone(m_fieldInfo))
        return;

    {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
    {{/VARIABLE_MATERIAL}}

    std::vector<dealii::Vector<double> > solution_values(computed_quantities.size(), dealii::Vector<double>(numberOfSolutions));
    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(computed_quantities.size(), std::vector<dealii::Tensor<1,2> >(numberOfSolutions));
    std::vector<std::vector<dealii::Tensor<2,2> > >  solution_hessian(computed_quantities.size(), std::vector<dealii::Tensor<2,2> >(numberOfSolutions));

    for (unsigned int k = 0; k < computed_quantities.size(); k++)
    {
        dealii::Point<2> p = evaluation_points[k];

        for (int i = 0; i < numberOfSolutions; i++)
        {
            solution_values[k][i] = uh[k][i];
            solution_grads[k][i] = duh[k][i];
            solution_hessian[k][i] = dduh[k][i];
        }

        {{#VARIABLE_MATERIAL}}const Value *material_{{MATERIAL_VARIABLE}} = material->valueNakedPtr(QLatin1String("{{MATERIAL_VARIABLE}}"));
        {{/VARIABLE_MATERIAL}}
        {{#VARIABLE_SOURCE}}
        if ((m_variableHash == {{VARIABLE_HASH}})
                && (m_coordinateType == {{COORDINATE_TYPE}})
                && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                && (m_physicFieldVariableComp == {{PHYSICFIELDVARIABLECOMP_TYPE}}))
            computed_quantities[k](0) = {{EXPRESSION}};
        {{/VARIABLE_SOURCE}}
    }
}

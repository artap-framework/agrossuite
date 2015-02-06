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

#include "{{ID}}_weakform.h"
// #include "{{ID}}_extfunction.h"

#include "util.h"
#include "util/global.h"

#include "scene.h"
#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/bdf2.h"

#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/base/quadrature_lib.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>

#include <deal.II/base/work_stream.h>
#include <deal.II/base/multithread_info.h>

{{#EXACT_SOURCE}}
template <int dim>
class Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}} : public dealii::Function<dim>
{
public:
    Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}}(SceneBoundary *boundary)
    : dealii::Function<dim>({{NUM_SOLUTIONS}})
    {
        {{#VARIABLE_SOURCE_LINEAR}}
        {{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE_LINEAR}}
        {{#VARIABLE_SOURCE_NONLINEAR}}
        {{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE_NONLINEAR}}
    }

    virtual ~Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}}() {}

virtual double value (const dealii::Point<dim> &p,
                      const unsigned int component) const
{
    {{#VARIABLE_SOURCE_LINEAR}}
    const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}
    {{#VARIABLE_SOURCE_NONLINEAR}}
    const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

    {{#FORM_EXPRESSION_ESSENTIAL}}
    // {{EXPRESSION_ID}}
    if (component == {{ROW_INDEX}})
        return {{EXPRESSION}}; {{/FORM_EXPRESSION_ESSENTIAL}}

    assert(0);
    return 0.0;
}

virtual void vector_value (const dealii::Point<dim> &p,
                           dealii::Vector<double> &values) const
{    
    {{#VARIABLE_SOURCE_LINEAR}}
    const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}
    {{#VARIABLE_SOURCE_NONLINEAR}}
    const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

    {{#FORM_EXPRESSION_ESSENTIAL}}
    // {{EXPRESSION_ID}}
    values[{{ROW_INDEX}}] = {{EXPRESSION}};{{/FORM_EXPRESSION_ESSENTIAL}}
}

virtual void value_list (const std::vector<dealii::Point<dim> > &points,
                         std::vector<double> &values,
                         const unsigned int component = 0) const
{
    for (unsigned int i = 0; i < points.size(); ++i)
    {
        dealii::Point<2> p = points[i];

        {{#VARIABLE_SOURCE_LINEAR}}
        const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}
        {{#VARIABLE_SOURCE_NONLINEAR}}
        const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

        {{#FORM_EXPRESSION_ESSENTIAL}}
        // {{EXPRESSION_ID}}
        if (component == {{ROW_INDEX}})
            values[i] = {{EXPRESSION}};
        {{/FORM_EXPRESSION_ESSENTIAL}}
    }
}

virtual void vector_value_list (const std::vector<dealii::Point<dim> > &points,
                                std::vector<dealii::Vector<double> > &values) const
{
    for (unsigned int i = 0; i < points.size(); ++i)
        vector_value(points[i], values[i]);
}

private:
{{#VARIABLE_SOURCE_LINEAR}}
const Value *{{VARIABLE_SHORT}};{{/VARIABLE_SOURCE_LINEAR}}
{{#VARIABLE_SOURCE_NONLINEAR}}
const Value *{{VARIABLE_SHORT}};{{/VARIABLE_SOURCE_NONLINEAR}}
};
{{/EXACT_SOURCE}}

// *************************************************************************************************************************************************

SolverDeal{{CLASS}}::AssemblyScratchData::AssemblyScratchData(const dealii::hp::FECollection<2> &feCollection,
                                                              dealii::hp::QCollection<2> quadratureFormulas,
                                                              dealii::hp::QCollection<2-1> faceQuadratureFormulas)
    :
      hp_fe_values(feCollection, quadratureFormulas,
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(feCollection, faceQuadratureFormulas,
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
{}


SolverDeal{{CLASS}}::AssemblyScratchData::AssemblyScratchData(const AssemblyScratchData &scratch_data)
    :
      hp_fe_values(scratch_data.hp_fe_values.get_fe_collection(), scratch_data.hp_fe_values.get_quadrature_collection(),
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(scratch_data.hp_fe_face_values.get_fe_collection(), scratch_data.hp_fe_face_values.get_quadrature_collection(),
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
{}

SolverDeal{{CLASS}}::AssemblyCopyData::AssemblyCopyData(const dealii::hp::FECollection<2> &feCollection,
                                                        dealii::hp::QCollection<2> quadratureFormulas,
                                                        dealii::hp::QCollection<2-1> faceQuadratureFormulas,
                                                        const FieldInfo *fieldInfo)
    : isAssembled(false)
{
    /*
    // cache
    int max_dofs_per_cell = feCollection.max_dofs_per_cell();
    int max_n_quadrature_points = quadratureFormulas.max_n_quadrature_points();

    // volume value and grad cache
    shape_value = std::vector<dealii::Vector<double> >(max_dofs_per_cell, dealii::Vector<double>(max_n_quadrature_points));
    shape_grad = std::vector<std::vector<dealii::Tensor<1,2> > >(max_dofs_per_cell, std::vector<dealii::Tensor<1,2> >(max_n_quadrature_points));
    // surface cache
    shape_face_point = std::vector<std::vector<dealii::Point<2> > >(dealii::GeometryInfo<2>::faces_per_cell);
    shape_face_value = std::vector<std::vector<dealii::Vector<double> > >(dealii::GeometryInfo<2>::faces_per_cell, std::vector<dealii::Vector<double> >(max_dofs_per_cell));
    shape_face_JxW = std::vector<std::vector<double> >(dealii::GeometryInfo<2>::faces_per_cell);
    // std::vector<std::vector<dealii::Tensor<1,2> > > shape_face_grad(max_dofs_per_cell);

    // previous values and grads
    solution_value_previous = std::vector<dealii::Vector<double> >(max_n_quadrature_points, dealii::Vector<double>(fieldInfo->numberOfSolutions()));
    solution_grad_previous = std::vector<std::vector<dealii::Tensor<1,2> > >(max_n_quadrature_points, std::vector<dealii::Tensor<1,2> >(fieldInfo->numberOfSolutions()));
    */
}

void SolverDeal{{CLASS}}::assembleSystem()
{
    bool isTransient = (m_fieldInfo->hasTransientAnalysis() && m_fieldInfo->value(FieldInfo::TransientAnalysis).toBool());

    system_rhs = 0.0;
    if(m_assemble_matrix)
        system_matrix = 0.0;

    // transient
    if (isTransient)
        mass_matrix = 0.0;

    dealii::WorkStream::run(m_doFHandler->begin_active(),
                            m_doFHandler->end(),
                            *this,
                            &SolverDeal{{CLASS}}::localAssembleSystem,
                            &SolverDeal{{CLASS}}::copyLocalToGlobal,
                            AssemblyScratchData(*m_feCollection, m_quadrature_formulas, m_face_quadrature_formulas),
                            AssemblyCopyData(*m_feCollection, m_quadrature_formulas, m_face_quadrature_formulas, m_fieldInfo));

    if (isTransient)
        mass_matrix_inverse.initialize(mass_matrix);
}

void SolverDeal{{CLASS}}::localAssembleSystem(const typename dealii::hp::DoFHandler<2>::active_cell_iterator &cell,
                                              AssemblyScratchData &scratch_data,
                                              AssemblyCopyData &copy_data)
{
    CoordinateType coordinateType = m_problem->config()->coordinateType();
    bool isTransient = (m_fieldInfo->hasTransientAnalysis() && m_fieldInfo->value(FieldInfo::TransientAnalysis).toBool());

    // reinit volume
    scratch_data.hp_fe_values.reinit(cell);

    // materials
    SceneMaterial *material = m_scene->labels->at(cell->material_id() - 1)->marker(m_fieldInfo);

    if (material != m_scene->materials->getNone(m_fieldInfo))
    {
        // TODO: move outside
        // coupling sources{{#COUPLING_SOURCE}}
        dealii::hp::DoFHandler<2>::active_cell_iterator cell_{{COUPLING_SOURCE_ID}}, endc_{{COUPLING_SOURCE_ID}};
        const SolverDeal* {{COUPLING_SOURCE_ID}}_solver = ProblemSolver::solvers()["{{COUPLING_SOURCE_ID}}"];
        if(m_problem->hasField("{{COUPLING_SOURCE_ID}}"))
        {
            cell_{{COUPLING_SOURCE_ID}} = ProblemSolver::solvers()["{{COUPLING_SOURCE_ID}}"]->doFHandler()->begin_active();
            endc_{{COUPLING_SOURCE_ID}} = ProblemSolver::solvers()["{{COUPLING_SOURCE_ID}}"]->doFHandler()->end();
        }
        {{/COUPLING_SOURCE}}

        const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;

        // local matrix
        copy_data.cell_matrix.reinit(dofs_per_cell, dofs_per_cell);
        copy_data.cell_matrix = 0;
        copy_data.cell_rhs.reinit(dofs_per_cell);
        copy_data.cell_rhs = 0;
        if (isTransient)
        {
            copy_data.cell_mass_matrix.reinit(dofs_per_cell, dofs_per_cell);
            copy_data.cell_mass_matrix = 0;
        }

        scratch_data.hp_fe_values.reinit(cell);

        const dealii::FEValues<2> &fe_values = scratch_data.hp_fe_values.get_present_fe_values();
        const unsigned int n_q_points = fe_values.n_quadrature_points;

        // components cache
        std::vector<int> components(dofs_per_cell);

        // volume value and grad cache
        std::vector<dealii::Vector<double> > shape_value(dofs_per_cell, dealii::Vector<double>(n_q_points));
        std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad(dofs_per_cell, std::vector<dealii::Tensor<1,2> >(n_q_points));

        // surface cache
        std::vector<std::vector<dealii::Point<2> > > shape_face_point(dealii::GeometryInfo<2>::faces_per_cell);
        std::vector<std::vector<dealii::Vector<double> > > shape_face_value(dealii::GeometryInfo<2>::faces_per_cell, std::vector<dealii::Vector<double> >(dofs_per_cell));
        std::vector<std::vector<double> > shape_face_JxW(dealii::GeometryInfo<2>::faces_per_cell);
        // std::vector<std::vector<dealii::Tensor<1,2> > > shape_face_grad(dofs_per_cell);

        // previous values and grads
        std::vector<dealii::Vector<double> > solution_value_previous;
        std::vector<std::vector<dealii::Tensor<1,2> > > solution_grad_previous;

        if (m_solution_previous)
        {
            solution_value_previous = std::vector<dealii::Vector<double> > (n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
            solution_grad_previous = std::vector<std::vector<dealii::Tensor<1,2> > >(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

            fe_values.get_function_values(*m_solution_previous, solution_value_previous);
            fe_values.get_function_gradients(*m_solution_previous, solution_grad_previous);
        }

        // coupling sources{{#COUPLING_SOURCE}}
        FieldInfo* {{COUPLING_SOURCE_ID}}_fieldInfo = nullptr;
        std::vector<dealii::Vector<double> > {{COUPLING_SOURCE_ID}}_value(n_q_points, dealii::Vector<double>(1)); // todo: num source solutions
        std::vector<std::vector<dealii::Tensor<1,2> > > {{COUPLING_SOURCE_ID}}_grad(n_q_points, std::vector<dealii::Tensor<1,2> >(1));// todo: num source solutions

        if(m_problem->hasField("{{COUPLING_SOURCE_ID}}"))
        {
            {{COUPLING_SOURCE_ID}}_fieldInfo = Agros2D::problem()->fieldInfo("{{COUPLING_SOURCE_ID}}");
            // todo: we probably do not need to initialize everything
            dealii::hp::FEValues<2> {{COUPLING_SOURCE_ID}}_hp_fe_values(*{{COUPLING_SOURCE_ID}}_solver->feCollection(), {{COUPLING_SOURCE_ID}}_solver->quadrature_formulas(), dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);
            {{COUPLING_SOURCE_ID}}_hp_fe_values.reinit(cell_{{COUPLING_SOURCE_ID}});
            const dealii::FEValues<2> &{{COUPLING_SOURCE_ID}}_fe_values = {{COUPLING_SOURCE_ID}}_hp_fe_values.get_present_fe_values();
            {{COUPLING_SOURCE_ID}}_fe_values.get_function_values(*m_coupling_sources["{{COUPLING_SOURCE_ID}}"], {{COUPLING_SOURCE_ID}}_value);
            {{COUPLING_SOURCE_ID}}_fe_values.get_function_gradients(*m_coupling_sources["{{COUPLING_SOURCE_ID}}"], {{COUPLING_SOURCE_ID}}_grad);
        }
        {{/COUPLING_SOURCE}}

        // cache volume
        for (unsigned int i = 0; i < dofs_per_cell; ++i)
        {
            components[i] = cell->get_fe().system_to_component_index(i).first;

            for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
            {
                shape_value[i][q_point] = fe_values.shape_value(i, q_point);
                shape_grad[i][q_point] = fe_values.shape_grad(i, q_point);
            }
        }

        // cache surface
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if (cell->face(face)->at_boundary())
            {
                scratch_data.hp_fe_face_values.reinit(cell, face);

                const dealii::FEFaceValues<2> &fe_face_values = scratch_data.hp_fe_face_values.get_present_fe_values();
                const unsigned int n_face_q_points = fe_face_values.n_quadrature_points;

                shape_face_point[face].resize(n_face_q_points);
                shape_face_JxW[face].resize(n_face_q_points);

                for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                {
                    shape_face_point[face][q_point] = fe_face_values.quadrature_point(q_point);
                    shape_face_JxW[face][q_point] = fe_face_values.JxW(q_point);
                }

                for (unsigned int i = 0; i < dofs_per_cell; ++i)
                {
                    shape_face_value[face][i].reinit(n_face_q_points);
                    for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                    {
                        shape_face_value[face][i][q_point] = fe_face_values.shape_value(i, q_point);
                        // shape_face_grad[i][q_point] = fe_face_values.shape_grad(i, q_point);
                    }
                }
            }
        }

        const QMap<uint, QSharedPointer<Value> > materialValues = material->values();
        {{#VOLUME_SOURCE}}
        if ((coordinateType == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
        {
            // matrix
            {{#VARIABLE_SOURCE_LINEAR}}
            // {{VARIABLE}}
            const double {{VARIABLE_SHORT}}_val = materialValues[{{VARIABLE_HASH}}]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}
            {{#FUNCTION_SOURCE_CONSTANT}}
            const double {{FUNCTION_SHORT}} = {{FUNCTION_EXPRESSION}}; {{/FUNCTION_SOURCE_CONSTANT}}

            for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
            {
                const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                {{#VARIABLE_SOURCE_NONLINEAR}}
                // {{VARIABLE}}
                const double {{VARIABLE_SHORT}}_val = materialValues[{{VARIABLE_HASH}}]->{{VARIABLE_VALUE}};
                const double {{VARIABLE_SHORT}}_der = materialValues[{{VARIABLE_HASH}}]->{{VARIABLE_DERIVATIVE}}; {{/VARIABLE_SOURCE_NONLINEAR}}
                {{#FUNCTION_SOURCE_NONCONSTANT}}
                const double {{FUNCTION_SHORT}} = {{FUNCTION_EXPRESSION}}; {{/FUNCTION_SOURCE_NONCONSTANT}}

                for (unsigned int i = 0; i < dofs_per_cell; ++i)
                {
                    if(m_assemble_matrix)
                    {
                        for (unsigned int j = 0; j < dofs_per_cell; ++j)
                        {
                            {{#FORM_EXPRESSION_MATRIX}}
                            // {{EXPRESSION_ID}}
                            if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && {{EXPRESSION_CHECK}})
                            {
                                copy_data.cell_matrix(i,j) += fe_values.JxW(q_point) *({{EXPRESSION}});
                            }{{/FORM_EXPRESSION_MATRIX}}

                            if (isTransient)
                            {
                                {{#FORM_EXPRESSION_TRANSIENT}}
                                // {{EXPRESSION_ID}}
                                if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && {{EXPRESSION_CHECK}})
                                {
                                    copy_data.cell_mass_matrix(i,j) += fe_values.JxW(q_point) *({{EXPRESSION}});
                                }{{/FORM_EXPRESSION_TRANSIENT}}
                            }
                        }

                        // symetrical forms
                        for (unsigned int j = i; j < dofs_per_cell; ++j)
                        {
                            {{#FORM_EXPRESSION_MATRIX_SYM}}
                            // {{EXPRESSION_ID}}
                            if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && {{EXPRESSION_CHECK}})
                            {
                                double expression_value = fe_values.JxW(q_point) *({{EXPRESSION}});
                                copy_data.cell_matrix(i,j) += expression_value;
                                if(j != i)
                                    copy_data.cell_matrix(j,i) += expression_value;
                            }{{/FORM_EXPRESSION_MATRIX_SYM}}

                            if (isTransient)
                            {
                                {{#FORM_EXPRESSION_TRANSIENT_SYM}}
                                // {{EXPRESSION_ID}}
                                if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && {{EXPRESSION_CHECK}})
                                {
                                    double expression_value = fe_values.JxW(q_point) *({{EXPRESSION}});
                                    copy_data.cell_mass_matrix(i,j) += expression_value;
                                    if(j != i)
                                        copy_data.cell_mass_matrix(j,i) += expression_value;
                                }{{/FORM_EXPRESSION_TRANSIENT_SYM}}
                            }
                        }
                    }
                    {{#FORM_EXPRESSION_VECTOR}}
                    // {{EXPRESSION_ID}}
                    if (components[i] == {{ROW_INDEX}})
                    {
                        copy_data.cell_rhs(i) += fe_values.JxW(q_point) *({{EXPRESSION}});
                    }{{/FORM_EXPRESSION_VECTOR}}
                    {{#COUPLING_SOURCE}}
                    if({{COUPLING_SOURCE_ID}}_fieldInfo)
                    { {{#FORM_EXPRESSION_VECTOR}}
                        // {{EXPRESSION_ID}}
                        if (components[i] == {{ROW_INDEX}})
                        {
                            copy_data.cell_rhs(i) += fe_values.JxW(q_point) *({{EXPRESSION}});
                        }{{/FORM_EXPRESSION_VECTOR}}
                    }{{/COUPLING_SOURCE}}
                }
            }
        }
        {{/VOLUME_SOURCE}}


        // boundaries
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if (cell->face(face)->at_boundary())
            {
                SceneBoundary *boundary = m_scene->edges->at(cell->face(face)->boundary_indicator() - 1)->marker(m_fieldInfo);
                const QMap<uint, QSharedPointer<Value> > boundaryValues = boundary->values();

                if (boundary != m_scene->boundaries->getNone(m_fieldInfo))
                {
                    {{#SURFACE_SOURCE}}
                    // {{BOUNDARY_ID}}
                    if ((coordinateType == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                            && boundary->type() == "{{BOUNDARY_ID}}")
                    {
                        {{#VARIABLE_SOURCE_LINEAR}}
                        // {{VARIABLE}}
                        const double {{VARIABLE_SHORT}}_val = boundaryValues[{{VARIABLE_HASH}}]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}

                        // value and grad cache
                        std::vector<dealii::Vector<double> > shape_value = shape_face_value[face];
                        // std::vector<std::vector<dealii::Tensor<1,2> > > shape_grad = shape_face_grad;

                        const dealii::FEFaceValues<2> &fe_face_values = scratch_data.hp_fe_face_values.get_present_fe_values();
                        for (unsigned int q_point = 0; q_point < fe_face_values.n_quadrature_points; ++q_point)
                        {
                            const dealii::Point<2> p = shape_face_point[face][q_point];

                            {{#VARIABLE_SOURCE_NONLINEAR}}
                            // {{VARIABLE}}
                            const double {{VARIABLE_SHORT}}_val = boundaryValues[{{VARIABLE_HASH}}]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

                            for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            {
                                if(m_assemble_matrix)
                                {
                                    for (unsigned int j = 0; j < dofs_per_cell; ++j)
                                    {
                                        {{#FORM_EXPRESSION_MATRIX}}
                                        // {{EXPRESSION_ID}}
                                        if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}})
                                        {
                                            copy_data.cell_matrix(i,j) += shape_face_JxW[face][q_point] *({{EXPRESSION}});
                                        }{{/FORM_EXPRESSION_MATRIX}}
                                    }
                                }
                                {{#FORM_EXPRESSION_VECTOR}}
                                // {{EXPRESSION_ID}}
                                if (components[i] == {{ROW_INDEX}})
                                {
                                    copy_data.cell_rhs(i) += shape_face_JxW[face][q_point] *({{EXPRESSION}});
                                }{{/FORM_EXPRESSION_VECTOR}}
                            }
                        }
                    }
                    {{/SURFACE_SOURCE}}

                }
            }
        }

        // todo: different domains
        // coupling sources{{#COUPLING_SOURCE}}
        if(m_problem->hasField("{{COUPLING_SOURCE_ID}}"))
        {
            ++cell_{{COUPLING_SOURCE_ID}};
        }
        {{/COUPLING_SOURCE}}

        // distribute local to global matrix
        copy_data.local_dof_indices.resize(dofs_per_cell);
        cell->get_dof_indices(copy_data.local_dof_indices);

        copy_data.isAssembled = true;
    }
}

void SolverDeal{{CLASS}}::assembleDirichlet(bool useDirichletLift)
{
    CoordinateType coordinateType = m_problem->config()->coordinateType();

    for (int i = 0; i < m_scene->edges->count(); i++)
    {
        SceneEdge *edge = m_scene->edges->at(i);
        SceneBoundary *boundary = edge->marker(m_fieldInfo);

        if (boundary && (!boundary->isNone()))
        {
            {{#EXACT_SOURCE}}
            // {{BOUNDARY_ID}}
            if ((coordinateType == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                    && boundary->type() == "{{BOUNDARY_ID}}")
            {
                // component mask
                std::vector<bool> mask;
                {{#FORM_EXPRESSION_MASK}}
                mask.push_back({{MASK}});{{/FORM_EXPRESSION_MASK}}
                dealii::ComponentMask component_mask(mask);

                if(useDirichletLift)
                    dealii::VectorTools::interpolate_boundary_values (*m_doFHandler, i+1,
                                                                      Essential_{{COORDINATE_TYPE}}_{{ANALYSIS_TYPE}}_{{LINEARITY_TYPE}}_{{BOUNDARY_ID}}<2>(boundary),
                                                                      hanging_node_constraints, // boundary_values,
                                                                      component_mask);
                else
                    dealii::VectorTools::interpolate_boundary_values (*m_doFHandler, i+1,
                                                                      dealii::ZeroFunction<2>({{NUM_SOLUTIONS}}), // for the Newton method
                                                                      hanging_node_constraints, // boundary_values,
                                                                      component_mask);

                // dealii::MatrixTools::apply_boundary_values (boundary_values, system_matrix, *m_solution, system_rhs);
            }
            {{/EXACT_SOURCE}}
        }
    }
}


void SolverDeal{{CLASS}}::copyLocalToGlobal(const AssemblyCopyData &copy_data)
{
    if (copy_data.isAssembled)
    {
        // distribute local to global system
        if(m_assemble_matrix)
        {
            hanging_node_constraints.distribute_local_to_global(copy_data.cell_matrix,
                                                                copy_data.cell_rhs,
                                                                copy_data.local_dof_indices,
                                                                system_matrix,
                                                                system_rhs);
        }
        else
        {
            hanging_node_constraints.distribute_local_to_global(copy_data.cell_rhs,
                                                                copy_data.local_dof_indices,
                                                                system_rhs);
        }

        if (m_fieldInfo->hasTransientAnalysis() && m_fieldInfo->value(FieldInfo::TransientAnalysis).toBool())
        {
            // distribute local to global system
            hanging_node_constraints.distribute_local_to_global(copy_data.cell_mass_matrix,
                                                                copy_data.local_dof_indices,
                                                                mass_matrix);
        }
    }
}

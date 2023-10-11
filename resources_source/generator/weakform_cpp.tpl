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

#include "util/global.h"

#include "scene.h"

#include "solver/field.h"
#include "solver/form_info.h"
#include "solver/solver.h"
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

{{#MACRO}}#define {{MACRO_ID}} {{MACRO_EXPRESSION}}
{{/MACRO}}

void SolverDeal{{CLASS}}::Assemble{{CLASS}}::assembleSystem(const dealii::Vector<double> &solutionNonlinearPrevious,
                                                            bool assembleMatrix,
                                                            bool assembleRHS)
{
    bool isTransient = (m_fieldInfo->analysisType() == AnalysisType_Transient);

    systemRHS = 0.0;
    if (assembleMatrix)
        systemMatrix = 0.0;

    // transient solver
    if (isTransient)
        transientMassMatrix = 0.0;

    // dealii::SynchronousIterators<IteratorTuple> cell_and_source_begin(IteratorTuple(doFHandler.begin_active(),
    //                                                                                 doFHandler.begin_active()));
    // dealii::SynchronousIterators<IteratorTuple> cell_and_source_end(IteratorTuple(doFHandler.end(),
    //                                                                               doFHandler.end()));

    // m_solverDeal->clearCache();

    TYPENAME dealii::DoFHandler<2>::active_cell_iterator cell_begin, cell_end, source_begin, source_end;
    cell_begin = doFHandler.begin_active();
    cell_end = doFHandler.end();

    // if there is no source, we use the same as dummy
    source_begin = doFHandler.begin_active();
    source_end = doFHandler.end();

    // coupling sources{{#COUPLING_SOURCE}}
    if (m_computation->hasField("{{COUPLING_SOURCE_ID}}"))
    {
        source_begin = m_solverDeal->couplingSource("{{COUPLING_SOURCE_ID}}").doFHandler().begin_active();
        source_end = m_solverDeal->couplingSource("{{COUPLING_SOURCE_ID}}").doFHandler().end();

        // cell_and_source_begin = dealii::SynchronousIterators<IteratorTuple>(IteratorTuple(doFHandler.begin_active(),
        //                                                                                   m_solverDeal->couplingSource("{{COUPLING_SOURCE_ID}}").doFHandler()->begin_active()));
        // cell_and_source_end = dealii::SynchronousIterators<IteratorTuple>(IteratorTuple(doFHandler.end(),
        //                                                                                 m_solverDeal->couplingSource("{{COUPLING_SOURCE_ID}}").doFHandler()->end()));
    }
    {{/COUPLING_SOURCE}}

    // Fix the beginning cells wrt. subdomains.
    while (cell_begin != doFHandler.end())
    {
        if (!m_computation->scene()->labels->at(cell_begin->material_id() - 1)->marker(m_fieldInfo)->isNone())
            break;
        else
        {
            ++cell_begin;
            ++source_begin;
        }
    }

    // while (cell_and_source_begin != cell_and_source_end)
    // {
    //     if (!Agros::problem()->scene()->labels->at(std::get<0>(cell_and_source_begin.iterators)->material_id() - 1)->marker(m_fieldInfo)->isNone())
    //         break;
    //     else
    //         ++cell_and_source_begin;
    // }
    //
    //     dealii::WorkStream::run(cell_and_source_begin,
    //                             cell_and_source_end,

    dealii::WorkStream::run(DoubleCellIterator(source_begin, cell_begin, doFHandler, m_computation, m_fieldInfo),
                            DoubleCellIterator(source_end, cell_end, doFHandler, m_computation, m_fieldInfo),
                            *this,
                            &SolverDeal{{CLASS}}::Assemble{{CLASS}}::localAssembleSystem,
                            &SolverDeal{{CLASS}}::Assemble{{CLASS}}::copyLocalToGlobal,
                            AssemblyScratchData{{CLASS}}(m_computation,
                                                         *m_computation->problemSolver()->feCollection(m_fieldInfo),
                                                         *m_computation->problemSolver()->mappingCollection(m_fieldInfo),
                                                         m_solverDeal->quadratureFormulas(),
                                                         m_solverDeal->quadratureFormulasFace(),
                                                         solutionNonlinearPrevious,
                                                         assembleMatrix,
                                                         assembleRHS),
                            AssemblyCopyData());
}

// void SolverDeal{{CLASS}}::Assemble{{CLASS}}::localAssembleSystem(const dealii::SynchronousIterators<IteratorTuple> &iter,
void SolverDeal{{CLASS}}::Assemble{{CLASS}}::localAssembleSystem(const DoubleCellIterator &iter,
                                                                 AssemblyScratchData{{CLASS}} &scratch_data,
                                                                 AssemblyCopyData &copy_data)
{
    double actualTime = m_solverDeal->get_time();
    double frequency = m_computation->config()->value(ProblemConfig::Frequency).value<Value>().number();

    const TYPENAME dealii::DoFHandler<2>::active_cell_iterator cell = iter.cell_second;
    // const TYPENAME dealii::DoFHandler<2>::active_cell_iterator cell = std::get<0>(iter.iterators);

    // coupling sources{{#COUPLING_SOURCE}}
    dealii::DoFHandler<2>::active_cell_iterator cell_{{COUPLING_SOURCE_ID}};
    if (m_computation->hasField("{{COUPLING_SOURCE_ID}}"))
    {
        cell_{{COUPLING_SOURCE_ID}} = iter.cell_first;
        // cell_{{COUPLING_SOURCE_ID}} = std::get<1>(iter.iterators);
    }
    {{/COUPLING_SOURCE}}

    CoordinateType coordinateType = m_computation->config()->coordinateType();
    bool isTransient = (m_fieldInfo->analysisType() == AnalysisType_Transient);

    // materials
    SceneMaterial *material = m_computation->scene()->labels->at(cell->material_id() - 1)->marker(m_fieldInfo);

    if (material != m_computation->scene()->materials->getNone(m_fieldInfo))
    {
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

        // reinit volume
        scratch_data.hp_fe_values.reinit(cell);

        const dealii::FEValues<2> &fe_values = scratch_data.hp_fe_values.get_present_fe_values();
        const unsigned int n_q_points = fe_values.n_quadrature_points;

        // components cache
        std::vector<int> components(dofs_per_cell);

        // local cache
        SolverDeal::AssembleCache cache;

        // volume value and grad cache
        cache.shape_value = std::vector<std::vector<double> >(dofs_per_cell, std::vector<double>(n_q_points));
        cache.shape_grad = std::vector<std::vector<dealii::Tensor<1,2> > >(dofs_per_cell, std::vector<dealii::Tensor<1,2> >(n_q_points));

        // surface cache
        cache.shape_face_point = std::vector<std::vector<dealii::Point<2> > >(dealii::GeometryInfo<2>::faces_per_cell);
        cache.shape_face_value = std::vector<std::vector<std::vector<double> > >(dealii::GeometryInfo<2>::faces_per_cell, std::vector<std::vector<double> >(dofs_per_cell));
        cache.shape_face_JxW = std::vector<std::vector<double> >(dealii::GeometryInfo<2>::faces_per_cell);

        // previous values and grads
        cache.solution_value_previous = std::vector<dealii::Vector<double> >(n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
        cache.solution_grad_previous = std::vector<std::vector<dealii::Tensor<1,2> > >(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

        cache.dofs_per_cell = dofs_per_cell;
        cache.n_q_points = n_q_points;

        if (scratch_data.solutionNonlinearPrevious.size() > 0)
        {
            fe_values.get_function_values(scratch_data.solutionNonlinearPrevious, cache.solution_value_previous);
            fe_values.get_function_gradients(scratch_data.solutionNonlinearPrevious, cache.solution_grad_previous);
        }

        // coupling sources{{#COUPLING_SOURCE}}
        FieldInfo *{{COUPLING_SOURCE_ID}}_fieldInfo = nullptr;
        std::vector<dealii::Vector<double> > {{COUPLING_SOURCE_ID}}_value;
        std::vector<std::vector<dealii::Tensor<1,2> > > {{COUPLING_SOURCE_ID}}_grad;

        //{{COUPLING_SOURCE_ID}} variables{{#COUPLING_VARIABLES}}
        double {{VARIABLE_SHORT}} = 0.0;{{/COUPLING_VARIABLES}}

        SceneMaterial *{{COUPLING_SOURCE_ID}}_material = nullptr;
        if (m_computation->hasField("{{COUPLING_SOURCE_ID}}"))
        {
            {{COUPLING_SOURCE_ID}}_fieldInfo = m_computation->fieldInfo("{{COUPLING_SOURCE_ID}}");
            {{COUPLING_SOURCE_ID}}_material = m_computation->scene()->labels->at(cell->material_id() - 1)->marker({{COUPLING_SOURCE_ID}}_fieldInfo);

            scratch_data.{{COUPLING_SOURCE_ID}}_hp_fe_values->reinit(cell_{{COUPLING_SOURCE_ID}});
            const dealii::FEValues<2> &{{COUPLING_SOURCE_ID}}_fe_values = scratch_data.{{COUPLING_SOURCE_ID}}_hp_fe_values->get_present_fe_values();

            {{COUPLING_SOURCE_ID}}_value = std::vector<dealii::Vector<double> >({{COUPLING_SOURCE_ID}}_fe_values.n_quadrature_points, dealii::Vector<double>({{COUPLING_SOURCE_ID}}_fieldInfo->numberOfSolutions()));
            {{COUPLING_SOURCE_ID}}_grad = std::vector<std::vector<dealii::Tensor<1,2> > >({{COUPLING_SOURCE_ID}}_fe_values.n_quadrature_points, std::vector<dealii::Tensor<1,2> >({{COUPLING_SOURCE_ID}}_fieldInfo->numberOfSolutions()));

            {{COUPLING_SOURCE_ID}}_fe_values.get_function_values(m_solverDeal->couplingSource("{{COUPLING_SOURCE_ID}}").solution(), {{COUPLING_SOURCE_ID}}_value);
            {{COUPLING_SOURCE_ID}}_fe_values.get_function_gradients(m_solverDeal->couplingSource("{{COUPLING_SOURCE_ID}}").solution(), {{COUPLING_SOURCE_ID}}_grad);

            if ({{COUPLING_SOURCE_ID}}_material != m_computation->scene()->materials->getNone({{COUPLING_SOURCE_ID}}_fieldInfo))
            {
                const QMap<size_t, QSharedPointer<Value> > {{COUPLING_SOURCE_ID}}_materialValues = {{COUPLING_SOURCE_ID}}_material->values();
                //{{COUPLING_SOURCE_ID}} variables for individual source analysis types{{#COUPLING_VARIABLES_ANALYSIS_TYPE}}
                if({{COUPLING_SOURCE_ID}}_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                {{{#COUPLING_VARIABLES}}
                    // {{VARIABLE}}
                    {{VARIABLE_SHORT}} = {{COUPLING_SOURCE_ID}}_materialValues[{{VARIABLE_HASH}}u]->number(); {{/COUPLING_VARIABLES}}
                }{{/COUPLING_VARIABLES_ANALYSIS_TYPE}}
            }
        }
        {{/COUPLING_SOURCE}}

        // cache volume
        for (unsigned int i = 0; i < dofs_per_cell; ++i)
        {
            components[i] = cell->get_fe().system_to_component_index(i).first;

            for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
            {
                cache.shape_value[i][q_point] = fe_values.shape_value(i, q_point);
                cache.shape_grad[i][q_point] = fe_values.shape_grad(i, q_point);
            }
        }

        // cache surface
        cache.solution_value_previous_face.resize(dealii::GeometryInfo<2>::faces_per_cell);
        cache.solution_grad_previous_face.resize(dealii::GeometryInfo<2>::faces_per_cell);

        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if(cell->face(face)->user_index() > 0 )
            {
                SceneBoundary *boundary = m_computation->scene()->faces->at(cell->face(face)->user_index() - 1)->marker(m_fieldInfo);
                if (boundary != m_computation->scene()->boundaries->getNone(m_fieldInfo))
                {
                    scratch_data.hp_fe_face_values.reinit(cell, face);

                    const dealii::FEFaceValues<2> &fe_face_values = scratch_data.hp_fe_face_values.get_present_fe_values();
                    const unsigned int n_face_q_points = fe_face_values.n_quadrature_points;

                    cache.shape_face_point[face].resize(n_face_q_points);
                    cache.shape_face_JxW[face].resize(n_face_q_points);

                    for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                    {
                        cache.shape_face_point[face][q_point] = fe_face_values.quadrature_point(q_point);
                        cache.shape_face_JxW[face][q_point] = fe_face_values.JxW(q_point);
                    }
                    
                    for (unsigned int i = 0; i < dofs_per_cell; ++i)
                    {
                        cache.shape_face_value[face][i].resize(n_face_q_points);
                        for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                        {
                            cache.shape_face_value[face][i][q_point] = fe_face_values.shape_value(i, q_point);
                        }
                    }
                    
                    if (scratch_data.solutionNonlinearPrevious.size() > 0)
                    {
                        // previous values and grads
                        cache.solution_value_previous_face[face] = std::vector<dealii::Vector<double> >(n_face_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
                        cache.solution_grad_previous_face[face] = std::vector<std::vector<dealii::Tensor<1, 2> > >(n_face_q_points, std::vector<dealii::Tensor<1, 2> >(m_fieldInfo->numberOfSolutions()));

                        for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                        {
                            fe_face_values.get_function_values(scratch_data.solutionNonlinearPrevious, cache.solution_value_previous_face[face]);
                            // todo - throws some uninitialized error
                            //fe_face_values.get_function_gradients(scratch_data.solutionNonlinearPrevious, cache.solution_grad_previous_face[face]);
                        }
                    }
                }
            }
        }

        const QMap<size_t, QSharedPointer<Value> > materialValues = material->values();
        {{#VOLUME_SOURCE}}
        if ((coordinateType == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}}))
        {
            // matrix
            {{#VARIABLE_SOURCE_LINEAR}}
            // {{VARIABLE}}
            const double {{VARIABLE_SHORT}}_val = materialValues[{{VARIABLE_HASH}}u]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}
            {{#FUNCTION_SOURCE_CONSTANT}}
            const double {{FUNCTION_SHORT}} = {{FUNCTION_EXPRESSION}}; {{/FUNCTION_SOURCE_CONSTANT}}

            for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
            {
                const dealii::Point<2> p = fe_values.quadrature_point(q_point);
                {{#VARIABLE_SOURCE_NONLINEAR}}
                // {{VARIABLE}}
                const double {{VARIABLE_SHORT}}_val = materialValues[{{VARIABLE_HASH}}u]->{{VARIABLE_VALUE}};
                const double {{VARIABLE_SHORT}}_der = materialValues[{{VARIABLE_HASH}}u]->{{VARIABLE_DERIVATIVE}}; {{/VARIABLE_SOURCE_NONLINEAR}}
                {{#FUNCTION_SOURCE_NONCONSTANT}}
                const double {{FUNCTION_SHORT}} = {{FUNCTION_EXPRESSION}}; {{/FUNCTION_SOURCE_NONCONSTANT}}

                for (unsigned int i = 0; i < dofs_per_cell; ++i)
                {
                    if (scratch_data.assembleMatrix)
                    {
                        for (unsigned int j = 0; j < dofs_per_cell; ++j)
                        {
                            {{#FORM_EXPRESSION_MATRIX}}
                            // {{EXPRESSION_ID}}
                            if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && ({{EXPRESSION_CHECK}}))
                            {
                                copy_data.cell_matrix(i,j) += fe_values.JxW(q_point) *({{EXPRESSION}});
                            }{{/FORM_EXPRESSION_MATRIX}}

                            if (isTransient)
                            {
                                {{#FORM_EXPRESSION_TRANSIENT}}
                                // {{EXPRESSION_ID}}
                                if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && ({{EXPRESSION_CHECK}}))
                                {
                                    copy_data.cell_mass_matrix(i,j) += fe_values.JxW(q_point) *({{EXPRESSION}});
                                }{{/FORM_EXPRESSION_TRANSIENT}}
                            }
                        }

                        // symmetrical forms
                        for (unsigned int j = i; j < dofs_per_cell; ++j)
                        {
                            {{#FORM_EXPRESSION_MATRIX_SYM}}
                            // {{EXPRESSION_ID}}
                            if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && ({{EXPRESSION_CHECK}}))
                            {
                                double expression_value = fe_values.JxW(q_point) *({{EXPRESSION}});
                                copy_data.cell_matrix(i,j) += expression_value;
                                if (j != i)
                                    copy_data.cell_matrix(j,i) += expression_value;
                            }{{/FORM_EXPRESSION_MATRIX_SYM}}

                            if (isTransient)
                            {
                                {{#FORM_EXPRESSION_TRANSIENT_SYM}}
                                // {{EXPRESSION_ID}}
                                if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}} && ({{EXPRESSION_CHECK}}))
                                {
                                    double expression_value = fe_values.JxW(q_point) *({{EXPRESSION}});
                                    copy_data.cell_mass_matrix(i,j) += expression_value;
                                    if (j != i)
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
                    if({{COUPLING_SOURCE_ID}}_fieldInfo && ({{COUPLING_SOURCE_ID}}_material != m_computation->scene()->materials->getNone({{COUPLING_SOURCE_ID}}_fieldInfo)))
                    {{{#COUPLING_FORMS_ANALYSIS_TYPE}}
                        if({{COUPLING_SOURCE_ID}}_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                        {{{#FORM_EXPRESSION_COUPLING_VECTOR}}
                            // {{EXPRESSION_ID}}
                            if (components[i] == {{ROW_INDEX}})
                            {
                                copy_data.cell_rhs(i) += fe_values.JxW(q_point) *({{EXPRESSION}});
                            }{{/FORM_EXPRESSION_COUPLING_VECTOR}}
                        }{{/COUPLING_FORMS_ANALYSIS_TYPE}}
                    }{{/COUPLING_SOURCE}}
                }
            }
        }
        {{/VOLUME_SOURCE}}

        // boundaries
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if(cell->face(face)->user_index() > 0 )
            {
                SceneBoundary *boundary = m_computation->scene()->faces->at(cell->face(face)->user_index() - 1)->marker(m_fieldInfo);
                const QMap<size_t, QSharedPointer<Value> > boundaryValues = boundary->values();

                if (boundary != m_computation->scene()->boundaries->getNone(m_fieldInfo))
                {
                    {{#SURFACE_SOURCE}}
                    // {{BOUNDARY_ID}}
                    if ((coordinateType == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                            && boundary->type() == "{{BOUNDARY_ID}}")
                    {
                        {{#VARIABLE_SOURCE_LINEAR}}
                        // {{VARIABLE}}
                        const double {{VARIABLE_SHORT}}_val = boundaryValues[{{VARIABLE_HASH}}u]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}

                        scratch_data.hp_fe_face_values.reinit(cell, face);

                        const dealii::FEFaceValues<2> &fe_face_values = scratch_data.hp_fe_face_values.get_present_fe_values();
                        const unsigned int n_face_q_points = fe_face_values.n_quadrature_points;
                        
                        for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                        {
                            const dealii::Point<2> p = cache.shape_face_point[face][q_point];

                            {{#VARIABLE_SOURCE_NONLINEAR}}
                            // {{VARIABLE}}
                            const double {{VARIABLE_SHORT}}_val = boundaryValues[{{VARIABLE_HASH}}u]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

                            for (unsigned int i = 0; i < dofs_per_cell; ++i)
                            {
                                if(scratch_data.assembleMatrix)
                                {
                                    for (unsigned int j = 0; j < dofs_per_cell; ++j)
                                    {
                                        {{#FORM_EXPRESSION_MATRIX}}
                                        // {{EXPRESSION_ID}}
                                        if (components[i] == {{ROW_INDEX}} && components[j] == {{COLUMN_INDEX}})
                                        {
                                            copy_data.cell_matrix(i,j) += cache.shape_face_JxW[face][q_point] *({{EXPRESSION}});
                                        }{{/FORM_EXPRESSION_MATRIX}}
                                    }
                                }
                                {{#FORM_EXPRESSION_VECTOR}}
                                // {{EXPRESSION_ID}}
                                if (components[i] == {{ROW_INDEX}})
                                {
                                    copy_data.cell_rhs(i) += cache.shape_face_JxW[face][q_point] *({{EXPRESSION}});
                                }{{/FORM_EXPRESSION_VECTOR}}
                            }
                        }
                    }
                    {{/SURFACE_SOURCE}}

                }
            }
        }

        // distribute local to global matrix
        copy_data.local_dof_indices.resize(dofs_per_cell);
        cell->get_dof_indices(copy_data.local_dof_indices);

        copy_data.isAssembled = true;
    }
}

void SolverDeal{{CLASS}}::Assemble{{CLASS}}::assembleDirichlet(bool calculateDirichletLiftValue)
{
    double actualTime = m_solverDeal->get_time();
    CoordinateType coordinateType = m_computation->config()->coordinateType();

    // prepare QCollection
    dealii::hp::FECollection<2> feCollection(doFHandler.get_fe_collection());
    dealii::hp::QCollection<2-1> qCollection;
    for (unsigned int f = 0; f<feCollection.size(); ++f)
    {
        const dealii::FiniteElement<2> &fe = feCollection[f];

        if (fe.has_face_support_points())
            qCollection.push_back (dealii::Quadrature<2-1>(fe.get_unit_face_support_points()));
        else
        {
            std::vector<dealii::Point<2-1> > unit_support_points (fe.dofs_per_face);

            for (unsigned int i=0; i<fe.dofs_per_face; ++i)
                if (fe.is_primitive (fe.face_to_cell_index(i,0)))
                    // if (mask[fe.face_system_to_component_index(i).first] == true)
                    unit_support_points[i] = fe.unit_face_support_point(i);

            qCollection.push_back (dealii::Quadrature<2-1>(unit_support_points));
        }
    }

    // hp face values
    dealii::hp::FEFaceValues<2> hp_fe_face_values(*m_computation->problemSolver()->mappingCollection(m_fieldInfo),
                                                  feCollection, qCollection, dealii::update_quadrature_points);

    // dealii::hp::FEFaceValues<2> hp_fe_face_values(*m_solverDeal->mappingCollection(), m_solverDeal->feCollection(), m_solverDeal->quadratureFormulasFace(), dealii::update_values | dealii::update_quadrature_points);

    dealii::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
    for(; cell != endc; ++cell)
    {
        if (m_computation->scene()->labels->at(cell->material_id() - 1)->marker(m_fieldInfo) == m_computation->scene()->materials->getNone(m_fieldInfo))
            continue;
        
        // boundaries
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if (cell->face(face)->user_index() > 0)
            {
                SceneBoundary *boundary = m_computation->scene()->faces->at(cell->face(face)->user_index() - 1)->marker(m_fieldInfo);
                if (boundary != m_computation->scene()->boundaries->getNone(m_fieldInfo))
                {
                    const dealii::FiniteElement<2> &fe = cell->get_fe();

                    hp_fe_face_values.reinit(cell, face);
                    const dealii::FEFaceValues<2> &fe_values = hp_fe_face_values.get_present_fe_values();
                    std::vector<dealii::Point<2> > points;
                    points.reserve(doFHandler.get_fe_collection().max_dofs_per_face());
                    points = fe_values.get_quadrature_points();

                    const unsigned int dofs_per_face = fe.dofs_per_face;
                    std::vector<dealii::types::global_dof_index> local_face_dof_indices(dofs_per_face);

                    cell->face(face)->get_dof_indices(local_face_dof_indices, cell->active_fe_index());

                    {{#EXACT_SOURCE}}
                    // {{BOUNDARY_ID}}
                    if ((coordinateType == {{COORDINATE_TYPE}}) && (m_fieldInfo->analysisType() == {{ANALYSIS_TYPE}}) && (m_fieldInfo->linearityType() == {{LINEARITY_TYPE}})
                            && boundary->type() == "{{BOUNDARY_ID}}")
                    {
                        {{#VARIABLE_SOURCE_LINEAR}}
                        const Value *{{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE_LINEAR}}
                        {{#VARIABLE_SOURCE_NONLINEAR}}
                        const Value *{{VARIABLE_SHORT}} = boundary->valueNakedPtr("{{VARIABLE}}"); {{/VARIABLE_SOURCE_NONLINEAR}}

                        // component mask
                        std::vector<bool> mask;
                        {{#FORM_EXPRESSION_MASK}}
                        mask.push_back({{MASK}});{{/FORM_EXPRESSION_MASK}}

                        for (unsigned int i = 0; i < dofs_per_face; i++)
                        {
                            for (unsigned int comp = 0; comp < {{NUM_SOLUTIONS}}; comp++)
                            {
                                if (mask[comp])
                                {
                                    if (cell->get_fe().face_system_to_component_index(i).first == comp)
                                    {
                                        if (calculateDirichletLiftValue)
                                        {
                                            // Dirichlet lift (linear solution, Picard's method)
                                            constraintsDirichlet.add_line(local_face_dof_indices[i]);

                                            dealii::Point<2> p = points[i];
                                            
                                            {{#VARIABLE_SOURCE_LINEAR}}
                                            const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_LINEAR}}
                                            
                                            {{#VARIABLE_SOURCE_NONLINEAR}}
                                            const double {{VARIABLE_SHORT}}_val = {{VARIABLE_SHORT}}->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

                                            {{#FORM_EXPRESSION_ESSENTIAL}}
                                            
                                            // {{EXPRESSION_ID}}
                                            if (comp == {{ROW_INDEX}})
                                                constraintsDirichlet.set_inhomogeneity(local_face_dof_indices[i], {{EXPRESSION}});
                                            {{/FORM_EXPRESSION_ESSENTIAL}}
                                        }
                                        else
                                        {
                                            // zero Dirichlet lift (Newton's method)
                                            constraintsZeroDirichlet.add_line (local_face_dof_indices[i]);
                                            constraintsZeroDirichlet.set_inhomogeneity(local_face_dof_indices[i], 0.0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    {{/EXACT_SOURCE}}
                }
            }
        }
    }
}

void SolverDeal{{CLASS}}::Assemble{{CLASS}}::copyLocalToGlobal(const AssemblyCopyData &copy_data)
{
    // distribute local to global system
    if (copy_data.isAssembled)
    {
        bool emptyLocalMatrix = copy_data.cell_matrix.all_zero();

        if (emptyLocalMatrix)
        {
            // rhs only
            constraintsAll.distribute_local_to_global(copy_data.cell_rhs,
                                                      copy_data.local_dof_indices,
                                                      systemRHS);
        }
        else
        {
            // matrix and rhs
            constraintsAll.distribute_local_to_global(copy_data.cell_matrix,
                                                      copy_data.cell_rhs,
                                                      copy_data.local_dof_indices,
                                                      systemMatrix,
                                                      systemRHS);
        }

        if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        {
            // transient mass matrix
            constraintsAll.distribute_local_to_global(copy_data.cell_mass_matrix,
                                                      copy_data.local_dof_indices,
                                                      transientMassMatrix);
        }
    }

 /*   std::map<unsigned int,double> boundary_values;
    boundary_values.insert(std::pair<unsigned int, double>(2587, -666));
    dealii::MatrixTools::apply_boundary_values (boundary_values,
        systemMatrix,
        solution,
        systemRHS);
*/
}

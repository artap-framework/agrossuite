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

void SolverDeal{{CLASS}}::assembleSystem(const dealii::Vector<double> &solutionNonlinearPrevious,
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

    TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell_begin, cell_end, source_begin, source_end;
    cell_begin = m_doFHandler.begin_active();
    cell_end = m_doFHandler.end();

    // if there is no source, we use the same as dummy
    source_begin = m_doFHandler.begin_active();
    source_end = m_doFHandler.end();
    // coupling sources{{#COUPLING_SOURCE}}
    if (m_problem->hasField("{{COUPLING_SOURCE_ID}}"))
    {
        source_begin = ProblemSolver::solvers()["{{COUPLING_SOURCE_ID}}"]->doFHandler().begin_active();
        source_end = ProblemSolver::solvers()["{{COUPLING_SOURCE_ID}}"]->doFHandler().end();
    }
    {{/COUPLING_SOURCE}}

    // Fix the beginning cells wrt. subdomains.
    while (cell_begin != m_doFHandler.end())
    {
        if (!Agros2D::scene()->labels->at(cell_begin->material_id() - 1)->marker(m_fieldInfo)->isNone())
            break;
        else
        {
            ++cell_begin;
            ++source_begin;
        }
    }

    dealii::WorkStream::run(DoubleCellIterator(source_begin, cell_begin, m_doFHandler, m_fieldInfo),
                            DoubleCellIterator(source_end, cell_end, m_doFHandler, m_fieldInfo),
                            *this,
                            &SolverDeal{{CLASS}}::localAssembleSystem,
                            &SolverDeal{{CLASS}}::copyLocalToGlobal,
                            AssemblyScratchData(*m_feCollection, *m_mappingCollection, m_quadratureFormulas, m_quadratureFormulasFace,
                                                solutionNonlinearPrevious,
                                                assembleMatrix,
                                                assembleRHS),
                            AssemblyCopyData());
}

void SolverDeal{{CLASS}}::localAssembleSystem(const DoubleCellIterator &iter,
                                              AssemblyScratchData &scratch_data,
                                              AssemblyCopyData &copy_data)
{
    const TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell = iter.cell_second;

    // coupling sources{{#COUPLING_SOURCE}}
    dealii::hp::DoFHandler<2>::active_cell_iterator cell_{{COUPLING_SOURCE_ID}};
    const SolverDeal* {{COUPLING_SOURCE_ID}}_solver = ProblemSolver::solvers()["{{COUPLING_SOURCE_ID}}"];
    if (m_problem->hasField("{{COUPLING_SOURCE_ID}}"))
    {
        cell_{{COUPLING_SOURCE_ID}} = iter.cell_first;
    }
    {{/COUPLING_SOURCE}}

    CoordinateType coordinateType = m_problem->config()->coordinateType();
    bool isTransient = (m_fieldInfo->analysisType() == AnalysisType_Transient);

    // materials
    SceneMaterial *material = m_scene->labels->at(cell->material_id() - 1)->marker(m_fieldInfo);

    if (material != m_scene->materials->getNone(m_fieldInfo))
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

        // volume value and grad cache
        AssembleCache &cache = assembleCache(tbb::this_tbb_thread::get_id(), dofs_per_cell, n_q_points);

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
        if (m_problem->hasField("{{COUPLING_SOURCE_ID}}"))
        {            
            {{COUPLING_SOURCE_ID}}_fieldInfo = Agros2D::problem()->fieldInfo("{{COUPLING_SOURCE_ID}}");
            {{COUPLING_SOURCE_ID}}_material = m_scene->labels->at(cell->material_id() - 1)->marker({{COUPLING_SOURCE_ID}}_fieldInfo);

            // todo: we probably do not need to initialize everything
            dealii::hp::FEValues<2> {{COUPLING_SOURCE_ID}}_hp_fe_values({{COUPLING_SOURCE_ID}}_solver->feCollection(), {{COUPLING_SOURCE_ID}}_solver->quadrature_formulas(), dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);
            {{COUPLING_SOURCE_ID}}_hp_fe_values.reinit(cell_{{COUPLING_SOURCE_ID}});
            const dealii::FEValues<2> &{{COUPLING_SOURCE_ID}}_fe_values = {{COUPLING_SOURCE_ID}}_hp_fe_values.get_present_fe_values();

            {{COUPLING_SOURCE_ID}}_value = std::vector<dealii::Vector<double> >({{COUPLING_SOURCE_ID}}_fe_values.n_quadrature_points, dealii::Vector<double>({{COUPLING_SOURCE_ID}}_fieldInfo->numberOfSolutions()));
            {{COUPLING_SOURCE_ID}}_grad = std::vector<std::vector<dealii::Tensor<1,2> > >({{COUPLING_SOURCE_ID}}_fe_values.n_quadrature_points, std::vector<dealii::Tensor<1,2> >({{COUPLING_SOURCE_ID}}_fieldInfo->numberOfSolutions()));

            {{COUPLING_SOURCE_ID}}_fe_values.get_function_values(m_coupling_sources["{{COUPLING_SOURCE_ID}}"], {{COUPLING_SOURCE_ID}}_value);
            {{COUPLING_SOURCE_ID}}_fe_values.get_function_gradients(m_coupling_sources["{{COUPLING_SOURCE_ID}}"], {{COUPLING_SOURCE_ID}}_grad);

            if ({{COUPLING_SOURCE_ID}}_material != m_scene->materials->getNone({{COUPLING_SOURCE_ID}}_fieldInfo))
            {
                const QMap<uint, QSharedPointer<Value> > {{COUPLING_SOURCE_ID}}_materialValues = {{COUPLING_SOURCE_ID}}_material->values();
                //{{COUPLING_SOURCE_ID}} variables for individual source analysis types{{#COUPLING_VARIABLES_ANALYSIS_TYPE}}
                if({{COUPLING_SOURCE_ID}}_fieldInfo->analysisType() == {{ANALYSIS_TYPE}})
                {{{#COUPLING_VARIABLES}}
                    // {{VARIABLE}}
                    {{VARIABLE_SHORT}} = {{COUPLING_SOURCE_ID}}_materialValues[{{VARIABLE_HASH}}]->number(); {{/COUPLING_VARIABLES}}
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
                SceneBoundary *boundary = m_scene->edges->at(cell->face(face)->user_index() - 1)->marker(m_fieldInfo);
                if (boundary != m_scene->boundaries->getNone(m_fieldInfo))
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
                    if({{COUPLING_SOURCE_ID}}_fieldInfo && ({{COUPLING_SOURCE_ID}}_material != m_scene->materials->getNone({{COUPLING_SOURCE_ID}}_fieldInfo)))
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

        // prepare QCollection
        dealii::hp::FECollection<2> finite_elements(m_doFHandler.get_fe());
        dealii::hp::QCollection<2 - 1> q_collection;
        for (unsigned int f = 0; f<finite_elements.size(); ++f)
        {
            const dealii::FiniteElement<2> &fe = finite_elements[f];

            if (fe.has_face_support_points())
                q_collection.push_back(dealii::Quadrature<2 - 1>(fe.get_unit_face_support_points()));
            else
            {
                std::vector<dealii::Point<2 - 1> > unit_support_points(fe.dofs_per_face);

                for (unsigned int i = 0; i<fe.dofs_per_face; ++i)
                    if (fe.is_primitive(fe.face_to_cell_index(i, 0)))
                        // if (mask[fe.face_system_to_component_index(i).first] == true)
                        unit_support_points[i] = fe.unit_face_support_point(i);

                q_collection.push_back(dealii::Quadrature<2 - 1>(unit_support_points));
            }
        }
        dealii::hp::FEFaceValues<2> hp_fe_face_values(mappingCollection(), feCollection(), q_collection, dealii::update_quadrature_points);
        
        // boundaries
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if(cell->face(face)->user_index() > 0 )
            {
                SceneBoundary *boundary = m_scene->edges->at(cell->face(face)->user_index() - 1)->marker(m_fieldInfo);
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

                        scratch_data.hp_fe_face_values.reinit(cell, face);

                        const dealii::FEFaceValues<2> &fe_face_values = scratch_data.hp_fe_face_values.get_present_fe_values();
                        const unsigned int n_face_q_points = fe_face_values.n_quadrature_points;
                        
                        for (unsigned int q_point = 0; q_point < n_face_q_points; ++q_point)
                        {
                            const dealii::Point<2> p = cache.shape_face_point[face][q_point];

                            {{#VARIABLE_SOURCE_NONLINEAR}}
                            // {{VARIABLE}}
                            const double {{VARIABLE_SHORT}}_val = boundaryValues[{{VARIABLE_HASH}}]->{{VARIABLE_VALUE}}; {{/VARIABLE_SOURCE_NONLINEAR}}

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

void SolverDeal{{CLASS}}::assembleDirichlet(bool calculateDirichletLiftValue)
{
    CoordinateType coordinateType = m_problem->config()->coordinateType();

    // prepare QCollection
    dealii::hp::FECollection<2> finite_elements(m_doFHandler.get_fe());
    dealii::hp::QCollection<2-1> q_collection;
    for (unsigned int f = 0; f<finite_elements.size(); ++f)
    {
        const dealii::FiniteElement<2> &fe = finite_elements[f];

        if (fe.has_face_support_points())
            q_collection.push_back (dealii::Quadrature<2-1>(fe.get_unit_face_support_points()));
        else
        {
            std::vector<dealii::Point<2-1> > unit_support_points (fe.dofs_per_face);

            for (unsigned int i=0; i<fe.dofs_per_face; ++i)
                if (fe.is_primitive (fe.face_to_cell_index(i,0)))
                    // if (mask[fe.face_system_to_component_index(i).first] == true)
                    unit_support_points[i] = fe.unit_face_support_point(i);

            q_collection.push_back (dealii::Quadrature<2-1>(unit_support_points));
        }
    }

    // hp face values
    dealii::hp::FEFaceValues<2> hp_fe_face_values(mappingCollection(), feCollection(), q_collection, dealii::update_quadrature_points);

    dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler.begin_active(), endc = m_doFHandler.end();
    for(; cell != endc; ++cell)
    {
        // boundaries
        for (unsigned int face = 0; face < dealii::GeometryInfo<2>::faces_per_cell; ++face)
        {
            if (cell->face(face)->user_index() > 0)
            {
                SceneBoundary *boundary = m_scene->edges->at(cell->face(face)->user_index() - 1)->marker(m_fieldInfo);
                if (boundary != m_scene->boundaries->getNone(m_fieldInfo))
                {
                    const dealii::FiniteElement<2> &fe = cell->get_fe();

                    hp_fe_face_values.reinit(cell, face);
                    const dealii::FEFaceValues<2> &fe_values = hp_fe_face_values.get_present_fe_values();
                    std::vector<dealii::Point<2> > points;
                    points.reserve(dealii::DoFTools::max_dofs_per_face(m_doFHandler));
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
                                        // todo: nonconstant essential BC
                                        if (calculateDirichletLiftValue)
                                        {
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

void SolverDeal{{CLASS}}::copyLocalToGlobal(const AssemblyCopyData &copy_data)
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
}

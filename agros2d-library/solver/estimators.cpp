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

// deal.ii
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/work_stream.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/grid/grid_refinement.h>

#include "estimators.h"
#include "solver.h"

#include "problem.h"

void ErrorEstimator::estimateAdaptivitySmoothness(const dealii::hp::FECollection<2> &feCollection,
                                                  const dealii::hp::DoFHandler<2> &doFHandler,
                                                  const dealii::Vector<double> &solution,
                                                  dealii::Vector<float> &smoothness_indicators)
{
    const unsigned int N = 5;
    std::vector<dealii::Tensor<1,2> > k_vectors;
    std::vector<unsigned int> k_vectors_magnitude;


    for (unsigned int i=0; i<N; ++i)
        for (unsigned int j=0; j<N; ++j)
            if (!((i==0) && (j==0)) && (i*i + j*j < N*N))
            {
                k_vectors.push_back (dealii::Point<2>(M_PI * i, M_PI * j));
                k_vectors_magnitude.push_back (i*i+j*j);
            }


    const unsigned n_fourier_modes = k_vectors.size();
    std::vector<double> ln_k (n_fourier_modes);
    for (unsigned int i=0; i<n_fourier_modes; ++i)
        ln_k[i] = std::log (k_vectors[i].norm());

    std::vector<dealii::Table<2,std::complex<double> > > fourier_transform_matrices (feCollection.size());
    dealii::QGauss<1> base_quadrature(2);
    dealii::QIterated<2> quadrature (base_quadrature, N);
    for (unsigned int fe = 0; fe < feCollection.size(); ++fe)
    {
        fourier_transform_matrices[fe].reinit (n_fourier_modes, (feCollection)[fe].dofs_per_cell);
        for (unsigned int k=0; k<n_fourier_modes; ++k)
        {
            for (unsigned int j = 0; j < (feCollection)[fe].dofs_per_cell; ++j)
            {
                std::complex<double> sum = 0;
                for (unsigned int q=0; q<quadrature.size(); ++q)
                {
                    const dealii::Point<2> x_q = quadrature.point(q);
                    sum += std::exp(std::complex<double>(0,1) * (k_vectors[k] * x_q)) * (feCollection)[fe].shape_value(j,x_q) * quadrature.weight(q);
                }
                fourier_transform_matrices[fe](k,j) = sum / std::pow(2*M_PI, 1);
            }
        }
    }
    std::vector<std::complex<double> > fourier_coefficients (n_fourier_modes);
    dealii::Vector<double> local_dof_values;
    TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
    for (unsigned int index=0; cell!=endc; ++cell, ++index)
    {
        local_dof_values.reinit (cell->get_fe().dofs_per_cell);
        cell->get_dof_values(solution, local_dof_values);
        for (unsigned int f=0; f<n_fourier_modes; ++f)
        {
            fourier_coefficients[f] = 0;
            for (unsigned int i=0; i<cell->get_fe().dofs_per_cell; ++i)
                fourier_coefficients[f] += fourier_transform_matrices[cell->active_fe_index()](f,i) * local_dof_values(i);
        }

        std::map<unsigned int, double> k_to_max_U_map;
        for (unsigned int f=0; f<n_fourier_modes; ++f)
            if ((k_to_max_U_map.find (k_vectors_magnitude[f]) ==
                 k_to_max_U_map.end()) || (k_to_max_U_map[k_vectors_magnitude[f]] < std::abs (fourier_coefficients[f])))
                k_to_max_U_map[k_vectors_magnitude[f]] = std::abs (fourier_coefficients[f]);
        double sum_1 = 0,
                sum_ln_k = 0,
                sum_ln_k_square = 0,
                sum_ln_U = 0,
                sum_ln_U_ln_k = 0;
        for (unsigned int f=0; f<n_fourier_modes; ++f)
        {
            if (k_to_max_U_map[k_vectors_magnitude[f]] == std::abs (fourier_coefficients[f]))
            {
                sum_1 += 1;
                sum_ln_k += ln_k[f];
                sum_ln_k_square += ln_k[f]*ln_k[f];
                sum_ln_U += std::log (std::abs (fourier_coefficients[f]));
                sum_ln_U_ln_k += std::log (std::abs (fourier_coefficients[f])) * ln_k[f];
            }
        }

        const double mu = (1./(sum_1*sum_ln_k_square - sum_ln_k*sum_ln_k) * (sum_ln_k*sum_ln_U - sum_1*sum_ln_U_ln_k));
        smoothness_indicators(index) = mu - 1;
    }
}

void ErrorEstimator::prepareGridRefinement(const FieldInfo *fieldInfo,
                                           const dealii::hp::FECollection<2> &feCollection,
                                           const dealii::hp::QCollection<2-1> &quadratureFormulasFace,
                                           const dealii::Vector<double> &solution,
                                           dealii::hp::DoFHandler<2> &doFHandler,
                                           int maxHIncrease,
                                           int maxPIncrease)
{
    // estimated error per cell
    dealii::Vector<float> estimated_error_per_cell(Agros2D::problem()->calculationMesh().n_active_cells());

    bool estimate = ((AdaptivityStrategy) fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt()) != AdaptivityStrategy_GlobalRefinement;

    // estimator
    if (estimate)
    {
        switch ((AdaptivityEstimator) fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt())
        {
        case AdaptivityEstimator_Kelly:
            dealii::KellyErrorEstimator<2>::estimate(doFHandler,
                                                     quadratureFormulasFace,
                                                     TYPENAME dealii::FunctionMap<2>::type(),
                                                     solution,
                                                     estimated_error_per_cell);
            break;
        case AdaptivityEstimator_Gradient:
            GradientErrorEstimator::estimate(doFHandler,
                                             solution,
                                             estimated_error_per_cell);
            break;
        default:
            assert(0);
        }
    }

    // cout << estimated_error_per_cell.l2_norm() << endl;

    // strategy
    switch ((AdaptivityStrategy) fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt())
    {
    case AdaptivityStrategy_FixedFractionOfCells:
    {
        dealii::GridRefinement::refine_and_coarsen_fixed_number(Agros2D::problem()->calculationMesh(),
                                                                estimated_error_per_cell,
                                                                fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt() / 100.0,
                                                                fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt() / 100.0);
    }
        break;
    case AdaptivityStrategy_FixedFractionOfTotalError:
    {
        dealii::GridRefinement::refine_and_coarsen_fixed_fraction(Agros2D::problem()->calculationMesh(),
                                                                  estimated_error_per_cell,
                                                                  fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt() / 100.0,
                                                                  fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt() / 100.0);
    }
        break;
    case AdaptivityStrategy_BalancedErrorAndCost:
    {
        dealii::GridRefinement::refine_and_coarsen_optimize(Agros2D::problem()->calculationMesh(),
                                                            estimated_error_per_cell);
    }
        break;
    case AdaptivityStrategy_GlobalRefinement:
    {
        dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endcmm = doFHandler.end();
        for (unsigned int index = 0; cell != endcmm; ++cell, ++index)
            cell->set_refine_flag();
    }
        break;
    default:
        assert(0);
    }

    // additional informations for p and hp adaptivity
    float min_smoothness = 0.0;
    float max_smoothness = 0.0;
    dealii::Vector<float> smoothnessIndicators;

    if (fieldInfo->adaptivityType() == AdaptivityMethod_HP)
    {
        smoothnessIndicators.reinit(Agros2D::problem()->calculationMesh().n_active_cells());
        ErrorEstimator::estimateAdaptivitySmoothness(feCollection, doFHandler, solution, smoothnessIndicators);

        min_smoothness = *std::max_element(smoothnessIndicators.begin(), smoothnessIndicators.end());
        max_smoothness = *std::min_element(smoothnessIndicators.begin(), smoothnessIndicators.end());
        dealii::hp::DoFHandler<2>::active_cell_iterator cellmm = doFHandler.begin_active(), endcmm = doFHandler.end();
        for (unsigned int index = 0; cellmm != endcmm; ++cellmm, ++index)
        {
            if (cellmm->refine_flag_set())
            {
                max_smoothness = std::max(max_smoothness, smoothnessIndicators(index));
                min_smoothness = std::min(min_smoothness, smoothnessIndicators(index));
            }
        }
    }

    if ((fieldInfo->adaptivityType() == AdaptivityMethod_P) || (fieldInfo->adaptivityType() == AdaptivityMethod_HP))
    {
        const float threshold_smoothness = (max_smoothness + min_smoothness) / 2;

        dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
        for (unsigned int index = 0; cell != endc; ++cell, ++index)
        {
            if (fieldInfo->adaptivityType() == AdaptivityMethod_P)
            {
                if (cell->refine_flag_set())
                {
                    // remove h adaptivity flag
                    cell->clear_refine_flag();

                    if ((maxPIncrease == -1) || (cell->active_fe_index() - fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) <= maxPIncrease)
                    {
                        if (cell->active_fe_index() + 1 < doFHandler.get_fe().size())
                        {
                            // increase order
                            cell->set_active_fe_index(cell->active_fe_index() + 1);
                        }
                    }
                }
            }

            if (fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                if ((maxPIncrease == -1) || (cell->active_fe_index() - fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) <= maxPIncrease)
                {
                    if (cell->refine_flag_set() && (smoothnessIndicators(index) > threshold_smoothness)
                            && (cell->active_fe_index() + 1 < doFHandler.get_fe().size()))
                    {
                        // remove h adaptivity flag
                        cell->clear_refine_flag();
                        // increase order
                        cell->set_active_fe_index(cell->active_fe_index() + 1);
                    }
                }
            }
        }
    }


    // number of refinements
    if (maxHIncrease != -1)
    {
        dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
        for (unsigned int index = 0; cell != endc; ++cell, ++index)
        {
            // remove h adaptivity flag
            if (cell->level() > maxHIncrease)
                cell->clear_refine_flag();
        }
    }
}

GradientErrorEstimator::EstimateScratchData::EstimateScratchData(const dealii::hp::FECollection<2> &fe,
                                                                 const dealii::Vector<double> &solution)
    :
      fe_midpoint_value(fe,
                        dealii::hp::QCollection<2>(dealii::QMidpoint<2>()),
                        dealii::update_values | dealii::update_quadrature_points),
      solution(solution)
{
}

GradientErrorEstimator::EstimateScratchData::EstimateScratchData(const EstimateScratchData &scratch_data)
    :
      fe_midpoint_value(scratch_data.fe_midpoint_value.get_fe_collection(),
                        scratch_data.fe_midpoint_value.get_quadrature_collection(),
                        dealii::update_values | dealii::update_quadrature_points),
      solution(scratch_data.solution)
{
}

void GradientErrorEstimator::estimate(const dealii::hp::DoFHandler<2> &dof_handler,
                                      const dealii::Vector<double> &solution,
                                      dealii::Vector<float> &error_per_cell)
{
    Assert (error_per_cell.size() == dof_handler.get_tria().n_active_cells(),
            ExcInvalidVectorLength (error_per_cell.size(),
                                    dof_handler.get_tria().n_active_cells()));
    typedef std::tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Vector<float>::iterator> IteratorTuple;

    dealii::SynchronousIterators<IteratorTuple>
            begin_sync_it (IteratorTuple (dof_handler.begin_active(), error_per_cell.begin())),
            end_sync_it (IteratorTuple (dof_handler.end(), error_per_cell.end()));

    dealii::WorkStream::run(begin_sync_it,
                            end_sync_it,
                            &GradientErrorEstimator::estimate_cell,
                            std::function<void (const EstimateCopyData &)> (),
                            EstimateScratchData(dof_handler.get_fe(), solution),
                            EstimateCopyData());
}

void GradientErrorEstimator::estimate_cell(const dealii::SynchronousIterators<std::tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator,
                                           dealii::Vector<float>::iterator> > &cell,
                                           EstimateScratchData &scratch_data,
                                           const EstimateCopyData &)
{
    dealii::Tensor<2,2> Y;
    std::vector<typename dealii::hp::DoFHandler<2>::active_cell_iterator> active_neighbors;
    active_neighbors.reserve(dealii::GeometryInfo<2>::faces_per_cell * dealii::GeometryInfo<2>::max_children_per_face);
    TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell_it(std::get<0>(cell.iterators));
    scratch_data.fe_midpoint_value.reinit(cell_it);

    dealii::Tensor<1,2> projected_gradient;
    active_neighbors.clear ();
    for (unsigned int face_no = 0; face_no<dealii::GeometryInfo<2>::faces_per_cell; ++face_no)
    {
        if (!std::get<0>(cell.iterators)->at_boundary(face_no))
        {
            const TYPENAME dealii::hp::DoFHandler<2>::face_iterator face = std::get<0>(cell.iterators)->face(face_no);
            const TYPENAME dealii::hp::DoFHandler<2>::cell_iterator neighbor = std::get<0>(cell.iterators)->neighbor(face_no);
            if (neighbor->active())
            {
                active_neighbors.push_back(neighbor);
            }
            else
            {
                for (unsigned int subface_no=0; subface_no<face->n_children(); ++subface_no)
                    active_neighbors.push_back(std::get<0>(cell.iterators)->neighbor_child_on_subface(face_no,subface_no));
            }
        }
    }

    const dealii::Point<2> this_center = scratch_data.fe_midpoint_value.get_present_fe_values().quadrature_point(0);
    std::vector<double> this_midpoint_value(1);
    scratch_data.fe_midpoint_value.get_present_fe_values().get_function_values(scratch_data.solution, this_midpoint_value);
    std::vector<double> neighbor_midpoint_value(1);
    TYPENAME std::vector<TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator>::const_iterator neighbor_ptr = active_neighbors.begin();
    for (; neighbor_ptr!=active_neighbors.end(); ++neighbor_ptr)
    {
        const TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator neighbor = *neighbor_ptr;
        scratch_data.fe_midpoint_value.reinit (neighbor);
        const dealii::Point<2> neighbor_center = scratch_data.fe_midpoint_value.get_present_fe_values().quadrature_point(0);
        scratch_data.fe_midpoint_value.get_present_fe_values().get_function_values (scratch_data.solution, neighbor_midpoint_value);
        dealii::Point<2> y = neighbor_center - this_center;
        const double distance = std::sqrt(y.square());
        y /= distance;

        for (unsigned int i = 0; i < 2; ++i)
            for (unsigned int j = 0; j < 2; ++j)
                Y[i][j] += y[i] * y[j];

        projected_gradient += (neighbor_midpoint_value[0] - this_midpoint_value[0]) / distance * y;
    }

    AssertThrow (determinant(Y) != 0, ExcInsufficientDirections());
    const dealii::Tensor<2,2> Y_inverse = invert(Y);
    dealii::Point<2> gradient;
    contract (gradient, Y_inverse, projected_gradient);
    *(std::get<1>(cell.iterators)) = (std::pow(std::get<0>(cell.iterators)->diameter(), 2) * std::sqrt(gradient.square()));
}

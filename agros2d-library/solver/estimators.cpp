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
#include <deal.II/fe/fe_tools.h>

#include "estimators.h"

#include "problem.h"

#include <numeric>
#include <functional>

ErrorEstimator::ErrorEstimator(int maxDegree)
    : m_N(5), m_maxDegree(maxDegree)
{    
    int dim = 2;

    // We need to resize the matrix of fourier coefficients according to the number of modes N.
    dealii::TableIndices<2> size;
    for (unsigned int d = 0; d < dim; d++)
        size[d] = m_N;
    fourier_coefficients.reinit(size);
}

void ErrorEstimator::estimateAdaptivitySmoothness(const dealii::hp::DoFHandler<2> &doFHandler,
                                                  const dealii::hp::QCollection<2> &quadratureFormulas,
                                                  const dealii::Vector<double> &solution,
                                                  dealii::Vector<float> &smoothness_indicators)
{
    dealii::FESeries::Fourier<2> fourier(m_N, doFHandler.get_fe(), quadratureFormulas);

    // local DoF values:
    dealii::Vector<double> local_dof_values;

    // Then here is the loop:
    typename dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
    for (; cell != endc; ++cell)
    {
        local_dof_values.reinit(cell->get_fe().dofs_per_cell);
        cell->get_dof_values(solution, local_dof_values);

        fourier.calculate(local_dof_values, cell->active_fe_index(), fourier_coefficients);

        // The next thing, as explained in the introduction, is that we wanted
        // to only fit our exponential decay of Fourier coefficients to the
        // largest coefficients for each possible value of $|{\bf k}|$. To
        // this end, we use FESeries::process_coefficients() to rework coefficients
        // into the desired format.
        // We'll only take those Fourier coefficients with the largest magnitude for a given value of $|{\bf k}|$ and
        // thereby need to use VectorTools::Linfty_norm:
        std::pair<std::vector<unsigned int>, std::vector<double> > res =
                dealii::FESeries::process_coefficients<2>(fourier_coefficients,
                                                          dealii::std_cxx11::bind(&ErrorEstimator::predicate, this, dealii::std_cxx11::_1),
                                                          dealii::VectorTools::Linfty_norm);

        // The first vector in the <code>std::pair</code> will store values of the predicate,
        // that is $i*i+j*j= const$ or $i*i+j*j+k*k = const$
        // in 2D or 3D respectively. This
        // vector will be the same for all the cells so we can calculate
        // logarithms of the corresponding Fourier vectors $|{\bf k}|$ only once
        // in the whole hp-refinement cycle:
        if (ln_k.size() == 0)
        {
            ln_k.resize(res.first.size(),0);
            for (unsigned int f = 0; f < ln_k.size(); f++)
                ln_k[f] = std::log(2.0*M_PI*std::sqrt(1.*res.first[f]));
        }

        // We have to calculate the logarithms of absolute
        // values of coefficients and use it in linear regression fit to
        // obtain $\mu$.
        for (unsigned int f = 0; f < res.second.size(); f++)
            res.second[f] = std::log(res.second[f]);

        std::pair<double,double> fit = dealii::FESeries::linear_regression(ln_k,res.second);

        // The final step is to compute the Sobolev index $s=\mu-\frac d2$ and
        // store it in the vector of estimated values for each cell:
        int dim = 2;
        smoothness_indicators(cell->active_cell_index()) = -fit.first - 1.*dim/2;
    }
}

std::pair<bool, unsigned int> ErrorEstimator::predicate(const dealii::TableIndices<2> &ind)
{
    int dim = 2;
    unsigned int v = 0;

    for (unsigned int i = 0; i < dim; i++)
        v += ind[i]*ind[i];

    if (v > 0 && v < m_maxDegree*m_maxDegree)
        return std::make_pair(true,v);
    else
        return std::make_pair(false,v);
}


/*
void ErrorEstimator::estimateAdaptivitySmoothnessOld(const dealii::hp::DoFHandler<2> &doFHandler,
                                                     const dealii::hp::QCollection<2> &quadratureFormulas,
                                                     const dealii::Vector<double> &solution,
                                                     dealii::Vector<float> &smoothness_indicators)
{
    const dealii::hp::FECollection<2> &feCollection = doFHandler.get_fe();

    //    const unsigned int N = 5;
    //    dealii::FESeries::Fourier<2> fourier(N, feCollection, quadratureFormulas);
    //    dealii::Table<2, std::complex<double> > fourier_coefficients;

    //    dealii::Vector<double> local_dof_values;
    //    typename dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
    //    for (; cell != endc; ++cell)
    //    {
    //        local_dof_values.reinit (cell->get_fe().dofs_per_cell);
    //        cell->get_dof_values (solution, local_dof_values);
    //        fourier.calculate(local_dof_values,
    //                          cell->active_fe_index(),
    //                          fourier_coefficients);
    //        std::pair<std::vector<unsigned int>, std::vector<double> > res =
    //                dealii::FESeries::process_coefficients<2>(fourier_coefficients,
    //                                                          std_cxx11::bind(&LaplaceProblem<dim>::predicate,
    //                                                                          this,
    //                                                                          std_cxx11::_1),
    //                                                          VectorTools::Linfty_norm);
    //        Assert (res.first.size() == res.second.size(),
    //                ExcInternalError());
    //        if (ln_k.size() == 0)
    //        {
    //            ln_k.resize(res.first.size(),0);
    //            for (unsigned int f = 0; f < ln_k.size(); f++)
    //                ln_k[f] = std::log (2.0*numbers::PI*std::sqrt(1.*res.first[f]));
    //        }
    //        for (unsigned int f = 0; f < res.second.size(); f++)
    //            res.second[f] = std::log(res.second[f]);
    //        std::pair<double,double> fit = FESeries::linear_regression(ln_k,res.second);
    //        smoothness_indicators(cell->active_cell_index()) = -fit.first - 1.*dim/2;
    //    }

    const unsigned int N = 5;
    std::vector<dealii::Tensor<1,2> > k_vectors;
    std::vector<unsigned int> k_vectors_magnitude;


    for (unsigned int i=0; i<N; ++i)
    {
        for (unsigned int j=0; j<N; ++j)
        {
            if (!((i==0) && (j==0)) && (i*i + j*j < N*N))
            {
                k_vectors.push_back (dealii::Point<2>(M_PI * i, M_PI * j));
                k_vectors_magnitude.push_back (i*i+j*j);
            }
        }
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
    std::vector<std::complex<double> > fourier_coefficients(n_fourier_modes);
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
        {
            if ((k_to_max_U_map.find (k_vectors_magnitude[f]) ==
                 k_to_max_U_map.end()) || (k_to_max_U_map[k_vectors_magnitude[f]] < std::abs (fourier_coefficients[f])))
                k_to_max_U_map[k_vectors_magnitude[f]] = std::abs (fourier_coefficients[f]);
        }

        double sum_1 = 0;
        double sum_ln_k = 0;
        double sum_ln_k_square = 0;
        double sum_ln_U = 0;
        double sum_ln_U_ln_k = 0;
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
*/

double ErrorEstimator::relativeChangeBetweenSolutions(const dealii::hp::DoFHandler<2> &doFHandler,
                                                      const dealii::hp::QCollection<2> &quadratureFormulas,
                                                      const dealii::Vector<double> &sln1,
                                                      const dealii::Vector<double> &sln2)
{
    int numberOfSolutions = doFHandler.get_fe().n_components();

    double normCurrentL2 = 0.0;
    double normCurrentH1Semi = 0.0;
    double normPrevious = 0.0;
    double normDifference = 0.0;

    dealii::hp::FEValues<2> hp_fe_values(doFHandler.get_fe(), quadratureFormulas, dealii::update_values | dealii::update_gradients | dealii::update_JxW_values);

    dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = doFHandler.begin_active(), endc_int = doFHandler.end();
    for (; cell_int != endc_int; ++cell_int)
    {
        // volume integration
        hp_fe_values.reinit(cell_int);

        const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
        const unsigned int n_q_points = fe_values.n_quadrature_points;

        std::vector<dealii::Vector<double> > solution_values(n_q_points, dealii::Vector<double>(numberOfSolutions));
        std::vector<std::vector<dealii::Tensor<1,2> > > solution_gradients(n_q_points, std::vector<dealii::Tensor<1,2> >(numberOfSolutions));

        fe_values.get_function_values(sln1, solution_values);
        fe_values.get_function_gradients(sln1, solution_gradients);

        std::vector<dealii::Vector<double> > solution_previous_values(n_q_points, dealii::Vector<double>(numberOfSolutions));
        std::vector<std::vector<dealii::Tensor<1,2> > >  solution_previous_grads(n_q_points, std::vector<dealii::Tensor<1,2> >(numberOfSolutions));

        fe_values.get_function_values(sln2, solution_previous_values);
        fe_values.get_function_gradients(sln2, solution_previous_grads);

        for (unsigned int j = 0; j < numberOfSolutions; j++)
        {
            for (unsigned int k = 0; k < n_q_points; ++k)
            {
                normCurrentL2 += fe_values.JxW(k) * (solution_values[k][j] * solution_values[k][j]);
                normCurrentH1Semi += fe_values.JxW(k) * (solution_gradients[k][j] * solution_gradients[k][j]);

                normPrevious += fe_values.JxW(k) * (solution_previous_values[k][j] * solution_previous_values[k][j] + solution_previous_grads[k][j] * solution_previous_grads[k][j]);
                normDifference += fe_values.JxW(k) * (((solution_values[k][j] - solution_previous_values[k][j]) * (solution_values[k][j] - solution_previous_values[k][j]))
                                                      + ((solution_gradients[k][j] - solution_previous_grads[k][j]) * (solution_gradients[k][j] - solution_previous_grads[k][j])));
            }
        }
    }

    double normCurrent = normCurrentL2 + normCurrentH1Semi;

    // std::cout << "normPrevious = " << normPrevious <<
    //              ", normCurrentL2 = " << normCurrentL2 <<
    //              ", normCurrentH1Semi = " << normCurrentH1Semi <<
    //              ", normCurrent = " << normCurrent <<
    //              ", normDifference = " << normDifference <<
    //              ", relChangeSol = " << fabs(normDifference / normCurrent) * 100.0 << std::endl;

    // compute difference between previous and current solution - H1 norm
    double relChangeSol = fabs(normDifference / normCurrent) * 100.0;

    return relChangeSol;
}

// ************************************************************************************************************************

DifferenceErrorEstimator::EstimateScratchData::EstimateScratchData(const dealii::hp::FECollection<2> &primal_fe,
                                                                   const dealii::Vector<double> &primal_solution,
                                                                   const dealii::Vector<double> &dual_solution)
    :
      hp_fe_values(primal_fe,
                   dealii::hp::QCollection<2>(dealii::QGauss<2>(5)),
                   dealii::update_values | dealii::update_gradients | dealii::update_JxW_values),
      primal_solution(primal_solution),
      dual_solution(dual_solution)
{
}

DifferenceErrorEstimator::EstimateScratchData::EstimateScratchData(const EstimateScratchData &scratch_data)
    :
      hp_fe_values(scratch_data.hp_fe_values.get_fe_collection(),
                   scratch_data.hp_fe_values.get_quadrature_collection(),
                   dealii::update_values | dealii::update_gradients | dealii::update_JxW_values),
      primal_solution(scratch_data.primal_solution),
      dual_solution(scratch_data.dual_solution)
{
}

void DifferenceErrorEstimator::estimate(const dealii::hp::DoFHandler<2> &primal_dof,
                                        const dealii::Vector<double> &primal_solution,
                                        const dealii::Vector<double> &dual_solution,
                                        dealii::Vector<float> &error_per_cell)
{
    Assert (error_per_cell.size() == primal_dof.get_tria().n_active_cells(),
            ExcInvalidVectorLength (error_per_cell.size(),
                                    primal_dof.get_tria().n_active_cells()));
    typedef std_tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Vector<float>::iterator> IteratorTuple;

    dealii::SynchronousIterators<IteratorTuple>
            begin_sync_it (IteratorTuple(primal_dof.begin_active(), error_per_cell.begin())),
            end_sync_it (IteratorTuple(primal_dof.end(), error_per_cell.end()));

    dealii::WorkStream::run(begin_sync_it,
                            end_sync_it,
                            &DifferenceErrorEstimator::estimate_cell,
                            std::function<void (const EstimateCopyData &)> (),
                            EstimateScratchData(primal_dof.get_fe(), primal_solution, dual_solution),
                            EstimateCopyData());
}

void DifferenceErrorEstimator::estimate_cell(const dealii::SynchronousIterators<std_tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Vector<float>::iterator> > &cell,
                                             EstimateScratchData &scratch_data,
                                             const EstimateCopyData &)
{
    TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell_it(std_get<0>(cell.iterators));

    const unsigned int dofs_per_cell = cell_it->get_fe().dofs_per_cell;

    // reinit volume
    scratch_data.hp_fe_values.reinit(cell_it);

    const dealii::FEValues<2> &fe_values = scratch_data.hp_fe_values.get_present_fe_values();
    const unsigned int n_q_points = fe_values.n_quadrature_points;

    // TODO: more solutions
    std::vector<dealii::Vector<double> > primal_solution_value(n_q_points, dealii::Vector<double>(cell_it->get_fe().n_components()));
    std::vector<std::vector<dealii::Tensor<1, 2> > > primal_solution_gradients(n_q_points, std::vector<dealii::Tensor<1, 2> >(cell_it->get_fe().n_components()));
    std::vector<dealii::Vector<double> > dual_solution_value(n_q_points, dealii::Vector<double>(cell_it->get_fe().n_components()));
    std::vector<std::vector<dealii::Tensor<1, 2> > > dual_solution_gradients(n_q_points, std::vector<dealii::Tensor<1, 2> >(cell_it->get_fe().n_components()));
    fe_values.get_function_values(scratch_data.primal_solution, primal_solution_value);
    fe_values.get_function_gradients(scratch_data.primal_solution, primal_solution_gradients);
    fe_values.get_function_values(scratch_data.dual_solution, dual_solution_value);
    fe_values.get_function_gradients(scratch_data.dual_solution, dual_solution_gradients);

    // h1-norm
    double value = 0.0;
    for (unsigned int k = 0; k < n_q_points; ++k)
    {
        for (unsigned int l = 0; l < cell_it->get_fe().n_components(); l++)
        {
            // h1-norm
            value += fe_values.JxW(k) * ((primal_solution_value[k][l] - dual_solution_value[k][l]) * (primal_solution_value[k][l] - dual_solution_value[k][l])
                                         + (primal_solution_gradients[k][l] - dual_solution_gradients[k][l]) * (primal_solution_gradients[k][l] - dual_solution_gradients[k][l]));
            // l2-norm
            // value += fe_values.JxW(k) * ((primal_solution_value[k][l] - dual_solution_value[k][l]) * (primal_solution_value[k][l] - dual_solution_value[k][l]));
            // h1-seminorm
            // value += fe_values.JxW(k) * ((primal_solution_gradients[k][l] - dual_solution_gradients[k][l]) * (primal_solution_gradients[k][l] - dual_solution_gradients[k][l]));
        }
    }

    *(std_get<1>(cell.iterators)) = value;
    // *(std_get<1>(cell.iterators)) = log(value);
}

// ************************************************************************************************************************

/*
template <int dim>
void
WeightedResidual<dim>::estimate_error (dealii::Vector<float> &error_indicators) const
{
    dealii::ConstraintMatrix dual_hanging_node_constraints;
    dealii::DoFTools::make_hanging_node_constraints (dual_solver->doFHandler,
                                                     dual_hanging_node_constraints);
    dual_hanging_node_constraints.close();
    dealii::Vector<double> primal_solution (dual_solver->doFHandler.n_dofs());
    dealii::FETools::interpolate(primal_solver->doFHandler,
                                 primal_solver->solution,
                                 dual_solver->doFHandler,
                                 dual_hanging_node_constraints,
                                 primal_solution);

    dealii::ConstraintMatrix primal_hanging_node_constraints;
    dealii::DoFTools::make_hanging_node_constraints (primal_solver->doFHandler,
                                                     primal_hanging_node_constraints);
    primal_hanging_node_constraints.close();

    // NEW - CHECK
    dealii::Vector<double> dual_solution (dual_solver->doFHandler.n_dofs());
    dealii::FETools::interpolate(dual_solver->doFHandler,
                                 dual_solver->solution,
                                 primal_solver->doFHandler,
                                 primal_hanging_node_constraints,
                                 dual_solution);


    dealii::Vector<double> dual_weights (dual_solver->doFHandler.n_dofs());
    dual_weights = dual_solution;
    dual_weights.add(-1, primal_solution);


    FaceIntegrals face_integrals;
    for (dealii::hp::DoFHandler<2>::active_cell_iterator cell = dual_solver->doFHandler.begin_active(); cell != dual_solver->doFHandler.end(); ++cell)
        for (unsigned int face_no=0; face_no<dealii::GeometryInfo<dim>::faces_per_cell; ++face_no)
            face_integrals[cell->face(face_no)] = -1e20;
    error_indicators.reinit (dual_solver->doFHandler.get_tria().n_active_cells());

    typedef std_tuple<dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Vector<float>::iterator> IteratorTuple;
    dealii::SynchronousIterators<IteratorTuple> cell_and_error_begin(IteratorTuple (dual_solver->doFHandler.begin_active(), error_indicators.begin()));
    dealii::SynchronousIterators<IteratorTuple> cell_and_error_end  (IteratorTuple (dual_solver->doFHandler.end(), error_indicators.begin()));
    dealii::WorkStream::run(cell_and_error_begin,
                            cell_and_error_end,
                            std::bind(&WeightedResidual<dim>::estimate_on_one_cell,
                                      this,
                                      std::placeholders::_1,
                                      std::placeholders::_2,
                                      std::placeholders::_3,
                                      std::ref(face_integrals)),
                            std::function<void (const WeightedResidualCopyData &)>(),
                            WeightedResidualScratchData (primal_solver,
                                                         dual_solver,
                                                         primal_solution,
                                                         dual_weights),
                            WeightedResidualCopyData());
    unsigned int present_cell=0;
    for (dealii::hp::DoFHandler<2>::active_cell_iterator cell=dual_solver->doFHandler.begin_active(); cell!=dual_solver->doFHandler.end(); ++cell, ++present_cell)
        for (unsigned int face_no=0; face_no<dealii::GeometryInfo<dim>::faces_per_cell;
             ++face_no)
        {
            Assert(face_integrals.find(cell->face(face_no)) !=
                    face_integrals.end(),
                   ExcInternalError());
            error_indicators(present_cell) -= 0.5*face_integrals[cell->face(face_no)];
        }
    std::cout << "   Estimated error="
              << std::accumulate (error_indicators.begin(),
                                  error_indicators.end(), 0.)
              << std::endl;
}

template <int dim>
void
WeightedResidual<dim>::estimate_on_one_cell (const dealii::SynchronousIterators<std_tuple<active_cell_iterator, dealii::Vector<float>::iterator> > &cell_and_error,
                                             WeightedResidualScratchData &scratch_data,
                                             WeightedResidualCopyData &copy_data,
                                             FaceIntegrals &face_integrals) const
{
    active_cell_iterator cell = std_get<0>(cell_and_error.iterators);
    integrate_over_cell (cell_and_error,
                         scratch_data.primal_solution,
                         scratch_data.dual_weights,
                         scratch_data.cell_data);

    for (unsigned int face_no=0; face_no<dealii::GeometryInfo<dim>::faces_per_cell; ++face_no)
    {
        if (cell->face(face_no)->at_boundary())
        {
            face_integrals[cell->face(face_no)] = 0;
            continue;
        }
        if ((cell->neighbor(face_no)->has_children() == false) &&
                (cell->neighbor(face_no)->level() == cell->level()) &&
                (cell->neighbor(face_no)->index() < cell->index()))
            continue;
        if (cell->at_boundary(face_no) == false)
            if (cell->neighbor(face_no)->level() < cell->level())
                continue;
        if (cell->face(face_no)->has_children() == false)
            integrate_over_regular_face (cell, face_no,
                                         scratch_data.primal_solution,
                                         scratch_data.dual_weights,
                                         scratch_data.face_data,
                                         face_integrals);
        else
            integrate_over_irregular_face (cell, face_no,
                                           scratch_data.primal_solution,
                                           scratch_data.dual_weights,
                                           scratch_data.face_data,
                                           face_integrals);
    }
}
template <int dim>
void WeightedResidual<dim>::integrate_over_cell (const dealii::SynchronousIterators<std_tuple<active_cell_iterator, dealii::Vector<float>::iterator> > &cell_and_error,
                                                 const dealii::Vector<double> &primal_solution,
                                                 const dealii::Vector<double> &dual_weights,
                                                 CellData &cell_data) const
{
    cell_data.hp_fe_values.reinit (std_get<0>(cell_and_error.iterators));
    int n_quadrature_points = cell_data.hp_fe_values.get_present_fe_values().n_quadrature_points;

    cell_data.rhs_values.resize(n_quadrature_points);
    cell_data.cell_residual.resize(n_quadrature_points);
    cell_data.dual_weights.resize(n_quadrature_points);
    cell_data.cell_laplacians.resize(n_quadrature_points);

    cell_data.hp_fe_values.get_present_fe_values().get_function_laplacians (primal_solution, cell_data.cell_laplacians);
    cell_data.hp_fe_values.get_present_fe_values().get_function_values (dual_weights, cell_data.dual_weights);

    qDebug() << cell_data.right_hand_side->value(dealii::Point<2>(0, 0));

    cell_data.right_hand_side->value_list(cell_data.hp_fe_values.get_present_fe_values().get_quadrature_points(), cell_data.rhs_values);

    double sum = 0;
    for (unsigned int p = 0; p < cell_data.hp_fe_values.get_present_fe_values().n_quadrature_points; ++p)
        sum += ((cell_data.rhs_values[p] + cell_data.cell_laplacians[p]) * cell_data.dual_weights[p] * cell_data.hp_fe_values.get_present_fe_values().JxW(p));

    *(std_get<1>(cell_and_error.iterators)) += sum;
}
template <int dim>
void WeightedResidual<dim>::integrate_over_regular_face (const active_cell_iterator &cell,
                                                         const unsigned int face_no,
                                                         const dealii::Vector<double> &primal_solution,
                                                         const dealii::Vector<double> &dual_weights,
                                                         FaceData &face_data,
                                                         FaceIntegrals &face_integrals) const
{
    face_data.fe_face_values_cell.reinit (cell, face_no);

    const unsigned int n_q_points = face_data.fe_face_values_cell.get_present_fe_values().n_quadrature_points;
    face_data.fe_face_values_cell.get_present_fe_values().get_function_gradients (primal_solution, face_data.cell_grads);
    Assert (cell->neighbor(face_no).state() == IteratorState::valid, ExcInternalError());

    const unsigned int neighbor_neighbor = cell->neighbor_of_neighbor (face_no);
    const active_cell_iterator neighbor = cell->neighbor(face_no);
    face_data.fe_face_values_neighbor.reinit (neighbor, neighbor_neighbor);
    face_data.fe_face_values_neighbor.get_present_fe_values().get_function_gradients (primal_solution,
                                                                                      face_data.neighbor_grads);
    for (unsigned int p=0; p<n_q_points; ++p)
        face_data.jump_residual[p] = ((face_data.cell_grads[p] - face_data.neighbor_grads[p]) * face_data.fe_face_values_cell.get_present_fe_values().normal_vector(p));
    face_data.fe_face_values_cell.get_present_fe_values().get_function_values (dual_weights, face_data.dual_weights);

    double face_integral = 0;
    for (unsigned int p=0; p<n_q_points; ++p)
        face_integral += (face_data.jump_residual[p] *
                          face_data.dual_weights[p]  *
                          face_data.fe_face_values_cell.get_present_fe_values().JxW(p));
    Assert (face_integrals.find (cell->face(face_no)) != face_integrals.end(), ExcInternalError());
    Assert (face_integrals[cell->face(face_no)] == -1e20, ExcInternalError());
    face_integrals[cell->face(face_no)] = face_integral;
}

template <int dim>
void WeightedResidual<dim>::integrate_over_irregular_face (const active_cell_iterator &cell,
                                                           const unsigned int          face_no,
                                                           const dealii::Vector<double> &primal_solution,
                                                           const dealii::Vector<double> &dual_weights,
                                                           FaceData &face_data,
                                                           FaceIntegrals &face_integrals) const
{
    const unsigned int n_q_points = face_data.fe_face_values_cell.get_present_fe_values().n_quadrature_points;
    const typename dealii::hp::DoFHandler<dim>::face_iterator face = cell->face(face_no);
    const typename dealii::hp::DoFHandler<dim>::cell_iterator neighbor = cell->neighbor(face_no);

    Assert (neighbor.state() == IteratorState::valid, ExcInternalError());
    Assert (neighbor->has_children(), ExcInternalError());
    const unsigned int neighbor_neighbor = cell->neighbor_of_neighbor (face_no);
    for (unsigned int subface_no=0; subface_no<face->n_children(); ++subface_no)
    {
        const active_cell_iterator neighbor_child = cell->neighbor_child_on_subface (face_no, subface_no);
        Assert (neighbor_child->face(neighbor_neighbor) == cell->face(face_no)->child(subface_no), ExcInternalError());
        face_data.fe_subface_values_cell.reinit (cell, face_no, subface_no);
        face_data.fe_subface_values_cell.get_present_fe_values().get_function_gradients (primal_solution, face_data.cell_grads);
        face_data.fe_face_values_neighbor.reinit (neighbor_child, neighbor_neighbor);
        face_data.fe_face_values_neighbor.get_present_fe_values().get_function_gradients (primal_solution, face_data.neighbor_grads);
        for (unsigned int p=0; p<n_q_points; ++p)
            face_data.jump_residual[p] = ((face_data.neighbor_grads[p] - face_data.cell_grads[p]) * face_data.fe_face_values_neighbor.get_present_fe_values().normal_vector(p));
        face_data.fe_face_values_neighbor.get_present_fe_values().get_function_values (dual_weights, face_data.dual_weights);
        double face_integral = 0;
        for (unsigned int p=0; p<n_q_points; ++p)
            face_integral += (face_data.jump_residual[p] * face_data.dual_weights[p] * face_data.fe_face_values_neighbor.get_present_fe_values().JxW(p));
        face_integrals[neighbor_child->face(neighbor_neighbor)]
                = face_integral;
    }
    double sum = 0;
    for (unsigned int subface_no=0;
         subface_no<face->n_children(); ++subface_no)
    {
        Assert (face_integrals.find(face->child(subface_no)) !=
                face_integrals.end(),
                ExcInternalError());
        Assert (face_integrals[face->child(subface_no)] != -1e20,
                ExcInternalError());
        sum += face_integrals[face->child(subface_no)];
    }
    face_integrals[face] = sum;
}

template class WeightedResidual<2>;
*/

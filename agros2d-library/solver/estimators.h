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

#ifndef ESTIMATORS_H
#define ESTIMATORS_H

#include "util.h"
#include "field.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/hp/dof_handler.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/hp/q_collection.h>
#include <deal.II/hp/fe_values.h>
#include <deal.II/base/synchronous_iterator.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#define signals public

namespace ErrorEstimator
{
void estimateAdaptivitySmoothness(const dealii::hp::FECollection<2> &feCollection,
                                  const dealii::hp::DoFHandler<2> &doFHandler,
                                  const dealii::Vector<double> &solution,
                                  dealii::Vector<float> &smoothness_indicators);
};

class GradientErrorEstimator
{
public:
    static void estimate(const dealii::hp::DoFHandler<2> &dof,
                         const dealii::Vector<double> &solution,
                         dealii::Vector<float> &error_per_cell);
    DeclException2 (ExcInvalidVectorLength,
                    int, int,
                    << "Vector has length " << arg1 << ", but should have "
                    << arg2);
    DeclException0 (ExcInsufficientDirections);

private:
    struct EstimateScratchData
    {
        EstimateScratchData(const dealii::hp::FECollection<2> &fe,
                            const dealii::Vector<double> &solution);
        EstimateScratchData(const EstimateScratchData &data);

        dealii::hp::FEValues<2> fe_midpoint_value;
        dealii::Vector<double> solution;
    };
    struct EstimateCopyData {};

    static void estimate_cell(const dealii::SynchronousIterators<std::tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator,
                              dealii::Vector<float>::iterator> > &cell,
                              EstimateScratchData &scratch_data,
                              const EstimateCopyData &copy_data);
};

class DifferenceErrorEstimator
{
public:
    static void estimate(const dealii::hp::DoFHandler<2> &primal_dof,
                         const dealii::Vector<double> &primal_solution,
                         const dealii::Vector<double> &dual_solution,
                         dealii::Vector<float> &error_per_cell);

    DeclException2 (ExcInvalidVectorLength,
                    int, int,
                    << "Vector has length " << arg1 << ", but should have "
                    << arg2);
    DeclException0 (ExcInsufficientDirections);

private:
    struct EstimateScratchData
    {
        EstimateScratchData(const dealii::hp::FECollection<2> &primal_fe,
                            const dealii::Vector<double> &primal_solution,
                            const dealii::Vector<double> &dual_solution);
        EstimateScratchData(const EstimateScratchData &data);

        dealii::hp::FEValues<2> hp_fe_values;

        dealii::Vector<double> primal_solution;
        dealii::Vector<double> dual_solution;
    };
    struct EstimateCopyData {};

    static void estimate_cell(const dealii::SynchronousIterators<std::tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Vector<float>::iterator> > &cell,
                              EstimateScratchData &scratch_data,
                              const EstimateCopyData &copy_data);
};
// *******************************************************************************************

/*
template <int dim>
class WeightedResidual : public PrimalSolver<dim>,
        public DualSolver<dim>
{
public:
    WeightedResidual (Triangulation<dim>       &coarse_grid,
                      const FiniteElement<dim> &primal_fe,
                      const FiniteElement<dim> &dual_fe,
                      const Quadrature<dim>    &quadrature,
                      const Quadrature<dim-1>  &face_quadrature,
                      const Function<dim>      &rhs_function,
                      const Function<dim>      &boundary_values,
                      const DualFunctional::DualFunctionalBase<dim> &dual_functional);

    virtual
    void
    solve_problem ();

    virtual
    void
    postprocess (const Evaluation::EvaluationBase<dim> &postprocessor) const;

    virtual
    unsigned int
    n_dofs () const;

    virtual void refine_grid ();

    virtual
    void
    output_solution () const;

private:
    // In the private section, we have two functions that are used to call
    // the <code>solve_problem</code> functions of the primal and dual base
    // classes. These two functions will be called in parallel by the
    // <code>solve_problem</code> function of this class.
    void solve_primal_problem ();
    void solve_dual_problem ();
    // Then declare abbreviations for active cell iterators, to avoid that
    // we have to write this lengthy name over and over again:

    typedef
    typename DoFHandler<dim>::active_cell_iterator
    active_cell_iterator;

    // Next, declare a data type that we will us to store the contribution
    // of faces to the error estimator. The idea is that we can compute the
    // face terms from each of the two cells to this face, as they are the
    // same when viewed from both sides. What we will do is to compute them
    // only once, based on some rules explained below which of the two
    // adjacent cells will be in charge to do so. We then store the
    // contribution of each face in a map mapping faces to their values, and
    // only collect the contributions for each cell by looping over the
    // cells a second time and grabbing the values from the map.
    //
    // The data type of this map is declared here:
    typedef
    typename std::map<typename DoFHandler<dim>::face_iterator,double>
    FaceIntegrals;

    // In the computation of the error estimates on cells and faces, we need
    // a number of helper objects, such as <code>FEValues</code> and
    // <code>FEFaceValues</code> functions, but also temporary objects
    // storing the values and gradients of primal and dual solutions, for
    // example. These fields are needed in the three functions that do the
    // integration on cells, and regular and irregular faces, respectively.
    //
    // There are three reasonable ways to provide these fields: first, as
    // local variables in the function that needs them; second, as member
    // variables of this class; third, as arguments passed to that function.
    //
    // These three alternatives all have drawbacks: the third that their
    // number is not negligible and would make calling these functions a
    // lengthy enterprise. The second has the drawback that it disallows
    // parallelization, since the threads that will compute the error
    // estimate have to have their own copies of these variables each, so
    // member variables of the enclosing class will not work. The first
    // approach, although straightforward, has a subtle but important
    // drawback: we will call these functions over and over again, many
    // thousands of times maybe; it now turns out that allocating
    // vectors and other objects that need memory from the heap is an
    // expensive business in terms of run-time, since memory allocation is
    // expensive when several threads are involved. It is thus
    // significantly better to allocate the memory only once, and recycle
    // the objects as often as possible.
    //
    // What to do? Our answer is to use a variant of the third strategy.
    // In fact, this is exactly what the WorkStream concept is supposed to
    // do (we have already introduced it above, but see also @ref threads).
    // To avoid that we have to give these functions a dozen or so
    // arguments, we pack all these variables into two structures, one which
    // is used for the computations on cells, the other doing them on the
    // faces. Both are then joined into the WeightedResidualScratchData class
    // that will serve as the "scratch data" class of the WorkStream concept:
    struct CellData
    {
        dealii::FEValues<dim>    fe_values;
        const dealii::SmartPointer<const dealii::Function<dim> > right_hand_side;

        std::vector<double> cell_residual;
        std::vector<double> rhs_values;
        std::vector<double> dual_weights;
        std::vector<double> cell_laplacians;
        CellData (const dealii::FiniteElement<dim> &fe,
                  const dealii::Quadrature<dim>    &quadrature,
                  const dealii::Function<dim>      &right_hand_side);
        CellData (const CellData &cell_data);
    };

    struct FaceData
    {
        dealii::FEFaceValues<dim>    fe_face_values_cell;
        dealii::FEFaceValues<dim>    fe_face_values_neighbor;
        dealii::FESubfaceValues<dim> fe_subface_values_cell;

        std::vector<double> jump_residual;
        std::vector<double> dual_weights;
        typename std::vector<dealii::Tensor<1,dim> > cell_grads;
        typename std::vector<dealii::Tensor<1,dim> > neighbor_grads;
        FaceData (const dealii::FiniteElement<dim> &fe,
                  const dealii::Quadrature<dim-1>  &face_quadrature);
        FaceData (const FaceData &face_data);
    };

    struct WeightedResidualScratchData
    {
        WeightedResidualScratchData(const PrimalSolver<dim> &primal_solver,
                                    const DualSolver<dim>   &dual_solver,
                                    const Vector<double>    &primal_solution,
                                    const Vector<double>    &dual_weights);

        WeightedResidualScratchData(const WeightedResidualScratchData &scratch_data);

        CellData       cell_data;
        FaceData       face_data;
        Vector<double> primal_solution;
        Vector<double> dual_weights;
    };


    // WorkStream::run generally wants both a scratch object and a copy object.
    // Here, for reasons similar to what we had in step-9 when discussing the
    // computation of an approximation of the gradient, we don't actually
    // need a "copy data" structure. Since WorkStream insists on having one of
    // these, we just declare an empty structure that does nothing other than
    // being there.
    struct WeightedResidualCopyData
    {};



    // Regarding the evaluation of the error estimator, we have one driver
    // function that uses WorkStream::run to call the second function on every
    // cell. The concept of using SynchronousIterators was already explained
    // in step-9:
    void estimate_error (Vector<float> &error_indicators) const;

    void estimate_on_one_cell (const SynchronousIterators<std_cxx11::tuple<
                               active_cell_iterator,Vector<float>::iterator> > &cell_and_error,
                               WeightedResidualScratchData                     &scratch_data,
                               WeightedResidualCopyData                        &copy_data,
                               FaceIntegrals                                   &face_integrals) const;

    // Then we have functions that do the actual integration of the error
    // representation formula. They will treat the terms on the cell
    // interiors, on those faces that have no hanging nodes, and on those
    // faces with hanging nodes, respectively:
    void
    integrate_over_cell (const SynchronousIterators<std_cxx11::tuple<
                         active_cell_iterator,Vector<float>::iterator> > &cell_and_error,
                         const Vector<double>                            &primal_solution,
                         const Vector<double>                            &dual_weights,
                         CellData                                        &cell_data) const;

    void
    integrate_over_regular_face (const active_cell_iterator &cell,
                                 const unsigned int          face_no,
                                 const Vector<double>       &primal_solution,
                                 const Vector<double>       &dual_weights,
                                 FaceData                   &face_data,
                                 FaceIntegrals              &face_integrals) const;
    void
    integrate_over_irregular_face (const active_cell_iterator &cell,
                                   const unsigned int          face_no,
                                   const Vector<double>       &primal_solution,
                                   const Vector<double>       &dual_weights,
                                   FaceData                   &face_data,
                                   FaceIntegrals              &face_integrals) const;
};



// In the implementation of this class, we first have the constructors of
// the <code>CellData</code> and <code>FaceData</code> member classes, and
// the <code>WeightedResidual</code> constructor. They only initialize
// fields to their correct lengths, so we do not have to discuss them in
// too much detail:
template <int dim>
WeightedResidual<dim>::CellData::
CellData (const FiniteElement<dim> &fe,
          const Quadrature<dim>    &quadrature,
          const Function<dim>      &right_hand_side)
    :
      fe_values (fe, quadrature,
                 update_values   |
                 update_hessians |
                 update_quadrature_points |
                 update_JxW_values),
      right_hand_side (&right_hand_side),
      cell_residual (quadrature.size()),
      rhs_values (quadrature.size()),
      dual_weights (quadrature.size()),
      cell_laplacians (quadrature.size())
{}



template <int dim>
WeightedResidual<dim>::CellData::
CellData (const CellData &cell_data)
    :
      fe_values (cell_data.fe_values.get_fe(),
                 cell_data.fe_values.get_quadrature(),
                 update_values   |
                 update_hessians |
                 update_quadrature_points |
                 update_JxW_values),
      right_hand_side (cell_data.right_hand_side),
      cell_residual (cell_data.cell_residual),
      rhs_values (cell_data.rhs_values),
      dual_weights (cell_data.dual_weights),
      cell_laplacians (cell_data.cell_laplacians)
{}



template <int dim>
WeightedResidual<dim>::FaceData::
FaceData (const FiniteElement<dim> &fe,
          const Quadrature<dim-1>  &face_quadrature)
    :
      fe_face_values_cell (fe, face_quadrature,
                           update_values        |
                           update_gradients     |
                           update_JxW_values    |
                           update_normal_vectors),
      fe_face_values_neighbor (fe, face_quadrature,
                               update_values     |
                               update_gradients  |
                               update_JxW_values |
                               update_normal_vectors),
      fe_subface_values_cell (fe, face_quadrature,
                              update_gradients)
{
    const unsigned int n_face_q_points
            = face_quadrature.size();

    jump_residual.resize(n_face_q_points);
    dual_weights.resize(n_face_q_points);
    cell_grads.resize(n_face_q_points);
    neighbor_grads.resize(n_face_q_points);
}



template <int dim>
WeightedResidual<dim>::FaceData::
FaceData (const FaceData &face_data)
    :
      fe_face_values_cell (face_data.fe_face_values_cell.get_fe(),
                           face_data.fe_face_values_cell.get_quadrature(),
                           update_values        |
                           update_gradients     |
                           update_JxW_values    |
                           update_normal_vectors),
      fe_face_values_neighbor (face_data.fe_face_values_neighbor.get_fe(),
                               face_data.fe_face_values_neighbor.get_quadrature(),
                               update_values     |
                               update_gradients  |
                               update_JxW_values |
                               update_normal_vectors),
      fe_subface_values_cell (face_data.fe_subface_values_cell.get_fe(),
                              face_data.fe_subface_values_cell.get_quadrature(),
                              update_gradients),
      jump_residual (face_data.jump_residual),
      dual_weights (face_data.dual_weights),
      cell_grads (face_data.cell_grads),
      neighbor_grads (face_data.neighbor_grads)
{}



template <int dim>
WeightedResidual<dim>::WeightedResidualScratchData::
WeightedResidualScratchData (const PrimalSolver<dim> &primal_solver,
                             const DualSolver<dim>   &dual_solver,
                             const dealii::Vector<double> &primal_solution,
                             const dealii::Vector<double> &dual_weights)
    :
      cell_data (*dual_solver.fe,
                 *dual_solver.quadrature,
                 *primal_solver.rhs_function),
      face_data (*dual_solver.fe,
                 *dual_solver.face_quadrature),
      primal_solution(primal_solution),
      dual_weights(dual_weights)
{}

template <int dim>
WeightedResidual<dim>::WeightedResidualScratchData::
WeightedResidualScratchData (const WeightedResidualScratchData &scratch_data)
    :
      cell_data(scratch_data.cell_data),
      face_data(scratch_data.face_data),
      primal_solution(scratch_data.primal_solution),
      dual_weights(scratch_data.dual_weights)
{}
}
*/

#endif // ESTIMATORS_H

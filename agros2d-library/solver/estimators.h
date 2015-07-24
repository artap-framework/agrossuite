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
#include "solver.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/hp/dof_handler.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/hp/q_collection.h>
#include <deal.II/hp/fe_values.h>
#include <deal.II/base/synchronous_iterator.h>
#include <deal.II/base/function.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#define signals public

#ifdef _MSC_VER
#define std_get dealii::std_cxx11::get
#define std_tuple boost::tuples::tuple
#else
#define std_get std::get
#define std_tuple std::tuple
#endif

namespace ErrorEstimator
{
void estimateAdaptivitySmoothness(const dealii::hp::DoFHandler<2> &doFHandler,
                                  const dealii::Vector<double> &solution,
                                  dealii::Vector<float> &smoothness_indicators);

double relativeChangeBetweenSolutions(const dealii::hp::DoFHandler<2> &doFHandler,
                                      const dealii::hp::QCollection<2> &quadratureFormulas,
                                      const dealii::Vector<double> &sln1,
                                      const dealii::Vector<double> &sln2);
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

    static void estimate_cell(const dealii::SynchronousIterators<std_tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator,
                              dealii::Vector<float>::iterator> > &cell,
                              EstimateScratchData &scratch_data,
                              const EstimateCopyData &copy_data);
};

// ***********************************************************************************************************************

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

    static void estimate_cell(const dealii::SynchronousIterators<std_tuple<typename dealii::hp::DoFHandler<2>::active_cell_iterator, dealii::Vector<float>::iterator> > &cell,
                              EstimateScratchData &scratch_data,
                              const EstimateCopyData &copy_data);
};

// ***********************************************************************************************************************

// As with the other components of the program, we put everything we need to
// describe dual functionals into a namespace of its own, and define an
// abstract base class that provides the interface the class solving the
// dual problem needs for its work.
//
// We will then implement two such classes, for the evaluation of a point
// value and of the derivative of the solution at that point. For these
// functionals we already have the corresponding evaluation objects, so they
// are complementary.
namespace DualFunctional
{
// @sect4{The DualFunctionalBase class}

// First start with the base class for dual functionals. Since for linear
// problems the characteristics of the dual problem play a role only in
// the right hand side, we only need to provide for a function that
// assembles the right hand side for a given discretization:
template <int dim>
class DualFunctionalBase : public dealii::Subscriptor
{
public:
    virtual
    void
    assemble_rhs (const dealii::DoFHandler<dim> &dof_handler,
                  dealii::Vector<double> &rhs) const = 0;
};


// @sect4{The PointValueEvaluation class}

// As a first application, we consider the functional corresponding to the
// evaluation of the solution's value at a given point which again we
// assume to be a vertex. Apart from the constructor that takes and stores
// the evaluation point, this class consists only of the function that
// implements assembling the right hand side.
template <int dim>
class PointValueEvaluation : public DualFunctionalBase<dim>
{
public:
    PointValueEvaluation (const dealii::Point<dim> &evaluation_point)
        : evaluation_point (evaluation_point) {}

    virtual
    void
    assemble_rhs (const dealii::DoFHandler<dim> &dof_handler,
                  dealii::Vector<double>        &rhs) const
    {
        // So, first set everything to zeros...
        rhs.reinit (dof_handler.n_dofs());

        // ...then loop over cells and find the evaluation point among the
        // vertices (or very close to a vertex, which may happen due to floating
        // point round-off):
        typename dealii::DoFHandler<dim>::active_cell_iterator
                cell = dof_handler.begin_active(),
                endc = dof_handler.end();
        for (; cell!=endc; ++cell)
            for (unsigned int vertex = 0; vertex<dealii::GeometryInfo<dim>::vertices_per_cell; ++vertex)
                if (cell->vertex(vertex).distance(evaluation_point) < cell->diameter()*1e-8)
                {
                    // Ok, found, so set corresponding entry, and leave function
                    // since we are finished:
                    rhs(cell->vertex_dof_index(vertex,0)) = 1;
                    return;
                }

        // Finally, a sanity check: if we somehow got here, then we must have
        // missed the evaluation point, so raise an exception unconditionally:
        AssertThrow (false, ExcEvaluationPointNotFound(evaluation_point));
    }

    DeclException1 (ExcEvaluationPointNotFound,
                    dealii::Point<dim>,
                    << "The evaluation point " << arg1
                    << " was not found among the vertices of the present grid.");

protected:
    const dealii::Point<dim> evaluation_point;
};
}

template <int dim>
class WeightedResidual
{
public:    
    WeightedResidual (shared_ptr<SolverDeal::AssembleBase> primal_solver,
                      shared_ptr<SolverDeal::AssembleBase> dual_solver) :
        primal_solver(primal_solver),
        dual_solver(dual_solver)
      /*
          :
          Base<dim> (coarse_grid),
          PrimalSolver<dim> (coarse_grid, primal_fe,
                             quadrature, face_quadrature,
                             rhs_function, bv),
          DualSolver<dim> (coarse_grid, dual_fe,
                           quadrature, face_quadrature,
                           dual_functional)
                           */
    {}


    // Regarding the evaluation of the error estimator, we have one driver
    // function that uses WorkStream::run to call the second function on every
    // cell.
    void estimate_error (dealii::Vector<float> &error_indicators) const;

private:
    shared_ptr<SolverDeal::AssembleBase> primal_solver;
    shared_ptr<SolverDeal::AssembleBase> dual_solver;

    // Then declare abbreviations for active cell iterators, to avoid that
    // we have to write this lengthy name over and over again:

    typedef typename dealii::hp::DoFHandler<dim>::active_cell_iterator active_cell_iterator;

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
    typedef typename std::map<typename dealii::hp::DoFHandler<dim>::face_iterator, double> FaceIntegrals;

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
        dealii::hp::FEValues<dim> hp_fe_values;
        std::shared_ptr<const dealii::Function<dim> > right_hand_side;

        std::vector<double> cell_residual;
        std::vector<double> rhs_values;
        std::vector<double> dual_weights;
        std::vector<double> cell_laplacians;

        CellData (const dealii::hp::MappingCollection<2> &mappingCollection,
                  const dealii::hp::FECollection<dim> &feCollection,
                  const dealii::hp::QCollection<dim> &quadratureFormulas,
                  std::shared_ptr<const dealii::Function<dim> > right_hand_side)
            :
              hp_fe_values (mappingCollection,
                            feCollection,
                            quadratureFormulas,
                            dealii::update_values | dealii::update_hessians | dealii::update_quadrature_points | dealii::update_JxW_values),
              right_hand_side (right_hand_side),
              cell_residual (quadratureFormulas.size()),
              rhs_values (quadratureFormulas.size()),
              dual_weights (quadratureFormulas.size()),
              cell_laplacians (quadratureFormulas.size())
        {}

        CellData (const CellData &cell_data) :
            hp_fe_values (cell_data.hp_fe_values.get_mapping_collection(),
                          cell_data.hp_fe_values.get_fe_collection(),
                          cell_data.hp_fe_values.get_quadrature_collection(),
                          dealii::update_values | dealii::update_hessians | dealii::update_quadrature_points | dealii::update_JxW_values),
            right_hand_side (cell_data.right_hand_side),
            cell_residual (cell_data.cell_residual),
            rhs_values (cell_data.rhs_values),
            dual_weights (cell_data.dual_weights),
            cell_laplacians (cell_data.cell_laplacians)
        {}
    };

    struct FaceData
    {
        dealii::hp::FEFaceValues<dim> fe_face_values_cell;
        dealii::hp::FEFaceValues<dim> fe_face_values_neighbor;
        dealii::hp::FESubfaceValues<dim> fe_subface_values_cell;

        std::vector<double> jump_residual;
        std::vector<double> dual_weights;
        typename std::vector<dealii::Tensor<1,dim> > cell_grads;
        typename std::vector<dealii::Tensor<1,dim> > neighbor_grads;

        FaceData (const dealii::hp::MappingCollection<2> &mappingCollection,
                  const dealii::hp::FECollection<dim> &feCollection,
                  const dealii::hp::QCollection<dim-1> &quadratureFormulasFace)
            :
              fe_face_values_cell (mappingCollection, feCollection, quadratureFormulasFace,
                                   dealii::update_values | dealii::update_gradients | dealii::update_JxW_values | dealii::update_normal_vectors),
              fe_face_values_neighbor (mappingCollection, feCollection, quadratureFormulasFace,
                                       dealii::update_values | dealii::update_gradients | dealii::update_JxW_values | dealii::update_normal_vectors),
              fe_subface_values_cell (mappingCollection, feCollection, quadratureFormulasFace,
                                      dealii::update_gradients)
        {
            const unsigned int n_face_q_points = 100; // quadratureFormulasFace.size();

            jump_residual.resize(n_face_q_points);
            dual_weights.resize(n_face_q_points);
            cell_grads.resize(n_face_q_points);
            neighbor_grads.resize(n_face_q_points);
        }

        FaceData (const FaceData &face_data)
            :
              fe_face_values_cell (face_data.fe_face_values_cell.get_mapping_collection(),
                                   face_data.fe_face_values_cell.get_fe_collection(),
                                   face_data.fe_face_values_cell.get_quadrature_collection(),
                                   dealii::update_values | dealii::update_gradients | dealii::update_JxW_values | dealii::update_normal_vectors),
              fe_face_values_neighbor (face_data.fe_face_values_cell.get_mapping_collection(),
                                       face_data.fe_face_values_cell.get_fe_collection(),
                                       face_data.fe_face_values_cell.get_quadrature_collection(),
                                       dealii::update_values | dealii::update_gradients | dealii::update_JxW_values | dealii::update_normal_vectors),
              fe_subface_values_cell (face_data.fe_face_values_cell.get_mapping_collection(),
                                      face_data.fe_face_values_cell.get_fe_collection(),
                                      face_data.fe_face_values_cell.get_quadrature_collection(),
                                      dealii::update_gradients),
              jump_residual (face_data.jump_residual),
              dual_weights (face_data.dual_weights),
              cell_grads (face_data.cell_grads),
              neighbor_grads (face_data.neighbor_grads)
        {}
    };

    struct WeightedResidualScratchData
    {
        WeightedResidualScratchData(const shared_ptr<SolverDeal::AssembleBase> primal_solver,
                                    const shared_ptr<SolverDeal::AssembleBase> dual_solver,
                                    const dealii::Vector<double> &primal_solution,
                                    const dealii::Vector<double> &dual_weights) :
            cell_data (dual_solver->solverDeal->mappingCollection(),
                       dual_solver->solverDeal->feCollection(),
                       dual_solver->solverDeal->quadratureFormulas(),
                       std::shared_ptr<dealii::ZeroFunction<dim> >(new dealii::ZeroFunction<dim>())), // primal_solver.rhs_function), // TODO: rhs
            face_data (dual_solver->solverDeal->mappingCollection(),
                       dual_solver->solverDeal->feCollection(),
                       dual_solver->solverDeal->quadratureFormulasFace()),
            primal_solution(primal_solution),
            dual_weights(dual_weights)
        {
            qDebug() << cell_data.right_hand_side->value(dealii::Point<2>(0, 0));
        }

        WeightedResidualScratchData(const WeightedResidualScratchData &scratch_data)
            :
              cell_data(scratch_data.cell_data),
              face_data(scratch_data.face_data),
              primal_solution(scratch_data.primal_solution),
              dual_weights(scratch_data.dual_weights)
        {}

        CellData cell_data;
        FaceData face_data;
        const dealii::Vector<double> primal_solution;
        const dealii::Vector<double> dual_weights;
    };


    // WorkStream::run generally wants both a scratch object and a copy object.
    // Here, for reasons similar to what we had in step-9 when discussing the
    // computation of an approximation of the gradient, we don't actually
    // need a "copy data" structure. Since WorkStream insists on having one of
    // these, we just declare an empty structure that does nothing other than
    // being there.
    struct WeightedResidualCopyData
    {};

    void estimate_on_one_cell (const dealii::SynchronousIterators<std_tuple<active_cell_iterator, dealii::Vector<float>::iterator> > &cell_and_error,
                               WeightedResidualScratchData &scratch_data,
                               WeightedResidualCopyData &copy_data,
                               FaceIntegrals &face_integrals) const;

    // Then we have functions that do the actual integration of the error
    // representation formula. They will treat the terms on the cell
    // interiors, on those faces that have no hanging nodes, and on those
    // faces with hanging nodes, respectively:
    void
    integrate_over_cell (const dealii::SynchronousIterators<std_tuple<active_cell_iterator, dealii::Vector<float>::iterator> > &cell_and_error,
                         const dealii::Vector<double> &primal_solution,
                         const dealii::Vector<double> &dual_weights,
                         CellData &cell_data) const;

    void
    integrate_over_regular_face (const active_cell_iterator &cell,
                                 const unsigned int face_no,
                                 const dealii::Vector<double> &primal_solution,
                                 const dealii::Vector<double> &dual_weights,
                                 FaceData &face_data,
                                 FaceIntegrals &face_integrals) const;
    void
    integrate_over_irregular_face (const active_cell_iterator &cell,
                                   const unsigned int face_no,
                                   const dealii::Vector<double> &primal_solution,
                                   const dealii::Vector<double> &dual_weights,
                                   FaceData &face_data,
                                   FaceIntegrals &face_integrals) const;
};

#endif // ESTIMATORS_H

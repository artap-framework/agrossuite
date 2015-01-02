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

#include "estimators.h"
#include "solver.h"

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

        for (unsigned int i=0; i<2; ++i)
            for (unsigned int j=0; j<2; ++j)
                Y[i][j] += y[i] * y[j];

        projected_gradient += (neighbor_midpoint_value[0] - this_midpoint_value[0]) / distance * y;
    }
    AssertThrow (determinant(Y) != 0, ExcInsufficientDirections());
    const dealii::Tensor<2,2> Y_inverse = invert(Y);
    dealii::Point<2> gradient;
    contract (gradient, Y_inverse, projected_gradient);
    *(std::get<1>(cell.iterators)) = (std::pow(std::get<0>(cell.iterators)->diameter(), 2) * std::sqrt(gradient.square()));
}

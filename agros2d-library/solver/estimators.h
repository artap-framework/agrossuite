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

    void prepareGridRefinement(const FieldInfo *fieldInfo,
                               const dealii::hp::FECollection<2> &feCollection,
                               const dealii::hp::QCollection<2-1> &quadratureFormulasFace,
                               const dealii::Vector<double> &solution,
                               dealii::hp::DoFHandler<2> &doFHandler,
                               int maxHIncrease = -1,
                               int maxPIncrease = -1);
};

class GradientErrorEstimator
{
public:
    static void estimate (const dealii::hp::DoFHandler<2> &dof,
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

#endif // ESTIMATORS_H

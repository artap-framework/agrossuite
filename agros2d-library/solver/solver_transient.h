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

#ifndef SOLVER_TRANSIENT_H
#define SOLVER_TRANSIENT_H

#include "util.h"
#include "util/global.h"
#include "solutiontypes.h"
#include "scene.h"
#include "linear_solver.h"
#include "solver.h"
#include "solver_nonlinear.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/hp/dof_handler.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/hp/q_collection.h>
#include <deal.II/hp/fe_values.h>
#include <deal.II/hp/mapping_collection.h>
#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/lac/solver.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/numerics/solution_transfer.h>
#include <deal.II/base/time_stepping.h>
#define signals public

class AGROS_LIBRARY_API SolverDealTransient : public SolverDealNonlinear
{
public:
    SolverDealTransient(const FieldInfo *fieldInfo);

    virtual void solve();
    void solveTransient();

    // BDF methods
    void transientBDF(const double timeStep,
                      dealii::Vector<double> &solution,
                      const std::vector<dealii::Vector<double> > solutions,
                      const BDF2Table &bdf2Table);

    inline void set_time(const double new_time) { m_time = new_time; }
    inline double get_time() const { return m_time; }

    virtual void setup(bool useDirichletLift);

protected:
    double m_time;

    // transient mass matrix
    dealii::SparseMatrix<double> transientMassMatrix;
    dealii::SparseMatrix<double> transientTotalMatrix;
};

#endif // SOLVER_TRANSIENT_H

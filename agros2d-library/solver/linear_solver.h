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

#ifndef LINEAR_SOLVER_H
#define LINEAR_SOLVER_H

#include "util.h"
#include "util/global.h"
#include "solutiontypes.h"
#include "scene.h"

#include "tbb/tbb.h"

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

class AGROS_LIBRARY_API SolverLinearSolver
{
public:
    SolverLinearSolver(const FieldInfo *fieldInfo);

    // linear system
    void solveUMFPACK(dealii::SparseMatrix<double> &system,
                      dealii::Vector<double> &rhs,
                      dealii::Vector<double> &sln,
                      bool reuseDecomposition = false);

    void solveExternal(dealii::SparseMatrix<double> &system,
                              dealii::Vector<double> &rhs,
                              dealii::Vector<double> &sln);

    void solvedealii(dealii::SparseMatrix<double> &system,
                     dealii::Vector<double> &rhs,
                     dealii::Vector<double> &sln);

private:
    // local reference
    const FieldInfo *m_fieldInfo;

    // we need to be able to keep LU decomposition for Jacobian reuse
    dealii::SparseDirectUMFPACK direct_solver;
};

#endif // LINEAR_SOLVER_H

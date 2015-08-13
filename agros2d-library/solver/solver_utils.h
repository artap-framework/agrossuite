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

#ifndef SOLVER_UTILS_H
#define SOLVER_UTILS_H

#include "util.h"
#include "util/global.h"
#include "solutiontypes.h"
#include "scene.h"
#include "solver.h"
#include "linear_solver.h"

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

void writeMatioVector(dealii::Vector<double> &vec, const QString &name, const QString &varName);
void writeMatioMatrix(dealii::SparseMatrix<double> &mtx, const QString &name, const QString &varName);

class DoubleCellIterator
{
public:
    dealii::hp::DoFHandler<2>::active_cell_iterator cell_first, cell_second;

    DoubleCellIterator(const dealii::hp::DoFHandler<2>::active_cell_iterator &cell_first,
                       const dealii::hp::DoFHandler<2>::active_cell_iterator &cell_second,
                       const dealii::hp::DoFHandler<2> &doFHandler,
                       const FieldInfo *fieldInfo) :
        cell_first(cell_first), cell_second(cell_second), m_fieldInfo(fieldInfo), m_doFHandler(&doFHandler)
    {
    }

    DoubleCellIterator(const DoubleCellIterator &dci) :
        cell_first(dci.cell_first), cell_second(dci.cell_second), m_fieldInfo(dci.m_fieldInfo), m_doFHandler(dci.m_doFHandler)
    {
    }

    DoubleCellIterator& operator=(const DoubleCellIterator& ehi)
    {
        cell_first = ehi.cell_first;
        cell_second = ehi.cell_second;
        m_fieldInfo = ehi.m_fieldInfo;
        m_doFHandler = ehi.m_doFHandler;

        return *this;
    }

    DoubleCellIterator& operator++()
    {
        ++cell_first;
        ++cell_second;

        while (cell_second != this->m_doFHandler->end())
        {
            if (!Agros2D::scene()->labels->at(cell_second->material_id() - 1)->marker(m_fieldInfo)->isNone())
                break;
            else
            {
                ++cell_first;
                ++cell_second;
            }
        }

        return *this;
    }

    DoubleCellIterator operator++(int)
    {
        DoubleCellIterator tmp(*this); // copy
        operator++(); // pre-increment
        return tmp;   // return old value
    }
    const FieldInfo *m_fieldInfo;
    const dealii::hp::DoFHandler<2> *m_doFHandler;
};


inline bool operator==(const DoubleCellIterator& a, const DoubleCellIterator& b)
{
    // is it ok? should be () && () ??
    if((a.cell_second == b.cell_second) || (a.cell_first == b.cell_first))
    {
        assert((a.cell_second == b.cell_second) && (a.cell_first == b.cell_first));
        return true;
    }
    return false;
}

inline bool operator!=(const DoubleCellIterator& a, const DoubleCellIterator& b)
{
    return ! (a==b);
}

class SolverAgros
{
public:
    SolverAgros() : m_jacobianCalculations(0), m_phase(Phase_Undefined) {}

    enum Phase
    {
        Phase_Undefined,
        Phase_Init,
        Phase_Solving,
        Phase_DampingFactorChanged,
        Phase_JacobianReused,
        Phase_Finished
    };

    inline QVector<double> steps() const { return m_steps; }
    inline QVector<double> damping() const { return m_damping; }
    inline QVector<double> residualNorms() const { return m_residualNorms; }
    inline QVector<double> solutionNorms() const { return m_solutionNorms; }
    inline QVector<double> relativeChangeOfSolutions() const { return m_relativeChangeOfSolutions; }
    inline int jacobianCalculations() const { return m_jacobianCalculations; }

    inline Phase phase() const { return m_phase; }

    void clearSteps()
    {
        m_steps.clear();
        m_damping.clear();
        m_residualNorms.clear();
        m_solutionNorms.clear();
    }

protected:
    Phase m_phase;

    virtual void setError() = 0;

    QVector<double> m_steps;
    QVector<double> m_damping;
    QVector<double> m_residualNorms;
    QVector<double> m_solutionNorms;
    QVector<double> m_relativeChangeOfSolutions;
    int m_jacobianCalculations;
};

class AGROS_LIBRARY_API ProblemSolver
{
public:
    ProblemSolver();

    static void init();
    static void clear();
    static void solveProblem();
    static inline QMap<QString, SolverDeal *> solvers() { return m_solverDeal; }
    static inline const SolverDeal *solver(const QString &solver) { assert(m_solverDeal.contains(solver)); return m_solverDeal[solver]; }

private:
    static QMap<QString, SolverDeal *> m_solverDeal;
};

#endif // SOLVER_UTILS_H

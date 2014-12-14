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

#ifndef SOLVER_NEWTON_H
#define SOLVER_NEWTON_H

#include "util.h"
#include "solutiontypes.h"
#include "solver.h"

class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

template <typename Scalar>
class NewtonSolverAgros : public SolverAgros, public NewtonSolver<Scalar>
{
public:
    NewtonSolverAgros(Block *block);

    virtual bool on_initialization();
    virtual bool on_initial_step_end();
    virtual bool on_step_begin();
    virtual bool on_step_end();
    virtual bool on_finish();
    virtual void on_damping_factor_updated();
    virtual void on_reused_jacobian_step_end();

protected:
    virtual void setError();
};
template <typename Scalar>
class NewtonSolverContainer : public HermesSolverContainer<Scalar>
{
public:
    NewtonSolverContainer(Block* block);
    ~NewtonSolverContainer();

    virtual void solve(Scalar* previousSolutionVector);
    virtual void setMatrixRhsOutput(QString solverName, int adaptivityStep) { this->setMatrixRhsOutputGen(m_newtonSolver, solverName, adaptivityStep); }
    virtual Mixins::SettableSpaces<Scalar>* setTableSpaces() { return m_newtonSolver; }
    virtual void setWeakFormulation(WeakFormSharedPtr<Scalar> wf) { m_newtonSolver->set_weak_formulation(wf); }
    virtual LinearMatrixSolver<Scalar> *linearSolver() { return m_newtonSolver->get_linear_matrix_solver(); }

    virtual SolverAgros *solver() const { return m_newtonSolver; }

private:
    NewtonSolverAgros<Scalar> *m_newtonSolver;
};

#endif // SOLVER_NEWTON_H
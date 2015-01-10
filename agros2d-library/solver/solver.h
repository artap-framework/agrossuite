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

#ifndef SOLVER_H
#define SOLVER_H

#include "util.h"
#include "solutiontypes.h"

#undef signals
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>

#include <deal.II/hp/dof_handler.h>
#include <deal.II/hp/fe_collection.h>
#include <deal.II/hp/q_collection.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/dofs/dof_tools.h>
#include <deal.II/lac/solver.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>

#include <deal.II/base/time_stepping.h>

#define signals public

class FieldInfo;

template <typename Scalar>
class ExactSolutionScalarAgros;

class SceneBoundary;

class AGROS_LIBRARY_API SolverDeal
{
public:
    SolverDeal(const FieldInfo *fieldInfo);
    virtual ~SolverDeal();

    inline dealii::Vector<double> *solution() { return m_solution; }
    inline dealii::Triangulation<2> *triangulation() { return m_triangulation; }
    inline dealii::hp::HpDoFHandler<2> *doFHandler() { return m_doFHandler; }
    dealii::hp::FECollection<2> *feCollection() { return m_feCollection; }

    inline dealii::hp::QCollection<2> quadrature_formulas() const { return m_quadrature_formulas; }
    inline dealii::hp::QCollection<2-1> face_quadrature_formulas() const { return m_face_quadrature_formulas; }

    // if use_dirichlet_lift == false, zero dirichlet boundary condition is used
    // this is used for later iterations of the Newton method
    // this function, however, has to be called to ensure zero dirichlet boundary
    virtual void setup(bool use_dirichlet_lift);
    virtual void setupNewtonInitial();

    virtual void assembleSystem() = 0;

    virtual void assembleDirichlet(bool use_dirichlet_lift) = 0;

    // problem
    void solve();
    void solveTransientStep();

    double computeNorm();

    // transient - Runge Kutta - future step!
    void assembleMassMatrix();
    /*
    void explicitMethod(const dealii::TimeStepping::runge_kutta_method method, const unsigned int n_time_steps, const double initial_time, const double final_time);
    void implicitMethod(const dealii::TimeStepping::runge_kutta_method method, const unsigned int n_time_steps, const double initial_time, const double final_time);
    unsigned int explicitEmbeddedMethod(const dealii::TimeStepping::runge_kutta_method method, const unsigned int n_time_steps, const double initial_time, const double final_time);
    */

protected:
    const FieldInfo *m_fieldInfo;

    dealii::Triangulation<2> *m_triangulation;
    dealii::hp::HpDoFHandler<2> *m_doFHandler;
    dealii::hp::FECollection<2> *m_feCollection;

    // quadrature cache
    dealii::hp::QCollection<2> m_quadrature_formulas;
    dealii::hp::QCollection<2-1> m_face_quadrature_formulas;

    // current solution
    dealii::Vector<double> *m_solution;
    // previous solution
    dealii::Vector<double> *m_solution_previous;

    // hanging nodes and sparsity pattern
    dealii::ConstraintMatrix hanging_node_constraints;
    dealii::SparsityPattern sparsity_pattern;

    // matrix and rhs
    dealii::SparseMatrix<double> system_matrix;
    dealii::Vector<double> system_rhs;

    // transient steady and mass matrix
    dealii::SparseMatrix<double> steady_matrix;
    dealii::SparseMatrix<double> mass_matrix;

    // linear system
    void solveLinearSystem();
    void solveUMFPACK();
    void solvedealii();

    //  linearity
    void solveLinearity();
    void solveLinearityLinear();
    void solveLinearityNonLinear();
    void solveLinearityNonLinearPicard();
    void solveLinearityNonLinearNewton();

    // adaptivity
    void solveAdaptivity();
    void solveAdaptivitySimple();
    void solveAdaptivityAdaptive();

    void estimateSmoothness(dealii::Vector<float> &smoothness_indicators) const;

    // transient
    void solveTransient();


};

namespace Module {
class ErrorCalculator;
}

class SolverAgros
{
public:
    SolverAgros() : m_jacobianCalculations(0), m_phase(Phase_Undefined) {}

    enum Phase
    {
        Phase_Undefined,
        Phase_Init,
        Phase_DFDetermined,
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

    void clearSteps();

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

/*
class AgrosExternalSolverExternal : public QObject
{
    Q_OBJECT

public:
    AgrosExternalSolverExternal(CSCMatrix<double> *m, SimpleVector<double> *rhs);
    void solve();
    void solve(double* initial_guess);

    virtual void setSolverCommand() = 0;

protected:
    QProcess *m_process;

    QString command;

    QString fileMatrix;
    QString fileRHS;
    QString fileInitial;
    QString fileSln;

    double *initialGuess;

protected slots:
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode);
};

class AgrosExternalSolverMUMPS : public AgrosExternalSolverExternal
{
public:
    AgrosExternalSolverMUMPS(CSCMatrix<double> *m, SimpleVector<double> *rhs);

    virtual void setSolverCommand();
    virtual void free();
};

class AgrosExternalSolverUMFPack : public AgrosExternalSolverExternal
{
public:
    AgrosExternalSolverUMFPack(CSCMatrix<double> *m, SimpleVector<double> *rhs);

    virtual void setSolverCommand();
    virtual void free();
};
*/

// solve
class ProblemSolver
{
public:
    ProblemSolver();

    void init();
    void solveProblem();

private:
    QMap<FieldInfo *, SolverDeal *> m_solverDeal;
};

#endif // SOLVER_H

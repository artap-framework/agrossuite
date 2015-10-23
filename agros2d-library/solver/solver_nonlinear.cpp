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
#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_in.h>
#include <deal.II/grid/grid_reordering.h>
#include <deal.II/grid/grid_tools.h>
#include <deal.II/grid/tria_accessor.h>
#include <deal.II/grid/tria_iterator.h>
#include <deal.II/grid/grid_refinement.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_dgp.h>
#include <deal.II/fe/fe_nothing.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/mapping_q.h>

#include <deal.II/dofs/dof_tools.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_accessor.h>

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/multithread_info.h>

#include <deal.II/base/function.h>
#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/fe_field_function.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/solution_transfer.h>

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/compressed_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/solver_bicgstab.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/solver_richardson.h>
#include <deal.II/lac/solver_minres.h>
#include <deal.II/lac/solver_qmrs.h>
#include <deal.II/lac/solver_relaxation.h>
#include <deal.II/lac/precondition.h>

#include <streambuf>
#include <sstream>

#include "solver.h"
#include "linear_solver.h"
#include "estimators.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "field.h"
#include "problem.h"
#include "solver/problem_config.h"
//#include "module.h"
#include "coupling.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "plugin_interface.h"
#include "weak_form.h"
#include "bdf2.h"

#include "pythonlab/pythonengine.h"
#include "solver/paralution_dealii.hpp"

#include <functional>
#include <typeinfo>

// todo: is it defined somewhere?
const int MAX_NUM_NONLIN_ITERS = 100;


void AssembleNonlinear::solve()
{
    m_computation->propagateBoundaryMarkers();
    solveProblemNonLinear();
}

void AssembleNonlinear::solveProblemNonLinear()
{
    if (m_fieldInfo->linearityType() == LinearityType_Linear)
    {
        solveProblemLinear();
    }
    else if (m_fieldInfo->linearityType() == LinearityType_Picard)
    {
        solveProblemNonLinearPicard();
    }
    else if (m_fieldInfo->linearityType() == LinearityType_Newton)
    {
        solveProblemNonLinearNewton();
    }
    else
    {
        assert(0);
    }
}

void AssembleNonlinear::solveProblemNonLinearPicard()
{
    const double minAllowedDampingCoeff = 1e-4;
    const double autoDampingRatio = 2.0;

    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    setup(true);

    // initial relative change of solutions
    double lastRelChangeSol = 100.0;
    dealii::Vector<double> solutionNonlinearPrevious(doFHandler.n_dofs());

    // last solution (if exists - adaptivity)    
    /*
    FieldSolutionID solutionID(m_fieldInfo,
                               Agros2D::solutionStore()->lastTimeStep(m_fieldInfo),
                               Agros2D::solutionStore()->lastAdaptiveStep(m_fieldInfo));

    if (Agros2D::solutionStore()->contains(solutionID))
    {
        MultiArray lastSolution = Agros2D::solutionStore()->multiArray(solutionID);
        solutionNonlinearPrevious = lastSolution.solution();
    }
    */

    double dampingFactor = (m_fieldInfo->value(FieldInfo::NonlinearDampingType) == DampingType_Off ? 1.0 : m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());
    int dampingSuccessfulSteps = 0;

    int iteration = 0;
    bool criteriaReached = false;
    while ((iteration < MAX_NUM_NONLIN_ITERS) && !criteriaReached && !m_computation->isAborted())
    {
        SolverAgros::Phase phase = SolverAgros::Phase_Solving;

        iteration++;

        assembleSystem(solutionNonlinearPrevious);
        solveLinearSystem(systemMatrix, systemRHS, solution);

        // estimate error
        dealii::Vector<double> vec(solutionNonlinearPrevious);
        vec.add(-1, solution);
        double relChangeSol = vec.l2_norm() / solution.l2_norm() * 100.0;

        if ((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt() == DampingType_Automatic)
        {
            if ((lastRelChangeSol < relChangeSol) && (dampingFactor > minAllowedDampingCoeff))
            {
                phase = SolverAgros::Phase_DampingFactorChanged;

                // decrease damping
                dampingFactor = dampingFactor * 1.0 / autoDampingRatio;
                dampingSuccessfulSteps = -1;
            }
            else
            {
                dampingSuccessfulSteps++;
                // increase damping
                if (dampingSuccessfulSteps >= m_fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt())
                {
                    if (dampingFactor * 0.75 * autoDampingRatio <= m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble())
                    {
                        dampingFactor = dampingFactor * 0.75 * autoDampingRatio;
                        if (dampingFactor > 1.0)
                            dampingFactor = 1.0;
                    }
                    else
                    {
                        dampingFactor = m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble();
                    }
                }
            }
        }

        // copy solution
        solutionNonlinearPrevious.add(-dampingFactor, vec);

        // update
        steps.append(iteration);
        relativeChangeOfSolutions.append(relChangeSol);
        lastRelChangeSol = relChangeSol;

        criteriaReached = true;

        if (m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() < relChangeSol)
            criteriaReached = false;

        // log messages
        if (criteriaReached)
            phase = SolverAgros::Phase_Finished;

        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Iteration: %1 (rel. change of sol.: %2 %, damping: %3)")
                                     .arg(iteration)
                                     .arg(QString::number(relativeChangeOfSolutions.last(), 'e', 3))
                                     .arg(dampingFactor));

        Agros2D::log()->updateNonlinearChartInfo(phase, steps, relativeChangeOfSolutions);
    }

    constraintsAll.distribute(solution);

    // qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

void AssembleNonlinear::solveProblemNonLinearNewton()
{
    const double minAllowedDampingCoeff = 1e-4;
    const double autoDampingRatio = 2.0;

    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    setup(false);

    // setup nonlinear solution
    dealii::Vector<double> solutionNonlinearPrevious(doFHandler.n_dofs());
    for (dealii::types::global_dof_index dof = 0; dof < doFHandler.n_dofs(); dof++)
    {
        if (constraintsDirichlet.is_constrained(dof))
        {
            (solutionNonlinearPrevious)(dof) = constraintsDirichlet.get_inhomogeneity(dof);
        }
    }

    // first assemble just residual.
    assembleSystem(solutionNonlinearPrevious, false);
    // system_rhs.print(std::cout);
    double residualNorm = systemRHS.l2_norm();

    Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Initial residual norm: %1")
                                 .arg(residualNorm));

    // initial damping factor
    double dampingFactor = (m_fieldInfo->value(FieldInfo::NonlinearDampingType) == DampingType_Off ? 1.0 : m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());
    int dampingSuccessfulSteps = 0;

    int iteration = 0;
    int numReusedJacobian = 0;
    bool criteriaReached = false;
    while ((iteration < MAX_NUM_NONLIN_ITERS) && !criteriaReached && !m_computation->isAborted())
    {
        SolverAgros::Phase phase = SolverAgros::Phase_Solving;

        QTime time;
        time.start();

        iteration++;

        double previousResidualNorm = residualNorm;
        bool jacobianReused = false;

        if (numReusedJacobian == m_fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt())
        {
            // Jacobian has been reused too many times. Do not do it this time
            numReusedJacobian = 0;
        }
        else
        {
            // Jacobian can be reused, if it is allowed
            if((iteration > 1) && (m_fieldInfo->value(FieldInfo::NewtonReuseJacobian).toBool()))
            {
                // first try to reuse the Jacobian
                time.start();
                assembleSystem(solutionNonlinearPrevious, false);
                // std::cout << "assemble for Jac reuse (" << time.elapsed() << "ms )" << std::endl;

                systemRHS *= -1.0;

                // since m_assemble_matrix is false, this will reuse the LU decomposition
                time.start();
                solveLinearSystem(systemMatrix, systemRHS, solution, true);
                //solution.print(std::cout);
                //system_matrix.print(std::cout);
                //system_rhs.print(std::cout);

                // std::cout << "back substitution (" << time.elapsed() << "ms )" << std::endl;

                // Update
                solutionNonlinearPrevious.add(dampingFactor, solution);

                time.start();
                // calculate residual - we are not wasting time on matrix assembly.
                assembleSystem(solutionNonlinearPrevious, false);
                // Residual norm.
                residualNorm = systemRHS.l2_norm();

                // std::cout << "assemble residual (" << time.elapsed() << "ms ), norm: "  << residualNorm << std::endl;

                if (residualNorm < previousResidualNorm * m_fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble())
                {
                    jacobianReused = true;
                    numReusedJacobian++;
                }
                else
                {
                    // revert step
                    solutionNonlinearPrevious.add(-dampingFactor, solution);
                    jacobianReused = false;
                    numReusedJacobian = 0;
                }
                // std::cout << "norms: " << residualNorm << ", old: " << previousResidualNorm << " -> " << jacobianReused << std::endl;
            }
        }

        if (!jacobianReused)
        {
            time.start();
            assembleSystem(solutionNonlinearPrevious);

            time.start();
            systemRHS *= -1.0;
            solveLinearSystem(systemMatrix, systemRHS, solution);
            // solution.print(std::cout);
            // system_matrix.print(std::cout);
            // system_rhs.print(std::cout);

            // std::cout << "full system solve (" << time.elapsed() << "ms )" << std::endl;

            // Update.
            solutionNonlinearPrevious.add(dampingFactor, solution);

            // Calculate residual.
            assembleSystem(solutionNonlinearPrevious, false);
            // Residual norm.
            residualNorm = systemRHS.l2_norm();

            // automatic damping factor
            if ((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt() == DampingType_Automatic)
            {
                previousResidualNorm = residualNorm;
                assert(previousResidualNorm > 0.0);

                // todo: code repetition, get rid of it together with jacobian reuse
                time.start();
                assembleSystem(solutionNonlinearPrevious, false);
                residualNorm = systemRHS.l2_norm();
                // std::cout << "assemble residual (" << time.elapsed() << "ms ), norm: "  << residualNorm << std::endl;

                while(residualNorm > previousResidualNorm * m_fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble())
                {
                    dampingSuccessfulSteps = -1;
                    double previousDampingFactor = dampingFactor;

                    if (dampingFactor > minAllowedDampingCoeff)
                    {
                        phase = SolverAgros::Phase_DampingFactorChanged;

                        dampingFactor = dampingFactor * 1.0 / autoDampingRatio;
                    }
                    else
                    {
                        // assert(0);
                        // todo: damping factor below minimal possible
                    }

                    // Line search. Take back the previous steps (too long) and make a new one, with new damping factor
                    solutionNonlinearPrevious.add(- previousDampingFactor + dampingFactor, solution);

                    // todo: code repetition, get rid of it together with jacobian reuse
                    time.start();
                    assembleSystem(solutionNonlinearPrevious, false);
                    residualNorm = systemRHS.l2_norm();
                    // std::cout << "assemble residual (" << time.elapsed() << "ms ), norm: "  << residualNorm << std::endl;
                }

                dampingSuccessfulSteps++;
                if (dampingSuccessfulSteps > 0)
                {

                    if (dampingSuccessfulSteps >= m_fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt())
                    {
                        if (dampingFactor * autoDampingRatio <= m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble())
                            dampingFactor = dampingFactor * autoDampingRatio;
                        else
                            dampingFactor = m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble();
                    }
                }
            }
        }
        // update
        steps.append(iteration);
        double relChangeSol = dampingFactor * solution.l2_norm() / solutionNonlinearPrevious.l2_norm() * 100;
        relativeChangeOfSolutions.append(relChangeSol);

        // stop criteria
        criteriaReached = true;
        if ((m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble() > 0) &&
                (m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble() < residualNorm))
            criteriaReached = false;

        if ((m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() > 0) &&
                (m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() < relChangeSol))
            criteriaReached = false;

        // log messages
        if (criteriaReached)
            phase = SolverAgros::Phase_Finished;

        Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Iteration: %1 (rel. change of sol.: %2 %, residual: %3, damping: %4)")
                                     .arg(iteration)
                                     .arg(QString::number(relativeChangeOfSolutions.last(), 'e', 3))
                                     .arg(QString::number(residualNorm, 'e', 3))
                                     .arg(dampingFactor));

        Agros2D::log()->updateNonlinearChartInfo(phase, steps, relativeChangeOfSolutions);
    }

    // put the final solution into the solution
    solution = solutionNonlinearPrevious;

    constraintsDirichlet.distribute(solution);

    // qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

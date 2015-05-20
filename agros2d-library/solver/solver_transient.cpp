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

const int MAX_NUM_TRANSIENT_ADAPTIVE_ITERS = 20;

SolverDealTransient::SolverDealTransient(const FieldInfo *fieldInfo) : SolverDealNonlinear(fieldInfo),
    m_time(0.0)
{
}

void SolverDealTransient::setup(bool useDirichletLift)
{
    SolverDeal::setup(useDirichletLift);

    // mass matrix (transient)
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        transientMassMatrix.reinit(sparsityPattern);
        transientTotalMatrix.reinit(sparsityPattern);
    }
}

void SolverDealTransient::solveTransient()
{
    setup(true);
    assembleSystem();

    // initial condition
    m_solution = 0.0;
    dealii::Vector<double> initialSolution(m_doFHandler.n_dofs());

    dealii::VectorTools::interpolate(m_doFHandler,
                                     dealii::ConstantFunction<2>(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble()),
                                     initialSolution);

    // initial step
    // only one trasient is supported
    assert(Agros2D::problem()->timeLastStep() == 0);

    FieldSolutionID solutionID(m_fieldInfo, 0, 0);
    SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler.n_dofs());
    Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, initialSolution), runTime);
    m_solution = initialSolution;

    // parameters
    double constantTimeStep = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
    if (((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFNumSteps
            || ((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
        if (Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0)
            constantTimeStep = Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble();

    double actualTimeStep = constantTimeStep;

    // BDF table
    BDF2ATable bdf2Table;
    const double relativeTimeStepLen = actualTimeStep / Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble();
    const double maxTimeStepRatio = relativeTimeStepLen > 0.02 ? 2.0 : 3.0; // small steps may rise faster
    const double maxTimeStepLength = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / 5;
    const double maxToleranceMultiplyToAccept = 2.5;
    double tolerance = 0.0;
    TimeStepMethod timeStepMethod = (TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt();
    double nextTimeStepLength = constantTimeStep;
    double averageErrorToLenghtRatio = 0.0;

    // solution transfer
    dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(m_doFHandler);
    std::vector<dealii::Vector<double> > previousSolutions;

    // solutions and step length
    std::vector<dealii::Vector<double> > solutions;
    QList<double> stepLengths;

    int timeStep = 1;
    bool refused = false;

    while (true)
    {
        if (Agros2D::problem()->isAborted())
            break;

        if (m_time > Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() - EPS_ZERO)
            break;

        // actual time step
        actualTimeStep = nextTimeStepLength;
        // actual time
        m_time += actualTimeStep;

        // set last step
        if (m_time > Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble())
        {
            actualTimeStep = actualTimeStep - (m_time - Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble());
            m_time = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble();
        }

        // update time dep variables
        Module::updateTimeFunctions(m_time);

        // remove first solution and step length
        if (solutions.size() > Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt() - 1)
        {
            solutions.erase(solutions.begin());
            stepLengths.removeFirst();
        }
        assert(solutions.size() == stepLengths.size());

        // store sln
        solutions.push_back(m_solution);
        stepLengths.append(actualTimeStep);

        int order = std::min(timeStep, (int) solutions.size());
        // cout << "order: " << order << " solutions.size() " << solutions.size() << " stepSizes.size() " << stepLengths.size() <<  endl;

        double relChangeSol = 100.0;
        int maxSteps = (timeStep == 1) ? m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() : 1;
        for (int adaptiveStep = 0; adaptiveStep < maxSteps; adaptiveStep++)
            // int adaptiveStep = 0;
        {
            // assemble system
            assembleSystem();
            refused = false;

            dealii::Vector<double> previousSolution;
            if (m_fieldInfo->adaptivityType() != AdaptivityMethod_None)
                previousSolution = m_solution;

            if (timeStep < order || order == 1 || timeStepMethod == TimeStepMethod_Fixed)
            {
                // constant time step
                // cout << "constant step" << endl;

                if (stepLengths.size() > order - 1)
                    bdf2Table.setOrderAndPreviousSteps(order, stepLengths);
                transientBDF(actualTimeStep, m_solution, solutions, bdf2Table);

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Constant step %1/%2, time %3 s").
                                             arg(timeStep).
                                             arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                             arg(m_time));
            }
            else
            {
                // cout << "adaptive step" << endl;
                double error = 0.0;

                // estimate error
                dealii::Vector<double> estSln(m_doFHandler.n_dofs());

                // low order computation
                bdf2Table.setOrderAndPreviousSteps(order - 1, stepLengths);
                transientBDF(actualTimeStep, estSln, solutions, bdf2Table);

                // high order computation
                bdf2Table.setOrderAndPreviousSteps(order, stepLengths);
                transientBDF(actualTimeStep, m_solution, solutions, bdf2Table);

                // estimate error
                estSln.add(-1, m_solution);
                error = estSln.l2_norm() / m_solution.l2_norm();
                if (error < EPS_ZERO)
                    qDebug() << error;

                // ratio
                double actualRatio = error / actualTimeStep;
                averageErrorToLenghtRatio = ((timeStep - 2) * averageErrorToLenghtRatio + actualRatio) / (timeStep - 1);

                if (timeStepMethod == TimeStepMethod_BDFTolerance)
                {
                    tolerance = Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble() / 100.0;
                }
                else if (timeStepMethod == TimeStepMethod_BDFNumSteps)
                {
                    int desiredNumSteps = (Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->constantTimeStepLength());
                    int remainingSteps = max(2, desiredNumSteps - timeStep);
                    double desiredStepSize = (Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() - actualTimeStep) / remainingSteps;

                    tolerance = desiredStepSize * averageErrorToLenghtRatio;
                }

                // refuse
                if (error > tolerance)
                    refused = true;

                // this guess is based on assymptotic considerations
                nextTimeStepLength = pow((tolerance / maxToleranceMultiplyToAccept) / error, 1.0 / (order + 1)) * actualTimeStep;

                nextTimeStepLength = min(nextTimeStepLength, maxTimeStepLength);
                nextTimeStepLength = min(nextTimeStepLength, actualTimeStep * maxTimeStepRatio);
                nextTimeStepLength = max(nextTimeStepLength, actualTimeStep / maxTimeStepRatio);

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Adaptive step, time %1 s, rel. error %2, step size %4 -> %5 (%6 %), average err/len %7").
                                             arg(m_time).
                                             arg(error * 100.0).
                                             arg(actualTimeStep).
                                             arg(nextTimeStepLength).
                                             arg(nextTimeStepLength / actualTimeStep * 100.0).
                                             arg(averageErrorToLenghtRatio));
            }

            if (refused)
            {
                if (timeStep == 2)
                {
                    // remove all steps (initial step should be wrong)
                    // shift time
                    m_time = 0;
                    timeStep = 1;
                    stepLengths.clear();
                    solutions.clear();

                    Agros2D::problem()->removeLastTimeStepLength();
                    Agros2D::solutionStore()->removeSolution(solutionID);
                    m_solution = initialSolution;
                }
                else
                {
                    // remove last step
                    // shift time
                    m_time -= actualTimeStep;
                    stepLengths.removeLast();
                    solutions.erase(solutions.end());

                    if (solutions.size() > 0)
                        m_solution = solutions.back();
                    else
                        m_solution = initialSolution;
                }

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Transient step refused"));
            }
            else
            {
                constraintsAll.distribute(m_solution);

                Agros2D::problem()->setActualTimeStepLength(actualTimeStep);
                Agros2D::log()->updateTransientChartInfo(m_time);

                solutionID = FieldSolutionID(m_fieldInfo, timeStep, adaptiveStep);
                SolutionStore::SolutionRunTimeDetails runTime(actualTimeStep, relChangeSol, m_doFHandler.n_dofs());
                Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_solution), runTime);

                // adapt mesh
                if (m_fieldInfo->adaptivityType() != AdaptivityMethod_None)
                {
                    // compute difference between previous and current solution
                    if (adaptiveStep > 0)
                    {
                        previousSolution.add(-1, m_solution);
                        double differenceSolutionNorm = previousSolution.l2_norm();
                        double currentSolutionNorm = m_solution.l2_norm();
                        double currentRelChangeSol = fabs(differenceSolutionNorm / currentSolutionNorm) * 100.0;

                        double relativeDifference = fabs(relChangeSol - currentRelChangeSol) / currentRelChangeSol * 100.0;
                        relChangeSol = currentRelChangeSol;

                        qDebug() << timeStep << adaptiveStep << "relChangeSol" << relChangeSol << "m_doFHandler.n_dofs()" << m_doFHandler.n_dofs()
                                 << "differenceSolutionNorm" << differenceSolutionNorm << "currentSolutionNorm" << currentSolutionNorm
                                 << "relativeDifference" << relativeDifference;

                        if (differenceSolutionNorm < EPS_ZERO)
                            break;
                    }

                    Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                                 arg(1).
                                                 arg(0.0).
                                                 arg(m_doFHandler.n_dofs()));

                    if (adaptiveStep > 0)
                        Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, timeStep, adaptiveStep);

                    solutionTrans = dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(m_doFHandler);
                    previousSolutions.clear();
                    // all previous solutions
                    for (int i = 0; i < solutions.size(); i++)
                        previousSolutions.push_back(solutions[i]);
                    // add current solution
                    previousSolutions.push_back(m_solution);

                    Agros2D::problem()->calculationMesh().prepare_coarsening_and_refinement();
                    solutionTrans.prepare_for_coarsening_and_refinement(previousSolutions);

                    ErrorEstimator::prepareGridRefinement(m_fieldInfo, *m_feCollection, m_quadratureFormulasFace, m_solution, m_doFHandler, 2, 2);

                    /*
                    int min_grid_level = 1;
                    int max_grid_level = 2;
                    if (m_triangulation->n_levels() > max_grid_level)
                        for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(max_grid_level); cell != m_triangulation->end(); ++cell)
                            cell->clear_refine_flag();
                    for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(min_grid_level); cell != m_triangulation->end_active(min_grid_level); ++cell)
                        cell->clear_coarsen_flag();

                    */

                    // execute transfer solution
                    Agros2D::problem()->calculationMesh().execute_coarsening_and_refinement();

                    // reinit system
                    setup(true);

                    for (int i = 0; i < solutions.size(); i++)
                        solutions[i].reinit(m_doFHandler.n_dofs());
                    // prepare for interpolating current solution
                    solutions.push_back(dealii::Vector<double>(m_doFHandler.n_dofs()));

                    // interpolate
                    solutionTrans.interpolate(previousSolutions, solutions);

                    // store current solution
                    m_solution = solutions.back();
                    // remove interpolated solution
                    solutions.pop_back();
                }
            }
        }

        // increase step
        if (!refused)
            timeStep++;
    }
}

void SolverDealTransient::solve()
{
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        solveTransient();
    }
    else
    {
        SolverDeal::solve();
    }
}

// BDF methods
void SolverDealTransient::transientBDF(const double timeStep,
                                       dealii::Vector<double> &solution,
                                       const std::vector<dealii::Vector<double> > solutions,
                                       const BDF2Table &bdf2Table)
{
    // LHM = (M + dt * K)
    transientTotalMatrix.copy_from(transientMassMatrix);
    transientTotalMatrix *= bdf2Table.matrixFormCoefficient();
    transientTotalMatrix.add(timeStep, systemMatrix);

    // m = sum(M * SLN)
    dealii::Vector<double> m(solution.size());
    for (int i = 0; i < bdf2Table.order(); i++)
    {
        // m += M * SLNi
        dealii::Vector<double> sln(solution.size());
        transientMassMatrix.vmult(sln, solutions[solutions.size() - i - 1]);
        m.add(- bdf2Table.vectorFormCoefficient(i), sln);
    }

    // m += dt * RHS
    m.add(timeStep, systemRHS);

    solveLinearSystem(transientTotalMatrix, m, solution);
    // solveProblem();
}

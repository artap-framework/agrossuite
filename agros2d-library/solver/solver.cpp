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
#include <deal.II/fe/fe_tools.h>
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
#include "solver_utils.h"
#include "linear_solver.h"
#include "estimators.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"
#include "gui/chart.h"

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

#include <functional>
#include <typeinfo>

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// todo: find better place
// todo: what for curved elements?
const int QUADRATURE_ORDER_INCREASE = 2;

// todo: is it defined somewhere?
const int MAX_NUM_NONLIN_ITERS = 100;
const int MAX_NUM_TRANSIENT_ADAPTIVE_ITERS = 20;

tbb::mutex createCache;
tbb::mutex resizeCache;

SolverDeal::AssemblyScratchData::AssemblyScratchData(const dealii::hp::FECollection<2> &feCollection,
                                                     const dealii::hp::MappingCollection<2> &mappingCollection,
                                                     const dealii::hp::QCollection<2> &quadratureFormulas,
                                                     const dealii::hp::QCollection<2-1> &faceQuadratureFormulas,
                                                     const dealii::Vector<double> &solutionNonlinearPrevious,
                                                     bool assembleMatrix,
                                                     bool assembleRHS)
    :
      hp_fe_values(mappingCollection,
                   feCollection,
                   quadratureFormulas,
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(mappingCollection,
                        feCollection,
                        faceQuadratureFormulas,
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values),
      solutionNonlinearPrevious(solutionNonlinearPrevious),
      assembleMatrix(assembleMatrix),
      assembleRHS(assembleRHS)
{}

SolverDeal::AssemblyScratchData::AssemblyScratchData(const AssemblyScratchData &scratch_data)
    :
      hp_fe_values(scratch_data.hp_fe_values.get_mapping_collection(),
                   scratch_data.hp_fe_values.get_fe_collection(),
                   scratch_data.hp_fe_values.get_quadrature_collection(),
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(scratch_data.hp_fe_face_values.get_mapping_collection(),
                        scratch_data.hp_fe_face_values.get_fe_collection(),
                        scratch_data.hp_fe_face_values.get_quadrature_collection(),
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values),
      solutionNonlinearPrevious(scratch_data.solutionNonlinearPrevious),
      assembleMatrix(scratch_data.assembleMatrix),
      assembleRHS(scratch_data.assembleRHS)
{}


void SolverDeal::solveProblem()
{
    m_computation->propagateBoundaryMarkers();

    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
        solveTransient();
    else
        solveSteadyState();
}

SolverDeal::AssembleCache &SolverDeal::assembleCache(tbb::tbb_thread::id thread_id, int dofs_per_cell, int n_q_points)
{
    // create or resize cache
    bool idExists = !(m_assembleCache.find(thread_id) == m_assembleCache.end());

    if (!idExists || m_assembleCache[thread_id].dofs_per_cell < dofs_per_cell)
    {
        {
            tbb::mutex::scoped_lock lock(createCache);

            SolverDeal::AssembleCache cache;

            // volume value and grad cache
            cache.shape_value = std::vector<std::vector<double> >(dofs_per_cell, std::vector<double>(n_q_points));
            cache.shape_grad = std::vector<std::vector<dealii::Tensor<1,2> > >(dofs_per_cell, std::vector<dealii::Tensor<1,2> >(n_q_points));

            // surface cache
            cache.shape_face_point = std::vector<std::vector<dealii::Point<2> > >(dealii::GeometryInfo<2>::faces_per_cell);
            cache.shape_face_value = std::vector<std::vector<std::vector<double> > >(dealii::GeometryInfo<2>::faces_per_cell, std::vector<std::vector<double> >(dofs_per_cell));
            cache.shape_face_JxW = std::vector<std::vector<double> >(dealii::GeometryInfo<2>::faces_per_cell);

            // previous values and grads
            cache.solution_value_previous = std::vector<dealii::Vector<double> >(n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
            cache.solution_grad_previous = std::vector<std::vector<dealii::Tensor<1,2> > >(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

            cache.dofs_per_cell = dofs_per_cell;
            cache.n_q_points = n_q_points;

            m_assembleCache[thread_id] = cache;
            // std::cout << "init " << thread_id << std::endl;
        }
    }

    return m_assembleCache[thread_id];
}


SolverDeal::AssemblyCopyData::AssemblyCopyData()
    : isAssembled(false), cell_matrix(0), cell_mass_matrix(0), cell_rhs(0)
{}

// *****************************************************************************************************************

SolverDeal::AssembleBase::AssembleBase(Computation *computation, SolverDeal *solverDeal, dealii::Triangulation<2> &triangulation) :
    m_computation(computation),
    m_solverDeal(solverDeal),
    // dof handler
    doFHandler(triangulation),
    // linear solver
    linearSolver(SolverLinearSolver(solverDeal->m_fieldInfo)),
    m_fieldInfo(solverDeal->m_fieldInfo)
{
    // find those elements, which are used for this field
    dealii::hp::DoFHandler<2>::active_cell_iterator cell = doFHandler.begin_active(), endc = doFHandler.end();
    for (unsigned int index = 0; cell != endc; ++cell, ++index)
    {
        if (cell->active_fe_index() != 0)
            std::cout << "assert" << std::endl;

        //std::cout << "material id " << cell->material_id() - 1 << std::endl;
        if (m_computation->scene()->labels->at(cell->material_id() - 1)->marker(m_fieldInfo) != m_computation->scene()->materials->getNone(m_fieldInfo))
        {
            // set default polynomial order
            cell->set_active_fe_index(m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
        }
    }
}

void SolverDeal::AssembleBase::recreateConstraints(bool zeroDirichletLift)
{
    // this function also resolves chains of constraints (from documentation)
    constraintsHangingNodes.clear();
    dealii::DoFTools::make_hanging_node_constraints(doFHandler, constraintsHangingNodes);
    constraintsHangingNodes.close();

    // assemble Dirichlet
    // - may introduce additional constraints (but in a different entity, which will be taken care of by merging).
    // even Newton needs exact Dirichlet lift, so it is calculated always.
    constraintsDirichlet.clear();
    assembleDirichlet(true);
    constraintsDirichlet.close();

    // zero Dirichlet lift for Newton
    if (zeroDirichletLift)
    {
        constraintsZeroDirichlet.clear();
        assembleDirichlet(false);
        constraintsZeroDirichlet.close();
    }

    // merge constraints
    constraintsAll.clear();
    constraintsAll.merge(constraintsHangingNodes);
    if (zeroDirichletLift)
        constraintsAll.merge(constraintsZeroDirichlet);
    else
        constraintsAll.merge(constraintsDirichlet);
    constraintsAll.close();
}

void SolverDeal::AssembleBase::solveLinearSystem(dealii::SparseMatrix<double> &system,
                                                 dealii::Vector<double> &rhs,
                                                 dealii::Vector<double> &sln,
                                                 bool reuseDecomposition)
{
    // QTime time;
    // time.start();

    switch (m_fieldInfo->matrixSolver())
    {
    case SOLVER_UMFPACK:
        linearSolver.solveUMFPACK(system, rhs, sln, reuseDecomposition);
        break;
    case SOLVER_DEALII:
        linearSolver.solvedealii(system, rhs, sln);
        break;
    case SOLVER_EXTERNAL:
        linearSolver.solveExternal(system, rhs, sln);
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    if (Agros2D::configComputer()->value(Config::Config_LinearSystemSave).toBool())
    {
        QDateTime datetime(QDateTime::currentDateTime());
        QString tm = QString("%1").arg(datetime.toString("yyyy-MM-dd-hh-mm-ss-zzz"));

        QString matrixName = QString("%1/%2/solver-%3-%4_matrix.mat").arg(cacheProblemDir()).arg(m_computation->problemDir()).arg(m_fieldInfo->fieldId()).arg(tm);
        QString rhsName = QString("%1/%2/solver-%3-%4_rhs.mat").arg(cacheProblemDir()).arg(m_computation->problemDir()).arg(m_fieldInfo->fieldId()).arg(tm);
        QString slnName = QString("%1/%2/solver-%3-%4_sln.mat").arg(cacheProblemDir()).arg(m_computation->problemDir()).arg(m_fieldInfo->fieldId()).arg(tm);

        writeMatioMatrix(system, matrixName, "matrix");
        writeMatioVector(rhs, rhsName, "rhs");
        writeMatioVector(sln, slnName, "sln");
    }

    // qDebug() << "solved (" << time.elapsed() << "ms )";
}

void SolverDeal::AssembleBase::setup(bool useDirichletLift)
{
    QTime time;
    time.start();

    doFHandler.distribute_dofs(m_solverDeal->feCollection());
    // std::cout << "Number of degrees of freedom: " << m_doFHandler.n_dofs() << std::endl;

    // Handle hanging nodes.
    recreateConstraints(!useDirichletLift);

    // create sparsity pattern
    dealii::DynamicSparsityPattern csp(doFHandler.n_dofs(), doFHandler.n_dofs());
    dealii::DoFTools::make_sparsity_pattern(doFHandler, csp, constraintsAll);
    constraintsAll.condense(csp);
    sparsityPattern.copy_from(csp);

    // reinit system matrix
    systemMatrix.reinit(sparsityPattern);
    systemRHS.reinit(doFHandler.n_dofs());
    solution.reinit(doFHandler.n_dofs());

    // mass matrix (transient)
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        transientMassMatrix.reinit(sparsityPattern);
        transientTotalMatrix.reinit(sparsityPattern);
    }

    // qDebug() << "setup (" << time.elapsed() << "ms )";
}

// BDF methods
void SolverDeal::AssembleBase::transientBDF(const double timeStep,
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
}

void SolverDeal::AssembleBase::solve()
{
    m_computation->propagateBoundaryMarkers();
    solveProblemLinear();
}

void SolverDeal::AssembleBase::solveProblemLinear()
{
    setup(true);
    assembleSystem();

    solveLinearSystem(systemMatrix, systemRHS, solution);
    constraintsAll.distribute(solution);
}

// ***************************************************************************************************************************

SolverDeal::SolverDeal(Computation *computation, const FieldInfo *fieldInfo)
    : m_computation(computation),
      m_fieldInfo(fieldInfo),
      // fe collection
      m_feCollection(m_computation->problemSolver()->feCollection(m_fieldInfo)),
      // mapping collection
      m_mappingCollection(m_computation->problemSolver()->mappingCollection(m_fieldInfo)),
      // time
      m_time(0.0)
{
    m_quadratureFormulas.push_back(dealii::QGauss<2>(1));
    m_quadratureFormulasFace.push_back(dealii::QGauss<2 - 1>(1));

    // Gauss quadrature
    for (unsigned int degree = 1; degree <= DEALII_MAX_ORDER + 1; degree++)
    {
        m_quadratureFormulas.push_back(dealii::QGauss<2>(degree + QUADRATURE_ORDER_INCREASE));
        m_quadratureFormulasFace.push_back(dealii::QGauss<2-1>(degree + QUADRATURE_ORDER_INCREASE));
    }
}


SolverDeal::~SolverDeal()
{
    m_assembleCache.clear();
}

void SolverDeal::prepareGridRefinement(shared_ptr<SolverDeal::AssembleBase> primal,
                                       shared_ptr<SolverDeal::AssembleBase> dual,
                                       int maxHIncrease, int maxPIncrease)
{
    // estimated error per cell
    dealii::Vector<float> estimated_error_per_cell(m_computation->calculationMesh().n_active_cells());

    // estimator
    AdaptivityEstimator estimator = (AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt();

    switch (estimator)
    {
    case AdaptivityEstimator_Kelly:
    {
        dealii::KellyErrorEstimator<2>::estimate(primal->doFHandler,
                                                 m_quadratureFormulasFace,
                                                 TYPENAME dealii::FunctionMap<2>::type(),
                                                 primal->solution,
                                                 estimated_error_per_cell);
    }
        break;
    case AdaptivityEstimator_Gradient:
    {
        GradientErrorEstimator::estimate(primal->doFHandler,
                                         primal->solution,
                                         estimated_error_per_cell);
    }
        break;
    case AdaptivityEstimator_Uniform:
    {
        estimated_error_per_cell.add(1.0);
    }
        break;
    case AdaptivityEstimator_ReferenceOrder:
    case AdaptivityEstimator_ReferenceSpatialAndOrder:
    case AdaptivityEstimator_ReferenceSpatial:
    {
        // increase order
        int orderIncrease = 1;

        dealii::Triangulation<2> tria;
        tria.copy_triangulation(m_computation->calculationMesh());
        // tria.refine_global();

        dual = createAssembleBase(tria);

        std::stringstream fsDoF(std::ios::out | std::ios::in | std::ios::binary);
        boost::archive::binary_oarchive sboDoF(fsDoF);
        primal->doFHandler.save(sboDoF, 0);

        // std::cout << "Number of degrees of freedom (before load): " << dual->doFHandler.n_dofs() << std::endl;
        boost::archive::binary_iarchive sbiDoF(fsDoF);
        dual->doFHandler.load(sbiDoF, 0);
        // std::cout << "Number of degrees of freedom (after load): " << dual->doFHandler.n_dofs() << std::endl;

        dealii::hp::DoFHandler<2>::active_cell_iterator cellHandler;
        dealii::hp::DoFHandler<2>::active_cell_iterator endcHandler;
        cellHandler = dual->doFHandler.begin_active();
        endcHandler = dual->doFHandler.end();

        /*
        std::stringstream refineHistory(std::ios::out | std::ios::in | std::ios::binary);
        std::stringstream coarsenHistory(std::ios::out | std::ios::in | std::ios::binary);
        Agros2D::problem()->calculationMesh().save_refine_flags(refineHistory);
        Agros2D::problem()->calculationMesh().save_coarsen_flags(coarsenHistory);
        */
        std::cout << "Number of degrees of freedom: " << dual->doFHandler.n_dofs() << std::endl;

        for (unsigned int index = 0; cellHandler != endcHandler; ++cellHandler, ++index)
        {
            if ((estimator == AdaptivityEstimator_ReferenceOrder) || (estimator == AdaptivityEstimator_ReferenceSpatialAndOrder))
                if (cellHandler->active_fe_index() < DEALII_MAX_ORDER - 1)
                    cellHandler->set_active_fe_index(cellHandler->active_fe_index() + orderIncrease);
            if ((estimator == AdaptivityEstimator_ReferenceSpatial) || (estimator == AdaptivityEstimator_ReferenceSpatialAndOrder))
                cellHandler->set_refine_flag();
        }

        // execute refinement
        if ((estimator == AdaptivityEstimator_ReferenceSpatial) || (estimator == AdaptivityEstimator_ReferenceSpatialAndOrder))
            tria.execute_coarsening_and_refinement();

        // std::cout << "Number of degrees of freedom (before solve): " << dual->doFHandler.n_dofs() << std::endl;
        dual->solve();

        // std::cout << "Number of degrees of freedom (after solve): " << dual->doFHandler.n_dofs() << ", primal: " << primal->doFHandler.n_dofs() << std::endl;

        dealii::Vector<double> dualInterpolation(primal->doFHandler.n_dofs());
        dealii::FETools::interpolate(dual->doFHandler,
                                     dual->solution,
                                     primal->doFHandler,
                                     primal->constraintsAll,
                                     dualInterpolation);

        DifferenceErrorEstimator::estimate(primal->doFHandler,
                                           primal->solution,
                                           dualInterpolation,
                                           estimated_error_per_cell);

        // qDebug() << "estimated_error_per_cell";
        // for (int i = 0; i < estimated_error_per_cell.size(); i++)
        //     qDebug() << estimated_error_per_cell[i];
    }
        break;
    default:
        assert(0);
    }

    // cout << estimated_error_per_cell.l2_norm() << endl;

    // strategy
    switch ((AdaptivityStrategy) m_fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt())
    {
    case AdaptivityStrategy_FixedFractionOfCells:
    {
        dealii::GridRefinement::refine_and_coarsen_fixed_number(m_computation->calculationMesh(),
                                                                estimated_error_per_cell,
                                                                m_fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt() / 100.0,
                                                                m_fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt() / 100.0);
    }
        break;
    case AdaptivityStrategy_FixedFractionOfTotalError:
    {
        dealii::GridRefinement::refine_and_coarsen_fixed_fraction(m_computation->calculationMesh(),
                                                                  estimated_error_per_cell,
                                                                  m_fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt() / 100.0,
                                                                  m_fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt() / 100.0);
    }
        break;
    case AdaptivityStrategy_BalancedErrorAndCost:
    {
        dealii::GridRefinement::refine_and_coarsen_optimize(m_computation->calculationMesh(),
                                                            estimated_error_per_cell);
    }
        break;
    default:
        assert(0);
    }

    // p-adaptivity
    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_P)
    {
        dealii::hp::DoFHandler<2>::active_cell_iterator cell = primal->doFHandler.begin_active(), endc = primal->doFHandler.end();
        for (unsigned int index = 0; cell != endc; ++cell, ++index)
        {
            if (cell->refine_flag_set())
            {
                // remove h adaptivity flag
                cell->clear_refine_flag();

                if ((maxPIncrease == -1) ||
                        (cell->active_fe_index() <= m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) ||
                        ((cell->active_fe_index() > m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) && (cell->active_fe_index() - m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) <= maxPIncrease))
                {
                    if (cell->active_fe_index() + 1 < primal->doFHandler.get_fe().size())
                    {
                        // increase order
                        if (cell->active_fe_index() < DEALII_MAX_ORDER - 1)
                            cell->set_active_fe_index(cell->active_fe_index() + 1);
                    }
                }
            }

            if (cell->coarsen_flag_set())
            {
                // remove h adaptivity flag
                cell->clear_coarsen_flag();

                if ((cell->active_fe_index() > 1))
                {
                    cell->set_active_fe_index(cell->active_fe_index() - 1);
                }
            }
        }
    }

    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
    {
        switch ((AdaptivityStrategyHP) m_fieldInfo->value(FieldInfo::AdaptivityStrategyHP).toInt())
        {
        case AdaptivityStrategyHP_FourierSeries:
        {
            // tutorial 27, deal.II
            // https://www.dealii.org/developer/doxygen/deal.II/step_27.html

            // additional informations for p and hp adaptivity
            float min_smoothness = 0.0;
            float max_smoothness = 0.0;
            dealii::Vector<float> smoothnessIndicators;

            if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                smoothnessIndicators.reinit(m_computation->calculationMesh().n_active_cells());
                ErrorEstimator::estimateAdaptivitySmoothness(primal->doFHandler, primal->solution, smoothnessIndicators);

                min_smoothness = *std::max_element(smoothnessIndicators.begin(), smoothnessIndicators.end());
                max_smoothness = *std::min_element(smoothnessIndicators.begin(), smoothnessIndicators.end());
                dealii::hp::DoFHandler<2>::active_cell_iterator cellmm = primal->doFHandler.begin_active(), endcmm = primal->doFHandler.end();
                for (unsigned int index = 0; cellmm != endcmm; ++cellmm, ++index)
                {
                    if (cellmm->refine_flag_set())
                    {
                        max_smoothness = std::max(max_smoothness, smoothnessIndicators(index));
                        min_smoothness = std::min(min_smoothness, smoothnessIndicators(index));
                    }
                }
            }

            const float threshold_smoothness = (max_smoothness + min_smoothness) / 2;

            dealii::hp::DoFHandler<2>::active_cell_iterator cell = primal->doFHandler.begin_active(), endc = primal->doFHandler.end();
            for (unsigned int index = 0; cell != endc; ++cell, ++index)
            {
                if ((maxPIncrease == -1) ||
                        (cell->active_fe_index() <= m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) ||
                        ((cell->active_fe_index() > m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) && (cell->active_fe_index() - m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) <= maxPIncrease))
                {
                    if (cell->refine_flag_set() && (smoothnessIndicators(index) > threshold_smoothness)
                            && (cell->active_fe_index() + 1 < primal->doFHandler.get_fe().size()))
                    {
                        // remove h adaptivity flag
                        cell->clear_refine_flag();
                        // increase order
                        if (cell->active_fe_index() < DEALII_MAX_ORDER)
                            cell->set_active_fe_index(cell->active_fe_index() + 1);
                    }
                }

                if (cell->coarsen_flag_set())
                {
                    // remove h adaptivity flag
                    cell->clear_coarsen_flag();

                    if ((cell->active_fe_index() > 1))
                    {
                        cell->set_active_fe_index(cell->active_fe_index() - 1);
                    }
                }
            }
        }
            break;
        case AdaptivityStrategyHP_Alternate:
        {
            dealii::hp::DoFHandler<2>::active_cell_iterator cell = primal->doFHandler.begin_active(), endc = primal->doFHandler.end();
            for (unsigned int index = 0; cell != endc; ++cell, ++index)
            {
                // odd - h-adaptivity
                // do nothing

                // even - p-adaptivity
                if (cell->refine_flag_set() && (((cell->active_fe_index() + cell->level()) % 2) == 0))
                {
                    // remove h adaptivity flag
                    cell->clear_refine_flag();

                    if ((maxPIncrease == -1) ||
                            (cell->active_fe_index() <= m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) ||
                            ((cell->active_fe_index() > m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) && (cell->active_fe_index() - m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()) <= maxPIncrease))
                    {
                        if (cell->active_fe_index() + 1 < primal->doFHandler.get_fe().size())
                        {
                            // increase order
                            if (cell->active_fe_index() < DEALII_MAX_ORDER)
                                cell->set_active_fe_index(cell->active_fe_index() + 1);
                        }
                    }
                }

                if (cell->coarsen_flag_set())
                {
                    // remove h adaptivity flag
                    cell->clear_coarsen_flag();

                    if ((cell->active_fe_index() > 1))
                    {
                        cell->set_active_fe_index(cell->active_fe_index() - 1);
                    }
                }
            }
        }
            break;
        default:
            assert(0);
        }


    }

    // maximum number of refinements
    if (maxHIncrease != -1)
    {
        dealii::hp::DoFHandler<2>::active_cell_iterator cell = primal->doFHandler.begin_active(), endc = primal->doFHandler.end();
        for (unsigned int index = 0; cell != endc; ++cell, ++index)
        {
            // remove h adaptivity flag
            if (cell->level() > maxHIncrease)
            {
                cell->clear_refine_flag();
            }
        }
    }
}

void SolverDeal::solveSteadyState()
{
    shared_ptr<SolverDeal::AssembleBase> primal = createAssembleBase(m_computation->calculationMesh());
    shared_ptr<SolverDeal::AssembleBase> dual = nullptr;

    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
    {
        // no adaptivity
        primal->solve();

        FieldSolutionID solutionID(m_fieldInfo->fieldId(), m_computation->timeLastStep(), 0);
        SolutionStore::SolutionRunTimeDetails runTime;
        runTime.setValue(SolutionStore::SolutionRunTimeDetails::TimeStepLength, 0.0);
        runTime.setValue(SolutionStore::SolutionRunTimeDetails::AdaptivityError, 0.0);
        runTime.setValue(SolutionStore::SolutionRunTimeDetails::DOFs, (int) primal->doFHandler.n_dofs());
        // add solution to the store
        m_computation->solutionStore()->addSolution(solutionID, primal->doFHandler, primal->solution, runTime);
    }
    else
    {
        // h, p or hp-adaptivity
        AdaptivityEstimator estimator = (AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt();

        switch (estimator)
        {
        case AdaptivityEstimator_Kelly:
        case AdaptivityEstimator_Gradient:
        case AdaptivityEstimator_Uniform:
        {
        }
            break;
        case AdaptivityEstimator_ReferenceOrder:
        case AdaptivityEstimator_ReferenceSpatial:
        case AdaptivityEstimator_ReferenceSpatialAndOrder:
        {
            // reference solution
            dual = createAssembleBase(m_computation->calculationMesh());
        }
            break;
        default:
            assert(0);
        }

        QVector<double> adaptiveSteps;
        QVector<double> adaptiveDOFs;
        QVector<double> adaptiveError;

        // solution transfer
        dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(primal->doFHandler);
        dealii::Vector<double> previousSolution;

        for (int adaptiveStep = 0; adaptiveStep < m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); adaptiveStep++)
        {
            if (m_computation->isAborted())
                break;

            if (adaptiveStep > 0)
            {
                // prepare for transfer solution
                solutionTrans = dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(primal->doFHandler);
                previousSolution = primal->solution;

                m_computation->calculationMesh().prepare_coarsening_and_refinement();
                solutionTrans.prepare_for_coarsening_and_refinement(previousSolution);

                prepareGridRefinement(primal, dual, 2, 2);

                // execute transfer solution
                m_computation->calculationMesh().execute_coarsening_and_refinement();
            }

            // solve problem
            primal->solve();

            // error
            double relChangeSol = 100.0;
            if (adaptiveStep > 0)
            {
                // interpolate previous solution to current grid
                dealii::Vector<double> previousSolutionInterpolated(primal->solution.size());
                solutionTrans.interpolate(previousSolution, previousSolutionInterpolated);

                relChangeSol = ErrorEstimator::relativeChangeBetweenSolutions(primal->doFHandler, quadratureFormulas(),
                                                                              primal->solution,
                                                                              previousSolutionInterpolated);
            }

            FieldSolutionID solutionID(m_fieldInfo->fieldId(), m_computation->timeLastStep(), adaptiveStep);
            SolutionStore::SolutionRunTimeDetails runTime;
            runTime.setValue(SolutionStore::SolutionRunTimeDetails::TimeStepLength, 0.0);
            runTime.setValue(SolutionStore::SolutionRunTimeDetails::AdaptivityError, relChangeSol);
            runTime.setValue(SolutionStore::SolutionRunTimeDetails::DOFs, (int) primal->doFHandler.n_dofs());
            // add solution to the store
            m_computation->solutionStore()->addSolution(solutionID, primal->doFHandler, primal->solution, runTime);

            if (adaptiveStep > 0)
                Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, 0, adaptiveStep);

            Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2 %, DOFs: %3)").
                                         arg(adaptiveStep + 1).
                                         arg(relChangeSol).
                                         arg(primal->doFHandler.n_dofs()));

            // add info
            adaptiveSteps.append(adaptiveStep + 1);
            adaptiveDOFs.append(primal->doFHandler.n_dofs());
            adaptiveError.append(relChangeSol);

            // Python callback
            double cont = 1.0;
            QString command = QString("(agros2d.problem().field(\"%1\").adaptivity_callback(agros2d.computation('%2'), %3) if (agros2d.problem().field(\"%1\").adaptivity_callback is not None and hasattr(agros2d.problem().field(\"%1\").adaptivity_callback, '__call__')) else True)").
                    arg(m_fieldInfo->fieldId()).
                    arg(m_computation->problemDir()).
                    arg(adaptiveStep);
            bool successfulRun = currentPythonEngine()->runExpression(command, &cont);
            if (!successfulRun)
            {
                ErrorResult result = currentPythonEngine()->parseError();
                Agros2D::log()->printError(QObject::tr("Adaptivity callback"), result.error());
            }
            if (!cont)
                break;

            // stopping criterium
            if ((relChangeSol < m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble())
                    || (relChangeSol < EPS_ZERO))
                break;
        }

        // save chart
        ChartAdaptivityImage chart;
        chart.setError(adaptiveSteps, adaptiveError);
        chart.setDOFs(adaptiveSteps, adaptiveDOFs);
        QString fn = chart.save();

        Agros2D::log()->appendImage(fn);
    }
}

void SolverDeal::solveTransient()
{
    shared_ptr<SolverDeal::AssembleBase> primal = createAssembleBase(m_computation->calculationMesh());

    primal->setup(true);
    primal->assembleSystem();

    // initial condition
    primal->solution = 0.0;
    dealii::Vector<double> initialSolution(primal->doFHandler.n_dofs());

    dealii::VectorTools::interpolate(primal->doFHandler,
                                     dealii::ConstantFunction<2>(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble()),
                                     initialSolution);

    // initial step
    // only one trasient is supported
    assert(m_computation->timeLastStep() == 0);

    FieldSolutionID solutionID(m_fieldInfo->fieldId(), 0, 0);
    SolutionStore::SolutionRunTimeDetails runTime;
    runTime.setValue(SolutionStore::SolutionRunTimeDetails::TimeStepLength, 0.0);
    runTime.setValue(SolutionStore::SolutionRunTimeDetails::AdaptivityError, 0.0);
    runTime.setValue(SolutionStore::SolutionRunTimeDetails::DOFs, (int) primal->doFHandler.n_dofs());
    // add solution to the store
    m_computation->solutionStore()->addSolution(solutionID, primal->doFHandler, initialSolution, runTime);

    // Python callback
    /*
    double cont = 1.0;
    QString command = QString("(agros2d.problem().time_callback(agros2d.computation('%1'), %2) if (agros2d.problem().time_callback is not None and hasattr(agros2d.problem().time_callback, '__call__')) else True)").
            arg(m_computation->problemDir()).
            arg(0);
    bool successfulRun = currentPythonEngine()->runExpression(command, &cont);
    */

    primal->solution = initialSolution;

    // parameters
    double constantTimeStep = m_computation->config()->value(ProblemConfig::TimeTotal).toDouble() / m_computation->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
    if (((TimeStepMethod) m_computation->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFNumSteps
            || ((TimeStepMethod) m_computation->config()->value(ProblemConfig::TimeMethod).toInt()) == TimeStepMethod_BDFTolerance)
        if (m_computation->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0)
            constantTimeStep = m_computation->config()->value(ProblemConfig::TimeInitialStepSize).toDouble();

    double actualTimeStep = constantTimeStep;

    // BDF table
    BDF2ATable bdf2Table;
    const double relativeTimeStepLen = actualTimeStep / m_computation->config()->value(ProblemConfig::TimeTotal).toDouble();
    const double maxTimeStepRatio = relativeTimeStepLen > 0.02 ? 2.0 : 3.0; // small steps may rise faster
    const double maxTimeStepLength = m_computation->config()->value(ProblemConfig::TimeTotal).toDouble() / 5;
    const double maxToleranceMultiplyToAccept = 2.5;
    double tolerance = 0.0;
    TimeStepMethod timeStepMethod = (TimeStepMethod) m_computation->config()->value(ProblemConfig::TimeMethod).toInt();
    double nextTimeStepLength = constantTimeStep;
    double averageErrorToLenghtRatio = 0.0;

    // solution transfer
    dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(primal->doFHandler);
    std::vector<dealii::Vector<double> > previousSolutions;

    // solutions and step length
    std::vector<dealii::Vector<double> > solutions;
    QList<double> stepLengths;

    int timeStep = 1;
    bool refused = false;

    while (true)
    {
        if (m_computation->isAborted())
            break;

        if (m_time > m_computation->config()->value(ProblemConfig::TimeTotal).toDouble() - EPS_ZERO)
            break;

        // actual time step
        actualTimeStep = nextTimeStepLength;
        // actual time
        m_time += actualTimeStep;

        // set last step
        if (m_time > m_computation->config()->value(ProblemConfig::TimeTotal).toDouble())
        {
            actualTimeStep = actualTimeStep - (m_time - m_computation->config()->value(ProblemConfig::TimeTotal).toDouble());
            m_time = m_computation->config()->value(ProblemConfig::TimeTotal).toDouble();
        }

        // remove first solution and step length
        if (solutions.size() > m_computation->config()->value(ProblemConfig::TimeOrder).toInt() - 1)
        {
            solutions.erase(solutions.begin());
            stepLengths.removeFirst();
        }
        assert(solutions.size() == stepLengths.size());

        // store sln
        solutions.push_back(primal->solution);
        stepLengths.append(actualTimeStep);

        int order = std::min(timeStep, (int) solutions.size());

        double relChangeSol = 100.0;
        int maxAdaptiveSteps = ((timeStep == 1) && (m_fieldInfo->adaptivityType() != AdaptivityMethod_None)) ? m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() : 1;
        for (int adaptiveStep = 0; adaptiveStep < maxAdaptiveSteps; adaptiveStep++)
        {
            // assemble system
            primal->assembleSystem();
            refused = false;

            dealii::Vector<double> previousSolution;
            if (m_fieldInfo->adaptivityType() != AdaptivityMethod_None)
                previousSolution = primal->solution;

            if (timeStep < order || order == 1 || timeStepMethod == TimeStepMethod_Fixed)
            {
                // constant time step
                // cout << "constant step" << endl;

                if (stepLengths.size() > order - 1)
                    bdf2Table.setOrderAndPreviousSteps(order, stepLengths);
                primal->transientBDF(actualTimeStep, primal->solution, solutions, bdf2Table);

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Constant step %1/%2, time %3 s").
                                             arg(timeStep).
                                             arg(m_computation->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                             arg(m_time));
            }
            else
            {
                // cout << "adaptive step" << endl;
                double error = 0.0;

                // estimate error
                dealii::Vector<double> estSln(primal->doFHandler.n_dofs());

                // low order computation
                bdf2Table.setOrderAndPreviousSteps(order - 1, stepLengths);
                primal->transientBDF(actualTimeStep, estSln, solutions, bdf2Table);

                // high order computation
                bdf2Table.setOrderAndPreviousSteps(order, stepLengths);
                primal->transientBDF(actualTimeStep, primal->solution, solutions, bdf2Table);

                // estimate error
                estSln.add(-1, primal->solution);
                error = estSln.l2_norm() / primal->solution.l2_norm();
                if (error < EPS_ZERO)
                    qDebug() << error;

                // ratio
                double actualRatio = error / actualTimeStep;
                averageErrorToLenghtRatio = ((timeStep - 2) * averageErrorToLenghtRatio + actualRatio) / (timeStep - 1);

                if (timeStepMethod == TimeStepMethod_BDFTolerance)
                {
                    tolerance = m_computation->config()->value(ProblemConfig::TimeMethodTolerance).toDouble() / 100.0;
                }
                else if (timeStepMethod == TimeStepMethod_BDFNumSteps)
                {
                    int desiredNumSteps = (m_computation->config()->value(ProblemConfig::TimeTotal).toDouble() / m_computation->config()->constantTimeStepLength());
                    int remainingSteps = max(2, desiredNumSteps - timeStep);
                    double desiredStepSize = (m_computation->config()->value(ProblemConfig::TimeTotal).toDouble() - actualTimeStep) / remainingSteps;

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

                    m_computation->removeLastTimeStepLength();
                    m_computation->solutionStore()->removeSolution(solutionID);
                    primal->solution = initialSolution;
                }
                else
                {
                    // remove last step
                    // shift time
                    m_time -= actualTimeStep;
                    stepLengths.removeLast();
                    solutions.erase(solutions.end());

                    if (solutions.size() > 0)
                        primal->solution = solutions.back();
                    else
                        primal->solution = initialSolution;
                }

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Transient step refused"));
            }
            else
            {
                primal->constraintsAll.distribute(primal->solution);

                m_computation->setActualTimeStepLength(actualTimeStep);
                Agros2D::log()->updateTransientChartInfo(m_time);

                solutionID = FieldSolutionID(m_fieldInfo->fieldId(), timeStep, adaptiveStep);
                SolutionStore::SolutionRunTimeDetails runTime;
                runTime.setValue(SolutionStore::SolutionRunTimeDetails::TimeStepLength, actualTimeStep);
                runTime.setValue(SolutionStore::SolutionRunTimeDetails::AdaptivityError, relChangeSol);
                runTime.setValue(SolutionStore::SolutionRunTimeDetails::DOFs, (int) primal->doFHandler.n_dofs());
                // add solution to the store
                m_computation->solutionStore()->addSolution(solutionID, primal->doFHandler, primal->solution, runTime);

                // Python callback
                /*
                QString command = QString("(agros2d.problem().time_callback(agros2d.computation('%1'), %2) if (agros2d.problem().time_callback is not None and hasattr(agros2d.problem().time_callback, '__call__')) else True)").
                        arg(m_computation->problemDir()).
                        arg(timeStep);

                double cont = 1.0;
                bool successfulRun = currentPythonEngine()->runExpression(command, &cont);
                if (!successfulRun)
                {
                    ErrorResult result = currentPythonEngine()->parseError();
                    Agros2D::log()->printError(QObject::tr("Transient callback"), result.error());
                }

                if (!cont)
                    break;
                */

                // adapt mesh
                if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
                {
                    // store current solution
                    dealii::Vector<double> sln = primal->solution;

                    // reinit system
                    primal->setup(true);

                    // restore current solution
                    primal->solution = sln;
                }
                else
                {
                    // compute difference between previous and current solution
                    if (adaptiveStep > 0)
                    {
                        previousSolution.add(-1, primal->solution);
                        double differenceSolutionNorm = previousSolution.l2_norm();
                        double currentSolutionNorm = primal->solution.l2_norm();
                        double currentRelChangeSol = fabs(differenceSolutionNorm / currentSolutionNorm) * 100.0;

                        double relativeDifference = fabs(relChangeSol - currentRelChangeSol) / currentRelChangeSol * 100.0;
                        relChangeSol = currentRelChangeSol;

                        qDebug() << timeStep << adaptiveStep << "relChangeSol" << relChangeSol << "doFHandler.n_dofs()" << primal->doFHandler.n_dofs()
                                 << "differenceSolutionNorm" << differenceSolutionNorm << "currentSolutionNorm" << currentSolutionNorm
                                 << "relativeDifference" << relativeDifference;

                        if (differenceSolutionNorm < EPS_ZERO)
                            break;
                    }

                    Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                                 arg(1).
                                                 arg(0.0).
                                                 arg(primal->doFHandler.n_dofs()));

                    if (adaptiveStep > 0)
                        Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, timeStep, adaptiveStep);

                    solutionTrans = dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(primal->doFHandler);
                    previousSolutions.clear();
                    // all previous solutions
                    for (int i = 0; i < solutions.size(); i++)
                        previousSolutions.push_back(solutions[i]);
                    // add current solution
                    previousSolutions.push_back(primal->solution);

                    m_computation->calculationMesh().prepare_coarsening_and_refinement();
                    solutionTrans.prepare_for_coarsening_and_refinement(previousSolutions);

                    prepareGridRefinement(primal, nullptr, 2, 2);

                    // execute transfer solution
                    m_computation->calculationMesh().execute_coarsening_and_refinement();

                    // reinit system
                    primal->setup(true);

                    for (int i = 0; i < solutions.size(); i++)
                        solutions[i].reinit(primal->doFHandler.n_dofs());
                    // prepare for interpolating current solution
                    solutions.push_back(dealii::Vector<double>(primal->doFHandler.n_dofs()));

                    // interpolate
                    solutionTrans.interpolate(previousSolutions, solutions);

                    // store current solution
                    primal->solution = solutions.back();
                    // remove interpolated solution
                    solutions.pop_back();
                }
            }
        }

        // increase step
        if (!refused)
            timeStep++;
    }

    /*
    // time step lengths
    QList<double> timeStepLengths() const { return m_timeStepLengths; }
    // cumulative times
    QList<double> timeStepTimes() const;
      */
    // save chart
    QVector<double> transientSteps;
    for (int i = 1; i < m_computation->timeStepLengths().size() + 1; i++)
        transientSteps.append(i);

    ChartTransientImage chart;
    chart.setStepLength(transientSteps, m_computation->timeStepLengths().toVector());
    chart.setTotalTime(transientSteps, m_computation->timeStepTimes().toVector());
    QString fn = chart.save();

    Agros2D::log()->appendImage(fn);
}

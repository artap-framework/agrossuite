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

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using namespace paralution;

// todo: find better place
// todo: what for curved elements?
const int QUADRATURE_ORDER_INCREASE = 2;

// todo: is it defined somewhere?
const int MAX_NUM_NONLIN_ITERS = 100;
const int MAX_NUM_TRANSIENT_ADAPTIVE_ITERS = 20;

tbb::mutex createCache;
tbb::mutex resizeCache;

dealii::hp::FECollection<2> *SolverDeal::createFECollection(const FieldInfo *fieldInfo)
{
    dealii::hp::FECollection<2> *feCollection = new dealii::hp::FECollection<2>();

    // qDebug() << fieldInfo->name();
    QMap<int, Module::Space> spaces = fieldInfo->spaces();

    // first position of feCollection, quadrature_formulas and face_quadrature_formulas belongs to NONE space
    // this will be used for implementation of different meshes
    std::vector<const dealii::FiniteElement<2> *> fes;
    std::vector<unsigned int> multiplicities;
    foreach (int key, spaces.keys())
    {
        fes.push_back(new dealii::FE_Nothing<2>());
        multiplicities.push_back(1);
    }
    feCollection->push_back(dealii::FESystem<2>(fes, multiplicities));

    // fe collections
    for (unsigned int degree = fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        std::vector<const dealii::FiniteElement<2> *> fes;
        std::vector<unsigned int> multiplicities;

        foreach (int key, spaces.keys())
        {
            Module::Space space = spaces[key];
            if (space.type() == "h1")
                fes.push_back(new dealii::FE_Q<2>(degree + space.orderAdjust()));
            else if (spaces.value(key).type() == "l2")
                fes.push_back(new dealii::FE_Q<2>(degree + space.orderAdjust())); // fes.push_back(new dealii::FE_DGP<2>(degree + space.orderAdjust()));

            multiplicities.push_back(1);
        }

        feCollection->push_back(dealii::FESystem<2>(fes, multiplicities));
    }

    return feCollection;
}

dealii::hp::MappingCollection<2> *SolverDeal::createMappingCollection(const FieldInfo *fieldInfo)
{
    dealii::hp::MappingCollection<2> *mappingCollection = new dealii::hp::MappingCollection<2>();

    for (unsigned int degree = fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        mappingCollection->push_back(dealii::MappingQ<2>(1, true));
    }

    return mappingCollection;
}

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
      hp_fe_face_values(scratch_data.hp_fe_values.get_mapping_collection(),
                        scratch_data.hp_fe_face_values.get_fe_collection(),
                        scratch_data.hp_fe_face_values.get_quadrature_collection(),
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values),
      solutionNonlinearPrevious(scratch_data.solutionNonlinearPrevious),
      assembleMatrix(scratch_data.assembleMatrix),
      assembleRHS(scratch_data.assembleRHS)
{}

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

// ***************************************************************************************************************************

SolverDeal::SolverDeal(const FieldInfo *fieldInfo)
    : m_fieldInfo(fieldInfo),
      m_scene(Agros2D::scene()),
      m_problem(Agros2D::problem()),
      // fe collection
      m_feCollection(createFECollection(m_fieldInfo)),
      // mapping collection
      m_mappingCollection(SolverDeal::createMappingCollection(m_fieldInfo)),
      // dof handler
      m_doFHandler(Agros2D::problem()->calculationMesh()),
      // linear solver
      m_linearSolver(SolverLinearSolver(fieldInfo))
{       
    m_quadratureFormulas.push_back(dealii::QGauss<2>(1));
    m_quadratureFormulasFace.push_back(dealii::QGauss<2 - 1>(1));

    // Gauss quadrature
    for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        m_quadratureFormulas.push_back(dealii::QGauss<2>(degree +  QUADRATURE_ORDER_INCREASE));
        m_quadratureFormulasFace.push_back(dealii::QGauss<2-1>(degree + QUADRATURE_ORDER_INCREASE));
    }

    // find those elements, which are used for this field
    dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler.begin_active(), endc = m_doFHandler.end();
    for (unsigned int index = 0; cell != endc; ++cell, ++index)
    {
        if (cell->active_fe_index() != 0)
            std::cout << "assert" << std::endl;

        //std::cout << "material id " << cell->material_id() - 1 << std::endl;
        if(m_scene->labels->at(cell->material_id() - 1)->marker(m_fieldInfo) != m_scene->materials->getNone(m_fieldInfo))
        {
            cell->set_active_fe_index(1);
        }
    }
}

SolverDeal::~SolverDeal()
{
    delete m_mappingCollection;
    delete m_feCollection;
}

void SolverDeal::setup(bool useDirichletLift)
{
    QTime time;
    time.start();

    m_doFHandler.distribute_dofs(*m_feCollection);
    // std::cout << "Number of degrees of freedom: " << m_doFHandler.n_dofs() << std::endl;

    // Handle hanging nodes.
    recreateConstraints(!useDirichletLift);

    // create sparsity pattern
    dealii::CompressedSetSparsityPattern csp(m_doFHandler.n_dofs(), m_doFHandler.n_dofs());
    dealii::DoFTools::make_sparsity_pattern(m_doFHandler, csp, constraintsAll);
    constraintsAll.condense(csp);
    sparsityPattern.copy_from(csp);

    // reinit system matrix
    systemMatrix.reinit(sparsityPattern);
    systemRHS.reinit(m_doFHandler.n_dofs());
    m_solution.reinit(m_doFHandler.n_dofs());
    // qDebug() << "setup (" << time.elapsed() << "ms )";
}

void SolverDeal::solveLinearSystem(dealii::SparseMatrix<double> &system,
                                   dealii::Vector<double> &rhs,
                                   dealii::Vector<double> &sln,
                                   bool reuseDecomposition)
{
    // QTime time;
    // time.start();

    switch (m_fieldInfo->matrixSolver())
    {
    case SOLVER_UMFPACK:
        m_linearSolver.solveUMFPACK(system, rhs, sln, reuseDecomposition);
        break;
    case SOLVER_DEALII:
        m_linearSolver.solvedealii(system, rhs, sln);
        break;
    case SOLVER_PARALUTION:
    {
        if (m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONDoublePrecision).toBool())
            m_linearSolver.solvePARALUTIONDouble(system, rhs, sln, sparsityPattern);
        else
            m_linearSolver.solvePARALUTIONFloat(system, rhs, sln, sparsityPattern);
        break;
    }
    case SOLVER_EXTERNAL:
        m_linearSolver.solveExternalUMFPACK(system, rhs, sln);
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    // qDebug() << "solved (" << time.elapsed() << "ms )";
}

void SolverDeal::solve()
{
    solveAdaptivity();
}

void SolverDeal::solveProblem()
{
    Agros2D::problem()->propagateBoundaryMarkers();
}

void SolverDeal::solveProblemLinear()
{
    setup(true);
    assembleSystem();

    solveLinearSystem(systemMatrix, systemRHS, m_solution);
    constraintsAll.distribute(m_solution);
}

void SolverDeal::recreateConstraints(bool zeroDirichletLift)
{
    constraintsHangingNodes.clear();
    dealii::DoFTools::make_hanging_node_constraints(m_doFHandler, constraintsHangingNodes);
    // (from documentation) - this function also resolves chains of constraints.
    constraintsHangingNodes.close();

    // Assemble Dirichlet
    // - may introduce additional constraints (but in a different entity, which will be taken care of by merging).
    // Even Newton needs exact Dirichlet lift, so it is calculated always.
    constraintsDirichlet.clear();
    assembleDirichlet(true);
    constraintsDirichlet.close();

    // Zero Dirichlet lift for Newton
    if (zeroDirichletLift)
    {
        constraintsZeroDirichlet.clear();
        assembleDirichlet(false);
        constraintsZeroDirichlet.close();
    }

    // Merge constraints
    constraintsAll.clear();
    constraintsAll.merge(constraintsHangingNodes);
    if (zeroDirichletLift)
        constraintsAll.merge(constraintsZeroDirichlet);
    else
        constraintsAll.merge(constraintsDirichlet);
    constraintsAll.close();
}

dealii::hp::DoFHandler<2> *copyDoFHandler(dealii::hp::DoFHandler<2> &source, dealii::Triangulation<2> &tria)
{
    // TODO: create copy constructor

    // dof handler
    std::stringstream fsDoF(std::ios::out | std::ios::in | std::ios::binary);
    boost::archive::binary_oarchive sboDoF(fsDoF);
    source.save(sboDoF, 0);
    // new handler
    dealii::hp::DoFHandler<2> *cacheDoFHandler = new dealii::hp::DoFHandler<2>(tria);
    cacheDoFHandler->distribute_dofs(source.get_fe());
    // load
    boost::archive::binary_iarchive sbiDoF(fsDoF);
    cacheDoFHandler->load(sbiDoF, 0);

    return cacheDoFHandler;
}

void SolverDeal::solveAdaptivity()
{
    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
    {
        solveProblem();

        FieldSolutionID solutionID(m_fieldInfo, Agros2D::problem()->timeLastStep(), 0);
        SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler.n_dofs());
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_solution), runTime);
    }
    else
    {
        // solution transfer
        dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(m_doFHandler);
        dealii::Vector<double> previousSolution;

        for (int adaptiveStep = 0; adaptiveStep < m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); adaptiveStep++)
        {
            if (adaptiveStep > 0)
            {
                // prepare for transfer solution
                solutionTrans = dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(m_doFHandler);
                previousSolution = m_solution;

                Agros2D::problem()->calculationMesh().prepare_coarsening_and_refinement();
                solutionTrans.prepare_for_coarsening_and_refinement(previousSolution);

                ErrorEstimator::prepareGridRefinement(m_fieldInfo, *m_feCollection, m_quadratureFormulasFace, m_solution, m_doFHandler);

                // execute transfer solution
                Agros2D::problem()->calculationMesh().execute_coarsening_and_refinement();
            }

            // solve problem
            solveProblem();

            // error
            double relChangeSol = 100.0;
            if (adaptiveStep > 0)
            {
                // interpolate previous solution to current grid
                dealii::Vector<double> previousSolutionInterpolated(m_solution.size());
                solutionTrans.interpolate(previousSolution, previousSolutionInterpolated);

                // compute difference between previous and current solution
                previousSolutionInterpolated.add(-1, m_solution);
                double differenceSolutionNorm = previousSolutionInterpolated.l2_norm();
                double currentSolutionNorm = m_solution.l2_norm();
                relChangeSol = fabs(differenceSolutionNorm / currentSolutionNorm) * 100.0;

                // cout << differenceSolutionNorm << " : " << currentSolutionNorm << endl;
            }
            // cout << "error: " << error << endl;

            FieldSolutionID solutionID(m_fieldInfo, Agros2D::problem()->timeLastStep(), adaptiveStep);
            SolutionStore::SolutionRunTimeDetails runTime(0.0, relChangeSol, m_doFHandler.n_dofs());
            Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_solution), runTime);

            if (adaptiveStep > 0)
                Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, 0, adaptiveStep);

            Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2 %, DOFs: %3)").
                                         arg(adaptiveStep + 1).
                                         arg(relChangeSol).
                                         arg(m_doFHandler.n_dofs()));

            // stopping criterium
            if (relChangeSol < m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble())
                break;
        }
    }
}

// *************************************************************************************************************************************************

void SolverAgros::clearSteps()
{
    m_steps.clear();
    m_damping.clear();
    m_residualNorms.clear();
    m_solutionNorms.clear();
}

// *************************************************************************************************************************************************

QMap<FieldInfo *, SolverDeal *> ProblemSolver::m_solverDeal;

ProblemSolver::ProblemSolver()
{
}

void ProblemSolver::clear()
{
    foreach (FieldInfo *fieldInfo, m_solverDeal.keys())
        if (m_solverDeal[fieldInfo])
            delete m_solverDeal[fieldInfo];
    m_solverDeal.clear();
}

void ProblemSolver::init()
{
    clear();

    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        m_solverDeal[fieldInfo] = fieldInfo->plugin()->solverDeal(fieldInfo);
    }
}

QMap<QString, const SolverDeal *> ProblemSolver::solvers()
{
    QMap<QString, const SolverDeal *> res;
    foreach(FieldInfo* fieldInfo, m_solverDeal.keys())
    {
        res[fieldInfo->fieldId()] = m_solverDeal[fieldInfo];
    }

    return res;
}


void ProblemSolver::solveProblem()
{
    QList<FieldInfo *> fieldInfosSorted = Agros2D::problem()->fieldInfos().values();

    // sort fields (very small arrays -> sufficiently fast)
    bool swapped = false;
    do
    {
        swapped = false;
        foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos().values())
        {
            if (couplingInfo->couplingType() == CouplingType_Weak)
            {
                int sourceIndex = fieldInfosSorted.indexOf(couplingInfo->sourceField());
                int targetIndex = fieldInfosSorted.indexOf(couplingInfo->targetField());

                if (targetIndex < sourceIndex)
                {
                    fieldInfosSorted.move(sourceIndex, 0);
                    swapped = true;
                }
            }
        }
    }
    while (swapped);

    foreach (FieldInfo* targetfieldInfo, fieldInfosSorted)
    {
        // frequency
        // TODO: find some better place, where some values are initialized
        targetfieldInfo->setFrequency(Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble());

        SolverDeal *solverDeal = m_solverDeal[targetfieldInfo];

        // look for coupling sources
        foreach (FieldInfo* sourceFieldInfo, fieldInfosSorted)
        {
            if (Agros2D::problem()->hasCoupling(sourceFieldInfo, targetfieldInfo))
            {
                FieldSolutionID solutionID(sourceFieldInfo,
                                           Agros2D::solutionStore()->lastTimeStep(sourceFieldInfo),
                                           Agros2D::solutionStore()->lastAdaptiveStep(sourceFieldInfo));

                MultiArray sourceSolution = Agros2D::solutionStore()->multiArray(solutionID);

                solverDeal->setCouplingSource(sourceFieldInfo->fieldId(), sourceSolution.solution());
            }
        }

        solverDeal->solve();
    }
}

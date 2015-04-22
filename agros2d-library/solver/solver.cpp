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
      m_time(0.0),
      // fe collection
      m_feCollection(createFECollection(m_fieldInfo)),
      // mapping collection
      m_mappingCollection(SolverDeal::createMappingCollection(m_fieldInfo)),
      // dof handler
      m_doFHandler(Agros2D::problem()->calculationMesh()),
      // linear solver
      m_linearSolver(SolverLinearSolver(fieldInfo))
{       
    // calculation mesh
    m_triangulation = &Agros2D::problem()->calculationMesh();

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

void SolverDeal::estimateAdaptivitySmoothness(dealii::Vector<float> &smoothness_indicators) const
{
    const unsigned int N = 5;
    std::vector<dealii::Tensor<1,2> > k_vectors;
    std::vector<unsigned int> k_vectors_magnitude;


    for (unsigned int i=0; i<N; ++i)
        for (unsigned int j=0; j<N; ++j)
            if (!((i==0) && (j==0)) && (i*i + j*j < N*N))
            {
                k_vectors.push_back (dealii::Point<2>(M_PI * i, M_PI * j));
                k_vectors_magnitude.push_back (i*i+j*j);
            }


    const unsigned n_fourier_modes = k_vectors.size();
    std::vector<double> ln_k (n_fourier_modes);
    for (unsigned int i=0; i<n_fourier_modes; ++i)
        ln_k[i] = std::log (k_vectors[i].norm());
    std::vector<dealii::Table<2,std::complex<double> > > fourier_transform_matrices (m_feCollection->size());
    dealii::QGauss<1> base_quadrature(2);
    dealii::QIterated<2> quadrature (base_quadrature, N);
    for (unsigned int fe=0; fe<m_feCollection->size(); ++fe)
    {
        fourier_transform_matrices[fe].reinit (n_fourier_modes, (*m_feCollection)[fe].dofs_per_cell);
        for (unsigned int k=0; k<n_fourier_modes; ++k)
            for (unsigned int j=0; j<(*m_feCollection)[fe].dofs_per_cell; ++j)
            {
                std::complex<double> sum = 0;
                for (unsigned int q=0; q<quadrature.size(); ++q)
                {
                    const dealii::Point<2> x_q = quadrature.point(q);
                    sum += std::exp(std::complex<double>(0,1) * (k_vectors[k] * x_q)) * (*m_feCollection)[fe].shape_value(j,x_q) * quadrature.weight(q);
                }
                fourier_transform_matrices[fe](k,j) = sum / std::pow(2*M_PI, 1);
            }
    }
    std::vector<std::complex<double> > fourier_coefficients (n_fourier_modes);
    dealii::Vector<double> local_dof_values;
    TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler.begin_active(), endc = m_doFHandler.end();
    for (unsigned int index=0; cell!=endc; ++cell, ++index)
    {
        local_dof_values.reinit (cell->get_fe().dofs_per_cell);
        cell->get_dof_values (m_solution, local_dof_values);
        for (unsigned int f=0; f<n_fourier_modes; ++f)
        {
            fourier_coefficients[f] = 0;
            for (unsigned int i=0; i<cell->get_fe().dofs_per_cell; ++i)
                fourier_coefficients[f] += fourier_transform_matrices[cell->active_fe_index()](f,i) * local_dof_values(i);
        }
        std::map<unsigned int, double> k_to_max_U_map;
        for (unsigned int f=0; f<n_fourier_modes; ++f)
            if ((k_to_max_U_map.find (k_vectors_magnitude[f]) ==
                 k_to_max_U_map.end())
                    ||
                    (k_to_max_U_map[k_vectors_magnitude[f]] <
                     std::abs (fourier_coefficients[f])))
                k_to_max_U_map[k_vectors_magnitude[f]]
                        = std::abs (fourier_coefficients[f]);
        double sum_1 = 0,
                sum_ln_k = 0,
                sum_ln_k_square = 0,
                sum_ln_U = 0,
                sum_ln_U_ln_k = 0;
        for (unsigned int f=0; f<n_fourier_modes; ++f)
            if (k_to_max_U_map[k_vectors_magnitude[f]] ==
                    std::abs (fourier_coefficients[f]))
            {
                sum_1 += 1;
                sum_ln_k += ln_k[f];
                sum_ln_k_square += ln_k[f]*ln_k[f];
                sum_ln_U += std::log (std::abs (fourier_coefficients[f]));
                sum_ln_U_ln_k += std::log (std::abs (fourier_coefficients[f])) *
                        ln_k[f];
            }
        const double mu = (1./(sum_1*sum_ln_k_square - sum_ln_k*sum_ln_k) * (sum_ln_k*sum_ln_U - sum_1*sum_ln_U_ln_k));
        smoothness_indicators(index) = mu - 1;
    }
}

void SolverDeal::prepareGridRefinement()
{
    // estimated error per cell
    dealii::Vector<float> estimated_error_per_cell(m_triangulation->n_active_cells());

    // estimator
    switch ((AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt())
    {
    case AdaptivityEstimator_Kelly:
        dealii::KellyErrorEstimator<2>::estimate(m_doFHandler,
                                                 m_quadratureFormulasFace,
                                                 TYPENAME dealii::FunctionMap<2>::type(),
                                                 m_solution,
                                                 estimated_error_per_cell);
        break;
    case AdaptivityEstimator_Gradient:
        GradientErrorEstimator::estimate(m_doFHandler,
                                         m_solution,
                                         estimated_error_per_cell);
        break;
    default:
        assert(0);
    }

    // cout << estimated_error_per_cell.l2_norm() << endl;
    dealii::GridRefinement::refine_and_coarsen_fixed_number(*m_triangulation,
                                                            estimated_error_per_cell,
                                                            m_fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt() / 100.0,
                                                            m_fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt() / 100.0);

    // additional informations for p and hp adaptivity
    float min_smoothness = 0.0;
    float max_smoothness = 0.0;
    dealii::Vector<float> smoothnessIndicators;

    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
    {
        smoothnessIndicators.reinit(m_triangulation->n_active_cells());
        estimateAdaptivitySmoothness(smoothnessIndicators);

        min_smoothness = *std::max_element(smoothnessIndicators.begin(), smoothnessIndicators.end());
        max_smoothness = *std::min_element(smoothnessIndicators.begin(), smoothnessIndicators.end());
        dealii::hp::DoFHandler<2>::active_cell_iterator cellmm = m_doFHandler.begin_active(), endcmm = m_doFHandler.end();
        for (unsigned int index = 0; cellmm != endcmm; ++cellmm, ++index)
        {
            if (cellmm->refine_flag_set())
            {
                max_smoothness = std::max(max_smoothness, smoothnessIndicators(index));
                min_smoothness = std::min(min_smoothness, smoothnessIndicators(index));
            }
        }
    }

    if ((m_fieldInfo->adaptivityType() == AdaptivityMethod_P) || (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP))
    {
        const float threshold_smoothness = (max_smoothness + min_smoothness) / 2;

        dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler.begin_active(), endc = m_doFHandler.end();
        for (unsigned int index = 0; cell != endc; ++cell, ++index)
        {
            if (m_fieldInfo->adaptivityType() == AdaptivityMethod_P)
            {
                if (cell->refine_flag_set())
                {
                    // remove h adaptivity flag
                    cell->clear_refine_flag();

                    if (cell->active_fe_index() + 1 < m_doFHandler.get_fe().size())
                    {
                        // increase order
                        cell->set_active_fe_index(cell->active_fe_index() + 1);
                    }
                }
            }

            if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                if (cell->refine_flag_set() && (smoothnessIndicators(index) > threshold_smoothness)
                        && (cell->active_fe_index() + 1 < m_doFHandler.get_fe().size()))
                {
                    // remove h adaptivity flag
                    cell->clear_refine_flag();
                    // increase order
                    cell->set_active_fe_index(cell->active_fe_index() + 1);
                }
            }
        }
    }
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

    // mass matrix (transient)
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        transientMassMatrix.reinit(sparsityPattern);
        transientTotalMatrix.reinit(sparsityPattern);
    }
    // qDebug() << "setup (" << time.elapsed() << "ms )";
}

void SolverDeal::solveLinearSystem(dealii::SparseMatrix<double> &system,
                                   dealii::Vector<double> &rhs,
                                   dealii::Vector<double> &sln,
                                   bool reuseDecomposition)
{
    QTime time;
    time.start();

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
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
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
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_triangulation, initialSolution), runTime);
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
        dealii::Vector<double> previousSolution;

        // solutions and step length
        QList<dealii::Vector<double> > solutions;
        QList<double> stepLengths;

        int step = 1;
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
            assembleSystem();

            // interpolate solution
            if ((m_fieldInfo->adaptivityType() != AdaptivityMethod_None) && (step > 1))
            {
                if (previousSolution.size() != m_solution.size())
                    solutionTrans.interpolate(previousSolution, m_solution);
            }

            // remove first solution and step length
            if (solutions.size() > Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt() - 1)
            {
                solutions.removeFirst();
                stepLengths.removeFirst();
            }
            assert(solutions.size() == stepLengths.size());

            // store sln
            solutions.append(m_solution);
            stepLengths.append(actualTimeStep);

            int order = std::min(step, solutions.size());
            // cout << "order: " << order << " solutions.size() " << solutions.size() << " stepSizes.size() " << stepLengths.size() <<  endl;

            refused = false;

            if (step < order || order == 1 || timeStepMethod == TimeStepMethod_Fixed)
            {
                // constant time step
                // cout << "constant step" << endl;

                if (stepLengths.size() > order - 1)
                    bdf2Table.setOrderAndPreviousSteps(order, stepLengths);
                transientBDF(actualTimeStep, m_solution, solutions, bdf2Table);

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Constant step %1/%2, time %3 s").
                                             arg(step).
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
                averageErrorToLenghtRatio = ((step - 2) * averageErrorToLenghtRatio + actualRatio) / (step - 1);

                if (timeStepMethod == TimeStepMethod_BDFTolerance)
                {
                    tolerance = Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble() / 100.0;
                }
                else if (timeStepMethod == TimeStepMethod_BDFNumSteps)
                {
                    int desiredNumSteps = (Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->constantTimeStepLength());
                    int remainingSteps = max(2, desiredNumSteps - step);
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
                if (step == 2)
                {
                    // remove all steps (initial step should be wrong)
                    // shift time
                    m_time = 0;
                    step = 1;
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
                    solutions.removeLast();

                    if (solutions.size() > 0)
                        m_solution = solutions.last();
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

                solutionID = FieldSolutionID(m_fieldInfo, step, 0);
                SolutionStore::SolutionRunTimeDetails runTime(actualTimeStep, 0.0, m_doFHandler.n_dofs());
                Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_triangulation, m_solution), runTime);

                // increase step
                step++;

                // adapt mesh
                if (m_fieldInfo->adaptivityType() != AdaptivityMethod_None)
                {
                    // Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, step, 1);
                    Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                                 arg(1).
                                                 arg(0.0).
                                                 arg(m_doFHandler.n_dofs()));

                    constraintsAll.distribute(m_solution);

                    // prepare for transfer solution
                    solutionTrans = dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(m_doFHandler);
                    previousSolution = m_solution;

                    m_triangulation->prepare_coarsening_and_refinement();
                    solutionTrans.prepare_for_coarsening_and_refinement(previousSolution);

                    prepareGridRefinement();

                    int min_grid_level = 1;
                    int max_grid_level = 2;

                    if (m_triangulation->n_levels() > max_grid_level)
                        for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(max_grid_level); cell != m_triangulation->end(); ++cell)
                            cell->clear_refine_flag();
                    for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(min_grid_level); cell != m_triangulation->end_active(min_grid_level); ++cell)
                        cell->clear_coarsen_flag();


                    // execute transfer solution
                    m_triangulation->execute_coarsening_and_refinement();

                    // reinit system
                    setup(true);
                }
            }
        }
    }
    else
    {
        solveAdaptivity();
    }
}

void SolverDeal::solveProblem()
{
    // this is a little bit inconsistent. Each solver has pointer to triangulation, but they actually point to mesh object of Agros2D::problem()
    Agros2D::problem()->propagateBoundaryMarkers();

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

void SolverDeal::solveProblemLinear()
{
    setup(true);
    assembleSystem();

    solveLinearSystem(systemMatrix, systemRHS, m_solution);
    constraintsAll.distribute(m_solution);
}

void SolverDeal::solveProblemNonLinearPicard()
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
    dealii::Vector<double> solutionNonlinearPrevious(m_doFHandler.n_dofs());

    double dampingFactor = (m_fieldInfo->value(FieldInfo::NonlinearDampingType) == DampingType_Off ? 1.0 : m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());
    int dampingSuccessfulSteps = 0;

    int iteration = 0;
    bool criteriaReached = false;
    while ((iteration < MAX_NUM_NONLIN_ITERS) && !criteriaReached && !Agros2D::problem()->isAborted())
    {
        SolverAgros::Phase phase = SolverAgros::Phase_Solving;

        iteration++;

        assembleSystem(solutionNonlinearPrevious);
        solveLinearSystem(systemMatrix, systemRHS, m_solution);

        // estimate error
        dealii::Vector<double> vec(solutionNonlinearPrevious);
        vec.add(-1, m_solution);
        double relChangeSol = vec.l2_norm() / m_solution.l2_norm() * 100.0;

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

    constraintsAll.distribute(m_solution);

    // qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
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

void SolverDeal::solveProblemNonLinearNewton()
{
    const double minAllowedDampingCoeff = 1e-4;
    const double autoDampingRatio = 2.0;

    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    // decide, how the adaptivity, nonlinear (and time ) steps will be organized
    setup(false);

    // setup nonlinear solution
    dealii::Vector<double> solutionNonlinearPrevious(m_doFHandler.n_dofs());
    for (dealii::types::global_dof_index dof = 0; dof < m_doFHandler.n_dofs(); dof++)
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
    while ((iteration < MAX_NUM_NONLIN_ITERS) && !criteriaReached && !Agros2D::problem()->isAborted())
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
                solveLinearSystem(systemMatrix, systemRHS, m_solution, true);
                //m_solution.print(std::cout);
                //system_matrix.print(std::cout);
                //system_rhs.print(std::cout);

                // std::cout << "back substitution (" << time.elapsed() << "ms )" << std::endl;

                // Update
                solutionNonlinearPrevious.add(dampingFactor, m_solution);

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
                    solutionNonlinearPrevious.add(-dampingFactor, m_solution);
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
            solveLinearSystem(systemMatrix, systemRHS, m_solution);
            // m_solution.print(std::cout);
            // system_matrix.print(std::cout);
            // system_rhs.print(std::cout);

            // std::cout << "full system solve (" << time.elapsed() << "ms )" << std::endl;

            // Update.
            solutionNonlinearPrevious.add(dampingFactor, m_solution);

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
                    solutionNonlinearPrevious.add(- previousDampingFactor + dampingFactor, m_solution);

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
        double relChangeSol = dampingFactor * m_solution.l2_norm() / solutionNonlinearPrevious.l2_norm() * 100;
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
    m_solution = solutionNonlinearPrevious;

    constraintsDirichlet.distribute(m_solution);

    // qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

void SolverDeal::solveAdaptivity()
{
    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
    {
        solveProblem();

        FieldSolutionID solutionID(m_fieldInfo, Agros2D::problem()->timeLastStep(), 0);
        SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler.n_dofs());
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_triangulation, m_solution), runTime);
    }
    else
    {
        // solution transfer
        dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(m_doFHandler);
        dealii::Vector<double> previousSolution;

        // double previousNorm = 0.0;
        for (int i = 0; i < m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); i++)
        {
            if (i > 0)
            {
                // prepare for transfer solution
                solutionTrans = dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(m_doFHandler);
                previousSolution = m_solution;

                m_triangulation->prepare_coarsening_and_refinement();
                solutionTrans.prepare_for_coarsening_and_refinement(previousSolution);

                prepareGridRefinement();

                // execute transfer solution
                m_triangulation->execute_coarsening_and_refinement();
            }

            // solve problem
            solveProblem();

            // error
            double relChangeSol = 100.0;
            if (i > 0)
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

            FieldSolutionID solutionID(m_fieldInfo, Agros2D::problem()->timeLastStep(), i);
            SolutionStore::SolutionRunTimeDetails runTime(0.0, relChangeSol, m_doFHandler.n_dofs());
            Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_triangulation, m_solution), runTime);

            if (i > 0)
                Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, 0, i);

            Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2 %, DOFs: %3)").
                                         arg(i + 1).
                                         arg(relChangeSol).
                                         arg(m_doFHandler.n_dofs()));

            // stopping criterium
            if (relChangeSol < m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble())
                break;
        }
    }
}

// BDF methods
void SolverDeal::transientBDF(const double timeStep,
                              dealii::Vector<double> &solution,
                              const QList<dealii::Vector<double> > solutions,
                              const BDF2Table &bdf2Table,
                              bool spatialAdaptivity)
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

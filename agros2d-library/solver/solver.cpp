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
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>

#include <deal.II/dofs/dof_tools.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_accessor.h>

#include <deal.II/base/quadrature_lib.h>

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
#include "estimators.h"
// #include "solver_linear.h"
// #include "solver_newton.h"
// #include "solver_picard.h"

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
#include "bdf2.h"
#include "plugin_interface.h"
#include "weak_form.h"

#include "pythonlab/pythonengine.h"

#include <functional>

// todo: find better place
// todo: what for curved elements?
const int QUADRATURE_ORDER_INCREASE = 1;

// todo: is it defined somewhere?
const int MAX_NUM_NONLIN_ITERS = 100;

dealii::hp::FECollection<2> *SolverDeal::createFECollection(const FieldInfo *fieldInfo)
{
    dealii::hp::FECollection<2> *feCollection = new dealii::hp::FECollection<2>();

    // Gauss quadrature and fe collection
    for (unsigned int degree = fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        /*
        std::vector<const dealii::FiniteElement<2> *> fes;
        std::vector<unsigned int> multiplicities;

        QMap<int, Module::Space> spaces = m_fieldInfo->spaces();
        foreach (int key, spaces.keys())
        {
            dealii::FE_Q<2> *fe = new dealii::FE_Q<2>(degree + spaces[key].orderAdjust());
            fes.push_back(fe);
            multiplicities.push_back(1);
        }
        */
        feCollection->push_back(dealii::FESystem<2>(dealii::FE_Q<2>(degree), fieldInfo->numberOfSolutions()));
        // m_feCollection->push_back(dealii::FESystem<2>(fes, multiplicities));
    }

    return feCollection;
}

SolverDeal::SolverDeal(const FieldInfo *fieldInfo)
    : m_fieldInfo(fieldInfo), m_scene(Agros2D::scene()), m_problem(Agros2D::problem()), m_solution_previous(NULL), m_time(0.0)
{    
    // fe collection
    qDebug() << "SolverDeal::SolverDeal: numberOfSolutions" << fieldInfo->numberOfSolutions();

    // copy initial mesh
    m_triangulation = new dealii::Triangulation<2>();
    m_triangulation->copy_triangulation(*m_fieldInfo->initialMesh());

    // create dof handler
    m_doFHandler = new dealii::hp::DoFHandler<2>(*m_triangulation);

    // create solution vector
    m_solution = new dealii::Vector<double>();

<<<<<<< HEAD
    // fe collection
    m_feCollection = SolverDeal::createFECollection(m_fieldInfo);
=======
    // this will be set to false for Jacobian reuse
    m_assemble_matrix = true;

    // Gauss quadrature and fe collection
    for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        /*
        std::vector<const dealii::FiniteElement<2> *> fes;
        std::vector<unsigned int> multiplicities;
>>>>>>> Implicit time methods work using DEAL, previsously it was only hand-made implicit Euler.

    for (unsigned int degree = fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        m_quadrature_formulas.push_back(dealii::QGauss<2>(degree + QUADRATURE_ORDER_INCREASE));
        m_face_quadrature_formulas.push_back(dealii::QGauss<2-1>(degree + QUADRATURE_ORDER_INCREASE));
    }
}

SolverDeal::~SolverDeal()
{
    if (m_triangulation)
        delete m_triangulation;
    m_triangulation = nullptr;

    if (m_doFHandler)
        delete m_doFHandler;
    m_doFHandler = nullptr;

    if (m_solution)
        delete m_solution;
    m_solution = nullptr;

    if (m_feCollection)
        delete m_feCollection;
    m_feCollection = nullptr;
}

void SolverDeal::solveLinearSystem()
{
    QTime time;
    time.start();

    switch (m_fieldInfo->matrixSolver())
    {
    case SOLVER_UMFPACK:
        solveUMFPACK();
        break;
    case SOLVER_DEALII:
        solvedealii();
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    hanging_node_constraints.distribute(*m_solution);

    qDebug() << "solved (" << time.elapsed() << "ms )";
}

void SolverDeal::solveUMFPACK()
{
    dealii::SparseDirectUMFPACK direct;
    direct.initialize(system_matrix);
    direct.vmult(*m_solution, system_rhs);
}

void SolverDeal::solvedealii()
{
    // preconditioner
    dealii::PreconditionSSOR<> preconditioner;

    switch ((PreconditionerType) m_fieldInfo->value(FieldInfo::LinearSolverIterPreconditioner).toInt())
    {
    case PreconditionerType_SSOR:
    {
        // TODO:
        preconditioner.initialize(system_matrix, 1.2);
    }
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Preconditioner '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    // solver control
    dealii::SolverControl solver_control(m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt(),
                                         m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());

    switch ((IterSolverType) m_fieldInfo->value(FieldInfo::LinearSolverIterMethod).toInt())
    {
    case IterSolverType_CG:
    {
        dealii::SolverCG<> solver(solver_control);
        solver.solve(system_matrix, *m_solution, system_rhs, preconditioner);
    }
        break;
    case IterSolverType_BiCGStab:
    {
        dealii::SolverBicgstab<> solver(solver_control);
        solver.solve(system_matrix, *m_solution, system_rhs, preconditioner);
    }
        break;
    case IterSolverType_GMRES:
    {
        dealii::SolverGMRES<> solver(solver_control);
        solver.solve(system_matrix, *m_solution, system_rhs, preconditioner);
    }
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver method '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }
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
    TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler->begin_active(), endc = m_doFHandler->end();
    for (unsigned int index=0; cell!=endc; ++cell, ++index)
    {
        local_dof_values.reinit (cell->get_fe().dofs_per_cell);
        cell->get_dof_values (*m_solution, local_dof_values);
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

void SolverDeal::refineGrid(bool refine)
{
    // estimated error per cell
    dealii::Vector<float> estimated_error_per_cell(m_triangulation->n_active_cells());

    // estimator
    switch ((AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt())
    {
    case AdaptivityEstimator_Kelly:
        dealii::KellyErrorEstimator<2>::estimate(*m_doFHandler,
                                                 m_face_quadrature_formulas,
                                                 TYPENAME dealii::FunctionMap<2>::type(),
                                                 *m_solution,
                                                 estimated_error_per_cell);
        break;
    case AdaptivityEstimator_Gradient:
        GradientErrorEstimator::estimate(*m_doFHandler,
                                         *m_solution,
                                         estimated_error_per_cell);
        break;
    default:
        assert(0);
    }

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
        dealii::hp::DoFHandler<2>::active_cell_iterator cellmm = m_doFHandler->begin_active(), endcmm = m_doFHandler->end();
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

        dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler->begin_active(), endc = m_doFHandler->end();
        for (unsigned int index = 0; cell != endc; ++cell, ++index)
        {
            if (m_fieldInfo->adaptivityType() == AdaptivityMethod_P)
            {
                if (cell->refine_flag_set())
                {
                    // remove h adaptivity flag
                    cell->clear_refine_flag();

                    if (cell->active_fe_index() + 1 < m_doFHandler->get_fe().size())
                    {
                        // increase order
                        cell->set_active_fe_index(cell->active_fe_index() + 1);
                    }
                }
            }

            if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                if (cell->refine_flag_set() && (smoothnessIndicators(index) > threshold_smoothness)
                        && (cell->active_fe_index() + 1 < m_doFHandler->get_fe().size()))
                {
                    // remove h adaptivity flag
                    cell->clear_refine_flag();
                    // increase order
                    cell->set_active_fe_index(cell->active_fe_index() + 1);
                }
            }
        }
    }

    if (refine)
        m_triangulation->execute_coarsening_and_refinement();
}

double SolverDeal::computeNorm()
{
    double h1Norm = 0.0;

    dealii::hp::FEValues<2> hp_fe_values(*m_feCollection, m_quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_JxW_values);

    dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = m_doFHandler->begin_active(), endc_int = m_doFHandler->end();
    for (; cell_int != endc_int; ++cell_int)
    {
        // volume integration
        hp_fe_values.reinit(cell_int);

        const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
        const unsigned int n_q_points = fe_values.n_quadrature_points;

        std::vector<dealii::Vector<double> > solution_values(n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
        std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

        fe_values.get_function_values(*m_solution, solution_values);
        fe_values.get_function_gradients(*m_solution, solution_grads);

        // expressions
        for (unsigned int k = 0; k < n_q_points; ++k)
        {
            // H1-norm
            h1Norm += fe_values.JxW(k) * (solution_values[k][0]*solution_values[k][0] + (solution_grads[k][0][0]*solution_grads[k][0][0] + solution_grads[k][0][1]*solution_grads[k][0][1]));
        }
    }

    return h1Norm;
}

void SolverDeal::setup(bool useDirichletLift)
{
    QTime time;
    time.start();

    m_doFHandler->distribute_dofs(*m_feCollection);
    // std::cout << "Number of degrees of freedom: " << m_doFHandler->n_dofs() << std::endl;

    // reinit sln and rhs
    system_rhs.reinit(m_doFHandler->n_dofs());
    m_solution->reinit(m_doFHandler->n_dofs());

    hanging_node_constraints.clear();
    dealii::DoFTools::make_hanging_node_constraints(*m_doFHandler, hanging_node_constraints);

    // assemble Dirichlet
    assembleDirichlet(useDirichletLift);

    hanging_node_constraints.close();

    // create sparsity pattern
    dealii::CompressedSetSparsityPattern csp(m_doFHandler->n_dofs(), m_doFHandler->n_dofs());
    dealii::DoFTools::make_sparsity_pattern(*m_doFHandler, csp, hanging_node_constraints); // , false
    sparsity_pattern.copy_from(csp);

    // reinit system matrix
    system_matrix.reinit(sparsity_pattern);

    // mass matrix (transient)
    if (m_fieldInfo->hasTransientAnalysis() && m_fieldInfo->value(FieldInfo::TransientAnalysis).toBool())
    {
        mass_matrix.reinit(sparsity_pattern);
        mass_minus_tau_Jacobian.reinit(sparsity_pattern);
    }
    // qDebug() << "setup (" << time.elapsed() << "ms )";
}

void SolverDeal::setupProblemNonLinearNewton()
{
    m_doFHandler->distribute_dofs(*m_feCollection);

    // reinit sln
    if (!m_solution_previous)
        m_solution_previous = new dealii::Vector<double>();

    m_solution_previous->reinit(m_doFHandler->n_dofs());

    hanging_node_constraints.clear();
    dealii::DoFTools::make_hanging_node_constraints(*m_doFHandler, hanging_node_constraints);

    // assemble Dirichlet
    assembleDirichlet(true);

    hanging_node_constraints.close();

    // todo: this has to be verified
    // I hope, that it will construct dirichlet lift, taking into account hanging nodes close to the boundary,
    // since to the hanging_node_constraints, first are applied hanging nodes and than the Dirichlet BC
    // todo: is that correct?
    for(dealii::types::global_dof_index dof = 0; dof < m_doFHandler->n_dofs(); dof++)
    {
        if (hanging_node_constraints.is_constrained(dof))
        {
            // first consider only BC, not hanging nodes
            // todo: extend
            assert(hanging_node_constraints.get_constraint_entries(dof)->size() == 0);
            //            const std::vector<std::pair<dealii::types::global_dof_index,double> > * constraints = hanging_node_constraints.get_constraint_entries(dof);
            //            if(constraints != nullptr)
            //            {
            //                for(int i = 0; i < constraints->size(); i++)
            //                {
            //                    std::cout << "(" << (*constraints)[i].first << ", " << (*constraints)[i].second << "), ";
            //                }
            //                std::cout << std::endl;
            //            }

            //            std::cout << "inhomogenity " << hanging_node_constraints.get_inhomogeneity(dof) << std::endl;

            (*m_solution_previous)(dof) = hanging_node_constraints.get_inhomogeneity(dof);
        }
    }

    //    for (std::map<dealii::types::global_dof_index, double>::const_iterator p = hanging_node_constraints.begin(); p != hanging_node_constraints.end(); ++p)
    //        m_solution_previous(p->first) = p->second;
}

void SolverDeal::solve()
{
    if (m_fieldInfo->hasTransientAnalysis() && m_fieldInfo->value(FieldInfo::TransientAnalysis).toBool())
    {
        setup(true);
        assembleSystem();

        // initial condition
        *m_solution = 0.0;
        dealii::VectorTools::interpolate(*m_doFHandler,
                                         dealii::ConstantFunction<2>(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble()),
                                         *m_solution);

        // initial step
        FieldSolutionID solutionID(m_fieldInfo, 0, 0, SolutionMode_Normal);
        SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler->n_dofs());
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);

        // parameters
        double time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
        const double coarsen_param = 1.2;
        const double refine_param = 0.8;
        const double min_delta = 1e-8;
        const double max_delta = 10 * time_step;
        const double refine_tol = 1e-1;
        const double coarsen_tol = 1e-5;

        std::shared_ptr<dealii::TimeStepping::RungeKutta<dealii::Vector<double> > > rungeKutta;

        switch (timeStepMethodType((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()))
        {
        case TimeStepMethodType_Implicit:
            rungeKutta = std::shared_ptr<dealii::TimeStepping::ImplicitRungeKutta<dealii::Vector<double> > >(
                        new dealii::TimeStepping::ImplicitRungeKutta<dealii::Vector<double> >((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()));
            break;
        case TimeStepMethodType_Explicit:
            rungeKutta = std::shared_ptr<dealii::TimeStepping::ExplicitRungeKutta<dealii::Vector<double> > >(
                        new dealii::TimeStepping::ExplicitRungeKutta<dealii::Vector<double> >((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()));
            break;
        case TimeStepMethodType_EmbeddedExplicit:
            rungeKutta = std::shared_ptr<dealii::TimeStepping::EmbeddedExplicitRungeKutta<dealii::Vector<double> > >(
                        new dealii::TimeStepping::EmbeddedExplicitRungeKutta<dealii::Vector<double> >((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt(),
                                                                                                      coarsen_param,
                                                                                                      refine_param,
                                                                                                      min_delta,
                                                                                                      max_delta,
                                                                                                      refine_tol,
                                                                                                      coarsen_tol));
            break;
        default:
            assert(0);
        }

        double time = 0.0;
        for (unsigned int i = 0; i < Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); ++i)
        {
            /*
            switch (timeStepMethodType((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()))
            {
            case TimeStepMethodType_Implicit:
                time = static_cast<dealii::TimeStepping::ImplicitRungeKutta<dealii::Vector<double> > *>(rungeKutta.get())->
                        evolve_one_time_step(std::bind(&SolverDeal::transientEvaluateMassMatrixExplicitPart,
                                                       this, std::placeholders::_1, std::placeholders::_2),
                                             std::bind(&SolverDeal::transientEvaluateMassMatrixImplicitPart,
                                                       this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                             time, time_step, *m_solution);
                break;
            case TimeStepMethodType_Explicit:
                time = static_cast<dealii::TimeStepping::ExplicitRungeKutta<dealii::Vector<double> > *>(rungeKutta.get())->
                        evolve_one_time_step(std::bind(&SolverDeal::transientEvaluateMassMatrixExplicitPart,
                                                       this, std::placeholders::_1, std::placeholders::_2),
                                             time, time_step, *m_solution);
                break;
            case TimeStepMethodType_EmbeddedExplicit:
                if (time + time_step > Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble())
                    time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() - time;
                time = static_cast<dealii::TimeStepping::EmbeddedExplicitRungeKutta<dealii::Vector<double> > *>(rungeKutta.get())->
                        evolve_one_time_step(std::bind(&SolverDeal::transientEvaluateMassMatrixExplicitPart,
                                                       this, std::placeholders::_1, std::placeholders::_2),
                                             time, time_step, *m_solution);

                time_step = static_cast<dealii::TimeStepping::EmbeddedExplicitRungeKutta<dealii::Vector<double> > *>(rungeKutta.get())->get_status().delta_t_guess;
                break;
            default:
                assert(0);
            }
            */

            // implicit Euler
            *m_solution = transientEvaluateMassMatrixImplicitPart(time, time_step, *m_solution);
            time += time_step;

            // set new time
            set_time(time);
            // store time actual timestep
            Agros2D::problem()->setActualTimeStepLength(time_step);

            Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                         QObject::tr("Transient step %1/%2 (actual time: %3 s)").
                                         arg(i).
                                         arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                         arg(time));

            Agros2D::log()->updateTransientChartInfo(time);            

            // add solution
            // TODO: create better adaptive, linear, time workflow!!!!!!

            hanging_node_constraints.distribute(*m_solution);

            FieldSolutionID solutionID(m_fieldInfo, i+1, 0, SolutionMode_Normal);
            SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler->n_dofs());
            Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);

            // adapt mesh
            if (m_fieldInfo->adaptivityType() != AdaptivityMethod_None &&
                    i < Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt() - 1)
            {
                Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, i, 1);
                Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                             arg(1).
                                             arg(0.0).
                                             arg(m_doFHandler->n_dofs()));

                refineGrid(false);

                int min_grid_level = 1;
                int max_grid_level = 2;

                if (m_triangulation->n_levels() > max_grid_level)
                    for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(max_grid_level); cell != m_triangulation->end(); ++cell)
                        cell->clear_refine_flag();
                // for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(min_grid_level); cell != m_triangulation->end_active(min_grid_level); ++cell)
                //     cell->clear_coarsen_flag();

                hanging_node_constraints.distribute(*m_solution);

                dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(*m_doFHandler);
                dealii::Vector<double> previousSolution = *m_solution;

                m_triangulation->prepare_coarsening_and_refinement();
                solutionTrans.prepare_for_coarsening_and_refinement(previousSolution);
                m_triangulation->execute_coarsening_and_refinement();

                // reinit
                setup(true);
                assembleSystem();

                // transfer solution
                solutionTrans.interpolate(previousSolution, *m_solution);
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
    if (m_fieldInfo->linearityType() == LinearityType_Linear)
    {
        setup(true);
        assembleSystem();
        solveLinearSystem();
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
        assert(0);
}

void SolverDeal::solveProblemNonLinearPicard()
{
    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    setup(true);

    // initial relative change of solutions
    double relChangeSol = 100.0;

    int iteration = 0;
    bool criteriaReached = false;
    while ((iteration < MAX_NUM_NONLIN_ITERS) && !criteriaReached)
    {
        SolverAgros::Phase phase = SolverAgros::Phase_Solving;

        iteration++;

        qDebug() << "step: " << iteration;
        assembleSystem();
        solveLinearSystem();

        // copy solution
        if (m_solution_previous)
        {
            m_solution_previous->add(-1, *m_solution);
            relChangeSol = m_solution_previous->l2_norm() / m_solution->l2_norm() * 100;
            delete m_solution_previous;
        }

        m_solution_previous = new dealii::Vector<double>(*m_solution);

        // update
        steps.append(iteration);
        relativeChangeOfSolutions.append(relChangeSol);

        criteriaReached = true;

        if ((m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() > 0) &&
                (m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() < relChangeSol))
            criteriaReached = false;

        // log messages
        if (criteriaReached)
            phase = SolverAgros::Phase_Finished;

        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Iteration: %1 (rel. change of sol.: %2 %)")
                                     .arg(iteration)
                                     .arg(QString::number(relativeChangeOfSolutions.last(), 'f', 5)));

        Agros2D::log()->updateNonlinearChartInfo(phase, steps, relativeChangeOfSolutions);
    }

    qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

void SolverDeal::solveProblemNonLinearNewton()
{
    const double minAllowedDampingCoeff = 1e-4;
    const double autoDampingRatio = 2.0;

    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    // todo: some work is duplicated
    // decide, how the adaptivity, nonlinear (and time ) steps will be organized
    setupProblemNonLinearNewton();
    setup(false);

    // initial residual norm
    double residualNorm = 0.0;

    // initial damping factor
    double dampingFactor = m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble();
    int dampingSuccessfulSteps = 0;

    int iteration = 0;
    bool criteriaReached = false;
    while ((iteration < MAX_NUM_NONLIN_ITERS) && !criteriaReached)
    {
        SolverAgros::Phase phase = SolverAgros::Phase_Solving;

        QTime time;
        time.start();

        iteration++;
        qDebug() << "step: " << iteration;
        assembleSystem();
        qDebug() << "assemble (" << time.elapsed() << "ms )";

        system_rhs *= -1.0;
        solveLinearSystem();

        // residual norm
        double previousResidualNorm = residualNorm;
        residualNorm = system_rhs.l2_norm();

        assert(m_solution_previous);
        // automatic damping factor
        if ((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt() == DampingType_Automatic)
        {
            if (previousResidualNorm > 0.0 && (residualNorm > previousResidualNorm * m_fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble()))
            {
                if (dampingFactor > minAllowedDampingCoeff)
                {
                    phase = SolverAgros::Phase_DampingFactorChanged;

                    dampingFactor = dampingFactor * 1.0 / autoDampingRatio;
                    dampingSuccessfulSteps = 0;
                }
            }
            else
            {
                dampingSuccessfulSteps++;

                if (dampingSuccessfulSteps >= m_fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt())
                {
                    if (dampingFactor * autoDampingRatio <= m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble())
                        dampingFactor = dampingFactor * 0.75 * autoDampingRatio;
                    else
                        dampingFactor = m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble();
                }
            }
        }

        m_solution_previous->add(dampingFactor, *m_solution);

        // update
        steps.append(iteration);
        double relChangeSol = dampingFactor * (*m_solution).l2_norm() / m_solution_previous->l2_norm() * 100;
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
                                     .arg(QString::number(relativeChangeOfSolutions.last(), 'f', 5))
                                     .arg(QString::number(residualNorm, 'e', 3))
                                     .arg(dampingFactor));

        Agros2D::log()->updateNonlinearChartInfo(phase, steps, relativeChangeOfSolutions);
    }

    // put the final solution into the solution
    assert(m_solution);
    delete(m_solution);
    m_solution = new dealii::Vector<double>(*m_solution_previous);
    delete m_solution_previous;
    m_solution_previous = nullptr;

    qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

void SolverDeal::solveAdaptivity()
{
    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
    {
        solveProblem();

        FieldSolutionID solutionID(m_fieldInfo, 0, 0, SolutionMode_Normal);
        SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler->n_dofs());
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);
    }
    else
    {
        double previousNorm = 0.0;
        for (int i = 0; i < m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); i++)
        {
            if (i > 0)
            {
                refineGrid(true);
            }

            solveProblem();

            double norm = computeNorm();
            double error = std::fabs(previousNorm - norm) / norm * 100.0;
            previousNorm = norm;

            FieldSolutionID solutionID(m_fieldInfo, 0, i, SolutionMode_Normal);
            SolutionStore::SolutionRunTimeDetails runTime(0.0, error, m_doFHandler->n_dofs());
            Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);

            if (i > 0)
                Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, 0, i);

            Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                         arg(i + 1).
                                         arg(error).
                                         arg(m_doFHandler->n_dofs()));
        }
    }
}

dealii::Vector<double> SolverDeal::transientEvaluateMassMatrixExplicitPart(const double time, const dealii::Vector<double> &y) const
{
    dealii::SparseDirectUMFPACK inverse_mass_matrix;

    dealii::Vector<double> tmp(m_doFHandler->n_dofs());
    tmp = 0.0;
    system_matrix.vmult(tmp, y);
    tmp *= -1.0;
    tmp.add(system_rhs);

    inverse_mass_matrix.initialize(mass_matrix);
    dealii::Vector<double> value(m_doFHandler->n_dofs());
    inverse_mass_matrix.vmult(value, tmp);

    return value;
}

dealii::Vector<double> SolverDeal::transientEvaluateMassMatrixImplicitPart(const double time, const double tau, const dealii::Vector<double> &y)
{
    dealii::SparseDirectUMFPACK inverse_mass_minus_tau_Jacobian;

    mass_minus_tau_Jacobian.copy_from(mass_matrix);
    mass_minus_tau_Jacobian.add(tau, system_matrix);

    inverse_mass_minus_tau_Jacobian.initialize(mass_minus_tau_Jacobian);

    // add rhs
    dealii::Vector<double> tmp(m_doFHandler->n_dofs());
    mass_matrix.vmult(tmp, y);

    dealii::Vector<double> result(y);
    inverse_mass_minus_tau_Jacobian.vmult(result, tmp);

    return result;
}

// hand made Euler, used only for debugging
void SolverDeal::transientForwardEuler()
{
    const double time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();

    dealii::SparseDirectUMFPACK inverse_mass_matrix;
    inverse_mass_matrix.initialize(mass_matrix);
    dealii::Vector<double> tmp(m_doFHandler->n_dofs());

    double time = 0.0;
    for (unsigned int i = 0; i < Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); ++i)
    {
        tmp = 0.0;
        system_matrix.vmult(tmp, *m_solution);
        tmp.add(-1.0, system_rhs);

        dealii::Vector<double> value(m_doFHandler->n_dofs());
        inverse_mass_matrix.vmult(value, tmp);

        m_solution->add(-time_step, value);
        time += time_step;

        // set new time
        set_time(time);
        // store time actual timestep
        Agros2D::problem()->setActualTimeStepLength(time_step);

        Agros2D::log()->printMessage(QObject::tr("Solver (%1), Forward Euler").arg(m_fieldInfo->fieldId()),
                                     QObject::tr("Transient step %1/%2 (actual time: %3 s)").
                                     arg(i).
                                     arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                     arg(time));

        Agros2D::log()->updateTransientChartInfo(time);
    }

    hanging_node_constraints.distribute(*m_solution);
}

// hand made Euler, used only for debugging
void SolverDeal::transientBackwardEuler()
{
    const double time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
    dealii::TimeStepping::ImplicitRungeKutta<dealii::Vector<double> >
            implicit_runge_kutta((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt());

    dealii::SparseDirectUMFPACK inverse_mass_minus_tau_Jacobian;

    mass_minus_tau_Jacobian.copy_from(mass_matrix);
    //mass_minus_tau_Jacobian.add(-tau, system_matrix);
    mass_minus_tau_Jacobian.add(time_step, system_matrix);

    inverse_mass_minus_tau_Jacobian.initialize(mass_minus_tau_Jacobian);
    dealii::Vector<double> tmp(m_doFHandler->n_dofs());

    double time = 0.0;
    for (unsigned int i = 0; i < Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); ++i)
    {
        mass_matrix.vmult(tmp, *m_solution);
        tmp.add(time_step, system_rhs);

        inverse_mass_minus_tau_Jacobian.vmult(*m_solution, tmp);

        time += time_step;

        // set new time
        set_time(time);
        // store time actual timestep
        Agros2D::problem()->setActualTimeStepLength(time_step);

        Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                     QObject::tr("Transient step %1/%2 (actual time: %3 s)").
                                     arg(i).
                                     arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                     arg(time));

        Agros2D::log()->updateTransientChartInfo(time);
    }

    hanging_node_constraints.distribute(*m_solution);
}

void SolverDeal::transientExplicitMethod()
{
//    transientForwardEuler();
//    return;

    const double time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
    dealii::TimeStepping::ExplicitRungeKutta<dealii::Vector<double> >
            explicit_runge_kutta((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt());

    double time = 0.0;
    for (unsigned int i = 0; i < Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); ++i)
    {
        time = explicit_runge_kutta.evolve_one_time_step(std::bind(&SolverDeal::transientEvaluateMassMatrixExplicitPart,
                                                                   this, std::placeholders::_1, std::placeholders::_2),
                                                         time, time_step, *m_solution);
        // set new time
        set_time(time);
        // store time actual timestep
        Agros2D::problem()->setActualTimeStepLength(time_step);

        Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                     QObject::tr("Transient step %1/%2 (actual time: %3 s)").
                                     arg(i).
                                     arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                     arg(time));

        Agros2D::log()->updateTransientChartInfo(time);
    }

    hanging_node_constraints.distribute(*m_solution);
}


void SolverDeal::transientImplicitMethod()
{
//    transientBackwardEuler();
//    return;


    const double time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() / Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
    dealii::TimeStepping::ImplicitRungeKutta<dealii::Vector<double> >
            implicit_runge_kutta((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt());

    double time = 0.0;
    for (unsigned int i = 0; i < Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); ++i)
    {
        time = implicit_runge_kutta.evolve_one_time_step(std::bind(&SolverDeal::transientEvaluateMassMatrixExplicitPart,
                                                                   this, std::placeholders::_1, std::placeholders::_2),
                                                         std::bind(&SolverDeal::transientEvaluateMassMatrixImplicitPart,
                                                                   this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                                                         time, time_step, *m_solution);

        // set new time
        set_time(time);
        // store time actual timestep
        Agros2D::problem()->setActualTimeStepLength(time_step);

        Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                     QObject::tr("Transient step %1/%2 (actual time: %3 s)").
                                     arg(i).
                                     arg(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()).
                                     arg(time));

        Agros2D::log()->updateTransientChartInfo(time);
    }

    hanging_node_constraints.distribute(*m_solution);
}

unsigned int SolverDeal::transientExplicitEmbeddedMethod()
{
    double time_step = Agros2D::problem()->config()->constantTimeStepLength();

    const double coarsen_param = 1.2;
    const double refine_param = 0.8;
    const double min_delta = 1e-8;
    const double max_delta = 10 * time_step;
    const double refine_tol = 1e-1;
    const double coarsen_tol = 1e-5;

    dealii::TimeStepping::EmbeddedExplicitRungeKutta<dealii::Vector<double> >
            embedded_explicit_runge_kutta((dealii::TimeStepping::runge_kutta_method) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt(),
                                          coarsen_param,
                                          refine_param,
                                          min_delta,
                                          max_delta,
                                          refine_tol,
                                          coarsen_tol);

    unsigned int n_steps = 0;
    double time = 0.0;
    while (time < Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble())
    {
        if (time + time_step > Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble())
            time_step = Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() - time;
        time = embedded_explicit_runge_kutta.evolve_one_time_step(std::bind(&SolverDeal::transientEvaluateMassMatrixExplicitPart,
                                                                            this, std::placeholders::_1, std::placeholders::_2),
                                                                  time, time_step, *m_solution);
        std::cout << "time: " << time << std::endl;

        time_step = embedded_explicit_runge_kutta.get_status().delta_t_guess;
        ++n_steps;
    }

    hanging_node_constraints.distribute(*m_solution);

    return n_steps;
}

void SolverAgros::clearSteps()
{
    m_steps.clear();
    m_damping.clear();
    m_residualNorms.clear();
    m_solutionNorms.clear();
}

/*
void AgrosExternalSolverExternal::solve(double* initial_guess)
{
    initialGuess = initial_guess;

    fileMatrix = QString("%1/solver_matrix").arg(cacheProblemDir());
    fileRHS = QString("%1/solver_rhs").arg(cacheProblemDir());
    fileInitial = QString("%1/solver_initial").arg(cacheProblemDir());
    fileSln = QString("%1/solver_sln").arg(cacheProblemDir());

    this->set_matrix_export_format(EXPORT_FORMAT_BSON);
    this->set_matrix_filename(fileMatrix.toStdString());
    this->set_matrix_varname("matrix");
    this->set_matrix_number_format((char *) "%g");
    this->set_rhs_export_format(EXPORT_FORMAT_BSON);
    this->set_rhs_filename(fileRHS.toStdString());
    this->set_rhs_varname("rhs");
    this->set_rhs_number_format((char *) "%g");

    // store state
    bool matrixOn = this->output_matrixOn;
    bool rhsOn = this->output_rhsOn;
    this->output_matrixOn = true;
    this->output_rhsOn = true;

    // write matrix and rhs to disk
    // QTime time;
    // time.start();
    this->process_matrix_output(this->m);
    // qDebug() << "process_matrix_output" << time.elapsed();
    // time.start();
    this->process_vector_output(this->rhs);
    // qDebug() << "process_vector_output" << time.elapsed();

    // write initial guess to disk
    if (initialGuess)
    {
        SimpleVector<double> initialVector;
        initialVector.alloc(rhs->get_size());
        initialVector.set_vector(initialGuess);
        initialVector.export_to_file(fileInitial.toStdString().c_str(),
                                     (char *) "initial",
                                     EXPORT_FORMAT_BSON,
                                     (char *) "%lf");
        initialVector.free();
    }

    if (!(Agros2D::problem()->isTransient() || Agros2D::problem()->isNonlinear()))
        this->m->free();
    this->rhs->free();

    // restore state
    this->output_matrixOn = matrixOn;
    this->output_rhsOn = rhsOn;

    // exec octave
    m_process = new QProcess();
    m_process->setStandardOutputFile(tempProblemDir() + "/solver.out");
    m_process->setStandardErrorFile(tempProblemDir() + "/solver.err");
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));

    setSolverCommand();
    m_process->start(command);

    // execute an event loop to process the request (nearly-synchronous)
    QEventLoop eventLoop;
    QObject::connect(m_process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
    QObject::connect(m_process, SIGNAL(error(QProcess::ProcessError)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    SimpleVector<double> slnVector;
    // time.start();
    slnVector.import_from_file((char*) fileSln.toStdString().c_str(), "sln", EXPORT_FORMAT_BSON);
    // qDebug() << "slnVector import_from_file" << time.elapsed();

    delete [] this->sln;
    this->sln = new double[slnVector.get_size()];
    memcpy(this->sln, slnVector.v, slnVector.get_size() * sizeof(double));

    QFile::remove(command);
    if (initialGuess)
        QFile::remove(fileInitial);
    QFile::remove(fileMatrix);
    QFile::remove(fileRHS);
    QFile::remove(fileSln);

    QFile::remove(tempProblemDir() + "/solver.out");
    QFile::remove(tempProblemDir() + "/solver.err");
}
*/

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
    // todo: this is temporary!!!
    QList<FieldInfo*> fieldInfosSorted;
    QList<QString> fieldInfoOrder;
    fieldInfoOrder.push_back("electrostatic");
    fieldInfoOrder.push_back("magnetic");
    fieldInfoOrder.push_back("current");
    fieldInfoOrder.push_back("heat");
    fieldInfoOrder.push_back("elasticity");
    fieldInfoOrder.push_back("rf_te");
    fieldInfoOrder.push_back("rf_tm");
    fieldInfoOrder.push_back("flow");
    fieldInfoOrder.push_back("acoustic");

    foreach(QString fieldName, fieldInfoOrder)
    {
        foreach(FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
        {
            if(fieldInfo->fieldId() == fieldName)
            {
                fieldInfosSorted.push_back(fieldInfo);
            }
        }
    }

    foreach (FieldInfo* fieldInfo, fieldInfosSorted)
    {
        // frequency
        // todo: find some better place, where some values are initialized
        fieldInfo->setFrequency(Agros2D::problem()->config()->value(ProblemConfig::Frequency).toDouble());

        qDebug() << "solving " << fieldInfo->name();
        SolverDeal *solverDeal = m_solverDeal[fieldInfo];

        // look for coupling sources
        foreach(FieldInfo* sourceFieldInfo, fieldInfosSorted)
        {
            // todo: check if it is also used!
            if(couplingList()->isCouplingAvailable(sourceFieldInfo, fieldInfo, CouplingType_Weak))
            {
                FieldSolutionID solutionID(sourceFieldInfo, 0, 0, SolutionMode_Normal);
                MultiArray sourceSolution = Agros2D::solutionStore()->multiArray(solutionID);

                solverDeal->setCouplingSource(sourceFieldInfo->fieldId(), sourceSolution.solution());
            }
        }

        solverDeal->solve();
    }
}

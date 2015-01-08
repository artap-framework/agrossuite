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
#include <deal.II/lac/sparse_direct.h>
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


template<int dim>
class RightHandSide : public dealii::Function<dim>
{
public:
    RightHandSide ()
        :
          dealii::Function<dim>()
    {}
    virtual double value (const dealii::Point<dim> &p,
                          const unsigned int component = 0) const;
};

template<int dim>
double RightHandSide<dim>::value (const dealii::Point<dim> &p,
                                  const unsigned int component) const
{
    Assert (component == 0, ExcInternalError());
    Assert (dim == 2, ExcNotImplemented());

    const double time = this->get_time();

    if ((time >= 0.0) && (time <= 10))
    {
        if ((p[0] >= 0.013) && (p[0] <= 0.028) && (p[1] >= 0.041) && (p[1] <= 0.17))
        {
            return 2e6;
        }
    }

    return 0;
}

// *******************************************************************************************

SolverDeal::SolverDeal(const FieldInfo *fieldInfo)
    : m_fieldInfo(fieldInfo), m_solution_previous(NULL)
{    
    // fe collection
    qDebug() << "SolverDeal::SolverDeal: numberOfSolutions" << fieldInfo->numberOfSolutions();
    m_feCollection = new dealii::hp::FECollection<2>();

    // copy initial mesh
    m_triangulation = new dealii::Triangulation<2>();
    m_triangulation->copy_triangulation(*m_fieldInfo->initialMesh());

    // info
    // std::vector<dealii::types::boundary_id> bindicators = m_triangulation->get_boundary_indicators();
    // std::cout << "Number of boundary indicators: " << bindicators.size() << std::endl;
    // std::cout << "Number of active cells: " << m_triangulation->n_active_cells() << std::endl;
    // std::cout << "Total number of cells: " << m_triangulation->n_cells() << std::endl;

    // create dof handler
    m_doFHandler = new dealii::hp::DoFHandler<2>(*m_triangulation);

    // create solution vector
    m_solution = new dealii::Vector<double>();

    // Gauss quadrature and fe collection
    for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        m_feCollection->push_back(dealii::FESystem<2>(dealii::FE_Q<2>(degree), fieldInfo->numberOfSolutions()));
        m_quadrature_formulas.push_back(dealii::QGauss<2>(degree +  QUADRATURE_ORDER_INCREASE));
        m_face_quadrature_formulas.push_back(dealii::QGauss<2-1>(degree + QUADRATURE_ORDER_INCREASE));
    }
}

SolverDeal::~SolverDeal()
{
    // delete m_triangulation;
    // delete m_fe;
}

void SolverDeal::setup()
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
    assembleDirichlet();

    hanging_node_constraints.close();

    // create sparsity pattern
    dealii::CompressedSetSparsityPattern csp(m_doFHandler->n_dofs(), m_doFHandler->n_dofs());
    dealii::DoFTools::make_sparsity_pattern(*m_doFHandler, csp, hanging_node_constraints, false);
    sparsity_pattern.copy_from(csp);

    // reinit system matrix
    system_matrix.reinit(sparsity_pattern);

    // mass matrix (transient)
    // if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        // mass_matrix.reinit(sparsity_pattern);
        // assembleMassMatrix();
        // qDebug() << "transient - mass_matrix";
    }
    // qDebug() << "setup (" << time.elapsed() << "ms )";
}

void SolverDeal::solve()
{
    // solveTransientStep();
    solveAdaptivity();
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

    // copy solution
    if (m_fieldInfo->linearityType() != LinearityType_Linear)
    {
        if (m_solution_previous)
            delete m_solution_previous;
        m_solution_previous = new dealii::Vector<double>(*m_solution);
    }

    qDebug() << "solved (" << time.elapsed() << "ms )";
}

void SolverDeal::solveLinearity()
{
    if (m_fieldInfo->linearityType() == LinearityType_Linear)
        solveLinearityLinear();
    else
        solveLinearityNonLinear();
}

void SolverDeal::solveLinearityLinear()
{
    setup();

    QTime time;
    time.start();

    assembleSystem();
    qDebug() << "assemble (" << time.elapsed() << "ms )";

    solveLinearSystem();

    qDebug() << "solve linear total (" << time.elapsed() << "ms )";

}

void SolverDeal::solveLinearityNonLinear()
{
    if (m_fieldInfo->linearityType() == LinearityType_Picard)
        solveLinearityNonLinearPicard();
    else if (m_fieldInfo->linearityType() == LinearityType_Newton)
        solveLinearityNonLinearNewton();
    else
        assert(0);
}

void SolverDeal::solveLinearityNonLinearPicard()
{
    QTime time;
    time.start();

    QVector<double> steps;
    QVector<double> relativeChangeOfSolutions;

    setup();
    for (int iteration = 0; iteration < 6; iteration++)
    {
        qDebug() << "step: " << iteration;
        assembleSystem();
        solveLinearSystem();

        // update
        steps.append(iteration);
        relativeChangeOfSolutions.append(1.0);

        // damping: %2
        Agros2D::log()->printMessage(QObject::tr("Solver (Picard)"), QObject::tr("Iteration: %1 (rel. change of sol.: %2 %)")
                                     .arg(iteration)
                                     .arg(QString::number(relativeChangeOfSolutions.last(), 'f', 5)));

        Agros2D::log()->updateNonlinearChartInfo(SolverAgros::Phase_Finished, steps, relativeChangeOfSolutions);
    }
    qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

void SolverDeal::solveLinearityNonLinearNewton()
{
    assert(0);
}

void SolverDeal::solveAdaptivity()
{
    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
        solveAdaptivitySimple();
    else
        solveAdaptivityAdaptive();
}

void SolverDeal::solveAdaptivitySimple()
{
    solveLinearity();

    FieldSolutionID solutionID(m_fieldInfo, 0, 0, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(), 0.0, m_doFHandler->n_dofs());
    Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);
}

void SolverDeal::solveAdaptivityAdaptive()
{
    double previousNorm = 0.0;
    for (int i = 0; i < m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); i++)
    {
        if (i > 0)
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
            dealii::Vector<float> smoothness_indicators;

            if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
            {
                smoothness_indicators.reinit(m_triangulation->n_active_cells());
                estimateSmoothness(smoothness_indicators);

                min_smoothness = *std::max_element(smoothness_indicators.begin(), smoothness_indicators.end());
                max_smoothness = *std::min_element(smoothness_indicators.begin(), smoothness_indicators.end());
                dealii::hp::DoFHandler<2>::active_cell_iterator cellmm = m_doFHandler->begin_active(), endcmm = m_doFHandler->end();
                for (unsigned int index = 0; cellmm != endcmm; ++cellmm, ++index)
                {
                    if (cellmm->refine_flag_set())
                    {
                        max_smoothness = std::max(max_smoothness, smoothness_indicators(index));
                        min_smoothness = std::min(min_smoothness, smoothness_indicators(index));
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
                        if (cell->refine_flag_set() && (smoothness_indicators(index) > threshold_smoothness)
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

            m_triangulation->execute_coarsening_and_refinement();
        }

        solveLinearity();

        // *********************************************************************************

        // TODO: remove (only for paper)
        //        double energy = 0.0;

        //        dealii::hp::QCollection<2> quadrature_formulas;
        //        for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
        //            quadrature_formulas.push_back(dealii::QGauss<2>(degree + 1));

        //        dealii::hp::FEValues<2> hp_fe_values(m_doFHandler->get_fe(), quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);

        //        for (int iLabel = 0; iLabel < Agros2D::scene()->labels->count(); iLabel++)
        //        {
        //            SceneLabel *label = Agros2D::scene()->labels->at(iLabel);

        //            SceneMaterial *material = label->marker(m_fieldInfo);

        //            const Value *material_electrostatic_permittivity = material->valueNakedPtr(QLatin1String("electrostatic_permittivity"));
        //            const Value *material_electrostatic_charge_density = material->valueNakedPtr(QLatin1String("electrostatic_charge_density"));

        //            // Then start the loop over all cells, and select those cells which are close enough to the evaluation point:
        //            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = m_doFHandler->begin_active(), endc_int = m_doFHandler->end();
        //            for (; cell_int != endc_int; ++cell_int)
        //            {
        //                // volume integration
        //                if (cell_int->material_id() - 1 == iLabel)
        //                {
        //                    hp_fe_values.reinit(cell_int);

        //                    const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
        //                    const unsigned int n_q_points = fe_values.n_quadrature_points;

        //                    std::vector<dealii::Vector<double> > solution_values(n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
        //                    std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

        //                    fe_values.get_function_values(*m_solution, solution_values);
        //                    fe_values.get_function_gradients(*m_solution, solution_grads);

        //                    // expressions

        //                    for (unsigned int k = 0; k < n_q_points; ++k)
        //                    {
        //                        const dealii::Point<2> p = fe_values.quadrature_point(k);

        //                        energy += fe_values.JxW(k) * (2.0*M_PI*p[0]*0.5*material_electrostatic_permittivity->number()*8.854e-12*(solution_grads[k][0][0]*solution_grads[k][0][0]+solution_grads[k][0][1]*solution_grads[k][0][1]));
        //                    }

        //                }
        //            }
        //        }

        //        std::cout << energy << std::endl;

        // ********************************************************************************

        double norm = computeNorm();
        double error = std::fabs(previousNorm - norm) / norm * 100.0;
        previousNorm = norm;

        FieldSolutionID solutionID(m_fieldInfo, 0, i, SolutionMode_Normal);
        SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(), error, m_doFHandler->n_dofs());
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);

        if (i > 0)
            Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, 0, i);

        Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                     arg(i + 1).
                                     arg(error).
                                     arg(m_doFHandler->n_dofs()));
    }
}

void SolverDeal::solveTransientStep()
{
    setup();
    assembleSystem();

    dealii::Vector<double> tmp;
    dealii::Vector<double> forcing_terms;

    tmp.reinit(m_solution->size());
    forcing_terms.reinit(m_solution->size());

    // initial condition
    m_solution_previous = new dealii::Vector<double>(m_solution->size());
    dealii::VectorTools::interpolate(*m_doFHandler, dealii::ConstantFunction<2>(293.15), *m_solution_previous);

    const unsigned int initial_global_refinement = m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt();
    const unsigned int n_adaptive_pre_refinement_steps = 2;

    double timestep_number = 0;
    double time = 0;
    double theta = 0.5;
    double total_time = 15;
    double time_step = total_time / 150;
    // output_results();

    // copy system matrix to steady part
    steady_matrix.reinit(sparsity_pattern);
    steady_matrix.copy_from(system_matrix);
    system_matrix = 0;

    RightHandSide<2> rhs_function;

    while (time <= total_time)
    {
        time += time_step;
        ++timestep_number;

        std::cout << "Time step " << timestep_number << " at t=" << time << std::endl;

        mass_matrix.vmult(system_rhs, *m_solution_previous);
        steady_matrix.vmult(tmp, *m_solution_previous);
        system_rhs.add(- (1 - theta) * time_step, tmp);

        rhs_function.set_time(time);
        dealii::VectorTools::create_right_hand_side(*m_doFHandler, m_quadrature_formulas, rhs_function, tmp);
        forcing_terms = tmp;
        forcing_terms *= time_step * theta;

        rhs_function.set_time(time - time_step);
        dealii::VectorTools::create_right_hand_side(*m_doFHandler, m_quadrature_formulas, rhs_function, tmp);
        forcing_terms.add(time_step * (1 - theta), tmp);

        system_rhs += forcing_terms;

        system_matrix.copy_from(mass_matrix);
        system_matrix.add(theta * time_step, steady_matrix);

        solveLinearSystem();
        // output_results();

        // adapt mesh
        if (true) // adapt
        {
            dealii::Vector<float> estimated_error_per_cell(m_triangulation->n_active_cells());
            dealii::KellyErrorEstimator<2>::estimate(*m_doFHandler,
                                                     m_face_quadrature_formulas,
													 TYPENAME dealii::FunctionMap<2>::type(),
                                                     *m_solution,
                                                     estimated_error_per_cell);

            dealii::GridRefinement::refine_and_coarsen_fixed_fraction(*m_triangulation, estimated_error_per_cell, 0.6, 0.4);
            // dealii::GridRefinement::refine_and_coarsen_fixed_number(*m_triangulation, estimated_error_per_cell, 0.6, 0.3);

            // hp
            if (false)
            {
                dealii::Vector<float> smoothness_indicators;

                smoothness_indicators.reinit(m_triangulation->n_active_cells());
                estimateSmoothness(smoothness_indicators);

                float min_smoothness = *std::max_element(smoothness_indicators.begin(), smoothness_indicators.end());
                float max_smoothness = *std::min_element(smoothness_indicators.begin(), smoothness_indicators.end());
				TYPENAME dealii::hp::DoFHandler<2>::active_cell_iterator cellmm = m_doFHandler->begin_active(), endcmm = m_doFHandler->end();
                for (unsigned int index = 0; cellmm != endcmm; ++cellmm, ++index)
                {
                    if (cellmm->refine_flag_set())
                    {
                        max_smoothness = std::max(max_smoothness, smoothness_indicators(index));
                        min_smoothness = std::min(min_smoothness, smoothness_indicators(index));
                    }
                }

                const float threshold_smoothness = (max_smoothness + min_smoothness) / 2;

                dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler->begin_active(), endc = m_doFHandler->end();
                for (unsigned int index = 0; cell != endc; ++cell, ++index)
                {
                    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_P)
                    {
                        if (cell->refine_flag_set() && (cell->active_fe_index() + 1 < m_doFHandler->get_fe().size()))
                        {
                            // remove h adaptivity flag
                            cell->clear_refine_flag();
                            // increase order
                            cell->set_active_fe_index(cell->active_fe_index() + 1);
                        }
                    }

                    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_HP)
                    {
                        if (cell->refine_flag_set() && (smoothness_indicators(index) > threshold_smoothness)
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

            int min_grid_level = initial_global_refinement;
            int max_grid_level = initial_global_refinement + n_adaptive_pre_refinement_steps;

            if (m_triangulation->n_levels() > max_grid_level)
                for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(max_grid_level); cell != m_triangulation->end(); ++cell)
                    cell->clear_refine_flag();
            // for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(min_grid_level); cell != m_triangulation->end_active(min_grid_level); ++cell)
            //     cell->clear_coarsen_flag();

            hanging_node_constraints.distribute(*m_solution);

            dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solution_trans(*m_doFHandler);
            dealii::Vector<double> previous_solution = *m_solution;

            m_triangulation->prepare_coarsening_and_refinement();
            solution_trans.prepare_for_coarsening_and_refinement(previous_solution);
            m_triangulation->execute_coarsening_and_refinement();

            // reinit
            setup();
            assembleSystem();
            tmp.reinit(m_solution->size());
            forcing_terms.reinit(m_solution->size());
            // transfer solution
            solution_trans.interpolate(previous_solution, *m_solution);

            // copy system matrix to steady part
            steady_matrix.reinit(sparsity_pattern);
            steady_matrix.copy_from(system_matrix);
            system_matrix = 0;
        }

        // copy solution
        m_solution_previous = new dealii::Vector<double>(*m_solution);
    }

    hanging_node_constraints.distribute(*m_solution);

    FieldSolutionID solutionID(m_fieldInfo, 0, 0, SolutionMode_Normal);
    SolutionStore::SolutionRunTimeDetails runTime(Agros2D::problem()->actualTimeStepLength(), 0.0, m_doFHandler->n_dofs());
    Agros2D::solutionStore()->addSolution(solutionID, MultiArray(m_doFHandler, m_solution), runTime);
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

void SolverDeal::estimateSmoothness(dealii::Vector<float> &smoothness_indicators) const
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

// TODO: move to module
void SolverDeal::assembleMassMatrix()
{
    // assemble
    dealii::hp::FEValues<2> hp_fe_values(*m_feCollection, m_quadrature_formulas, dealii::update_values | dealii::update_quadrature_points | dealii::update_JxW_values);

    dealii::FullMatrix<double> cell_matrix;

    std::vector<dealii::types::global_dof_index> local_dof_indices;

    double sourceAir = 1000 * 1.2;
    double sourceCopper = 8700 * 385;
    double sourceIron = 7800 * 460;

    dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler->begin_active(), endc = m_doFHandler->end();
    for (; cell != endc; ++cell)
    {
        double source = 0;
        if ((cell->material_id() == 1) || (cell->material_id() == 2))
            source = sourceIron;
        else if (cell->material_id() == 3)
            source = sourceCopper;
        else if (cell->material_id() == 4)
            source = sourceAir;

        assert(source > 0);

        const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;

        // local matrix
        cell_matrix.reinit(dofs_per_cell, dofs_per_cell);
        cell_matrix = 0;

        hp_fe_values.reinit(cell);

        const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
        const unsigned int n_q_points = fe_values.n_quadrature_points;

        for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
        {
            const dealii::Point<2> p = fe_values.quadrature_point(q_point);

            for (unsigned int i = 0; i < dofs_per_cell; ++i)
            {
                // const unsigned int component_i = cell->get_fe().system_to_component_index(i).first;
                for (unsigned int j = 0; j < dofs_per_cell; ++j)
                {
                    // const unsigned int component_j = cell->get_fe().system_to_component_index(j).first;

                    cell_matrix(i,j) += source * p[0] * fe_values.shape_value(i,q_point) *
                            fe_values.shape_value(j,q_point) *
                            fe_values.JxW(q_point);
                }
            }
        }

        local_dof_indices.resize(dofs_per_cell);
        cell->get_dof_indices(local_dof_indices);

        // distribute local to global system
        hanging_node_constraints.distribute_local_to_global(cell_matrix,
                                                            local_dof_indices,
                                                            mass_matrix);
    }

    // inverse_mass_matrix.initialize(mass_matrix);
}
//
//dealii::Vector<double> SolverDeal::evaluateDiffusion(const double time, const dealii::Vector<double> &y) const
//{
//    dealii::Vector<double> tmp(m_doFHandler->n_dofs());
//    tmp = 0.0;

//    system_matrix.vmult(tmp, y);

//    dealii::hp::FEValues<2> hp_fe_values(*m_feCollection, m_quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values);

//    dealii::Vector<double> cell_rhs;

//    std::vector<dealii::types::global_dof_index> local_dof_indices;

//    dealii::hp::DoFHandler<2>::active_cell_iterator cell = m_doFHandler->begin_active(), endc = m_doFHandler->end();
//    for (; cell != endc; ++cell)
//    {
//        const unsigned int dofs_per_cell = cell->get_fe().dofs_per_cell;

//        // local matrix
//        cell_rhs.reinit(dofs_per_cell);
//        cell_rhs = 0;

//        hp_fe_values.reinit(cell);

//        const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
//        const unsigned int n_q_points = fe_values.n_quadrature_points;

//        for (unsigned int q_point = 0; q_point < n_q_points; ++q_point)
//        {
//            double source = 0.0; //(8700*380);
//            if (cell->material_id() == 1)
//                source = -1e6; // * time; // get_source(time, fe_values.quadrature_point(q_point));

//            for (unsigned int i = 0; i < dofs_per_cell; ++i)
//                cell_rhs(i) += source * fe_values.shape_value(i,q_point) * fe_values.JxW(q_point);

//            // qDebug() << cell->material_id() << source;
//        }

//        local_dof_indices.resize(dofs_per_cell);
//        cell->get_dof_indices(local_dof_indices);

//        // distribute local to global system
//        hanging_node_constraints.distribute_local_to_global(cell_rhs,
//                                                            local_dof_indices,
//                                                            tmp);
//    }

//    dealii::Vector<double> value(m_doFHandler->n_dofs());
//    inverse_mass_matrix.vmult(value, tmp);

//    return value;
//}

//dealii::Vector<double> SolverDeal::id_minus_tau_J_inverse(const double time, const double tau, const dealii::Vector<double> &y)
//{
//    dealii::SparseDirectUMFPACK inverse_mass_minus_tau_Jacobian;

//    mass_minus_tau_Jacobian.copy_from(mass_matrix);
//    mass_minus_tau_Jacobian.add(-tau, system_matrix);

//    inverse_mass_minus_tau_Jacobian.initialize(mass_minus_tau_Jacobian);

//    dealii::Vector<double> tmp(m_doFHandler->n_dofs());
//    mass_matrix.vmult(tmp, y);

//    dealii::Vector<double> result(y);
//    inverse_mass_minus_tau_Jacobian.vmult(result, tmp);

//    return result;
//}

//void SolverDeal::explicitMethod(const dealii::TimeStepping::runge_kutta_method method,
//                                const unsigned int n_time_steps, const double initial_time, const double final_time)
//{
//    const double time_step = (final_time - initial_time) / static_cast<double> (n_time_steps);
//    double time = initial_time;
//    *m_solution = 0;

//    dealii::TimeStepping::ExplicitRungeKutta<dealii::Vector<double> > explicit_runge_kutta(method);
//    // output_results(0, method);

//    for (unsigned int i = 0; i < n_time_steps; ++i)
//    {
//        time = explicit_runge_kutta.evolve_one_time_step(std::bind(&SolverDeal::evaluateDiffusion,
//                                                                   this, std::placeholders::_1, std::placeholders::_2),
//                                                         time, time_step, *m_solution);
//        qDebug() << time;

//        // if ((i+1) %10 == 0)
//        //     output_results(i+1,method);
//    }

//    hanging_node_constraints.distribute(*m_solution);
//}

//void SolverDeal::implicitMethod(const dealii::TimeStepping::runge_kutta_method method,
//                                const unsigned int n_time_steps, const double initial_time, const double final_time)
//{
//    const double time_step = (final_time - initial_time) / static_cast<double> (n_time_steps);
//    double time = initial_time;
//    *m_solution = 0;

//    dealii::TimeStepping::ImplicitRungeKutta<dealii::Vector<double> > implicit_runge_kutta(method);
//    // output_results(0, method);

//    for (unsigned int i = 0; i < n_time_steps; ++i)
//    {
//        time = implicit_runge_kutta.evolve_one_time_step(std::bind(&SolverDeal::evaluateDiffusion,
//                                                                   this, std::placeholders::_1, std::placeholders::_2),
//                                                         std::bind(&SolverDeal::id_minus_tau_J_inverse,
//                                                                   this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
//                                                         time, time_step, *m_solution);
//        // qDebug() << time;
//        // if ((i+1) %10 == 0)
//        //     output_results(i+1,method);
//    }
//}

//unsigned int SolverDeal::explicitEmbeddedMethod(const dealii::TimeStepping::runge_kutta_method method,
//                                                const unsigned int n_time_steps, const double initial_time, const double final_time)
//{
//    double time_step = (final_time - initial_time) / static_cast<double> (n_time_steps);
//    double time = initial_time;
//    *m_solution = 0;

//    const double coarsen_param = 1.2;
//    const double refine_param = 0.8;
//    const double min_delta = 1e-8;
//    const double max_delta = 10 * time_step;
//    const double refine_tol = 1e-1;
//    const double coarsen_tol = 1e-5;

//    dealii::TimeStepping::EmbeddedExplicitRungeKutta<dealii::Vector<double> > embedded_explicit_runge_kutta(method,
//                                                                                                            coarsen_param, refine_param, min_delta, max_delta,
//                                                                                                            refine_tol, coarsen_tol);
//    // output_results (0, method);

//    unsigned int n_steps = 0;
//    while (time < final_time)
//    {
//        if (time + time_step > final_time)
//            time_step = final_time - time;
//        time = embedded_explicit_runge_kutta.evolve_one_time_step(std::bind(&SolverDeal::evaluateDiffusion,
//                                                                            this, std::placeholders::_1, std::placeholders::_2),
//                                                                  time, time_step, *m_solution);
//        // qDebug() << time;
//        // if ((i+1) %10 == 0)
//        //     output_results(n_steps + 1,method);

//        time_step = embedded_explicit_runge_kutta.get_status().delta_t_guess;
//        ++n_steps;
//    }

//    return n_steps;
//}

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

ProblemSolver::ProblemSolver()
{
}

void ProblemSolver::init()
{
    foreach (FieldInfo *fieldInfo, m_solverDeal.keys())
        if (m_solverDeal[fieldInfo])
            delete m_solverDeal[fieldInfo];
    m_solverDeal.clear();

    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        m_solverDeal[fieldInfo] = fieldInfo->plugin()->solverDeal(fieldInfo);
    }
}

void ProblemSolver::solveProblem()
{
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SolverDeal *solverDeal = m_solverDeal[fieldInfo];
        solverDeal->solve();
    }
}


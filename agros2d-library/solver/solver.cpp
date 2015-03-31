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
#include "solver_external.h"
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

using namespace paralution;

// todo: find better place
// todo: what for curved elements?
const int QUADRATURE_ORDER_INCREASE = 1;

// todo: is it defined somewhere?
const int MAX_NUM_NONLIN_ITERS = 100;
const int MAX_NUM_TRANSIENT_ADAPTIVE_ITERS = 20;

tbb::mutex createCache;
tbb::mutex resizeCache;

dealii::hp::FECollection<2> *SolverDeal::createFECollection(const FieldInfo *fieldInfo)
{
    dealii::hp::FECollection<2> *feCollection = new dealii::hp::FECollection<2>();

    // Gauss quadrature and fe collection
    for (unsigned int degree = fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        std::vector<const dealii::FiniteElement<2> *> fes;
        std::vector<unsigned int> multiplicities;

        QMap<int, Module::Space> spaces = fieldInfo->spaces();
        foreach (int key, spaces.keys())
        {
            if (spaces.value(key).type() == "h1")
                fes.push_back(new dealii::FE_Q<2>(degree + spaces[key].orderAdjust()));
            else if (spaces.value(key).type() == "l2")
                fes.push_back(new dealii::FE_Q<2>(degree + spaces[key].orderAdjust())); // fes.push_back(new dealii::FE_DGP<2>(degree + spaces[key].orderAdjust()));

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
                                                     const dealii::hp::QCollection<2-1> &faceQuadratureFormulas)
    :
      hp_fe_values(mappingCollection,
                   feCollection,
                   quadratureFormulas,
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(mappingCollection,
                        feCollection,
                        faceQuadratureFormulas,
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
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
                        dealii::update_values | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
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

// *******************************************************************************************

SolverDeal::SolverDeal(const FieldInfo *fieldInfo)
    : m_fieldInfo(fieldInfo),
      m_scene(Agros2D::scene()),
      m_problem(Agros2D::problem()),
      m_time(0.0),
      m_assemble_matrix(true),
      // fe collection
      m_feCollection(new dealii::hp::FECollection<2>()),
      // mapping collection
      m_mappingCollection(SolverDeal::createMappingCollection(m_fieldInfo)),
      // dof handler
      m_doFHandler(Agros2D::problem()->calculationMesh())
{       
    // calculation mesh
    m_triangulation = &Agros2D::problem()->calculationMesh();

    // first position of feCollection, quadrature_formulas and face_quadrature_formulas belongs to NONE space
    // this will be used for implementation of different meshes
    m_feCollection->push_back(dealii::FESystem<2>(dealii::FE_Nothing<2>(), fieldInfo->numberOfSolutions()));
    m_quadrature_formulas.push_back(dealii::QGauss<2>(1));
    m_face_quadrature_formulas.push_back(dealii::QGauss<2 - 1>(1));

    // Gauss quadrature and fe collection
    for (unsigned int degree = m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt(); degree <= DEALII_MAX_ORDER; degree++)
    {
        m_feCollection->push_back(dealii::FESystem<2>(dealii::FE_Q<2>(degree), fieldInfo->numberOfSolutions()));
        m_quadrature_formulas.push_back(dealii::QGauss<2>(degree +  QUADRATURE_ORDER_INCREASE));
        m_face_quadrature_formulas.push_back(dealii::QGauss<2-1>(degree + QUADRATURE_ORDER_INCREASE));
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

void SolverDeal::solveLinearSystem(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln)
{
    QTime time;
    time.start();

    switch (m_fieldInfo->matrixSolver())
    {
    case SOLVER_UMFPACK:
        solveUMFPACK(system, rhs, sln);
        break;
    case SOLVER_DEALII:
        solvedealii(system, rhs, sln);
        break;
    case SOLVER_PARALUTION:
        solvePARALUTION(system, rhs, sln);
        break;
    case SOLVER_EXTERNAL:
        solveExternalUMFPACK(system, rhs, sln);
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    // qDebug() << "solved (" << time.elapsed() << "ms )";
}

void SolverDeal::solveUMFPACK(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln)
{
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Direct solver - UMFPACK"));

    if (m_assemble_matrix)
        direct_solver.initialize(system);
    else
        qDebug() << "LU decomposition has been reused";

    direct_solver.vmult(sln, rhs);
}

void SolverDeal::solveExternalUMFPACK(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln)
{    
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Direct solver - UMFPACK (external)"));

    AgrosExternalSolverUMFPack ext(&system, &rhs);
    ext.solve();
    sln = ext.solution();
}

void SolverDeal::solvedealii(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln)
{
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Iterative solver: deal.II (%1, %2)")
                               .arg(iterLinearSolverDealIIMethodString((IterSolverDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt()))
                               .arg(iterLinearSolverDealIIPreconditionerString((PreconditionerDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt())));

    // preconditioner
    dealii::PreconditionSSOR<> preconditioner;

    switch ((PreconditionerDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt())
    {
    case PreconditionerDealII_SSOR:
    {
        // TODO:
        preconditioner.initialize(system, 1.2);
    }
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Preconditioner '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    // solver control
    dealii::SolverControl solver_control(m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt(),
                                         m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble() * system_rhs.l2_norm());

    switch ((IterSolverDealII) m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt())
    {
    case IterSolverDealII_CG:
    {
        dealii::SolverCG<> solver(solver_control);
        solver.solve(system, sln, rhs, preconditioner);
    }
        break;
    case IterSolverDealII_BiCGStab:
    {
        dealii::SolverBicgstab<> solver(solver_control);
        solver.solve(system, sln, rhs, preconditioner);
    }
        break;
    case IterSolverDealII_GMRES:
    {
        dealii::SolverGMRES<> solver(solver_control);
        solver.solve(system, sln, rhs, preconditioner);
    }
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver method (deal.II) '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }
}

void SolverDeal::solvePARALUTION(dealii::SparseMatrix<double> &system, dealii::Vector<double> &rhs, dealii::Vector<double> &sln)
{
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Iterative solver: PARALUTION (%1, %2)")
                               .arg(iterLinearSolverPARALUTIONMethodString((IterSolverPARALUTION) m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONMethod).toInt()))
                               .arg(iterLinearSolverPARALUTIONPreconditionerString((PreconditionerPARALUTION) m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONPreconditioner).toInt())));

    LocalVector<double> sln_paralution;
    LocalVector<double> rhs_paralution;
    LocalMatrix<double> mat_paralution;

    sln_paralution.Allocate("sol", sln.size());
    rhs_paralution.Allocate("rhs", rhs.size());

    import_dealii_matrix(sparsity_pattern, system, &mat_paralution);
    mat_paralution.ConvertToCSR();
    import_dealii_vector(rhs, &rhs_paralution);
    import_dealii_vector(sln, &sln_paralution);

    mat_paralution.MoveToAccelerator();
    sln_paralution.MoveToAccelerator();
    rhs_paralution.MoveToAccelerator();

    // linear solver
    IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double > *ls;
    switch ((IterSolverPARALUTION) m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONMethod).toInt())
    {
    case IterSolverPARALUTION_CG:
        ls = new CG<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case IterSolverPARALUTION_BiCGStab:
        ls = new BiCGStab<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case IterSolverPARALUTION_GMRES:
        ls = new GMRES<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case IterSolverPARALUTION_FGMRES:
        ls = new FGMRES<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case IterSolverPARALUTION_CR:
        ls = new CR<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case IterSolverPARALUTION_IDR:
        ls = new IDR<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver method (PARALUTION) '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    ls->Init(m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble(),
             1e-8,
             1e8,
             m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());

    // preconditioner
    Preconditioner<LocalMatrix<double>, LocalVector<double>, double> *p;
    switch ((PreconditionerPARALUTION) m_fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONPreconditioner).toInt())
    {
    case PreconditionerPARALUTION_Jacobi:
        p = new Jacobi<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case PreconditionerPARALUTION_MultiColoredGS:
        p = new MultiColoredGS<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case PreconditionerPARALUTION_MultiColoredSGS:
        p = new MultiColoredSGS<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case PreconditionerPARALUTION_ILU:
        p = new ILU<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case PreconditionerPARALUTION_MultiColoredILU:
        p = new MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case PreconditionerPARALUTION_MultiElimination:
        p = new MultiElimination<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    case PreconditionerPARALUTION_FSAI:
        p = new FSAI<LocalMatrix<double>, LocalVector<double>, double >();
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Precoditioner (PARALUTION) '%1' is not supported.").arg(m_fieldInfo->matrixSolver()));
        return;
    }

    ls->SetOperator(mat_paralution);
    ls->SetPreconditioner(*p);
    // AMG<LocalMatrix<double>, LocalVector<double>, double > amg;
    // amg.InitMaxIter(1) ;
    // amg.Verbose(0);
    // ls->SetPreconditioner(amg);
    ls->Verbose(1); // 2
    ls->Build();

    mat_paralution.info();

    ls->Solve(rhs_paralution, &sln_paralution);

    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Iterative solver: PARALUTION (residual %1, steps %2)")
                               .arg(ls->GetCurrentResidual())
                               .arg(ls->GetIterationCount()));

    export_dealii_vector(sln_paralution, &sln);

    ls->Clear();
    delete ls;

    p->Clear();
    delete p;

    rhs_paralution.Clear();
    sln_paralution.Clear();
    mat_paralution.Clear();
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

void SolverDeal::refineGrid(bool refine)
{
    // previousSolutionTrans = std::shared_ptr<dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > >(
    //             new dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> >(m_doFHandler));

    // estimated error per cell
    dealii::Vector<float> estimated_error_per_cell(m_triangulation->n_active_cells());

    // estimator
    switch ((AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt())
    {
    case AdaptivityEstimator_Kelly:
        dealii::KellyErrorEstimator<2>::estimate(m_doFHandler,
                                                 m_face_quadrature_formulas,
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

    if (refine)
    {
        // hanging_node_constraints.distribute(m_solution);

        // cout << m_solution.size() << endl;
        m_triangulation->prepare_coarsening_and_refinement();
        // previousSolutionTrans->prepare_for_coarsening_and_refinement(m_solution);
        // previousSolutionTrans->prepare_for_pure_refinement();
        m_triangulation->execute_coarsening_and_refinement();        
    }
}

double SolverDeal::computeNorm()
{
    double h1Norm = 0.0;

    dealii::hp::FEValues<2> hp_fe_values(*m_feCollection, m_quadrature_formulas, dealii::update_values | dealii::update_gradients | dealii::update_JxW_values);

    dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = m_doFHandler.begin_active(), endc_int = m_doFHandler.end();
    for (; cell_int != endc_int; ++cell_int)
    {
        // volume integration
        hp_fe_values.reinit(cell_int);

        const dealii::FEValues<2> &fe_values = hp_fe_values.get_present_fe_values();
        const unsigned int n_q_points = fe_values.n_quadrature_points;

        std::vector<dealii::Vector<double> > solution_values(n_q_points, dealii::Vector<double>(m_fieldInfo->numberOfSolutions()));
        std::vector<std::vector<dealii::Tensor<1,2> > >  solution_grads(n_q_points, std::vector<dealii::Tensor<1,2> >(m_fieldInfo->numberOfSolutions()));

        fe_values.get_function_values(m_solution, solution_values);
        fe_values.get_function_gradients(m_solution, solution_grads);

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

    m_doFHandler.distribute_dofs(*m_feCollection);
    // std::cout << "Number of degrees of freedom: " << m_doFHandler.n_dofs() << std::endl;

    // Handle hanging nodes.
    this->recreateConstraints(!useDirichletLift);

    // create sparsity pattern
    dealii::CompressedSetSparsityPattern csp(m_doFHandler.n_dofs(), m_doFHandler.n_dofs());
    dealii::DoFTools::make_sparsity_pattern(m_doFHandler, csp, all_constraints);
    all_constraints.condense(csp);
    sparsity_pattern.copy_from(csp);

    // reinit system matrix
    system_matrix.reinit(sparsity_pattern);
    system_rhs.reinit(m_doFHandler.n_dofs());
    m_solution.reinit(m_doFHandler.n_dofs());

    // mass matrix (transient)
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        mass_matrix.reinit(sparsity_pattern);
        transient_left_matrix.reinit(sparsity_pattern);
    }
    // qDebug() << "setup (" << time.elapsed() << "ms )";
}

void SolverDeal::setupProblemNonLinearNewton()
{
    m_solution_nonlinear_previous.reinit(m_doFHandler.n_dofs());

    for (dealii::types::global_dof_index dof = 0; dof < m_doFHandler.n_dofs(); dof++)
    {
        if (Dirichlet_constraints.is_constrained(dof))
        {
            (m_solution_nonlinear_previous)(dof) = Dirichlet_constraints.get_inhomogeneity(dof);
        }
    }
}

void SolverDeal::solve()
{
    if (m_fieldInfo->analysisType() == AnalysisType_Transient)
    {
        // int order = 1;
        // for (unsigned int o = 0; o < order - 1; o++)
        //    solution_transient_previous.push_back(dealii::Vector<double>(m_doFHandler.n_dofs()));

        setup(true);
        assembleSystem();

        // initial condition
        m_solution = 0.0;
        dealii::Vector<double> initialSolution(m_doFHandler.n_dofs());

        dealii::VectorTools::interpolate(m_doFHandler,
                                         dealii::ConstantFunction<2>(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble()),
                                         initialSolution);

        // initial step
        FieldSolutionID solutionID(m_fieldInfo, 0, 0, SolutionMode_Normal);
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

            // cout << "actualTimeStep = " << actualTimeStep << ", time = " << m_time << endl;

            // update time dep variables
            Module::updateTimeFunctions(m_time);
            assembleSystem();

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

            // cout << "Agros2D::solutionStore()->size = " << Agros2D::solutionStore()->timeLevels(m_fieldInfo).size() <<
            //         ", Agros2D::problem()->size = " << Agros2D::problem()->timeStepLengths().size() <<
            //         ", stepLengths.size = " << stepLengths.size() <<
            //         ", solutions.size = " << solutions.size() <<
            //         ", actualTimeStep = " << actualTimeStep <<
            //         endl;

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

                    //cout << "ref step = " << step << ", numTimeLevels() = " << Agros2D::problem()->numTimeLevels() << endl;
                }
                else
                {
                    // remove last step
                    // shift time
                    m_time -= actualTimeStep;
                    stepLengths.removeLast();

                    if (step == Agros2D::problem()->numTimeLevels())
                    {
                        solutions.removeLast();

                        if (solutions.size() > 0)
                            m_solution = solutions.last();
                        else
                            m_solution = initialSolution;
                    }
                }

                Agros2D::log()->printMessage(QObject::tr("Solver (%1)").arg(m_fieldInfo->fieldId()),
                                             QObject::tr("Transient step refused"));
            }
            else
            {
                // cout << "step = " << step << ", numTimeLevels() = " << Agros2D::problem()->numTimeLevels() << endl;

                Agros2D::problem()->setActualTimeStepLength(actualTimeStep);
                Agros2D::log()->updateTransientChartInfo(m_time);

                solutionID = FieldSolutionID(m_fieldInfo, step, 0, SolutionMode_Normal);
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

                    refineGrid(false);

                    int min_grid_level = 1;
                    int max_grid_level = 2;

                    if (m_triangulation->n_levels() > max_grid_level)
                        for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(max_grid_level); cell != m_triangulation->end(); ++cell)
                            cell->clear_refine_flag();
                    // for (dealii::Triangulation<2>::active_cell_iterator cell = m_triangulation->begin_active(min_grid_level); cell != m_triangulation->end_active(min_grid_level); ++cell)
                    //     cell->clear_coarsen_flag();

                    all_constraints.distribute(m_solution);

                    dealii::SolutionTransfer<2, dealii::Vector<double>, dealii::hp::DoFHandler<2> > solutionTrans(m_doFHandler);
                    dealii::Vector<double> previousSolution = m_solution;

                    m_triangulation->prepare_coarsening_and_refinement();
                    solutionTrans.prepare_for_coarsening_and_refinement(previousSolution);
                    m_triangulation->execute_coarsening_and_refinement();

                    // reinit
                    setup(true);
                    assembleSystem();

                    // transfer solution
                    solutionTrans.interpolate(previousSolution, m_solution);
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
    // this is a little bit inconsistent. Each solver has pointer to triangulation, but they actually point to
    // mesh object of Agrs2D::problem()
    Agros2D::problem()->propagateBoundaryMarkers();

    if (m_fieldInfo->linearityType() == LinearityType_Linear)
    {
        setup(true);
        // QTime time;
        // time.start();
        assembleSystem();
        // std::cout << "assemble: " << time.elapsed() << ", ndofs: " << m_doFHandler.n_dofs() <<  std::endl;
        // time.start();
        solveLinearSystem(system_matrix, system_rhs, m_solution);
        all_constraints.distribute(m_solution);

        // std::cout << "solve: " << time.elapsed() << std::endl;
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

        assembleSystem();
        solveLinearSystem(system_matrix, system_rhs, m_solution);

        // copy solution
        m_solution_nonlinear_previous.add(-1, m_solution);
        relChangeSol = m_solution_nonlinear_previous.l2_norm() / m_solution.l2_norm() * 100;
        m_solution_nonlinear_previous = m_solution;

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

void SolverDeal::recreateConstraints(bool zeroDirichletLift)
{
    hanging_node_constraints.clear();
    dealii::DoFTools::make_hanging_node_constraints(m_doFHandler, hanging_node_constraints);
    // (from documentation) - this function also resolves chains of constraints.
    hanging_node_constraints.close();

    // Assemble Dirichlet
    // - may introduce additional constraints (but in a different entity, which will be taken care of by merging).
    // Even Newton needs exact Dirichlet lift, so it is calculated always.
    Dirichlet_constraints.clear();
    assembleDirichlet(true);
    Dirichlet_constraints.close();

    // Zero Dirichlet lift for Newton
    if (zeroDirichletLift)
    {
        zero_Dirichlet_constraints.clear();
        assembleDirichlet(false);
        zero_Dirichlet_constraints.close();
    }

    // Merge constraints
    all_constraints.clear();
    all_constraints.merge(hanging_node_constraints);
    if (zeroDirichletLift)
        all_constraints.merge(zero_Dirichlet_constraints);
    else
        all_constraints.merge(Dirichlet_constraints);
    all_constraints.close();
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
    setupProblemNonLinearNewton();

    // initial residual norm

    // first assemble just residual.
    m_assemble_matrix = false;
    assembleSystem();
   // system_rhs.print(std::cout);
    double residualNorm = system_rhs.l2_norm();

    Agros2D::log()->printMessage(QObject::tr("Solver (Newton)"), QObject::tr("Initial residual norm: %1")
        .arg(residualNorm));

    m_assemble_matrix = true;

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
                m_assemble_matrix = false;

                time.start();
                assembleSystem();
                // std::cout << "assemble for Jac reuse (" << time.elapsed() << "ms )" << std::endl;

                system_rhs *= -1.0;

                // since m_assemble_matrix is false, this will reuse the LU decomposition
                time.start();
                solveLinearSystem(system_matrix, system_rhs, m_solution);
                //m_solution.print(std::cout);
                //system_matrix.print(std::cout);
                //system_rhs.print(std::cout);

                // std::cout << "back substitution (" << time.elapsed() << "ms )" << std::endl;

                // Update
                m_solution_nonlinear_previous.add(dampingFactor, m_solution);

                time.start();
                // Calculate residual - m_assemble_matrix is false at this point, so we are not wasting time on matrix assembly.
                assembleSystem();
                // Residual norm.
                residualNorm = system_rhs.l2_norm();

                // std::cout << "assemble residual (" << time.elapsed() << "ms ), norm: "  << residualNorm << std::endl;

                if(residualNorm < previousResidualNorm * m_fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble())
                {
                    jacobianReused = true;
                    numReusedJacobian++;
                }
                else
                {
                    // revert step
                    m_solution_nonlinear_previous.add(-dampingFactor, m_solution);
                    jacobianReused = false;
                    numReusedJacobian = 0;
                }
                // std::cout << "norms: " << residualNorm << ", old: " << previousResidualNorm << " -> " << jacobianReused << std::endl;

                m_assemble_matrix = true;
            }
        }

        if(! jacobianReused)
        {
            time.start();
            assembleSystem();
            
            time.start();
            system_rhs *= -1.0;
            solveLinearSystem(system_matrix, system_rhs, m_solution);
           // m_solution.print(std::cout);
           // system_matrix.print(std::cout);
           // system_rhs.print(std::cout);

            // std::cout << "full system solve (" << time.elapsed() << "ms )" << std::endl;

            // Update.
            m_solution_nonlinear_previous.add(dampingFactor, m_solution);

            // Calculate residual.
            m_assemble_matrix = false;
            assembleSystem();
            m_assemble_matrix = true;
            // Residual norm.
            residualNorm = system_rhs.l2_norm();

            // automatic damping factor
            if ((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt() == DampingType_Automatic)
            {
                previousResidualNorm = residualNorm;
                assert(previousResidualNorm > 0.0);

                // todo: code repetition, get rid of it together with jacobian reuse
                time.start();
                m_assemble_matrix = false;
                assembleSystem();
                m_assemble_matrix = true;
                residualNorm = system_rhs.l2_norm();
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
                    m_solution_nonlinear_previous.add(-previousDampingFactor + dampingFactor, m_solution);

                    // todo: code repetition, get rid of it together with jacobian reuse
                    time.start();
                    m_assemble_matrix = false;
                    assembleSystem();
                    m_assemble_matrix = true;
                    residualNorm = system_rhs.l2_norm();
                    // std::cout << "assemble residual (" << time.elapsed() << "ms ), norm: "  << residualNorm << std::endl;
                }

                dampingSuccessfulSteps++;
                if(dampingSuccessfulSteps > 0)
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
        double relChangeSol = dampingFactor * (m_solution).l2_norm() / m_solution_nonlinear_previous.l2_norm() * 100;
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
    m_solution = m_solution_nonlinear_previous;


    this->Dirichlet_constraints.distribute(m_solution);

    qDebug() << "solve nonlinear total (" << time.elapsed() << "ms )";
}

void SolverDeal::solveAdaptivity()
{
    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
    {
        solveProblem();

        FieldSolutionID solutionID(m_fieldInfo, 0, 0, SolutionMode_Normal);
        SolutionStore::SolutionRunTimeDetails runTime(0.0, 0.0, m_doFHandler.n_dofs());
        Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_triangulation, m_solution), runTime);
    }
    else
    {
        double previousNorm = 0.0;
        for (int i = 0; i < m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt(); i++)
        {

            dealii::Vector<double> previousSolution;
            if (i > 0)
            {
                previousSolution = m_solution;
                refineGrid(true);
            }

            // dealii::Vector<double> previousSolution = m_solution;

            solveProblem();

            // error
            // double error = 1;
            if (i > 0)
            {
                // adaptiveSolutionInterpolated.reinit(m_doFHandler.n_dofs());
                // solutionTrans.refine_interpolate(m_solution, previousSolutionInterpolated);
                // previousSolutionTrans->interpolate(previousSolution, adaptiveSolutionInterpolated);
                // cout << m_solution.size() << " , " << adaptiveSolutionInterpolated.size() << endl;

                // adaptiveSolutionInterpolated.add(-1, m_solution);
                // cout << adaptiveSolutionInterpolated.l2_norm() << endl;
                // error = adaptiveSolutionInterpolated.l2_norm() / m_solution.l2_norm();
            }

            // TODO: do it better
            double norm = computeNorm();
            double error = std::fabs(previousNorm - norm) / norm * 100.0;
            previousNorm = norm;
            // cout << "error: " << error << endl;

            FieldSolutionID solutionID(m_fieldInfo, 0, i, SolutionMode_Normal);
            SolutionStore::SolutionRunTimeDetails runTime(0.0, error, m_doFHandler.n_dofs());
            Agros2D::solutionStore()->addSolution(solutionID, MultiArray(&m_doFHandler, m_triangulation, m_solution), runTime);

            if (i > 0)
                Agros2D::log()->updateAdaptivityChartInfo(m_fieldInfo, 0, i);

            Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Adaptivity step: %1 (error: %2, DOFs: %3)").
                                         arg(i + 1).
                                         arg(error).
                                         arg(m_doFHandler.n_dofs()));
        }
    }
}

// BDF methods
void SolverDeal::transientBDF(const double timeStep,
                              dealii::Vector<double> &solution,
                              const QList<dealii::Vector<double> > solutions,
                              const BDF2Table &bdf2Table)
{
    // LHM = (M + dt * K)
    transient_left_matrix.copy_from(mass_matrix);
    transient_left_matrix *= bdf2Table.matrixFormCoefficient();
    transient_left_matrix.add(timeStep, system_matrix);

    // m = sum(M * SLN)
    dealii::Vector<double> m(m_doFHandler.n_dofs());
    for (int i = 0; i < bdf2Table.order(); i++)
    {
        // m += M * SLNi
        dealii::Vector<double> sln(m_doFHandler.n_dofs());
        mass_matrix.vmult(sln, solutions[solutions.size() - i - 1]);
        m.add(- bdf2Table.vectorFormCoefficient(i), sln);

        // m += Mi * SLNi
        // m.add(- bdf2Table.vectorFormCoefficient(i), solutions[solutions.size() - i - 1]);
    }
    // m += dt * RHS
    m.add(timeStep, system_rhs);

    solveLinearSystem(transient_left_matrix, m, solution);
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
    // todo: this is temporary!!!
    QList<FieldInfo *> fieldInfosSorted;

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

        // qDebug() << "solving " << fieldInfo->name();
        SolverDeal *solverDeal = m_solverDeal[fieldInfo];

        // look for coupling sources
        foreach(FieldInfo* sourceFieldInfo, fieldInfosSorted)
        {
            // todo: check if it is also used!
            if(couplingList()->isCouplingAvailable(sourceFieldInfo, fieldInfo, CouplingType_Weak))
            {

                FieldSolutionID solutionID = Agros2D::solutionStore()->lastTimeAndAdaptiveSolution(sourceFieldInfo, SolutionMode_Normal);
                MultiArray sourceSolution = Agros2D::solutionStore()->multiArray(solutionID);

                solverDeal->setCouplingSource(sourceFieldInfo->fieldId(), sourceSolution.solution());
            }
        }

        solverDeal->solve();
    }
}

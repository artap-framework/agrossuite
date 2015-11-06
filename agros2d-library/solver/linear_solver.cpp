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
#include "linear_solver_external.h"
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

#include "solver/paralution_dealii.hpp"

#include <functional>
#include <typeinfo>

using namespace paralution;

SolverLinearSolver::SolverLinearSolver(const FieldInfo *fieldInfo)
    : m_fieldInfo(fieldInfo)
{

}

void SolverLinearSolver::solveUMFPACK(dealii::SparseMatrix<double> &system,
                                      dealii::Vector<double> &rhs,
                                      dealii::Vector<double> &sln,
                                      bool reuseDecomposition)
{
    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Direct solver - UMFPACK"));

    if (!reuseDecomposition)
        direct_solver.initialize(system);
    else
        qDebug() << "LU decomposition has been reused";

    direct_solver.vmult(sln, rhs);
}

void SolverLinearSolver::solveExternal(dealii::SparseMatrix<double> &system,
                                       dealii::Vector<double> &rhs,
                                       dealii::Vector<double> &sln)
{
    // read command parameters
    QFile f(QString("%1/libs/%2").arg(datadir()).arg(m_fieldInfo->value(FieldInfo::LinearSolverExternalName).toString()));
    if (!f.open(QFile::ReadOnly | QFile::Text))
        throw AgrosSolverException(QObject::tr("Cannot not open external command file."));
    QTextStream in(&f);
    // command at second line
    QStringList commandContent = in.readAll().split("\n");

    // command at second line
    QString name = commandContent.at(0);
    QString command = commandContent.at(1);

    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("External solver - %1").arg(name));

    AgrosExternalSolver ext(&system, &rhs);
    ext.setCommandTemplate(command);
    ext.solve();
    sln = ext.solution();
}

void SolverLinearSolver::solvedealii(dealii::SparseMatrix<double> &system,
                                     dealii::Vector<double> &rhs,
                                     dealii::Vector<double> &sln)
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
                                         m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble() * rhs.l2_norm());

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

template <typename NumberType>
void solvePARALUTION(dealii::SparseMatrix<double> &system,
                     dealii::Vector<double> &rhs,
                     dealii::Vector<double> &sln,
                     dealii::SparsityPattern &sparsity_pattern,
                     const FieldInfo *fieldInfo)
{
    paralution::Paralution_Backend_Descriptor *desc = paralution::_get_backend_descriptor();

    QString backend = QObject::tr("OpenMP");
    if (desc->accelerator && desc->backend == OCL)
        backend = QObject::tr("OpenCL");
    else if (desc->accelerator && desc->backend == GPU)
        backend = QObject::tr("CUDA");
    else if (desc->accelerator && desc->backend == MIC)
        backend = QObject::tr("Xeon MIC");

    Agros2D::log()->printDebug(QObject::tr("Solver"),
                               QObject::tr("Iterative solver: PARALUTION (%1, %2, %3, %4)")
                               .arg(iterLinearSolverPARALUTIONMethodString((IterSolverPARALUTION) fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONMethod).toInt()))
                               .arg(iterLinearSolverPARALUTIONPreconditionerString((PreconditionerPARALUTION) fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONPreconditioner).toInt()))
                               .arg(backend)
                               .arg(strcmp(typeid(NumberType).name(), "d") == 0 ? "double" : "single"));

    QTime time;
    time.start();

    LocalVector<NumberType> sln_paralution;
    LocalVector<NumberType> rhs_paralution;
    LocalMatrix<NumberType> mat_paralution;

    sln_paralution.Allocate("sol", sln.size());
    rhs_paralution.Allocate("rhs", rhs.size());

    import_dealii_matrix(sparsity_pattern, system, &mat_paralution);
    import_dealii_vector(rhs, &rhs_paralution);
    import_dealii_vector(sln, &sln_paralution);

    cout << "matrix and vecs conv = " << time.elapsed() << endl;
    time.start();

    if (!Agros2D::configComputer()->value(Config::Config_DisableAccelerator).toBool())
    {
        mat_paralution.MoveToAccelerator();
        sln_paralution.MoveToAccelerator();
        rhs_paralution.MoveToAccelerator();
    }

    // linear solver
    IterativeLinearSolver<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType > *ls;
    switch ((IterSolverPARALUTION) fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONMethod).toInt())
    {
    case IterSolverPARALUTION_CG:
        ls = new CG<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case IterSolverPARALUTION_BiCGStab:
        ls = new BiCGStab<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case IterSolverPARALUTION_GMRES:
        ls = new GMRES<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case IterSolverPARALUTION_FGMRES:
        ls = new FGMRES<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case IterSolverPARALUTION_CR:
        ls = new CR<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case IterSolverPARALUTION_IDR:
        ls = new IDR<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Solver method (PARALUTION) '%1' is not supported.").arg(fieldInfo->matrixSolver()));
        return;
    }

    ls->Init(fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble(),
             1e-8,
             1e8,
             fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());

    // preconditioner
    Preconditioner<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType> *p;
    switch ((PreconditionerPARALUTION) fieldInfo->value(FieldInfo::LinearSolverIterPARALUTIONPreconditioner).toInt())
    {
    case PreconditionerPARALUTION_Jacobi:
        p = new Jacobi<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case PreconditionerPARALUTION_MultiColoredGS:
        p = new MultiColoredGS<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case PreconditionerPARALUTION_MultiColoredSGS:
        p = new MultiColoredSGS<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case PreconditionerPARALUTION_ILU:
        p = new ILU<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case PreconditionerPARALUTION_MultiColoredILU:
        p = new MultiColoredILU<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case PreconditionerPARALUTION_MultiElimination:
        p = new MultiElimination<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    case PreconditionerPARALUTION_FSAI:
        p = new FSAI<LocalMatrix<NumberType>, LocalVector<NumberType>, NumberType >();
        break;
    default:
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Precoditioner (PARALUTION) '%1' is not supported.").arg(fieldInfo->matrixSolver()));
        return;
    }

    ls->SetOperator(mat_paralution);
    ls->SetPreconditioner(*p);
    // AMG<LocalMatrix<NumberType>, LocalVector<NumberType>, double > amg;
    // amg.InitMaxIter(1) ;
    // amg.Verbose(0);
    // ls->SetPreconditioner(amg);
    ls->Verbose(1); // 2
    ls->Build();

    mat_paralution.info();

    cout << "rezie = " << time.elapsed() << endl;
    time.start();
    ls->Solve(rhs_paralution, &sln_paralution);

    cout << "solve = " << time.elapsed() << endl;

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

void SolverLinearSolver::solvePARALUTIONFloat(dealii::SparseMatrix<double> &system,
                                              dealii::Vector<double> &rhs,
                                              dealii::Vector<double> &sln,
                                              dealii::SparsityPattern &sparsity_pattern)
{
    solvePARALUTION<float>(system, rhs, sln, sparsity_pattern, m_fieldInfo);
}

void SolverLinearSolver::solvePARALUTIONDouble(dealii::SparseMatrix<double> &system,
                                               dealii::Vector<double> &rhs,
                                               dealii::Vector<double> &sln,
                                               dealii::SparsityPattern &sparsity_pattern)
{
    solvePARALUTION<double>(system, rhs, sln, sparsity_pattern, m_fieldInfo);
}

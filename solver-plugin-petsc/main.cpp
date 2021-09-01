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

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "../3rdparty/tclap/CmdLine.h"
#include "../agros-library/util/sparse_io.h"
#include "../agros-library/solver/plugin_solver_interface.h"

#include <petscksp.h>
#include <petscmat.h>
#include <petscblaslapack.h>

class PETScSolverInterface : public QObject, public PluginSolverInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginSolverInterface)
    Q_PLUGIN_METADATA(IID PluginSolverInterface_IID)

public:
    PETScSolverInterface() {}
    virtual ~PETScSolverInterface() {}

    virtual QString name() const { return QString("PETSc"); }
    virtual QStringList methods() const
    {
        QStringList meth;

        meth.append("richardson");
        meth.append("chebyshev");
        meth.append("cg");
        meth.append("gmres");
        meth.append("cgs");
        meth.append("bicg");

        return meth;
    }

    virtual QStringList preconditioners() const
    {
        QStringList precs;

        precs.append("none");
        precs.append("jacobi");
        precs.append("sor");
        precs.append("lu");
        precs.append("cgs");
        precs.append("ilu");
        precs.append("icc");
        precs.append("ksp");
        precs.append("cholesky");
        precs.append("hypre");

        return precs;
    }

    virtual void solve(dealii::SparseMatrix<double> &system,
                       dealii::Vector<double> &rhs,
                       dealii::Vector<double> &sln)
    {
        this->prepare_crs(system);

        PetscErrorCode ierr = -1;
        PetscBool nonzeroguess = PETSC_FALSE;
        MPI_Comm comm = MPI_COMM_SELF;

        int argc = 0;
        char **argv[] = {};

        PetscLogDefaultBegin();

        PetscInitialize(&argc, argv, (char*) 0, " ");
        // PetscInitializeNoArguments();

        PetscInt n_rows = system.m();
        PetscInt n_nonzero_elements = system.n_nonzero_elements();

        Mat A;
        // ierr = MatCreate(comm, &A); CHKERRQ(ierr);
        ierr = MatCreateSeqAIJ(comm, n_rows, n_rows, PETSC_DEFAULT, NULL, &A); CHKERRQ(ierr);
        // ierr = MatSetType(A, MATSEQAIJ); CHKERRQ(ierr);
        // ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n_rows, n_rows); CHKERRQ(ierr);

        // ierr = MatGetOwnershipRange(A, &istart,&iend); CHKERRQ(ierr);
        // std::cout << rank << "   " << istart << "  " << iend << "  " << "\n";

        ierr = MatSeqAIJSetPreallocation(A, n_rows, PETSC_NULL); CHKERRQ(ierr);
        ierr = MatSetFromOptions(A);

        // loop over the elements of the matrix row by row
        for (int row = 0; row < n_rows; row++)
        {
            int col_start = Ap[row];
            int col_end = Ap[row + 1];

            //            for (int i = col_start; i < col_end; i++)
            //            {
            //                int cooRowInd = row + 0;
            //                int cooColInd = Ai[i] + 0;
            //                double value = Ax[i];

            //                std::cout << row << " - (" << cooRowInd << ", " << cooColInd << ") " << value << std::endl;
            //            }
            MatSetValues(A, 1, &row, col_end-col_start, &Ai[col_start], &Ax[col_start], INSERT_VALUES);
        }

        ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
        ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);

        // MatView(A, PETSC_VIEWER_STDOUT_SELF);

        Vec x = nullptr;
        Vec b = nullptr;
        ierr = VecCreateSeq(comm, n_rows, &x); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
        ierr = VecSetFromOptions(x); CHKERRQ(ierr);
        ierr = VecDuplicate(x, &b); CHKERRQ(ierr);

        // ierr = VecGetOwnershipRange(x,&istart,&iend); CHKERRQ(ierr);
        // std::cout << istart << "  ";
        // for (int i = istart; i < iend; i++)

        for (int i = 0; (i < n_rows); i++)
        {
            VecSetValues(b, 1, &i, &rhs[i], INSERT_VALUES);
        }

        ierr = VecAssemblyBegin(b); CHKERRQ(ierr);
        ierr = VecAssemblyEnd(b); CHKERRQ(ierr);
        // VecView(b, PETSC_VIEWER_STDOUT_SELF);

        // Create linear solver context
        KSP ksp = nullptr;
        PC pc = nullptr;
        ierr = KSPCreate(comm, &ksp); CHKERRQ(ierr);
        ierr = KSPSetOperators(ksp, A, A); CHKERRQ(ierr);
        if (this->method.isEmpty())
            this->method = "gmres";
        ierr = KSPSetType(ksp, this->method.toStdString().c_str()); CHKERRQ(ierr);
        ierr = KSPSetInitialGuessNonzero(ksp, PETSC_TRUE); CHKERRQ(ierr);

        // std::string preconditioner = linearSystem->preconditionerArg.getValue();
        // preconditioner
        ierr = KSPGetPC(ksp, &pc); CHKERRQ(ierr);
        ierr = PCSetType(pc, PCHYPRE); CHKERRQ(ierr);
        // PCFactorSetShiftType(pc, MAT_SHIFT_NONZERO);
        // PCFactorSetShiftType(pc, MAT_SHIFT_POSITIVE_DEFINITE);
        ierr = KSPSetFromOptions(ksp); CHKERRQ(ierr);
        ierr = KSPSetUp(ksp); CHKERRQ(ierr);

        ierr = KSPSetTolerances(ksp, 1e-4, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT); CHKERRQ(ierr);
        ierr = KSPSetFromOptions(ksp); CHKERRQ(ierr);

        if (nonzeroguess)
        {
            PetscScalar p = .5;
            ierr = VecSet(x,p); CHKERRQ(ierr);
            ierr = KSPSetInitialGuessNonzero(ksp,PETSC_TRUE); CHKERRQ(ierr);
        }

        ierr = KSPSolve(ksp, b, x); CHKERRQ(ierr);

        KSPConvergedReason reason;
        PetscInt its;
        KSPGetConvergedReason(ksp,&reason);
        if (reason == KSP_DIVERGED_INDEFINITE_PC)
        {
            PetscPrintf(comm, "\nDivergence because of indefinite preconditioner;\n");
            PetscPrintf(comm, "Run the executable again but with -pc_factor_shift_positive_definite option.\n");
        }
        else if (reason < 0)
        {
            PetscPrintf(comm, "\nOther kind of divergence: this should not happen.\n");
        }
        else
        {
            KSPGetIterationNumber(ksp, &its);
            PetscPrintf(comm, "\nConvergence in %d iterations.\n", (int) its);
        }
        PetscPrintf(comm, "\n");

        std::cout << "8" << std::endl;

        sln = dealii::Vector<double>(rhs.size());
        for (int i = 0; i < rhs.size(); i++)
        {
            double value;
            VecGetValues(x, 1, &i, &value);
            // std::cout << value << std::endl;
            sln[i] = value;
        }

        ierr = KSPDestroy(&ksp); CHKERRQ(ierr);
        ierr = VecDestroy(&x); CHKERRQ(ierr);
        ierr = VecDestroy(&b); CHKERRQ(ierr);
        ierr = MatDestroy(&A); CHKERRQ(ierr);

        PetscLogDump("log.txt");
        ierr = PetscFinalize(); CHKERRQ(ierr);

        Ap.clear();
        Ai.clear();
        Ax.clear();
    }
};

#include "main.moc"

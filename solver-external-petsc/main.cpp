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

#include "mpi/mpi.h"

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#define JOB_INIT -1
#define JOB_END -2
#define USE_COMM_WORLD -987654

#include <petscksp.h>
#include <petscmat.h>
#include <petscblaslapack.h>



#include "../../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

class LinearSystemPetscArgs : public LinearSystemArgs
{

};

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        LinearSystemArgs linearSystem("External solver - PETSc", argc, argv);

        Vec x,b;
        Mat  A;
        PetscMPIInt size;
        PetscErrorCode ierr;
        PetscBool nonzeroguess = PETSC_FALSE;
        KSP ksp;
        PC pc;
        PetscInt n_rows = 0;
        PetscInt n = 0;
        PetscInt *column_indicies = NULL;

        PetscInitialize(&argc,&argv, (char*)0," ");
        ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
        int rank;
        ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        std::cout << "rank =  " << rank << std::endl;
        ierr = PetscOptionsGetBool(NULL,"-nonzero_guess", &nonzeroguess,NULL); CHKERRQ(ierr);


        PetscInt *row_indicies = NULL;
        PetscInt *row_lengths = NULL;

        //        double  * globaldata = NULL;
        //        double localdata[3];
        //        const int N = 12;

        //        if (rank == 0)
        //        {
        //            globaldata = new double[N];

        //            for(int i = 0; i < N; i++)
        //            {
        //                globaldata[i] = i+1;
        //            }
        //        }
        //        MPI_Scatter(globaldata, 3, MPI_DOUBLE, localdata, 3, MPI_DOUBLE, 0, PETSC_COMM_WORLD);
        //        for(int i = 0; i < 3; i++)
        //        {
        //            std::cout << "Rank:" << rank << "  " << localdata[i] << std::endl;
        //        }




        //   if (rank == 0)
        {
            linearSystem.readLinearSystem();
            n_rows = linearSystem.n();
            n = linearSystem.nz();
            row_indicies = new PetscInt[n_rows];
            row_lengths = new PetscInt[n_rows];

            for (int i = 0; i < n_rows; i++)
            {
                row_indicies[i] = linearSystem.system_matrix_pattern->rowstart[i];
            }

            column_indicies = reinterpret_cast<PetscInt *>(linearSystem.system_matrix_pattern->colnums);

            for(int i = 0; i < n_rows; i++)
            {
                if (i == (n_rows - 1))
                {
                    row_lengths[i] = n - row_indicies[i];
                }
                else
                {
                    row_lengths[i] = row_indicies[i+1] - row_indicies[i];
                }
            }
        }


        ierr = MatCreate(PETSC_COMM_WORLD, &A); CHKERRQ(ierr);
        ierr = MatSetType(A, MATMPIAIJ); CHKERRQ(ierr);
        ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n_rows, n_rows); CHKERRQ(ierr);
        ierr = MatMPIAIJSetPreallocation(A, 7, PETSC_NULL, 7, PETSC_NULL); CHKERRQ(ierr);
        ierr = MatSeqAIJSetPreallocation(A, 7, NULL);CHKERRQ(ierr);
        ierr = MatSetFromOptions(A);




        int istart, iend;

        ierr = MatGetOwnershipRange(A, &istart,&iend); CHKERRQ(ierr);
        int mm, nn;
        ierr = MatGetLocalSize(A, &mm, &nn);
        std::cout << rank << "   " << mm << "  " << nn << "  " << "\n";

        int l = 0;
        for (int i = istart; (i < iend) && (i < n_rows); i++)
        {
            MatSetValues(A, 1, &i, row_lengths[i], &column_indicies[row_indicies[i]],&linearSystem.system_matrix->val[row_indicies[i]], INSERT_VALUES);
            l++;
        }

        ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
        ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

        MatView(A, PETSC_VIEWER_STDOUT_SELF);

        ierr = VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, n_rows, &x); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
        ierr = VecSetFromOptions(x); CHKERRQ(ierr);
        ierr = VecDuplicate(x,&b); CHKERRQ(ierr);


        ierr = VecGetOwnershipRange(x,&istart,&iend); CHKERRQ(ierr);
        std::cout << istart << "  ";
        for (int i = istart; i < iend; i++)
        {
            VecSetValues(b, 1, &i, &linearSystem.system_rhs->val[i], INSERT_VALUES);
        }

        ierr = VecAssemblyBegin(b); CHKERRQ(ierr);
        ierr = VecAssemblyEnd(b); CHKERRQ(ierr);
        VecView(b, PETSC_VIEWER_STDOUT_SELF);

        //       Create linear solver context
        ierr = KSPCreate(PETSC_COMM_WORLD,&ksp);CHKERRQ(ierr);
        ierr = KSPSetOperators(ksp,A,A,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);


        ierr = KSPGetPC(ksp,&pc);CHKERRQ(ierr);
        ierr = PCSetType(pc, PCHYPRE);CHKERRQ(ierr);
        PCFactorSetShiftType(pc, MAT_SHIFT_NONZERO);
        ierr = KSPSetTolerances(ksp,1.e-10,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT);CHKERRQ(ierr);

        ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);

        if (nonzeroguess)
        {
            PetscScalar p = .5;
            ierr = VecSet(x,p); CHKERRQ(ierr);
            ierr = KSPSetInitialGuessNonzero(ksp,PETSC_TRUE);CHKERRQ(ierr);
        }

        for(int i = 0; i < 1; i++)
        {
            ierr = KSPSolve(ksp, b, x); CHKERRQ(ierr);
        }

        MatView(A, PETSC_VIEWER_STDOUT_SELF);


        //       if (rank == 0)
        {
            for (int i = 0; i < linearSystem.n(); i++)
            {
                VecGetValues(x,1,&i, &linearSystem.system_rhs->val[i]);
            }

            linearSystem.system_sln = linearSystem.system_rhs;

            linearSystem.writeSolution();

            // check solution
            if (linearSystem.hasReferenceSolution())
                status = linearSystem.compareWithReferenceSolution();

            delete [] row_indicies;
            delete [] row_lengths;

        }

        ierr = VecDestroy(&x); CHKERRQ(ierr);
        ierr = VecDestroy(&b); CHKERRQ(ierr); ierr = MatDestroy(&A);CHKERRQ(ierr);
        ierr = KSPDestroy(&ksp); CHKERRQ(ierr);

        ierr = PetscFinalize();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    return 1;
}

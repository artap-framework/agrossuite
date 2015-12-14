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

int main(int argc, char *argv[])
{


    try
    {

        //        ierr = MPI_Init(&argc, &argv);
        //        ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        // std::cout << "Rank " << rank << " started." << std::endl;

        // time
        // MPI_Barrier(MPI_COMM_WORLD);
        // double startTotal = MPI_Wtime();

        // define the problem on the host
        SparsityPatternRW system_matrix_pattern;
        SparseMatrixRW system_matrix;
        VectorRW system_rhs;

        system_matrix_pattern.max_dim;

        std::string slnFileName;
        // command line info
        TCLAP::CmdLine cmd("External solver - MUMPS", ' ');

        TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
        TCLAP::ValueArg<std::string> matrixPatternArg("p", "matrix_pattern", "Matrix pattern", true, "", "string");
        TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
        TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", true, "", "string");
        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");

        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(initialArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        slnFileName = solutionArg.getValue();

        std::ifstream readMatrixSparsityPattern(matrixPatternArg.getValue());
        system_matrix_pattern.block_read(readMatrixSparsityPattern);
        readMatrixSparsityPattern.close();

        std::ifstream readMatrix(matrixArg.getValue());
        // system_matrix.reinit(system_matrix_pattern);
        system_matrix.block_read(readMatrix);
        readMatrix.close();

        std::ifstream readRHS(rhsArg.getValue());
        system_rhs.block_read(readRHS);
        readRHS.close();


        Vec x,b;
        Mat A;
        PetscMPIInt    size;        
        PetscErrorCode ierr;
        PetscBool      nonzeroguess = PETSC_FALSE;
        KSP ksp;
        PC pc ;
        // PetscInt m = 5;


        PetscInitialize(&argc,&argv,(char*)0," ");
        ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);
        if (size != 1) SETERRQ(PETSC_COMM_WORLD,1,"This is a uniprocessor example only!");
        // ierr = PetscOptionsGetInt(NULL,"-n",&m,NULL);CHKERRQ(ierr);  // read parameter from command line
        ierr = PetscOptionsGetBool(NULL,"-nonzero_guess",&nonzeroguess,NULL);CHKERRQ(ierr);

        PetscInt n_rows = system_matrix_pattern.rows;
        PetscInt n = system_matrix.max_len;

        PetscScalar * Matrix = system_matrix.val;
        PetscScalar * vector = new PetscScalar[n_rows];
        PetscInt  * row_indicies = new PetscInt[n_rows];
        PetscInt  * row_lengths = new PetscInt[n_rows];

        ierr = VecCreate(PETSC_COMM_WORLD,&x);CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) x, "Solution");CHKERRQ(ierr);
        ierr = VecSetSizes(x,PETSC_DECIDE,n_rows);CHKERRQ(ierr);
        ierr = VecSetFromOptions(x);CHKERRQ(ierr);
        ierr = VecDuplicate(x,&b);CHKERRQ(ierr);

        for(int i = 0; i < n_rows; i++)
        {
            row_indicies[i] = system_matrix_pattern.rowstart[i];
            vector[i] = system_rhs.val[i];
        }

        PetscInt  * column_indicies = reinterpret_cast<PetscInt *>(system_matrix_pattern.colnums);

        for(int i = 0; i < n_rows; i++)
        {
            if (i == (n_rows - 1))
            {
                row_lengths[i] = n - row_indicies[i];
            } else
            {
                row_lengths[i] = row_indicies[i+1] - row_indicies[i];
            }
        }


        ierr = MatCreateSeqAIJ(PETSC_COMM_WORLD, n_rows, n_rows, 0, row_lengths, &A);
        ierr = MatSetType(A, MATSEQAIJ);


        for(int i = 0; i < n_rows; i++)
        {
            MatSetValues(A, 1, &i, row_lengths[i], &column_indicies[row_indicies[i]],&Matrix[row_indicies[i]], INSERT_VALUES);
        }

        ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
        ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
        // MatView(A, PETSC_VIEWER_STDOUT_SELF);

        for (int i = 0; i < n_rows; i++) {
            VecSetValues(b, 1, &i, &vector[i],INSERT_VALUES);
        }

        VecAssemblyBegin(b);
        VecAssemblyEnd(b);
        // VecView(b, PETSC_VIEWER_STDOUT_SELF);

        //       Create linear solver context
        ierr = KSPCreate(PETSC_COMM_WORLD,&ksp);CHKERRQ(ierr);
        ierr = KSPSetOperators(ksp,A,A,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);

        /*
               Set linear solver defaults for this problem (optional).
               - By extracting the KSP and PC contexts from the KSP context,
                 we can then directly call any KSP and PC routines to set
                 various options.
               - The following four statements are optional; all of these
                 parameters could alternatively be specified at runtime via
                 KSPSetFromOptions();
            */
        ierr = KSPGetPC(ksp,&pc);CHKERRQ(ierr);
        ierr = PCSetType(pc, PCLU);CHKERRQ(ierr);
        ierr = KSPSetTolerances(ksp,1.e-10,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT);CHKERRQ(ierr);

        /*
              Set runtime options, e.g.,
                  -ksp_type <type> -pc_type <type> -ksp_monitor -ksp_rtol <rtol>
              These options will override those specified above as long as
              KSPSetFromOptions() is called _after_ any other customization
              routines.
            */
        ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);

        if (nonzeroguess) {
            PetscScalar p = .5;
            ierr = VecSet(x,p);CHKERRQ(ierr);
            ierr = KSPSetInitialGuessNonzero(ksp,PETSC_TRUE);CHKERRQ(ierr);
        }

        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                                Solve the linear system
               - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
        /*
               Solve linear system
            */
        ierr = KSPSolve(ksp, b, x);CHKERRQ(ierr);

        /*
               View solver info; we could instead use the option -ksp_view to
               print this info to the screen at the conclusion of KSPSolve().
            */
        // ierr = KSPView(ksp,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);


        //  VecView(x, PETSC_VIEWER_STDOUT_SELF);
       //  VecGetArray1d(x,1,0, system_rhs.val);

        for (int i = 0; i < n_rows; i++) {
            VecGetValues(x,1,&i, &system_rhs.val[i]);
        }

        // Check the error

        // ierr = VecAXPY(x,neg_one,u);CHKERRQ(ierr);
        // ierr = VecNorm(x,NORM_2,&norm);CHKERRQ(ierr);
        // ierr = KSPGetIterationNumber(ksp,&its);CHKERRQ(ierr);
        //if (norm > tol) {
        //    ierr = PetscPrintf(PETSC_COMM_WORLD,"Norm of error %G, Iterations %D\n",norm,its);CHKERRQ(ierr);
        //}

        /*
               Free work space.  All PETSc objects should be destroyed when they
               are no longer needed.
            */

        delete [] row_indicies;
        delete [] vector;
        delete [] row_lengths;

        ierr = VecDestroy(&x);CHKERRQ(ierr);
        ierr = VecDestroy(&b);CHKERRQ(ierr); ierr = MatDestroy(&A);CHKERRQ(ierr);
        ierr = KSPDestroy(&ksp);CHKERRQ(ierr);

        /*
               Always call PetscFinalize() before exiting a program.  This routine
                 - finalizes the PETSc libraries as well as MPI
                 - provides summary and diagnostic information if certain runtime
                   options are chosen (e.g., -log_summary).
            */
        ierr = PetscFinalize();        
        std::ofstream writeSln(slnFileName);
        system_rhs.block_write(writeSln);
        writeSln.close();
        // std::cout << "Total time: " << (end - startTotal) << std::endl;


        exit(0);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }

    return 1;
}

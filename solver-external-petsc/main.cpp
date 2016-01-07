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


class LinearSystemPETScArgs : public LinearSystemArgs
{
// another used args (not listed here): -s, -r, -p, -m, -q
public:
    LinearSystemPETScArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
          solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          aggregationTypeArg(TCLAP::ValueArg<std::string>("e", "aggregationType", "AggregationType", false, "", "string")),
          smootherTypeArg(TCLAP::ValueArg<std::string>("o", "smootherType", "SmootherType", false, "", "string")),
          coarseTypeArg(TCLAP::ValueArg<std::string>("z", "coarseType", "CoarseType", false, "", "string")),
          // absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int")),
          multigridArg(TCLAP::SwitchArg("g", "multigrid", "Algebraic multigrid", false))
    {
        cmd.add(solverArg);
        cmd.add(preconditionerArg);
        cmd.add(aggregationTypeArg);
        cmd.add(smootherTypeArg);
        cmd.add(coarseTypeArg);
        // cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
        cmd.add(multigridArg);
    }

    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<std::string> preconditionerArg;
    TCLAP::ValueArg<std::string> aggregationTypeArg;
    TCLAP::ValueArg<std::string> smootherTypeArg;
    TCLAP::ValueArg<std::string> coarseTypeArg;
    // TCLAP::ValueArg<double> absTolArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
    TCLAP::SwitchArg multigridArg;
};

LinearSystemPETScArgs *createLinearSystem(std::string extSolverName, int argc, char *argv[])
{
    LinearSystemPETScArgs *linearSystem = new LinearSystemPETScArgs(extSolverName, argc, argv);
    linearSystem->readLinearSystem();
    // create empty solution vector (Agros2D)
    linearSystem->system_sln->resize(linearSystem->system_rhs->max_len);
    linearSystem->convertToCOO();

    return linearSystem;
}
// usage:
// LinearSystemTrilinosArgs *linearSystem = nullptr;
// ...
// linearSystem = createLinearSystem("External solver - TRILINOS", argc, argv);
// -----
// get parameters to local value
// double relTol = linearSystem->relTolArg.getValue();
// int maxIter = linearSystem->maxIterArg.getValue();

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        LinearSystemPETScArgs linearSystem("External solver - PETSc", argc, argv);

        Vec x,b;
        Mat A;
        PetscMPIInt size;
        PetscErrorCode ierr;
        PetscBool nonzeroguess = PETSC_FALSE;
        KSP ksp;
        PC pc;
        // PetscInt m = 5;

        PetscInitialize(&argc,&argv, (char*)0," ");
        ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
        int rank;
        ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        std::cout << "rank =  " << rank << std::endl;
        // if (size != 1) SETERRQ(PETSC_COMM_WORLD, 1, "This is a uniprocessor example only!");
        ierr = PetscOptionsGetBool(NULL,"-nonzero_guess", &nonzeroguess,NULL); CHKERRQ(ierr);

        if (rank == 0)
        {
            linearSystem.readLinearSystem();
            PetscInt n_rows = linearSystem.n();
            PetscInt n = linearSystem.nz();

            PetscScalar *matrix = linearSystem.system_matrix->val;
            PetscScalar *vector = linearSystem.system_rhs->val;
            PetscInt *row_indicies = new PetscInt[n_rows];
            PetscInt *row_lengths = new PetscInt[n_rows];

            ierr = VecCreate(PETSC_COMM_WORLD,&x); CHKERRQ(ierr);
            ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
            ierr = VecSetSizes(x,PETSC_DECIDE,n_rows); CHKERRQ(ierr);
            ierr = VecSetFromOptions(x); CHKERRQ(ierr);
            ierr = VecDuplicate(x,&b); CHKERRQ(ierr);

            for (int i = 0; i < n_rows; i++)
            {
                row_indicies[i] = linearSystem.system_matrix_pattern->rowstart[i];
            }

            PetscInt *column_indicies = reinterpret_cast<PetscInt *>(linearSystem.system_matrix_pattern->colnums);

            for (int i = 0; i < n_rows; i++)
            {
                VecSetValues(b, 1, &i, &linearSystem.system_rhs->val[i], INSERT_VALUES);
            }

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

            ierr = MatCreateSeqAIJ(PETSC_COMM_WORLD, n_rows, n_rows, 0, row_lengths, &A);
            ierr = MatSetType(A, MATSEQAIJ);

            for (int i = 0; i < n_rows; i++)
            {
                MatSetValues(A, 1, &i, row_lengths[i], &column_indicies[row_indicies[i]],&matrix[row_indicies[i]], INSERT_VALUES);
            }

            ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
            ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
            // MatView(A, PETSC_VIEWER_STDOUT_SELF);

            VecAssemblyBegin(b);
            VecAssemblyEnd(b);
            // VecView(b, PETSC_VIEWER_STDOUT_SELF);

            delete [] row_indicies;
            delete [] row_lengths;
        }

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
        PCFactorSetShiftType(pc, MAT_SHIFT_NONZERO);
        ierr = KSPSetTolerances(ksp,1.e-10,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT);CHKERRQ(ierr);

        /*
              Set runtime options, e.g.,
                  -ksp_type <type> -pc_type <type> -ksp_monitor -ksp_rtol <rtol>
              These options will override those specified above as long as
              KSPSetFromOptions() is called _after_ any other customization
              routines.
            */
        ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);

        if (nonzeroguess)
        {
            PetscScalar p = .5;
            ierr = VecSet(x,p); CHKERRQ(ierr);
            ierr = KSPSetInitialGuessNonzero(ksp,PETSC_TRUE);CHKERRQ(ierr);
        }

        /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                                Solve the linear system
               - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
        /*
               Solve linear system
            */
        ierr = KSPSolve(ksp, b, x); CHKERRQ(ierr);

        /*
               View solver info; we could instead use the option -ksp_view to
               print this info to the screen at the conclusion of KSPSolve().
            */
        // ierr = KSPView(ksp,PETSC_VIEWER_STDOUT_WORLD);CHKERRQ(ierr);


        //  VecView(x, PETSC_VIEWER_STDOUT_SELF);
        //  VecGetArray1d(x,1,0, system_rhs.val);

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


        /*
               Always call PetscFinalize() before exiting a program.  This routine
                 - finalizes the PETSc libraries as well as MPI
                 - provides summary and diagnostic information if certain runtime
                   options are chosen (e.g., -log_summary).
            */

        if (rank == 0)
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

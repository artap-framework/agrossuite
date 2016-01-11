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
#include <set>

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


KSPType solver(std::string solver)
{
    if( solver == "richardson")
        return KSPRICHARDSON;
    else if( solver == "chebyshev")
        return KSPCHEBYSHEV;
    else
        return "none";
}


PCType preConditioner(std::string preConditioner)
{
    if( preConditioner == "jacobi")
        return PCJACOBI;
    else if (preConditioner == "hypre")
        return PCHYPRE;
    else if (preConditioner == "sor")
        return PCSOR;
    else if (preConditioner == "lu")
        return PCLU;
    else if (preConditioner == "mg")
        return PCMG;
    else if (preConditioner == "shell")
        return PCSHELL;
    else if (preConditioner == "bjacobi")
        return PCBJACOBI;
    else if (preConditioner == "eisenstat")
        return PCEISENSTAT;
    else
        return "none";
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;


        LinearSystemPETScArgs *linearSystem = NULL;
        linearSystem = createLinearSystem("External solver - PETSc", argc, argv);
        auto timeStart = std::chrono::steady_clock::now();

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
        // ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
        int rank;
        // ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        // std::cout << "rank =  " << rank << std::endl;
        ierr = PetscOptionsGetBool(NULL,"-nonzero_guess", &nonzeroguess,NULL); CHKERRQ(ierr);


        PetscInt *row_indicies = NULL;
        PetscInt *row_lengths = NULL;



        //   if (rank == 0)
        {
            n_rows = linearSystem->n();
            n = linearSystem->nz();
            row_indicies = new PetscInt[n_rows];
            row_lengths = new PetscInt[n_rows];
            // diagonal = new PetscInt[n_rows];
            // offDiagonal = new PetscInt[n_rows];

            for (int i = 0; i < n_rows; i++)
            {
                row_indicies[i] = linearSystem->system_matrix_pattern->rowstart[i];
            }

            column_indicies = reinterpret_cast<PetscInt *>(linearSystem->system_matrix_pattern->colnums);

            int localSize  = n_rows / size;
            int localSizeLast = n_rows % size;


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

//        for(int i = 0; i < n_rows; i++)
//        {
//            std::cout << row_indicies[i] << "  ";
//        }
//        std::cout << std::endl;
//        std::cout << "Columns:";
//        std::cout << std::endl;

//        std::cout << std::endl;

//        for(int i = 0; i < n; i++)
//        {
//            std::cout << column_indicies[i] << "  ";
//        }


//        std::cout << std::endl;

//        for(int j = 0; j < n; j++)
//        {
//            std::cout << linearSystem->system_matrix->val[j] << "  ";
//        }
//        std::cout << std::endl;



        linearSystem->setInfoNumOfProc(size);
        ierr = MatCreate(PETSC_COMM_WORLD, &A); CHKERRQ(ierr);
        ierr = MatSetType(A, MATSEQAIJ); CHKERRQ(ierr);
        int istart, iend;


        ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n_rows, n_rows); CHKERRQ(ierr);

        // ierr = MatGetOwnershipRange(A, &istart,&iend); CHKERRQ(ierr);
        // std::cout << rank << "   " << istart << "  " << iend << "  " << "\n";

        // ierr = MatMPIAIJSetPreallocation(A, n_rows, PETSC_NULL, n_rows, PETSC_NULL); CHKERRQ(ierr);
        ierr = MatSeqAIJSetPreallocation(A, 0, row_lengths);CHKERRQ(ierr);
        ierr = MatSetFromOptions(A);




        // ierr = MatGetOwnershipRange(A, &istart,&iend); CHKERRQ(ierr);
        // int mm, nn;
        // ierr = MatGetLocalSize(A, &mm, &nn);
        // std::cout << rank << "   " << mm << "  " << nn << "  " << "\n";

        linearSystem->setInfoTimeReadMatrix(elapsedSeconds(timeStart));
        for (int i = 0; i < n_rows; i++)
        {
            MatSetValues(A, 1, &i, row_lengths[i], &column_indicies[row_indicies[i]],&linearSystem->system_matrix->val[row_indicies[i]], INSERT_VALUES);

        }


        ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
        ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

        // MatView(A, PETSC_VIEWER_STDOUT_SELF);

        ierr = VecCreateSeq(PETSC_COMM_WORLD, n_rows, &x); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
        ierr = VecSetFromOptions(x); CHKERRQ(ierr);
        ierr = VecDuplicate(x,&b); CHKERRQ(ierr);


        // ierr = VecGetOwnershipRange(x,&istart,&iend); CHKERRQ(ierr);
        // std::cout << istart << "  ";
        // for (int i = istart; i < iend; i++)

        for (int i = 0; (i < n_rows); i++)
        {
            VecSetValues(b, 1, &i, &linearSystem->system_rhs->val[i], INSERT_VALUES);
        }

        ierr = VecAssemblyBegin(b); CHKERRQ(ierr);
        ierr = VecAssemblyEnd(b); CHKERRQ(ierr);
        // VecView(b, PETSC_VIEWER_STDOUT_SELF);

        //       Create linear solver context
        ierr = KSPCreate(PETSC_COMM_WORLD,&ksp);CHKERRQ(ierr);
        ierr = KSPSetOperators(ksp,A,A,DIFFERENT_NONZERO_PATTERN);CHKERRQ(ierr);

        std::string preconditioner = linearSystem->preconditionerArg.getValue();

        // preconditioner
        PCType pcArg = preConditioner(preconditioner);
        ierr = KSPGetPC(ksp,&pc);CHKERRQ(ierr);
        ierr = PCSetType(pc, pcArg);CHKERRQ(ierr);
        linearSystem->setInfoSolverPreconditionerName(preconditioner);

        // solver
        std::string solver_name = linearSystem->solverArg.getValue();
        KSPType kspArg = solver(solver_name);
        if(kspArg != "none")
        {
            ierr = KSPSetType(ksp, kspArg);
            linearSystem->setInfoSolverSolverName(solver_name);
        }
        else
            linearSystem->setInfoSolverSolverName("default");


        PCFactorSetShiftType(pc, MAT_SHIFT_NONZERO);
        ierr = KSPSetTolerances(ksp,1.e-4,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT);CHKERRQ(ierr);

        ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);

        if (nonzeroguess)
        {
            PetscScalar p = .5;
            ierr = VecSet(x,p); CHKERRQ(ierr);
            ierr = KSPSetInitialGuessNonzero(ksp,PETSC_TRUE);CHKERRQ(ierr);
        }

        auto timeSolveStart = std::chrono::steady_clock::now();
        ierr = KSPSolve(ksp, b, x); CHKERRQ(ierr);
        linearSystem->setInfoTimeSolver(elapsedSeconds(timeSolveStart));

        // MatView(A, PETSC_VIEWER_STDOUT_SELF);


        //       if (rank == 0)
        {
            for (int i = 0; i < linearSystem->n(); i++)
            {
                VecGetValues(x,1,&i, &linearSystem->system_rhs->val[i]);
            }

            linearSystem->system_sln = linearSystem->system_rhs;

            linearSystem->writeSolution();

            // check solution
            if (linearSystem->hasReferenceSolution())
                status = linearSystem->compareWithReferenceSolution();

            linearSystem->setInfoTimeTotal(elapsedSeconds(timeStart));

            delete [] row_indicies;
            delete [] row_lengths;

            if (linearSystem->verbose() > 0)
            {
                linearSystem->printStatus();

                if (linearSystem->verbose() > 2)
                    linearSystem->exportStatusToFile();
            }

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

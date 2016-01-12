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
          absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int")),
          multigridArg(TCLAP::SwitchArg("g", "multigrid", "Algebraic multigrid", false))
    {
        cmd.add(solverArg);
        cmd.add(preconditionerArg);
        cmd.add(aggregationTypeArg);
        cmd.add(smootherTypeArg);
        cmd.add(coarseTypeArg);
        cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
        cmd.add(multigridArg);
    }

    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<std::string> preconditionerArg;
    TCLAP::ValueArg<std::string> aggregationTypeArg;
    TCLAP::ValueArg<std::string> smootherTypeArg;
    TCLAP::ValueArg<std::string> coarseTypeArg;
    TCLAP::ValueArg<double> absTolArg;
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
    if(solver == "richardson")
        return KSPRICHARDSON;
    else if( solver == "chebyshev")
        return KSPCHEBYSHEV;
    else if( solver == "cg")
        return KSPCG;
    else if (solver == "groppcg")
        return KSPGROPPCG;
    else if (solver == "pipecg")
        return KSPPIPECG;
    else if (solver == "cgne")
        return   KSPCGNE;
    else if (solver == "nash")
        return KSPNASH;
    else if (solver == "stcg")
        return KSPSTCG;
    else if (solver == "gltr")
        return KSPGLTR;
    else if (solver == "gmres")
        return KSPGMRES;
    else if (solver == "fgmres")
        return KSPFGMRES;
    else if (solver == "lgmres")
        return KSPLGMRES;
    else if (solver == "dgmres")
        return KSPLGMRES;
    else if (solver == "pgmres")
        return KSPPGMRES;
    else if (solver == "tcqmr")
        return KSPTCQMR;
    else if (solver == "bcgs")
        return KSPBCGS;
    else if (solver == "ibcgs")
        return KSPIBCGS;
    else if (solver == "fbcgs")
        return KSPFBCGS;
    else if (solver == "fbcgsr")
        return KSPFBCGSR;
    else if (solver == "bcgsl")
        return KSPBCGSL;
    else if (solver == "cgs")
        return KSPCGS;
    else if (solver == "tfqmr")
        return KSPTFQMR;
    else if (solver == "cr")
        return KSPCR;
    else if (solver == "pipecr")
        return KSPPIPECR;
    else if (solver == "lsqr")
        return KSPLSQR;
    else if (solver == "preonly")
        return KSPPREONLY;
    else if (solver == "qcg")
        return KSPQCG;
    else if (solver == "bicg")
        return KSPBICG;
    else if (solver == "minres")
        return KSPMINRES;
    else if (solver == "symmlq")
        return KSPSYMMLQ;
    else if (solver == "lcd")
        return KSPLCD;
    else if (solver == "python")
        return KSPPYTHON;
    else if (solver == "gcr")
        return KSPGCR;
    else if (solver == "specest")
        return KSPSPECEST;
    else
        return KSPRICHARDSON;
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
    else if (preConditioner == "ilu")
        return PCILU;
    else if (preConditioner == "icc")
        return PCICC;
    else if (preConditioner == "asm")
        return PCASM;
    else if (preConditioner == "gasm")
        return PCGASM;
    else if (preConditioner == "ksp")
        return PCKSP;
    else if (preConditioner == "composite")
        return PCCOMPOSITE;
    else if (preConditioner == "redundant")
        return PCREDUNDANT;
    else if (preConditioner == "spai")
        return PCSPAI;
    else if (preConditioner == "nn")
        return PCNN;
    else if (preConditioner == "cholesky")
        return PCCHOLESKY;
    else if (preConditioner == "pbjacobi")
        return PCPBJACOBI;
    else if (preConditioner == "mat")
        return PCMAT;
    else if (preConditioner == "parms")
        return PCPARMS;
    else if (preConditioner == "fieldsplit")
        return PCFIELDSPLIT;
    else if (preConditioner == "fieldsplit")
        return PCFIELDSPLIT;
    else if (preConditioner == "tfs")
        return PCTFS;
    else if (preConditioner == "ml")
        return PCML;
    else if (preConditioner == "galerkin")
        return PCGALERKIN;
    else if (preConditioner == "exotic")
        return PCEXOTIC;
    else if (preConditioner == "hmpi")
        return PCHMPI;
    else if (preConditioner == "supportgraph")
        return PCSUPPORTGRAPH;
    else if (preConditioner == "asa")
        return PCASA;
    else if (preConditioner == "cp")
        return PCCP;
    else if (preConditioner == "bfbt")
        return PCBFBT;
    else if (preConditioner == "lsc")
        return PCLSC;
    else if (preConditioner == "python")
        return PCPYTHON;
    else if (preConditioner == "pfmg")
        return PCPFMG;
    else if (preConditioner == "syspfmg")
        return PCSYSPFMG;
    else if (preConditioner == "redistribute")
        return PCREDISTRIBUTE;
    else if (preConditioner == "svd")
        return PCSVD;
    else if (preConditioner == "gamg")
        return PCSVD;
    else if (preConditioner == "sacusp")
        return PCSACUSP; /* these four run on NVIDIA GPUs using CUSP */
    else if (preConditioner == "sacusppoly")
        return PCSACUSPPOLY;
    else if (preConditioner == "bicgstabcusp")
        return PCBICGSTABCUSP;
    else if (preConditioner == "ainvcusp")
        return PCAINVCUSP;
    else if (preConditioner == "bddc")
        return PCBDDC;
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
        ierr = MPI_Comm_size(PETSC_COMM_WORLD, &size); CHKERRQ(ierr);
        int rank;
        ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        // std::cout << "rank =  " << rank << std::endl;
        ierr = PetscOptionsGetBool(NULL,"-nonzero_guess", &nonzeroguess,NULL); CHKERRQ(ierr);


        PetscInt *row_indicies = NULL;
        PetscInt *row_lengths = NULL;


        n_rows = linearSystem->n();
        n = linearSystem->nz();
        column_indicies = new PetscInt[n];
        row_indicies = new PetscInt[n_rows];
        row_lengths = new PetscInt[n_rows];
        int * diagonal = new PetscInt[n_rows];
        int * offDiagonal = new PetscInt[n_rows];

        for (int i = 0; i < n_rows; i++)
        {
            row_indicies[i] = linearSystem->system_matrix_pattern->rowstart[i];
        }

        for (int i = 0; i < n; i++)
        {
            column_indicies[i] = linearSystem->system_matrix_pattern->colnums[i];

        }


        int maxlen = 0;
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
            // std::cout << row_lengths[i] << "  ";
            if ((row_lengths[i]) > maxlen)
            {
                maxlen = row_lengths[i];
            }

        }



        linearSystem->setInfoNumOfProc(size);
        ierr = MatCreate(PETSC_COMM_WORLD, &A); CHKERRQ(ierr);
        ierr = MatSetType(A, MATMPIAIJ); CHKERRQ(ierr);
        int istart, iend;
        ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, n_rows, n_rows); CHKERRQ(ierr);

        ierr = MatSeqAIJSetPreallocation(A, 0, row_lengths);CHKERRQ(ierr);
        ierr = MatMPIAIJSetPreallocation(A, n_rows, NULL, n_rows, NULL); CHKERRQ(ierr);
        ierr = MatSetFromOptions(A);
        ierr = MatGetOwnershipRange(A, &istart,&iend); CHKERRQ(ierr);
        // std::cout << "Process:" << rank << " Local Size:" << localSize << " istart:" << istart << " iend:" << iend << std::endl;

        int j = 0;
        for(int i = 0; i < n_rows; i++)
        {
            int diag = 0;
            int offDiag = 0;

            for(int k = 0; k < row_lengths[i]; k++)
            {
                if((column_indicies[j] >= istart ) && (column_indicies[j] < iend ))
                {
                    diag++;
                } else
                {
                    offDiag++;
                }
                j++;
            }

            diagonal[i] = row_lengths[i] +1;
            offDiagonal[i] = offDiag;
        }

        //        if(rank == 0)
        //        {
        //            for(int i = 0; i < n_rows; i++)
        //            {
        //                std::cout << diagonal[i] + offDiagonal[i] << "  " <<  row_lengths[i] << std::endl;
        //            }
        //        }



        ierr = MatMPIAIJSetPreallocation(A, maxlen, NULL, maxlen, NULL); CHKERRQ(ierr);
        // ierr = MatMPIAIJSetPreallocationCSR(A, row_indicies, column_indicies, NULL); CHKERRQ(ierr);

        linearSystem->setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        for (int i = istart; i < iend; i++)
        {
            // std::cout << "row:" << i << " row lengths:" << row_lengths[i] << "  " << column_indicies[row_indicies[i]+1] << std::endl;
            MatSetValues(A, 1, &i, row_lengths[i], &column_indicies[row_indicies[i]],&linearSystem->system_matrix->val[row_indicies[i]], INSERT_VALUES);
        }


        ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
        ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

        MatInfo matinfo;
        MatGetInfo(A,MAT_LOCAL,&matinfo);
        // std::cout  << "Dnnz:" << (PetscInt)matinfo.nz_used << std::endl;

        // MatView(A, PETSC_VIEWER_STDOUT_SELF);

        ierr = VecCreateMPI(PETSC_COMM_WORLD, PETSC_DECIDE, n_rows, &x); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
        ierr = VecSetFromOptions(x); CHKERRQ(ierr);
        ierr = VecDuplicate(x,&b); CHKERRQ(ierr);


        ierr = VecGetOwnershipRange(x,&istart,&iend); CHKERRQ(ierr);
        for (int i = istart; i < iend; i++)

            for (int i = istart; (i < iend); i++)
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
        PCHYPRESetType(pc, "boomeramg");
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

        PetscReal relTol;
        if (linearSystem->relTolArg.isSet())
        {
            relTol = linearSystem->relTolArg.getValue();
        }
        else relTol = PETSC_DEFAULT;

        PetscReal absTol;
        if (linearSystem-> absTolArg.isSet())
        {
            absTol = linearSystem->absTolArg.getValue();
        }
        else absTol = PETSC_DEFAULT;

        PetscInt maxIter;
        if (linearSystem-> maxIterArg.isSet())
        {
            maxIter = linearSystem->maxIterArg.getValue();
        }
        else maxIter = PETSC_DEFAULT;


        ierr = KSPSetTolerances(ksp, relTol, absTol, PETSC_DEFAULT, maxIter);CHKERRQ(ierr);

        ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);

        if (nonzeroguess)
        {
            PetscScalar p = .1;
            ierr = VecSet(x,p); CHKERRQ(ierr);
            ierr = KSPSetInitialGuessKnoll(ksp,PETSC_TRUE);CHKERRQ(ierr);
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

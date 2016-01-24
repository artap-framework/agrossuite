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

int rank = 0; // MPI process rank

class LinearSystemPETScArgs : public LinearSystemArgs
{
    // another used args (not listed here): -s, -r, -p, -m, -q
public:
    LinearSystemPETScArgs(const std::string &name, int argc, char *argv[])
        : LinearSystemArgs(name, argc, argv),
          comm(PETSC_COMM_WORLD)
    {
        MPI_Comm_size(PETSC_COMM_WORLD, &infoNumOfProc);
        MPI_Comm_rank(PETSC_COMM_WORLD, &rank);
        // comm = (size == 1) ? PETSC_COMM_SELF : PETSC_COMM_WORLD;

        if (infoParameterPreconditioner.empty())
        {
            if (comm == PETSC_COMM_WORLD)
                infoParameterPreconditioner = "bjacobi";
            else
                infoParameterPreconditioner = "jacobi";
        }

        if (infoParameterSolver.empty())
            infoParameterSolver = "richardson";
    }

    MPI_Comm comm;
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

KSPType solver(LinearSystemPETScArgs *linearSystem, std::string solver)
{
    if (solver == "richardson")
        return KSPRICHARDSON;
    else if (solver == "chebyshev")
        return KSPCHEBYSHEV;
    else if (solver == "cg")
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
    else
        assert(0);
}

PCType preConditioner(LinearSystemPETScArgs *linearSystem, std::string preConditioner)
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
        return PCGAMG;
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
        assert(0);
}

PetscErrorCode assembleRHS(LinearSystemPETScArgs *linearSystem, Vec &b)
{
    int ierr = -1;
    int istart = 0;
    int iend = linearSystem->n();

    if (linearSystem->comm == PETSC_COMM_WORLD)
        ierr = VecGetOwnershipRange(b, &istart, &iend); CHKERRQ(ierr);

    // local assemble
    PetscInt *vecIdx = new PetscInt[iend - istart];
    PetscScalar *vecVal = new PetscScalar[iend - istart];
    for (int i = 0; i < iend - istart; i++)
    {
        vecIdx[i] = istart + i;
        vecVal[i] = linearSystem->system_rhs->val[istart + i];
    }

    ierr = VecSetValues(b, iend - istart, vecIdx, vecVal, INSERT_VALUES); CHKERRQ(ierr);

    ierr = VecAssemblyBegin(b); CHKERRQ(ierr);
    ierr = VecAssemblyEnd(b); CHKERRQ(ierr);

    PetscFree(vecIdx);
    PetscFree(vecVal);
}

void getCSR(LinearSystemPETScArgs *linearSystem, int start, int end,
            PetscInt *csrRowPtr, PetscInt *csrColInd, PetscScalar *csrVal = nullptr)
{
    // loop over the elements of the matrix row by row
    unsigned int index = 0;
    for (unsigned int row = 0; row < end - start; row++)
    {
        std::size_t col_start = linearSystem->system_matrix_pattern->rowstart[row + start];
        std::size_t col_end = linearSystem->system_matrix_pattern->rowstart[row + start + 1];

        csrRowPtr[row] = index;

        for (unsigned int i = col_start; i < col_end; i++)
        {
            csrColInd[index] = linearSystem->system_matrix_pattern->colnums[i];
            if (csrVal)
                csrVal[index] = linearSystem->matA[i];

            index++;
        }
    }
    csrRowPtr[end - start] = index;
}

PetscErrorCode assembleMatrix(LinearSystemPETScArgs *linearSystem, Mat &A)
{
    int ierr = -1;

    // preallocate whole matrix
    if (linearSystem->comm == PETSC_COMM_WORLD)
    {
        int istart = 0;
        int iend = linearSystem->n();

        PetscInt *csrRowPtr = new PetscInt[linearSystem->n() + 1];
        PetscInt *csrColInd = new PetscInt[linearSystem->nz()];

        getCSR(linearSystem, 0, linearSystem->n(), csrRowPtr, csrColInd);

        // preallocate whole matrix
        ierr = MatMPIAIJSetPreallocationCSR(A, csrRowPtr, csrColInd, PETSC_NULL); CHKERRQ(ierr);
        ierr = MatGetOwnershipRange(A, &istart, &iend); CHKERRQ(ierr);

        PetscFree(csrRowPtr);
        PetscFree(csrColInd);

        // local assemble
        int nzLocal = 0;
        for (unsigned int row = istart; row < iend; row++)
            nzLocal += linearSystem->system_matrix_pattern->rowstart[row + 1] - linearSystem->system_matrix_pattern->rowstart[row];

        PetscInt *csrRowPtrLocal = new PetscInt[iend - istart + 1];
        PetscInt *csrColIndLocal = new PetscInt[nzLocal];
        PetscScalar *csrValLocal = new PetscScalar[nzLocal];

        getCSR(linearSystem, istart, iend, csrRowPtrLocal, csrColIndLocal, csrValLocal);

        // the i, j, and a arrays ARE copied by this routine
        ierr = MatCreateMPIAIJWithArrays(linearSystem->comm, iend - istart, PETSC_DECIDE, PETSC_DETERMINE, linearSystem->n(),
                                         csrRowPtrLocal, csrColIndLocal, csrValLocal, &A); CHKERRQ(ierr);

        PetscFree(csrRowPtrLocal);
        PetscFree(csrColIndLocal);
        PetscFree(csrValLocal);
    }
    else
    {
        PetscInt *csrRowPtr = new PetscInt[linearSystem->n() + 1];
        PetscInt *csrColInd = new PetscInt[linearSystem->nz()];
        PetscScalar *csrVal = new PetscScalar[linearSystem->nz()];

        getCSR(linearSystem, 0, linearSystem->n(), csrRowPtr, csrColInd, csrVal);

        ierr = MatSeqAIJSetPreallocationCSR(A, csrRowPtr, csrColInd, csrVal); CHKERRQ(ierr);
        // the i, j, and a arrays are not copied by this routine
        ierr = MatCreateSeqAIJWithArrays(linearSystem->comm, linearSystem->n(), linearSystem->n(),
                                         csrRowPtr, csrColInd, csrVal, &A); CHKERRQ(ierr);

    }

    //  MatView(A, PETSC_VIEWER_STDOUT_SELF);

    ierr = MatAssemblyBegin(A, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
    ierr = MatAssemblyEnd(A, MAT_FINAL_ASSEMBLY); CHKERRQ(ierr);
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;
        PetscErrorCode ierr = -1;

        auto timeStart = std::chrono::steady_clock::now();

        PetscInitialize(&argc, &argv, (char*) 0, " ");
        LinearSystemPETScArgs *linearSystem = createLinearSystem("External solver - PETSc", argc, argv);

        if (rank == 0)
        {
            if (linearSystem->comm == PETSC_COMM_WORLD)
                std::cout << "version: mpi" << std::endl;
            else
                std::cout << "version: seq" << std::endl;
        }

        // create vector
        Vec x,b;
        ierr = VecCreateMPI(linearSystem->comm, PETSC_DECIDE, linearSystem->n(), &x); CHKERRQ(ierr);
        ierr = VecCreateMPI(linearSystem->comm, PETSC_DECIDE, linearSystem->n(), &b); CHKERRQ(ierr);
        /*
        ierr = VecCreate(linearSystem->comm, &x); CHKERRQ(ierr);
        ierr = VecCreate(linearSystem->comm, &b); CHKERRQ(ierr);
        ierr = VecSetSizes(x, PETSC_DECIDE, linearSystem->n()); CHKERRQ(ierr);
        ierr = VecSetSizes(b, PETSC_DECIDE, linearSystem->n()); CHKERRQ(ierr);
        */
        ierr = PetscObjectSetName((PetscObject) x, "Solution"); CHKERRQ(ierr);
        ierr = PetscObjectSetName((PetscObject) b, "RHS"); CHKERRQ(ierr);
        ierr = VecSetFromOptions(x); CHKERRQ(ierr);
        ierr = VecSetFromOptions(b); CHKERRQ(ierr);

        // local assemble
        assembleRHS(linearSystem, b);
        // VecView(b, PETSC_VIEWER_STDOUT_SELF);

        // create matrix
        Mat A;
        ierr = MatCreate(linearSystem->comm, &A); CHKERRQ(ierr);
        // this matrix type is identical to MATSEQAIJ when constructed with a single process communicator, and MATMPIAIJ otherwise
        ierr = MatSetType(A, MATMPIAIJ); CHKERRQ(ierr);
        ierr = MatSetSizes(A, PETSC_DECIDE, PETSC_DECIDE, linearSystem->n(), linearSystem->n()); CHKERRQ(ierr);
        ierr = MatSetFromOptions(A); CHKERRQ(ierr);

        assembleMatrix(linearSystem, A);

        MatInfo matinfo;
        MatGetInfo(A, MAT_LOCAL, &matinfo);
        // std::cout << "rank: " << rank << ", nnz: " << (PetscInt) matinfo.nz_used << std::endl;
        linearSystem->setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        // clear structures
        linearSystem->system_matrix->clear();
        linearSystem->system_matrix_pattern->clear();
        linearSystem->system_rhs->clear();

        // Create linear solver context
        KSP ksp;
        ierr = KSPCreate(linearSystem->comm, &ksp); CHKERRQ(ierr);
#if (PETSC_VERSION_GT(3,6,0))
        ierr = KSPSetOperators(ksp, A, A); CHKERRQ(ierr);
#else
        ierr = KSPSetOperators(ksp, A, A, DIFFERENT_NONZERO_PATTERN); CHKERRQ(ierr);
#endif
        PetscReal relTol = PETSC_DEFAULT;
        if (linearSystem->infoParameterRelTol != PETSC_DEFAULT)
            relTol = linearSystem->infoParameterRelTol;

        PetscReal absTol = PETSC_DEFAULT;
        if (linearSystem->infoParameterAbsTol != PETSC_DEFAULT)
            absTol = linearSystem->infoParameterAbsTol;

        PetscInt maxIter = PETSC_DEFAULT;
        if (linearSystem->infoParameterMaxIter != PETSC_DEFAULT)
            maxIter = linearSystem->infoParameterMaxIter;

        PC pc;
        ierr = KSPGetPC(ksp, &pc);


        if (linearSystem->infoParameterPreconditioner == "hypre")
        {
           PCHYPRESetType(pc, "boomeramg");
        }

        ierr = PCSetType(pc, preConditioner(linearSystem, linearSystem->infoParameterPreconditioner)); CHKERRQ(ierr);

        ierr = KSPSetTolerances(ksp, relTol, absTol, PETSC_DEFAULT, maxIter); CHKERRQ(ierr);
        ierr = KSPSetType(ksp, solver(linearSystem, linearSystem->infoParameterSolver)); CHKERRQ(ierr);
        ierr = KSPSetFromOptions(ksp); CHKERRQ(ierr);
        auto timeSolveStart = std::chrono::steady_clock::now();
        ierr = KSPSolve(ksp, b, x); CHKERRQ(ierr);
        linearSystem->setInfoTimeSolver(elapsedSeconds(timeSolveStart));
        PetscInt iterations = 0;
        ierr = KSPGetIterationNumber(ksp, &iterations); CHKERRQ(ierr);
        linearSystem->setInfoSolverNumOfIterations(iterations);        

        // convert to local vector
        Vec localX;
        VecScatter scatter;
        ierr = VecScatterCreateToZero(x, &scatter, &localX); CHKERRQ(ierr);
        ierr = VecScatterBegin(scatter, x, localX, INSERT_VALUES, SCATTER_FORWARD); CHKERRQ(ierr);
        ierr = VecScatterEnd(scatter, x, localX, INSERT_VALUES, SCATTER_FORWARD); CHKERRQ(ierr);

        MPI_Barrier(linearSystem->comm);
        if (rank == 0)
        {
            // get pointers to vector data
            VecGetArray(localX, &linearSystem->system_sln->val);
            linearSystem->writeSolution();

            // check solution
            if (linearSystem->hasReferenceSolution())
                status = linearSystem->compareWithReferenceSolution();

            linearSystem->setInfoTimeTotal(elapsedSeconds(timeStart));
            linearSystem->setInfoSolverStateSolved();

            if (linearSystem->verbose() > 0)
            {
                linearSystem->printStatus();

                if (linearSystem->verbose() > 2)
                    linearSystem->exportStatusToFile();
            }
        }

        // destroy scatter context
        ierr = VecScatterDestroy(&scatter); CHKERRQ(ierr);

        ierr = VecDestroy(&x); CHKERRQ(ierr);
        ierr = VecDestroy(&b); CHKERRQ(ierr);
        ierr = MatDestroy(&A); CHKERRQ(ierr);
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

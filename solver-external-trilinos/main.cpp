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

// DEVELOPMENT TEMPORARY INFO - command line call:
// ./solver_TRILINOS -s test.sol -r testmatice.rhs -p testmatice.matrix_pattern -m testmatice.matrix
// mpi version tutorial - https://github.com/trilinos/Trilinos_tutorial/wiki/EpetraLesson03

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "Amesos_ConfigDefs.h"
#include "Amesos.h"
#include "Amesos_BaseSolver.h"
#include "Amesos_ConfigDefs.h"
#include "Epetra_Object.h"
#include <Epetra_Vector.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_FECrsMatrix.h>
#include <EpetraExt_MatrixMatrix.h>
#include "Epetra_RowMatrix.h"
#include "Epetra_MultiVector.h"
#include "Epetra_LinearProblem.h"
#include <Epetra_Export.h>
#include <Epetra_Import.h>

#include "mpi.h"
#include "Epetra_MpiComm.h"

#include <Epetra_Map.h>
#include <Epetra_CrsGraph.h>
#include <EpetraExt_RowMatrixOut.h>
#include <EpetraExt_VectorOut.h>
#include <EpetraExt_MultiComm.h>
#include <EpetraExt_MultiMpiComm.h>
#include <Teuchos_RCP.hpp>
#include <NOX_Common.H>
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TestForException.hpp"
// required by AztecOO
#include "AztecOO.h"
#include "Aztec2Petra.h"
#include "AztecOOParameterList.hpp"
// required by ML
#include "ml_include.h"
#include "ml_MultiLevelPreconditioner.h"
//----
#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

int rank = 0; // MPI process rank

class LinearSystemTrilinosArgs : public LinearSystemArgs
{
private:
    std::string selectMLsmootherCoarserType(std::string scType)
    {
        //  "Aztec"
        //  "IFPACK"
        //  "Jacobi"
        //  "ML symmetric Gauss-Seidel"
        //  "symmetric Gauss-Seidel"
        //  "ML Gauss-Seidel"
        //  "Gauss-Seidel"
        //  "block Gauss-Seidel"
        //  "symmetric block Gauss-Seidel"
        //  "Chebyshev"
        //  "MLS"
        //  "Hiptmair"
        //  "Amesos-KLU"
        //  "Amesos-Superlu"
        //  "Amesos-UMFPACK"
        //  "Amesos-Superludist"
        //  "Amesos-MUMPS"
        //  "user-defined"
        //  "SuperLU"
        //  "IFPACK-Chebyshev"
        //  "self"
        //  "do-nothing"
        //  "IC"
        //  "ICT"
        //  "ILU"
        //  "ILUT"
        //  "Block Chebyshev"
        //  "IFPACK-Block Chebyshev"
        //  "line Jacobi"
        //  "line Gauss-Seidel"
        //  "SILU"

        if ((scType == "Aztec")
                || (scType == "IFPACK")
                || (scType == "Jacobi")
                || (scType == "ML_symmetric_Gauss-Seidel")
                || (scType == "symmetric_Gauss-Seidel")
                || (scType == "ML_Gauss-Seidel")
                || (scType == "Gauss-Seidel")
                || (scType == "block_Gauss-Seidel")
                || (scType == "symmetric_block_Gauss-Seidel")
                || (scType == "Chebyshev")
                || (scType == "MLS")
                || (scType == "Hiptmair")
                || (scType == "Amesos-KLU")
                || (scType == "Amesos-Superlu")
                || (scType == "Amesos-UMFPACK")
                || (scType == "Amesos-Superludist")
                || (scType == "Amesos-MUMPS")
                || (scType == "user-defined")
                || (scType == "SuperLU")
                || (scType == "IFPACK-Chebyshev")
                || (scType == "self")
                || (scType == "do-nothing")
                || (scType == "IC")
                || (scType == "ICT")
                || (scType == "ILU")
                || (scType == "ILUT")
                || (scType == "Block_Chebyshev")
                || (scType == "IFPACK-Block_Chebyshev")
                || (scType == "line_Jacobi")
                || (scType == "line_Gauss-Seidel")
                || (scType == "SILU"))
        {
            if (scType == "ML_symmetric_Gauss-Seidel")
                scType = "ML symmetric Gauss-Seidel";

            if (scType == "symmetric_Gauss-Seidel")
                scType = "symmetric Gauss-Seidel";

            if (scType == "ML_Gauss-Seidel")
                scType = "ML Gauss-Seidel";

            if (scType == "block_Gauss-Seidel")
                scType = "block Gauss-Seidel";

            if (scType == "symmetric_block_Gauss-Seidel")
                scType = "symmetric block Gauss-Seidel";

            if (scType == "Block_Chebyshev")
                scType = "Block Chebyshev";

            if (scType == "IFPACK-Block_Chebyshev")
                scType = "IFPACK-Block Chebyshev";

            if (scType == "line_Jacobi")
                scType = "line Jacobi";

            if (scType == "line_Gauss-Seidel")
                scType = "line Gauss-Seidel";

            if (rank == 0)
                    std::cout << scType << std::endl;

            return scType;
        }
        else
        {
            if (rank == 0)
                    std::cout << "default" << std::endl;

            return "";
        }
    }
public:
    LinearSystemTrilinosArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv)
    {
        // defaults
        if (infoParameterSolver.empty())
            infoParameterSolver = "Amesos_Klu";

        // TODO: Not good - new paremeter!
        if (infoParameterMultigrid && infoParameterPreconditioner.empty())
            infoParameterPreconditioner = "SA";

        // if use default for set of param, get param. from solver object
        if (infoParameterMultigrid && infoParameterMultigridAggregator.empty())
            infoParameterMultigridAggregator = "";
        if (infoParameterMultigrid && infoParameterMultigridCoarser.empty())
            infoParameterMultigridCoarser = "";
        if (infoParameterMultigrid && infoParameterMultigridSmoother.empty())
            infoParameterMultigridSmoother = "";
    }

    // AztecOO part ------------------------

    int getAztecOOpreconditioner(const std::string precondName)
    {
        // default
        int preconditioner = AZ_none;

        if (precondName == "AZ_none")
            preconditioner = AZ_none;
        else if (precondName == "AZ_Jacobi")
            preconditioner = AZ_Jacobi;
        else if (precondName == "AZ_Neumann")
            preconditioner = AZ_Neumann;
        else if (precondName == "AZ_ls")
            preconditioner = AZ_ls;
        else if (precondName == "AZ_sym_GS")
            preconditioner = AZ_sym_GS;
        else if (precondName == "AZ_dom_decomp")
            preconditioner = AZ_dom_decomp;
        else
            preconditioner = AZ_dom_decomp; // default

        return preconditioner;
    }

    std::string getAztecOOpreconditionerName(const int preconditioner)
    {
        std::string precondName = "";

        switch (preconditioner)
        {
        case AZ_none:
            precondName = "AZ_none";
            break;
        case AZ_Jacobi:
            precondName = "AZ_Jacobi";
            break;
        case AZ_Neumann:
            precondName = "AZ_Neumann";
            break;
        case AZ_ls:
            precondName = "AZ_ls";
            break;
        case AZ_sym_GS:
            precondName = "AZ_sym_GS";
            break;
        case AZ_dom_decomp:
            precondName = "AZ_dom_decomp";
            break;
        default:
            precondName = "UNRECOGNIZED";
        }
        return precondName;
    }

    int getAztecOOsolver(const std::string solverName)
    {
        int solver;

        if ((solverName == "AztecOO_cg") || (solverName == "AztecOOML_cg"))                       // Conjugate gradient (Applicable to symmetric positive definite matrices, sometimes usable with mildly non-symmetric matrices).
            solver = AZ_cg;
        else if ((solverName == "AztecOO_cg_condnum") || (solverName == "AztecOOML_cg_condnum"))  // Conjugate gradient with condition number estimation.(Similar to AZ cg. Additionally computes extreme eigenvalue estimates using the generated Lanczos matrix).
            solver = AZ_cg_condnum;
        else if ((solverName == "AztecOO_gmres") || (solverName == "AztecOOML_gmres"))            // Restarted generalized minimal residual.
            solver = AZ_gmres;
        else if ((solverName == "AztecOO_gmres_condnum") || (solverName == "AztecOOML_gmres_condnum"))  // Restarted GMRES with condition number estimation. (Similar to AZ gmres. Additionally computes extreme eigenvalues using the generated Hessenberg matrix.)
            solver = AZ_gmres_condnum;
        else if ((solverName == "AztecOO_cgs") || (solverName == "AztecOOML_cgs"))                // Conjugate gradient squared.
            solver = AZ_cgs;
        else if ((solverName == "AztecOO_tfqmr") || (solverName == "AztecOOML_tfqmr"))            // Transpose-free quasi-minimal residual.
            solver = AZ_tfqmr;
        else if ((solverName == "AztecOO_bicgstab") || (solverName == "AztecOOML_bicgstab"))      // Bi-conjugate gradient with stabilization.
            solver = AZ_bicgstab;
        else if (solverName == "AztecOO_lu")      // Sparse direct solver (single processor only). Note: This option is available only when –enable-aztecoo-azlu is specified on the AztecOO configure script invocation command
            solver = AZ_lu;
        else
            solver = AZ_tfqmr; // default

        return solver;
    }

    // reverse function to get solver name from AZ constant
    std::string getAztecOOsolverName(const int solver, const bool isMultigrid)
    {
        std::string solverName = "AztecOO";
        if (isMultigrid)
            solverName = solverName + "ML";

        if (solver == AZ_cg)                       // Conjugate gradient (Applicable to symmetric positive definite matrices, sometimes usable with mildly non-symmetric matrices).
            solverName = solverName + "_cg";
        else if (solver == AZ_cg_condnum)  // Conjugate gradient with condition number estimation.(Similar to AZ cg. Additionally computes extreme eigenvalue estimates using the generated Lanczos matrix).
            solverName = solverName + "_cg_condnum";
        else if (solver == AZ_gmres)            // Restarted generalized minimal residual.
            solverName = solverName + "_gmres";
        else if (solver == AZ_gmres_condnum)  // Restarted GMRES with condition number estimation. (Similar to AZ gmres. Additionally computes extreme eigenvalues using the generated Hessenberg matrix.)
            solverName = solverName + "_gmres_condnum";
        else if (solver == AZ_cgs)                // Conjugate gradient squared.
            solverName = solverName + "_cgs";
        else if (solver == AZ_tfqmr)            // Transpose-free quasi-minimal residual.
            solverName = solverName + "_tfqmr";
        else if (solver == AZ_bicgstab)      // Bi-conjugate gradient with stabilization.
            solverName = solverName + "_bicgstab";
        else if (solver == AZ_lu)      // Sparse direct solver (single processor only). Note: This option is available only when –enable-aztecoo-azlu is specified on the AztecOO configure script invocation command
            solverName = solverName + "_lu";
        else
        {
            solverName = "_UNRECOGNIZED";
        }

        return solverName;
    }

    std::string getMLpreconditioner(const std::string precondName)
    {
        if ((precondName == "SA")
                || (precondName == "SA")             // - "SA" : classical smoothed aggregation preconditioners;
                || (precondName == "NSSA")           // - "NSSA" : default values for Petrov-Galerkin preconditioner for nonsymmetric systems
                || (precondName == "maxwell")        // - "maxwell" : default values for aggregation preconditioner for eddy current systems
                || (precondName == "DD")             // - "DD" : defaults for 2-level domain decomposition preconditioners based on aggregation;
                || (precondName == "RefMaxwell")     // - ?? instead of "DD-LU" : Like "DD", but use exact LU decompositions on each subdomain;
                || (precondName == "DD-ML")          // - "DD-ML" : 3-level domain decomposition preconditioners, with coarser spaces defined by aggregation;
                || (precondName == "DD-ML-LU"))      // - "DD-ML-LU" : Like "DD-ML", but with LU decompositions on each subdomain.
        {
            if (rank == 0)
                std::cout << "ML preconditioner is set to: " << precondName << std::endl;

            return precondName;
        }
        else
        {
            assert(0);
        }
    }

    std::string getMLaggregationType(const std::string aggregationType)
    {
        // aggregationType
        //"Uncoupled"
        //"Coupled"
        //"MIS"
        //"Uncoupled-MIS"
        //"METIS"
        //"ParMETIS"
        //"Zoltan"
        //"user"
        if ((aggregationType == "Uncoupled")
                || (aggregationType == "Coupled")
                || (aggregationType == "MIS")
                || (aggregationType == "Uncoupled-MIS")
                || (aggregationType == "METIS")
                || (aggregationType == "ParMETIS")
                || (aggregationType == "Zoltan")   // does not work, yet
                || (aggregationType == "user"))    // does not work, yet
        {
            if (rank == 0)
                std::cout << "ML aggregation type is set to: " << aggregationType << std::endl;

            return aggregationType;
        }
        else
        {
            if (rank == 0)
                std::cout << "ML aggregation type is set to: default" << std::endl;
            return "";
        }
    }


    std::string getMLcoarseType(std::string coarseType)
    {
        if (rank == 0)
            std::cout << "ML coarse type is set to: ";

        return selectMLsmootherCoarserType(coarseType);
    }

    std::string getMLsmootherType(std::string smootherType)
    {
        if (rank == 0)
            std::cout << "ML smoother type is set to: ";

        return selectMLsmootherCoarserType(smootherType);
    }
};

// Amesos part ------------------------

int solveAmesos(LinearSystemTrilinosArgs *linearSystem, const Epetra_LinearProblem &problem, std::string solverTypeName =  "Amesos_Klu")
{
    Amesos amesosFactory;
    const char *amesosSolverType = solverTypeName.c_str(); // in default uses the Amesos_Klu direct solver

    if (rank == 0)
        std::cout << "Amesos solver variant: " << solverTypeName << std::endl;

    Amesos_BaseSolver *amesosSolver = amesosFactory.Create(amesosSolverType, problem);
    assert(amesosSolver);

    // create parameter list for solver
    Teuchos::ParameterList parListAmesos;
    parListAmesos.set ("PrintTiming", false); // test of parameter setting
    parListAmesos.set ("PrintStatus", false); // test of parameter setting
    amesosSolver->SetParameters(parListAmesos);

    amesosSolver->SymbolicFactorization();
    amesosSolver->NumericFactorization();
    int status = amesosSolver->Solve();

    delete amesosSolver;

    return status;
}

int solveAztecOO(LinearSystemTrilinosArgs *linearSystem, const Epetra_LinearProblem &problem, int maxIter, double relTol, int preconditioner, int solver)
{
    AztecOO aztecooSolver(problem);
    // create parameter list for solver
    Teuchos::ParameterList parListAztecOO;
    parListAztecOO.set ("PrintTiming", false); // test of parameter setting
    parListAztecOO.set ("PrintStatus", false); // test of parameter setting
    aztecooSolver.SetParameters(parListAztecOO);
    aztecooSolver.SetAztecOption(AZ_precond, preconditioner);

    aztecooSolver.SetAztecOption(AZ_subdomain_solve, AZ_ilut);
    aztecooSolver.SetAztecOption(AZ_solver, solver);  // solver
    int status = aztecooSolver.Iterate(maxIter, relTol);
    // std::cout << "Solver performed " << aztecooSolver.NumIters() << " iterations." << std::endl << "Norm of true residual = " << aztecooSolver.TrueResidual() << std::endl;

    // on process 0 fill info for testing
    if (linearSystem)
    {
        linearSystem->setInfoSolverNumOfIterations(aztecooSolver.NumIters());
    }

    return status;
}

int solveAztecOOML(LinearSystemTrilinosArgs *linearSystem, const Epetra_LinearProblem &problem, int maxIter, double relTol, std::string preconditioner, std::string aggregationType, std::string smootherType, std::string coarseType, int solver, int numOfSweeps, int verboseMode)
{
    // create a parameter list for ML options
    Teuchos::ParameterList mlList;
    // Sets default parameters.
    // After this call, MLList contains the default values for the ML parameters.
    ML_Epetra::SetDefaults(preconditioner, mlList);

    // overwrite some parameters. Please refer to the Trilinos user's guide for more information

    // output level, 0 being silent and 10 verbose
    if (verboseMode > 0)
        mlList.set("ML output", 10);
    else
        mlList.set("ML output", 0);

    // TODO: set as cmd line parameter - maximum number of levels
    // mlList.set("max levels", 10);

    // TODO: set as cmd line parameter
    // mlList.set("increasing or decreasing", "increasing");

    // used scheme to create the aggregate
    if (aggregationType != "")
    {
        mlList.set("aggregation: type", aggregationType);
    }

    // smoother type (example file `ml/examples/TwoLevelDD/ml_2level_DD.cpp' shows how to use AZTEC's preconditioners as smoothers)
    if (smootherType != "")
    {
        mlList.set("smoother: type", smootherType);
    }

    if (numOfSweeps >= 0)
    {
      mlList.set("smoother: sweeps", numOfSweeps);
    }

    // TODO: set as cmd line parameter - use both pre and post smoothing
    // mlList.set("smoother: pre or post", "both");
    // solve with solver in "coarseType"
    if (coarseType != "")
    {
        mlList.set("coarse: type", coarseType);
    }

    // Creates the preconditioning object. We suggest to use `new' and
    // `delete' because the destructor contains some calls to MPI (as
    // required by ML and possibly Amesos). This is an issue only if the
    // destructor is called **after** MPI_Finalize().
    ML_Epetra::MultiLevelPreconditioner* mlPrec = new ML_Epetra::MultiLevelPreconditioner(*problem.GetMatrix(), mlList);
    // verify unused parameters on process 0 (put -1 to print on all processes)
    // mlPrec->PrintUnused(0);

    AztecOO aztecooSolver(problem);

    aztecooSolver.SetPrecOperator(mlPrec);
    // aztecooSolver.SetAztecOption(AZ_solver, AZ_cg);
    aztecooSolver.SetAztecOption(AZ_solver, solver);
    aztecooSolver.SetAztecOption(AZ_output, 32);   // ?? TODO: what is it?

    int status = aztecooSolver.Iterate(maxIter, relTol);

    // on process 0 fill info for testing
    if (linearSystem)
    {
        linearSystem->setInfoSolverNumOfIterations(aztecooSolver.NumIters());
    }

    return status;
}

LinearSystemTrilinosArgs *createLinearSystem(std::string extSolverName, int argc, char *argv[])
{
    LinearSystemTrilinosArgs *linearSystem = new LinearSystemTrilinosArgs(extSolverName, argc, argv);
    linearSystem->readLinearSystem();
    // create empty solution vector (Agros2D)
    linearSystem->system_sln->resize(linearSystem->system_rhs->max_len);

    return linearSystem;
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        // initialize MPI, MpiComm
        int ierr;
        MPI_Init (&argc, &argv);
        ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        Epetra_MpiComm comm(MPI_COMM_WORLD);

        int numProcs = comm.NumProc();
        int numProcZeroElements = 0;

        // start stop watch
        auto timeStart = std::chrono::steady_clock::now();

        // read matrices and parameters
        LinearSystemTrilinosArgs *linearSystem = createLinearSystem("External solver - TRILINOS", argc, argv);
        // std::cout << "rank = " << rank << ", linear system - read = " << elapsedSeconds(timeStart) << std::endl;
        // auto timeStartAssemble = std::chrono::steady_clock::now();

        // save number of processes
        if (rank == 0)
        {
            numProcZeroElements = (int) linearSystem->n();
            linearSystem->setInfoNumOfProc(numProcs);
            std::cout << "Total number of processes: " << numProcs << std::endl;
        }

        // construct a map that puts approximately the same number of equations on each processor
        Epetra_Map epeGlobalMap((int) linearSystem->n(), 0, comm);

        // local map
        Epetra_Map epeProcZeroMap(numProcZeroElements, numProcZeroElements, 0, comm);
        // exporter
        Epetra_Import importer(epeProcZeroMap, epeGlobalMap);

        // the number of elements on a specific (calling) process (given by the distribution of elements on the individual processes)
        int numMyElements = epeGlobalMap.NumMyElements();

        // vector of global IDs of local elements
        int *myGlobalElements = new int[numMyElements];
        // get the list of global indices that this process owns.
        epeGlobalMap.MyGlobalElements(myGlobalElements);

        // integer array of length NumRows
        int *numEntriesPerRow = new int[numMyElements];
        // loop over the elements of the matrix row by row
        for (int localIndex = 0; localIndex < numMyElements; localIndex++)
        {
            int row = myGlobalElements[localIndex];

            std::size_t col_start = linearSystem->system_matrix_pattern->rowstart[row];
            std::size_t col_end = linearSystem->system_matrix_pattern->rowstart[row + 1];

            numEntriesPerRow[localIndex] = col_end - col_start;
        }

        // create Epetra CrsMatrix
        Epetra_CrsMatrix epeA(Copy, epeGlobalMap, numEntriesPerRow, true);
        // create Epetra vectors x and b
        Epetra_Vector epeX(epeGlobalMap);
        Epetra_Vector epeB(epeGlobalMap);

        // prepare data from Agros2D matrix
        for (int localIndex = 0; localIndex < numMyElements; localIndex++)
        {
            int row = myGlobalElements[localIndex];

            // evaluate number of entries per row and create matrix
            int nCols = numEntriesPerRow[localIndex];
            int *localColInd = new int[nCols];
            double *localMatA = new double[nCols];

            std::size_t col_start = linearSystem->system_matrix_pattern->rowstart[row];
            std::size_t col_end = linearSystem->system_matrix_pattern->rowstart[row + 1];
            int index = 0;
            for (int j = col_start; j < col_end; j++)
            {
                localColInd[index] = linearSystem->system_matrix_pattern->colnums[j];
                localMatA[index] = linearSystem->system_matrix->val[j];

                index++;
            }
            assert(index == nCols);

            // matrix
            epeA.InsertGlobalValues(row, nCols, localMatA, localColInd);

            // fill vectors with local indices
            epeB[localIndex] = linearSystem->system_rhs->val[row];
            epeX[localIndex] = 0.0;

            delete [] localColInd;
            delete [] localMatA;
        }

        delete [] numEntriesPerRow;

        // transform from GIDs to LIDs
        epeA.FillComplete();

        // clear structures
        linearSystem->system_matrix->clear();
        linearSystem->system_matrix_pattern->clear();
        linearSystem->system_rhs->clear();

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0)
            linearSystem->setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        // create linear problem
        Epetra_LinearProblem problem(&epeA, &epeX, &epeB);

        // start of solver stop watch
        auto timeSolveStart = std::chrono::steady_clock::now();

        int solved = false;
        if (linearSystem->infoParameterMultigrid)
        {
            linearSystem->infoParameterPreconditioner = linearSystem->getMLpreconditioner(linearSystem->infoParameterPreconditioner);
            linearSystem->infoParameterSolver = linearSystem->getAztecOOsolverName(linearSystem->getAztecOOsolver(linearSystem->infoParameterSolver), true);

            if (linearSystem->infoParameterSolver.compare(0, 9, "AztecOOML") == 0) // one of multigrid AztecOOML solvers selected
            {
                solved = !solveAztecOOML(linearSystem, problem,
                                         linearSystem->infoParameterMaxIter,
                                         linearSystem->infoParameterRelTol,
                                         linearSystem->getMLpreconditioner(linearSystem->infoParameterPreconditioner),
                                         linearSystem->getMLaggregationType(linearSystem->infoParameterMultigridAggregator),
                                         linearSystem->getMLsmootherType(linearSystem->infoParameterMultigridSmoother),
                                         linearSystem->getMLcoarseType(linearSystem->infoParameterMultigridCoarser),
                                         linearSystem->getAztecOOsolver(linearSystem->infoParameterSolver),
                                         linearSystem->infoParameterNumSweeps,
                                         linearSystem->verbose());
            }
            else
                assert(0 && "No solver selected !!!");
        }
        else
        {
            if (linearSystem->infoParameterSolver == "Amesos_Klu")
            {
                solved = !solveAmesos(linearSystem, problem, "Amesos_Klu");
            }
            else if (linearSystem->infoParameterSolver == "Amesos_Paraklete")
            {
                solved = !solveAmesos(linearSystem, problem, "Amesos_Paraklete");
            }
            else if (linearSystem->infoParameterSolver.compare(0, 7, "AztecOO") == 0)   // one of AztecOO solvers selected
            {
                linearSystem->infoParameterPreconditioner = linearSystem->getAztecOOpreconditionerName(linearSystem->getAztecOOpreconditioner(linearSystem->infoParameterPreconditioner));
                linearSystem->infoParameterSolver = linearSystem->getAztecOOsolverName(linearSystem->getAztecOOsolver(linearSystem->infoParameterSolver), false);

                solved = !solveAztecOO(linearSystem, problem,
                                       linearSystem->infoParameterMaxIter,
                                       linearSystem->infoParameterRelTol,
                                       linearSystem->getAztecOOpreconditioner(linearSystem->infoParameterPreconditioner),
                                       linearSystem->getAztecOOsolver(linearSystem->infoParameterSolver));
            }
            else
            {
                assert(0 && "No solver selected !!!");
            }
        }

        // local matrix
        Epetra_Vector localX(epeProcZeroMap);
        localX.Import(epeX, importer, Insert);

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0)
        {
            if (solved)
            {
                linearSystem->setInfoTimeSolver(elapsedSeconds(timeSolveStart));

                // copy results into the solution vector (for Agros2D)
                linearSystem->system_sln->val = localX.Values();

                // write solution
                linearSystem->writeSolution();

                // check solution
                if (linearSystem->hasReferenceSolution())
                    status = linearSystem->compareWithReferenceSolution();

                linearSystem->setInfoTimeTotal(elapsedSeconds(timeStart));
                linearSystem->setInfoSolverStateSolved();
            }

            if (linearSystem->verbose() > 0)
            {
                linearSystem->printStatus();

                if (linearSystem->verbose() > 2)
                    linearSystem->exportStatusToFile();
            }
        }
        delete linearSystem;

        MPI_Finalize();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

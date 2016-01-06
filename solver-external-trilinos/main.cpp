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

#ifdef HAVE_MPI
#  include "mpi.h"
#  include "Epetra_MpiComm.h"
#else
#  include "Epetra_SerialComm.h"
#endif

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

class LinearSystemTrilinosArgs : public LinearSystemArgs
{
public:
    LinearSystemTrilinosArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
          solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          // absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int")),
          multigridArg(TCLAP::SwitchArg("g", "multigrid", "Algebraic multigrid", false))
    {
        cmd.add(solverArg);
        cmd.add(preconditionerArg);
        // cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
        cmd.add(multigridArg);
    }

    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<std::string> preconditionerArg;
    // TCLAP::ValueArg<double> absTolArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
    TCLAP::SwitchArg multigridArg;
};

void solveAmesos(const Epetra_LinearProblem &problem, std::string solverTypeName =  "Amesos_Klu")
{
    Amesos amesosFactory;
    const char *amesosSolverType = solverTypeName.c_str(); // in default uses the Amesos_Klu direct solver

    Amesos_BaseSolver *amesosSolver = amesosFactory.Create(amesosSolverType, problem);
    assert(amesosSolver);

    // create parameter list for solver
    Teuchos::ParameterList parListAmesos;
    parListAmesos.set ("PrintTiming", false); // test of parameter setting
    parListAmesos.set ("PrintStatus", false); // test of parameter setting
    amesosSolver->SetParameters(parListAmesos);

    amesosSolver->SymbolicFactorization();
    amesosSolver->NumericFactorization();
    amesosSolver->Solve();

    delete amesosSolver;
}

void solveAztecOO(const Epetra_LinearProblem &problem, int maxIter, double relTol)
{
    AztecOO aztecooSolver(problem);
    // create parameter list for solver
    Teuchos::ParameterList parListAztec00;
    parListAztec00.set ("PrintTiming", false); // test of parameter setting
    parListAztec00.set ("PrintStatus", false); // test of parameter setting
    aztecooSolver.SetParameters(parListAztec00);
    aztecooSolver.SetAztecOption(AZ_precond, AZ_dom_decomp);
    aztecooSolver.SetAztecOption(AZ_subdomain_solve, AZ_ilut);
    aztecooSolver.SetAztecOption(AZ_solver, AZ_tfqmr);
    // aztecooSolver.SetAztecOption(AZ_precond, AZ_Jacobi);
    // aztecooSolver.SetAztecOption(AZ_solver, AZ_bicgstab);
    aztecooSolver.Iterate(maxIter, relTol);
    // std::cout << "Solver performed " << aztecooSolver.NumIters() << " iterations." << std::endl << "Norm of true residual = " << aztecooSolver.TrueResidual() << std::endl;
}

void solveAztecOOML(const Epetra_LinearProblem &problem, int maxIter, double relTol, std::string preconditioner)
{
    // create a parameter list for ML options
    Teuchos::ParameterList mlList;
    // Sets default parameters.
    // After this call, MLList contains the default values for the ML parameters.
    // - "SA" : classical smoothed aggregation preconditioners;
    // - "NSSA" : default values for Petrov-Galerkin preconditioner for nonsymmetric systems
    // - "maxwell" : default values for aggregation preconditioner for eddy current systems
    // - "DD" : defaults for 2-level domain decomposition preconditioners based on aggregation;
    // - "DD-LU" : Like "DD", but use exact LU decompositions on each subdomain;
    // - "DD-ML" : 3-level domain decomposition preconditioners, with coarser spaces defined by aggregation;
    // - "DD-ML-LU" : Like "DD-ML", but with LU decompositions on each subdomain.
    ML_Epetra::SetDefaults("SA", mlList);
    // overwrite some parameters. Please refer to the user's guide
    // for more information
    // some of the parameters do not differ from their default value,
    // and they are here reported for the sake of clarity
    // output level, 0 being silent and 10 verbose
    mlList.set("ML output", 10);
    // maximum number of levels
    mlList.set("max levels", 5);
    // set finest level to 0
    mlList.set("increasing or decreasing", "increasing");
    // use Uncoupled scheme to create the aggregate
    mlList.set("aggregation: type", "Uncoupled");
    // smoother is Chebyshev. Example file `ml/examples/TwoLevelDD/ml_2level_DD.cpp' shows how to use AZTEC's preconditioners as smoothers
    mlList.set("smoother: type", "Chebyshev");
    mlList.set("smoother: sweeps", 3);
    // use both pre and post smoothing
    mlList.set("smoother: pre or post", "both");
    // solve with serial direct solver KLU
    mlList.set("coarse: type", "Amesos-KLU");

    // Creates the preconditioning object. We suggest to use `new' and
    // `delete' because the destructor contains some calls to MPI (as
    // required by ML and possibly Amesos). This is an issue only if the
    // destructor is called **after** MPI_Finalize().
    ML_Epetra::MultiLevelPreconditioner* mlPrec = new ML_Epetra::MultiLevelPreconditioner(*problem.GetMatrix(), mlList);
    // verify unused parameters on process 0 (put -1 to print on all processes)
    // mlPrec->PrintUnused(0);

    AztecOO aztecooSolver(problem);

    aztecooSolver.SetPrecOperator(mlPrec);
    aztecooSolver.SetAztecOption(AZ_solver, AZ_cg);
    aztecooSolver.SetAztecOption(AZ_output, 32);
    aztecooSolver.Iterate(maxIter, relTol);
}

std::string getMLpreconditioner(std::string preconditioner)
{
    if ((preconditioner == "SA")
            || (preconditioner == "SA")             // - "SA" : classical smoothed aggregation preconditioners;
            || (preconditioner == "NSSA")           // - "NSSA" : default values for Petrov-Galerkin preconditioner for nonsymmetric systems
            || (preconditioner == "maxwell")        // - "maxwell" : default values for aggregation preconditioner for eddy current systems
            || (preconditioner == "DD")             // - "DD" : defaults for 2-level domain decomposition preconditioners based on aggregation;
            || (preconditioner == "DD-LU")          // - "DD-LU" : Like "DD", but use exact LU decompositions on each subdomain;
            || (preconditioner == "DD-ML")          // - "DD-ML" : 3-level domain decomposition preconditioners, with coarser spaces defined by aggregation;
            || (preconditioner == "DD-ML-LU"))      // - "DD-ML-LU" : Like "DD-ML", but with LU decompositions on each subdomain.
    {
        std::cout << "ML preconditioner is set to: " << preconditioner << std::endl;
        return preconditioner;
    }
    else
    {
        std::cout << "ML preconditioner is set to default (SA)" << std::endl;
        return "SA";
    }
}

LinearSystemTrilinosArgs *createLinearSystem(std::string extSolverName, int argc, char *argv[])
{
    LinearSystemTrilinosArgs *linearSystem = new LinearSystemTrilinosArgs("External solver - TRILINOS", argc, argv);
    linearSystem->readLinearSystem();
    // create empty solution vector (Agros2D)
    linearSystem->system_sln->resize(linearSystem->system_rhs->max_len);
    linearSystem->convertToCOO();

    return linearSystem;
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        int rank = 0;           // MPI process rank
        int numProcs = 0;       // total number of processes

        LinearSystemTrilinosArgs *linearSystem = nullptr;

#ifdef HAVE_MPI       
        // Initialize MPI, MpiComm
        int ierr;
        MPI_Init (&argc, &argv);
        ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        Epetra_MpiComm comm(MPI_COMM_WORLD);

        numProcs = comm.NumProc();

        MPI_Barrier(MPI_COMM_WORLD);
        {
            std::cout << "MPI rank = " << rank << std::endl;

            if (rank == 0)
            {
                std::cout << "Total number of processes: " << numProcs << std::endl;
            }
        }
#else
        // in case of serial version
        Epetra_SerialComm comm;
#endif


        int globalNumberOfRows = 0;  // it is necessary to variable, it is necesary on each process
        int numberOfNonZeros = 0;  // it is necessary to variable, it is necesary on each process
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0)
        {
            linearSystem = createLinearSystem("External solver - TRILINOS", argc, argv);
            globalNumberOfRows = (int) linearSystem->n();
            numberOfNonZeros = (int) linearSystem->nz();
        }
// MPI
        // send globalNumberOfRows from process 0 to all processes
//        MPI_Bcast(&globalNumberOfRows, 1, MPI_INT, 0,MPI_COMM_WORLD);
//        MPI_Bcast(&numberOfNonZeros, 1, MPI_INT, 0,MPI_COMM_WORLD);
//        std::cout << "Process ID " << rank << ": " << "Global number of rows/equatins = " << globalNumberOfRows << std::endl; // develop.
//        std::cout << "Process ID " << rank << ": " << "Number of nonzero elements = " << numberOfNonZeros << std::endl; // develop.

        // Construct a Map that puts approximately the same number of equations on each processor
        const int indexBase = 0;
        Epetra_Map epeMap(globalNumberOfRows, indexBase, comm);
        // Epetra_Map epeMap(-1, numOfRows, 0, comm);

        // the number of elements on a specific (calling) process (given by the distribution of elements on the individual processes)
        int numMyElements = epeMap.NumMyElements();

        // the total number of elemnts across all processes
        int numGlobalElements = epeMap.NumGlobalElements();

        // vector of global IDs of local elements
        int *myGlobalElements = NULL;
        // get the list of global indices that this process owns.
        myGlobalElements = epeMap.MyGlobalElements();

        // tests like this really should synchronize across all
        // processes.  However, the likely cause for this case is a
        // misconfiguration of Epetra, so we expect it to happen on all
        // processes, if it happens at all.
        if ((numMyElements > 0) && (myGlobalElements == NULL))
        {
            throw std::logic_error ("Failed to get the list of global indices");
        }
// MPI

        // print Epetra Map
//        for (int i = 0; i < numProcs; ++i )
//        {
//            // MPI_Barrier(MPI_COMM_WORLD);
//            if (i == rank)
//            {
//                std::cout << "*** Process ID: " << rank << std::endl;
//                // std::cout << "Epetra Map:" << std::endl << epeMap << std::endl;
//                std::cout << "numMyElements:" << numMyElements << std::endl;
//                std::cout << "numGlobalElements:" << numGlobalElements << std::endl;
//                std::cout << "myGlobalElements:" << std::endl;
//                for (int j = 0; j < numMyElements; j++)
//                {
//                    std::cout <<  *(myGlobalElements + j) << "\t";
//                }
//                std::cout << std::endl;
//            }
//        }

//        if (rank == 0)
//        {
//            std::cout << std::endl << "Creating the sparse matrix" << std::endl;
//            std::cout << "Number of rows/equatins = " << linearSystem->n() << std::endl;
//            std::cout << "Number of non-zero elements = " << linearSystem->nz() << std::endl;
//            std::cout << "Max. num. per row = " << linearSystem->n() / ((linearSystem->n() * linearSystem->n()) / linearSystem->nz()) << std::endl;
//        }

        // create an Epetra_Matrix (sparse) whose rows have distribution given
        // by the Map.  The max number of entries per row is maxNumPerRow (aproximation).
        // estimated number non-zero elements in the row based on total percentage on nz in matrix - TODO: approve to be sofisticated
        int maxNumPerRow = globalNumberOfRows / ((globalNumberOfRows * globalNumberOfRows) / numberOfNonZeros);

        // no MPI
        Epetra_CrsMatrix epeA(Copy, epeMap, maxNumPerRow);

        // prepare data from Agros2D matrix
        for (int index = 0; index < linearSystem->nz(); index++)
        {
            int globalRow = epeA.GRID(linearSystem->cooRowInd[index]);
            // epeA.InsertGlobalValues(globalRow, 1, &linearSystem->cooA[index], &linearSystem->cooJCN[index]);
            epeA.InsertGlobalValues(globalRow, 1, &linearSystem->matA[index], (int *) &linearSystem->cooColInd[index]);
        }
        epeA.FillComplete(); // Transform from GIDs to LIDs

// MPI
          // prepare distributed Epetra matrix
//        Epetra_FECrsMatrix epeA(Copy, epeMap, maxNumPerRow);

//        // prepare data on process 0
//        if (rank == 0)
//        {
//            // prepare data from Agros2D matrix
//            for (int index = 0; index < numberOfNonZeros; index++)
//            {
//                int indices[2] = {0,0};
//                indices[0] = linearSystem->cooRowInd[index];
//                indices[1] = linearSystem->cooColInd[index];
//                epeA.SumIntoGlobalValues(1, indices, &linearSystem->matA[index]);
//                //epeA.InsertGlobalValues(globalRow, 1, &linearSystem->matA[index], (int *) &linearSystem->cooColInd[index]);   // develop - change to Epetra_FECrsMatrix ???
//            }
//        }
//        // send appropriate part of matrix to the processes
//        epeA.GlobalAssemble();
//        std::cout << "Process ID " << rank << std::endl << "epeA" << std::endl << epeA << std::endl;

// for testing MPI !!!!!!!!!!!!!!!!!!!!!!!!!!!!
//                        #ifdef HAVE_MPI
//                                // for MPI version
//                                MPI_Finalize() ;
//                        #endif
//                                return 0;
// end of test !!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // create Epetra_Vectors
        // vectors x and b
        Epetra_Vector epeX(epeMap);
        Epetra_Vector epeB(epeMap);

        // copy rhs values (Agros2D) into the Epetra vector b
        for (int i = 0; i < linearSystem->n(); i++)
            epeB[i] = linearSystem->system_rhs->val[i];

        // create linear problem
        Epetra_LinearProblem problem(&epeA, &epeX, &epeB);

        // get parameters to local value
        double relTol = linearSystem->relTolArg.getValue();
        int maxIter = linearSystem->maxIterArg.getValue();


        // solver calling
        std::string solver = linearSystem->solverArg.getValue();
        if (linearSystem->multigridArg.getValue())
        {
            if (solver == "AztecOOML" || solver == "") // default for AMG
                solveAztecOOML(problem, maxIter, relTol, getMLpreconditioner(linearSystem->preconditionerArg.getValue()));
            else
                assert(0 && "No solver selected !!!");
        }
        else
        {
            if (solver == "Amesos_Klu" || solver == "") // default
                solveAmesos(problem, "Amesos_Klu");
            else if (solver == "AztecOO")
                solveAztecOO(problem, maxIter, relTol);
            else
                assert(0 && "No solver selected !!!");

        }

        // copy results into the solution vector (for Agros2D)
        if (rank == 0)
        {
            for (int i = 0; i < linearSystem->n(); i++)
                linearSystem->system_sln->val[i] = epeX[i];       //solution[i] = demoEpeX[i]; // for test of export to Agros2D

            // write solution
            linearSystem->writeSolution();

            // check solution
            if (linearSystem->hasReferenceSolution())
                status = linearSystem->compareWithReferenceSolution();
        }
        delete linearSystem;

#ifdef HAVE_MPI
        // for MPI version
        MPI_Finalize() ;
#endif
        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

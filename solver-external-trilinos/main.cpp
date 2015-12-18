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
#include "AztecOO.h"
#include "Aztec2Petra.h"
#include "AztecOOParameterList.hpp"
//----
#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

class LinearSystemTrilinosArgs : public LinearSystemArgs
{
public:
    LinearSystemTrilinosArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
        solverVariant(TCLAP::ValueArg<int>("v", "solverVariant", "SolverVariant", false, 0, "int"))
        // preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
        // solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string"))
        // absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
        // relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
        // maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int"))
    {
        cmd.add(solverVariant);
        // cmd.add(preconditionerArg);
        // cmd.add(solverArg);
        // cmd.add(absTolArg);
        // cmd.add(relTolArg);
        // cmd.add(maxIterArg);
    }

public:
    TCLAP::ValueArg<int> solverVariant;
    // TCLAP::ValueArg<std::string> preconditionerArg;
    // TCLAP::ValueArg<std::string> solverArg;
    // TCLAP::ValueArg<double> absTolArg;
    // TCLAP::ValueArg<double> relTolArg;
    // TCLAP::ValueArg<int> maxIterArg;
};

int solveAmesos(const Epetra_LinearProblem &problem, std::string solverTypeName =  "Amesos_Klu")
{
    Amesos amesosFactory;
    const char *amesosSolverType = solverTypeName.c_str(); // in default uses the Amesos_Klu direct solver

    Amesos_BaseSolver *amesosSolver = amesosFactory.Create(amesosSolverType, problem);
    assert(amesosSolver);

    // create parameter list for solver
    Teuchos::ParameterList parList;
    parList.set ("PrintTiming", false); // test of parameter setting
    parList.set ("PrintStatus", false); // test of parameter setting
    amesosSolver->SetParameters(parList);

    AMESOS_CHK_ERR(amesosSolver->SymbolicFactorization());

    AMESOS_CHK_ERR(amesosSolver->NumericFactorization());
    AMESOS_CHK_ERR(amesosSolver->Solve());

    delete amesosSolver;
}

// TODO:  - not work yet, problem with include ??
int solveAztecOO(const Epetra_LinearProblem &problem)
{
    AztecOO aztecooSolver(problem);
    // create parameter list for solver
    Teuchos::ParameterList parList;
    parList.set ("PrintTiming", false); // test of parameter setting
    parList.set ("PrintStatus", false); // test of parameter setting
    aztecooSolver.SetParameters(parList);
    aztecooSolver.SetAztecOption(AZ_precond, AZ_Jacobi);
    aztecooSolver.SetAztecOption(AZ_solver, AZ_bicgstab);
    aztecooSolver.Iterate(1000, 1e-4);
    // std::cout << "Solver performed " << aztecooSolver.NumIters() << " iterations." << std::endl << "Norm of true residual = " << aztecooSolver.TrueResidual() << std::endl;
}

LinearSystemTrilinosArgs *createLinearSystem(std::string extSolverName, int argc, char *argv[], int &numOfRows, int &numOfNonZero)
{
    LinearSystemTrilinosArgs *linearSystem = new LinearSystemTrilinosArgs("External solver - TRILINOS", argc, argv);
    linearSystem->readLinearSystem();
    // create empty solution vector (Agros2D)
    linearSystem->system_sln->resize(linearSystem->system_rhs->max_len);
    linearSystem->convertToCOO();

    // number of unknowns
    numOfRows = linearSystem->n();

    // number of nonzero elements in matrix
    numOfNonZero = linearSystem->nz();

    return linearSystem;
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        int numOfRows = 0;      // number of unknowns, i.e. number of global elements - variable init
        int numOfNonZero = 0;   // number of nonzero elements in matrix, variable init

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
        std::cout << "MPI rank = " << rank << std::endl;

        if (rank == 0)
        {
            // Print out the Epetra software version.
            std::cout << "Total number of processes: " << numProcs << std::endl;
        }

#else
        // in case of serial version
        Epetra_SerialComm comm;
#endif

        if (rank == 0)
        {
            linearSystem = createLinearSystem("External solver - TRILINOS", argc, argv, numOfRows, numOfNonZero);
        }

        // Construct a Map that puts approximately the same number of equations on each processor
        const int indexBase = 0;
        Epetra_Map epeMap(numOfRows, indexBase, comm);
        // Epetra_Map epeMap(-1, numOfRows, 0, comm);


        int numMyElements = epeMap.NumMyElements();
        int numGlobalElements = epeMap.NumGlobalElements();
        int *myGlobalElements = NULL;  // rename, remove "my"

        // get the list of global indices that this process owns.
        myGlobalElements = epeMap.MyGlobalElements();

        // tests like this really should synchronize across all
        // processes.  However, the likely cause for this case is a
        // misconfiguration of Epetra, so we expect it to happen on all
        // processes, if it happens at all.
        if ((numMyElements > 0) && (myGlobalElements == NULL)) {
            throw std::logic_error ("Failed to get the list of global indices");
        }

        // print Epetra Map
//        for (int i = 0; i < numProcs; ++i )
//        {
//            MPI_Barrier(MPI_COMM_WORLD);
//            if (i == rank)
//            {
//                std::cout << "Epetra Map:" << std::endl << epeMap << std::endl;

//            }
//        }

        if (rank == 0) {
            std::cout << std::endl << "Creating the sparse matrix" << std::endl;
        }

        // create an Epetra_Matrix (sparse) whose rows have distribution given
        // by the Map.  The max number of entries per row is maxNumPerRow (aproximation).
        int maxNumPerRow = numOfRows / 10;
        Epetra_CrsMatrix epeA(Copy, epeMap, maxNumPerRow);

        // prepare data from Agros2D matrix
        int globalRow = 0;  // index of row in global matrix
        for (int index = 0; index < numOfNonZero; index++)
        {
            globalRow = epeA.GRID(linearSystem->cooIRN[index]);
            epeA.InsertGlobalValues(globalRow, 1, &linearSystem->cooA[index], &linearSystem->cooJCN[index]);
        }

        epeA.FillComplete(); // Transform from GIDs to LIDs

        // create Epetra_Vectors
        // vectors x and b
        Epetra_Vector epeX(epeMap);
        Epetra_Vector epeB(epeMap);

        // copy rhs values (Agros2D) into the Epetra vector b
        for (int i = 0; i < numOfRows; i++)
            epeB[i] = linearSystem->system_rhs->val[i];

        // create linear problem
        Epetra_LinearProblem problem(&epeA, &epeX, &epeB);

        // solver calling
        switch (linearSystem->solverVariant.getValue()) {
        case 0: // ------ Amesos solver
            solveAmesos(problem, "Amesos_Klu");
        case 1: // ------ AztecOO solver
            solveAztecOO(problem);
            break;
        default:
            assert(0 && "No solver selected !!!");
            break;
        }

        // copy results into the solution vector (for Agros2D)
        if (rank == 0)
        {
            for (int i = 0; i < numOfRows; i++)
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

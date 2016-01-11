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
    // another used args (not listed here): -s, -r, -p, -m, -q, -i, -v
public:
    LinearSystemTrilinosArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
          solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          aggregationTypeArg(TCLAP::ValueArg<std::string>("e", "aggregationType", "AggregationType", false, "", "string")),
          smootherTypeArg(TCLAP::ValueArg<std::string>("o", "smootherType", "SmootherType", false, "", "string")),
          coarseTypeArg(TCLAP::ValueArg<std::string>("z", "coarseType", "CoarseType", false, "", "string")),
          // absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-6, "double")),
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

void solveAmesos(LinearSystemTrilinosArgs *linearSystem, const Epetra_LinearProblem &problem, std::string solverTypeName =  "Amesos_Klu")
{
    Amesos amesosFactory;
    const char *amesosSolverType = solverTypeName.c_str(); // in default uses the Amesos_Klu direct solver

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
    amesosSolver->Solve();

    // on process 0 fill info for testing
    if (linearSystem != nullptr)
    {
        linearSystem->setInfoSolverPreconditionerName(linearSystem->preconditionerArg.getValue());
    }
    delete amesosSolver;
}

void solveAztecOO(LinearSystemTrilinosArgs *linearSystem, const Epetra_LinearProblem &problem, int maxIter, double relTol, int preconditioner, int solver)
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
    aztecooSolver.Iterate(maxIter, relTol);
    // std::cout << "Solver performed " << aztecooSolver.NumIters() << " iterations." << std::endl << "Norm of true residual = " << aztecooSolver.TrueResidual() << std::endl;

    // on process 0 fill info for testing
    if (linearSystem != nullptr)
    {
        linearSystem->setInfoSolverPreconditionerName(linearSystem->preconditionerArg.getValue());
        linearSystem->setInfoSolverNumOfIterations(aztecooSolver.NumIters());

        std::string precondName;
        switch (aztecooSolver.GetAztecOption(AZ_precond))
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
            precondName = "Not resolved";
        }
        linearSystem->setInfoSolverPreconditionerName(precondName);
        linearSystem->setInfoSolverNumOfIterations(aztecooSolver.GetAztecOption(AZ_its));
    }
}

// TODO: prepare reverse function to get preconditioner name from AZ constant
int getAztecOOpreconditioner(std::string precondName)
{
    int preconditioner;

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
    {
        preconditioner = AZ_dom_decomp;
        precondName = "AZ_dom_decomp";
    }

    std::cout << "AztecOO preconditioner is set to: " << precondName << std::endl;
    return preconditioner;
}

// TODO: prepare reverse function to get solver name from AZ constant
int getAztecOOsolver(std::string solverName)
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
    {
        solver = AZ_tfqmr;
        if (solverName.compare(0, 9, "Aztec00ML"))
            solverName = "AztecOOML_tfqmr";
        else
            solverName = "AztecOO_tfqmr";
    }

    std::cout << "AztecOO solver is set to: " << solverName << std::endl;
    return solver;
}

void solveAztecOOML(LinearSystemTrilinosArgs *linearSystem, const Epetra_LinearProblem &problem, int maxIter, double relTol, std::string preconditioner, std::string aggregationType, std::string smootherType, std::string coarseType, int solver)
{
    // create a parameter list for ML options
    Teuchos::ParameterList mlList;
    // Sets default parameters.
    // After this call, MLList contains the default values for the ML parameters.
    ML_Epetra::SetDefaults(preconditioner, mlList);
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
    mlList.set("aggregation: type", aggregationType);
    // smoother is Chebyshev. Example file `ml/examples/TwoLevelDD/ml_2level_DD.cpp' shows how to use AZTEC's preconditioners as smoothers
    mlList.set("smoother: type", smootherType);
    mlList.set("smoother: sweeps", 3);
    // use both pre and post smoothing
    mlList.set("smoother: pre or post", "both");
    // solve with solver in "coarseType"
    mlList.set("coarse: type", coarseType);

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
    aztecooSolver.SetAztecOption(AZ_output, 32);   // ??
    aztecooSolver.Iterate(maxIter, relTol);
    // on process 0 fill info for testing
    if (linearSystem != nullptr)
    {
        linearSystem->setInfoSolverPreconditionerName(linearSystem->preconditionerArg.getValue());
        linearSystem->setInfoSolverNumOfIterations(aztecooSolver.NumIters());
        linearSystem->setInfoSolverPreconditionerName("AMG-" + preconditioner + "-" + aggregationType + "-" + smootherType + "-" + coarseType);

        //linearSystem->setInfoSolverNumOfIterations(aztecooSolver.GetAztecOption(AZ_its));  // ???
    }
}

std::string getMLpreconditioner(std::string precondName)
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
        std::cout << "ML preconditioner is set to: " << precondName << std::endl;
        return precondName;
    }
    else
    {
        std::cout << "ML preconditioner is set to default (SA)" << std::endl;
        return "SA";
    }
}

std::string getMLaggregationType(std::string aggregationType)
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
        std::cout << "ML aggregation type is set to: " << aggregationType << std::endl;
        return aggregationType;
    }
    else
    {
        std::cout << "ML aggregation type is set to default (Uncoupled)" << std::endl;
        return "Uncoupled";
    }
}

std::string getMLsmootherType(std::string smootherType)
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

    if ((smootherType == "Aztec")
            || (smootherType == "IFPACK")
            || (smootherType == "Jacobi")
            || (smootherType == "ML symmetric Gauss-Seidel")
            || (smootherType == "symmetric Gauss-Seidel")
            || (smootherType == "ML Gauss-Seidel")
            || (smootherType == "Gauss-Seidel")
            || (smootherType == "block Gauss-Seidel")
            || (smootherType == "symmetric block Gauss-Seidel")
            || (smootherType == "Chebyshev")
            || (smootherType == "MLS")
            || (smootherType == "Hiptmair")
            || (smootherType == "Amesos-KLU")
            || (smootherType == "Amesos-Superlu")
            || (smootherType == "Amesos-UMFPACK")
            || (smootherType == "Amesos-Superludist")
            || (smootherType == "Amesos-MUMPS")
            || (smootherType == "user-defined")
            || (smootherType == "SuperLU")
            || (smootherType == "IFPACK-Chebyshev")
            || (smootherType == "self")
            || (smootherType == "do-nothing")
            || (smootherType == "IC")
            || (smootherType == "ICT")
            || (smootherType == "ILU")
            || (smootherType == "ILUT")
            || (smootherType == "Block Chebyshev")
            || (smootherType == "IFPACK-Block Chebyshev")
            || (smootherType == "line Jacobi")
            || (smootherType == "line Gauss-Seidel")
            || (smootherType == "SILU"))
    {
        std::cout << "ML smoother type is set to: " << smootherType << std::endl;
        return smootherType;
    }
    else
    {
        std::cout << "ML smoother type is set to default (Chebyshev)" << std::endl;
        return "Chebyshev";
    }
}

std::string getMLcoarseType(std::string coarseType)
{
    //  same as in getMLsmootherType()
    if ((coarseType == "Aztec")
            || (coarseType == "IFPACK")
            || (coarseType == "Jacobi")
            || (coarseType == "ML symmetric Gauss-Seidel")
            || (coarseType == "symmetric Gauss-Seidel")
            || (coarseType == "ML Gauss-Seidel")
            || (coarseType == "Gauss-Seidel")
            || (coarseType == "block Gauss-Seidel")
            || (coarseType == "symmetric block Gauss-Seidel")
            || (coarseType == "Chebyshev")
            || (coarseType == "MLS")
            || (coarseType == "Hiptmair")
            || (coarseType == "Amesos-KLU")
            || (coarseType == "Amesos-Superlu")
            || (coarseType == "Amesos-UMFPACK")
            || (coarseType == "Amesos-Superludist")
            || (coarseType == "Amesos-MUMPS")
            || (coarseType == "user-defined")
            || (coarseType == "SuperLU")
            || (coarseType == "IFPACK-Chebyshev")
            || (coarseType == "self")
            || (coarseType == "do-nothing")
            || (coarseType == "IC")
            || (coarseType == "ICT")
            || (coarseType == "ILU")
            || (coarseType == "ILUT")
            || (coarseType == "Block Chebyshev")
            || (coarseType == "IFPACK-Block Chebyshev")
            || (coarseType == "line Jacobi")
            || (coarseType == "line Gauss-Seidel")
            || (coarseType == "SILU"))
    {
        std::cout << "ML coarse type is set to: " << coarseType << std::endl;
        return coarseType;
    }
    else
    {
        std::cout << "ML coarse type is set to default (Amesos-KLU)" << std::endl;
        return "Amesos-KLU";
    }
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

        int rank = 0;           // MPI process rank
        int numProcs = 0;       // total number of processes

        LinearSystemTrilinosArgs *linearSystem = nullptr;

        int *numEntriesPerRow = nullptr;

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

        // start stop watch
        auto timeStart = std::chrono::steady_clock::now();

        if (rank == 0)
        {
            // read matrixes, parameters etc. onlzy on process 0, on other processe linearSystem == nullptr
            linearSystem = createLinearSystem("External solver - TRILINOS", argc, argv);
            // save time of marix reading
            linearSystem->setInfoTimeReadMatrix(elapsedSeconds(timeStart));

            // save number of processes
            linearSystem->setInfoNumOfProc(numProcs);

            globalNumberOfRows = (int) linearSystem->n();
            numberOfNonZeros = (int) linearSystem->nz();

            // integer array of length NumRows
            numEntriesPerRow = new int[linearSystem->n()];
            // loop over the elements of the matrix row by row
            for (unsigned int row = 0; row < linearSystem->n(); row++)
            {
                std::size_t col_start = linearSystem->system_matrix_pattern->rowstart[row];
                std::size_t col_end = linearSystem->system_matrix_pattern->rowstart[row + 1];

                int index = 0;
                for (unsigned int i = col_start; i < col_end; i++)
                    index++;

                numEntriesPerRow[row] = index;
            }
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

        /*
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
        */

        // no MPI
        Epetra_FECrsMatrix epeA(Copy, epeMap, numEntriesPerRow);

        // prepare data from Agros2D matrix
        int indexGlobal = 0;
        for (int row = 0; row < linearSystem->n(); row++)
        {
            // int globalRow = epeA.GRID(linearSystem->cooRowInd[index]);
            // epeA.InsertGlobalValues(globalRow, 1, &linearSystem->matA[index], (int *) &linearSystem->cooColInd[index]);

            // evaluate number of entries per row and create matrix
            int nCols = numEntriesPerRow[row];
            int *localColInd = new int[nCols];
            double *localMatA = new double[nCols];

            std::size_t col_start = linearSystem->system_matrix_pattern->rowstart[row];
            std::size_t col_end = linearSystem->system_matrix_pattern->rowstart[row + 1];
            int index = 0;
            for (int i = col_start; i < col_end; i++)
            {
                localColInd[index] = linearSystem->system_matrix_pattern->colnums[i];
                localMatA[index] = linearSystem->system_matrix->val[indexGlobal];

                index++;
                indexGlobal++;
            }

            epeA.InsertGlobalValues(1, (int *) &row, nCols, localColInd, localMatA, Epetra_FECrsMatrix::ROW_MAJOR);

            delete [] localColInd;
            delete [] localMatA;
        }
        epeA.FillComplete(); // Transform from GIDs to LIDs

        // MPI
        // prepare distributed Epetra matrix
        //        Epetra_FECrsMatrix epeA(Copy, epeMap, maxNumPerRow); // upravit dle no MPI verze - Epetra_FECrsMatrix epeA(Copy, epeMap, numEntriesPerRow);

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
        std::cout << "Solver: " << solver << std::endl;

        // add solver name to test info
        linearSystem->setInfoSolverSolverName(solver);
        auto timeSolveStart = std::chrono::steady_clock::now();
        if (linearSystem->multigridArg.getValue())
        {
            if (solver.compare(0, 9, "AztecOOML") == 0) // one of multigrid AztecOOML solvers selected
            {
                solveAztecOOML(linearSystem, problem, maxIter, relTol,
                               getMLpreconditioner(linearSystem->preconditionerArg.getValue()),
                               getMLaggregationType(linearSystem->aggregationTypeArg.getValue()),
                               getMLsmootherType(linearSystem->smootherTypeArg.getValue()),
                               getMLcoarseType(linearSystem->coarseTypeArg.getValue()),
                               getAztecOOsolver(solver));
            }
            else
                assert(0 && "No solver selected !!!");
        }
        else
        {
            if (solver == "Amesos_Klu" || solver == "") // default
            {
                linearSystem->setInfoSolverSolverName("Amesos_Klu");
                solveAmesos(linearSystem, problem, "Amesos_Klu");
            }
            else if (solver == "Amesos_Paraklete")
            {
                solveAmesos(linearSystem, problem, "Amesos_Paraklete");
            }
            else if (solver.compare(0, 7, "AztecOO") == 0)   // one of AztecOO solvers selected
            {
                solveAztecOO(linearSystem, problem, maxIter, relTol, getAztecOOpreconditioner(linearSystem->preconditionerArg.getValue()), getAztecOOsolver(solver));
            }
            else
            {
                assert(0 && "No solver selected !!!");
            }
        }
        linearSystem->setInfoTimeSolver(elapsedSeconds(timeSolveStart));
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

            linearSystem->setInfoTimeTotal(elapsedSeconds(timeStart));

            if (linearSystem->verbose() > 0)
            {
                linearSystem->printStatus();

                if (linearSystem->verbose() > 2)
                    linearSystem->exportStatusToFile();
            }
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

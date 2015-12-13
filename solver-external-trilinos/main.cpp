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

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "Amesos.h"
#include "Amesos_BaseSolver.h"
#include "Amesos_ConfigDefs.h"
#include "Epetra_Object.h"
#include <Epetra_Vector.h>
#include <Epetra_CrsMatrix.h>
#include <EpetraExt_MatrixMatrix.h>
// develop. - Serial version, not yet prepared for MPI (page 14 of Trilinos pdf manual)
#include <Epetra_SerialComm.h>
#include <Epetra_SerialDenseMatrix.h>
#include <Epetra_SerialCommData.h>
#include <Epetra_SerialDenseOperator.h>
#include <Epetra_SerialDenseSolver.h>
#include <Epetra_SerialDenseVector.h>
#include <Epetra_SerialDenseSVD.h>
#include <Epetra_SerialDistributor.h>
#include <Epetra_SerialSpdDenseSolver.h>
#include <Epetra_SerialSymDenseMatrix.h>
//----
// develop. - Serial version, not yet prepared for MPI (page 14 of Trilinos pdf manual)
#include "mpi/mpi.h"
#include <Epetra_Map.h>
#include <Epetra_CrsGraph.h>
#include <EpetraExt_RowMatrixOut.h>
#include <EpetraExt_VectorOut.h>
#include <AztecOO.h>
//----
#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

class LinearSystemTrilinosArgs : public LinearSystemArgs
{
public:
    LinearSystemTrilinosArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv)
          // preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          // solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          // absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          // relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          // maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int"))
    {
        // cmd.add(preconditionerArg);
        // cmd.add(solverArg);
        // cmd.add(absTolArg);
        // cmd.add(relTolArg);
        // cmd.add(maxIterArg);
    }

public:
    // TCLAP::ValueArg<std::string> preconditionerArg;
    // TCLAP::ValueArg<std::string> solverArg;
    // TCLAP::ValueArg<double> absTolArg;
    // TCLAP::ValueArg<double> relTolArg;
    // TCLAP::ValueArg<int> maxIterArg;
};

void denseVersion(int numOfRows, int numOfNonZero, VectorRW &system_rhs, VectorRW &solution, double *matrixA, int *iRn, int *jCn)
{
    // -------------------------- Dense serial version - TODO: separate to class -----------
    // --- develop ---
    // print elements in (of the matrix row by row)
    //        std::cout << "Matrices -----" << std::endl;
    //        for (int row = 0; row < system_matrix_pattern.rows; ++row)
    //        {
    //            std::size_t col_start = system_matrix_pattern.rowstart[row];
    //            std::size_t col_end = system_matrix_pattern.rowstart[row + 1];

    //            for (int i = col_start; i < col_end; i++)
    //            {
    //                std::cout << row << " " << system_matrix_pattern.colnums[i] << " " <<  system_matrix.val[i] << std::endl;
    //            }
    //        }

            Epetra_SerialDenseVector epeRhs(numOfRows);     // it have automatic deletion of memory allocation when destructed
            epeRhs.SetLabel("RHS - com Epetra");

            // fill Epetra vector by rhs values
            for (int i = 0; i < numOfRows; i++) {
    //            std::cout << system_rhs[i] << " ";    // show original rhs values - develop
                epeRhs(i)= system_rhs[i];             // (i) enforce bounds checking
            }
    //        std::cout << std::endl << "Filled RHS check" << std::endl << epeRhs << std::endl;  // develop


            Epetra_SerialDenseMatrix epeA(numOfRows, numOfRows);
            // fill the Epetra matrix A by solved system A matrix values
            for (int index = 0; index < numOfNonZero; index++) {
                epeA(iRn[index], jCn[index]) = matrixA[index];
            }
    //        std::cout << std::endl << "Filled A matrix check" << std::endl << epeA << std::endl;  // develop

    //        develop - simple test of matrix multiplication
    //        Epetra_SerialDenseMatrix epeMultiplication(numOfRows, numOfRows);
    //        epeMultiplication.Multiply('Y', 'N', 1.0, epeA, epeA, 1.0);
    //        std::cout << "Multiplication" << std::endl << epeMultiplication << std::endl;


            Epetra_SerialDenseVector epeSolution(numOfRows);     // Epetra solution vector
    // ---------------

    //        Epetra_LinearProblem Problem(&A, &X, &rhs);
            Epetra_SerialDenseSolver solver;
            solver.SetMatrix(epeA);
            solver.SetVectors(epeSolution, epeRhs);

    //        solver.ApplyRefinement();
            int solvStatus = solver.Solve();
            std::cout << "Solver status: " << solvStatus << std::endl;
            std::cout << "Solved ?: " << solver.Solved() << std::endl;
            std::cout << "rcond = " << solver.RCOND() << std::endl;
    // -------------------------- END of Dense version ------------------------------
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        LinearSystemTrilinosArgs linearSystem("External solver - TRILINOS", argc, argv);
        linearSystem.readLinearSystem();
        // create empty solution vector (Agros2D)
        linearSystem.system_sln->resize(linearSystem.system_rhs->max_len);
        linearSystem.convertToCOO();

        // number of unknowns
        int numOfRows = linearSystem.n();

        // number of nonzero elements in matrix
        int numOfNonZero = linearSystem.nz();

// -------------------------- Sparse serial version - TODO: separate to class ---
        Epetra_SerialComm comm;

        // map puts same number of equations on each pe
        Epetra_Map epeMap(-1, numOfRows, 0, comm);
        int numGlobalElements = epeMap.NumGlobalElements();
        int numMyElements = epeMap.NumMyElements();

        // create an Epetra_Matrix
        Epetra_CrsMatrix epeA(Copy, epeMap, 3);
        // std::cout << epeA << std::endl;

        // prepare data from Agros2D
        // matrix
        // TODO: create raw values into 1D array
        // ---
        int globalRow = 0;  // index of row in global matrix
        for (int index = 0; index < numOfNonZero; index++)
        {
            globalRow = epeA.GRID(linearSystem.cooIRN[index]);
            epeA.InsertGlobalValues(globalRow, 1, &linearSystem.cooA[index], &linearSystem.cooJCN[index]);
        }

        // std::cout << epeA << std::endl;

        epeA.FillComplete(); // Transform from GIDs to LIDs
        // std::cout << epeA << std::endl;

        // create Epetra_Vectors
        // vectors x and b
        Epetra_Vector epeX(epeMap);
        Epetra_Vector epeB(epeMap);

        // copy rhs values (Agros2D) into the Epetra vector b
        for (int i = 0; i < numOfRows; i++)
            epeB[i] = linearSystem.system_rhs->val[i];
        // std::cout << "Epetra B vector" << std::endl << epeB << std::endl;

        // create linear problem
        Epetra_LinearProblem problem(&epeA, &epeX, &epeB);

        // ------ Amesos solver
        Amesos_BaseSolver* solver;
        Amesos factory;
        char* solverType = (char *) "Amesos_Klu"; // uses the KLU direct solver
        solver = factory.Create(solverType, problem);

        AMESOS_CHK_ERR(solver->SymbolicFactorization());

        AMESOS_CHK_ERR(solver->NumericFactorization());
        AMESOS_CHK_ERR(solver->Solve());
        // std::cout << std::endl << "Amesos Solution" << std::endl << epeX << std::endl;

        // ------ End of Amesos

        // ------ DEMO AztecOO solver - not work yet, problem with include ??
        //        AztecOO solver(problem);
        //        solver.SetAztecOption(AZ_precond, AZ_Jacobi);
        //        solver.Iterate(1000, 1.0E-8);
        //        std::cout << "Solver performed " << solver.NumIters() << " iterations." << std::endl << "Norm of true residual = " << solver.TrueResidual() << std::endl;
        // ------ End of DEMO AztecOO

// -------------------------- END of Sparse Serial version ------------------------------

// -------------------------- Distributed version - TODO: separate to class -----
// prepared for MPI
// -------------------------- END of Sparse Distributed version ------------------------------

        // copy results into the solution vector (for Agros2D)
        for (int i = 0; i < numOfRows; i++)
            linearSystem.system_sln->val[i] = epeX[i];       //solution[i] = demoEpeX[i]; // for test of export to Agros2D

        // write solution
        linearSystem.writeSolution();

        // check solution
        if (linearSystem.hasReferenceSolution())
            status = linearSystem.compareWithReferenceSolution();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

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

void denseVersion(int numOfRows, int numOfNonZero, VectorRW &system_rhs, VectorRW &solution, double *matrixA, int *iRn, int *jCn)
{
    // -------------------------- Dense serial version - TODO: separate to class -----------
    // --- develop ---
    // print elements in (of the matrix row by row)
    //        std::cout << "Matrixes -----" << std::endl;
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
        // command line info
        TCLAP::CmdLine cmd("External solver - TRILINOS", ' ');

        TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
        TCLAP::ValueArg<std::string> matrixPatternArg("p", "matrix_pattern", "Matrix pattern", true, "", "string");
        TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
        TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", true, "", "string");
//        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");
//        TCLAP::ValueArg<std::string> preconditionerArg("c", "preconditioner", "Preconditioner", false, "", "string");
//        TCLAP::ValueArg<std::string> solverArg("l", "solver", "Solver", false, "", "string");
//        TCLAP::ValueArg<double> absTolArg("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double");
//        TCLAP::ValueArg<double> relTolArg("t", "rel_tol", "Relative tolerance", false, 1e-9, "double");
//        TCLAP::ValueArg<int> maxIterArg("x", "max_iter", "Maximum number of iterations", false, 1000, "int");

        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
//        cmd.add(initialArg);
//        cmd.add(preconditionerArg);
//        cmd.add(solverArg);
//        cmd.add(absTolArg);
//        cmd.add(relTolArg);
//        cmd.add(maxIterArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        // read matrix pattern
        SparsityPatternRW system_matrix_pattern;
        std::ifstream readMatrixSparsityPattern(matrixPatternArg.getValue());
        system_matrix_pattern.block_read(readMatrixSparsityPattern);
        readMatrixSparsityPattern.close();

        // read matrix
        SparseMatrixRW system_matrix;
        std::ifstream readMatrix(matrixArg.getValue());
        system_matrix.block_read(readMatrix);
        readMatrix.close();

        // read rhs
        VectorRW system_rhs;
        std::ifstream readRHS(rhsArg.getValue());
        system_rhs.block_read(readRHS);
        readRHS.close();

        // test of rhs values ---
        std::cout << "TEST Trilinos matrixes -----" << std::endl;
        // system_rhs.block_write(std::cout);

        // create empty solution vector (Agros2D)
        VectorRW solution(system_rhs.max_len);

        // number of unknowns
        int numOfRows = system_matrix_pattern.rows;

        // number of nonzero elements in matrix
        int numOfNonZero = system_matrix.max_len;

        // representation of the matrix
        double *matrixA = new double[numOfNonZero];

        // matrix indices pointing to the row and column dimensions
        int *iRn = new int[numOfNonZero];
        int *jCn = new int[numOfNonZero];

        int index = 0;

        // loop over the elements of the matrix row by row
        for (int row = 0; row < system_matrix_pattern.rows; ++row)
        {
            std::size_t col_start = system_matrix_pattern.rowstart[row];
            std::size_t col_end = system_matrix_pattern.rowstart[row + 1];

            for (int i = col_start; i < col_end; i++)
            {
                iRn[index] = row;
                jCn[index] = system_matrix_pattern.colnums[i];
                matrixA[index] = system_matrix.val[i];
                // --- demo print
                // std::cout << iRn[index] << ", " << jCn[index] << ", " << matrixA[index] << " || ";
                ++index;
            }
            // --- demo print
            // std::cout << std::endl;
        }
// -------------------------- Sparse serial version - TODO: separate to class ---
        Epetra_SerialComm comm;
        // map puts same number of equations on each pe

        Epetra_Map epeMap(-1, numOfRows, 0, comm);
        int numGlobalElements = epeMap.NumGlobalElements();
        int numMyElements = epeMap.NumMyElements();
        // create an Epetra_Matrix
        Epetra_CrsMatrix epeA(Copy, epeMap, 3);
//        std::cout << epeA << std::endl;

        // prepare data from Agros2D
        // matrix
        int globalRow;  // index of row in global matrix
        for (int rawIndex = 0; rawIndex < numOfRows; rawIndex++) {
            globalRow = epeA.GRID(rawIndex);

            // TODO: create raw values into 1D array
            // ---

            // epeA.InsertGlobalValues(globalRow, 1, &posTwo, &globalRow);

            // std::cout << epeA << std::endl;
        }
        epeA.FillComplete(); // Transform from GIDs to LIDs
        // std::cout << epeA << std::endl;

        // create Epetra_Vectors
        // vectors x and b
        Epetra_Vector epeX(epeMap);
        Epetra_Vector epeB(epeMap);

        // copy rhs values (Agros2D) into the Epetra vector b
        for (int i = 0; i < numOfRows; i++)
            epeB[i] = system_rhs[i];
        // std::cout << "Epetra B vector" << std::endl << epeB << std::endl;

        // create linear problem
        // Epetra_LinearProblem problem(&epeA, &epeX, &epeB);


// --- DEMO example
        int demoNumOfRows = 10000;
        Epetra_Map demoMap(-1, demoNumOfRows, 0, comm);
        numGlobalElements = demoMap.NumGlobalElements();
        numMyElements = demoMap.NumMyElements();
        Epetra_CrsMatrix demoEpeA(Copy, demoMap, 3);
        //std::cout << demoEpeA << std::endl;

        double negOne = -1.0;
        double posTwo = 2.0;

        for (int i = 0; i < demoNumOfRows; i++) {
          int globalRow = demoEpeA.GRID(i);
          int rowLess1 = globalRow - 1;
          int rowPlus1 = globalRow + 1;

          if (rowLess1 != -1)
             demoEpeA.InsertGlobalValues(globalRow, 1, &negOne, &rowLess1);

          if (rowPlus1 != numGlobalElements)
             demoEpeA.InsertGlobalValues(globalRow, 1, &negOne, &rowPlus1);

          demoEpeA.InsertGlobalValues(globalRow, 1, &posTwo, &globalRow);
          // double p_testArr[9] = {3.1, 8.2, 0.0, 1.5, 0.0, 0.0, 0.0, 56.4, 0.0};
          // demoEpeA.InsertGlobalValues(globalRow, 9, &p_testArr, &globalRow);

          // std::cout << demoEpeA << std::endl;
        }
        demoEpeA.FillComplete(); // Transform from GIDs to LIDs
        // std::cout << demoEpeA << std::endl;

        // ---- demo x and b vectors ----
        Epetra_Vector demoEpeX(demoMap);
        Epetra_Vector demoEpeB(demoMap);
        demoEpeB.Random(); // Fill RHS with random #s
        // ----  demo create linear problem *****
        Epetra_LinearProblem problem(&demoEpeA, &demoEpeX, &demoEpeB);

        // ------ DEMO Amesos solver
        Amesos_BaseSolver* solver;
        Amesos factory;
        char* solverType = (char *) "Amesos_Klu"; // uses the KLU direct solver
        solver = factory.Create(solverType, problem);

        AMESOS_CHK_ERR(solver->SymbolicFactorization());

        AMESOS_CHK_ERR(solver->NumericFactorization());
        AMESOS_CHK_ERR(solver->Solve());
        std::cout << std::endl << "DEMO - Amesos Solution" << std::endl << demoEpeX << std::endl;

        // ------ End of DEMO Amesos

        //        ------ DEMO AztecOO solver - not work yet, problem with include ??
        //        AztecOO solver(problem);
        //        solver.SetAztecOption(AZ_precond, AZ_Jacobi);
        //        solver.Iterate(1000, 1.0E-8);
        //        std::cout << "Solver performed " << solver.NumIters() << " iterations." << std::endl << "Norm of true residual = " << solver.TrueResidual() << std::endl;
        // ------ End of DEMO AztecOO

// --- end of DEMO example

        // -------------------------- END of Sparse Serial version ------------------------------

// -------------------------- Distributed version - TODO: separate to class -----
// prepared for MPI
// -------------------------- END of Sparse Distributed version ------------------------------

        // copy results into the solution vector (for Agros2D)
        for (int i = 0; i < numOfRows; i++)
            solution[i] = epeX[i];       //solution[i] = demoEpeX[i]; // for test of export to Agros2D


        // write solution to file
        std::ofstream writeSln(solutionArg.getValue());
        solution.block_write(writeSln);
        writeSln.close();

        exit(0);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

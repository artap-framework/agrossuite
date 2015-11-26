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

#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

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
        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");
//        TCLAP::ValueArg<std::string> preconditionerArg("c", "preconditioner", "Preconditioner", false, "", "string");
//        TCLAP::ValueArg<std::string> solverArg("l", "solver", "Solver", false, "", "string");
//        TCLAP::ValueArg<double> absTolArg("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double");
//        TCLAP::ValueArg<double> relTolArg("t", "rel_tol", "Relative tolerance", false, 1e-9, "double");
//        TCLAP::ValueArg<int> maxIterArg("x", "max_iter", "Maximum number of iterations", false, 1000, "int");

        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(initialArg);
//        cmd.add(preconditionerArg);
//        cmd.add(solverArg);
//        cmd.add(absTolArg);
//        cmd.add(relTolArg);
//        cmd.add(maxIterArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        SparsityPatternRW system_matrix_pattern;
        std::ifstream readMatrixSparsityPattern(matrixPatternArg.getValue());
        system_matrix_pattern.block_read(readMatrixSparsityPattern);
        readMatrixSparsityPattern.close();

        SparseMatrixRW system_matrix;
        std::ifstream readMatrix(matrixArg.getValue());
        system_matrix.block_read(readMatrix);
        readMatrix.close();

        VectorRW system_rhs;
        std::ifstream readRHS(rhsArg.getValue());
        system_rhs.block_read(readRHS);
        readRHS.close();

        VectorRW solution(system_rhs.max_len);

        // number of unknowns
        int n = system_matrix_pattern.rows;

        // number of nonzero elements in matrix
        int nz = system_matrix.max_len;

        Epetra_CrsMatrix A;
        Epetra_Vector X;
        Epetra_Vector B;

        Epetra_LinearProblem Problem(&A, &X, &B);

        Amesos_BaseSolver* Solver;
        Amesos Factory;
        char* SolverType = "Amesos_Klu"; // uses the KLU direct solver
        Solver = Factory.Create(SolverType, Problem);

        AMESOS_CHK_ERR(Solver->SymbolicFactorization());

        AMESOS_CHK_ERR(Solver->NumericFactorization());
        AMESOS_CHK_ERR(Solver->Solve());

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

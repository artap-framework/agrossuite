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

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "../3rdparty/tclap/CmdLine.h"

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Solver UMFPACK", ' ');

        TCLAP::ValueArg<std::string> solverArg("o", "solver", "Solver", true, "", "string");
        TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
        TCLAP::ValueArg<std::string> matrixPatternArg("p", "matrix_pattern", "Matrix pattern", true, "", "string");
        TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
        TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", true, "", "string");
        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");

        cmd.add(solverArg);
        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(initialArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        dealii::SparsityPattern system_matrix_pattern;
        std::ifstream readMatrixSparsityPattern(matrixPatternArg.getValue());
        system_matrix_pattern.block_read(readMatrixSparsityPattern);
        readMatrixSparsityPattern.close();

        dealii::SparseMatrix<double> system_matrix;
        std::ifstream readMatrix(matrixArg.getValue());
        system_matrix.reinit(system_matrix_pattern);
        system_matrix.block_read(readMatrix);
        readMatrix.close();

        dealii::Vector<double> system_rhs;
        std::ifstream readRHS(rhsArg.getValue());
        system_rhs.block_read(readRHS);
        readRHS.close();

        dealii::Vector<double> solution(system_rhs.size());
        if (solverArg.getValue() == "umfpack")
        {
            dealii::SparseDirectUMFPACK direct;
            direct.initialize(system_matrix);
            direct.vmult(solution, system_rhs);
        }

        std::ofstream writeSln(solutionArg.getValue());
        solution.block_write(writeSln);
        writeSln.close();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

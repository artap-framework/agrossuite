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

#include "viennacl/scalar.hpp"
#include "viennacl/compressed_matrix.hpp"
#include "viennacl/coordinate_matrix.hpp"
#include "viennacl/matrix.hpp"
#include "viennacl/vector.hpp"
#include "viennacl/linalg/bicgstab.hpp"
#include "viennacl/linalg/ilu.hpp"

#include <boost/numeric/ublas/matrix_sparse.hpp>

#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

typedef float ScalarType;
//typedef double ScalarType;

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("External solver - UMFPACK", ' ');

        TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
        TCLAP::ValueArg<std::string> matrixPatternArg("p", "matrix_pattern", "Matrix pattern", true, "", "string");
        TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
        TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", true, "", "string");
        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");

        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(initialArg);

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

        // number of unknowns
        int n = system_matrix_pattern.rows;

        // number of nonzero elements in matrix
        int nz = system_matrix.max_len;

        boost::numeric::ublas::compressed_matrix<ScalarType> ublas_matrix(n, n, nz);
        viennacl::compressed_matrix<ScalarType> vcl_matrix(n, n, nz);
        viennacl::vector<ScalarType> vcl_rhs(n);

        // loop over the elements of the matrix row by row
        for (int row = 0; row < system_matrix_pattern.cols; ++row)
        {
            std::size_t col_start = system_matrix_pattern.rowstart[row];
            std::size_t col_end = system_matrix_pattern.rowstart[row + 1];

            for (int i = col_start; i < col_end; i++)
            {
                // vcl_matrix(row, system_matrix_pattern.colnums[i]) = system_matrix.val[i];
                ublas_matrix(row, system_matrix_pattern.colnums[i]) = system_matrix.val[i];
            }

            vcl_rhs(row) = system_rhs.val[row];
        }

        copy(ublas_matrix, vcl_matrix);

        // clear ublas_matrix and other structures
        ublas_matrix.clear();
        system_matrix.clear();
        system_matrix_pattern.clear();
        system_rhs.clear();

        // incomplete LU factorization with threshold
        viennacl::linalg::ilut_tag ilut_config(20, 1e-4, true);
        viennacl::linalg::ilut_precond<viennacl::compressed_matrix<ScalarType> > vcl_ilut(vcl_matrix, ilut_config);

        // cg flags
        viennacl::linalg::bicgstab_tag custom_cg(1e-9, 1000);

        viennacl::vector<ScalarType> vcl_sln = viennacl::linalg::solve(vcl_matrix, vcl_rhs, custom_cg, vcl_ilut);
        std::cout << "Iters: " << custom_cg.iters() << ", error: " << custom_cg.error() << std::endl;

        // write solution
        VectorRW solution(n);
        for (int row = 0; row < n; ++row)
            solution.val[row] = vcl_sln(row);

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

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

#include "viennacl/compressed_matrix.hpp"
#include "viennacl/matrix.hpp"
#include "viennacl/vector.hpp"

// solvers
#include "viennacl/linalg/cg.hpp"
#include "viennacl/linalg/mixed_precision_cg.hpp"
#include "viennacl/linalg/bicgstab.hpp"
#include "viennacl/linalg/gmres.hpp"

// preconditioners
#include "viennacl/linalg/ilu.hpp"

// amg
#include "viennacl/linalg/amg.hpp"

#include <boost/numeric/ublas/matrix_sparse.hpp>

#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

typedef float ScalarType;
//typedef double ScalarType;

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        // command line info
        TCLAP::CmdLine cmd("External solver - UMFPACK", ' ');

        TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
        TCLAP::ValueArg<std::string> matrixPatternArg("p", "matrix_pattern", "Matrix pattern", true, "", "string");
        TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
        TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", false, "", "string");
        TCLAP::ValueArg<std::string> referenceSolutionArg("q", "reference_solution", "Reference solution", false, "", "string");
        TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");
        TCLAP::ValueArg<std::string> preconditionerArg("c", "preconditioner", "Preconditioner", false, "", "string");
        TCLAP::ValueArg<std::string> solverArg("l", "solver", "Solver", false, "", "string");
        TCLAP::ValueArg<double> relTolArg("t", "rel_tol", "Relative tolerance", false, 1e-9, "double");
        TCLAP::ValueArg<int> maxIterArg("x", "max_iter", "Maximum number of iterations", false, 1000, "int");

        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(referenceSolutionArg);
        cmd.add(initialArg);
        cmd.add(preconditionerArg);
        cmd.add(solverArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        std::string slnFileName;
        std::string slnRefFileName;

        if (!solutionArg.getValue().empty())
            slnFileName = solutionArg.getValue();

        if (!referenceSolutionArg.getValue().empty())
            slnRefFileName = referenceSolutionArg.getValue();

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

        // tolerances
        double relTol = relTolArg.getValue();
        int maxIter = maxIterArg.getValue();

        viennacl::vector<ScalarType> vcl_sln;

        // incomplete LU factorization with threshold
        // if (preconditionerArg.getValue() == "ILUT" || preconditionerArg.getValue() == "") // default

        // ilut
        viennacl::linalg::ilut_tag ilut_config(20, 1e-4, true);
        viennacl::linalg::ilut_precond<viennacl::compressed_matrix<ScalarType> > ilut_precond(vcl_matrix, ilut_config);

        // bicgstab
        viennacl::linalg::bicgstab_tag custom_bicgstab(relTol, maxIter);
        vcl_sln = viennacl::linalg::solve(vcl_matrix, vcl_rhs, custom_bicgstab, ilut_precond);
        std::cout << "BiCGStab: iters: " << custom_bicgstab.iters() << ", error: " << custom_bicgstab.error() << std::endl;

        // viennacl::context host_ctx(viennacl::MAIN_MEMORY);
        // viennacl::context target_ctx(viennacl::CUDA_MEMORY);
        /*
        viennacl::linalg::amg_tag custom_amg;
        // custom_amg.set_coarsening_method(viennacl::linalg::AMG_COARSENING_METHOD_ONEPASS);
        // custom_amg.set_interpolation_method(viennacl::linalg::AMG_INTERPOLATION_METHOD_DIRECT);
        // custom_amg.set_strong_connection_threshold(0.25);
        // custom_amg.set_jacobi_weight(0.67);
        // custom_amg.set_presmooth_steps(1);
        // custom_amg.set_postsmooth_steps(1);
        // custom_amg.set_setup_context(host_ctx);    // run setup on host
        // custom_amg.set_target_context(target_ctx); // run solver cycles on device

        viennacl::linalg::amg_precond<viennacl::compressed_matrix<ScalarType> > amg_precond(vcl_matrix, custom_amg);
        amg_precond.setup();
        vcl_sln = viennacl::linalg::solve(vcl_matrix, vcl_rhs, viennacl::linalg::bicgstab_tag(), amg_precond);
        std::cout << "AMG: coarse_levels: " << custom_amg.get_coarse_levels()
                  << ", presmooth_steps: " << custom_amg.get_presmooth_steps()
                  << ", postsmooth_steps: " << custom_amg.get_postsmooth_steps() << std::endl;
        */
        // write solution
        VectorRW solution(n);
        for (int row = 0; row < n; ++row)
            solution.val[row] = vcl_sln(row);

        if (!slnFileName.empty())
        {
            std::ofstream writeSln(slnFileName);
            solution.block_write(writeSln);
            writeSln.close();
        }

        if (!slnRefFileName.empty())
        {
            // check solution
            if (!solution.compare(slnRefFileName))
                status = -1;
        }

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

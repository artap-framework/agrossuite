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

class LinearSystemViennaCLArgs : public LinearSystemArgs
{
public:
    LinearSystemViennaCLArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
          preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int"))
    {
        cmd.add(preconditionerArg);
        cmd.add(solverArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
    }

public:
    TCLAP::ValueArg<std::string> preconditionerArg;
    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
};

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        auto timeStart = std::chrono::steady_clock::now();

        LinearSystemViennaCLArgs linearSystem("External solver - ViennaCL", argc, argv);
        linearSystem.readLinearSystem();

        linearSystem.setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        // number of unknowns
        int n = linearSystem.n();

        // number of nonzero elements in matrix
        int nz = linearSystem.nz();

        boost::numeric::ublas::compressed_matrix<ScalarType> ublas_matrix(n, n, nz);
        viennacl::compressed_matrix<ScalarType> vcl_matrix(n, n, nz);
        viennacl::vector<ScalarType> vcl_rhs(n);

        // loop over the elements of the matrix row by row
        for (int row = 0; row < linearSystem.n(); ++row)
        {
            std::size_t col_start = linearSystem.system_matrix_pattern->rowstart[row];
            std::size_t col_end = linearSystem.system_matrix_pattern->rowstart[row + 1];

            for (int i = col_start; i < col_end; i++)
            {
                // vcl_matrix(row, system_matrix_pattern.colnums[i]) = system_matrix.val[i];
                ublas_matrix(row, linearSystem.system_matrix_pattern->colnums[i]) = linearSystem.system_matrix->val[i];
            }

            vcl_rhs(row) = linearSystem.system_rhs->val[row];
        }

        copy(ublas_matrix, vcl_matrix);

        // clear ublas_matrix and other structures
        ublas_matrix.clear();

        // tolerances
        double relTol = linearSystem.relTolArg.getValue();
        int maxIter = linearSystem.maxIterArg.getValue();

        viennacl::vector<ScalarType> vcl_sln;

        // incomplete LU factorization with threshold
        // if (preconditionerArg.getValue() == "ILUT" || preconditionerArg.getValue() == "") // default

        auto timeSolveStart = std::chrono::steady_clock::now();
        // ilut
        viennacl::linalg::ilut_tag ilut_config(20, 1e-4, true);
        viennacl::linalg::ilut_precond<viennacl::compressed_matrix<ScalarType> > ilut_precond(vcl_matrix, ilut_config);

        // bicgstab
        viennacl::linalg::bicgstab_tag custom_bicgstab(relTol, maxIter);
        vcl_sln = viennacl::linalg::solve(vcl_matrix, vcl_rhs, custom_bicgstab, ilut_precond);
        std::cout << "BiCGStab: iters: " << custom_bicgstab.iters() << ", error: " << custom_bicgstab.error() << std::endl;
        linearSystem.setInfoTimeSolveSystem(elapsedSeconds(timeSolveStart));

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

        // write solution - user RHS (memory saving)
        linearSystem.system_sln = linearSystem.system_rhs;
        for (int row = 0; row < n; ++row)
            linearSystem.system_sln->val[row] = vcl_sln(row);

        // write solution
        linearSystem.writeSolution();

        // check solution
        if (linearSystem.hasReferenceSolution())
            status = linearSystem.compareWithReferenceSolution();

        linearSystem.setInfoTimeTotal(elapsedSeconds(timeStart));
        if (linearSystem.isVerbose())
            linearSystem.printStatus();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

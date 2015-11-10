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

#include <umfpack.h>

#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

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

        VectorRW solution(system_rhs.max_len);

        // number of unknowns
        int n = system_matrix_pattern.rows;

        // number of nonzero elements in matrix
        int nz = system_matrix.max_len;

        // representation of the matrix and rhs
        double *a = new double[nz];

        // matrix indices pointing to the row and column dimensions
        int *irn = new int[nz];
        int *jcn = new int[nz];

        int index = 0;

        // loop over the elements of the matrix row by row
        for (int row = 0; row < system_matrix_pattern.cols; ++row)
        {
            std::size_t col_start = system_matrix_pattern.rowstart[row];
            std::size_t col_end = system_matrix_pattern.rowstart[row + 1];

            for (int i = col_start; i < col_end; i++)
            {
                irn[index] = row + 0;
                jcn[index] = system_matrix_pattern.colnums[i] + 0;
                a[index] = system_matrix.val[i];

                ++index;
            }
        }

        system_matrix.clear();
        system_matrix_pattern.clear();

        int *Ap = new int[n+1];
        int *Ai = new int[nz];
        double *Ax = new double[nz];

        // reporting
        double Info[UMFPACK_INFO];
        double Control[UMFPACK_CONTROL];
        // Control[UMFPACK_PRL] = 6;

        int statusTripletToCol = umfpack_di_triplet_to_col (n, n, nz, irn, jcn, a, Ap, Ai, Ax, (int *) NULL);

        if (statusTripletToCol != UMFPACK_OK)
        {
            std::cerr << "UMFPACK triplet to col: " << statusTripletToCol << std::endl;
            exit(1);
        }

        delete [] irn;
        delete [] jcn;
        delete [] a;

        // umfpack_di_report_matrix(system_matrix_pattern.rows, system_matrix_pattern.cols, Ap, Ai, Ax, 1, Control);

        // factorizing symbolically
        void *symbolic;
        int statusSymbolic = umfpack_di_symbolic(n, n,
                                                 Ap, Ai, Ax,
                                                 &symbolic, Control, Info);

        if (statusSymbolic == UMFPACK_OK)
        {
            // LU factorization of matrix
            void *numeric;
            int statusNumeric = umfpack_di_numeric(Ap, Ai, Ax,
                                                   symbolic, &numeric, Control, Info);

            //  free the memory associated with the symbolic factorization.
            if (symbolic)
                umfpack_di_free_symbolic (&symbolic);

            if (statusNumeric == UMFPACK_OK)
            {
                //  solve the linear system.
                int statusSolve = umfpack_di_solve(UMFPACK_A, Ap, Ai, Ax,
                                                   solution.val, system_rhs.val, numeric, Control, Info);

                // free the memory associated with the numeric factorization.
                if (numeric)
                    umfpack_di_free_numeric(&numeric);

                if (statusSolve == UMFPACK_OK)
                {
                    // umfpack_di_report_vector (n, solution.val, Control) ;

                    std::ofstream writeSln(solutionArg.getValue());
                    solution.block_write(writeSln);
                    writeSln.close();

                    system_rhs.clear();

                    delete [] Ap;
                    delete [] Ai;
                    delete [] Ax;

                    exit(0);
                }
                else
                {
                    std::cerr << "UMFPACK numeric error: " << statusNumeric << std::endl;
                    exit(1);
                }
            }
            else
            {
                std::cerr << "UMFPACK numeric error: " << statusNumeric << std::endl;
                exit(1);
            }
        }
        else
        {
            umfpack_di_report_info(Control, Info);
            umfpack_di_report_status(Control, statusSymbolic);

            std::cerr << "UMFPACK symbolic factorization: " << statusSymbolic << std::endl;
            exit(1);
        }
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

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
        int status = 0;

        auto timeStart = std::chrono::steady_clock::now();

        LinearSystemArgs linearSystem("External solver - UMFPACK", argc, argv);
        linearSystem.readLinearSystem();
        linearSystem.system_sln->resize(linearSystem.system_rhs->max_len);
        linearSystem.convertToCOO();

        linearSystem.setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        // number of unknowns
        int n = linearSystem.n();

        // number of nonzero elements in matrix
        int nz = linearSystem.nz();

        linearSystem.system_matrix->clear();
        linearSystem.system_matrix_pattern->clear();

        int *Ap = new int[n+1];
        int *Ai = new int[nz];
        double *Ax = new double[nz];

        // reporting
        double Info[UMFPACK_INFO];
        double Control[UMFPACK_CONTROL];
        // Control[UMFPACK_PRL] = 6;

        int statusTripletToCol = umfpack_di_triplet_to_col (n, n, nz, linearSystem.cooIRN, linearSystem.cooJCN, linearSystem.cooA, Ap, Ai, Ax, (int *) NULL);

        if (statusTripletToCol != UMFPACK_OK)
        {
            std::cerr << "UMFPACK triplet to col: " << statusTripletToCol << std::endl;
            exit(1);
        }

        // umfpack_di_report_matrix(system_matrix_pattern.rows, system_matrix_pattern.cols, Ap, Ai, Ax, 1, Control);

        auto timeSolveStart = std::chrono::steady_clock::now();

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
                                                   linearSystem.system_sln->val,
                                                   linearSystem.system_rhs->val,
                                                   numeric, Control, Info);

                linearSystem.setInfoTimeSolveSystem(elapsedSeconds(timeSolveStart));

                // free the memory associated with the numeric factorization.
                if (numeric)
                    umfpack_di_free_numeric(&numeric);

                if (statusSolve == UMFPACK_OK)
                {
                    // write solution
                    linearSystem.writeSolution();

                    // check solution
                    if (linearSystem.hasReferenceSolution())
                        status = linearSystem.compareWithReferenceSolution();

                    linearSystem.setInfoTimeTotal(elapsedSeconds(timeStart));
                    if (linearSystem.isVerbose())
                        linearSystem.printStatus();

                    linearSystem.system_rhs->clear();

                    delete [] Ap;
                    delete [] Ai;
                    delete [] Ax;

                    exit(status);
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

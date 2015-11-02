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

#include "mpi/mpi.h"
#include "dmumps_c.h"

#define DEAL_II_WITH_MPI

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/base/timer.h>

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#define JOB_INIT -1
#define JOB_END -2
#define USE_COMM_WORLD -987654

#include "../../3rdparty/tclap/CmdLine.h"

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("External solver - MUMPS", ' ');

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

        dealii::Timer timer;
        timer.start();

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

        DMUMPS_STRUC_C id;
        // number of unknowns
        int n = system_matrix.n();

        // number of nonzero elements in matrix
        int nz = system_matrix.n_actually_nonzero_elements();

        // representation of the matrix and rhs
        double *a = new double[nz];
        double *rhs = new double[nz];

        // matrix indices pointing to the row and column dimensions
        // respectively of the matrix representation above (a): ie. a[k] is
        // the matrix element (irn[k], jcn[k])
        int *irn = new int[nz];
        int *jcn = new int[nz];

        int index = 0;

        // loop over the elements of the matrix row by row, as suggested in
        // the documentation of the sparse matrix iterator class
        for (int row = 0; row < system_matrix.m(); ++row)
        {
            for (typename dealii::SparseMatrix<double>::const_iterator ptr = system_matrix.begin(row); ptr != system_matrix.end (row); ++ptr)
            {
                if (std::abs (ptr->value ()) > 0.0)
                {
                    a[index]   = ptr->value ();
                    irn[index] = row + 1;
                    jcn[index] = ptr->column () + 1;
                    ++index;
                }
            }
            rhs[row] = system_rhs[row];
        }

        int myid, ierr;
        ierr = MPI_Init(&argc, &argv);
        ierr = MPI_Comm_rank(MPI_COMM_WORLD, &myid);

        /* Initialize a MUMPS instance. Use MPI_COMM_WORLD */
        id.job = JOB_INIT;
        id.par = 1;
        id.sym = 0;
        id.comm_fortran = USE_COMM_WORLD;
        dmumps_c(&id);

        /* Define the problem on the host */
        if (myid == 0)
        {
            id.n = n;
            id.nz = nz;
            id.irn = irn;
            id.jcn = jcn;
            id.a = a;
            id.rhs = rhs;
        }

        /* No outputs */
        id.icntl[0] = -1;
        id.icntl[1] = -1;
        id.icntl[2] = -1;
        id.icntl[3] =  0;

        /* Call the MUMPS package. */
        id.job = 6;
        dmumps_c(&id);
        id.job = JOB_END;
        dmumps_c(&id); /* Terminate instance */

        if (myid == 0)
        {
            dealii::Vector<double> solution(system_rhs.size());

            for (int row = 0; row < system_matrix.m(); ++row)
            {
                solution[row] = rhs[row];
            }

            std::ofstream writeSln(solutionArg.getValue());
            solution.block_write(writeSln);
            writeSln.close();
        }
        ierr = MPI_Finalize();

        delete [] a;
        delete [] rhs;
        delete [] irn;
        delete [] jcn;

        timer.stop();
        std::cout << "MUMPS: total time: " << myid << " : " << timer() << std::endl;
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

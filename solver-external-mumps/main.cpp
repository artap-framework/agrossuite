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

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#define JOB_INIT -1
#define JOB_END -2
#define JOB_SOLVE 6
#define USE_COMM_WORLD -987654

#include "../../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        int rank = 0;
        int ierr = 0;
        int np = 1;
        ierr = MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &np);

        // time
        MPI_Barrier(MPI_COMM_WORLD);
        double timeStart = 0;
        if (rank == 0)
            timeStart = MPI_Wtime();

        // initialize a MUMPS instance. Use MPI_COMM_WORLD
        DMUMPS_STRUC_C id;
        id.job = JOB_INIT;
        id.par = 1;
        id.sym = 0;
        id.comm_fortran = USE_COMM_WORLD;
        dmumps_c(&id);

        // define the problem on the host
        LinearSystemArgs linearSystem("External solver - MUMPS", argc, argv);
        linearSystem.setInfoNumOfProc(np);

        if (rank == 0)
        {
            linearSystem.readLinearSystem();

            // number of unknowns
            id.n = linearSystem.n();

            // number of nonzero elements in matrix
            id.nz = linearSystem.nz();

            // representation of the matrix and rhs
            id.a = linearSystem.system_matrix->val;
            id.rhs = linearSystem.system_rhs->val;

            // matrix indices pointing to the row and column dimensions
            id.irn = new int[id.nz];
            id.jcn = new int[id.nz];
            int index = 0;

            // loop over the elements of the matrix row by row
            for (int row = 0; row < linearSystem.n(); ++row)
            {
                std::size_t col_start = linearSystem.system_matrix_pattern->rowstart[row];
                std::size_t col_end = linearSystem.system_matrix_pattern->rowstart[row + 1];

                for (int i = col_start; i < col_end; i++)
                {
                    id.irn[index] = row + 1;
                    id.jcn[index] = linearSystem.system_matrix_pattern->colnums[i] + 1;

                    ++index;
                }
            }

            linearSystem.setInfoTimeReadMatrix(MPI_Wtime() - timeStart);


            // clear structures
            linearSystem.system_matrix_pattern->clear();
        }

        // no outputs
        id.icntl[0] = -1;
        id.icntl[1] = -1;
        id.icntl[2] = -1;
        id.icntl[3] =  0;

        double timeSolveStart = 0;
        if (rank == 0)
            timeSolveStart = MPI_Wtime();

        // call the MUMPS package.
        id.job = JOB_SOLVE;
        dmumps_c(&id);
        id.job = JOB_END;
        dmumps_c(&id); // Terminate instance

        switch (id.infog[1])
        {
        case 0:  // no error
        {
            if (rank == 0)
            {
                linearSystem.setInfoTimeSolver(MPI_Wtime() - timeSolveStart);

                // MUMPS (rhs = sln)
                linearSystem.system_sln = linearSystem.system_rhs;

                // write solution
                linearSystem.writeSolution();

                // check solution
                if (linearSystem.hasReferenceSolution())
                    status = linearSystem.compareWithReferenceSolution();

                delete [] id.irn;
                delete [] id.jcn;

                linearSystem.setInfoTimeTotal(MPI_Wtime() - timeStart);
                linearSystem.setInfoSolverStateSolved();
            }
        }
            break;
        case -1:
        {
            std::cerr << "Error occured on processor " << id.infog[2] << std::endl;
            status = -1;
        }
            break;
        case -2:
        {
            std::cerr << "Number of nonzeros (NNZ) is out of range." << std::endl;
            status = -1;
        }
            break;
        case -3:
        {
            std::cerr << "MUMPS called with an invalid option for JOB." << std::endl;
            status = -1;
        }
            break;
        case -5:
        {
            std::cerr << "Problem of REAL or COMPLEX workspace allocation of size " << id.infog[2] << " during analysis." << std::endl;
            status = -1;
        }
            break;
        case -6:
        {
            std::cerr << "Matrix is singular in structure." << std::endl;
            status = -1;
        }
            break;
        case -7:
        {
            std::cerr << "Problem of INTEGER workspace allocation of size " << id.infog[2] << " during analysis."<< std::endl;
            status = -1;
        }
            break;
        case -10:
        {
            std::cerr << "Numerically singular matrix." << std::endl;
            status = -1;
        }
            break;
        default:
        {
            std::cerr << "Non-detailed exception in MUMPS: INFOG(1) = " << id.infog[1] << std::endl;
            status = -1;
        }
            break;
        }

        if (rank == 0)
        {
            if (linearSystem.verbose() > 0)
            {
                linearSystem.printStatus();

                if (linearSystem.verbose() > 2)
                    linearSystem.exportStatusToFile();
            }
        }

        ierr = MPI_Finalize();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

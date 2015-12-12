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
#define USE_COMM_WORLD -987654

#include "../../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        int rank, ierr;
        ierr = MPI_Init(&argc, &argv);
        ierr = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        // std::cout << "Rank " << rank << " started." << std::endl;

        // time
        MPI_Barrier(MPI_COMM_WORLD);
        double startTotal = MPI_Wtime();

        // initialize a MUMPS instance. Use MPI_COMM_WORLD
        DMUMPS_STRUC_C id;
        id.job = JOB_INIT;
        id.par = 1;
        id.sym = 0;
        id.comm_fortran = USE_COMM_WORLD;
        dmumps_c(&id);

        // define the problem on the host
        SparsityPatternRW system_matrix_pattern;
        SparseMatrixRW system_matrix;
        VectorRW system_rhs;
        std::string slnFileName;
        std::string slnRefFileName;
        if (rank == 0)
        {
            // command line info
            TCLAP::CmdLine cmd("External solver - MUMPS", ' ');

            TCLAP::ValueArg<std::string> matrixArg("m", "matrix", "Matrix", true, "", "string");
            TCLAP::ValueArg<std::string> matrixPatternArg("p", "matrix_pattern", "Matrix pattern", true, "", "string");
            TCLAP::ValueArg<std::string> rhsArg("r", "rhs", "RHS", true, "", "string");
            TCLAP::ValueArg<std::string> solutionArg("s", "solution", "Solution", false, "", "string");
            TCLAP::ValueArg<std::string> referenceSolutionArg("q", "reference_solution", "Reference solution", false, "", "string");
            TCLAP::ValueArg<std::string> initialArg("i", "initial", "Initial vector", false, "", "string");

            cmd.add(matrixArg);
            cmd.add(matrixPatternArg);
            cmd.add(rhsArg);
            cmd.add(solutionArg);
            cmd.add(referenceSolutionArg);
            cmd.add(initialArg);

            // parse the argv array.
            cmd.parse(argc, argv);

            if (!solutionArg.getValue().empty())
                slnFileName = solutionArg.getValue();

            if (!referenceSolutionArg.getValue().empty())
                slnRefFileName = referenceSolutionArg.getValue();

            std::ifstream readMatrixSparsityPattern(matrixPatternArg.getValue());
            system_matrix_pattern.block_read(readMatrixSparsityPattern);
            readMatrixSparsityPattern.close();

            std::ifstream readMatrix(matrixArg.getValue());
            // system_matrix.reinit(system_matrix_pattern);
            system_matrix.block_read(readMatrix);
            readMatrix.close();

            std::ifstream readRHS(rhsArg.getValue());
            system_rhs.block_read(readRHS);
            readRHS.close();

            // std::cout << "Matrix size: " << system_rhs.max_len << std::endl;

            // number of unknowns
            id.n = system_matrix_pattern.rows;

            // number of nonzero elements in matrix
            id.nz = system_matrix.max_len;

            // representation of the matrix and rhs
            id.a = system_matrix.val;
            id.rhs = system_rhs.val;

            // matrix indices pointing to the row and column dimensions
            id.irn = new int[id.nz];
            id.jcn = new int[id.nz];
            int index = 0;

            // loop over the elements of the matrix row by row
            for (int row = 0; row < system_matrix_pattern.cols; ++row)
            {
                std::size_t col_start = system_matrix_pattern.rowstart[row];
                std::size_t col_end = system_matrix_pattern.rowstart[row + 1];

                for (int i = col_start; i < col_end; i++)
                {
                    id.a[index] = system_matrix.val[i];
                    id.irn[index] = row + 1;
                    id.jcn[index] = system_matrix_pattern.colnums[i] + 1;

                    ++index;
                }
            }

            // MPI_Barrier(MPI_COMM_WORLD);
            double end = MPI_Wtime();

            // std::cout << "Read matrix: " << (end - startTotal) << std::endl;
        }

        // no outputs
        id.icntl[0] = -1;
        id.icntl[1] = -1;
        id.icntl[2] = -1;
        id.icntl[3] =  0;

        // MPI_Barrier(MPI_COMM_WORLD);
        // double start = MPI_Wtime();

        // call the MUMPS package.
        id.job = 6;
        dmumps_c(&id);
        id.job = JOB_END;
        dmumps_c(&id); // Terminate instance

        MPI_Barrier(MPI_COMM_WORLD);
        double end = MPI_Wtime();

        // std::cout << "Rank time: " << rank << " : " << (end - start) << std::endl;

        if (rank == 0)
        {
            // system_rhs (solution)
            if (!slnFileName.empty())
            {
                std::ofstream writeSln(slnFileName);
                system_rhs.block_write(writeSln);
                writeSln.close();
            }

            if (!slnRefFileName.empty())
            {
                // check solution
                if (!system_rhs.compare(slnRefFileName))
                    status = -1;
            }

            delete [] id.irn;
            delete [] id.jcn;

            // std::cout << "Total time: " << (end - startTotal) << std::endl;
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

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

#include "paralution.hpp"

#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

//typedef float ScalarType;
typedef double ScalarType;

using namespace paralution;

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
        TCLAP::ValueArg<std::string> preconditionerArg("c", "preconditioner", "Preconditioner", false, "", "string");
        TCLAP::ValueArg<std::string> solverArg("l", "solver", "Solver", false, "", "string");

        cmd.add(matrixArg);
        cmd.add(matrixPatternArg);
        cmd.add(rhsArg);
        cmd.add(solutionArg);
        cmd.add(initialArg);
        cmd.add(preconditionerArg);
        cmd.add(solverArg);

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
        for (int row = 0; row < system_matrix_pattern.rows; ++row)
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

        // init PARALUTION
        paralution::init_paralution();
        // paralution::info_paralution();

        // info
        paralution::Paralution_Backend_Descriptor *desc = paralution::_get_backend_descriptor();

        //QString backend = QObject::tr("OpenMP");
        //if (desc->accelerator && desc->backend == OCL)
        //    backend = QObject::tr("OpenCL");
        //else if (desc->accelerator && desc->backend == GPU)
        //    backend = QObject::tr("CUDA");
        //else if (desc->accelerator && desc->backend == MIC)
        //    backend = QObject::tr("Xeon MIC");

        LocalVector<ScalarType> sln_paralution;
        LocalVector<ScalarType> rhs_paralution;
        LocalMatrix<ScalarType> mat_paralution;

        mat_paralution.SetDataPtrCOO(&irn, &jcn, &a, "matrix", nz, n, n);
        mat_paralution.ConvertToCSR();

        // rhs_paralution.Allocate("rhs", n);
        rhs_paralution.SetDataPtr(&system_rhs.val, "rhs", n);
        // sln_paralution.Allocate("sln", n);
        sln_paralution.SetDataPtr(&solution.val, "sln", n);

        bool moveToAccelerator = false;
        if (moveToAccelerator)
        {
            mat_paralution.MoveToAccelerator();
            sln_paralution.MoveToAccelerator();
            rhs_paralution.MoveToAccelerator();
        }

        // preconditioner
        Preconditioner<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType> *p = nullptr;
        if (preconditionerArg.getValue() == "Jacobi" || preconditionerArg.getValue() == "") // default
            p = new Jacobi<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (preconditionerArg.getValue() == "MultiColoredGS")
            p = new MultiColoredGS<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (preconditionerArg.getValue() == "MultiColoredSGS")
            p = new MultiColoredSGS<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (preconditionerArg.getValue() == "ILU")
            p = new ILU<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (preconditionerArg.getValue() == "MultiColoredILU")
            p = new MultiColoredILU<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (preconditionerArg.getValue() == "MultiElimination")
            p = new MultiElimination<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (preconditionerArg.getValue() == "FSAI")
            p = new FSAI<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

        // solver
        IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ls = nullptr;
        if (solverArg.getValue() == "BiCGStab" || solverArg.getValue() == "") // default
            ls = new BiCGStab<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solverArg.getValue() == "CG")
            ls = new CG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solverArg.getValue() == "GMRES")
            ls = new GMRES<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solverArg.getValue() == "FGMRES")
            ls = new FGMRES<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solverArg.getValue() == "CR")
            ls = new CR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solverArg.getValue() == "IDR")
            ls = new IDR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

        ls->Init(1e-13, 1e-9, 1e8, 1000);

        ls->SetOperator(mat_paralution);
        ls->SetPreconditioner(*p);
        // AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, double > amg;
        // amg.InitMaxIter(1) ;
        // amg.Verbose(0);
        // ls->SetPreconditioner(amg);
        ls->Verbose(1); // 2
        ls->Build();

        ls->Solve(rhs_paralution, &sln_paralution);
        sln_paralution.LeaveDataPtr(&solution.val);

        std::ofstream writeSln(solutionArg.getValue());
        solution.block_write(writeSln);
        writeSln.close();

        ls->Clear();
        delete ls;

        p->Clear();
        delete p;

        rhs_paralution.Clear();
        sln_paralution.Clear();
        mat_paralution.Clear();

        delete [] irn;
        delete [] jcn;
        delete [] a;

        // stop PARALUTION
        paralution::stop_paralution();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

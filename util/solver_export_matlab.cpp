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
#include "sparse_io.h"

#include "../3rdparty/matio/matio.h"

int main(int argc, char *argv[])
{
    // define the problem on the host
    LinearSystemArgs linearSystem("Solver - statistics", argc, argv);
    linearSystem.readLinearSystem();

    std::string fn = linearSystem.infoFileName + ".mat";
    linearSystem.convertToCSR();

    // vector
    size_t dimsVec[2];
    dimsVec[0] = linearSystem.n();
    dimsVec[1] = 1;

    mat_t *mat = Mat_CreateVer(fn.c_str(), "", MAT_FT_MAT5);

    matvar_t *rhsVar = Mat_VarCreate("rhs", MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dimsVec, linearSystem.system_rhs->val, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, rhsVar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(rhsVar);

    // matrix
    mat_sparse_t sparse;
    sparse.nzmax = linearSystem.nz();
    sparse.nir = linearSystem.nz();
    sparse.njc = linearSystem.n() + 1;
    sparse.ndata = linearSystem.nz();

    size_t dimsMat[2];
    dimsMat[0] = linearSystem.n();
    dimsMat[1] = linearSystem.n();

    // transpose
    int *cscColPtr = new int[linearSystem.n() + 1];
    int *cscRowInd = new int[linearSystem.nz()];
    double *cscA = new double[linearSystem.nz()];

    int run_i = 0;
    for (int target_row = 0; target_row < linearSystem.n(); target_row++)
    {
        cscColPtr[target_row] = run_i;
        for (int src_column = 0; src_column < linearSystem.n(); src_column++)
        {
            for (int src_row = linearSystem.csrRowPtr[src_column]; src_row < linearSystem.csrRowPtr[src_column + 1]; src_row++)
            {
                if (linearSystem.csrColInd[src_row] == target_row)
                {
                    cscRowInd[run_i] = src_column;
                    cscA[run_i++] = linearSystem.matA[src_row];
                }
            }
        }
    }

    cscColPtr[linearSystem.n()] = linearSystem.nz();

    sparse.data = cscA;
    sparse.jc = cscColPtr;
    sparse.ir = cscRowInd;

    matvar_t *matvar = Mat_VarCreate("A", MAT_C_SPARSE, MAT_T_DOUBLE, 2, dimsMat, &sparse, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] cscA;
    delete [] cscColPtr;
    delete [] cscRowInd;

    return 0;
}

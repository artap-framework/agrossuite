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

    // allocate space for the transposed matrix
    int *cscColPtr = new int[linearSystem.n() + 1];
    int *cscRowInd = new int[linearSystem.nz()];
    double *cscA = new double[linearSystem.nz()];

    // compute number of non-zero entries per column of A
    std::fill(cscColPtr, cscColPtr + linearSystem.n(), 0);

    for (int n = 0; n < linearSystem.nz(); n++)
        cscColPtr[linearSystem.csrColInd[n]]++;

    // cumsum the nz per column to get ptr[]
    for(int col = 0, cumsum = 0; col < linearSystem.n(); col++)
    {
        int temp = cscColPtr[col];
        cscColPtr[col] = cumsum;
        cumsum += temp;
    }
    cscColPtr[linearSystem.n()] = linearSystem.nz();

    for (int row = 0; row < linearSystem.n(); row++)
    {
        for (int jj = linearSystem.csrRowPtr[row]; jj < linearSystem.csrRowPtr[row+1]; jj++)
        {
            int col = linearSystem.csrColInd[jj];
            int dest = cscColPtr[col];

            cscRowInd[dest] = row;
            cscA[dest] = linearSystem.matA[jj];

            cscColPtr[col]++;
        }
    }

    for (int col = 0, last = 0; col <= linearSystem.n(); col++)
    {
        int temp = cscColPtr[col];
        cscColPtr[col] = last;
        last = temp;
    }

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

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

#include "solver_io.h"


#include "matio/matio.h"

void csr2csc(int size, int nnz, double *data, int *ir, int *jc)
{
    int *tempjc = new int[size + 1];
    int *tempir = new int[nnz];
    double *tempdata = new double[nnz];

    int run_i = 0;
    for (int target_row = 0; target_row < size; target_row++)
    {
        tempjc[target_row] = run_i;
        for (int src_column = 0; src_column < size; src_column++)
        {
            for (int src_row = jc[src_column]; src_row < jc[src_column + 1]; src_row++)
            {
                if (ir[src_row] == target_row)
                {
                    tempir[run_i] = src_column;
                    tempdata[run_i++] = data[src_row];
                }
            }
        }
    }

    tempjc[size] = nnz;

    memcpy(ir, tempir, sizeof(int) * nnz);
    memcpy(jc, tempjc, sizeof(int) * (size + 1));
    memcpy(data, tempdata, sizeof(double) * nnz);

    delete [] tempir;
    delete [] tempdata;
    delete [] tempjc;
}

bool readMatioVector(dealii::Vector<double> &vec, const QString &name, const QString &varName)
{
    mat_t *mat = Mat_Open(name.toStdString().c_str(), MAT_ACC_RDONLY);
    if (mat != NULL)
    {
        matvar_t *matvar = Mat_VarRead(mat, "sln");
        if (matvar != NULL )
        {
            // qInfo() << matvar->rank << matvar->dims[0] << matvar->dims[1] << matvar->data_type  << matvar->data_size;
            if (matvar->rank == 2 && matvar->dims[0] == 1 && matvar->dims[1] > 0)
            {
                double *data = (double *)matvar->data;
                for (unsigned i = 0; i < matvar->dims[1]; i++)
                {
                    vec[i] = data[i];
                }
            }

            Mat_VarFree(matvar);
        }
        else
        {
            qCritical() << "Variable ’" + name + "’ not found, or error reading MAT file.";
        }

        return true;
    }
    else
    {
        return false;
    }

}

void writeMatioVector(dealii::Vector<double> &vec, const QString &name, const QString &varName)
{
    size_t dims[2];
    dims[0] = vec.size();
    dims[1] = 1;

    mat_t *mat = Mat_CreateVer(name.toStdString().c_str(), "", MAT_FT_MAT4);

    double *data = new double[vec.size()];
    for (unsigned int i = 0; i < vec.size(); i++)
        data[i] = vec[i];

    matvar_t *matvar = Mat_VarCreate(varName.toStdString().c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, data, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] data;
}

void writeMatioMatrix(dealii::SparseMatrix<double> &mtx, const QString &name, const QString &varName)
{
    mat_sparse_t sparse;
    sparse.nzmax = mtx.n_nonzero_elements();
    sparse.nir = mtx.n_nonzero_elements();
    sparse.njc = mtx.n() + 1;
    sparse.ndata = mtx.n_nonzero_elements();

    size_t dims[2];
    dims[0] = mtx.m();
    dims[1] = mtx.n();

    mat_t *mat = Mat_CreateVer(name.toStdString().c_str(), "", MAT_FT_MAT4);

    double *csrA = new double[mtx.n_nonzero_elements()];
    int *csrRowPtr = new int[mtx.n() + 1];
    int *csrColInd = new int[mtx.n_nonzero_elements()];

    int index = 0;
    int rowIndex = 0;
    for (unsigned int i = 0; i < mtx.n(); i++)
    {
        csrRowPtr[rowIndex] = index + 0;
        rowIndex++;

        dealii::SparseMatrix<double>::iterator it = mtx.begin(i);
        for (; it != mtx.end(i); ++it)
        {
            if (it->is_valid_entry())
            {
                csrA[index] = it->value();
                csrColInd[index] = it->column() + 0;
                index++;
            }
        }
    }

    // by convention, we define jc[n+1] = nzz + 1
    csrRowPtr[mtx.n()] = mtx.n_nonzero_elements(); // indices from 0

    // allocate space for the transposed matrix
    mat_uint32_t *cscColPtr = new mat_uint32_t[mtx.m() + 1];
    mat_uint32_t *cscRowInd = new mat_uint32_t[mtx.n_nonzero_elements()];
    double *cscA = new double[mtx.n_nonzero_elements()];

    // compute number of non-zero entries per column of A
    std::fill(cscColPtr, cscColPtr + mtx.n(), 0);

    for (size_t n = 0; n < mtx.n_nonzero_elements(); n++)
        cscColPtr[csrColInd[n]]++;

    // cumsum the nz per column to get ptr[]
    for(size_t col = 0, cumsum = 0; col < mtx.m(); col++)
    {
        size_t temp = cscColPtr[col];
        cscColPtr[col] = cumsum;
        cumsum += temp;
    }
    cscColPtr[mtx.n()] = mtx.n_nonzero_elements();

    for (size_t row = 0; row < mtx.n(); row++)
    {
        for (int jj = csrRowPtr[row]; jj < csrRowPtr[row+1]; jj++)
        {
            int col = csrColInd[jj];
            int dest = cscColPtr[col];

            cscRowInd[dest] = row;
            cscA[dest] = csrA[jj];

            cscColPtr[col]++;
        }
    }

    for (size_t col = 0, last = 0; col <= mtx.n(); col++)
    {
        int temp = cscColPtr[col];
        cscColPtr[col] = last;
        last = temp;
    }

    delete [] csrA;
    delete [] csrRowPtr;
    delete [] csrColInd;

    sparse.data = cscA;
    sparse.jc = cscColPtr;
    sparse.ir = cscRowInd;

    matvar_t *matvar = Mat_VarCreate(varName.toStdString().c_str(), MAT_C_SPARSE, MAT_T_DOUBLE, 2, dims, &sparse, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] cscA;
    delete [] cscColPtr;
    delete [] cscRowInd;
}

void writeMatioMatrix(std::vector<dealii::Vector<double> > vecs, const QString &name, const QString &varName)
{
    if (vecs.size() == 0)
        return;

    size_t dims[2];
    dims[0] = vecs.front().size();
    dims[1] = vecs.size();

    mat_t *mat = Mat_CreateVer(name.toStdString().c_str(), "", MAT_FT_MAT4);

    double *data = new double[dims[0]*dims[1]];
    for (size_t i = 0; i < dims[0]; ++i)
        for (size_t j = 0; j < dims[1]; ++j)
            data[j*dims[0] + i] = vecs[j][i];

    matvar_t *matvar = Mat_VarCreate(varName.toStdString().c_str(), MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, data, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] data;
}

void writeMatioMatrix(std::vector<dealii::Vector<int> > vecs, const QString &name, const QString &varName)
{
    if (vecs.size() == 0)
        return;

    size_t dims[2];
    dims[0] = vecs.front().size();
    dims[1] = vecs.size();

    mat_t *mat = Mat_CreateVer(name.toStdString().c_str(), "", MAT_FT_MAT4);

    size_t *data = new size_t[dims[0]*dims[1]];
    for (size_t i = 0; i < dims[0]; ++i)
        for (size_t j = 0; j < dims[1]; ++j)
            data[j*dims[0] + i] = vecs[j][i];

    matvar_t *matvar = Mat_VarCreate(varName.toStdString().c_str(), MAT_C_INT32, MAT_T_INT32, 2, dims, data, MAT_F_DONT_COPY_DATA);

    Mat_VarWrite(mat, matvar, MAT_COMPRESSION_ZLIB);
    Mat_VarFree(matvar);
    Mat_Close(mat);

    delete [] data;
}

// **************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2015  PARALUTION Labs UG (haftungsbeschränkt) & Co. KG
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRA 706051
//                        Vertreten durch:
//                        PARALUTION Labs Verwaltungs UG (haftungsbeschränkt)
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRB 721277
//                        Geschäftsführer: Dimitar Lukarski, Nico Trost
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// **************************************************************************



// PARALUTION version 1.0.0 


__kernel void kernel_ell_nnz_coo(const int nrow, const int max_row,
                                 __global const int *row_offset, __global int *nnz_coo) {

  int gid = get_global_id(0);

  if (gid < nrow) {

    nnz_coo[gid] = 0;
    int nnz_per_row = row_offset[gid+1] - row_offset[gid];

    if (nnz_per_row > max_row)
      nnz_coo[gid] = nnz_per_row - max_row;

  }

}

__kernel void kernel_ell_fill_ell(const int nrow, const int max_row,
                                  __global const int *row_offset, __global const int *col,
                                  __global const ValueType *val, __global int *ELL_col,
                                  __global ValueType *ELL_val, __global int *nnz_ell) {

  int gid = get_global_id(0);

  if (gid < nrow) {

    int n = 0;

    for (int i=row_offset[gid]; i<row_offset[gid+1]; ++i) {

      if (n >= max_row) break;

      int idx = n * nrow + gid;

      ELL_col[idx] = col[i];
      ELL_val[idx] = val[i];

      ++n;

    }

    nnz_ell[gid] = n;

  }

}

__kernel void kernel_ell_fill_coo(const int nrow, __global const int *row_offset,
                                  __global const int *col, __global const ValueType *val,
                                  __global const int *nnz_coo, __global const int *nnz_ell,
                                  __global int *COO_row, __global int *COO_col, __global ValueType *COO_val) {

  int gid = get_global_id(0);

  if (gid < nrow) {

    int row_ptr = row_offset[gid+1];

    for (int i=row_ptr - nnz_coo[gid]; i<row_ptr; ++i) {

      int idx = i - nnz_ell[gid];

      COO_row[idx] = gid;
      COO_col[idx] = col[i];
      COO_val[idx] = val[i];

    }

  }

}

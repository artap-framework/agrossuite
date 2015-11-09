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


__kernel void kernel_dense_spmv(const int nrow, const int ncol, __global const ValueType *val,
                                __global const ValueType *in, __global ValueType *out) {

  int ai = get_global_id(0);

  if (ai < nrow) {

    ValueType sum = (ValueType)(0.0);

    for (int aj=0; aj<ncol; ++aj)
      sum += val[ai+aj*nrow] * in[aj];

    out[ai] = sum;

  }

}

// Replace column vector
__kernel void kernel_dense_replace_column_vector(__global const ValueType *vec, const int idx, const int nrow,
                                                 __global ValueType *mat) {

  int ai = get_global_id(0);

  if(ai < nrow)
    mat[ai+idx*nrow] = vec[ai];

}

// Replace row vector
__kernel void kernel_dense_replace_row_vector(__global const ValueType *vec, const int idx, const int nrow,
                                              const int ncol, __global ValueType *mat) {

  int aj = get_global_id(0);

  if (aj < ncol)
    mat[idx+aj*nrow] = vec[aj];

}

// Extract column vector
__kernel void kernel_dense_extract_column_vector(__global ValueType *vec, const int idx, const int nrow,
                                                 __global const ValueType *mat) {

  int ai = get_global_id(0);

  if (ai < nrow)
    vec[ai] = mat[ai+idx*nrow];

}

// Extract row vector
__kernel void kernel_dense_extract_row_vector(__global ValueType *vec, const int idx, const int nrow,
                                              const int ncol, __global const ValueType *mat) {

  int aj = get_global_id(0);

  if (aj < ncol)
    vec[aj] = mat[idx+aj*nrow];

}

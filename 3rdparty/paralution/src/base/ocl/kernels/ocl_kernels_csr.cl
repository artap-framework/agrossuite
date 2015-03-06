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


__kernel void kernel_csr_spmv_scalar(const int nrow, __global const int *row_offset, __global const int *col,
                                     __global const ValueType *val, __global const ValueType *in,
                                     __global ValueType *out) {

  int ai = get_global_id(0);

  if (ai < nrow) {

    ValueType sum = (ValueType)(0.0);

    for (int aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      sum += val[aj] * in[col[aj]];

    out[ai] = sum;

  }

}

__kernel void kernel_csr_add_spmv_scalar(const int nrow, __global const int *row_offset,
                                         __global const int *col, __global const ValueType *val,
                                         const ValueType scalar, __global const ValueType *in,
                                         __global ValueType *out) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

    ValueType sum = out[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      sum += scalar * val[aj] * in[col[aj]];

    out[ai] = sum;

  }

}

__kernel void kernel_csr_scale_diagonal(const int nrow, __global const int *row_offset, __global const int *col,
                                        const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        val[aj] = alpha * val[aj];

}

__kernel void kernel_csr_scale_offdiagonal(const int nrow, __global const int *row_offset, __global const int *col,
                                           const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai != col[aj])
        val[aj] = alpha * val[aj];

}

__kernel void kernel_csr_add_diagonal(const int nrow, __global const int *row_offset,
                                      __global const int *col, const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        val[aj] += alpha;

}

__kernel void kernel_csr_add_offdiagonal(const int nrow, __global const int *row_offset,
                                         __global const int *col, const ValueType alpha, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai != col[aj])
        val[aj] += alpha;

}

__kernel void kernel_csr_extract_diag(const int nrow, __global const int *row_offset, __global const int *col,
                                      __global const ValueType *val, __global ValueType *vec) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        vec[ai] = val[aj];

}

__kernel void kernel_csr_extract_inv_diag(const int nrow, __global const int *row_offset,
                                          __global const int *col, __global const ValueType *val,
                                          __global ValueType *vec) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (ai == col[aj])
        vec[ai] = (ValueType)(1.0) / val[aj];

}

__kernel void kernel_csr_extract_submatrix_row_nnz(__global const int *row_offset, __global const int *col,
                                                   __global const ValueType *val, const int smrow_offset,
                                                   const int smcol_offset, const int smrow_size,
                                                   const int smcol_size, __global int *row_nnz) {

  int ai = get_global_id(0);
  int aj;

  if (ai < smrow_size) {

    int nnz = 0;
    int ind = ai + smrow_offset;

    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj)

      if ( (col[aj] >= smcol_offset) &&
           (col[aj] <  smcol_offset + smcol_size) )
        ++nnz;

    row_nnz[ai] = nnz;

  }

}

__kernel void kernel_csr_extract_submatrix_copy(__global const int *row_offset, __global const int *col,
                                                __global const ValueType *val, const int smrow_offset,
                                                const int smcol_offset, const int smrow_size,
                                                const int smcol_size, __global const int *sm_row_offset,
                                                __global int *sm_col, __global ValueType *sm_val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < smrow_size) {

    int row_nnz = sm_row_offset[ai];
    int ind = ai + smrow_offset;

    for (aj=row_offset[ind]; aj<row_offset[ind+1]; ++aj) {

      if ( (col[aj] >= smcol_offset) &&
           (col[aj] <  smcol_offset + smcol_size) ) {

        sm_col[row_nnz] = col[aj] - smcol_offset;
        sm_val[row_nnz] = val[aj];
        ++row_nnz;

      }

    }

  }

}

__kernel void kernel_csr_diagmatmult_r(const int nrow, __global const int *row_offset, __global const int *col,
                                       __global const ValueType *diag, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      val[aj] = val[aj] * diag[col[aj]];

}

__kernel void kernel_csr_diagmatmult_l(const int nrow, __global const int *row_offset,
                                       __global const ValueType *diag, __global ValueType *val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow)
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      val[aj] = val[aj] * diag[ai];

}

__kernel void kernel_csr_add_csr_same_struct(const int nrow, __global const int *out_row_offset,
                                             __global const int *out_col, __global const int *in_row_offset,
                                             __global const int *in_col, __global const ValueType *in_val,
                                             const ValueType alpha, const ValueType beta, __global ValueType *out_val) {

  int ai = get_global_id(0);
  int aj, ajj;

  if (ai < nrow) {

    int first_col = in_row_offset[ai];
      
    for (ajj=out_row_offset[ai]; ajj<out_row_offset[ai+1]; ++ajj)
      for (aj=first_col; aj<in_row_offset[ai+1]; ++aj)
        if (in_col[aj] == out_col[ajj]) {
          
          out_val[ajj] = alpha * out_val[ajj] + beta * in_val[aj];
          ++first_col;
          break;

        }

  }

}

__kernel void kernel_buffer_addscalar(const int size, const ValueType scalar, __global ValueType *buff) {

  int gid = get_global_id(0);

  if (gid < size)
    buff[gid] += scalar;

}

__kernel void kernel_reverse_index(const int size, __global const int *perm, __global int *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[perm[gid]] = gid;

}

__kernel void kernel_csr_calc_row_nnz(const int nrow, __global const int *row_offset, __global int *row_nnz) {

  int ai = get_global_id(0);

  if (ai < nrow)
    row_nnz[ai] = row_offset[ai+1]-row_offset[ai];

}

__kernel void kernel_csr_permute_row_nnz(         const int  nrow,
                                         __global const int *row_nnz_src,
                                         __global const int *perm_vec,
                                         __global       int *row_nnz_dst) {

  int ai = get_global_id(0);

  if (ai < nrow)
    row_nnz_dst[perm_vec[ai]] = row_nnz_src[ai];

}

__kernel void kernel_csr_permute_rows(       const       int  nrow,
                                    __global const       int *row_offset,
                                    __global const       int *perm_row_offset,
                                    __global const       int *col,
                                    __global const ValueType *data,
                                    __global const       int *perm_vec,
                                    __global const       int *row_nnz,
                                    __global             int *perm_col,
                                    __global       ValueType *perm_data) {

  int ai = get_global_id(0);

  if (ai < nrow) {

    int num_elems = row_nnz[ai];
    int perm_index = perm_row_offset[perm_vec[ai]];
    int prev_index = row_offset[ai];

    for (int i = 0; i < num_elems; ++i) {
      perm_data[perm_index + i] = data[prev_index + i];
      perm_col[perm_index + i]  = col[prev_index + i];
    }

  }

}

__kernel void kernel_csr_permute_cols(         const       int  nrow,
                                      __global const       int *row_offset,
                                      __global const       int *perm_vec,
                                      __global const       int *row_nnz,
                                      __global const       int *perm_col,
                                      __global const ValueType *perm_data,
                                      __global             int *col,
                                      __global       ValueType *data) {

  int ai = get_global_id(0);
  int j;

  if (ai < nrow) {

    int num_elems = row_nnz[ai];
    int elem_index = row_offset[ai];

    for (int i = 0; i < num_elems; ++i) {

      int comp = perm_vec[perm_col[elem_index+i]];

      for (j = i-1; j >= 0 ; --j) {

        if (col[elem_index+j]>comp) {
          data[elem_index+j+1] = data[elem_index+j];
          col[elem_index+j+1]  = col[elem_index+j];
        } else
          break;
      }

      data[elem_index+j+1] = perm_data[elem_index+i];
      col[elem_index+j+1]  = comp;

    }

  }

}

__kernel void kernel_csr_extract_l_triangular(const int nrow,
                                              __global const int *src_row_offset, __global const int *src_col,
                                              __global const ValueType *src_val,  __global int *nnz_per_row,
                                              __global int *dst_col, __global ValueType *dst_val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

      int dst_index = nnz_per_row[ai];
      int src_index = src_row_offset[ai];

    for (aj=0; aj<nnz_per_row[ai+1]-nnz_per_row[ai]; ++aj) {
      
      dst_col[dst_index] = src_col[src_index];
      dst_val[dst_index] = src_val[src_index];
      
      ++dst_index;
      ++src_index;
      
    }
  }

}

__kernel void kernel_csr_extract_u_triangular(const int nrow,
                                              __global const int *src_row_offset, __global const int *src_col,
                                              __global const ValueType *src_val,  __global int *nnz_per_row,
                                              __global int *dst_col, __global ValueType *dst_val) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

      int num_elements = nnz_per_row[ai+1]-nnz_per_row[ai];
      int dst_index = nnz_per_row[ai];
      int src_index = src_row_offset[ai+1]-num_elements;

    for (aj=0; aj<num_elements; ++aj) {
      
      dst_col[dst_index] = src_col[src_index];
      dst_val[dst_index] = src_val[src_index];
      
      ++dst_index;
      ++src_index;
      
    }
  }

}

__kernel void kernel_csr_slower_nnz_per_row(const int nrow, __global const int *src_row_offset,
                                            __global const int *src_col, __global int *nnz_per_row) {

  int ai = get_global_id(0);
  int aj;
  
  if (ai < nrow) {
    nnz_per_row[ai+1] = 0;
    for (aj=src_row_offset[ai]; aj<src_row_offset[ai+1]; ++aj)
      if (src_col[aj] < ai)
        ++nnz_per_row[ai+1];
  }

}

__kernel void kernel_csr_supper_nnz_per_row(const int nrow, __global const int *src_row_offset,
                                            __global const int *src_col, __global int *nnz_per_row) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {
    nnz_per_row[ai+1] = 0;
    for (aj=src_row_offset[ai]; aj<src_row_offset[ai+1]; ++aj)
      if (src_col[aj] > ai)
        ++nnz_per_row[ai+1];
  }

}

__kernel void kernel_csr_lower_nnz_per_row(const int nrow, __global const int *src_row_offset,
                                           __global const int *src_col, __global int *nnz_per_row) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {
    nnz_per_row[ai+1] = 0;
    for (aj=src_row_offset[ai]; aj<src_row_offset[ai+1]; ++aj)
      if (src_col[aj] <= ai)
        ++nnz_per_row[ai+1];
  }

}

__kernel void kernel_csr_upper_nnz_per_row(const int nrow, __global const int *src_row_offset,
                                           __global const int *src_col, __global int *nnz_per_row) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {
    nnz_per_row[ai+1] = 0;
    for (aj=src_row_offset[ai]; aj<src_row_offset[ai+1]; ++aj)
      if (src_col[aj] >= ai)
        ++nnz_per_row[ai+1];
  }

}

__kernel void kernel_csr_compress_count_nrow(__global const int *row_offset, __global const int *col,
                                             __global const ValueType *val, const int nrow, const double drop_off,
                                             __global int *row_offset_new) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {
    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {

      if ( (fabs(val[aj]) > drop_off) ||
           ( col[aj] == ai))
        row_offset_new[ai]++;
    }
  }

}

__kernel void kernel_csr_compress_copy(__global const int *row_offset, __global const int *col,
                                       __global const ValueType *val, const int nrow, const double drop_off,
                                       __global const int *row_offset_new, __global int *col_new,
                                       __global ValueType *val_new) {

  int ai = get_global_id(0);
  int aj;
  int ajj = row_offset_new[ai];

  if (ai < nrow) {

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {

      if ( (fabs(val[aj]) > drop_off) ||
           ( col[aj] == ai)) {
        col_new[ajj] = col[aj];
        val_new[ajj] = val[aj];
        ajj++;
      }
    }

  }

}

// Extract column vector
__kernel void kernel_csr_extract_column_vector(__global const int *row_offset, __global const int *col,
                                               __global const ValueType *val, const int nrow, const int idx,
                                               __global ValueType *vec) {

  int ai = get_global_id(0);
  int aj;

  if (ai < nrow) {

    vec[ai] = (ValueType)(0.0);

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj)
      if (idx == col[aj])
        vec[ai] = val[aj];

  }

}

// Replace column vector - compute new offset
__kernel void kernel_csr_replace_column_vector_offset(__global const int *row_offset, __global const int *col,
                                                      const int nrow, const int idx,
                                                      __global const ValueType *vec, __global int *offset) {

  int ai = get_global_id(0);
  int aj;
  int add = 1;

  if (ai < nrow) {

    offset[ai+1] = row_offset[ai+1] - row_offset[ai];

    for (aj=row_offset[ai]; aj<row_offset[ai+1]; ++aj) {
      if (col[aj] == idx) {
        add = 0;
        break;
      }
    }

    if (add == 1 && vec[ai] != (ValueType)(0.0))
      ++offset[ai+1];

    if (add == 0 && vec[ai] == (ValueType)(0.0))
      --offset[ai+1];

  }

}

// Replace column vector - compute new offset
__kernel void kernel_csr_replace_column_vector(__global const int *row_offset, __global const int *col,
                                               __global const ValueType *val, const int nrow, const int idx,
                                               __global const ValueType *vec, __global const int *offset,
                                               __global int *new_col, __global ValueType *new_val) {

  int ai = get_global_id(0);
  int aj = row_offset[ai];
  int k  = offset[ai];

  if (ai < nrow) {

    for (; aj<row_offset[ai+1]; ++aj) {
      if (col[aj] < idx) {
        new_col[k] = col[aj];
        new_val[k] = val[aj];
        ++k;
      } else
        break;
    }

    if (vec[ai] != (ValueType)(0.0)) {
      new_col[k] = idx;
      new_val[k] = vec[ai];
      ++k;
      ++aj;
    }

    for (; aj<row_offset[ai+1]; ++aj) {
      if (col[aj] > idx) {
        new_col[k] = col[aj];
        new_val[k] = val[aj];
        ++k;
      }
    }

  }

}

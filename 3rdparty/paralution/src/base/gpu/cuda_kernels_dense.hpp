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


#ifndef PARALUTION_GPU_CUDA_KERNELS_DENSE_HPP_
#define PARALUTION_GPU_CUDA_KERNELS_DENSE_HPP_

#include "../matrix_formats_ind.hpp"

namespace paralution {

// Replace column vector
template <typename ValueType, typename IndexType>
__global__ void kernel_dense_replace_column_vector(const ValueType *vec, const IndexType idx, const IndexType nrow,
                                                   const IndexType ncol, ValueType *mat) {

  IndexType ai = blockIdx.x * blockDim.x + threadIdx.x;

  if(ai < nrow)
    mat[DENSE_IND(ai, idx, nrow, ncol)] = vec[ai];

}

// Replace row vector
template <typename ValueType, typename IndexType>
__global__ void kernel_dense_replace_row_vector(const ValueType *vec, const IndexType idx, const IndexType nrow,
                                                const IndexType ncol, ValueType *mat) {

  IndexType aj = blockIdx.x * blockDim.x + threadIdx.x;

  if (aj < ncol)
    mat[DENSE_IND(idx, aj, nrow, ncol)] = vec[aj];

}

// Extract column vector
template <typename ValueType, typename IndexType>
__global__ void kernel_dense_extract_column_vector(ValueType *vec, const IndexType idx, const IndexType nrow,
                                                   const IndexType ncol, const ValueType *mat) {

  IndexType ai = blockIdx.x * blockDim.x + threadIdx.x;

  if (ai < nrow)
    vec[ai] = mat[DENSE_IND(ai, idx, nrow, ncol)];

}

// Extract row vector
template <typename ValueType, typename IndexType>
__global__ void kernel_dense_extract_row_vector(ValueType *vec, const IndexType idx, const IndexType nrow,
                                                const IndexType ncol, const ValueType *mat) {

  IndexType aj = blockIdx.x * blockDim.x + threadIdx.x;

  if (aj < ncol)
    vec[aj] = mat[DENSE_IND(idx, aj, nrow, ncol)];

}


}

#endif

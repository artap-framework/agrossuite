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


#ifndef PARALUTION_KERNELS_OCL_HPP_
#define PARALUTION_KERNELS_OCL_HPP_

#define KERNELCOUNT 74

static const std::string kernels_ocl[KERNELCOUNT] = {
                "kernel_set_to",
                "kernel_reverse_index",
                "kernel_buffer_addscalar",
                "kernel_scaleadd",
                "kernel_scaleaddscale",
                "kernel_scaleadd2",
                "kernel_pointwisemult",
                "kernel_pointwisemult2",
                "kernel_copy_offset_from",
                "kernel_permute",
                "kernel_permute_backward",
                "kernel_dot",
                "kernel_axpy",
                "kernel_csr_spmv_scalar",
                "kernel_csr_add_spmv_scalar",
                "kernel_csr_scale_diagonal",
                "kernel_csr_scale_offdiagonal",
                "kernel_csr_add_diagonal",
                "kernel_csr_add_offdiagonal",
                "kernel_csr_extract_diag",
                "kernel_csr_extract_inv_diag",
                "kernel_csr_extract_submatrix_row_nnz",
                "kernel_csr_extract_submatrix_copy",
                "kernel_csr_diagmatmult_r",
                "kernel_csr_add_csr_same_struct",
                "kernel_scale",
                "kernel_mcsr_spmv_scalar",
                "kernel_mcsr_add_spmv_scalar",
                "kernel_ell_spmv",
                "kernel_ell_add_spmv",
                "kernel_dia_spmv",
                "kernel_dia_add_spmv",
                "kernel_coo_permute",
                "kernel_coo_spmv_flat",
                "kernel_coo_spmv_reduce_update",
                "kernel_coo_spmv_serial",
                "kernel_red_recurse",
                "kernel_red_partial_sum",
                "kernel_red_extrapolate",
                "kernel_csr_permute_rows",
                "kernel_csr_permute_cols",
                "kernel_csr_calc_row_nnz",
                "kernel_csr_permute_row_nnz",
                "kernel_reduce",
                "kernel_ell_max_row",
                "kernel_ell_csr_to_ell",
                "kernel_asum",
                "kernel_amax",
                "kernel_dense_spmv",
                "kernel_csr_extract_l_triangular",
                "kernel_csr_slower_nnz_per_row",
                "kernel_csr_supper_nnz_per_row",
                "kernel_csr_lower_nnz_per_row",
                "kernel_csr_upper_nnz_per_row",
                "kernel_csr_compress_count_nrow",
                "kernel_csr_compress_copy",
                "kernel_scaleaddscale_offset",
                "kernel_csr_extract_u_triangular",
                "kernel_norm",
                "kernel_dotc",
                "kernel_csr_diagmatmult_l",
                "kernel_power",
                "kernel_ell_nnz_coo",
                "kernel_ell_fill_ell",
                "kernel_ell_fill_coo",
                "kernel_copy_from_float",
                "kernel_copy_from_double",
                "kernel_dense_replace_column_vector",
                "kernel_dense_replace_row_vector",
                "kernel_dense_extract_column_vector",
                "kernel_dense_extract_row_vector",
                "kernel_csr_extract_column_vector",
                "kernel_csr_replace_column_vector_offset",
                "kernel_csr_replace_column_vector"
};

#define CL_KERNEL_SET_TO_INT                        paralution_get_kernel_ocl<int>      (  0)
#define CL_KERNEL_SET_TO                            paralution_get_kernel_ocl<ValueType>(  1)
#define CL_KERNEL_REVERSE_INDEX                     paralution_get_kernel_ocl<ValueType>(  5)
#define CL_KERNEL_BUFFER_ADDSCALAR                  paralution_get_kernel_ocl<ValueType>(  9)
#define CL_KERNEL_SCALEADD                          paralution_get_kernel_ocl<ValueType>( 13)
#define CL_KERNEL_SCALEADDSCALE                     paralution_get_kernel_ocl<ValueType>( 17)
#define CL_KERNEL_SCALEADD2                         paralution_get_kernel_ocl<ValueType>( 21)
#define CL_KERNEL_POINTWISEMULT                     paralution_get_kernel_ocl<ValueType>( 25)
#define CL_KERNEL_POINTWISEMULT2                    paralution_get_kernel_ocl<ValueType>( 29)
#define CL_KERNEL_COPY_OFFSET_FROM                  paralution_get_kernel_ocl<ValueType>( 33)
#define CL_KERNEL_PERMUTE                           paralution_get_kernel_ocl<ValueType>( 37)
#define CL_KERNEL_PERMUTE_BACKWARD                  paralution_get_kernel_ocl<ValueType>( 41)
#define CL_KERNEL_DOT                               paralution_get_kernel_ocl<ValueType>( 45)
#define CL_KERNEL_AXPY                              paralution_get_kernel_ocl<ValueType>( 49)
#define CL_KERNEL_CSR_SPMV_SCALAR                   paralution_get_kernel_ocl<ValueType>( 53)
#define CL_KERNEL_CSR_ADD_SPMV_SCALAR               paralution_get_kernel_ocl<ValueType>( 57)
#define CL_KERNEL_CSR_SCALE_DIAGONAL                paralution_get_kernel_ocl<ValueType>( 61)
#define CL_KERNEL_CSR_SCALE_OFFDIAGONAL             paralution_get_kernel_ocl<ValueType>( 65)
#define CL_KERNEL_CSR_ADD_DIAGONAL                  paralution_get_kernel_ocl<ValueType>( 69)
#define CL_KERNEL_CSR_ADD_OFFDIAGONAL               paralution_get_kernel_ocl<ValueType>( 73)
#define CL_KERNEL_CSR_EXTRACT_DIAG                  paralution_get_kernel_ocl<ValueType>( 77)
#define CL_KERNEL_CSR_EXTRACT_INV_DIAG              paralution_get_kernel_ocl<ValueType>( 81)
#define CL_KERNEL_CSR_EXTRACT_SUBMATRIX_ROW_NNZ     paralution_get_kernel_ocl<ValueType>( 85)
#define CL_KERNEL_CSR_EXTRACT_SUBMATRIX_COPY        paralution_get_kernel_ocl<ValueType>( 89)
#define CL_KERNEL_CSR_DIAGMATMULT_R                 paralution_get_kernel_ocl<ValueType>( 93)
#define CL_KERNEL_CSR_ADD_CSR_SAME_STRUCT           paralution_get_kernel_ocl<ValueType>( 97)
#define CL_KERNEL_SCALE                             paralution_get_kernel_ocl<ValueType>(101)
#define CL_KERNEL_MCSR_SPMV_SCALAR                  paralution_get_kernel_ocl<ValueType>(105)
#define CL_KERNEL_MCSR_ADD_SPMV_SCALAR              paralution_get_kernel_ocl<ValueType>(109)
#define CL_KERNEL_ELL_SPMV                          paralution_get_kernel_ocl<ValueType>(113)
#define CL_KERNEL_ELL_ADD_SPMV                      paralution_get_kernel_ocl<ValueType>(117)
#define CL_KERNEL_DIA_SPMV                          paralution_get_kernel_ocl<ValueType>(121)
#define CL_KERNEL_DIA_ADD_SPMV                      paralution_get_kernel_ocl<ValueType>(125)
#define CL_KERNEL_COO_PERMUTE                       paralution_get_kernel_ocl<ValueType>(129)
#define CL_KERNEL_COO_SPMV_FLAT                     paralution_get_kernel_ocl<ValueType>(133)
#define CL_KERNEL_COO_SPMV_REDUCE_UPDATE            paralution_get_kernel_ocl<ValueType>(137)
#define CL_KERNEL_COO_SPMV_SERIAL                   paralution_get_kernel_ocl<ValueType>(141)
#define CL_KERNEL_RED_RECURSE                       paralution_get_kernel_ocl<ValueType>(145)
#define CL_KERNEL_RED_PARTIAL_SUM                   paralution_get_kernel_ocl<ValueType>(149)
#define CL_KERNEL_RED_EXTRAPOLATE                   paralution_get_kernel_ocl<ValueType>(153)
#define CL_KERNEL_CSR_PERMUTE_ROWS                  paralution_get_kernel_ocl<ValueType>(157)
#define CL_KERNEL_CSR_PERMUTE_COLS                  paralution_get_kernel_ocl<ValueType>(161)
#define CL_KERNEL_CSR_CALC_ROW_NNZ                  paralution_get_kernel_ocl<ValueType>(165)
#define CL_KERNEL_CSR_PERMUTE_ROW_NNZ               paralution_get_kernel_ocl<ValueType>(169)
#define CL_KERNEL_REDUCE                            paralution_get_kernel_ocl<ValueType>(173)
#define CL_KERNEL_ELL_MAX_ROW                       paralution_get_kernel_ocl<ValueType>(177)
#define CL_KERNEL_ELL_CSR_TO_ELL                    paralution_get_kernel_ocl<ValueType>(181)
#define CL_KERNEL_ASUM                              paralution_get_kernel_ocl<ValueType>(185)
#define CL_KERNEL_AMAX                              paralution_get_kernel_ocl<ValueType>(189)
#define CL_KERNEL_DENSE_SPMV                        paralution_get_kernel_ocl<ValueType>(193)
#define CL_KERNEL_CSR_EXTRACT_L_TRIANGULAR          paralution_get_kernel_ocl<ValueType>(197)
#define CL_KERNEL_CSR_SLOWER_NNZ_PER_ROW            paralution_get_kernel_ocl<ValueType>(201)
#define CL_KERNEL_CSR_SUPPER_NNZ_PER_ROW            paralution_get_kernel_ocl<ValueType>(205)
#define CL_KERNEL_CSR_LOWER_NNZ_PER_ROW             paralution_get_kernel_ocl<ValueType>(209)
#define CL_KERNEL_CSR_UPPER_NNZ_PER_ROW             paralution_get_kernel_ocl<ValueType>(213)
#define CL_KERNEL_CSR_COMPRESS_COUNT_NROW           paralution_get_kernel_ocl<ValueType>(217)
#define CL_KERNEL_CSR_COMPRESS_COPY                 paralution_get_kernel_ocl<ValueType>(221)
#define CL_KERNEL_SCALEADDSCALE_OFFSET              paralution_get_kernel_ocl<ValueType>(225)
#define CL_KERNEL_CSR_EXTRACT_U_TRIANGULAR          paralution_get_kernel_ocl<ValueType>(229)
#define CL_KERNEL_NORM                              paralution_get_kernel_ocl<ValueType>(233)
#define CL_KERNEL_DOTC                              paralution_get_kernel_ocl<ValueType>(237)
#define CL_KERNEL_CSR_DIAGMATMULT_L                 paralution_get_kernel_ocl<ValueType>(241)
#define CL_KERNEL_POWER                             paralution_get_kernel_ocl<ValueType>(245)
#define CL_KERNEL_ELL_NNZ_COO                       paralution_get_kernel_ocl<ValueType>(249)
#define CL_KERNEL_ELL_FILL_ELL                      paralution_get_kernel_ocl<ValueType>(253)
#define CL_KERNEL_ELL_FILL_COO                      paralution_get_kernel_ocl<ValueType>(257)
#define CL_KERNEL_COPY_FROM_FLOAT                   paralution_get_kernel_ocl<ValueType>(261)
#define CL_KERNEL_COPY_FROM_DOUBLE                  paralution_get_kernel_ocl<ValueType>(265)
#define CL_KERNEL_DENSE_REPLACE_COLUMN_VECTOR       paralution_get_kernel_ocl<ValueType>(269)
#define CL_KERNEL_DENSE_REPLACE_ROW_VECTOR          paralution_get_kernel_ocl<ValueType>(273)
#define CL_KERNEL_DENSE_EXTRACT_COLUMN_VECTOR       paralution_get_kernel_ocl<ValueType>(277)
#define CL_KERNEL_DENSE_EXTRACT_ROW_VECTOR          paralution_get_kernel_ocl<ValueType>(281)
#define CL_KERNEL_CSR_EXTRACT_COLUMN_VECTOR         paralution_get_kernel_ocl<ValueType>(285)
#define CL_KERNEL_CSR_REPLACE_COLUMN_VECTOR_OFFSET  paralution_get_kernel_ocl<ValueType>(289)
#define CL_KERNEL_CSR_REPLACE_COLUMN_VECTOR         paralution_get_kernel_ocl<ValueType>(293)

#endif // PARALUTION_KERNELS_OCL_HPP_

# *************************************************************************
#
#    PARALUTION   www.paralution.com
#
#    Copyright (C) 2015  PARALUTION Labs UG (haftungsbeschränkt) & Co. KG
#                        Am Hasensprung 6, 76571 Gaggenau
#                        Handelsregister: Amtsgericht Mannheim, HRA 706051
#                        Vertreten durch:
#                        PARALUTION Labs Verwaltungs UG (haftungsbeschränkt)
#                        Am Hasensprung 6, 76571 Gaggenau
#                        Handelsregister: Amtsgericht Mannheim, HRB 721277
#                        Geschäftsführer: Dimitar Lukarski, Nico Trost
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# *************************************************************************



# PARALUTION version 1.0.0 


include_directories(base/gpu)

set(GPU_SOURCES
  gpu_allocate_free.cu
  backend_gpu.cu
  gpu_vector.cu
  gpu_matrix_csr.cu
  gpu_matrix_coo.cu
  gpu_matrix_dense.cu
  gpu_matrix_mcsr.cu
  gpu_matrix_bcsr.cu
  gpu_matrix_ell.cu
  gpu_matrix_dia.cu
  gpu_matrix_hyb.cu
)

set(GPU_PUBLIC_HEADERS
  gpu_allocate_free.hpp
  backend_gpu.hpp
  gpu_vector.hpp
  gpu_matrix_csr.hpp
  gpu_matrix_coo.hpp
  gpu_matrix_dense.hpp
  gpu_matrix_mcsr.hpp
  gpu_matrix_bcsr.hpp
  gpu_matrix_ell.hpp
  gpu_matrix_dia.hpp
  gpu_matrix_hyb.hpp
)

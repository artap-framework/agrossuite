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


set(HOST_SOURCES
  host_matrix_csr.cpp
  host_matrix_mcsr.cpp
  host_matrix_bcsr.cpp
  host_matrix_coo.cpp
  host_matrix_dia.cpp
  host_matrix_ell.cpp
  host_matrix_hyb.cpp
  host_matrix_dense.cpp
  host_vector.cpp
  host_conversion.cpp  
  host_affinity.cpp
  host_io.cpp
  host_stencil_laplace2d.cpp
)

set(HOST_PUBLIC_HEADERS
  host_matrix_csr.hpp
  host_matrix_mcsr.hpp
  host_matrix_bcsr.hpp
  host_matrix_coo.hpp
  host_matrix_dia.hpp
  host_matrix_ell.hpp
  host_matrix_hyb.hpp
  host_matrix_dense.hpp
  host_vector.hpp
  host_conversion.hpp
  host_affinity.hpp
  host_io.hpp
  host_stencil_laplace2d.hpp
)

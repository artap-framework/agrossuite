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


set(BASE_SOURCES
  base_paralution.cpp
  operator.cpp
  vector.cpp
  local_matrix.cpp
  local_vector.cpp
  base_matrix.cpp
  base_vector.cpp
  backend_manager.cpp
  local_stencil.cpp
  base_stencil.cpp
)

set(BASE_PUBLIC_HEADERS
  version.hpp
  matrix_formats.hpp
  matrix_formats_ind.hpp
  base_paralution.hpp
  operator.hpp
  vector.hpp
  local_matrix.hpp
  local_vector.hpp
  base_matrix.hpp
  base_vector.hpp
  backend_manager.hpp
  local_stencil.hpp
  base_stencil.hpp
  stencil_types.hpp
)

include(base/host/CMakeLists.txt)
foreach(i ${HOST_SOURCES})
  list(APPEND BASE_SOURCES host/${i})
endforeach()

foreach(i ${HOST_PUBLIC_HEADERS})
  list(APPEND BASE_PUBLIC_HEADERS host/${i})
endforeach()

if (SUPPORT_CUDA)
  include(base/gpu/CMakeLists.txt)
  foreach(i ${GPU_SOURCES})
    list(APPEND BASE_SOURCES gpu/${i})
  endforeach()

  foreach(i ${GPU_PUBLIC_HEADERS})
    list(APPEND BASE_PUBLIC_HEADERS gpu/${i})
  endforeach()
endif()

if (SUPPORT_OCL)
  include(base/ocl/CMakeLists.txt)
  foreach(i ${OCL_SOURCES})
    list(APPEND BASE_SOURCES ocl/${i})
  endforeach()

  foreach(i ${OCL_PUBLIC_HEADERS})
    list(APPEND BASE_PUBLIC_HEADERS ocl/${i})
  endforeach()
endif()

if (SUPPORT_MIC)
  include(base/mic/CMakeLists.txt)
  foreach(i ${MIC_SOURCES})
    list(APPEND BASE_SOURCES mic/${i})
  endforeach()

  foreach(i ${MIC_PUBLIC_HEADERS})
    list(APPEND BASE_PUBLIC_HEADERS mic/${i})
  endforeach()
endif()


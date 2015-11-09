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


set(SOLVERS_SOURCES
  krylov/cg.cpp
  krylov/cr.cpp
  krylov/bicgstab.cpp
  krylov/gmres.cpp
  krylov/fgmres.cpp
  krylov/idr.cpp
  multigrid/base_multigrid.cpp
  multigrid/base_amg.cpp
  multigrid/multigrid.cpp
  multigrid/amg.cpp
  deflation/dpcg.cpp
  direct/inversion.cpp
  direct/lu.cpp
  direct/qr.cpp
  solver.cpp
  chebyshev.cpp
  mixed_precision.cpp
  preconditioners/preconditioner.cpp
  preconditioners/preconditioner_ai.cpp
  preconditioners/preconditioner_as.cpp
  preconditioners/preconditioner_multielimination.cpp
  preconditioners/preconditioner_saddlepoint.cpp
  preconditioners/preconditioner_blockprecond.cpp
  preconditioners/preconditioner_multicolored.cpp
  preconditioners/preconditioner_multicolored_gs.cpp
  preconditioners/preconditioner_multicolored_ilu.cpp
  iter_ctrl.cpp
  eigenvalue/cg_hn.cpp
  eigenvalue/ampe_sira.cpp
)

set(SOLVERS_PUBLIC_HEADERS
  krylov/cg.hpp
  krylov/cr.hpp
  krylov/bicgstab.hpp
  krylov/gmres.hpp
  krylov/fgmres.hpp
  krylov/idr.hpp
  multigrid/base_multigrid.hpp
  multigrid/base_amg.hpp
  multigrid/multigrid.hpp
  multigrid/amg.hpp
  deflation/dpcg.hpp
  direct/inversion.hpp
  direct/lu.hpp
  direct/qr.hpp
  solver.hpp
  chebyshev.hpp
  mixed_precision.hpp
  preconditioners/preconditioner.hpp
  preconditioners/preconditioner_ai.hpp
  preconditioners/preconditioner_as.hpp
  preconditioners/preconditioner_multielimination.hpp
  preconditioners/preconditioner_saddlepoint.hpp
  preconditioners/preconditioner_blockprecond.hpp
  preconditioners/preconditioner_multicolored.hpp
  preconditioners/preconditioner_multicolored_gs.hpp
  preconditioners/preconditioner_multicolored_ilu.hpp
  iter_ctrl.hpp
  eigenvalue/cg_hn.hpp
  eigenvalue/ampe_sira.hpp
)

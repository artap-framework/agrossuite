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


#ifndef PARALUTION_MATRIX_FORMATS_IND_HPP_
#define PARALUTION_MATRIX_FORMATS_IND_HPP_

// Matrix indexing

// DENSE indexing
#define DENSE_IND(ai, aj, nrow, ncol) (ai) + (aj) * (nrow)
//#define DENSE_IND(ai, aj, nrow, ncol) (aj) + (ai) * (ncol)

// DENSE_IND_BASE == 0 - column-major
// DENSE_IND_BASE == 1 - row-major
#define DENSE_IND_BASE (DENSE_IND(2,2,10,0) == 22 ? 0 : 1)

// ELL indexing
#define ELL_IND_ROW(row, el, nrow, max_row) (el) * (nrow) + (row)
#define ELL_IND_EL(row, el, nrow, max_row) (el) + (max_row) * (row)

#ifdef SUPPORT_MIC
#define ELL_IND(row, el, nrow, max_row)  ELL_IND_EL(row, el, nrow, max_row)
#else
#define ELL_IND(row, el, nrow, max_row)  ELL_IND_ROW(row, el, nrow, max_row)
#endif

// DIA indexing
#define DIA_IND_ROW(row, el, nrow, ndiag) (el) * (nrow) + (row)
#define DIA_IND_EL(row, el, nrow, ndiag) (el) + (ndiag) * (row)

#ifdef SUPPORT_MIC
#define DIA_IND(row, el, nrow, ndiag) DIA_IND_EL(row, el, nrow, ndiag)
#else
#define DIA_IND(row, el, nrow, ndiag) DIA_IND_ROW(row, el, nrow, ndiag)
#endif


#endif // PARALUTION_MATRIX_FORMATS_IND_HPP_

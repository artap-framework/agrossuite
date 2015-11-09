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


#ifndef PARALUTION_OPERATOR_HPP_
#define PARALUTION_OPERATOR_HPP_

#include "base_paralution.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

namespace paralution {

template <typename ValueType>
class LocalVector;

/// Operator class defines the generic interface
/// for applying an operator (e.g. matrix, stencil)
/// from/to global and local vectors
template <typename ValueType>
class Operator : public BaseParalution<ValueType> {

public:

  Operator();
  virtual ~Operator();

  /// Return the number of rows in the matrix/stencil
  virtual int get_nrow(void) const = 0;
  /// Return the number of columns in the matrix/stencil
  virtual int get_ncol(void) const = 0;
  /// Return the number of non-zeros in the matrix/stencil
  virtual int get_nnz(void) const = 0;

  /// Return the number of rows in the local matrix/stencil
  virtual int get_local_nrow(void) const;
  /// Return the number of columns in the local matrix/stencil
  virtual int get_local_ncol(void) const;
  /// Return the number of non-zeros in the local matrix/stencil
  virtual int get_local_nnz(void) const;

  /// Return the number of rows in the ghost matrix/stencil
  virtual int get_ghost_nrow(void) const;
  /// Return the number of columns in the ghost matrix/stencil
  virtual int get_ghost_ncol(void) const;
  /// Return the number of non-zeros in the ghost matrix/stencil
  virtual int get_ghost_nnz(void) const;


  /// Apply the operator, out = Operator(in), where in, out are local vectors
  virtual void Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const;

  /// Apply and add the operator, out = out + scalar*Operator(in), where in, out are local vectors
  virtual void ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar,
                        LocalVector<ValueType> *out) const;

};


}

#endif // PARALUTION_OPERTOR_HPP_

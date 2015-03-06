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


#include "../../utils/def.hpp"
#include "multigrid.hpp"

#include "../../base/local_matrix.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"

#include <complex>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
MultiGrid<OperatorType, VectorType, ValueType>::MultiGrid() {

  LOG_DEBUG(this, "MultiGrid::MultiGrid()",
            "default constructor");

  this->scaling_ = true;

}

template <class OperatorType, class VectorType, typename ValueType>
MultiGrid<OperatorType, VectorType, ValueType>::~MultiGrid() {

  LOG_DEBUG(this, "MultiGrid::~MultiGrid()",
            "destructor");

    delete[] this->restrict_op_level_;
    delete[] this->prolong_op_level_;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetRestrictOperator(OperatorType **op) {

  LOG_DEBUG(this, "MultiGrid::SetRestrictOperator()",
            "");

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->levels_ > 0);

  this->restrict_op_level_ = new Operator<ValueType>*[this->levels_];

  for (int i=0; i<this->levels_-1; ++i)
    this->restrict_op_level_[i] = op[i];

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetProlongOperator(OperatorType **op) {

  LOG_DEBUG(this, "MultiGrid::SetProlongOperator()",
            "");

  assert(this->build_ == false);
  assert(op != NULL);
  assert(this->levels_ > 0);

  this->prolong_op_level_ = new Operator<ValueType>*[this->levels_];

  for (int i=0; i<this->levels_-1; ++i)
    this->prolong_op_level_[i] = op[i];

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiGrid<OperatorType, VectorType, ValueType>::SetOperatorHierarchy(OperatorType **op) {

  LOG_DEBUG(this, "MultiGrid::SetOperatorHierarchy()",
            "");
  
  assert(this->build_ == false);
  assert(op != NULL );

  this->op_level_ = op;

}


template class MultiGrid< LocalMatrix<double>, LocalVector<double>, double >;
template class MultiGrid< LocalMatrix<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class MultiGrid< LocalMatrix<std::complex<double> >, LocalVector<std::complex<double> >, std::complex<double> >;
template class MultiGrid< LocalMatrix<std::complex<float> >,  LocalVector<std::complex<float> >,  std::complex<float> >;
#endif

}

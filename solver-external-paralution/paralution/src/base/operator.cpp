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


#include "../utils/def.hpp"
#include "operator.hpp"
#include "vector.hpp"
#include "local_vector.hpp"
#include "../utils/log.hpp"

#include <complex>

namespace paralution {

template <typename ValueType>
Operator<ValueType>::Operator() {

  LOG_DEBUG(this, "Operator::Operator()",
            "default constructor");

  this->object_name_ = "";

}

template <typename ValueType>
Operator<ValueType>::~Operator() {

  LOG_DEBUG(this, "Operator::~Operator()",
            "default destructor");

}

template <typename ValueType>
int Operator<ValueType>::get_local_nrow(void) const {
 
  return this->get_nrow();
}

template <typename ValueType>
int Operator<ValueType>::get_local_ncol(void) const {
 
  return this->get_ncol();
}

template <typename ValueType>
int Operator<ValueType>::get_local_nnz(void) const {
 
  return this->get_nnz();
}

template <typename ValueType>
int Operator<ValueType>::get_ghost_nrow(void) const {
 
  return 0;
}

template <typename ValueType>
int Operator<ValueType>::get_ghost_ncol(void) const {
 
  return 0;
}

template <typename ValueType>
int Operator<ValueType>::get_ghost_nnz(void) const {
 
  return 0;
}

template <typename ValueType>
void Operator<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  LOG_INFO("Operator<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out)");
  LOG_INFO("Mismatched types:");
  this->info();
  in.info();
  out->info();
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void Operator<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, LocalVector<ValueType> *out) const {

  LOG_INFO("Operator<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, LocalVector<ValueType> *out)");
  LOG_INFO("Mismatched types:");
  this->info();
  in.info();
  out->info();
  FATAL_ERROR(__FILE__, __LINE__); 

}


template class Operator<double>;
template class Operator<float>;
#ifdef SUPPORT_COMPLEX
template class Operator<std::complex<double> >;
template class Operator<std::complex<float> >;
#endif

template class Operator<int>;

}

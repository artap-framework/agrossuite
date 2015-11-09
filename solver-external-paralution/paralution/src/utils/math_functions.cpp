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


#include "def.hpp"
#include "math_functions.hpp"

#include <stdlib.h>
#include <math.h>

namespace paralution {

float paralution_abs(const float val) {

  return std::fabs(val);

}

double paralution_abs(const double val) {

  return std::fabs(val);

}

float paralution_abs(const std::complex<float> val) {

  return std::abs(val);

}

double paralution_abs(const std::complex<double> val) {

  return std::abs(val);

}

int paralution_abs(const int val) {

  return abs(val);

}

template <typename ValueType>
bool operator<(const std::complex<ValueType> &lhs, const std::complex<ValueType> &rhs) {

  if (&lhs == &rhs)
    return false;

  assert(lhs.imag() == rhs.imag() && lhs.imag() == ValueType(0.0));

  return lhs.real() < rhs.real();

}

template <typename ValueType>
bool operator>(const std::complex<ValueType> &lhs, const std::complex<ValueType> &rhs) {

  if (&lhs == &rhs)
    return false;

  assert(lhs.imag() == rhs.imag() && lhs.imag() == ValueType(0.0));

  return lhs.real() > rhs.real();

}

template <typename ValueType>
bool operator<=(const std::complex<ValueType> &lhs, const std::complex<ValueType> &rhs) {

  if (&lhs == &rhs)
    return true;

  assert(lhs.imag() == rhs.imag() && lhs.imag() == ValueType(0.0));

  return lhs.real() <= rhs.real();

}

template <typename ValueType>
bool operator>=(const std::complex<ValueType> &lhs, const std::complex<ValueType> &rhs) {

  if (&lhs == &rhs)
    return true;

  assert(lhs.imag() == rhs.imag() && lhs.imag() == ValueType(0.0));

  return lhs.real() >= rhs.real();

}


template bool operator<(const std::complex<float> &lhs, const std::complex<float> &rhs);
template bool operator<(const std::complex<double> &lhs, const std::complex<double> &rhs);

template bool operator>(const std::complex<float> &lhs, const std::complex<float> &rhs);
template bool operator>(const std::complex<double> &lhs, const std::complex<double> &rhs);

template bool operator<=(const std::complex<float> &lhs, const std::complex<float> &rhs);
template bool operator<=(const std::complex<double> &lhs, const std::complex<double> &rhs);

template bool operator>=(const std::complex<float> &lhs, const std::complex<float> &rhs);
template bool operator>=(const std::complex<double> &lhs, const std::complex<double> &rhs);

}

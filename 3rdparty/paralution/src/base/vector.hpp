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


#ifndef PARALUTION_VECTOR_HPP_
#define PARALUTION_VECTOR_HPP_

#include "base_paralution.hpp"

#include <iostream>
#include <string>
#include <cstdlib>

namespace paralution {

template <typename ValueType>
class LocalVector;

// Vector
template <typename ValueType>
class Vector : public BaseParalution<ValueType> {

public:

  Vector();
  virtual ~Vector();

  /// Return the size of the vector
  virtual int get_size(void) const = 0;

  /// Return true if the vector is ok (empty vector is also ok)
  /// and false if some of values are NaN
  virtual bool Check(void) const = 0;

  /// Clear (free) the vector
  virtual void Clear() = 0;

  /// Set the values of the vector to zero
  virtual void Zeros() = 0;

  /// Set the values of the vector to one
  virtual void Ones() = 0;

  /// Set the values of the vector to given argument
  virtual void SetValues(const ValueType val) = 0;

  /// Set random values from interval [a,b]
  virtual void SetRandom(const ValueType a = -1.0, const ValueType b = 1.0, const int seed = 0) = 0;

  /// Read vector from ASCII file
  virtual void ReadFileASCII(const std::string) = 0;

  /// Write vector to ASCII file
  virtual void WriteFileASCII(const std::string) const = 0;

  /// Read vector from binary file
  virtual void ReadFileBinary(const std::string) = 0;

  /// Write vector to binary file
  virtual void WriteFileBinary(const std::string) const = 0;

  /// Copy values from another local vector
  virtual void CopyFrom(const LocalVector<ValueType> &src);

  /// Async copy
  virtual void CopyFromAsync(const LocalVector<ValueType> &src);

  /// Copy values from float vector
  virtual void CopyFromFloat(const LocalVector<float> &src);

  /// Copy values from double vector
  virtual void CopyFromDouble(const LocalVector<double> &src);

  /// Clone the entire vector (data+backend descr) from another local vector
  virtual void CloneFrom(const LocalVector<ValueType> &src);

  /// Perform vector update of type this = this + alpha*x
  virtual void AddScale(const LocalVector<ValueType> &x, const ValueType alpha);

  /// Perform vector update of type this = alpha*this + x
  virtual void ScaleAdd(const ValueType alpha, const LocalVector<ValueType> &x);

  /// Perform vector update of type this = alpha*this + x*beta
  virtual void ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x,
                             const ValueType beta);

  /// Perform vector update of type this = alpha*this + x*beta + y*gamma
  virtual void ScaleAdd2(const ValueType alpha, const LocalVector<ValueType> &x,
                         const ValueType beta, const LocalVector<ValueType> &y, const ValueType gamma);

  /// Perform vector scaling this = alpha*this
  virtual void Scale(const ValueType alpha) = 0;

  /// Computes partial sum
  virtual void PartialSum(const LocalVector<ValueType> &x);

  /// Compute dot (scalar) product, return this^T y
  virtual ValueType Dot(const LocalVector<ValueType> &x) const;

  /// Compute non-conjugate dot (scalar) product, return this^T y
  virtual ValueType DotNonConj(const LocalVector<ValueType> &x) const;

  /// Compute L2 norm of the vector, return =  srqt(this^T this)
  virtual ValueType Norm(void) const = 0;

  /// Reduce the vector
  virtual ValueType Reduce(void) const = 0;

  /// Compute the sum of the absolute values of the vector (L1 norm), return =  sum(|this|)
  virtual ValueType Asum(void) const = 0;

  /// Compute the absolute max value of the vector, return = index(max(|this|))
  virtual int Amax(ValueType &value) const = 0;

  /// Perform point-wise multiplication (element-wise) of type this = this * x
  virtual void PointWiseMult(const LocalVector<ValueType> &x);

  /// Perform point-wise multiplication (element-wise) of type this = x*y
  virtual void PointWiseMult(const LocalVector<ValueType> &x, const LocalVector<ValueType> &y);

  /// Perform power operation to a vector
  virtual void Power(const double power) = 0;

};


}

#endif // PARALUTION_VECTOR_HPP_

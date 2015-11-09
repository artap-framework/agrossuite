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


#ifndef PARALUTION_LOCAL_VECTOR_HPP_
#define PARALUTION_LOCAL_VECTOR_HPP_

#include "vector.hpp"
#include "base_vector.hpp"
#include "host/host_vector.hpp"

namespace paralution {

template <typename ValueType>
class LocalMatrix;

template <typename ValueType>
class LocalStencil;

// Local vector
template <typename ValueType>
class LocalVector : public Vector<ValueType> {

public:

  LocalVector();
  virtual ~LocalVector();

  virtual void MoveToAccelerator(void);
  virtual void MoveToAcceleratorAsync(void);
  virtual void MoveToHost(void);
  virtual void MoveToHostAsync(void);
  virtual void Sync(void);

  virtual void info(void) const;
  virtual int get_size(void) const;

  virtual bool Check(void) const;

  /// Allocate a local vector with name and size
  virtual void Allocate(std::string name, const int size);

  /// Initialize a vector with externally allocated data
  void SetDataPtr(ValueType **ptr, std::string name, const int size);
  /// Get a pointer from the vector data and free the vector object
  void LeaveDataPtr(ValueType **ptr);

  /// Assembling
  void Assemble(const int *i, const ValueType *v,
                int size, std::string name, const int n=0);

  virtual void Clear();
  virtual void Zeros();
  virtual void Ones();
  virtual void SetValues(const ValueType val);
  virtual void SetRandom(const ValueType a = -1.0, const ValueType b = 1.0, const int seed = 0);

  /// Access operator (only for host data!)
  ValueType& operator[](const int i);
  /// Access operator (only for host data!)
  const ValueType& operator[](const int i) const;

  virtual void ReadFileASCII(const std::string filename);
  virtual void WriteFileASCII(const std::string filename) const;
  virtual void ReadFileBinary(const std::string filename);
  virtual void WriteFileBinary(const std::string filename) const;

  virtual void CopyFrom(const LocalVector<ValueType> &src);
  virtual void CopyFromAsync(const LocalVector<ValueType> &src);
  virtual void CopyFromFloat(const LocalVector<float> &src);
  virtual void CopyFromDouble(const LocalVector<double> &src);

  /// Copy data (not entire vector) from another local vector with specified src/dst offsets and size
  void CopyFrom(const LocalVector<ValueType> &src,
                const int src_offset,
                const int dst_offset,
                const int size);

  /// Copy a local vector under specified permutation (forward permutation)
  void CopyFromPermute(const LocalVector<ValueType> &src,
                       const LocalVector<int> &permutation);

  /// Copy a local vector under specified permutation (backward permutation)
  void CopyFromPermuteBackward(const LocalVector<ValueType> &src,
                               const LocalVector<int> &permutation);

  /// Clone the entire vector (values,backend descr) from another LocalVector
  void CloneFrom(const LocalVector<ValueType> &src);

  /// Copy (import) vector described in one array (values)
  /// The object data has to be allocated (call Allocate first)
  void CopyFromData(const ValueType *data);

  /// Copy (export) vector described in one array (values)
  /// The output array has to be allocated
  void CopyToData(ValueType *data) const;

  /// Perform inplace permutation (forward) of the vector
  void Permute(const LocalVector<int> &permutation);

  /// Perform inplace permutation (backward) of the vector
  void PermuteBackward(const LocalVector<int> &permutation);

  /// Restriction operator based on restriction mapping vector
  void Restriction(const LocalVector<ValueType> &vec_fine, const LocalVector<int> &map);

  /// Prolongation operator based on restriction(!) mapping vector
  void Prolongation(const LocalVector<ValueType> &vec_coarse, const LocalVector<int> &map);

  virtual void AddScale(const LocalVector<ValueType> &x, const ValueType alpha);
  virtual void ScaleAdd(const ValueType alpha, const LocalVector<ValueType> &x);
  virtual void ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta);
  /// Perform vector update of type this = alpha*this + x*beta with offsets for a specified part of a vector
  void ScaleAddScale(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta,
                     const int src_offset,
                     const int dst_offset,
                     const int size);

  virtual void ScaleAdd2(const ValueType alpha, const LocalVector<ValueType> &x, const ValueType beta, const LocalVector<ValueType> &y, const ValueType gamma);
  virtual void Scale(const ValueType alpha);
  virtual void PartialSum(const LocalVector<ValueType> &x);
  virtual ValueType Dot(const LocalVector<ValueType> &x) const;
  virtual ValueType DotNonConj(const LocalVector<ValueType> &x) const;
  virtual ValueType Norm(void) const;
  virtual ValueType Reduce(void) const;
  virtual ValueType Asum(void) const;
  virtual int Amax(ValueType &value) const;
  virtual void PointWiseMult(const LocalVector<ValueType> &x);
  virtual void PointWiseMult(const LocalVector<ValueType> &x, const LocalVector<ValueType> &y);
  virtual void Power(const double power);

protected:

  virtual bool is_host(void) const;
  virtual bool is_accel(void) const;

private:

  /// Pointer from the base vector class to the current allocated vector (host_ or accel_)
  BaseVector<ValueType> *vector_;

  /// Host Vector
  HostVector<ValueType> *vector_host_;

  /// Accelerator Vector
  AcceleratorVector<ValueType> *vector_accel_;

  friend class LocalVector<double>;
  friend class LocalVector<float>;
  friend class LocalVector<std::complex<double> >;
  friend class LocalVector<std::complex<float> >;
  friend class LocalVector<int>;

  friend class LocalMatrix<double>;
  friend class LocalMatrix<float>;
  friend class LocalMatrix<std::complex<double> >;
  friend class LocalMatrix<std::complex<float> >;

  friend class LocalStencil<double>;
  friend class LocalStencil<float>;
  friend class LocalStencil<std::complex<double> >;
  friend class LocalStencil<std::complex<float> >;

  friend class LocalMatrix<ValueType>;

};


}

#endif // PARALUTION_LOCAL_VECTOR_HPP_

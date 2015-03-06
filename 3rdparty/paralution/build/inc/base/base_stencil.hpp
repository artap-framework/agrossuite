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


#ifndef PARALUTION_BASE_STENCIL_HPP_
#define PARALUTION_BASE_STENCIL_HPP_

#include "base_vector.hpp"

namespace paralution {

// Forward declartions
template <typename ValueType>
class HostStencil;
template <typename ValueType>
class HostStencilLaplace2D;


template <typename ValueType>
class AcceleratorStencil;

template <typename ValueType>
class GPUAcceleratorStencil;
template <typename ValueType>
class GPUAcceleratorStencilLaplace2D;

template <typename ValueType>
class OCLAcceleratorStencil;
template <typename ValueType>
class OCLAcceleratorStencilLaplace2D;


template <typename ValueType>
class MICAcceleratorStencil;
template <typename ValueType>
class MICAcceleratorStencilLaplace2D;


/// Base class for all host/accelerator stencils
template <typename ValueType>
class BaseStencil {

public:

  BaseStencil();
  virtual ~BaseStencil();

  /// Return the number of rows in the stencil
  int get_nrow(void) const;
  /// Return the number of columns in the stencil
  int get_ncol(void) const;
  /// Return the dimension of the stencil
  int get_ndim(void) const;
  /// Return the nnz per row
  virtual int get_nnz(void) const = 0;

  /// Shows simple info about the object
  virtual void info(void) const = 0;
  /// Return the stencil format id (see stencil_formats.hpp)
  virtual unsigned int get_stencil_id(void) const = 0 ;
  /// Copy the backend descriptor information
  virtual void set_backend(const Paralution_Backend_Descriptor local_backend);
  // Set the grid size
  virtual void SetGrid(const int size);

  /// Apply the stencil to vector, out = this*in;
  virtual void Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const = 0; 
  /// Apply and add the stencil to vector, out = out + scalar*this*in;
  virtual void ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                        BaseVector<ValueType> *out) const = 0; 

protected:

  /// Number of rows
  int ndim_;
  /// Number of columns
  int size_;


  /// Backend descriptor (local copy)
  Paralution_Backend_Descriptor local_backend_;

  friend class BaseVector<ValueType>;
  friend class HostVector<ValueType>;
  friend class AcceleratorVector<ValueType>;
  friend class GPUAcceleratorVector<ValueType>;
  friend class OCLAcceleratorVector<ValueType>;
  friend class MICAcceleratorVector<ValueType>;

};

template <typename ValueType>
class HostStencil : public BaseStencil<ValueType> {

public:

  HostStencil();
  virtual ~HostStencil();

};

template <typename ValueType>
class AcceleratorStencil : public BaseStencil<ValueType> {

public:

  AcceleratorStencil();
  virtual ~AcceleratorStencil();

  /// Copy (accelerator stencil) from host stencil
  virtual void CopyFromHost(const HostStencil<ValueType> &src) = 0;

  /// Copy (accelerator stencil) to host stencil
  virtual void CopyToHost(HostStencil<ValueType> *dst) const = 0;

};

template <typename ValueType>
class GPUAcceleratorStencil : public AcceleratorStencil<ValueType> {

public:

  GPUAcceleratorStencil();
  virtual ~GPUAcceleratorStencil();

};

template <typename ValueType>
class OCLAcceleratorStencil : public AcceleratorStencil<ValueType> {

public:

  OCLAcceleratorStencil();
  virtual ~OCLAcceleratorStencil();

};

template <typename ValueType>
class MICAcceleratorStencil : public AcceleratorStencil<ValueType> {

public:

  MICAcceleratorStencil();
  virtual ~MICAcceleratorStencil();

};


}

#endif // PARALUTION_BASE_STENCIL_HPP_

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
#include "host_stencil_laplace2d.hpp"
#include "host_vector.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../stencil_types.hpp"

#include <complex>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_set_num_threads(num);
#endif

namespace paralution {

template <typename ValueType>
HostStencilLaplace2D<ValueType>::HostStencilLaplace2D() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
HostStencilLaplace2D<ValueType>::HostStencilLaplace2D(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "HostStencilLaplace2D::HostStencilLaplace2D()",
            "constructor with local_backend");

  this->set_backend(local_backend);

  this->ndim_ = 2;
}

template <typename ValueType>
HostStencilLaplace2D<ValueType>::~HostStencilLaplace2D() {

  LOG_DEBUG(this, "HostStencilLaplace2D::~HostStencilLaplace2D()",
            "destructor");

}

template <typename ValueType>
void HostStencilLaplace2D<ValueType>::info(void) const {

  LOG_INFO("Stencil 2D Laplace (Host) size=" << this->size_ << " dim=" << this->get_ndim());

}

template <typename ValueType>
int HostStencilLaplace2D<ValueType>::get_nnz(void) const {

  return 5;
}

template <typename ValueType>
void HostStencilLaplace2D<ValueType>::Apply(const BaseVector<ValueType> &in, BaseVector<ValueType> *out) const {

  if ((this->ndim_ > 0) && (this->size_ > 0)) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    int nrow = this->get_nrow();
    assert(in.  get_size() == nrow);
    assert(out->get_size() == nrow);
    assert(out->get_size() == in.  get_size());

    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in);
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    _set_omp_backend_threads(this->local_backend_, nrow);

    int idx = 0;

    // interior
#pragma omp parallel for
    for (int i=1; i<this->size_-1; ++i)
      for (int j=1; j<this->size_-1; ++j) {
        idx = i*this->size_ + j;

        cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-this->size_] //i-1
          + ValueType(-1.0)*cast_in->vec_[idx-1] // j-1
          + ValueType(4.0)*cast_in->vec_[idx] // i,j
          + ValueType(-1.0)*cast_in->vec_[idx+1] // j+1
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_]; //i+1
        
      }
    
    
    // boundary layers

#pragma omp parallel for
      for (int j=1; j<this->size_-1; ++j) {
        idx = 0*this->size_ + j;

        cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-1]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+1]
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         

        idx = (this->size_-1)*this->size_ + j;

        cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-this->size_]
          + ValueType(-1.0)*cast_in->vec_[idx-1]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+1];

        
      }

#pragma omp parallel for
      for (int i=1; i<this->size_-1; ++i) {
        idx = i*this->size_ + 0;

        cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-this->size_]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+1]
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         


        idx = i*this->size_ + this->size_-1;

        cast_out->vec_[idx] =  ValueType(-1.0)*cast_in->vec_[idx-this->size_]
          + ValueType(-1.0)*cast_in->vec_[idx-1]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         

        
      }


    // boundary points
    
    idx = 0*(this->size_) + 0;
    cast_out->vec_[idx] = ValueType(4.0)*cast_in->vec_[idx]
      + ValueType(-1.0)*cast_in->vec_[idx+1]
      + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         
    
    idx = 0*(this->size_) + this->size_-1;
    cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-1]
      + ValueType(4.0)*cast_in->vec_[idx]
      + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         
    
    idx = (this->size_-1)*(this->size_) + 0;
    cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-this->size_]
      + ValueType(4.0)*cast_in->vec_[idx]
      + ValueType(-1.0)*cast_in->vec_[idx+1];
    
    idx = (this->size_-1)*(this->size_) + this->size_-1;
    cast_out->vec_[idx] = ValueType(-1.0)*cast_in->vec_[idx-this->size_]
      + ValueType(-1.0)*cast_in->vec_[idx-1]
      + ValueType(4.0)*cast_in->vec_[idx];
    
  }

}

template <typename ValueType>
void HostStencilLaplace2D<ValueType>::ApplyAdd(const BaseVector<ValueType> &in, const ValueType scalar,
                                         BaseVector<ValueType> *out) const {

  if ((this->ndim_ > 0) && (this->size_ > 0)) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    int nrow = this->get_nrow();
    assert(in.  get_size() == nrow);
    assert(out->get_size() == nrow);
    assert(out->get_size() == in.  get_size());

    const HostVector<ValueType> *cast_in = dynamic_cast<const HostVector<ValueType>*> (&in);
    HostVector<ValueType> *cast_out      = dynamic_cast<      HostVector<ValueType>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    _set_omp_backend_threads(this->local_backend_, nrow);

    int idx = 0;

    // interior
#pragma omp parallel for
    for (int i=1; i<this->size_-1; ++i)
      for (int j=1; j<this->size_-1; ++j) {
        idx = i*this->size_ + j;

        cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-this->size_] //i-1
          + ValueType(-1.0)*cast_in->vec_[idx-1] // j-1
          + ValueType(4.0)*cast_in->vec_[idx] // i,j
          + ValueType(-1.0)*cast_in->vec_[idx+1] // j+1
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_]; //i+1
        
      }
    
    
    // boundary layers

#pragma omp parallel for
      for (int j=1; j<this->size_-1; ++j) {
        idx = 0*this->size_ + j;

        cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-1]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+1]
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         

        idx = (this->size_-1)*this->size_ + j;

        cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-this->size_]
          + ValueType(-1.0)*cast_in->vec_[idx-1]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+1];

        
      }

#pragma omp parallel for
      for (int i=1; i<this->size_-1; ++i) {
        idx = i*this->size_ + 0;

        cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-this->size_]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+1]
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         


        idx = i*this->size_ + this->size_-1;

        cast_out->vec_[idx] +=  ValueType(-1.0)*cast_in->vec_[idx-this->size_]
          + ValueType(-1.0)*cast_in->vec_[idx-1]
          + ValueType(4.0)*cast_in->vec_[idx]
          + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         

        
      }


    // boundary points
    
    idx = 0*(this->size_) + 0;
    cast_out->vec_[idx] += ValueType(4.0)*cast_in->vec_[idx]
      + ValueType(-1.0)*cast_in->vec_[idx+1]
      + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         
    
    idx = 0*(this->size_) + this->size_-1;
    cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-1]
      + ValueType(4.0)*cast_in->vec_[idx]
      + ValueType(-1.0)*cast_in->vec_[idx+this->size_];         
    
    idx = (this->size_-1)*(this->size_) + 0;
    cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-this->size_]
      + ValueType(4.0)*cast_in->vec_[idx]
      + ValueType(-1.0)*cast_in->vec_[idx+1];
    
    idx = (this->size_-1)*(this->size_) + this->size_-1;
    cast_out->vec_[idx] += ValueType(-1.0)*cast_in->vec_[idx-this->size_]
      + ValueType(-1.0)*cast_in->vec_[idx-1]
      + ValueType(4.0)*cast_in->vec_[idx];
    
  }


}


template class HostStencilLaplace2D<double>;
template class HostStencilLaplace2D<float>;
#ifdef SUPPORT_COMPLEX
template class HostStencilLaplace2D<std::complex<double> >;
template class HostStencilLaplace2D<std::complex<float> >;
#endif

}

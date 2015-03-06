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
#include "base_stencil.hpp"
#include "base_vector.hpp"
#include "backend_manager.hpp"
#include "../utils/log.hpp"

#include <stdlib.h>
#include <complex>

namespace paralution {

template <typename ValueType>
BaseStencil<ValueType>::BaseStencil() {

  LOG_DEBUG(this, "BaseStencil::BaseStencil()",
            "default constructor");

  this->ndim_ = 0;
  this->size_ = 0;

}

template <typename ValueType>
BaseStencil<ValueType>::~BaseStencil() {

  LOG_DEBUG(this, "BaseStencil::~BaseStencil()",
            "default destructor");

}

template <typename ValueType>
int BaseStencil<ValueType>::get_nrow(void) const {

  int dim = 1;

  if (this->get_ndim() > 0) {

    for (int i=0; i<ndim_; ++i)
      dim *= this->size_;

  }

  return dim;

} 

template <typename ValueType>
int BaseStencil<ValueType>::get_ncol(void) const {

  return this->get_nrow(); 
} 

template <typename ValueType>
int BaseStencil<ValueType>::get_ndim(void) const {

  return this->ndim_; 
} 

template <typename ValueType>
void BaseStencil<ValueType>::set_backend(const Paralution_Backend_Descriptor local_backend) {

  this->local_backend_ = local_backend;

}

template <typename ValueType>
void BaseStencil<ValueType>::SetGrid(const int size) {

  assert(size >= 0);
  this->size_ = size;

}





template <typename ValueType>
HostStencil<ValueType>::HostStencil() {
}

template <typename ValueType>
HostStencil<ValueType>::~HostStencil() {
}



template <typename ValueType>
AcceleratorStencil<ValueType>::AcceleratorStencil() {
}

template <typename ValueType>
AcceleratorStencil<ValueType>::~AcceleratorStencil() {
}


template <typename ValueType>
GPUAcceleratorStencil<ValueType>::GPUAcceleratorStencil() {
}

template <typename ValueType>
GPUAcceleratorStencil<ValueType>::~GPUAcceleratorStencil() {
}


template <typename ValueType>
OCLAcceleratorStencil<ValueType>::OCLAcceleratorStencil() {
}

template <typename ValueType>
OCLAcceleratorStencil<ValueType>::~OCLAcceleratorStencil() {
}


template <typename ValueType>
MICAcceleratorStencil<ValueType>::MICAcceleratorStencil() {
}

template <typename ValueType>
MICAcceleratorStencil<ValueType>::~MICAcceleratorStencil() {
}


template class BaseStencil<double>;
template class BaseStencil<float>;
#ifdef SUPPORT_COMPLEX
template class BaseStencil<std::complex<double> >;
template class BaseStencil<std::complex<float> >;
#endif
template class BaseStencil<int>;

template class HostStencil<double>;
template class HostStencil<float>;
#ifdef SUPPORT_COMPLEX
template class HostStencil<std::complex<double> >;
template class HostStencil<std::complex<float> >;
#endif
template class HostStencil<int>;

template class AcceleratorStencil<double>;
template class AcceleratorStencil<float>;
#ifdef SUPPORT_COMPLEX
template class AcceleratorStencil<std::complex<double> >;
template class AcceleratorStencil<std::complex<float> >;
#endif
template class AcceleratorStencil<int>;

template class GPUAcceleratorStencil<double>;
template class GPUAcceleratorStencil<float>;
#ifdef SUPPORT_COMPLEX
template class GPUAcceleratorStencil<std::complex<double> >;
template class GPUAcceleratorStencil<std::complex<float> >;
#endif
template class GPUAcceleratorStencil<int>;

template class OCLAcceleratorStencil<double>;
template class OCLAcceleratorStencil<float>;
#ifdef SUPPORT_COMPLEX
template class OCLAcceleratorStencil<std::complex<double> >;
template class OCLAcceleratorStencil<std::complex<float> >;
#endif
template class OCLAcceleratorStencil<int>;

template class MICAcceleratorStencil<double>;
template class MICAcceleratorStencil<float>;
#ifdef SUPPORT_COMPLEX
template class MICAcceleratorStencil<std::complex<double> >;
template class MICAcceleratorStencil<std::complex<float> >;
#endif
template class MICAcceleratorStencil<int>;

}

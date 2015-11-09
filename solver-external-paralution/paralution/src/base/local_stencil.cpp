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
#include "local_stencil.hpp"
#include "local_vector.hpp"
#include "stencil_types.hpp"
#include "host/host_stencil_laplace2d.hpp"

#include "../utils/log.hpp"

#include <complex>

namespace paralution {

template <typename ValueType>
LocalStencil<ValueType>::LocalStencil() {

  LOG_DEBUG(this, "LocalStencil::LocalStencil()",
            "default constructor");

  this->object_name_ = "";

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
LocalStencil<ValueType>::~LocalStencil() {

  LOG_DEBUG(this, "LocalStencil::~LocalStencil()",
            "default destructor");

  delete this->stencil_;

}

template <typename ValueType>
LocalStencil<ValueType>::LocalStencil(unsigned int type) {

  LOG_DEBUG(this, "LocalStencil::LocalStencil()",
            "constructor with stencil type");

  assert(type == Laplace2D); // the only one at the moment

  this->object_name_ = _stencil_type_names[type];

  this->stencil_host_ = new HostStencilLaplace2D<ValueType>(this->local_backend_);
  this->stencil_ = this->stencil_host_;

}

template <typename ValueType>
int LocalStencil<ValueType>::get_ndim(void) const {

  return this->stencil_->get_ndim();
}

template <typename ValueType>
int LocalStencil<ValueType>::get_nnz(void) const {

  return this->stencil_->get_nnz();
}

template <typename ValueType>
int LocalStencil<ValueType>::get_nrow(void) const {

  return this->stencil_->get_nrow();
}

template <typename ValueType>
int LocalStencil<ValueType>::get_ncol(void) const {

  return this->stencil_->get_ncol();
}


template <typename ValueType>
void LocalStencil<ValueType>::info(void) const {

  this->stencil_->info();

}

template <typename ValueType>
void LocalStencil<ValueType>::Clear(void) {


  LOG_DEBUG(this, "LocalStencil::Clear()",
            "");

  this->stencil_->SetGrid(0);


}

template <typename ValueType>
void LocalStencil<ValueType>::SetGrid(const int size) {

  LOG_DEBUG(this, "LocalStencil::SetGrid()",
            size);


  this->stencil_->SetGrid(size);

}


template <typename ValueType>
void LocalStencil<ValueType>::Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const {

  LOG_DEBUG(this, "LocalStencil::Apply()",
            "");

  assert(&in != NULL);
  assert(out != NULL);

  assert( ( (this->stencil_ == this->stencil_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->stencil_ == this->stencil_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  this->stencil_->Apply(*in.vector_, out->vector_);
  
}

template <typename ValueType>
void LocalStencil<ValueType>::ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, 
                                       LocalVector<ValueType> *out) const {

  LOG_DEBUG(this, "LocalStencil::ApplyAdd()",
            "");

  assert(&in != NULL);
  assert(out != NULL);

  assert( ( (this->stencil_ == this->stencil_host_)  && (in.vector_ == in.vector_host_) && (out->vector_ == out->vector_host_)) ||
          ( (this->stencil_ == this->stencil_accel_) && (in.vector_ == in.vector_accel_) && (out->vector_ == out->vector_accel_)) );

  this->stencil_->Apply(*in.vector_, out->vector_);

}

template <typename ValueType>
void LocalStencil<ValueType>::MoveToAccelerator(void) {

  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
void LocalStencil<ValueType>::MoveToHost(void) {

  LOG_INFO("The function is not implemented (yet)!");
  FATAL_ERROR(__FILE__, __LINE__);

}


template class LocalStencil<double>;
template class LocalStencil<float>;
#ifdef SUPPORT_COMPLEX
template class LocalStencil<std::complex<double> >;
template class LocalStencil<std::complex<float> >;
#endif

}

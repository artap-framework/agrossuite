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
#include "qr.hpp"

#include "../../base/local_matrix.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"

#include <math.h>
#include <complex>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
QR<OperatorType, VectorType, ValueType>::QR() {

  LOG_DEBUG(this, "QR::QR()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
QR<OperatorType, VectorType, ValueType>::~QR() {

  LOG_DEBUG(this, "QR::~QR()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("QR solver");

}


template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  LOG_INFO("QR direct solver starts");

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  LOG_INFO("QR ends");

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "QR::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());
  assert(this->op_->get_nrow() > 0);

  this->qr_.CloneFrom(*this->op_);
  this->qr_.QRDecompose();

  LOG_DEBUG(this, "QR::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "QR::Clear()",
            this->build_);

  if (this->build_ == true) {

    this->qr_.Clear();
    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "QR::MoveToHostLocalData_()",
            this->build_);

  if (this->build_ == true)
    this->qr_.MoveToHost();

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "QR::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true)
    this->qr_.MoveToAccelerator();

}

template <class OperatorType, class VectorType, typename ValueType>
void QR<OperatorType, VectorType, ValueType>::Solve_(const VectorType &rhs, VectorType *x) {

  LOG_DEBUG(this, "QR::Solve_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(&this->qr_ != NULL);
  assert(this->build_ == true);

  this->qr_.QRSolve(rhs, x);

  LOG_DEBUG(this, "QR::Solve_()",
            " #*# end");

}


template class QR< LocalMatrix<double>, LocalVector<double>, double >;
template class QR< LocalMatrix<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class QR< LocalMatrix<std::complex<double> >,  LocalVector<std::complex<double> >, std::complex<double> >;
template class QR< LocalMatrix<std::complex<float> >,  LocalVector<std::complex<float> >, std::complex<float> >;
#endif

}

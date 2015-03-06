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
#include "cg.hpp"
#include "../iter_ctrl.hpp"

#include "../../base/local_matrix.hpp"
#include "../../base/local_stencil.hpp"
#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"
#include "../../utils/math_functions.hpp"

#include <math.h>
#include <complex>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
CG<OperatorType, VectorType, ValueType>::CG() {

  LOG_DEBUG(this, "CG::CG()",
            "default constructor");

}

template <class OperatorType, class VectorType, typename ValueType>
CG<OperatorType, VectorType, ValueType>::~CG() {

  LOG_DEBUG(this, "CG::~CG()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::Print(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("CG solver");

  } else {

    LOG_INFO("PCG solver, with preconditioner:");
    this->precond_->Print();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  if (this->precond_ == NULL) {

    LOG_INFO("CG (non-precond) linear solver starts");

  } else {

    LOG_INFO("PCG solver starts, with preconditioner:");
    this->precond_->Print();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  if (this->precond_ == NULL) {

    LOG_INFO("CG (non-precond) ends");

  } else {

    LOG_INFO("PCG ends");

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "CG::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
    this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());
  assert(this->op_->get_nrow() > 0);

  if (this->precond_ != NULL) {

    this->precond_->SetOperator(*this->op_);

    this->precond_->Build();

    this->z_.CloneBackend(*this->op_);
    this->z_.Allocate("z", this->op_->get_nrow());

  }

  this->r_.CloneBackend(*this->op_);
  this->r_.Allocate("r", this->op_->get_nrow());

  this->p_.CloneBackend(*this->op_);
  this->p_.Allocate("p", this->op_->get_nrow());

  this->q_.CloneBackend(*this->op_);
  this->q_.Allocate("q", this->op_->get_nrow());

  LOG_DEBUG(this, "CG::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "CG::Clear()",
            this->build_);

  if (this->build_ == true) {

    if (this->precond_ != NULL) {
      this->precond_->Clear();
      this->precond_   = NULL;
    }

    this->r_.Clear();
    this->z_.Clear();
    this->p_.Clear();
    this->q_.Clear();

    this->iter_ctrl_.Clear();

    this->build_ = false;

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "CG::MoveToHostLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->r_.MoveToHost();
    this->p_.MoveToHost();
    this->q_.MoveToHost();

    if (this->precond_ != NULL) {
      this->z_.MoveToHost();
      this->precond_->MoveToHost();
    }

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "CG::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->r_.MoveToAccelerator();
    this->p_.MoveToAccelerator();
    this->q_.MoveToAccelerator();

    if (this->precond_ != NULL) {
      this->z_.MoveToAccelerator();
      this->precond_->MoveToAccelerator();
    }

  }

}

// TODO
// re-orthogonalization and
// residual - re-computed % iter
template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::SolveNonPrecond_(const VectorType &rhs,
                                                               VectorType *x) {

  LOG_DEBUG(this, "CG::SolveNonPrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_  == NULL);
  assert(this->build_ == true);

  const OperatorType *op = this->op_;

  VectorType *r = &this->r_;
  VectorType *p = &this->p_;
  VectorType *q = &this->q_;

  ValueType alpha, beta;
  ValueType rho, rho_old;

  // initial residual = b - Ax
  op->Apply(*x, r);
  r->ScaleAdd(ValueType(-1.0), rhs);

  // p = r
  p->CopyFrom(*r);

  // rho = (r,r)
  rho = r->DotNonConj(*r);

  // use for |b-Ax0|
  ValueType res_norm = this->Norm(*r);

  if (this->iter_ctrl_.InitResidual(paralution_abs(res_norm)) == false) {

    LOG_DEBUG(this, "CG::SolveNonPrecond_()",
              " #*# end");
    return;
  }

  // use for |b|
  //  this->iter_ctrl_.InitResidual(rhs.Norm());

  // q=Ap
  op->Apply(*p, q);

  // alpha = rho / (p,q)
  alpha = rho / p->DotNonConj(*q);

  // x = x + alpha*p
  x->AddScale(*p, alpha);

  // r = r - alpha*q
  r->AddScale(*q, ValueType(-1.0)*alpha);

  rho_old = rho;

  // rho = (r,r)
  rho = r->DotNonConj(*r);

  res_norm = this->Norm(*r);

  while (!this->iter_ctrl_.CheckResidual(paralution_abs(res_norm), this->index_)) {

    beta = rho / rho_old;

    // p = beta*p + r
    p->ScaleAdd(beta, *r);

    // q=Ap
    op->Apply(*p, q);

    // alpha = rho / (p,q)
    alpha = rho / p->DotNonConj(*q);

    // x = x + alpha*p
    x->AddScale(*p, alpha);

    // r = r - alpha*q
    r->AddScale(*q, ValueType(-1.0)*alpha);

    rho_old = rho;

    // rho = (r,r)
    rho = r->DotNonConj(*r);

    res_norm = this->Norm(*r);

  }

  LOG_DEBUG(this, "CG::SolveNonPrecond_()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void CG<OperatorType, VectorType, ValueType>::SolvePrecond_(const VectorType &rhs,
                                                            VectorType *x) {

  LOG_DEBUG(this, "CG::SolvePrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_ != NULL);
  assert(this->build_ == true);

  const OperatorType *op = this->op_;

  VectorType *r = &this->r_;
  VectorType *z = &this->z_;
  VectorType *p = &this->p_;
  VectorType *q = &this->q_;

  ValueType alpha, beta;
  ValueType rho, rho_old;

  // initial residual = b - Ax
  op->Apply(*x, r);
  r->ScaleAdd(ValueType(-1.0), rhs);

  // Solve Mz=r
  this->precond_->SolveZeroSol(*r, z);

  // p = z 
  p->CopyFrom(*z);

  // rho = (r,z)
  rho = r->DotNonConj(*z);

  // initial residual norm

  // use for |b-Ax0|
  ValueType res_norm = this->Norm(*r);

  if (this->iter_ctrl_.InitResidual(paralution_abs(res_norm)) == false) {

    LOG_DEBUG(this, "CG::SolvePrecond_()",
              " #*# end");
    return;

  }

  // use for |b|
  //  this->iter_ctrl_.InitResidual(rhs.Norm());

  // q=Ap
  op->Apply(*p, q);

  // alpha = rho / (p,q)
  alpha = rho / p->DotNonConj(*q);

  // x = x + alpha*p
  x->AddScale(*p, alpha);

  // r = r - alpha*q
  r->AddScale(*q, ValueType(-1.0)*alpha);

  res_norm = this->Norm(*r);

  while (!this->iter_ctrl_.CheckResidual(paralution_abs(res_norm), this->index_)) {

    rho_old = rho;

    // Solve Mz=r
    this->precond_->SolveZeroSol(*r, z);

    // rho = (r,z)
    rho = r->DotNonConj(*z);

    beta = rho / rho_old;

    // p = beta*p + z
    p->ScaleAdd(beta, *z);

    // q=Ap
    op->Apply(*p, q);

    // alpha = rho / (p,q)
    alpha = rho / p->DotNonConj(*q);

    // x = x + alpha*p
    x->AddScale(*p, alpha);

    // r = r - alpha*q
    r->AddScale(*q, ValueType(-1.0)*alpha);

    res_norm = this->Norm(*r);

  }

  LOG_DEBUG(this, "CG::SolvePrecond_()",
            " #*# end");

}


template class CG< LocalMatrix<double>, LocalVector<double>, double >;
template class CG< LocalMatrix<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class CG< LocalMatrix<std::complex<double> >,  LocalVector<std::complex<double> >, std::complex<double> >;
template class CG< LocalMatrix<std::complex<float> >,  LocalVector<std::complex<float> >, std::complex<float> >;
#endif

template class CG< LocalStencil<double>, LocalVector<double>, double >;
template class CG< LocalStencil<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class CG< LocalStencil<std::complex<double> >,  LocalVector<std::complex<double> >, std::complex<double> >;
template class CG< LocalStencil<std::complex<float> >,  LocalVector<std::complex<float> >, std::complex<float> >;
#endif

}

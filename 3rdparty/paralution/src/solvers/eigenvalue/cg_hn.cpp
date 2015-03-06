// *************************************************************************
//
//    This code is developed and maintained by NTU. 
//    The SIRA eigenvalue solver (files src/solvers/eigenvalue/amppe_sira.hpp 
//    src/solvers/eigenvalue/amppe_sira.cpp src/solvers/deflation/cg_hn.hpp
//    src/solvers/deflation/cg_hn.cpp) are released under GNU LESSER GENERAL 
//    PUBLIC LICENSE (LGPL v3)
//
//    Copyright (C) 2014 Weichung Wang (NTU)
//    National Taiwan University, National Taiwan
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as 
//    published by the Free Software Foundation, either version 3 of the 
//    License, or (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public 
//    License along with this program.  
//    If not, see <http://www.gnu.org/licenses/>.
//
// *************************************************************************

#include "../../utils/def.hpp"
#include "cg_hn.hpp"
#include "../iter_ctrl.hpp"
#include "../../base/local_matrix.hpp"
#include "../../base/local_stencil.hpp"
#include "../../base/local_vector.hpp"
#include "../../utils/log.hpp"


namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
CG_HN<OperatorType, VectorType, ValueType>::CG_HN() {

  LOG_DEBUG(this, "CG_HN::CG_HN()",
            "default constructor");

  this->CaseB_ = 0; 
  this->CaseC_ = 0; 
  this->UsingHN_ = 0; 
  this->IsFirst_ = 1; 
  this->GetBetaTime_ = 0;
  this->s_set_ = 0; 
  this->alpha_set_ = 0;
  this->beta_set_ = 0; 
  this->r_eig_ = 0;
  this->g_k_[0] = 0;
  this->g_k_[1] = 0;
  this->Ita_ = 0;
  this->Tol_out_ = 0;
  this->norm_r_out_ = 0;
  this->ResAtFirst_ = 0;
  this->Tau_1_ = 0;
  this->Tau_2_ = 0;
  this->Tau_3_ = 0;

}

template <class OperatorType, class VectorType, typename ValueType>
CG_HN<OperatorType, VectorType, ValueType>::~CG_HN() {

  LOG_DEBUG(this, "CG_HN::~CG_HN()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::Print(void) const {

  if (this->precond_ == NULL) { 

    LOG_INFO("CG_HN solver");

  } else {

    LOG_INFO("PCG_HN solver, with preconditioner:");
    this->precond_->Print();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  if (this->precond_ == NULL) {

    LOG_INFO("CG_HN (non-precond) linear solver starts");

  } else {

    LOG_INFO("PCG_HN solver starts, with preconditioner:");
    this->precond_->Print();

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

  if (this->precond_ == NULL) {

    LOG_INFO("CG_HN (non-precond) ends");

  } else {

    LOG_INFO("PCG_HN ends");

  }

}

template <class OperatorType, class VectorType, typename ValueType>
bool CG_HN<OperatorType, VectorType, ValueType>::Check_HN(ValueType res, VectorType *x, VectorType &u_k){

  ValueType Tau_1      = this->Tau_1_; 
  ValueType Tau_2      = this->Tau_2_; 
  ValueType norm_r_out = this->norm_r_out_;
  bool      IsFirst    = this->IsFirst_;
  ValueType *g_k       = this->g_k_;

  if (this->iter_ctrl_.CheckResidual(res)) return true;
  int iter_ = this->iter_ctrl_.GetIterationCount();
  int iter_max_ = this->iter_ctrl_.GetMaximumIterations();

  if (iter_ > iter_max_) return true;
  if (res < (Tau_1 * norm_r_out)) {

    if (IsFirst == 1){
      g_k[0] = res;
      //this->ResAtFirst_ = res;
      this->GetBeta(x, u_k); 
      this->s_set_ = x->Norm();
    } else if (res < (Tau_2 * norm_r_out)) {
      //g_k[1] = res;
      if ((this -> GetBetaTime_) < 2 && res < 0.5*(this->ResAtFirst_)){
        this->GetBeta(x, u_k);
        this->s_set_ = x->Norm();
      }
    }
    g_k[1] = res;

    this->IsFirst_ = 0;
    this->Get_r_eig();
    this->CaseB_ = this->JudgeCaseB();
    this->CaseC_ = this->JudgeCaseC();
    if (IsFirst != 1) g_k[0] = g_k[1]; 
    
    if (this->r_eig_ < this->Tol_out_) return true;
    if (this->CaseB_)                 return true;
    //if ( this->CaseC_ )                return true;
    return false;

  } else {
    return false; 
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::GetBeta(VectorType *x, const VectorType &u_k) {

  ValueType temp1, temp2, temp3;
  VectorType *u_A = &this->u_A_;
  VectorType *x_A = &this->x_A_;
  ValueType  Ita  =  this->Ita_;
  ValueType temp;
  const OperatorType *op = this->op_;

  op->Apply(*x, x_A);
  op->Apply(u_k,u_A);

  temp1 = u_k.Dot(*u_A); // theta = u'Au
  temp2 = u_k.Dot(*x_A); // u'Ax
  temp3 = u_k.Dot(*x);   // u'x
  temp  = temp1 - Ita + temp2 - temp3*Ita;
  this->beta_set_ = (temp > 0) ? temp : -temp;
  this->GetBetaTime_++;

  return;

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::Get_r_eig(void) {

  ValueType beta_set = this->beta_set_;
  ValueType s_set    = this->s_set_;
  ValueType *g_k     = this->g_k_;
  ValueType temp_g_k = (this->IsFirst_ == 1) ? g_k[0] : g_k[1];
  ValueType temp     = (beta_set * s_set) / (ValueType(1.0) + s_set * s_set);

  temp *= temp;
  temp = (temp_g_k * temp_g_k) / (ValueType(1.0) + s_set * s_set) + temp; 
  this->r_eig_ = pow(temp, ValueType(0.5));

  return;

}

template <class OperatorType, class VectorType, typename ValueType>
bool CG_HN<OperatorType, VectorType, ValueType>::JudgeCaseB(void) {
     
  ValueType beta_set  = this->beta_set_;
  ValueType alpha_set = this->alpha_set_;
  ValueType s_set     = this->s_set_;
  ValueType Tau_3     = this->Tau_3_;
  ValueType Tol_out   = this->Tol_out_;
  ValueType g         = (this-> IsFirst_ == 1) ? this->g_k_[0] : this->g_k_[1];
  ValueType temp      = pow(1+ s_set * s_set, ValueType(0.5));
   
  if ( ((beta_set * s_set) / ( alpha_set * (1 + s_set * s_set))) > (Tol_out /2) &&
       ( g < (Tau_3 * beta_set * s_set / temp))) {
    return true;
  } else {
    return false;
  }

}

template <class OperatorType, class VectorType, typename ValueType>
bool CG_HN<OperatorType, VectorType, ValueType>::JudgeCaseC(void){

  ValueType beta_set  = this->beta_set_;
  ValueType alpha_set = this->alpha_set_;
  ValueType s_set     = this->s_set_;
  ValueType Tol_out   = this->Tol_out_;
  if (this->IsFirst_ == 1) { 
    return false; 
  } else if ( ((beta_set * s_set ) / (alpha_set * (1+s_set * s_set))) > Tol_out/2  &&
            (this->g_k_[0] < this->g_k_[1])) {
    return true;
  } else {
    return false;
  }

}


template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::SolveNonPrecond(const VectorType &rhs,
                                                              VectorType *x, VectorType &u_k) {

  LOG_DEBUG(this, "CG_HN::SolveNonPrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_ != NULL);
  assert(this->precond_ == NULL);
  assert(this->build_ == true);

  const OperatorType *op = this->op_;
  this->IsFirst_     = 1;
  this->GetBetaTime_ = 0; 
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
  rho = r->Dot(*r);

  // use for |b-Ax0|
  ValueType res_norm;
  res_norm = this->Norm(*r);
  this->iter_ctrl_.InitResidual(res_norm);
  //this->ResAtFirst_      = res_norm;

  // use for |b|
  //  this->iter_ctrl_.InitResidual(rhs.Norm());
  
  // q=Ap
  op->Apply(*p, q);

  // alpha = rho / (p,q)
  alpha = rho / p->Dot(*q);
  
  // x = x + alpha*p
  x->AddScale(*p, alpha);

  // r = r - alpha*q
  r->AddScale(*q, ValueType(-1.0)*alpha);

  rho_old = rho;

  // rho = (r,r)
  rho = r->Dot(*r);

  ValueType res = this->Norm(*r);
  
  while (!this->Check_HN(res, x, u_k)) {
    
    beta = rho / rho_old;

    // p = beta*p + r 
    p->ScaleAdd(beta, *r);

    // q=Ap
    op->Apply(*p, q);

    // alpha = rho / (p,q)
    alpha = rho / p->Dot(*q);

    // x = x + alpha*p
    x->AddScale(*p, alpha);

    // r = r - alpha*q
    r->AddScale(*q, ValueType(-1.0)*alpha);

    rho_old = rho;
    
    // rho = (r,r)
    rho = r->Dot(*r);

    res = this->Norm(*r);
    //    this->residual_history_.push_back(res);
  }

  LOG_DEBUG(this, "CG_HN::SolveNonPrecond_()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::SolvePrecond(const VectorType &rhs,
                                                            VectorType *x, VectorType &u_k) {
  //double tmp;
  LOG_DEBUG(this, "CG_HN::SolvePrecond_()",
            " #*# begin");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_  != NULL);
  assert(this->precond_ != NULL);
  assert(this->build_ == true);
  this->IsFirst_     = 1;
  this->GetBetaTime_ = 0;
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
  rho = r->Dot(*z);

  // initial residual norm

  // use for |b-Ax0|
  ValueType res_norm;
  res_norm = this->Norm(*r) ;
  this->iter_ctrl_.InitResidual(res_norm);
  //this->ResAtFirst_      = res_norm;

  // use for |b|
  //  this->iter_ctrl_.InitResidual(rhs.Norm());

  // q=Ap
  op->Apply(*p, q);

  // alpha = rho / (p,q)
  alpha = rho / p->Dot(*q);
  
  // x = x + alpha*p
  x->AddScale(*p, alpha);

  // r = r - alpha*q
  r->AddScale(*q, ValueType(-1.0)*alpha);
  ValueType res = this->Norm(*r);
  while (!this->Check_HN(res, x, u_k)) {

    rho_old = rho;

    // Solve Mz=r
    this->precond_->SolveZeroSol(*r, z);

    // rho = (r,z)
    rho = r->Dot(*z);

    beta = rho / rho_old;

    // p = beta*p + z
    p->ScaleAdd(beta, *z);

    // q=Ap
    op->Apply(*p, q);

    // alpha = rho / (p,q)
    alpha = rho / p->Dot(*q);

    // x = x + alpha*p
    x->AddScale(*p, alpha);

    // r = r - alpha*q
    r->AddScale(*q, ValueType(-1.0)*alpha);
    res = this->Norm(*r);
    //    this->residual_history_.push_back(res);

  }

  LOG_DEBUG(this, "CG_HN::SolvePrecond_()",
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::Build(void) {

  LOG_DEBUG(this, "CG_HN::Build()",
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

  this->u_A_.CloneBackend(*this->op_);
  this->u_A_.Allocate("u_A", this->op_->get_nrow());

  this->x_A_.CloneBackend(*this->op_);
  this->x_A_.Allocate("x_A", this->op_->get_nrow());


  this->SetTau_1(0.316227766f);
  this->SetTau_2(0.1f);
  this->SetTau_3(1);
  this->SetAlpha(1);
  this->SetTol_out(1e-10f);
  this->SetIta(0);

  LOG_DEBUG(this, "CG_HN::Build()",
            this->build_ <<
            " #*# end");

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::Clear(void) {

  LOG_DEBUG(this, "CG_HN::Clear()",
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

    this->u_A_.Clear();
    this->x_A_.Clear();    

    this->iter_ctrl_.Clear();
    
    this->build_ = false;
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::MoveToHostLocalData_(void) {

  LOG_DEBUG(this, "CG_HN::MoveToHostLocalData_()",
            this->build_);  

  if (this->build_ == true) {

    this->r_.MoveToHost();
    this->p_.MoveToHost();
    this->q_.MoveToHost();
    
    this->u_A_.MoveToHost();
    this->x_A_.MoveToHost();

    if (this->precond_ != NULL) {
      this->z_.MoveToHost();
      this->precond_->MoveToHost();
    }
    
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::MoveToAcceleratorLocalData_(void) {

  LOG_DEBUG(this, "CG_HN::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {

    this->r_.MoveToAccelerator();
    this->p_.MoveToAccelerator();
    this->q_.MoveToAccelerator();

    this->u_A_.MoveToAccelerator();
    this->x_A_.MoveToAccelerator();

    if (this->precond_ != NULL) {
      this->z_.MoveToAccelerator();
      this->precond_->MoveToAccelerator();
    }
    
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void CG_HN<OperatorType, VectorType, ValueType>::Solve(const VectorType &rhs,
                                                    VectorType *x, VectorType &u_k) {

  LOG_DEBUG(this, "SIRA::CG_HN::Solve()",
            "");

  assert(x != NULL);
  assert(x != &rhs);
  assert(this->op_ != NULL);
  assert(this->build_ == true);

  if (this->precond_ == NULL) {

    this->SolveNonPrecond(rhs, x, u_k);

  } else {

    this->SolvePrecond(rhs, x, u_k);

  }

}

template class CG_HN< paralution::LocalMatrix<double>, paralution::LocalVector<double>, double >;
template class CG_HN< paralution::LocalMatrix<float>,  paralution::LocalVector<float>, float >;


}

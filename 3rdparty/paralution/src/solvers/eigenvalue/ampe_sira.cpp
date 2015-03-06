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

#include "ampe_sira.hpp"
#include "../../utils/def.hpp"
#include "../../utils/log.hpp"
#include "../../utils/allocate_free.hpp"
#include "../../utils/math_functions.hpp"
#include "../../base/local_stencil.hpp"
#include "../../base/local_vector.hpp"

#include <assert.h>
#include <math.h>

#ifdef SUPPORT_MKL
  #include <mkl.h>
  #include <mkl_spblas.h>
#endif

namespace paralution {

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
     OperatorTypeL, VectorTypeL, ValueTypeL>::SIRA(){

  LOG_DEBUG(this, "SIRA::SIRA()", "default constructor");

  this->verb_                  = 1;
  this->PrecondFlag_           = 1;
  this->solved_num_            = 0;
  this->target_                = 0;
  this->DoubleFlag_            = 0;
  this->single_count_          = 0;
  this->double_count_          = 0;
  this->r_out_norm_            = 0;
  this->StoppingCriterionFlag_ = -1;
  this->op_l_                  = NULL;
  this->inner_cg_h_            = NULL;
  this->inner_cg_hn_h_         = NULL;
  this->inner_cg_hn_l_         = NULL;
  this->p_inner_l_             = NULL;
  this->p_inner_h_             = NULL;
  this->p_fsai_h_              = NULL;
  this->p_fsai_l_              = NULL;

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<float> , LocalVector<float>,  float>::Clear(void){
		
  LOG_DEBUG(this, "SIRA::Clear()",
            this->build_);

  if (this->build_ == true) {

    this->V_              .Clear();
    this->AV_             .Clear();
    this->VtAV_           .Clear();
    this->EigVecs_        .Clear();
    this->s_out_          .Clear();
    this->Au_             .Clear();
    this->r_out_          .Clear();
    this->r_out_single_   .Clear();
    this->t_out_          .Clear();
    this->t_out_single_   .Clear();
    this->u_k_            .Clear();
    this->u_k_single_     .Clear();
    this->iter_ctrl_      .Clear();
    this->tmpUpdateVector_.Clear();
    this->inner_cg_hn_h_ ->Clear();
    this->inner_cg_hn_l_ ->Clear();

    delete this->inner_cg_hn_h_; 
    delete this->inner_cg_hn_l_; 
    delete this->op_l_;

    if (this->p_fsai_l_ != NULL) {
      delete this->p_fsai_l_;
      delete this->p_fsai_h_;
    }

    this->build_ = false;

  }

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<double>, LocalVector<double>, double>::Clear(void){
		
  LOG_DEBUG(this, "SIRA::Clear()",
            this->build_);

  if (this->build_ == true) {

    this->V_              .Clear();
    this->AV_             .Clear();
    this->VtAV_           .Clear();
    this->EigVecs_        .Clear();
    this->s_out_          .Clear();
    this->Au_             .Clear();
    this->r_out_          .Clear();
    this->t_out_          .Clear();
    this->u_k_            .Clear();
    this->iter_ctrl_      .Clear();
    this->tmpUpdateVector_.Clear();

    if (this->StoppingCriterionFlag_ == 0) {

      this->inner_cg_h_->Clear();
      delete this->inner_cg_h_; 

    } else {

      this->inner_cg_hn_h_->Clear();
      delete this->inner_cg_hn_h_; 

    }

    if (this->p_fsai_h_ != NULL) {
      delete this->p_fsai_h_;
    }

    this->build_ = false;

  }

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
     OperatorTypeL, VectorTypeL, ValueTypeL>::~SIRA(){

  LOG_DEBUG(this, "SIRA::~SIRA()", "destructor");

  this->Clear();

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Print(void ) const{
                
  LOG_INFO("SIRA solver");

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::PrintStart_(void ) const{

  LOG_INFO("SIRA eigensolver starts, with preconditioner:");
  
  if (this->PrecondFlag_ != 0) this->p_inner_h_->Print();

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::PrintEnd_(void ) const{

  LOG_INFO("SIRA eigensolver ends");

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Build(void){

  LOG_DEBUG(this, "SIRA::Build()",
            this->build_ <<
            " #*# begin");

  if (this->build_ == true)
      this->Clear();

  assert(this->build_ == false);
  this->build_ = true;

  assert(this->op_ != NULL);
  assert(this->op_->get_nrow() == this->op_->get_ncol());
  assert(this->op_->get_nrow() > 0);
  
  this->op_h_ = (LocalMatrix<ValueTypeH>*) this->op_;

  this->dim_ = this->op_->get_nrow();
  this->nnz_ = this->op_->get_nnz();

  // Set the maximum subdimension
  this->SubDim_ = 0;
  this->MaxSubDim_ = 50;
  this->MaxInnerIter_ = 100000;

  // Initialize iteration control
  this->iter_ctrl_.Init(1e-7, 1e-12, 1e+8, 300*this->target_num_);

  // Allocating matrices and vectors 
  this->V_              .CloneBackend(*this->op_);
  this->AV_             .CloneBackend(*this->op_);
  this->VtAV_           .CloneBackend(*this->op_);
  this->EigVecs_        .CloneBackend(*this->op_);
  this->Au_             .CloneBackend(*this->op_);  
  this->r_out_          .CloneBackend(*this->op_);  
  this->t_out_          .CloneBackend(*this->op_);
  this->u_k_            .CloneBackend(*this->op_);  
  this->s_out_          .CloneBackend(*this->op_);
  this->tmpUpdateVector_.CloneBackend(*this->op_);

  this->V_              .AllocateDENSE("mtx_V", this->op_->get_nrow(), this->MaxSubDim_);
  this->AV_             .AllocateDENSE("mtx_AV_", this->MaxSubDim_, this->op_->get_nrow());
  this->VtAV_           .AllocateDENSE("mtx_VtAV_", this->MaxSubDim_, this->MaxSubDim_); 
  this->EigVecs_        .AllocateDENSE("Eigvecs", this->dim_, this->target_num_);
  this->s_out_          .Allocate("vec_s", this->MaxSubDim_);
  this->Au_             .Allocate("vec_Au_", this->op_->get_nrow());
  this->r_out_          .Allocate("vec_r_out_", this->op_->get_nrow());
  this->t_out_          .Allocate("vec_t", this->op_->get_nrow());
  this->u_k_            .Allocate("vec_u_k_", this->op_->get_nrow());
  this->tmpUpdateVector_.Allocate("UpdateVector", this->op_->get_nrow());
  // End of allocating

  this->Build_();

  this->op_h_->info();
  if (this->op_l_ != NULL)
    this->op_l_->info();

  LOG_DEBUG(this, "SIRA::Build()",
            this->build_ <<
            " #*# end");

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<float> , LocalVector<float> , float>::Build_(void){
              
  if (this->StoppingCriterionFlag_ == -1)
    this->StoppingCriterionFlag_ = 1;

  this->op_l_ = new LocalMatrix<float>;
              
  // Copy the matrix from double to single
  // CSR H
  int *row_offset = NULL;
  int *col = NULL;
  double *val_h = NULL;

  // CSR L
  float *val_l = NULL;

  allocate_host(this->op_h_->get_nrow()+1, &row_offset);
  allocate_host(this->op_h_->get_nnz(),    &col);
  allocate_host(this->op_h_->get_nnz(),    &val_l);
  allocate_host(this->op_h_->get_nnz(),    &val_h);

  this->op_l_->CloneBackend(*this->op_);
  this->op_h_->MoveToHost();
  this->op_h_->CopyToCSR(row_offset, col, val_h);
  this->op_h_->CloneBackend(*this->op_l_);
  this->op_l_->MoveToHost();

  for (int i=0; i < this->op_h_->get_nnz(); ++i)
    val_l[i] = float(val_h[i]);

  this->op_l_->SetDataPtrCSR(&row_offset, &col, &val_l,
                            "Low precision Matrix", 
                            this->op_h_->get_nnz(),
                            this->op_h_->get_nrow(),
                            this->op_h_->get_ncol());
  free_host(&val_h);
  this->op_l_->CloneBackend(*this->op_);
  // End of copying matrix
  
  if ( (this->p_inner_h_ == NULL) && (this->PrecondFlag_ == 1) ) {
    this->p_fsai_l_  = new FSAI<LocalMatrix<float >, LocalVector<float>,  float >;
    this->p_fsai_h_  = new FSAI<LocalMatrix<double>, LocalVector<double>, double >;
    this->p_fsai_l_->Set(1);
    this->p_fsai_h_->Set(1);
    this->p_inner_l_ = this->p_fsai_l_;
    this->p_inner_h_ = this->p_fsai_h_;
  }
              
  // Set and build the inner linear solver

  this->inner_cg_hn_h_ = new CG_HN < LocalMatrix<double>, LocalVector<double>, double >;
  this->inner_cg_hn_l_ = new CG_HN < LocalMatrix<float>,  LocalVector<float>,  float >;

  if ( this->PrecondFlag_ == 1) {
    this->inner_cg_hn_l_->SetPreconditioner(*this->p_inner_l_);
    this->inner_cg_hn_h_->SetPreconditioner(*this->p_inner_h_);
  }

  this->inner_cg_hn_l_->SetOperator(*this->op_l_);
  this->inner_cg_hn_h_->SetOperator(*this->op_h_);
  this->inner_cg_hn_l_->Build();
  this->inner_cg_hn_h_->Build();
  this->inner_cg_hn_l_->Init(1e-10, 1e-10, 10e8, this->MaxInnerIter_);
  this->inner_cg_hn_h_->Init(1e-10, 1e-10, 10e8, this->MaxInnerIter_);
  this->inner_cg_hn_h_->Verbose(0);
  this->inner_cg_hn_l_->Verbose(0);

  this->r_out_single_.CloneBackend(*this->op_);
  this->t_out_single_.CloneBackend(*this->op_);
  this->u_k_single_  .CloneBackend(*this->op_);
  this->t_out_single_.Allocate("t_out_single_", this->op_->get_nrow());

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<double>, LocalVector<double>, double>::Build_(void){
              
  if (this->StoppingCriterionFlag_ == -1)
    this->StoppingCriterionFlag_ = 0;

  // Set and build the inner linear solver
  if ( (this->p_inner_h_ == NULL) && (this->PrecondFlag_ == 1) ) {
    // FSAI level 1
    this->p_fsai_h_ = new FSAI<LocalMatrix<double>, LocalVector<double>, double >;
    this->p_fsai_h_->Set(1);
    this->p_inner_h_ = this->p_fsai_h_;
  }

  if (this->StoppingCriterionFlag_ == 0 ) {
    this->inner_cg_h_    = new CG    < LocalMatrix<double>, LocalVector<double>, double >;
  } else {
    this->inner_cg_hn_h_ = new CG_HN < LocalMatrix<double>, LocalVector<double>, double >;
  }

  switch ( this->StoppingCriterionFlag_ ){
  case 0:
    if (this->PrecondFlag_ == 1) this->inner_cg_h_->SetPreconditioner(*this->p_inner_h_);
    this->inner_cg_h_->SetOperator(*this->op_h_);
    this->inner_cg_h_->Build();
    this->inner_cg_h_->Init(1e-10, 1e-10, 10e8, this->MaxInnerIter_);
    this->inner_cg_h_->Verbose(0);
    break;

  case 1:
    if (this->PrecondFlag_ == 1) this->inner_cg_hn_h_->SetPreconditioner(*this->p_inner_h_);
    this->inner_cg_hn_h_->SetOperator(*this->op_h_);
    this->inner_cg_hn_h_->Build();
    this->inner_cg_hn_h_->Init(1e-10, 1e-10, 10e8, this->MaxInnerIter_);
    this->inner_cg_hn_h_->Verbose(0);
    break;

  default:
    LOG_VERBOSE_INFO(3, "Invalid argument for SetInnerStoppingCriterion()");
    FATAL_ERROR(__FILE__, __LINE__);
    break;
  }
  // End of building the inner linear solver

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::SetNumberOfEigenvalues(const int num){
  
  LOG_DEBUG(this, "SIRA::SetNumberOfEigenvalues()",
            this->build_);
  
  this->target_num_ = num;

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<float> , LocalVector<float>,  float>::SetInnerStoppingCriterion(const int n){
  
  LOG_DEBUG(this, "SIRA::SetInnerStoppingCriterion()",
            this->build_);
  
  assert(n != 0);
  this->StoppingCriterionFlag_ = n;

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<double>, LocalVector<double>, double>::SetInnerStoppingCriterion(const int n){
  
  LOG_DEBUG(this, "SIRA::SetInnerStoppingCriterion()",
            this->build_);

  this->StoppingCriterionFlag_ = n;

}


template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Init(const ValueTypeH abs_tol, 
          const ValueTypeH rel_tol, const ValueTypeH div_tol, const int max_iter) {

  FATAL_ERROR(__FILE__, __LINE__);

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::SetInnerPreconditioner( Preconditioner<LocalMatrix<ValueTypeH>, LocalVector<ValueTypeH>, ValueTypeH > &p1, Preconditioner<LocalMatrix<ValueTypeL>, LocalVector<ValueTypeL>, ValueTypeL > &p2 ){

  this->p_inner_h_ = &p1;
  this->p_inner_l_ = &p2;

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::SetInnerPreconditioner( Preconditioner<LocalMatrix<ValueTypeH>, LocalVector<ValueTypeH>, ValueTypeH > &p){

  this->p_inner_h_ = &p;

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::SetInnerPreconditioner(int a){

  this->PrecondFlag_ = a;

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Init(ValueTypeH abs_tol, int MaxOuterIter, int MaxInnerIter){

  if (abs_tol == 0) 
    abs_tol = 1e-7;

  if (MaxOuterIter == 0) 
    MaxOuterIter = 300*this->target_num_;

  if ( MaxInnerIter > 0) {
    if ( MaxInnerIter <=1 ) {
      this->MaxInnerIter_ = this->dim_ * MaxInnerIter;
    }
  }

  this->iter_ctrl_.Init(abs_tol, 1e-12, 1e+8, MaxOuterIter); 

  if (this->StoppingCriterionFlag_ == 0) {
    this->inner_cg_h_->Init(1e-10, 1e-10, 10e8, int(this->MaxInnerIter_));
  } else {
    this->inner_cg_hn_l_->Init(1e-10, 1e-10, 10e8, int(this->MaxInnerIter_));
    this->inner_cg_hn_h_->Init(1e-10, 1e-10, 10e8, int(this->MaxInnerIter_));
  }

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::MoveToHostLocalData_(void ) {

  LOG_DEBUG(this, "SIRA::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {
  
    this->V_           .MoveToHost();
    this->AV_          .MoveToHost();
    this->VtAV_        .MoveToHost();
    this->EigVecs_     .MoveToHost();
    this->s_out_       .MoveToHost();
    this->Au_          .MoveToHost();
    this->r_out_       .MoveToHost();
    this->r_out_single_.MoveToHost();
    this->t_out_       .MoveToHost();
    this->t_out_single_.MoveToHost();
    this->u_k_         .MoveToHost();
    this->u_k_single_  .MoveToHost();
    if ( this->op_l_ != NULL )
      this->op_l_     ->MoveToHost();
    if ( this->inner_cg_h_ != NULL )
      this->inner_cg_h_->MoveToHost();
    if ( this->inner_cg_hn_h_ != NULL )
      this->inner_cg_hn_h_->MoveToHost();
    if ( this->inner_cg_hn_l_ != NULL )
      this->inner_cg_hn_l_->MoveToHost();

  }

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::MoveToAcceleratorLocalData_(void ) {

  LOG_DEBUG(this, "SIRA::MoveToAcceleratorLocalData_()",
            this->build_);

  if (this->build_ == true) {
  
    this->V_           .MoveToAccelerator();
    this->AV_          .MoveToAccelerator();
    this->VtAV_        .MoveToAccelerator();
    this->EigVecs_     .MoveToAccelerator();
    this->s_out_       .MoveToAccelerator();
    this->Au_          .MoveToAccelerator();
    this->r_out_       .MoveToAccelerator();
    this->r_out_single_.MoveToAccelerator();
    this->t_out_       .MoveToAccelerator();
    this->t_out_single_.MoveToAccelerator();
    this->u_k_         .MoveToAccelerator();
    this->u_k_single_  .MoveToAccelerator();
    if ( this->op_l_ != NULL )
      this->op_l_     ->MoveToAccelerator();
    if ( this->inner_cg_h_ != NULL )
      this->inner_cg_h_->MoveToAccelerator();
    if ( this->inner_cg_hn_h_ != NULL )
      this->inner_cg_hn_h_->MoveToAccelerator();
    if ( this->inner_cg_hn_l_ != NULL )
      this->inner_cg_hn_l_->MoveToAccelerator();

  }
}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Solve(VectorTypeH *vec_ans){

  VectorTypeH vec_init; 
  vec_init.CloneBackend(*this->op_);
  vec_init.Allocate("vector_init", this->op_->get_nrow());
  vec_init.SetRandom(0, 1, 2);
  vec_init.Scale(1/vec_init.Norm());

  Solve(vec_init, vec_ans);

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Solve(const VectorTypeH & vec_start, VectorTypeH *vec_ans){

  LOG_VERBOSE_INFO(3, "AMPE-SIRA (start) Solve()");
  

  if (this->verb_ > 0) {
    this->iter_ctrl_.PrintInit();
    this->PrintStart_();
  }

  vec_ans->MoveToHost();
  vec_ans->Allocate("Eigenvalues", this->target_num_);

  this->t_out_.CopyFrom(vec_start);
  this->t_out_.Scale(1.0/this->t_out_.Norm());

  while ( solved_num_ < this->target_num_ ) {

    // Updates AV_
    this->SubDim_++;
    this->V_.ReplaceColumnVector(this->SubDim_ - 1, this->t_out_);
    this->op_->Apply(this->t_out_, &this->tmpUpdateVector_);
    this->AV_.ReplaceRowVector(this->SubDim_ - 1, this->tmpUpdateVector_);
    this->VtAV_.MatrixMult(this->AV_, this->V_);
    
    // Compute the eigenpair of V'AV
    this->Eigpair(theta_k_);
    // u_k = V_k * s
    this->V_.Apply(this->s_out_, &this->u_k_);
    // Au = A * u_k
    this->op_->Apply(this->u_k_, &this->Au_);
    // r_out = Au - theta_k * u_k
    this->r_out_.ScaleAdd2(0, this->Au_, 1, this->u_k_, -theta_k_); 

    if ( this->iter_ctrl_.GetIterationCount() == 0 ) 
      this->iter_ctrl_.InitResidual(this->r_out_.Norm());

    if ( this->iter_ctrl_.CheckResidual(this->r_out_.Norm()) ) {

      if (this->verb_ > 0) this->iter_ctrl_.PrintStatus();

      if ( this->iter_ctrl_.GetSolverStatus() == 4 ) {
        if (solved_num_ == 0) {
          vec_ans->Clear();
        } else {
          VectorTypeH tmpAnsVector;
          tmpAnsVector.Allocate("tmpAnsVector", solved_num_);
          tmpAnsVector.CopyFrom(*vec_ans, 0, 0, solved_num_); 
          vec_ans->Clear();
          vec_ans->CloneFrom(tmpAnsVector);
          tmpAnsVector.Clear();
        }
        break;
      }

      (*vec_ans)[this->solved_num_] = theta_k_ + target_;
      this->EigVecs_.ReplaceColumnVector(solved_num_, this->u_k_);
      ++solved_num_;
      //Orthogonalize V_ to EigVecs_
      this->Locking();
      this->SubDim_--;

    } else {

      // Solve A * t = -r_out_
      this->Inner(); 
      // Orthogonalize t to EigVectors 
      this->MvOrthogonalize(&this->EigVecs_, &this->t_out_, solved_num_);
      // Orthogonalize t to V
      this->MvOrthogonalize(&this->V_, &this->t_out_, SubDim_);
      //this->V_.ReplaceColumnVector(this->SubDim_ -1, this->t_out_);

      // Restart
      if ( this->SubDim_ == this->MaxSubDim_ ) { 
        LOG_VERBOSE_INFO(3, "AMPE-SIRA SubDim_ >= MaxSubDim_");
        this->Restart();
      }

    }
  }

  if (this->verb_ > 0) {
    this->PrintEnd_();
    LOG_INFO( "Double Inner Solver Counts: " << this->double_count_ );
    LOG_INFO( "Single Inner Solver Counts: " << this->single_count_ );
    LOG_INFO( "Number of eigenvalue solved: " << this->solved_num_ );
  }

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double,
          LocalMatrix<float>,  LocalVector<float> , float>::Inner(){
	
  LOG_VERBOSE_INFO(3, "AMPE-SIRA (start) Inner()");
	
  double Tau_1 = 0.316227766;
  double Single_Tol = 1e-5;


  if ( Tau_1 * this->r_out_.Norm() < Single_Tol ) {     
  //if ( (this->r_out_norm_ - this->r_out_.Norm()) < Single_Tol ) {     
    this->DoubleFlag_ = 1;
  } else {
    this->DoubleFlag_ = 0;
  }

  this->r_out_norm_ = this->r_out_.Norm();

  if ( this->DoubleFlag_ == 0) {

    //Double to Single
    this->r_out_single_.CopyFromDouble(this->r_out_);
    this->u_k_single_.CopyFromDouble(this->u_k_);

  }

  if ( this->DoubleFlag_ == 0 ) {

    //Single & HN
    single_count_++;
    this->inner_cg_hn_l_->SetNorm_r_out(float(this->r_out_.Norm()));
    this->inner_cg_hn_l_->Solve(this->r_out_single_, &this->t_out_single_, this->u_k_single_);
      
    this->u_k_single_.Scale(1/this->u_k_single_.Norm());
    this->t_out_single_.ScaleAddScale(1, this->u_k_single_, -(this->u_k_single_.Dot(this->t_out_single_)));
      
    //Single to Double
    this->t_out_.CopyFromFloat(t_out_single_);
      
  } else {

    //Double
    double_count_++;
    this->inner_cg_hn_h_->SetNorm_r_out(this->r_out_.Norm());
    this->inner_cg_hn_h_->RecordResidualHistory();
    this->inner_cg_hn_h_->Solve(this->r_out_, &this->t_out_, this->u_k_);
    this->u_k_.Scale(1/this->u_k_.Norm());
    this->t_out_.ScaleAddScale( 1, this->u_k_, -(this->u_k_.Dot(this->t_out_)) );

  }

  LOG_VERBOSE_INFO(3, "AMPE-SIRA (end) Inner()");

}

template <>
void SIRA<LocalMatrix<double>, LocalVector<double>, double,
          LocalMatrix<double>, LocalVector<double>, double>::Inner(){
	
  LOG_VERBOSE_INFO(3, "AMPE-SIRA (start) Inner()");
	
  double r_out_norm = this->r_out_.Norm();	

  double_count_++;
  switch( this->StoppingCriterionFlag_ ){
    case 0: //Full Double
      this->inner_cg_h_->Solve(this->r_out_, &this->t_out_);
      break;
    case 1: //HN
      this->inner_cg_hn_h_->SetNorm_r_out(r_out_norm);
      this->inner_cg_hn_h_->Solve(this->r_out_, &this->t_out_, this->u_k_);
      break;
    default:
      FATAL_ERROR(__FILE__, __LINE__);
      break;
  }
  //?
  this->u_k_.Scale( 1/this->u_k_.Norm() );
  this->t_out_.ScaleAddScale( 1, this->u_k_, -(this->u_k_.Dot(this->t_out_)));


  LOG_VERBOSE_INFO(3, "AMPE-SIRA (end) Inner()");

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Restart(){
  
  LOG_DEBUG(this, "SIRA::Restart()",
            "");

  int restart_dim = 5;
  VectorTypeH zeroVec, tmpVector; 
  zeroVec.CloneBackend(*this->op_);
  zeroVec.Allocate("zeroVec", this->dim_);
  zeroVec.Zeros();
  tmpVector.CloneBackend(*this->op_);
  tmpVector.Allocate("tmpVector", this->dim_);

  for (int i=0; i < restart_dim; i++) {
    this->V_.ExtractColumnVector(this->MaxSubDim_ - restart_dim - 1 + i, &tmpVector); 
    this->V_.ReplaceColumnVector(i, tmpVector);
    this->AV_.ExtractRowVector(this->MaxSubDim_ - restart_dim - 1 + i, &tmpVector); 
    this->AV_.ReplaceRowVector(i, tmpVector);
  }

  for (int i=restart_dim; i<this->MaxSubDim_; i++) {
    this->V_.ReplaceColumnVector(i, zeroVec);
    this->AV_.ReplaceRowVector(i, zeroVec);
  }

  tmpVector.Clear();
  zeroVec.Clear();
  this->SubDim_ = restart_dim;

  /*
  this->V_.Clear();
  this->V_.AllocateDENSE("mtx_V", this->op_->get_nrow(), this->MaxSubDim_);
  this->AV_.Clear();
  this->AV_.AllocateDENSE("mtx_AV_", this->MaxSubDim_, this->op_->get_nrow());
  VectorTypeH *randVector = new VectorTypeH; randVector->CloneBackend(*this->op_);
  randVector->Allocate("v_rand", this->op_->get_nrow());
  randVector->CopyFrom(this->t_out_);
  this->MvOrthogonalize(&this->EigVecs_, randVector, solved_num_);
  this->V_.ReplaceColumnVector(0, *randVector);
  this->t_out_.CloneFrom(*randVector);
  this->SubDim_ = 1;
  delete randVector;
  */
  
}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Locking(){

  LOG_DEBUG(this, "SIRA::Locking()",
            "");

  VectorTypeH tmpVector, tmpVector2, tmpEigvector;
  tmpVector.CloneBackend(*this->op_);
  tmpVector2.CloneBackend(*this->op_);
  tmpEigvector.CloneBackend(*this->op_);
  tmpVector.Allocate("tmpVector", this->op_->get_nrow());
  tmpVector2.Allocate("tmpVector2", this->op_->get_nrow());
  tmpEigvector.Allocate("tmpEigvector", this->op_->get_nrow());

  // Orthogonalize all column vector of V to EigVecs
  for ( int i=0; i< SubDim_; i++ ){
    this->V_.ExtractColumnVector(i, &tmpVector);
    for (int j=0; j< solved_num_; j++) {
      this->EigVecs_.ExtractColumnVector(j, &tmpEigvector); 
      tmpVector.ScaleAddScale(1, tmpEigvector, -tmpVector.Dot(tmpEigvector));
    }
    this->V_.ReplaceColumnVector(i, tmpVector);
  }

  // Again
  for ( int i=0; i< SubDim_; i++ ){
    this->V_.ExtractColumnVector(i, &tmpVector);
    for (int j=0; j< solved_num_; j++) {
      this->EigVecs_.ExtractColumnVector(j, &tmpEigvector); 
      tmpVector.ScaleAddScale(ValueTypeH(1), tmpEigvector, -tmpVector.Dot(tmpEigvector));
    }
    this->V_.ReplaceColumnVector(i, tmpVector);
  }

  int temp_Dim = 0;
  double TOL = 1e-6;
  double temp_norm;

  // Clear the column vector of V_ 
  for ( int i=0; i<SubDim_; i++ ){
    this->V_.ExtractColumnVector(i, &tmpVector);
    temp_norm = tmpVector.Norm();
    if (temp_norm > TOL) {
      tmpVector.Scale(1/temp_norm);
      this->V_.ReplaceColumnVector(i, tmpVector);
      if ( i != temp_Dim )
        this->V_.ReplaceColumnVector(temp_Dim, tmpVector);
      // Orthogonalize
      for ( int j=i+1; j<SubDim_; j++){
        this->V_.ExtractColumnVector(j, &tmpVector2);
        tmpVector2.ScaleAddScale(ValueTypeH(1), tmpVector, -tmpVector2.Dot(tmpVector));
        this->V_.ReplaceColumnVector(j, tmpVector2);
      }
    temp_Dim++;
    }
  }

  // Again
  temp_Dim = 0;
  for ( int i=0; i<SubDim_; i++ ){
    this->V_.ExtractColumnVector(i, &tmpVector);
    temp_norm = tmpVector.Norm();
    if (temp_norm > TOL){
      tmpVector.Scale(1/temp_norm);
      this->V_.ReplaceColumnVector(i, tmpVector);
      if ( i != temp_Dim )
        this->V_.ReplaceColumnVector(temp_Dim, tmpVector);
      for ( int j=i+1; j<SubDim_; j++){
        this->V_.ExtractColumnVector(j, &tmpVector2);
        tmpVector2.ScaleAddScale(ValueTypeH(1), tmpVector, -tmpVector2.Dot(tmpVector));
        this->V_.ReplaceColumnVector(j, tmpVector2);
      }
    temp_Dim++;
    }
  }

  VectorTypeH v_zeros;
  v_zeros.CloneBackend(*this->op_);
  v_zeros.Allocate("zerosv", this->dim_);
  v_zeros.Zeros();

  // Clear V, AV of dim > SumDim
  for (int i = temp_Dim; i<SubDim_; i++){
    this->V_.ReplaceColumnVector(i, v_zeros); 
    this->AV_.ReplaceRowVector(i, v_zeros);
  }

  this->SubDim_ = temp_Dim;
  this->s_out_.Zeros();
  
  VectorTypeH tmpVcolvec;
  VectorTypeH tmpAVrowvec;
  tmpVcolvec.CloneBackend(*this->op_);
  tmpAVrowvec.CloneBackend(*this->op_);
  tmpVcolvec.Allocate("tmpVcolvec", this->dim_);
  tmpAVrowvec.Allocate("tmpAVrowvec", this->dim_);

  // Update AV_
  for (int i=0; i<this->SubDim_ -1; i++) {
    this->V_.ExtractColumnVector(i, &tmpVcolvec);
    this->op_->Apply(tmpVcolvec, &tmpAVrowvec);
    this->AV_.ReplaceRowVector(i, tmpAVrowvec);
  }

  // Update t_out_
  this->V_.ExtractColumnVector(this->SubDim_ - 1, &tmpVcolvec);
  this->t_out_.CloneFrom(tmpVcolvec);

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::MvOrthogonalize(OperatorTypeH *mat, VectorTypeH *vec, const int dim){

  LOG_DEBUG(this, "SIRA::MvOrthogonalize()",
            "");

  int NeedReOrth  = 0;
  ValueTypeH dot_res;
  VectorTypeH tmpColVec;
  tmpColVec.CloneBackend(*vec);
  tmpColVec.Allocate("tmpColVec", mat->get_nrow());

  ValueTypeH *tmpDot = new ValueTypeH[mat->get_nrow()];
  
  for ( int i=0; i<dim; i++ ) {
    mat->ExtractColumnVector(i, &tmpColVec);
    tmpDot[i] = -(vec->Dot(tmpColVec));
  }

  //t = t - < V(:, i)', t > * V(:, 1)
  for ( int i=0; i<dim; i++ ) {
    mat->ExtractColumnVector(i, &tmpColVec);
    vec->ScaleAddScale(ValueTypeH(1), tmpColVec, tmpDot[i]);
  }

  // Check if need to reorthogonalize 
  for (int i=0; i < dim; i++) {
    mat->ExtractColumnVector(i, &tmpColVec);
    dot_res = vec->Dot(tmpColVec);
    if ( dot_res < 0) { dot_res = -dot_res; }
    if ( dot_res > 1e-7 ) {
      NeedReOrth = 1;
      break;
    }
  }

  // Reorthogonalize ( Do again! )
  if (NeedReOrth == 1) {
    for ( int i=0; i<dim; i++ ) {
      mat->ExtractColumnVector(i, &tmpColVec);
      tmpDot[i] = -(vec->Dot(tmpColVec));
    }
    for ( int i=0; i<dim; i++ ) {
      mat->ExtractColumnVector(i, &tmpColVec);
      vec->ScaleAddScale(ValueTypeH(1), tmpColVec, tmpDot[i]);
      //t = t - < V(:, i)', t > * V(:, 2)
    }
  }

  vec->Scale(1/vec->Norm());

  delete [] tmpDot;

}

#ifdef SUPPORT_MKL

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Eigpair(ValueTypeH &theta_k_){

  LOG_DEBUG(this, "SIRA::Eigpair()",
            "with MKL");

  lapack_int  lapack_N    = this->SubDim_,
              lapack_lda  = this->MaxSubDim_,
              lapack_ldvl = this->SubDim_,
              lapack_ldvr = this->SubDim_,
              info;

  double *thetas    = (double*)malloc(sizeof(double)*this->SubDim_);
  double *Im_thetas = (double*)malloc(sizeof(double)*this->SubDim_);
  double *Left_sk   = (double*)malloc(sizeof(double)*SubDim_*this->SubDim_);
  double *EigVec    = (double*)malloc(sizeof(double)*SubDim_*this->SubDim_);

  ValueTypeH *VTAV_arr = NULL;
  this->VtAV_.MoveToHost();
  this->VtAV_.LeaveDataPtrDENSE(&VTAV_arr);
  this->VtAV_.Clear();
  this->VtAV_.AllocateDENSE("VtAV_", this->MaxSubDim_, this->MaxSubDim_);
  this->VtAV_.CloneBackend(*this->op_);

  info = LAPACKE_dgeev( LAPACK_COL_MAJOR, 'N', 'V', lapack_N, VTAV_arr, lapack_lda, 
         thetas, Im_thetas, Left_sk, lapack_ldvl, EigVec, lapack_ldvr);
  
  //min eigenvalue and its eigenvater
  int min_idx = 0;
  int i;
  this->theta_k_ = paralution_abs(thetas[0]);
  for (i = 1; i < this->SubDim_; i++){
    if ( paralution_abs(thetas[i] - this->target_) < this->theta_k_ ){
      this->theta_k_ = paralution_abs(thetas[i] );
      min_idx = i;
    }          
  }

  // Assign s_out_
  this->s_out_.MoveToHost();   
  for (i = 0; i < this->SubDim_; i++){
    ValueTypeH si  = EigVec[min_idx * lapack_ldvr + i];
    this->s_out_[i] = si;
  }

  this->theta_k_ = thetas[min_idx]; 
  this->s_out_.CloneBackend(*this->op_);

  free_host(&VTAV_arr);

}

#else // MKL

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Eigpair(ValueTypeH &theta_k_){

  LOG_DEBUG(this, "SIRA::Eigpair()",
            "w/o MKL");

  // Transform the symetric matrix to tridiagonal matrix by Householder 

  ValueTypeH *VTAV_arr = NULL;
  this->VtAV_.MoveToHost();
  this->VtAV_.LeaveDataPtrDENSE(&VTAV_arr);
  this->VtAV_.Clear();
  this->VtAV_.AllocateDENSE("VtAV_", this->MaxSubDim_, this->MaxSubDim_);
  this->VtAV_.CloneBackend(*this->op_);

  ValueTypeH *Q_arr = new ValueTypeH[this->SubDim_*this->SubDim_];
  ValueTypeH *H_arr = new ValueTypeH[this->SubDim_*this->SubDim_];

  for (int i=0; i<this->SubDim_; i++){
    for (int j=0; j<this->SubDim_; j++){
      if (i == j) {  
        Q_arr[j*this->SubDim_ + i] = 1.0;
        H_arr[j*this->SubDim_ + i] = 1.0;
      } else {
        Q_arr[j*this->SubDim_ + i] = 0.0;
        H_arr[j*this->SubDim_ + i] = 0.0;
      }
    }
  }

  int i;

  for (i = 0; i < this->SubDim_ - 2; i++ ){
      ValueTypeH *v = new ValueTypeH[this->SubDim_];
      ValueTypeH a;
      ValueTypeH alpha=0.0;

      // a = VtAV_(i+1, i)
      a = VTAV_arr[i*(this->MaxSubDim_) + i+1];

      // alpha = -norm( VtAV(i+1;n, i) )
      for (int j = i+1; j<this->SubDim_; j++){
          alpha += pow(VTAV_arr[i*this->MaxSubDim_ + j], 2);
      }
      alpha = -sqrt(alpha);
      ValueTypeH sqr = sqrt(2*alpha*(alpha-a));
      v[i+1] = (a-alpha)/sqr;

      // v(i+2:n) = VtAV_(i+2:n, i)/sqr
      for (int j = i+2; j<this->SubDim_; j++){
          v[j] = VTAV_arr[i*this->MaxSubDim_+j]/sqr;
      }

      // H = I - v*v'
      for (int aj = i+1; aj < this->SubDim_; aj++){
          for (int ai = i+1; ai < this->SubDim_; ai++){
              H_arr[aj*this->SubDim_ + ai] += -2 * v[ai] * v[aj];
          }
      }

      // VTAV = H * VTAV * H
      ValueTypeH *tmpVTAV = new ValueTypeH[this->SubDim_*this->SubDim_];
      for (int i=0; i< this->SubDim_*this->SubDim_; i++) tmpVTAV[i] = 0;
      for (int i=0; i < this->SubDim_; i++) {
        for (int j=0; j < this->SubDim_; j++) {
          for (int ai=0; ai < this->SubDim_; ai++) {
            for (int aj=0; aj < this->SubDim_; aj++){
              tmpVTAV[j*this->SubDim_ + i] += H_arr[aj * this->SubDim_ + j] * 
                                              H_arr[ai * this->SubDim_ + i] *
                                              VTAV_arr[aj * this->MaxSubDim_ + ai];
            }
          }
        }
      }

      // for (int i=0; i<this->SubDim_*this->SubDim_; i++) VTAV_arr[i] = tmpVTAV[i];
      for (int i=0; i<this->SubDim_; i++) {
        for (int j=0; j<this->SubDim_; j++) {
          VTAV_arr[j*this->MaxSubDim_ + i] = tmpVTAV[j*this->SubDim_ + i];
        }
      }

      ValueTypeH *tmpQ = new ValueTypeH[this->SubDim_*this->SubDim_];
      for (int i=0; i < this->SubDim_*this->SubDim_; i++) tmpQ[i] = 0;

      // Q = Q * H'
      for (int i=0; i < this->SubDim_; i++) {
        for (int j=0; j < this->SubDim_; j++) {
          for (int k=0; k < this->SubDim_; k++){
            tmpQ[j*this->SubDim_ + i] += Q_arr[k*this->SubDim_ + i]*H_arr[j*this->SubDim_ + k];
          }
        }
      }

      for (int i=0; i < this->SubDim_*this->SubDim_; i++) Q_arr[i] = tmpQ[i];

      // H = I
      for (int aj = 0; aj < this->SubDim_; aj++){
          for (int ai = 0; ai < this->SubDim_; ai++){
              if (ai == aj){
                  H_arr[aj * this->SubDim_ + ai] = 1.0;
              }else{
                  H_arr[aj * this->SubDim_ + ai] = 0.0;
              }
          }
      } 

      delete [] tmpVTAV;
      delete [] tmpQ;
      delete [] v;
  }

  //Transform the tridiagonal matrix to diagonal matrix by givens rotations
  int start = 0;
  int n = this->SubDim_ - 1;
  double tmp = 0.0;
  double error = 1e-11;
  ValueTypeH x, y, w, c, s, z, a_k, a_k1;
  int *v = new int[this->SubDim_];

  for (int i=0; i<this->SubDim_; i++) v[i] = 0;
  
  while (n > 0){
      while (n <= start){
          start = v[start];
      }
      // abs(VtAV_[start,start+1]) is small enough, we don't need Givens rotations
      while (n > start && paralution_abs( VTAV_arr[(start+1)*this->MaxSubDim_ + start] ) <= error ){
          v[start+1] = v[start];
          start++;
      }
      // abs(VtAV_[n-1,n]) is small enough, we don't need Givens rotations
      while (n > 0 && (paralution_abs( VTAV_arr[n*this->MaxSubDim_ + n-1 ] ) <= error) ){
          n--;          
      }
      if (n > 0){
          double d = ( VTAV_arr[(n-1)*this->MaxSubDim_+n-1] - VTAV_arr[n*this->MaxSubDim_+ n] ) / 2.0;
          double sgnd = 1.0;
          if (d == 0){
              tmp = VTAV_arr[n*this->MaxSubDim_+ n] - paralution_abs(VTAV_arr[n*this->MaxSubDim_+ n-1] );
          } else {
              if (d < 0)
                  sgnd = -1.0;
              double norm = 0.0;
              norm = sqrt( pow(d, 2.0) + pow(VTAV_arr[n*this->MaxSubDim_+ n-1], 2.0) );
              tmp = VTAV_arr[n*this->MaxSubDim_+ n] - pow(VTAV_arr[n*this->MaxSubDim_+ n-1], 2.0) / ( d + sgnd * norm );
          }
          x = VTAV_arr[start*this->MaxSubDim_+start] - tmp;
          y = VTAV_arr[(start+1)*this->MaxSubDim_ +  start];
          
          for (int k = start; k < n; k++){
              if (n > 1){
                  this->Givens(c, s, x, y);
              }           
              else{
                  ValueTypeH _start     = VTAV_arr[start*this->MaxSubDim_+start];
                  ValueTypeH _start_1   = VTAV_arr[(start+1)*this->MaxSubDim_+start];
                  this->Givens(c, s, _start, _start_1);
              }

              //Q = Q * G(k, k+1), where G is the Givens rotation matrix
              for (int ai=0; ai<this->SubDim_; ai++){
                  a_k = Q_arr[k*this->SubDim_ + ai];
                  a_k1 = Q_arr[(k+1)*this->SubDim_ + ai];
                  Q_arr[k*this->SubDim_+ai] = a_k * c - a_k1 * s;
                  Q_arr[(k+1)*this->SubDim_+ai] = a_k * s + a_k1 * c;
              }

              // VtAV = VtAV * G(k, k+1), where G is the Givens rotation matrix
              w = c * x - s * y;
              d = VTAV_arr[k*this->MaxSubDim_+k] - VTAV_arr[(k+1)*this->MaxSubDim_+ k+1];
              z = ( 2*c*VTAV_arr[(k+1)*this->MaxSubDim_ + k] + d*s ) *s;
              VTAV_arr[k*this->MaxSubDim_+k] += -z;
              VTAV_arr[(k+1)*this->MaxSubDim_+ k+1] += +z;
              VTAV_arr[(k+1)*this->MaxSubDim_ + k] = d*c*s + (pow(c,2.0) - pow(s,2.0)) * VTAV_arr[(k+1)*this->MaxSubDim_+k];
              x = VTAV_arr[(k+1)*this->MaxSubDim_ + k];
              if (k > start){
                  VTAV_arr[k*this->MaxSubDim_ + k-1] = w;
              }
              if (k < n-1){
                  y = -s*VTAV_arr[(k+2)*this->MaxSubDim_ + k+1];
                  VTAV_arr[(k+2)*this->MaxSubDim_ + k+1] = c*VTAV_arr[(k+2)*this->MaxSubDim_ + k+1];
              }
          }

          // abs(VtAV_[k,k]) is small enough
          for (int k = n; k > start; k--){
              if ( paralution_abs( VTAV_arr[k*this->MaxSubDim_+ k] ) < error ){
                  v[k] = start;
                  start = k;
                  break;
              }
          }
      }
  }
      
  //Find smallest eigenvalue in absolute value and its eigenvector
  int min_idx = 0;
  this->theta_k_ = paralution_abs(VTAV_arr[0]);
  for (i = 1; i < this->SubDim_; i++){
      if ( paralution_abs(VTAV_arr[i*this->MaxSubDim_+i] ) < this->theta_k_ ){
          this->theta_k_ = paralution_abs(VTAV_arr[i*this->MaxSubDim_+ i]);
          min_idx = i;
      }          
  }

  this->s_out_.MoveToHost();
  for (i = 0; i < this->SubDim_; i++){
      ValueTypeH si  = Q_arr[min_idx*this->SubDim_ + i];
      this->s_out_[i] = si;
  }
  this->theta_k_ = VTAV_arr[min_idx*this->MaxSubDim_+ min_idx];
  this->VtAV_.CloneBackend(*this->op_);
  this->s_out_.CloneBackend(*this->op_);

  delete [] v;
  delete [] Q_arr;
  delete [] H_arr;
  free_host(&VTAV_arr);

}

#endif // MKL

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
     OperatorTypeL, VectorTypeL, ValueTypeL>::Givens(ValueTypeH &c, ValueTypeH &s, ValueTypeH &a, ValueTypeH &b){

  LOG_DEBUG(this, "SIRA::Givens()",
            "");

  if ( b == 0){
    c = -1.0;
    s = 0.0;
  }else if (paralution_abs(b) > paralution_abs(a)){
    s = -1 / sqrt( 1 + pow(a/b, 2.0) );
    c = -s * a/ b;
  }else{
    c = -1 / sqrt( 1 + pow(b/a, 2.0) );
    s = -c * b/ a;
  }

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::RecordResidualHistory(void){

  LOG_DEBUG(this, "SIRA::RecordResidualHistory()",
            "");

  this->iter_ctrl_.RecordHistory();

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
int SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::GetIterationCount(void){

  LOG_DEBUG(this, "SIRA::GetIterationCount()",
            "");

  return this->iter_ctrl_.GetIterationCount();

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
double SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::GetCurrentResidual(void){

  LOG_DEBUG(this, "SIRA::GetCurrentResidual()",
            "");

  return this->iter_ctrl_.GetCurrentResidual();

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
int SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::GetSolverStatus(void){

  LOG_DEBUG(this, "SIRA::GetSolverStatus()",
            "");

  return this->iter_ctrl_.GetSolverStatus();

}

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
          class OperatorTypeL, class VectorTypeL, typename ValueTypeL>
void SIRA<OperatorTypeH, VectorTypeH, ValueTypeH, 
          OperatorTypeL, VectorTypeL, ValueTypeL>::Verbose(const int verb) {

  LOG_DEBUG(this, "SIRA::Verbose()",
            verb);

  this->verb_ = verb; 
  this->iter_ctrl_.Verbose(verb);

}

template class SIRA< LocalMatrix<double>, LocalVector<double>, double,
                     LocalMatrix<double>, LocalVector<double>, double >;
template class SIRA< LocalMatrix<double>, LocalVector<double>, double,
                     LocalMatrix<float>,  LocalVector<float>,  float >;

}

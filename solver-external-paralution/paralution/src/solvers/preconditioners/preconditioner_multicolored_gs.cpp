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
#include "preconditioner_multicolored_gs.hpp"
#include "preconditioner_multicolored.hpp"
#include "preconditioner.hpp"
#include "../solver.hpp"

#include "../../base/local_matrix.hpp"

#include "../../base/local_vector.hpp"

#include "../../utils/log.hpp"

#include <complex>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredSGS<OperatorType, VectorType, ValueType>::MultiColoredSGS() {

  LOG_DEBUG(this, "MultiColoredSGS::MultiColoredSGS()",
            "default constructor");

  this->omega_ = ValueType(1.0);

}

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredSGS<OperatorType, VectorType, ValueType>::~MultiColoredSGS() {

  LOG_DEBUG(this, "MultiColoredSGS::~MultiColoredSGS()",
            "destructor");

  this->Clear();

}

// TODO
// not optimal implementation - scale the diagonal vectors in the building phase
template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SetRelaxation(const ValueType omega) {

  LOG_DEBUG(this, "MultiColoredSGS::SetRelaxation()",
            omega);

  this->omega_ = omega ;

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Multicolored Symmetric Gauss-Seidel (SGS) preconditioner");  

  if (this->build_ == true) {
    LOG_INFO("number of colors = " << this->num_blocks_ ); 
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::ReBuildNumeric(void) {

  LOG_DEBUG(this, "MultiColoredSGS::ReBuildNumeric()",
            this->build_);

  if (this->preconditioner_ != NULL) {
    this->preconditioner_->Clear();
    delete this->preconditioner_;
  }
  
  for(int i=0; i<this->num_blocks_; ++i) {
    
    delete this->x_block_[i] ;
    delete this->diag_block_[i];
    delete this->diag_solver_[i];

    for(int j=0; j<this->num_blocks_; ++j)
      delete this->preconditioner_block_[i][j];

    delete[] this->preconditioner_block_[i];

  }

  delete[] this->preconditioner_block_;
  delete[] this->x_block_;
  delete[] this->diag_block_;
  delete[] this->diag_solver_;

  this->preconditioner_ = new OperatorType ;
  this->preconditioner_->CloneFrom(*this->op_);

  this->Permute_();
  this->Factorize_();
  this->Decompose_();

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::PostAnalyse_(void) {

  LOG_DEBUG(this, "MultiColoredSGS::PostAnalyse_()",
            this->build_);

  assert(this->build_ == true);  
  this->preconditioner_->LAnalyse(false);
  this->preconditioner_->UAnalyse(false);
  
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SolveL_(void) {

  LOG_DEBUG(this, "MultiColoredSGS::SolveL_()",
            "");

  assert(this->build_ == true);

  for (int i=0; i<this->num_blocks_; ++i){

    for (int j=0; j<i; ++j)
      this->preconditioner_block_[i][j]->ApplyAdd(*this->x_block_[j],
                                                  ValueType(-1.0),
                                                  this->x_block_[i]);

    this->diag_solver_[i]->Solve(*this->x_block_[i],
                                 this->x_block_[i]);

    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(ValueType(1.0)/this->omega_);

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SolveD_(void) {

  LOG_DEBUG(this, "MultiColoredSGS::SolveD_()",
            "");

  assert(this->build_ == true);

  for (int i=0; i<this->num_blocks_; ++i) {
    this->x_block_[i]->PointWiseMult(*this->diag_block_[i]);

    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(this->omega_/(ValueType(2.0) - this->omega_));

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::SolveR_(void) {

  LOG_DEBUG(this, "MultiColoredSGS::SolveR_()",
            "");

  assert(this->build_ == true);
  
  for (int i=this->num_blocks_-1; i>=0; --i){
    
    for (int j=this->num_blocks_-1; j>i; --j)
      this->preconditioner_block_[i][j]->ApplyAdd(*this->x_block_[j],
                                                  ValueType(-1.0),
                                                  this->x_block_[i]);

    this->diag_solver_[i]->Solve(*this->x_block_[i],
                                 this->x_block_[i]);
    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(ValueType(1.0)/this->omega_);

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredSGS<OperatorType, VectorType, ValueType>::Solve_(const VectorType &rhs,
                                                                  VectorType *x) {

  LOG_DEBUG(this, "MultiColoredSGS::Solve_()",
            "");

  this->x_.CopyFromPermute(rhs,
                           this->permutation_);   
  

  this->preconditioner_->LSolve(this->x_, x);
  
  x->PointWiseMult(this->diag_);

  this->preconditioner_->USolve(*x, &this->x_);
  
  x->CopyFromPermuteBackward(this->x_,
                             this->permutation_);

}

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredGS<OperatorType, VectorType, ValueType>::MultiColoredGS() {
}

template <class OperatorType, class VectorType, typename ValueType>
MultiColoredGS<OperatorType, VectorType, ValueType>::~MultiColoredGS() {

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("Multicolored Gauss-Seidel (GS) preconditioner");  

  if (this->build_ == true) {
    LOG_INFO("number of colors = " << this->num_blocks_ ); 
  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::PostAnalyse_(void) {

  assert(this->build_ == true);  
  this->preconditioner_->UAnalyse(false);

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::SolveL_(void) {
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::SolveD_(void) {
}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::SolveR_(void) {

  assert(this->build_ == true);
  
  for (int i=this->num_blocks_-1; i>=0; --i){
    
    for (int j=this->num_blocks_-1; j>i; --j)
      this->preconditioner_block_[i][j]->ApplyAdd(*this->x_block_[j],
                                                  ValueType(-1.0),
                                                  this->x_block_[i]);
    
    this->diag_solver_[i]->Solve(*this->x_block_[i], 
                                 this->x_block_[i]);

    // SSOR
    if (this->omega_ != ValueType(1.0))
      this->x_block_[i]->Scale(ValueType(1.0)/this->omega_);

  }

}

template <class OperatorType, class VectorType, typename ValueType>
void MultiColoredGS<OperatorType, VectorType, ValueType>::Solve_(const VectorType &rhs,
                                                                 VectorType *x) {

  LOG_INFO("No implemented yet");
  FATAL_ERROR(__FILE__, __LINE__);

}


template class MultiColoredSGS< LocalMatrix<double>, LocalVector<double>, double >;
template class MultiColoredSGS< LocalMatrix<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class MultiColoredSGS< LocalMatrix<std::complex<double> >,  LocalVector<std::complex<double> >, std::complex<double> >;
template class MultiColoredSGS< LocalMatrix<std::complex<float> >,  LocalVector<std::complex<float> >, std::complex<float> >;
#endif

template class MultiColoredGS< LocalMatrix<double>, LocalVector<double>, double >;
template class MultiColoredGS< LocalMatrix<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class MultiColoredGS< LocalMatrix<std::complex<double> >,  LocalVector<std::complex<double> >, std::complex<double> >;
template class MultiColoredGS< LocalMatrix<std::complex<float> >,  LocalVector<std::complex<float> >, std::complex<float> >;
#endif

}

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
#include "iter_ctrl.hpp"
#include "../utils/log.hpp"
#include "../utils/math_functions.hpp"

#include <math.h>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <complex>

namespace paralution {

IterationControl::IterationControl() {

  this->Clear();

}

IterationControl::~IterationControl() {

  this->Clear();

}

void IterationControl::Clear(void) {

  this->residual_history_.clear();
  this->iteration_ = 0 ;

  this->init_res_ = false;
  this->rec_ = false;
  this->verb_ = 1;
  this->reached_ = 0 ;

  this->current_res_ = double(0.0);
  this->current_index_ = -1;

  this->absolute_tol_   = double(1e-15);
  this->relative_tol_   = double(1e-6);
  this->divergence_tol_ = double(1e+8);
  this->maximum_iter_   = 1000000;

}

void IterationControl::Init(const double abs, const double rel, const double div, const int max) {

  this->InitTolerance(abs, rel, div);
  this->InitMaximumIterations(max);

}

bool IterationControl::InitResidual(const double res) {

  this->init_res_ = true;
  this->initial_residual_ = res;

  this->reached_ = 0 ;
  this->iteration_ = 0 ;

  if (this->verb_ > 0)
    LOG_INFO("IterationControl initial residual = " << res);

  if (this->rec_ == true) 
    this->residual_history_.push_back(res);
  
  if (( paralution_abs(res) == std::numeric_limits<double>::infinity()) || // infinity
      ( res != res ) ) { // not a number (NaN)

    LOG_INFO("Residual = " << res << " !!!");
    return false;

  }

  if ( paralution_abs(res) <= this->absolute_tol_ ) {
    
    this->reached_ = 1;
    return false;

  }

  return true;
}

void IterationControl::InitTolerance(const double abs, const double rel, const double div) {

  this->absolute_tol_   = abs;
  this->relative_tol_   = rel;
  this->divergence_tol_ = div;

  if (( paralution_abs(abs) == std::numeric_limits<double>::infinity()) || // infinity
      ( abs != abs ) ) // not a number (NaN)
    LOG_INFO("Abs tol = " << abs << " !!!");

  if (( paralution_abs(rel) == std::numeric_limits<double>::infinity()) || // infinity
      ( rel != rel ) ) // not a number (NaN)
    LOG_INFO("Rel tol = " << rel << " !!!");

  if (( paralution_abs(div) == std::numeric_limits<double>::infinity()) || // infinity
      ( div != div ) ) // not a number (NaN)
    LOG_INFO("Div tol = " << div << " !!!");

}

void IterationControl::InitMaximumIterations(const int max) {

  assert(max >= 0);

  this->maximum_iter_ = max;

  if (max != max) // not a number (NaN)
    LOG_INFO("Max iter = " << max << " !!!");

}

int IterationControl::GetMaximumIterations(void) const {

  return this->maximum_iter_;

}

int IterationControl::GetIterationCount(void) {

  return this->iteration_;

}

double IterationControl::GetCurrentResidual(void) {

  return this->current_res_;

}

int IterationControl::GetAmaxResidualIndex(void) {

  return this->current_index_;

}

int IterationControl::GetSolverStatus(void) {

  return this->reached_;

}

bool IterationControl::CheckResidual(const double res) {

  assert(this->init_res_ == true);

  this->iteration_++;
  this->current_res_ = res;

  if (this->verb_ > 1)
    LOG_INFO("IterationControl iter=" << this->iteration_ <<  "; residual=" << res);

  if (this->rec_ == true) 
    this->residual_history_.push_back(res);
  
  if (( paralution_abs(res) == std::numeric_limits<double>::infinity()) || // infinity
      ( res != res ) ) { // not a number (NaN)

    LOG_INFO("Residual = " << res << " !!!");
    return true;

  }

  if ( paralution_abs(res) <= this->absolute_tol_ ) {
    
    this->reached_ = 1;
    return true;

  }

  if ( res / this->initial_residual_ <= this->relative_tol_ ) {

    this->reached_ = 2;
    return true;

  }

  if ( res / this->initial_residual_ >= this->divergence_tol_ ) {

    this->reached_ = 3;
    return true;

  }

  if ( this->iteration_ >= this->maximum_iter_ ) {

    this->reached_ = 4;
    return true;

  }

  return false;

}

bool IterationControl::CheckResidual(const double res, const int index) {

  this->current_index_ = index;
  return this->CheckResidual(res);

}

bool IterationControl::CheckResidualNoCount(const double res) const {

  assert(this->init_res_ == true);

  if (( paralution_abs(res) == std::numeric_limits<double>::infinity()) || // infinity
      ( res != res ) ) { // not a number (NaN)

    LOG_INFO("Residual = " << res << " !!!");
    return true;

  }

  if ( paralution_abs(res) <= this->absolute_tol_ )
    return true;

  if ( res / this->initial_residual_ <= this->relative_tol_ )
    return true;

  if ( res / this->initial_residual_ >= this->divergence_tol_ )
    return true;

  if ( this->iteration_ >= this->maximum_iter_ )
    return true;

  return false;

}

void IterationControl::RecordHistory(void) {

  this->rec_ = true;

}

void IterationControl::Verbose(const int verb) {

  this->verb_ = verb;

}

void IterationControl::WriteHistoryToFile(const std::string filename) const {

  std::ofstream file;
  std::string line;

  assert(this->residual_history_.size() > 0);
  assert(this->iteration_ > 0);

  LOG_INFO("Writing residual history to filename = "<< filename << "; writing...");

  file.open(filename.c_str(), std::ifstream::out );
  
  if (!file.is_open()) {
    LOG_INFO("Can not open file [write]:" << filename);
    FATAL_ERROR(__FILE__, __LINE__);
  }

  file.setf(std::ios::scientific);

  for (int n=0; n<this->iteration_; n++)
    file << this->residual_history_[n] << std::endl;

  file.close();

  LOG_INFO("Writing residual history to filename = "<< filename << "; done");

}

void IterationControl::PrintInit(void) {

  LOG_INFO("IterationControl criteria: "
           << "abs tol=" << this->absolute_tol_ << "; "
           << "rel tol=" << this->relative_tol_ << "; "
           << "div tol=" << this->divergence_tol_ << "; "
           << "max iter=" << this->maximum_iter_);

}

void IterationControl::PrintStatus(void) {

  switch (reached_) {

  case 1:
    LOG_INFO("IterationControl ABSOLUTE criteria has been reached: "
             << "res norm=" << paralution_abs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  case 2:
    LOG_INFO("IterationControl RELATIVE criteria has been reached: "
             << "res norm=" << paralution_abs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  case 3:
    LOG_INFO("IterationControl DIVERGENCE criteria has been reached: "
             << "res norm=" << paralution_abs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  case 4:
    LOG_INFO("IterationControl MAX ITER criteria has been reached: "
             << "res norm=" << paralution_abs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);
    break;

  default:
    LOG_INFO("IterationControl NO criteria has been reached: "
             << "res norm=" << paralution_abs(this->current_res_) << "; "
             << "rel val=" << this->current_res_ / this->initial_residual_ << "; "
             << "iter=" << this->iteration_);

  }

}


}

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

#ifndef PARALUTION_EIGENVALUE_CG_HN_HPP_
#define PARALUTION_EIGENVALUE_CG_HN_HPP_

#include "../solver.hpp"
#include "../krylov/cg.hpp"
#include <assert.h>
#include <math.h>
#include <fstream>
#include <vector>

namespace paralution {
template <class OperatorType, class VectorType, typename ValueType>
class CG_HN : public paralution::CG<OperatorType, VectorType, ValueType> {

public:
        
  CG_HN();
  virtual ~CG_HN();
  virtual void Print(void) const;
  virtual void Build(void);
  virtual void Clear(void);

  virtual void MoveToAcceleratorLocalData_(void);
  virtual void MoveToHostLocalData_(void);

  virtual void Solve        (const VectorType &rhs, VectorType *x, VectorType &u_k);

  virtual void SetIta       (const ValueType a) {this->Ita_         = a; return;};
  virtual void SetTol_out   (const ValueType a) {this->Tol_out_     = a; return;};
  virtual void SetNorm_r_out(const ValueType a) {this->norm_r_out_  = a; return;};
  virtual void SetAlpha     (const ValueType a) {this->alpha_set_   = a; return;};
  virtual void SetTau_1     (const ValueType a) {this->Tau_1_       = a; return;};
  virtual void SetTau_2     (const ValueType a) {this->Tau_2_       = a; return;};
  virtual void SetTau_3     (const ValueType a) {this->Tau_3_       = a; return;};

protected:
  virtual void SolveNonPrecond(const VectorType &rhs, VectorType *x, VectorType &u_k);
  virtual void SolvePrecond   (const VectorType &rhs, VectorType *x, VectorType &u_k);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

private:

  virtual void GetBeta   (VectorType *x, const VectorType &u_k);
  virtual void Get_r_eig (void);
  virtual bool JudgeCaseB(void);
  virtual bool JudgeCaseC(void);
  virtual bool Check_HN  (ValueType res, VectorType *x, VectorType &u_k);
  bool  CaseB_, CaseC_, UsingHN_, IsFirst_;
  int   GetBetaTime_;
  VectorType r_, z_;
  VectorType p_, q_;
  VectorType u_A_, x_A_;
  ValueType s_set_, alpha_set_, beta_set_, r_eig_;
  ValueType g_k_[2];
  ValueType Ita_, Tol_out_, norm_r_out_, Tau_1_, Tau_2_, Tau_3_, ResAtFirst_;

};

}

#endif // PARALUTION_EIGENVALUE_CG_HN_HPP_



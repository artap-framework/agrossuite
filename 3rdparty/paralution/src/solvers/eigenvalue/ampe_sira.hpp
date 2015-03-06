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

#ifndef PARALUTION_EIGENVALUE_AMPE_SIRA_HPP_
#define PARALUTION_EIGENVALUE_AMPE_SIRA_HPP_

#include "cg_hn.hpp"
#include "../solver.hpp"
#include "../preconditioners/preconditioner_ai.hpp"
#include "../../base/local_matrix.hpp"
#include "../iter_ctrl.hpp"

namespace paralution {

template <class OperatorTypeH, class VectorTypeH, typename ValueTypeH,
		  class OperatorTypeL = LocalMatrix<double>, class VectorTypeL = LocalVector<double>, typename ValueTypeL = double>
class SIRA : public Solver <OperatorTypeH, VectorTypeH, ValueTypeH> {

public:

  SIRA();
  virtual ~SIRA();

  virtual void Print(void) const;
  virtual void Build(void);
  virtual void SetNumberOfEigenvalues(const int num);
  virtual void Clear(void);    

  /// Initialize the solver with abs_tol tolerance and 
  /// number of maximum outer iteration.
  /// The maximum number of inner iteration would be
  /// default         : MaxInnerIter = 0
  /// dim*MaxInnerIter: 0 < MaxInnerIter <= 1
  /// MaxInnerIter    : 1 < MaxInnerIter
  virtual void Init(ValueTypeH abs_tol, int MaxOuterIter, int MaxInnerIter);

  /// Set the stopping criteria of inner linear solver where
  /// 0: constant tolerance
  /// 1: H.N. stopping criteria
  virtual void SetInnerStoppingCriterion(const int n);

  /// Set the inner solver's preocnditioner
  virtual void SetInnerPreconditioner(Preconditioner<LocalMatrix<ValueTypeH>,
                                                     LocalVector<ValueTypeH>, 
                                                     ValueTypeH > &p);

  /// Set the inner solver's preocnditioner
  virtual void SetInnerPreconditioner(Preconditioner<LocalMatrix<ValueTypeH>,
                                                     LocalVector<ValueTypeH>, 
                                                     ValueTypeH > &p1,
                                      Preconditioner<LocalMatrix<ValueTypeL>, 
                                                     LocalVector<ValueTypeL>, 
                                                     ValueTypeL > &p2);
  
  /// Disable the preconditioner when a == 0 
  virtual void SetInnerPreconditioner(int a);

  /// Solve the eigenvalue with initial vector vec_start
  virtual void Solve(const VectorTypeH & vec_start, VectorTypeH *vec_ans);

  /// Solver the eigenvalue with random initial
  virtual void Solve(VectorTypeH *x);

  /// Return the iteration count
  virtual int GetIterationCount(void);

  /// Return the current residual
  virtual double GetCurrentResidual(void);

  /// Return the current status
  virtual int GetSolverStatus(void);

  /// Write the history to file
  void RecordResidualHistory(void);

  virtual void Verbose(const int verb=1);
protected:

  /// Solve the inner linear solver
  virtual void Inner();
  
  /// Orthogonalize vector to a orthogonal matrix
  virtual void MvOrthogonalize(OperatorTypeH *mat, VectorTypeH *vec, const int dim);

  /// Orthogonalize subspace to eigenspace 
  virtual void Locking();

  /// Perform Subspace restrating
  virtual void Restart();

  /// Givens rotation
  virtual void Givens(ValueTypeH &c, ValueTypeH &s, ValueTypeH &a, ValueTypeH &b); 

  /// Solve the eigenpair of small dense matrix
  virtual void Eigpair(ValueTypeH &theta_k_);

  virtual void Init(const ValueTypeH abs_tol, const ValueTypeH rel_tol,
                    const ValueTypeH div_tol, const int max_iter);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;
  virtual void MoveToHostLocalData_(void);
  virtual void MoveToAcceleratorLocalData_(void);

  virtual void Build_(void);

  /// Preconditioner for inner linear solver
  Preconditioner<LocalMatrix<ValueTypeH>, LocalVector<ValueTypeH>, ValueTypeH > *p_inner_h_;
  Preconditioner<LocalMatrix<ValueTypeL>, LocalVector<ValueTypeL>, ValueTypeL > *p_inner_l_;
  FSAI<LocalMatrix<ValueTypeH>, LocalVector<ValueTypeH>, ValueTypeH > *p_fsai_h_;
  FSAI<LocalMatrix<ValueTypeL>, LocalVector<ValueTypeL>, ValueTypeL > *p_fsai_l_;

  /// Inner linear solver 
  CG   <OperatorTypeH, VectorTypeH, ValueTypeH >* inner_cg_h_;
  CG_HN<OperatorTypeH, VectorTypeH, ValueTypeH >* inner_cg_hn_h_;
  CG_HN<OperatorTypeL, VectorTypeL, ValueTypeL >* inner_cg_hn_l_;

  /// Iteration control (monitor)
  IterationControl iter_ctrl_;


private:

  int dim_;
  int nnz_;
  int MaxSubDim_;
  int SubDim_;
  int DoubleFlag_;
  int StoppingCriterionFlag_;
  int target_num_;
  int solved_num_;
  int single_count_;
  int double_count_;
  int PrecondFlag_;
  int MaxInnerIter_;

  ValueTypeH target_;
  ValueTypeH theta_k_;
  ValueTypeH r_out_norm_;

  OperatorTypeH* op_h_;
  OperatorTypeL* op_l_;
  OperatorTypeH V_;
  OperatorTypeH AV_;
  OperatorTypeH VtAV_;
  OperatorTypeH EigVecs_;

  VectorTypeH tmpUpdateVector_;
  VectorTypeH s_out_;
  VectorTypeH Au_;
  VectorTypeH r_out_;
  VectorTypeL r_out_single_;
  VectorTypeH t_out_;
  VectorTypeL t_out_single_;
  VectorTypeH u_k_;
  VectorTypeL u_k_single_;


};

}

#endif // PARALUTION_EIGENVALUE_AMPE_SIRA_HPP_

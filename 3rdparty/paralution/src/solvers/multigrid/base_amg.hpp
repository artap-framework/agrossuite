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


#ifndef PARALUTION_BASE_AMG_HPP_
#define PARALUTION_BASE_AMG_HPP_

#include "../solver.hpp"
#include "base_multigrid.hpp"

#include <vector>

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
class BaseAMG : public BaseMultiGrid<OperatorType, VectorType, ValueType> {
  
public:

  BaseAMG();
  virtual ~BaseAMG();

  virtual void Build(void);
  virtual void Clear(void);
  virtual void ClearLocal(void);

  /// Creates AMG hierarchy
  virtual void BuildHierarchy(void);

  /// Creates AMG smoothers
  virtual void BuildSmoothers(void);

  /// Sets coarsest level for hierarchy creation
  virtual void SetCoarsestLevel(const int coarseSize);

  /// Sets flag to pass smoothers manually for each level
  virtual void SetManualSmoothers(const bool sm_manual);
  /// Sets flag to pass coarse grid solver manually
  virtual void SetManualSolver(const bool s_manual);

  /// Sets the smoother operator format
  virtual void SetDefaultSmootherFormat(const unsigned int op_format);
  /// Sets the operator format
  virtual void SetOperatorFormat(const unsigned int op_format);

  /// Returns the number of levels in hierarchy
  virtual int GetNumLevels();

  /// disabled function
  virtual void SetRestrictOperator(OperatorType **op);
  /// disabled function
  virtual void SetProlongOperator(OperatorType **op);
  /// disabled function
  virtual void SetOperatorHierarchy(OperatorType **op);

protected:

  /// Constructs the prolongation, restriction and coarse operator
  virtual void Aggregate(const OperatorType &op,
                         Operator<ValueType> *pro,
                         Operator<ValueType> *res,
                         OperatorType *coarse) = 0;


  /// maximal coarse grid size
  int coarse_size_;

  /// manual smoother or not
  bool set_sm_;
  Solver<OperatorType, VectorType, ValueType> **sm_default_;

  /// manual coarse grid solver or not
  bool set_s_;

  /// true if hierarchy is built
  bool hierarchy_;

  /// smoother operator format
  unsigned int sm_format_;
  /// operator format
  unsigned int op_format_;

};


}

#endif // PARALUTION_BASE_AMG_HPP_

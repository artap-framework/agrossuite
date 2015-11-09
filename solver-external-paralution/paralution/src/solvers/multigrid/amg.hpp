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


#ifndef PARALUTION_AMG_HPP_
#define PARALUTION_AMG_HPP_

#include "../solver.hpp"
#include "base_amg.hpp"

#include <vector>

namespace paralution {

enum _interp {
  Aggregation,
  SmoothedAggregation
};

template <class OperatorType, class VectorType, typename ValueType>
class AMG : public BaseAMG<OperatorType, VectorType, ValueType> {
  
public:

  AMG();
  virtual ~AMG();

  virtual void Print(void) const;

  /// Build AMG smoothers
  virtual void BuildSmoothers(void);

  /// Sets coupling strength
  virtual void SetCouplingStrength(const ValueType eps);
  /// Sets the interpolation type
  virtual void SetInterpolation(_interp interpType);
  /// Sets the relaxation parameter for smoothed aggregation
  virtual void SetInterpRelax(const ValueType relax);
  /// Sets over-interpolation parameter for aggregation
  virtual void SetOverInterp(const ValueType overInterp);

protected:

  /// Constructs the prolongation, restriction and coarse operator
  virtual void Aggregate(const OperatorType &op,
                         Operator<ValueType> *pro,
                         Operator<ValueType> *res,
                         OperatorType *coarse);

  virtual void PrintStart_(void) const;
  virtual void PrintEnd_(void) const;

private:

  /// Coupling strength
  ValueType eps_;

  /// Relaxation parameter for smoothed aggregation
  ValueType relax_;

  /// Over-interpolation parameter for aggregation
  ValueType over_interp_;
  
  /// interpolation type for grid transfer operators
  _interp interp_type_;

};


}

#endif // PARALUTION_AMG_HPP_

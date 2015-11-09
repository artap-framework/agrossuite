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
#include "amg.hpp"

#include "../../base/local_matrix.hpp"
#include "../../base/local_vector.hpp"

#include "../../solvers/preconditioners/preconditioner_multicolored_gs.hpp"

#include "../../utils/log.hpp"
#include "../../utils/math_functions.hpp"

namespace paralution {

template <class OperatorType, class VectorType, typename ValueType>
AMG<OperatorType, VectorType, ValueType>::AMG() {

  LOG_DEBUG(this, "AMG::AMG()",
            "default constructor");

  // parameter for strong couplings in smoothed aggregation
  this->eps_   = ValueType(0.01f);
  this->relax_ = ValueType(2.0f/3.0f);
  this->over_interp_ = ValueType(1.5f);
  this->interp_type_ = SmoothedAggregation;

}

template <class OperatorType, class VectorType, typename ValueType>
AMG<OperatorType, VectorType, ValueType>::~AMG() {

  LOG_DEBUG(this, "AMG::AMG()",
            "destructor");

  this->Clear();

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Print(void) const {

  LOG_INFO("AMG solver");

  LOG_INFO("AMG number of levels " << this->levels_);

  switch(this->interp_type_) {
    case Aggregation:
      LOG_INFO("AMG using aggregation interpolation");
      break;
    case SmoothedAggregation:
      LOG_INFO("AMG using smoothed aggregation interpolation");
      break;
  }

  LOG_INFO("AMG coarsest operator size = " << this->op_level_[this->levels_-2]->get_nrow());
  LOG_INFO("AMG coarsest level nnz = " <<this->op_level_[this->levels_-2]->get_nnz());
  LOG_INFO("AMG with smoother:");
  this->smoother_level_[0]->Print();
  
}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::PrintStart_(void) const {

  assert(this->levels_ > 0);

  LOG_INFO("AMG solver starts");
  LOG_INFO("AMG number of levels " << this->levels_);

  switch(this->interp_type_) {
    case Aggregation:
      LOG_INFO("AMG using aggregation interpolation");
      break;
    case SmoothedAggregation:
      LOG_INFO("AMG using smoothed aggregation interpolation");
      break;
  }

  LOG_INFO("AMG coarsest operator size = " << this->op_level_[this->levels_-2]->get_nrow());
  LOG_INFO("AMG coarsest level nnz = " <<this->op_level_[this->levels_-2]->get_nnz());
  LOG_INFO("AMG with smoother:");
  this->smoother_level_[0]->Print();

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::PrintEnd_(void) const {

    LOG_INFO("AMG ends");

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetInterpolation(_interp interpType) {

  this->interp_type_ = interpType;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetInterpRelax(const ValueType relax) {

  LOG_DEBUG(this, "AMG::SetInterpRelax()",
            relax);

  this->relax_ = relax;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetOverInterp(const ValueType overInterp) {

  LOG_DEBUG(this, "AMG::SetOverInterp()",
            overInterp);

  this->over_interp_ = overInterp;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::SetCouplingStrength(const ValueType eps) {

  LOG_DEBUG(this, "AMG::SetCouplingStrength()",
            eps);

  this->eps_ = eps;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::BuildSmoothers(void) {

  LOG_DEBUG(this, "AMG::BuildSmoothers()",
            " #*# begin");

  // Smoother for each level
  FixedPoint<OperatorType, VectorType, ValueType > **sm = NULL;
  sm = new FixedPoint<OperatorType, VectorType, ValueType >* [this->levels_-1];

  this->smoother_level_ = new IterativeLinearSolver<OperatorType, VectorType, ValueType>*[this->levels_-1];
  this->sm_default_ = new Solver<OperatorType, VectorType, ValueType>*[this->levels_-1];

  MultiColoredGS<OperatorType, VectorType, ValueType > **gs = NULL;
  gs = new MultiColoredGS<OperatorType, VectorType, ValueType >* [this->levels_-1];

  for (int i=0; i<this->levels_-1; ++i) {
    sm[i] = new FixedPoint<OperatorType, VectorType, ValueType >;
    gs[i] = new MultiColoredGS<OperatorType, VectorType, ValueType >;

    gs[i]->SetPrecondMatrixFormat(this->sm_format_);
    sm[i]->SetRelaxation(ValueType(1.3f));
    sm[i]->SetPreconditioner(*gs[i]);
    sm[i]->Verbose(0);
    this->smoother_level_[i] = sm[i];
    this->sm_default_[i] = gs[i];
  }

  delete[] gs;
  delete[] sm;

}

template <class OperatorType, class VectorType, typename ValueType>
void AMG<OperatorType, VectorType, ValueType>::Aggregate(const OperatorType &op,
                                                         Operator<ValueType> *pro,
                                                         Operator<ValueType> *res,
                                                         OperatorType *coarse) {

  LOG_DEBUG(this, "AMG::Aggregate()",
            this->build_);

  assert(&op    != NULL);
  assert(pro    != NULL);
  assert(res    != NULL);
  assert(coarse != NULL);

  OperatorType *cast_res = dynamic_cast<OperatorType*>(res);
  OperatorType *cast_pro = dynamic_cast<OperatorType*>(pro);

  assert(cast_res != NULL);
  assert(cast_pro != NULL);

  LocalVector<int> connections;
  LocalVector<int> aggregates;

  connections.CloneBackend(op);
  aggregates.CloneBackend(op);

  ValueType eps = this->eps_;
  for (int i=0; i<this->levels_-1; ++i)
    eps *= ValueType(0.5);

  op.AMGConnect(eps, &connections);
  op.AMGAggregate(connections, &aggregates);

  switch(this->interp_type_) {

    case Aggregation:
      op.AMGAggregation(aggregates, cast_pro, cast_res);
      break;

    case SmoothedAggregation:
      op.AMGSmoothedAggregation(this->relax_, aggregates, connections, cast_pro, cast_res);
      break;

    default:
      LOG_INFO("Aggregation type not valid");
      FATAL_ERROR(__FILE__, __LINE__);

  }

  // Free unused vectors
  connections.Clear();
  aggregates.Clear();

  OperatorType tmp;
  tmp.CloneBackend(op);
  coarse->CloneBackend(op);

  tmp.MatrixMult(*cast_res, op);
  coarse->MatrixMult(tmp, *cast_pro);

  if (this->interp_type_ == Aggregation && this->over_interp_ > ValueType(1.0))
    coarse->Scale(ValueType(1.0)/this->over_interp_);

}


template class AMG< LocalMatrix<double>, LocalVector<double>, double >;
template class AMG< LocalMatrix<float>,  LocalVector<float>, float >;
#ifdef SUPPORT_COMPLEX
template class AMG< LocalMatrix<std::complex<double> >, LocalVector<std::complex<double> >, std::complex<double> >;
template class AMG< LocalMatrix<std::complex<float> >,  LocalVector<std::complex<float> >,  std::complex<float> >;
#endif

}

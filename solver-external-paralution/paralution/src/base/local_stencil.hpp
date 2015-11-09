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


#ifndef PARALUTION_LOCAL_STENCIL_HPP_
#define PARALUTION_LOCAL_STENCIL_HPP_

#include "operator.hpp"
#include "local_vector.hpp"
#include "base_stencil.hpp"
#include "stencil_types.hpp"

namespace paralution {

template <typename ValueType>
class LocalVector;
template <typename ValueType>
class GlobalVector;

// Local Stencil
template <typename ValueType>
class LocalStencil : public Operator<ValueType> {

public:

  LocalStencil();
  LocalStencil(unsigned int type);
  virtual ~LocalStencil();

  virtual void info() const;

  virtual int get_ndim(void) const;
  virtual int get_nrow(void) const;
  virtual int get_ncol(void) const;
  virtual int get_nnz(void) const;

  virtual void SetGrid(const int size);

  virtual void Clear();

  virtual void Apply(const LocalVector<ValueType> &in, LocalVector<ValueType> *out) const; 
  virtual void ApplyAdd(const LocalVector<ValueType> &in, const ValueType scalar, 
                        LocalVector<ValueType> *out) const; 

  virtual void MoveToAccelerator(void);
  virtual void MoveToHost(void);

protected:

  virtual bool is_host(void) const {return true;};
  virtual bool is_accel(void) const {return false;};

private:
  
  std::string object_name_ ;

  BaseStencil<ValueType> *stencil_;

  HostStencil<ValueType> *stencil_host_;
  AcceleratorStencil<ValueType> *stencil_accel_;

  
  friend class LocalVector<ValueType>;
  friend class GlobalVector<ValueType>;

};


}

#endif // PARALUTION_LOCAL_STENCIL_HPP_

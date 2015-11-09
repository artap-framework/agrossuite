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


#ifndef PARALUTION_BASE_HPP_
#define PARALUTION_BASE_HPP_

#include "backend_manager.hpp"

#include <complex>
#include <vector>

namespace paralution {

class ParalutionObj {

public:

  ParalutionObj();
  virtual ~ParalutionObj();

  /// Clear (free all data) the object
  virtual void Clear() = 0;

protected:
  size_t global_obj_id;

};

/// Global data for all PARALUTION objects
struct Paralution_Object_Data {
  
  std::vector<class ParalutionObj*> all_obj;

};

/// Global obj tracking structure
extern struct Paralution_Object_Data Paralution_Object_Data_Tracking;

/// Base class for operator and vector 
/// (i.e. global/local matrix/stencil/vector) classes,
/// all the backend-related interface and data 
/// are defined here
template <typename ValueType>
class BaseParalution : public ParalutionObj {

public:

  BaseParalution();
  BaseParalution(const BaseParalution<ValueType> &src);
  virtual ~BaseParalution();

  BaseParalution<ValueType>& operator=(const BaseParalution<ValueType> &src);

  /// Move the object to the Accelerator backend
  virtual void MoveToAccelerator(void) = 0;

  /// Move the object to the Host backend
  virtual void MoveToHost(void) = 0;

  /// Move the object to the Accelerator backend with async move
  virtual void MoveToAcceleratorAsync(void);

  /// Move the object to the Host backend with async move
  virtual void MoveToHostAsync(void);

  // Sync (the async move)
  virtual void Sync(void);

  /// Clone the Backend descriptor from another object
  void CloneBackend(const BaseParalution<ValueType> &src);

  /// Clone the Backend descriptor from another object with different template ValueType
  template <typename ValueType2>
  void CloneBackend(const BaseParalution<ValueType2> &src);

  /// Print the object information (properties, backends)
  virtual void info() const = 0;

  /// Clear (free all data) the object
  virtual void Clear() = 0;

protected:

  /// Name of the object
  std::string object_name_;

  /// Backend descriptor 
  Paralution_Backend_Descriptor local_backend_;

  /// Return true if the object is on the host
  virtual bool is_host(void) const = 0;

  /// Return true if the object is on the accelerator
  virtual bool is_accel(void) const = 0;

  // active async transfer
  bool asyncf;

  friend class BaseParalution<double>;
  friend class BaseParalution<float>;
  friend class BaseParalution<std::complex<double> >;
  friend class BaseParalution<std::complex<float> >;

  friend class BaseParalution<int>;

};


}

#endif // PARALUTION_LOCAL_BASE_HPP_

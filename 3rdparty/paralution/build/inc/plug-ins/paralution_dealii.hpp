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


// Deal II path 
#include <lac/sparse_matrix.h>
#include <lac/sparsity_pattern.h>
#include <lac/vector.h>
#include <lac/block_sparse_matrix.h>


// PARALUTION path
#include <paralution.hpp>

/// Import (copy) a Deal.II block sparse matrix using its iterator and accessor into a
/// paralution Local Matrix
template <typename ValueType>
void import_dealii_matrix(dealii::BlockSparseMatrix<ValueType> &deal_mat,
                          paralution::LocalMatrix<ValueType> *mat) {

  // TODO
  // change for a Base class

  // Matrix size
  int n = deal_mat.n();
  int m = deal_mat.m();
  //  int nnz = deal_mat.n_actually_nonzero_elements();
  int nnz = deal_mat.n_nonzero_elements();

  // COO values
  int *col = NULL;
  int *row = NULL;
  ValueType *val = NULL;

  paralution::allocate_host(nnz, &row);
  paralution::allocate_host(nnz, &col);
  paralution::allocate_host(nnz, &val);

  dealii::BlockSparseMatrix<double>::iterator it = deal_mat.begin(),
    it_end = deal_mat.end();

  int i = 0;
  for (; it!=it_end; ++it) {

    col[i] = it.operator*().column();
    row[i] = it.operator*().row();
    val[i] = it.operator*().value();

    ++i;
  }

  assert(i == nnz);

  mat->SetDataPtrCOO(&row, &col, &val,
                     "Deal II Matrix",
                     nnz, n, m);
  
  mat->ConvertToCSR();

} ;


/// Import (copy) a Deal.II sparse matrix via prescribed sparsity pattern into a 
/// paralution Local Matrix
template <typename ValueType>
void import_dealii_matrix(const dealii::SparsityPattern &sp, 
                          const dealii::SparseMatrix<ValueType> &deal_mat,
                          paralution::LocalMatrix<ValueType> *mat) {

  // Matrix size
  int n = deal_mat.n();
  int m = deal_mat.m();
  //  int nnz = deal_mat.n_actually_nonzero_elements();
  int nnz = deal_mat.n_nonzero_elements();
  
  // COO values
  int *col = NULL;
  int *row = NULL;
  ValueType *val = NULL;

  paralution::allocate_host(nnz, &row);
  paralution::allocate_host(nnz, &col);
  paralution::allocate_host(nnz, &val);

  dealii::SparsityPatternIterators::Iterator it = sp.begin();
  dealii::SparsityPatternIterators::Iterator it_end = sp.end();

  int i = 0;
  for (; it!=it_end; ++it) {

    col[i] = it->column();
    row[i] = it->row();
    val[i] = deal_mat.el(it->row(),it->column());

    ++i;    
  }

  assert(i == nnz);

  mat->SetDataPtrCOO(&row, &col, &val,
                     "Deal II Matrix",
                     nnz, n, m);
  
  // mat->ConvertToCSR();

} ;

/// Import (copy) a Deal.II vector into a local vector
template <typename ValueType>
void import_dealii_vector(const dealii::Vector<ValueType> &deal_vec,                             
                             paralution::LocalVector<ValueType> *vec) {

  vec->MoveToHost();

  ValueType *val = NULL;
  paralution::allocate_host(int(deal_vec.size()), &val);

  for (unsigned int i=0; i<deal_vec.size(); ++i)
    val[i] = deal_vec[i];

  vec->SetDataPtr(&val, "DealII vector", int(deal_vec.size()));


  //  assert(int(vec->get_size()) == int(deal_vec.size()));
  //
  //  This is slower
  //
  //  for (unsigned int i=0; i<deal_vec.size(); ++i)
  //    (*vec)[i] = deal_vec[i];

}; 

/// Export (copy) a local vector into a Deal.II vector 
template <typename ValueType>
void export_dealii_vector(paralution::LocalVector<ValueType> &vec,
                            dealii::Vector<ValueType> *deal_vec) {

  vec.MoveToHost();
  assert(int(vec.get_size()) == int(deal_vec->size()));

  int size = vec.get_size();
  ValueType *val = NULL;

  vec.LeaveDataPtr(&val);

  for (int i=0; i<size; ++i)
    (*deal_vec)[i] = val[i];

  paralution::free_host(&val);
  
  //
  //  This is slower
  //
  //  for (unsigned int i=0; i<deal_vec->size(); ++i)
  //     (*deal_vec)[i] = vec[i];

}; 

/// Import (copy) a Deal.II vector into a local vector
template <typename ValueType>
void import_dealii_vector(const dealii::BlockVector<ValueType> &deal_vec,                             
                             paralution::LocalVector<ValueType> *vec) {

  vec->MoveToHost();

  ValueType *val = NULL;
  paralution::allocate_host(int(deal_vec.size()), &val);

  for (unsigned int i=0; i<deal_vec.size(); ++i)
    val[i] = deal_vec[i];

  vec->SetDataPtr(&val, "DealII vector", int(deal_vec.size()));


  //  assert(int(vec->get_size()) == int(deal_vec.size()));
  //
  //  This is slower
  //
  //  for (unsigned int i=0; i<deal_vec.size(); ++i)
  //    (*vec)[i] = deal_vec[i];

}; 

/// Export (copy) a local vector into a Deal.II vector 
template <typename ValueType>
void export_dealii_vector(paralution::LocalVector<ValueType> &vec,
                            dealii::BlockVector<ValueType> *deal_vec) {

  vec.MoveToHost();
  assert(int(vec.get_size()) == int(deal_vec->size()));

  int size = vec.get_size();
  ValueType *val = NULL;

  vec.LeaveDataPtr(&val);

  for (int i=0; i<size; ++i)
    (*deal_vec)[i] = val[i];

  paralution::free_host(&val);
  
  //
  //  This is slower
  //
  //  for (unsigned int i=0; i<deal_vec->size(); ++i)
  //     (*deal_vec)[i] = vec[i];

}; 

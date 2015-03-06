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
#include "gpu_matrix_csr.hpp"
#include "gpu_matrix_coo.hpp"
#include "gpu_matrix_dia.hpp"
#include "gpu_matrix_ell.hpp"
#include "gpu_matrix_hyb.hpp"
#include "gpu_matrix_mcsr.hpp"
#include "gpu_matrix_bcsr.hpp"
#include "gpu_matrix_dense.hpp"
#include "gpu_vector.hpp"
#include "../host/host_matrix_dense.hpp"
#include "../base_matrix.hpp"
#include "../base_vector.hpp"
#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "gpu_utils.hpp"
#include "cuda_kernels_general.hpp"
#include "cuda_kernels_dense.hpp"
#include "gpu_allocate_free.hpp"
#include "../matrix_formats_ind.hpp"

#include <cuda.h>

namespace paralution {

template <typename ValueType>
GPUAcceleratorMatrixDENSE<ValueType>::GPUAcceleratorMatrixDENSE() {

  // no default constructors
  LOG_INFO("no default constructor");
  FATAL_ERROR(__FILE__, __LINE__);

}

template <typename ValueType>
GPUAcceleratorMatrixDENSE<ValueType>::GPUAcceleratorMatrixDENSE(const Paralution_Backend_Descriptor local_backend) {

  LOG_DEBUG(this, "GPUAcceleratorMatrixDENSE::GPUAcceleratorMatrixDENSE()",
            "constructor with local_backend");

  this->mat_.val = NULL;
  this->set_backend(local_backend); 

  CHECK_CUDA_ERROR(__FILE__, __LINE__);

}


template <typename ValueType>
GPUAcceleratorMatrixDENSE<ValueType>::~GPUAcceleratorMatrixDENSE() {

  LOG_DEBUG(this, "GPUAcceleratorMatrixDENSE::~GPUAcceleratorMatrixDENSE()",
            "destructor");

  this->Clear();

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::info(void) const {

  LOG_INFO("GPUAcceleratorMatrixDENSE<ValueType>");

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::AllocateDENSE(const int nrow, const int ncol) {

  assert( ncol  >= 0);
  assert( nrow  >= 0);

  if (this->get_nnz() > 0)
    this->Clear();

  if (nrow*ncol > 0) {

    allocate_gpu(nrow*ncol, &this->mat_.val);
    set_to_zero_gpu(this->local_backend_.GPU_block_size, 
                    this->local_backend_.GPU_max_threads,
                    nrow*ncol, mat_.val);   

    this->nrow_ = nrow;
    this->ncol_ = ncol;
    this->nnz_  = nrow*ncol;

  }


}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::Clear() {

  if (this->get_nnz() > 0) {

    free_gpu(&this->mat_.val);

    this->nrow_ = 0;
    this->ncol_ = 0;
    this->nnz_  = 0;

  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::SetDataPtrDENSE(ValueType **val, const int nrow, const int ncol) {

  assert(*val != NULL);
  assert(nrow > 0);
  assert(ncol > 0);

  this->Clear();

  cudaDeviceSynchronize();

  this->nrow_ = nrow;
  this->ncol_ = ncol;
  this->nnz_  = nrow*ncol;

  this->mat_.val = *val;

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::LeaveDataPtrDENSE(ValueType **val) {

  assert(this->nrow_ > 0);
  assert(this->ncol_ > 0);
  assert(this->nnz_ > 0);
  assert(this->nnz_  == this->nrow_*this->ncol_);

  cudaDeviceSynchronize();

  *val = this->mat_.val;

  this->mat_.val = NULL;

  this->nrow_ = 0;
  this->ncol_ = 0;
  this->nnz_  = 0;

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyFromHost(const HostMatrix<ValueType> &src) {

  const HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    if (this->get_nnz() > 0) {

      cudaMemcpy(this->mat_.val,     // dst
                 cast_mat->mat_.val, // src
                 this->get_nnz()*sizeof(ValueType), // size
                 cudaMemcpyHostToDevice);    
      CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyToHost(HostMatrix<ValueType> *dst) const {

  HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixDENSE<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateDENSE(this->get_nrow(), this->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    if (this->get_nnz() > 0) {
      
      cudaMemcpy(cast_mat->mat_.val, // dst
                 this->mat_.val,     // src
                 this->get_nnz()*sizeof(ValueType), // size
                 cudaMemcpyDeviceToHost);    
      CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyFrom(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixDENSE<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    if (this->get_nnz() > 0) { 

        cudaMemcpy(this->mat_.val,         // dst
                   gpu_cast_mat->mat_.val, // src
                   this->get_nnz()*sizeof(ValueType), // size
                   cudaMemcpyDeviceToDevice);    
        CHECK_CUDA_ERROR(__FILE__, __LINE__);     
      }

  } else {

    //CPU to GPU
    if ((host_cast_mat = dynamic_cast<const HostMatrix<ValueType>*> (&src)) != NULL) {
      
      this->CopyFromHost(*host_cast_mat);
      
    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyTo(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixDENSE<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixDENSE<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateDENSE(dst->get_nrow(), dst->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    if (this->get_nnz() > 0) {

        cudaMemcpy(gpu_cast_mat->mat_.val, // dst
                   this->mat_.val,         // src
                   this->get_nnz()*sizeof(ValueType), // size
                   cudaMemcpyDeviceToHost);    
        CHECK_CUDA_ERROR(__FILE__, __LINE__);     
      }
    
  } else {

    //GPU to CPU
    if ((host_cast_mat = dynamic_cast<HostMatrix<ValueType>*> (dst)) != NULL) {
      
      this->CopyToHost(host_cast_mat);

    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }


}


template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyFromHostAsync(const HostMatrix<ValueType> &src) {

  const HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // CPU to GPU copy
  if ((cast_mat = dynamic_cast<const HostMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    if (this->get_nnz() > 0) {

      cudaMemcpyAsync(this->mat_.val,     // dst
                      cast_mat->mat_.val, // src
                      this->get_nnz()*sizeof(ValueType), // size
                      cudaMemcpyHostToDevice);    
      CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    src.info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyToHostAsync(HostMatrix<ValueType> *dst) const {

  HostMatrixDENSE<ValueType> *cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to CPU copy
  if ((cast_mat = dynamic_cast<HostMatrixDENSE<ValueType>*> (dst)) != NULL) {

    cast_mat->set_backend(this->local_backend_);   

  if (dst->get_nnz() == 0)
    cast_mat->AllocateDENSE(this->get_nrow(), this->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    if (this->get_nnz() > 0) {
      
      cudaMemcpyAsync(cast_mat->mat_.val, // dst
                      this->mat_.val,     // src
                      this->get_nnz()*sizeof(ValueType), // size
                      cudaMemcpyDeviceToHost);    
      CHECK_CUDA_ERROR(__FILE__, __LINE__);     
    }
    
  } else {
    
    LOG_INFO("Error unsupported GPU matrix type");
    this->info();
    dst->info();
    FATAL_ERROR(__FILE__, __LINE__);
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyFromAsync(const BaseMatrix<ValueType> &src) {

  const GPUAcceleratorMatrixDENSE<ValueType> *gpu_cast_mat;
  const HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == src.get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<const GPUAcceleratorMatrixDENSE<ValueType>*> (&src)) != NULL) {
    
  if (this->get_nnz() == 0)
    this->AllocateDENSE(src.get_nrow(), src.get_ncol() );

    assert(this->get_nnz()  == src.get_nnz());
    assert(this->get_nrow() == src.get_nrow());
    assert(this->get_ncol() == src.get_ncol());

    if (this->get_nnz() > 0) { 

      cudaMemcpy(this->mat_.val,         // dst
                 gpu_cast_mat->mat_.val, // src
                 this->get_nnz()*sizeof(ValueType), // size
                 cudaMemcpyDeviceToDevice);    
        CHECK_CUDA_ERROR(__FILE__, __LINE__);     
      }

  } else {

    //CPU to GPU
    if ((host_cast_mat = dynamic_cast<const HostMatrix<ValueType>*> (&src)) != NULL) {
      
      this->CopyFromHostAsync(*host_cast_mat);
      
    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      src.info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }
    
  }

}

template <typename ValueType>
void GPUAcceleratorMatrixDENSE<ValueType>::CopyToAsync(BaseMatrix<ValueType> *dst) const {

  GPUAcceleratorMatrixDENSE<ValueType> *gpu_cast_mat;
  HostMatrix<ValueType> *host_cast_mat;

  // copy only in the same format
  assert(this->get_mat_format() == dst->get_mat_format());

  // GPU to GPU copy
  if ((gpu_cast_mat = dynamic_cast<GPUAcceleratorMatrixDENSE<ValueType>*> (dst)) != NULL) {

    gpu_cast_mat->set_backend(this->local_backend_);       

  if (this->get_nnz() == 0)
    gpu_cast_mat->AllocateDENSE(dst->get_nrow(), dst->get_ncol() );

    assert(this->get_nnz()  == dst->get_nnz());
    assert(this->get_nrow() == dst->get_nrow());
    assert(this->get_ncol() == dst->get_ncol());

    if (this->get_nnz() > 0) {

        cudaMemcpy(gpu_cast_mat->mat_.val, // dst
                   this->mat_.val,         // src
                   this->get_nnz()*sizeof(ValueType), // size
                   cudaMemcpyDeviceToHost);    
        CHECK_CUDA_ERROR(__FILE__, __LINE__);     
      }
    
  } else {

    //GPU to CPU
    if ((host_cast_mat = dynamic_cast<HostMatrix<ValueType>*> (dst)) != NULL) {
      
      this->CopyToHostAsync(host_cast_mat);

    } else {
      
      LOG_INFO("Error unsupported GPU matrix type");
      this->info();
      dst->info();
      FATAL_ERROR(__FILE__, __LINE__);
      
    }

  }


}


template <typename ValueType>
bool GPUAcceleratorMatrixDENSE<ValueType>::ConvertFrom(const BaseMatrix<ValueType> &mat) {

  this->Clear();

  // empty matrix is empty matrix
  if (mat.get_nnz() == 0)
    return true;

  const GPUAcceleratorMatrixDENSE<ValueType>   *cast_mat_dense;
  
  if ((cast_mat_dense = dynamic_cast<const GPUAcceleratorMatrixDENSE<ValueType>*> (&mat)) != NULL) {

      this->CopyFrom(*cast_mat_dense);
      return true;

  }

  /*
  const GPUAcceleratorMatrixCSR<ValueType>   *cast_mat_csr;
  if ((cast_mat_csr = dynamic_cast<const GPUAcceleratorMatrixCSR<ValueType>*> (&mat)) != NULL) {
    
    this->Clear();
    
    FATAL_ERROR(__FILE__, __LINE__);
    
    this->nrow_ = cast_mat_csr->get_nrow();
    this->ncol_ = cast_mat_csr->get_ncol();
    this->nnz_  = cast_mat_csr->get_nnz();
    
    return true;
    
  }
  */

  return false;

}

template <>
void GPUAcceleratorMatrixDENSE<double>::Apply(const BaseVector<double> &in, BaseVector<double> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());

    const GPUAcceleratorVector<double> *cast_in = dynamic_cast<const GPUAcceleratorVector<double>*> (&in);
    GPUAcceleratorVector<double> *cast_out      = dynamic_cast<      GPUAcceleratorVector<double>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    cublasStatus_t stat_t;

    const double alpha = double(1.0);
    const double beta  = double(0.0);

    if (DENSE_IND_BASE == 0) {

      stat_t = cublasDgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_N,
                           this->get_nrow(), this->get_ncol(),
                           &alpha,
                           this->mat_.val, this->get_nrow(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    } else {

      stat_t = cublasDgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_T,
                           this->get_ncol(), this->get_nrow(),
                           &alpha,
                           this->mat_.val, this->get_ncol(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    }

  }

}

template <>
void GPUAcceleratorMatrixDENSE<float>::Apply(const BaseVector<float> &in, BaseVector<float> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());

    const GPUAcceleratorVector<float> *cast_in = dynamic_cast<const GPUAcceleratorVector<float>*> (&in);
    GPUAcceleratorVector<float> *cast_out      = dynamic_cast<      GPUAcceleratorVector<float>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    cublasStatus_t stat_t;

    const float alpha = float(1.0);
    const float beta  = float(0.0);

    if (DENSE_IND_BASE == 0) {

      stat_t = cublasSgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_N,
                           this->get_nrow(), this->get_ncol(),
                           &alpha,
                           this->mat_.val, this->get_nrow(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    } else {

      stat_t = cublasSgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_T,
                           this->get_ncol(), this->get_nrow(),
                           &alpha,
                           this->mat_.val, this->get_ncol(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    }

  }

}

template <>
void GPUAcceleratorMatrixDENSE<double>::ApplyAdd(const BaseVector<double> &in, const double scalar,
                                                 BaseVector<double> *out) const {

  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());

    const GPUAcceleratorVector<double> *cast_in = dynamic_cast<const GPUAcceleratorVector<double>*> (&in);
    GPUAcceleratorVector<double> *cast_out      = dynamic_cast<      GPUAcceleratorVector<double>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    cublasStatus_t stat_t;

    const double alpha = scalar;
    const double beta  = double(0.0);

    if (DENSE_IND_BASE == 0) {

      stat_t = cublasDgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_N,
                           this->get_nrow(), this->get_ncol(),
                           &alpha,
                           this->mat_.val, this->get_nrow(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    } else {

      stat_t = cublasDgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_T,
                           this->get_ncol(), this->get_nrow(),
                           &alpha,
                           this->mat_.val, this->get_ncol(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    }

  }

}

template <>
void GPUAcceleratorMatrixDENSE<float>::ApplyAdd(const BaseVector<float> &in, const float scalar,
                                                BaseVector<float> *out) const {
FATAL_ERROR(__FILE__, __LINE__);
  if (this->get_nnz() > 0) {

    assert(in.  get_size() >= 0);
    assert(out->get_size() >= 0);
    assert(in.  get_size() == this->get_ncol());
    assert(out->get_size() == this->get_nrow());

    const GPUAcceleratorVector<float> *cast_in = dynamic_cast<const GPUAcceleratorVector<float>*> (&in);
    GPUAcceleratorVector<float> *cast_out      = dynamic_cast<      GPUAcceleratorVector<float>*> (out);

    assert(cast_in != NULL);
    assert(cast_out!= NULL);

    cublasStatus_t stat_t;

    const float alpha = scalar;
    const float beta  = float(0.0);

    if (DENSE_IND_BASE == 0) {

      stat_t = cublasSgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_N,
                           this->get_nrow(), this->get_ncol(),
                           &alpha,
                           this->mat_.val, this->get_nrow(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    } else {

      stat_t = cublasSgemv(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_T,
                           this->get_ncol(), this->get_nrow(),
                           &alpha,
                           this->mat_.val, this->get_ncol(),
                           cast_in->vec_, 1,
                           &beta, cast_out->vec_, 1);

      CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

    }

  }

}

template <>
bool GPUAcceleratorMatrixDENSE<float>::MatMatMult(const BaseMatrix<float> &A, const BaseMatrix<float> &B) {

  assert((this != &A) && (this != &B));
  assert(&A != NULL);
  assert(&B != NULL);

  const GPUAcceleratorMatrixDENSE<float> *cast_mat_A = dynamic_cast<const GPUAcceleratorMatrixDENSE<float>*> (&A);
  const GPUAcceleratorMatrixDENSE<float> *cast_mat_B = dynamic_cast<const GPUAcceleratorMatrixDENSE<float>*> (&B);

  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);
  assert(cast_mat_A->ncol_ == cast_mat_B->nrow_);

  cublasStatus_t stat_t;

  const float alpha = float(1.0);
  const float beta  = float(0.0);

  if (DENSE_IND_BASE == 0) {

    stat_t = cublasSgemm(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_N, CUBLAS_OP_N,
                         cast_mat_A->nrow_, cast_mat_B->ncol_, cast_mat_A->ncol_,
                         &alpha, cast_mat_A->mat_.val, cast_mat_A->nrow_,
                         cast_mat_B->mat_.val, cast_mat_A->ncol_, &beta,
                         this->mat_.val, cast_mat_A->nrow_);

    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  } else {

    stat_t = cublasSgemm(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_T, CUBLAS_OP_T,
                         cast_mat_A->nrow_, cast_mat_B->ncol_, cast_mat_A->ncol_,
                         &alpha, cast_mat_A->mat_.val, cast_mat_A->ncol_,
                         cast_mat_B->mat_.val, cast_mat_B->ncol_, &beta,
                         this->mat_.val, cast_mat_A->nrow_);

    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }

  return true;

}

template <>
bool GPUAcceleratorMatrixDENSE<double>::MatMatMult(const BaseMatrix<double> &A, const BaseMatrix<double> &B) {

  assert((this != &A) && (this != &B));
  assert(&A != NULL);
  assert(&B != NULL);

  const GPUAcceleratorMatrixDENSE<double> *cast_mat_A = dynamic_cast<const GPUAcceleratorMatrixDENSE<double>*> (&A);
  const GPUAcceleratorMatrixDENSE<double> *cast_mat_B = dynamic_cast<const GPUAcceleratorMatrixDENSE<double>*> (&B);

  assert(cast_mat_A != NULL);
  assert(cast_mat_B != NULL);
  assert(cast_mat_A->ncol_ == cast_mat_B->nrow_);

  cublasStatus_t stat_t;

  const double alpha = double(1.0);
  const double beta  = double(0.0);

  if (DENSE_IND_BASE == 0) {

    stat_t = cublasDgemm(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_N, CUBLAS_OP_N,
                         cast_mat_A->nrow_, cast_mat_B->ncol_, cast_mat_A->ncol_,
                         &alpha, cast_mat_A->mat_.val, cast_mat_A->nrow_,
                         cast_mat_B->mat_.val, cast_mat_A->ncol_, &beta,
                         this->mat_.val, cast_mat_A->nrow_);

    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  } else {

    stat_t = cublasDgemm(CUBLAS_HANDLE(this->local_backend_.GPU_cublas_handle), CUBLAS_OP_T, CUBLAS_OP_T,
                         cast_mat_A->nrow_, cast_mat_B->ncol_, cast_mat_A->ncol_,
                         &alpha, cast_mat_A->mat_.val, cast_mat_A->ncol_,
                         cast_mat_B->mat_.val, cast_mat_B->ncol_, &beta,
                         this->mat_.val, cast_mat_A->nrow_);

    CHECK_CUBLAS_ERROR(stat_t, __FILE__, __LINE__);

  }

  return true;

}

template <typename ValueType>
bool GPUAcceleratorMatrixDENSE<ValueType>::ReplaceColumnVector(const int idx, const BaseVector<ValueType> &vec) {

  assert(&vec != NULL);
  assert(vec.get_size() == this->get_nrow());

  if (this->get_nnz() > 0) {

    const GPUAcceleratorVector<ValueType> *cast_vec = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&vec);
    assert(cast_vec != NULL);

    const int nrow = this->get_nrow();
    const int ncol = this->get_ncol();

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);

    kernel_dense_replace_column_vector<ValueType, int> <<<GridSize, BlockSize>>>(cast_vec->vec_,
                                                                                 idx,
                                                                                 nrow,
                                                                                 ncol,
                                                                                 this->mat_.val);

    CHECK_CUDA_ERROR(__FILE__,__LINE__);

  }

  return true;

}

template <typename ValueType>
bool GPUAcceleratorMatrixDENSE<ValueType>::ReplaceRowVector(const int idx, const BaseVector<ValueType> &vec) {

  assert(&vec != NULL);
  assert(vec.get_size() == this->get_ncol());

  if (this->get_nnz() > 0) {

    const GPUAcceleratorVector<ValueType> *cast_vec = dynamic_cast<const GPUAcceleratorVector<ValueType>*> (&vec);
    assert(cast_vec != NULL);

    const int nrow = this->get_nrow();
    const int ncol = this->get_ncol();

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(ncol / this->local_backend_.GPU_block_size + 1);

    kernel_dense_replace_row_vector<ValueType, int> <<<GridSize, BlockSize>>>(cast_vec->vec_,
                                                                              idx,
                                                                              nrow,
                                                                              ncol,
                                                                              this->mat_.val);

    CHECK_CUDA_ERROR(__FILE__,__LINE__);

  }

  return true;

}

template <typename ValueType>
bool GPUAcceleratorMatrixDENSE<ValueType>::ExtractColumnVector(const int idx, BaseVector<ValueType> *vec) const {

    assert(vec != NULL);
    assert(vec->get_size() == this->get_nrow());

  if (this->get_nnz() > 0) {

    GPUAcceleratorVector<ValueType> *cast_vec = dynamic_cast<GPUAcceleratorVector<ValueType>*> (vec);
    assert(cast_vec != NULL);

    const int nrow = this->get_nrow();
    const int ncol = this->get_ncol();

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(nrow / this->local_backend_.GPU_block_size + 1);

    kernel_dense_extract_column_vector<ValueType, int> <<<GridSize, BlockSize>>>(cast_vec->vec_,
                                                                                 idx,
                                                                                 nrow,
                                                                                 ncol,
                                                                                 this->mat_.val);

    CHECK_CUDA_ERROR(__FILE__,__LINE__);

  }

  return true;

}

template <typename ValueType>
bool GPUAcceleratorMatrixDENSE<ValueType>::ExtractRowVector(const int idx, BaseVector<ValueType> *vec) const {

    assert(vec != NULL);
    assert(vec->get_size() == this->get_ncol());

  if (this->get_nnz() > 0) {

    GPUAcceleratorVector<ValueType> *cast_vec = dynamic_cast<GPUAcceleratorVector<ValueType>*> (vec);
    assert(cast_vec != NULL);

    const int nrow = this->get_nrow();
    const int ncol = this->get_ncol();

    dim3 BlockSize(this->local_backend_.GPU_block_size);
    dim3 GridSize(ncol / this->local_backend_.GPU_block_size + 1);

    kernel_dense_extract_row_vector<ValueType, int> <<<GridSize, BlockSize>>>(cast_vec->vec_,
                                                                              idx,
                                                                              nrow,
                                                                              ncol,
                                                                              this->mat_.val);

    CHECK_CUDA_ERROR(__FILE__,__LINE__);

  }

  return true;

}


template class GPUAcceleratorMatrixDENSE<double>;
template class GPUAcceleratorMatrixDENSE<float>;

}

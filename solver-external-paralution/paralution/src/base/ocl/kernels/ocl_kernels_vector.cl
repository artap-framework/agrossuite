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


__kernel void kernel_scale(const int size, const ValueType alpha, __global ValueType *x) {

  int gid = get_global_id(0);

  if (gid < size)
    x[gid] = alpha * x[gid];

}

__kernel void kernel_scaleadd(const int size, const ValueType alpha,
                              __global const ValueType *x, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] = alpha * out[gid] + x[gid];

}

__kernel void kernel_scaleaddscale(const int size, const ValueType alpha, const ValueType beta, 
                                   __global const ValueType *x, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] = alpha * out[gid] + beta * x[gid];

}

__kernel void kernel_scaleaddscale_offset(const int size, const int src_offset, const int dst_offset, 
                                          const ValueType alpha, const ValueType beta, 
                                          __global const ValueType *x, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid+dst_offset] = alpha * out[gid+dst_offset] + beta * x[gid+src_offset];

}

__kernel void kernel_scaleadd2(const int size, const ValueType alpha, const ValueType beta, const ValueType gamma,
                               __global const ValueType *x, __global const ValueType *y, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] = alpha * out[gid] + beta * x[gid] + gamma * y[gid];

}

__kernel void kernel_pointwisemult(const int size, __global const ValueType *x, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] = out[gid] * x[gid];

}

__kernel void kernel_pointwisemult2(const int size, __global const ValueType *x, __global const ValueType *y,
                                    __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] = y[gid] * x[gid];

}

__kernel void kernel_copy_offset_from(const int size, const int src_offset, const int dst_offset,
                                      __global const ValueType *in, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid+dst_offset] = in[gid+src_offset];

}

__kernel void kernel_permute(const int size, __global const int *permute,
                             __global const ValueType *in, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[permute[gid]] = in[gid];

}

__kernel void kernel_permute_backward(const int size, __global const int *permute,
                                      __global const ValueType *in, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] = in[permute[gid]];

}

__kernel void kernel_dot(const       int  size,
                         __global const ValueType *x, __global const ValueType *y,
                         __global       ValueType *out, __local        ValueType *sdata,
                         const       int  GROUP_SIZE, const       int  LOCAL_SIZE) {

    int tid = get_local_id(0);

    sdata[tid] = (ValueType)(0.0);

    int group_id = GROUP_SIZE * get_group_id(0);
    int gid = group_id + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size)
        sdata[tid] += x[gid] * y[gid];
      else
        i = LOCAL_SIZE;

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        sdata[tid] += sdata[tid + i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0)
      out[get_group_id(0)] = sdata[tid];

}

__kernel void kernel_dotc(const int size,
                          __global const ValueType *x, __global const ValueType *y,
                          __global ValueType *out, __local ValueType *sdata,
                          const int GROUP_SIZE, const int LOCAL_SIZE) {

    int tid = get_local_id(0);

    sdata[tid] = (ValueType)(0.0);

    int group_id = GROUP_SIZE * get_group_id(0);
    int gid = group_id + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size)
        sdata[tid] += x[gid] * y[gid];
      else
        i = LOCAL_SIZE;

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        sdata[tid] += sdata[tid + i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0)
      out[get_group_id(0)] = sdata[tid];

}

__kernel void kernel_norm(const int  size, __global const ValueType *x,
                         __global ValueType *out, __local ValueType *sdata,
                         const int GROUP_SIZE, const int  LOCAL_SIZE) {

    int tid = get_local_id(0);

    sdata[tid] = (ValueType)(0.0);

    int group_id = GROUP_SIZE * get_group_id(0);
    int gid = group_id + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size)
        sdata[tid] += x[gid] * x[gid];
      else
        i = LOCAL_SIZE;

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        sdata[tid] += sdata[tid + i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0)
      out[get_group_id(0)] = sdata[tid];

}

__kernel void kernel_axpy(const int size, const ValueType alpha,
                          __global const ValueType *x, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < size)
    out[gid] += alpha * x[gid];

}

__kernel void kernel_reduce(         const       int  size,
                            __global const ValueType *data,
                            __global       ValueType *out,
                            __local        ValueType *sdata,
                                     const       int  GROUP_SIZE,
                                     const       int  LOCAL_SIZE) {

    int tid = get_local_id(0);

    sdata[tid] = (ValueType)(0.0);

    int gid = GROUP_SIZE * get_group_id(0) + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size)
        sdata[tid] += data[gid];
      else
        i = LOCAL_SIZE;

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if ( tid < i )
        sdata[tid] += sdata[tid + i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0)
      out[get_group_id(0)] = sdata[tid];

}

__kernel void kernel_asum(         const       int  size,
                          __global const ValueType *data,
                          __global       ValueType *out,
                          __local        ValueType *sdata,
                                   const       int  GROUP_SIZE,
                                   const       int  LOCAL_SIZE) {

    int tid = get_local_id(0);

    sdata[tid] = (ValueType)(0.0);

    int gid = GROUP_SIZE * get_group_id(0) + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size)
        sdata[tid] += fabs(data[gid]);
      else
        i = LOCAL_SIZE;

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i)
        sdata[tid] += sdata[tid + i];

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0)
      out[get_group_id(0)] = sdata[tid];

}

__kernel void kernel_amax(         const       int  size,
                          __global const ValueType *data,
                          __global       ValueType *out,
                          __global             int *iout,
                          __local        ValueType *sdata,
                          __local              int *idata,
                                   const       int  GROUP_SIZE,
                                   const       int  LOCAL_SIZE) {

    int tid = get_local_id(0);

    sdata[tid] = (ValueType)(0.0);
    idata[tid] = 0;

    int gid = GROUP_SIZE * get_group_id(0) + tid;

    for (int i = 0; i < LOCAL_SIZE; ++i, gid += BLOCK_SIZE) {

      if (gid < size) {
        ValueType tmp = data[gid];
        if (fabs(tmp) > fabs(sdata[tid])) {
          sdata[tid] = fabs(tmp);
          idata[tid] = gid;
        }
      }

    }

    barrier(CLK_LOCAL_MEM_FENCE);

    for (int i = BLOCK_SIZE/2; i > 0; i /= 2) {

      if (tid < i) {
        ValueType tmp = sdata[tid+i];
        if (fabs(tmp) > fabs(sdata[tid])) {
          sdata[tid] = fabs(tmp);
          idata[tid] = idata[tid+i];
        }
      }

      barrier(CLK_LOCAL_MEM_FENCE);

    }

    if (tid == 0) {
      out[get_group_id(0)] = sdata[tid];
      iout[get_group_id(0)] = idata[tid];
    }

}

__kernel void kernel_power(const int n, const double power, __global ValueType *out) {

  int gid = get_global_id(0);

  if (gid < n)
    out[gid] = pow(out[gid], (ValueType)(power));

}

__kernel void kernel_copy_from_float(const int n, __global const float *in, __global ValueType *out) {

  int ind = get_global_id(0);

  if (ind < n)
    out[ind] = (ValueType)(in[ind]);

}

__kernel void kernel_copy_from_double(const int n, __global const double *in, __global ValueType *out) {

  int ind = get_global_id(0);

  if (ind < n)
    out[ind] = (ValueType)(in[ind]);

}

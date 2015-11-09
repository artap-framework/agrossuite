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
#include "mic_matrix_mcsr_kernel.hpp"
#include "mic_utils.hpp"
#include "../matrix_formats_ind.hpp"

namespace paralution {

template <typename ValueType>
void spmv_mcsr(const int mic_dev, 
	       const int *row, const int *col, const ValueType *val,
	       const int nrow,
	       const ValueType *in, ValueType *out) {
  

#pragma offload target(mic:mic_dev)			    \
  in(row:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(col:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(val:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(in:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(out:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for
  for (int ai=0; ai<nrow; ++ai) {
    out[ai] = val[ai] * in[ai];
    for (int aj=row[ai]; aj<row[ai+1]; ++aj)
      out[ai] += val[aj] * in[ col[aj] ];
  }

}

template <typename ValueType>
void spmv_add_mcsr(const int mic_dev, 
		   const int *row, const int *col, const ValueType *val,
		   const int nrow,
		   const ValueType scalar,
		   const ValueType *in, ValueType *out) {
  

#pragma offload target(mic:mic_dev)			    \
  in(row:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(col:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(val:length(0) MIC_REUSE MIC_RETAIN)	    \
  in(in:length(0) MIC_REUSE MIC_RETAIN)		    \
  in(out:length(0) MIC_REUSE MIC_RETAIN)	    
#pragma omp parallel for
  for (int ai=0; ai<nrow; ++ai) {
    out[ai] += scalar*val[ai] * in[ai];
    for (int aj=row[ai]; aj<row[ai+1]; ++aj)
      out[ai] += scalar*val[aj] * in[ col[aj] ];
  }

}

template void spmv_mcsr<double>(const int mic_dev, 
				   const int *row, const int *col, const double *val,
				   const int nrow,
				   const double *in, double *out);

template void spmv_mcsr<float>(const int mic_dev, 
				   const int *row, const int *col, const float *val,
				   const int nrow,
				   const float *in, float *out);

template void spmv_mcsr<int>(const int mic_dev, 
				   const int *row, const int *col, const int *val,
				   const int nrow,
				   const int *in, int *out);

template void spmv_add_mcsr<double>(const int mic_dev, 
				       const int *row, const int *col, const double *val,
				       const int nrow,
				       const double scalar,
				       const double *in, double *out);

template void spmv_add_mcsr<float>(const int mic_dev, 
				       const int *row, const int *col, const float *val,
				       const int nrow,
				       const float scalar,
				       const float *in, float *out);

template void spmv_add_mcsr<int>(const int mic_dev, 
				       const int *row, const int *col, const int *val,
				       const int nrow,
				       const int scalar,
				       const int *in, int *out);
}


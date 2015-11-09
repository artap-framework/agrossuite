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


#include <paralution.hpp>
#include "mex.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

  // Check output argument
  if (nlhs != 1) {
    mexErrMsgTxt("Expecting one output argument.");
    return;
  }

  // Check input arguments
  if (nrhs < 2) {
    mexErrMsgTxt("Expecting at least two input arguments.");
    return;
  }

  // Check if input matrix and rhs are double precision
  if (!mxIsDouble(prhs[0]) || !mxIsSparse(prhs[0])) {
    mexErrMsgTxt("Expecting sparse matrix in double precision.");
    return;
  }

  if (!mxIsDouble(prhs[1])) {
    mexErrMsgTxt("Expecting rhs in double precision.");
    return;
  }

  double tol     = 1e-6;
  mwSize maxIter = 10000;

  if (nrhs > 2) tol     = mxGetScalar(prhs[2]);
  if (nrhs > 3) maxIter = mxGetScalar(prhs[3]);

  // Get matrix arrays
  mwIndex *mw_row     = mxGetIr(prhs[0]);
  mwIndex *mw_col_ptr = mxGetJc(prhs[0]);
  double  *mw_val     = mxGetPr(prhs[0]);

  // Get matrix sizes
  int nrow = int(mxGetN(prhs[0]));
  int ncol = int(mxGetM(prhs[0]));
  int nnz  = int(mw_col_ptr[nrow]);

  int *row = NULL;
  int *col_ptr = NULL;
  double *val = NULL;
  paralution::allocate_host(nnz, &val);
  paralution::allocate_host(nnz, &row);
  paralution::allocate_host(nrow+1, &col_ptr);

  // copy needed due to diff types
  for (int i=0; i<nnz; ++i) {
    row[i] = mw_row[i];
    val[i] = mw_val[i];
  }

  for (int i=0; i<nrow+1; ++i)
    col_ptr[i] = mw_col_ptr[i];

  // Get rhs
  double  *mw_rhs = mxGetPr(prhs[1]);
  double  *rhs = NULL;

  paralution::allocate_host(nrow, &rhs);

  for (int i=0; i<nrow; ++i)
    rhs[i] = mw_rhs[i];

  // Allocate output vector
  plhs[0] = mxCreateDoubleMatrix(nrow, 1, mxREAL);
  double  *sol = mxGetPr(plhs[0]);

  /*
  // Time measurement
  double tick, tack;
  tick = paralution::paralution_time();
  */

  // Initialize PARALUTION
  paralution::init_paralution();

  // Create PARALUTION data structures
  paralution::LocalMatrix<double> A;
  paralution::LocalVector<double> b;
  paralution::LocalVector<double> x;

  // Fill PARALUTION data
  // For symmetric matrices CSC == CSR
  A.SetDataPtrCSR(&col_ptr, &row, &val, "A", nnz, ncol, nrow);
  b.SetDataPtr(&rhs, "b", ncol);
  x.Allocate("x", nrow);

  // Solver
  paralution::CG<paralution::LocalMatrix<double>, paralution::LocalVector<double>, double> ls;
  // Preconditioner
  //  paralution::MultiColoredILU<paralution::LocalMatrix<double>, paralution::LocalVector<double>, double> p;
  paralution::Jacobi<paralution::LocalMatrix<double>, paralution::LocalVector<double>, double> p;

  ls.SetOperator(A);
  ls.SetPreconditioner(p);
  ls.Init(0.0, tol, 1e8, maxIter);

  // Build solver and preconditioner
  ls.Build();

  ls.MoveToAccelerator();
  A.MoveToAccelerator();
  b.MoveToAccelerator();
  x.MoveToAccelerator();

  x.Zeros();

  // Disable PARALUTION printing
  //ls.Verbose(0);

  // Verbose printing
  ls.Verbose(1);
  A.info();

  /*
  tack = paralution::paralution_time();
  std::cout << "PARALUTION::Building time: " << (tack-tick)/1000000. << "sec.\n";
  tick = paralution::paralution_time();
  */

  // Solve Ax=b
  ls.Solve(b, &x);

  /*
  tack = paralution::paralution_time();
  std::cout << "PARALUTION::Solving time: " << (tack-tick)/1000000. << "sec.\n";
  */

  ls.Clear();
  A.Clear();
  b.Clear();

  double *ptr_sol = NULL;
  x.MoveToHost();
  x.LeaveDataPtr(&ptr_sol);

  for (int i=0; i<nrow; ++i)
    sol[i] = ptr_sol[i];

  paralution::free_host(&ptr_sol);

  paralution::stop_paralution();

  return;

}


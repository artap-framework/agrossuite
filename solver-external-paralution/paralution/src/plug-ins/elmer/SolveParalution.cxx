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

extern "C" {

  void paralution_solve(int n, int m, int nnz, const int *fortran_row_offset, const int *fortran_col,
                        const double *fortran_val, const double *fortran_rhs, double atol, double div,
                        int maxiter, double *fortran_x) {

    paralution::init_paralution();

    paralution::LocalVector<double> paralution_x;
    paralution::LocalVector<double> paralution_rhs;
    paralution::LocalMatrix<double> paralution_mat;

    int *row_offset = NULL;
    int *col        = NULL;
    double *val     = NULL;

    paralution::allocate_host(n+1, &row_offset);
    paralution::allocate_host(nnz, &col);
    paralution::allocate_host(nnz, &val);

    double *in_rhs = NULL;
    double *in_x   = NULL;
    paralution::allocate_host(n, &in_rhs);
    paralution::allocate_host(n, &in_x);

    for (int i=0; i<n; ++i) {
      in_rhs[i] = fortran_rhs[i];
      in_x[i]   = fortran_x[i];
    }

    paralution_rhs.SetDataPtr(&in_rhs, "Imported Fortran rhs", n);
    paralution_x.SetDataPtr(&in_x, "Imported Fortran x", n);

    // Copy matrix so we can convert it to any other format without breaking the fortran code
    // Shift since Fortran arrays start at 1
    for (int i=0; i<n+1; ++i)
      row_offset[i] = fortran_row_offset[i] - 1;

    for (int i=0; i<nnz; ++i) {
      col[i] = fortran_col[i] - 1;
      val[i] = fortran_val[i];
    }

    // Allocate paralution data structures
    paralution_mat.SetDataPtrCSR(&row_offset, &col, &val, "Imported Fortran CSR Matrix", nnz, n, m);

    paralution_mat.MoveToAccelerator();
    paralution_x.MoveToAccelerator();
    paralution_rhs.MoveToAccelerator();

    paralution::BiCGStab<paralution::LocalMatrix<double>, paralution::LocalVector<double>, double > ls;
    paralution::MultiColoredILU<paralution::LocalMatrix<double>, paralution::LocalVector<double>, double > pr;

    pr.Set(0);
    ls.SetPreconditioner(pr);
    ls.SetOperator(paralution_mat);
    ls.Init(atol, 0.0, div, maxiter);
    ls.Verbose(1);

    ls.Build();

    paralution_mat.info();

    ls.Solve(paralution_rhs, &paralution_x);


    paralution_x.MoveToHost();
    paralution_x.LeaveDataPtr(&in_x);

    for (int i=0; i<n; ++i)
      fortran_x[i] = in_x[i];

    ls.Clear();

    paralution::stop_paralution();

  }

}


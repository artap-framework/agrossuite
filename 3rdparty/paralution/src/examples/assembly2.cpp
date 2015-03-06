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


#include <iostream>
#include <cstdlib>

#include <paralution.hpp>

using namespace paralution;

int main(int argc, char* argv[]) {

  init_paralution();

  if (argc > 1)
    set_omp_threads_paralution(atoi(argv[1]));

  info_paralution();

  LocalVector<double> x;
  LocalVector<double> rhs;

  LocalMatrix<double> mat;

  int i[11] = {0, 1, 2, 3, 4, 5, 1, 2, 4, 3, 3 };
  int j[11] = {0, 1, 2, 3, 4, 5, 2, 2, 5, 2, 2 };
  double a[11] = {2.3, 3.5, 4.7, 6.3, 0.4, 4.3, 6.2, 4.4, 4.6, 0.7, 4.8 };

  mat.Assemble(i, j, a, 11, "A");
  rhs.Assemble(i, a, 11, "rhs");
  x.Allocate("x", mat.get_ncol());

  mat.MoveToAccelerator();
  rhs.MoveToAccelerator();
  x.MoveToAccelerator();

  rhs.info();
  mat.info();

  // Linear Solver
  GMRES<LocalMatrix<double>, LocalVector<double>, double > ls;

  ls.SetOperator(mat);
  ls.Build();

  for (int k=0; k<10; k++) {

    mat.Zeros();
    x.Zeros();
    rhs.Zeros();

    // Modify the assembling data
    a[4] = k*k*3.3;
    a[8] = k*k-1.3;
    a[10] = k*k/2.0;
    a[3] = k;

    mat.AssembleUpdate(a);
    rhs.Assemble(i, a, 11, "rhs");

    ls.ResetOperator(mat);
    ls.Solve(rhs, &x);
  }

  x.MoveToHost();

  ls.Clear();

  stop_paralution();

  return 0;
}

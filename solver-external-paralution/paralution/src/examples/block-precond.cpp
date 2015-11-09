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

  if (argc == 1) { 
    std::cerr << argv[0] << " <matrix> [Num threads]" << std::endl;
    exit(1);
  }

  init_paralution();

  if (argc > 2) {
    set_omp_threads_paralution(atoi(argv[2]));
  } 

  info_paralution();

  LocalVector<double> x;
  LocalVector<double> rhs;

  LocalMatrix<double> mat;

  mat.ReadFileMTX(std::string(argv[1]));

  //  mat.MoveToAccelerator();
  //  x.MoveToAccelerator();
  //  rhs.MoveToAccelerator();

  x.Allocate("x", mat.get_nrow());
  rhs.Allocate("rhs", mat.get_nrow());

  // Linear Solver
  GMRES<LocalMatrix<double>, LocalVector<double>, double > ls;

  // Preconditioner
  BlockPreconditioner<LocalMatrix<double>, LocalVector<double>, double > p;
  Solver<LocalMatrix<double>, LocalVector<double>, double > **p2;

  int n = 2;
  int *size;
  size = new int[n];

  p2 = new Solver<LocalMatrix<double>, LocalVector<double>, double >*[n];

  for (int i=0; i<n; ++i) {
    size[i] = mat.get_nrow() / n ;   

    MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double > *mc;
    mc = new MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double >;
    p2[i] = mc;


    //    AMG<LocalMatrix<double>, LocalVector<double>, double > *amg;
    //    amg = new AMG<LocalMatrix<double>, LocalVector<double>, double >;
    //    amg->InitMaxIter(2);
    //    amg->Verbose(0);
    //    p2[i] = amg;

  }

  double tick, tack;
  
  rhs.Ones();
  x.Zeros(); 

  p.Set(n, 
        size,
        p2);
  p.SetDiagonalSolver();

  ls.SetOperator(mat);
  ls.SetPreconditioner(p);
  //  ls.Verbose(2);

  ls.Build();

  mat.info();

  tick = paralution_time();

  ls.Solve(rhs, &x);

  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  ls.Clear();

  for (int i=0; i<n; ++i) {
    delete p2[i];
    p2[i] = NULL;
  }

  delete[] size;
  delete[] p2;
  p2 = NULL;

  stop_paralution();

  return 0;
}

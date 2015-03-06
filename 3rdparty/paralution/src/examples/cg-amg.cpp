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

  double tick, tack, start, end;

  start = paralution_time();

  if (argc == 1) { 
    std::cerr << argv[0] << " <matrix> [Num threads]" << std::endl;
    exit(1);
  }

  init_paralution();

  if (argc > 2)
    set_omp_threads_paralution(atoi(argv[2]));

  info_paralution();

  LocalVector<double> x;
  LocalVector<double> rhs;

  LocalMatrix<double> mat;

  mat.ReadFileMTX(std::string(argv[1]));

  x.Allocate("x", mat.get_nrow());
  rhs.Allocate("rhs", mat.get_nrow());

  rhs.Ones();
  x.Zeros(); 

  tick = paralution_time();

  CG<LocalMatrix<double>, LocalVector<double>, double > ls;
  ls.Verbose(0);

  // AMG Preconditioner
  AMG<LocalMatrix<double>, LocalVector<double>, double > p;

  p.InitMaxIter(1);
  p.Verbose(0);

  ls.SetPreconditioner(p);
  ls.SetOperator(mat);
  ls.Build();

  tack = paralution_time();
  std::cout << "Building time:" << (tack-tick)/1000000 << " sec" << std::endl;

  // move after building since AMG building is not supported on GPU yet
  mat.MoveToAccelerator();
  x.MoveToAccelerator();
  rhs.MoveToAccelerator();
  ls.MoveToAccelerator();

  mat.info();

  tick = paralution_time();

  ls.Init(1e-10, 1e-8, 1e+8, 10000);
  ls.Verbose(2);

  ls.Solve(rhs, &x);

  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  ls.Clear();

  stop_paralution();

  end = paralution_time();
  std::cout << "Total runtime:" << (end-start)/1000000 << " sec" << std::endl;

  return 0;
}

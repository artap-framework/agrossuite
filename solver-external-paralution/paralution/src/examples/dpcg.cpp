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
    std::cerr << argv[0] << " <matrix> <initial_guess> <rhs> [Num threads]" << std::endl;
    exit(1);
  }

  init_paralution();

  if (argc > 4) {
    set_omp_threads_paralution(atoi(argv[4]));
  } 

  info_paralution();

  double tick, tack;

  LocalVector<double> x;
  LocalVector<double> rhs;
  LocalMatrix<double> mat;

  mat.ReadFileMTX(std::string(argv[1]));
  rhs.ReadFileASCII(std::string(argv[3]));
  x.ReadFileASCII(std::string(argv[2]));
  //  x.Zeros(); 

  // Linear Solver
  DPCG<LocalMatrix<double>, LocalVector<double>, double > ls;


  ls.SetOperator(mat);
  ls.Init(1e-6, 0.0, 1e8, 20000);

  ls.SetNVectors(2);

  // Uncomment for GPU
  mat.MoveToAccelerator();
  x.MoveToAccelerator();
  rhs.MoveToAccelerator();

  tick = paralution_time();

  ls.Build();

  tack = paralution_time();
  std::cout << "Building:" << (tack-tick)/1000000 << " sec" << std::endl;
  
  ls.Verbose(2);

  mat.info();

  tick = paralution_time();

  ls.Solve(rhs, &x);

  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  x.MoveToHost();
  x.WriteFileASCII("x_solution.dat");

  ls.Clear();

  stop_paralution();

  return 0;
}

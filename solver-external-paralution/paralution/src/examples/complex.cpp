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


#if defined(SUPPORT_CUDA) || defined(SUPPORT_OCL) || defined(SUPPORT_MIC)
#undef SUPPORT_COMPLEX
#else
#define SUPPORT_COMPLEX
#endif

#include <iostream>
#include <cstdlib>
#include <complex>

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

#ifdef SUPPORT_COMPLEX

  LocalVector<std::complex<double> > x;
  LocalVector<std::complex<double> > rhs;

  LocalMatrix<std::complex<double> > mat;

  mat.ReadFileMTX(std::string(argv[1]));

  mat.MoveToAccelerator();
  x.MoveToAccelerator();
  rhs.MoveToAccelerator();

  x.Allocate("x", mat.get_nrow());
  rhs.Allocate("rhs", mat.get_nrow());

  // Linear Solver
  CG<LocalMatrix<std::complex<double> >, LocalVector<std::complex<double> >, std::complex<double> > ls;

  // Preconditioner
  MultiColoredILU<LocalMatrix<std::complex<double> >, LocalVector<std::complex<double> >, std::complex<double> > p;

  rhs.Ones();
  x.Zeros(); 

  ls.SetOperator(mat);
  ls.SetPreconditioner(p);

  ls.Build();

//  ls.Verbose(2);

  mat.info();

  double tick, tack;
  tick = paralution_time();

  ls.Solve(rhs, &x);

  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  ls.Clear();

#else

  std::cout << "The basic version does not support complex on CUDA/OpenCL/MIC" << std::endl;

#endif

  stop_paralution();

  return 0;

}

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

  LocalVector<double> b, b_old, *b_k, *b_k1, *b_tmp;
  LocalMatrix<double> mat;

  mat.ReadFileMTX(std::string(argv[1]));

  // Gershgorin spectrum approximation
  double glambda_min, glambda_max;

  // Power method spectrum approximation
  double plambda_min, plambda_max;

  // Maximum number of iteration for the power method
  int iter_max = 10000;

  double tick, tack;

  // Gershgorin approximation of the eigenvalues
  mat.Gershgorin(glambda_min, glambda_max);
  std::cout << "Gershgorin : Lambda min = " << glambda_min
            << "; Lambda max = " << glambda_max << std::endl;


  mat.MoveToAccelerator();
  b.MoveToAccelerator();
  b_old.MoveToAccelerator();


  b.Allocate("b_k+1", mat.get_nrow());
  b_k1 = &b;

  b_old.Allocate("b_k", mat.get_nrow());
  b_k = &b_old;  

  b_k->Ones();

  mat.info();

  tick = paralution_time();

  // compute lambda max
  for (int i=0; i<=iter_max; ++i) {

    mat.Apply(*b_k, b_k1);

    //    std::cout << b_k1->Dot(*b_k) << std::endl;
    b_k1->Scale(double(1.0)/b_k1->Norm());

    b_tmp = b_k1;
    b_k1 = b_k;
    b_k = b_tmp;

  }

  // get lambda max (Rayleigh quotient)
  mat.Apply(*b_k, b_k1);
  plambda_max = b_k1->Dot(*b_k) ;

  tack = paralution_time();
  std::cout << "Power method (lambda max) execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  mat.AddScalarDiagonal(double(-1.0)*plambda_max);


  b_k->Ones();

  tick = paralution_time();

  // compute lambda min
  for (int i=0; i<=iter_max; ++i) {

    mat.Apply(*b_k, b_k1);

    //    std::cout << b_k1->Dot(*b_k) + plambda_max << std::endl;
    b_k1->Scale(double(1.0)/b_k1->Norm());

    b_tmp = b_k1;
    b_k1 = b_k;
    b_k = b_tmp;

  }

  // get lambda min (Rayleigh quotient)
  mat.Apply(*b_k, b_k1);
  plambda_min = (b_k1->Dot(*b_k) + plambda_max);

  // back to the original matrix
  mat.AddScalarDiagonal(plambda_max);

  tack = paralution_time();
  std::cout << "Power method (lambda min) execution:" << (tack-tick)/1000000 << " sec" << std::endl;


  std::cout << "Power method Lambda min = " << plambda_min
            << "; Lambda max = " << plambda_max 
            << "; iter=2x" << iter_max << std::endl;

  LocalVector<double> x;
  LocalVector<double> rhs;

  x.CloneBackend(mat);
  rhs.CloneBackend(mat);

  x.Allocate("x", mat.get_nrow());
  rhs.Allocate("rhs", mat.get_nrow());

  // Chebyshev iteration
  Chebyshev<LocalMatrix<double>, LocalVector<double>, double > ls;

  rhs.Ones();
  x.Zeros(); 

  ls.SetOperator(mat);

  ls.Set(plambda_min, plambda_max);

  ls.Build();

  tick = paralution_time();

  ls.Solve(rhs, &x);

  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  // PCG + Chebyshev polynomial
  CG<LocalMatrix<double>, LocalVector<double>, double > cg;
  AIChebyshev<LocalMatrix<double>, LocalVector<double>, double > p;

  // damping factor
  plambda_min = plambda_max / 7;
  p.Set(3, plambda_min, plambda_max);
  rhs.Ones();
  x.Zeros(); 

  cg.SetOperator(mat);
  cg.SetPreconditioner(p);

  cg.Build();

  tick = paralution_time();

  cg.Solve(rhs, &x);

  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

  stop_paralution();

  return 0;
}

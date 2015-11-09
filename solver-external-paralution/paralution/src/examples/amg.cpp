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
  start = paralution_time();

  // Linear Solver
  AMG<LocalMatrix<double>, LocalVector<double>, double > ls;

  ls.SetOperator(mat);

  // coupling strength
  ls.SetCouplingStrength(0.001);
  // number of unknowns on coarsest level
  ls.SetCoarsestLevel(300);
  // interpolation type for grid transfer operators
  ls.SetInterpolation(SmoothedAggregation);
  // Relaxation parameter for smoothed interpolation aggregation
  ls.SetInterpRelax(2./3.);
  // Manual smoothers
  ls.SetManualSmoothers(true);
  // Manual course grid solver
  ls.SetManualSolver(true);
  // grid transfer scaling
  ls.SetScaling(true);

  ls.BuildHierarchy();

  int levels = ls.GetNumLevels();

  // Smoother for each level
  IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double > **sm = NULL;
  MultiColoredGS<LocalMatrix<double>, LocalVector<double>, double > **gs = NULL;

  // Coarse Grid Solver
  CG<LocalMatrix<double>, LocalVector<double>, double > cgs;
  cgs.Verbose(0);

  sm = new IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double >*[levels-1];
  gs = new MultiColoredGS<LocalMatrix<double>, LocalVector<double>, double >*[levels-1];

  // Preconditioner
  //  MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double > p;
  //  cgs->SetPreconditioner(p);

  for (int i=0; i<levels-1; ++i) {
    FixedPoint<LocalMatrix<double>, LocalVector<double>, double > *fp;
    fp = new FixedPoint<LocalMatrix<double>, LocalVector<double>, double >;
    fp->SetRelaxation(1.3);
    sm[i] = fp;
    
    gs[i] = new MultiColoredGS<LocalMatrix<double>, LocalVector<double>, double >;
    gs[i]->SetPrecondMatrixFormat(ELL);
    
    sm[i]->SetPreconditioner(*gs[i]);
    sm[i]->Verbose(0);
  }

  ls.SetOperatorFormat(CSR);
  ls.SetSmoother(sm);
  ls.SetSolver(cgs);
  ls.SetSmootherPreIter(1);
  ls.SetSmootherPostIter(2);
  ls.Init(1e-10, 1e-8, 1e+8, 10000);
  ls.Verbose(2);
    
  ls.Build();
  
  mat.MoveToAccelerator();
  x.MoveToAccelerator();
  rhs.MoveToAccelerator();
  ls.MoveToAccelerator();

  mat.info();
  
  tack = paralution_time();
  std::cout << "Building time:" << (tack-tick)/1000000 << " sec" << std::endl;
  
  tick = paralution_time();
  
  ls.Solve(rhs, &x);
  
  tack = paralution_time();
  std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;
  
  ls.Clear();

  // Free all allocated data
  for (int i=0; i<levels-1; ++i) {
    delete gs[i];
    delete sm[i];
  }
  delete[] gs;
  delete[] sm;

  ls.Clear();

  stop_paralution();

  end = paralution_time();
  std::cout << "Total runtime:" << (end-start)/1000000 << " sec" << std::endl;

  return 0;
}

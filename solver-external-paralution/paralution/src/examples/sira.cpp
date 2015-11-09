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
#include <iostream>

using namespace paralution;

int main(int argc, char** argv){
    
    if (argc == 1) { 
      std::cerr << argv[0] << " <matrix> [Num threads]" << std::endl;
      exit(1);
    }

    init_paralution();

    if (argc > 2) {
      set_omp_threads_paralution(atoi(argv[2]));
    } 

    info_paralution();

    LocalMatrix<double> mat;
    LocalVector<double> eigenvalues;
  
    mat.ReadFileMTX( std::string(argv[1]) );
    mat.MoveToAccelerator();

    // Use a special starting vector
    //
    //    LocalVector<double> vec;
    //    vec.Allocate("initial vec", mat.get_nrow());
    //    for (int i=0; i<mat.get_nrow(); i++)
    //        vec[i] = i+1;
  
    // Mixed precision SIRA solver
    SIRA< LocalMatrix<double>, LocalVector<double>, double, 
          LocalMatrix<float>,  LocalVector<float>,  float > sira;

    // Set preconditioner, the default preconditioner is FSAI level 1
    MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double > p1;
    MultiColoredILU<LocalMatrix<float>,  LocalVector<float>,  float >  p2;
    sira.SetInnerPreconditioner(p1, p2);
    
    // Fixed precision SIRA with constant inner tolerance 
    // SIRA< LocalMatrix<double>, LocalVector<double>, double > sira;
    //  MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double > p1;
    // sira.SetInnerPreconditioner(p1);

    sira.SetOperator(mat);
    sira.SetNumberOfEigenvalues(3);
    
    sira.Build();
    
    double tick, tack;
    tick = paralution_time();

    sira.Solve(&eigenvalues);

    // use vec as a starting vector
    // sira.Solve(vec, &eigenvalues);

    tack = paralution_time();

    for (int i=0; i<eigenvalues.get_size(); ++i)
      std::cout << "Eigenvalue " << i << " = " << eigenvalues[i] << std::endl;
    
    std::cout << "Solver execution:" << (tack-tick)/1000000 << " sec" << std::endl;

    sira.Clear();


    stop_paralution();
    
    return 0;

}

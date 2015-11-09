 // *************************************************************************
//
//    Modification of file paralution_AMG.C is made by the PARALUTION team,
//    this file is a part of the PARALUTION library
//                                          
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2012-2013 Dimitar Lukarski
//
//    GPLv3 
//
// *************************************************************************
//
// The origin OpenFOAM license:
//
/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "paralution.hpp"
#include "../paralution/paralution_openfoam.H"
#include "paralution_AMG.H"


// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(paralution_AMG, 0);

    lduMatrix::solver::addsymMatrixConstructorToTable<paralution_AMG>
        addparalution_AMGSymMatrixConstructorToTable_;

    lduMatrix::solver::addasymMatrixConstructorToTable<paralution_AMG>
        addparalution_AMGAsymMatrixConstructorToTable_;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::paralution_AMG::paralution_AMG
(
    const word& fieldName,
    const lduMatrix& matrix,
    const FieldField<Field, scalar>& interfaceBouCoeffs,
    const FieldField<Field, scalar>& interfaceIntCoeffs,
    const lduInterfaceFieldPtrsList& interfaces,
    const dictionary& solverControls
)
:
    lduMatrix::solver
    (
        fieldName,
        matrix,
        interfaceBouCoeffs,
        interfaceIntCoeffs,
        interfaces,
        solverControls
    )
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::solverPerformance Foam::paralution_AMG::solve
(
    scalarField& psi,
    const scalarField& source,
    const direction cmpt
) const
{

    word precond_name = lduMatrix::preconditioner::getName(controlDict_);
    double div   = controlDict_.lookupOrDefault<double>("div", 1e+08);
    bool accel   = controlDict_.lookupOrDefault<bool>("useAccelerator", true);
    word mformat = controlDict_.lookupOrDefault<word>("MatrixFormat", "CSR");
    word pformat = controlDict_.lookupOrDefault<word>("PrecondFormat", "CSR");
    word sformat = controlDict_.lookupOrDefault<word>("SmootherFormat", "CSR");
    word solver_name = controlDict_.lookupOrDefault<word>("CoarseGridSolver", "CG");
    word smoother_name = controlDict_.lookupOrDefault<word>("smoother", "paralution_MultiColoredGS");
    int MEp      = controlDict_.lookupOrDefault<int>("MEp", 1);
    word LBPre   = controlDict_.lookupOrDefault<word>("LastBlockPrecond", "paralution_Jacobi");
    int iterPreSmooth = controlDict_.lookupOrDefault<int>("nPreSweeps", 1);
    int iterPostSmooth = controlDict_.lookupOrDefault<int>("nPostSweeps", 2);
    double epsCoupling = controlDict_.lookupOrDefault<double>("couplingStrength", 0.01);
    int coarsestCells = controlDict_.lookupOrDefault<int>("nCellsInCoarsestLevel", 300);
    int ILUp = controlDict_.lookupOrDefault<int>("ILUp", 0);
    int ILUq = controlDict_.lookupOrDefault<int>("ILUq", 1);
    double relax = controlDict_.lookupOrDefault<double>("Relaxation", 1.0);
    double aggrrelax = controlDict_.lookupOrDefault<double>("AggrRelax", 2./3.);
    bool scaling = controlDict_.lookupOrDefault<bool>("scaleCorrection", true);
    word interp_name = controlDict_.lookupOrDefault<word>("InterpolationType", "SmoothedAggregation");

    solverPerformance solverPerf(typeName + '(' + precond_name + ')', fieldName_);

    register label nCells = psi.size();

    scalarField pA(nCells);
    scalarField wA(nCells);

    // --- Calculate A.psi
    matrix_.Amul(wA, psi, interfaceBouCoeffs_, interfaces_, cmpt);

    // --- Calculate initial residual field
    scalarField rA(source - wA);

    // --- Calculate normalisation factor
    scalar normFactor = this->normFactor(psi, source, wA, pA);

    // --- Calculate normalised residual norm
    solverPerf.initialResidual() = gSumMag(rA)/normFactor;
    solverPerf.finalResidual() = solverPerf.initialResidual();

    if ( !solverPerf.checkConvergence(tolerance_, relTol_) ) {

      paralution::_matrix_format mf = paralution::CSR;
      if      (mformat == "CSR")   mf = paralution::CSR;
      else if (mformat == "DIA")   mf = paralution::DIA;
      else if (mformat == "HYB")   mf = paralution::HYB;
      else if (mformat == "ELL")   mf = paralution::ELL;
      else if (mformat == "MCSR")  mf = paralution::MCSR;
      else if (mformat == "BCSR")  mf = paralution::BCSR;
      else if (mformat == "COO")   mf = paralution::COO;
      else if (mformat == "DENSE") mf = paralution::DENSE;

      paralution::_interp ip = paralution::SmoothedAggregation;
      if      (interp_name == "SmoothedAggregation") ip = paralution::SmoothedAggregation;
      else if (interp_name == "Aggregation")         ip = paralution::Aggregation;

      paralution::LocalVector<double> x;
      paralution::LocalVector<double> rhs;
      paralution::LocalMatrix<double> mat;

      paralution::AMG<paralution::LocalMatrix<double>,
                      paralution::LocalVector<double>,
                      double> ls;

      paralution::import_openfoam_matrix(matrix(), &mat);
      paralution::import_openfoam_vector(source, &rhs);
      paralution::import_openfoam_vector(psi, &x);

      ls.SetOperator(mat);

      // coupling strength
      ls.SetCouplingStrength(epsCoupling);
      // number of unknowns on coarsest level
      ls.SetCoarsestLevel(coarsestCells);
      // interpolation type for grid transfer operators
      ls.SetInterpolation(ip);
      // Relaxation parameter for smoothed interpolation aggregation
      ls.SetInterpRelax(aggrrelax);
      // Manual smoothers
      ls.SetManualSmoothers(true);
      // Manual course grid solver
      ls.SetManualSolver(true);
      // grid transfer scaling
      ls.SetScaling(scaling);
      // operator format
      ls.SetOperatorFormat(mf);
      ls.SetSmootherPreIter(iterPreSmooth);
      ls.SetSmootherPostIter(iterPostSmooth);

      ls.BuildHierarchy();

      int levels = ls.GetNumLevels();

      // Smoother via preconditioned FixedPoint iteration
      paralution::IterativeLinearSolver<paralution::LocalMatrix<double>,
                                        paralution::LocalVector<double>,
                                        double > **fp = NULL;
      fp = new paralution::IterativeLinearSolver<paralution::LocalMatrix<double>,
                                                 paralution::LocalVector<double>,
                                                 double >*[levels-1];
      paralution::Preconditioner<paralution::LocalMatrix<double>,
                                 paralution::LocalVector<double>,
                                 double > **sm = NULL;
      sm = new paralution::Preconditioner<paralution::LocalMatrix<double>,
                                          paralution::LocalVector<double>,
                                          double >*[levels-1];

      for (int i=0; i<levels-1; ++i) {
        fp[i] = paralution::GetIterativeLinearSolver<double>("paralution_FixedPoint", relax);
        sm[i] = paralution::GetPreconditioner<double>(smoother_name, LBPre, sformat, ILUp, ILUq, MEp);
        fp[i]->SetPreconditioner(*sm[i]);
        fp[i]->Verbose(0);
      }

      // Coarse Grid Solver and its Preconditioner
      paralution::IterativeLinearSolver<paralution::LocalMatrix<double>,
                                        paralution::LocalVector<double>,
                                        double > *cgs = NULL;
      cgs = paralution::GetIterativeLinearSolver<double>(solver_name, relax);
      cgs->Verbose(0);
      paralution::Preconditioner<paralution::LocalMatrix<double>,
                                 paralution::LocalVector<double>,
                                 double > *cgp = NULL;
      cgp = paralution::GetPreconditioner<double>(precond_name, LBPre, pformat, ILUp, ILUq, MEp);
      if (cgp != NULL) cgs->SetPreconditioner(*cgp);

      ls.SetSmoother(fp);
      ls.SetSolver(*cgs);

      // Switch to L1 norm to be consistent with OpenFOAM solvers
      ls.SetResidualNorm(1);

      ls.Init(tolerance_*normFactor, // abs
              relTol_,    // rel
              div,        // div
              maxIter_);  // max iter

      ls.Build();

      if (accel) {
        mat.MoveToAccelerator();
        rhs.MoveToAccelerator();
        x.MoveToAccelerator();
        ls.MoveToAccelerator();
      }

      switch(mf) {
        case paralution::DENSE:
          mat.ConvertToDENSE();
          break;
        case paralution::CSR:
          mat.ConvertToCSR();
          break;
        case paralution::MCSR:
          mat.ConvertToMCSR();
          break;
        case paralution::BCSR:
          mat.ConvertToBCSR();
          break;
        case paralution::COO:
          mat.ConvertToCOO();
          break;
        case paralution::DIA:
          mat.ConvertToDIA();
          break;
        case paralution::ELL:
          mat.ConvertToELL();
          break;
        case paralution::HYB:
          mat.ConvertToHYB();
          break;
      }

      ls.Verbose(0);

      // Solve linear system
      ls.Solve(rhs, &x);

      paralution::export_openfoam_vector(x, &psi);

      solverPerf.finalResidual()   = ls.GetCurrentResidual() / normFactor; // divide by normFactor, see lduMatrixSolver.C
      solverPerf.nIterations()     = ls.GetIterationCount();
      solverPerf.checkConvergence(tolerance_, relTol_);

      // Clear MultiGrid object
      ls.Clear();

      // Free all structures
      for (int i=0; i<levels-1; ++i) {
        delete fp[i];
        delete sm[i];
      }
      cgs->Clear();

      if (cgp != NULL)
        delete cgp;

      delete[] fp;
      delete[] sm;
      delete cgs;

    }

    return solverPerf;

}


// ************************************************************************* //

// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more detaiamg->
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include <streambuf>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

#include "paralution.hpp"

#include "../3rdparty/tclap/CmdLine.h"
#include "../util/sparse_io.h"

// typedef float ScalarType;
typedef double ScalarType;

using namespace paralution;

class LinearSystemParalutionArgs : public LinearSystemArgs
{
public:
    LinearSystemParalutionArgs(const std::string &name, int argc, const char * const *argv)
        : LinearSystemArgs(name, argc, argv),
          preconditionerArg(TCLAP::ValueArg<std::string>("c", "preconditioner", "Preconditioner", false, "", "string")),
          solverArg(TCLAP::ValueArg<std::string>("l", "solver", "Solver", false, "", "string")),
          absTolArg(TCLAP::ValueArg<double>("a", "abs_tol", "Absolute tolerance", false, 1e-13, "double")),
          relTolArg(TCLAP::ValueArg<double>("t", "rel_tol", "Relative tolerance", false, 1e-9, "double")),
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int")),
          multigridArg(TCLAP::SwitchArg("g", "multigrid", "Algebraic multigrid", false))
    {
        cmd.add(preconditionerArg);
        cmd.add(solverArg);
        cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
        cmd.add(multigridArg);
    }

public:
    TCLAP::ValueArg<std::string> preconditionerArg;
    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<double> absTolArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
    TCLAP::SwitchArg multigridArg;
};

Preconditioner<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType> *getPrecoditioner(const std::string &preconditioner)
{
    Preconditioner<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType> *p = nullptr;
    if (preconditioner == "Jacobi")
        p = new Jacobi<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "SGS")
        p = new SGS<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "MultiColoredGS")
        p = new MultiColoredGS<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "MultiColoredSGS")
        p = new MultiColoredSGS<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "ILU")
        p = new ILU<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "MultiColoredILU")
        p = new MultiColoredILU<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "MultiElimination")
        p = new MultiElimination<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (preconditioner == "FSAI")
        p = new FSAI<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else
        std::cerr << "Preconditioner '" << preconditioner << "' not found." << std::endl;
    assert(p);

    return p;
}

IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *getKrylovSubspaceSolver(const std::string &solver)
{
    IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ils = nullptr;

    if (solver == "BiCGStab")
        ils = new BiCGStab<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "CG")
        ils = new CG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "CR")
        ils = new CR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "GMRES")
        ils = new GMRES<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "FGMRES")
        ils = new FGMRES<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "CR")
        ils = new CR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "IDR")
        ils = new IDR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else
        std::cerr << "Solver '" << solver << "' not found." << std::endl;
    assert(ils);

    return ils;
}

bool isKrylovSubspaceSolver(const std::string &solver)
{
    return (solver == "BiCGStab" || solver == "CG" || solver == "CR" || solver == "GMRES" || solver == "FGMRES" || solver == "CR" || solver == "IDR");
}

Solver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *getDirectSolver(const std::string &solver)
{
    Solver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ls = nullptr;
    if (solver == "LU")
        ls = new LU<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "QR")
        ls = new QR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
    else if (solver == "Inversion")
        ls = new Inversion<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

    assert(ls);

    return ls;
}

bool isDirectSolver(const std::string &solver)
{
    return (solver == "LU" || solver == "QR" || solver == "Inversion");
}

bool isDeflatedPCG(const std::string &solver)
{
    return (solver == "DPCG");
}

bool isChebyshev(const std::string &solver)
{
    return (solver == "Chebyshev");
}

bool isMixedPrecisionDC(const std::string &solver)
{
    return (solver == "MixedPrecisionDC");
}

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        // init PARALUTION
        paralution::init_paralution();
        auto timeStart = std::chrono::steady_clock::now();

        LinearSystemParalutionArgs linearSystem("External solver - PARALUTION", argc, argv);
        linearSystem.readLinearSystem();
        linearSystem.system_sln->resize(linearSystem.system_rhs->max_len);
        linearSystem.convertToCSR();
        // linearSystem.convertToCOO();

        linearSystem.setInfoTimeReadMatrix(elapsedSeconds(timeStart));

        // only 3/4 processors
        if (paralution::_get_backend_descriptor()->OpenMP_threads > 1)
        {
            int num = paralution::_get_backend_descriptor()->OpenMP_threads * 3.0/4.0;
            if (num == 0)
                num = 1;
            paralution::set_omp_threads_paralution(num);
        }

        if (linearSystem.verbose() > 1)
            paralution::info_paralution();

        linearSystem.setInfoNumOfProc(paralution::_get_backend_descriptor()->OpenMP_threads);

        // clear structures
        linearSystem.system_matrix_pattern->clear();

        // info
        paralution::Paralution_Backend_Descriptor *desc = paralution::_get_backend_descriptor();

        LocalVector<ScalarType> sln_paralution;
        LocalVector<ScalarType> rhs_paralution;
        LocalMatrix<ScalarType> mat_paralution;

        mat_paralution.SetDataPtrCSR((int **) &linearSystem.csrRowPtr, (int **) &linearSystem.csrColInd, &linearSystem.matA, "matrix", linearSystem.nz(), linearSystem.n(), linearSystem.n());
        rhs_paralution.SetDataPtr(&linearSystem.system_rhs->val, "rhs", linearSystem.n());
        sln_paralution.SetDataPtr(&linearSystem.system_sln->val, "sln", linearSystem.n());

        // move to accelerator
        bool moveToAccelerator = false;
        if (moveToAccelerator)
        {
            mat_paralution.MoveToAccelerator();
            sln_paralution.MoveToAccelerator();
            rhs_paralution.MoveToAccelerator();
        }

        // preconditioner
        std::string preconditioner = linearSystem.preconditionerArg.getValue();
        if (preconditioner.empty()) preconditioner = "Jacobi"; // default
        linearSystem.setInfoSolverPreconditionerName(preconditioner);

        // solver
        std::string solver = linearSystem.solverArg.getValue();
        if (solver.empty()) solver = "BiCGStab"; // default
        linearSystem.setInfoSolverSolverName(solver);

        // main solver and preconditioner
        Preconditioner<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType> *p = nullptr;
        Solver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ls = nullptr;

        // Direct Solvers
        if (isDirectSolver(solver))
        {
            ls = getDirectSolver(solver);
            ls->SetOperator(mat_paralution);
        }

        // tolerances
        double absTol = linearSystem.absTolArg.getValue();
        double relTol = linearSystem.relTolArg.getValue();
        int maxIter = linearSystem.maxIterArg.getValue();

        // Krylov Subspace Solvers
        if (isKrylovSubspaceSolver(solver))
        {
            p = getPrecoditioner(preconditioner);
            IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ils = getKrylovSubspaceSolver(solver);

            ils->SetOperator(mat_paralution);
            ils->Init(absTol, relTol, 1e8, maxIter);
            ils->SetPreconditioner(*p);

            ls = ils;
        }

        // Deflated PCG
        if (isDeflatedPCG(solver))
        {
            DPCG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *dpcg =
                    new DPCG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

            dpcg->SetOperator(mat_paralution);
            dpcg->Init(absTol, relTol, 1e8, maxIter);
            dpcg->SetNVectors(2);

            ls = dpcg;
        }

        // Chebyshev solver
        if (isChebyshev(solver))
        {
            Chebyshev<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *chebyshev =
                    new Chebyshev<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

            chebyshev->SetOperator(mat_paralution);
            // TODO: estimate minimum and the maximum eigenvalue of the matrix => not working
            chebyshev->Set(0, 100);
            chebyshev->Init(absTol, relTol, 1e8, maxIter);

            ls = chebyshev;
        }

        // Mixed-precision solver
        if (isMixedPrecisionDC(solver))
        {
            MixedPrecisionDC<LocalMatrix<double>, LocalVector<double>, double, LocalMatrix<float>, LocalVector<float>, float> *mp =
                    new MixedPrecisionDC<LocalMatrix<double>, LocalVector<double>, double, LocalMatrix<float>, LocalVector<float>, float>();

            CG<LocalMatrix<float>, LocalVector<float>, float> cg;
            MultiColoredILU<LocalMatrix<float>, LocalVector<float>, float> p;
            cg.SetPreconditioner(p);
            cg.Init(absTol, relTol, 1e8, maxIter);

            mp->SetOperator(mat_paralution);
            mp->Set(cg);

            ls = mp;
        }

        // AMG
        IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double > **amgLS = nullptr;
        Preconditioner<LocalMatrix<double>, LocalVector<double>, double > **amgP = nullptr;
        int amgLevels = 0;

        if (linearSystem.multigridArg.getValue())
        {
            AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *amg =
                    new AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

            amg->SetOperator(mat_paralution);

            // coupling strength
            amg->SetCouplingStrength(0.001);
            // number of unknowns on coarsest level
            amg->SetCoarsestLevel(300);
            // interpolation type for grid transfer operators
            amg->SetInterpolation(SmoothedAggregation);
            // Relaxation parameter for smoothed interpolation aggregation
            amg->SetInterpRelax(2./3.);
            // Manual smoothers
            amg->SetManualSmoothers(true);
            // Manual course grid solver
            amg->SetManualSolver(false);
            // grid transfer scaling
            amg->SetScaling(true);

            amg->BuildHierarchy();

            amgLevels = amg->GetNumLevels();

            // Smoother for each level
            IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double > **amgLS = nullptr;
            Preconditioner<LocalMatrix<double>, LocalVector<double>, double > **amgP = nullptr;

            // Coarse Grid Solver
            CG<LocalMatrix<double>, LocalVector<double>, double > cgs;
            if (linearSystem.verbose() > 1)
                cgs.Verbose(1);
            else
                cgs.Verbose(0);

            // Coarse Grid Preconditioner
            MultiColoredILU<LocalMatrix<double>, LocalVector<double>, double > p;
            cgs.SetPreconditioner(p);

            amgLS = new IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double >*[amgLevels-1];
            amgP = new Preconditioner<LocalMatrix<double>, LocalVector<double>, double >*[amgLevels-1];

            for (int i=0; i<amgLevels-1; ++i) {
                // FixedPoint<LocalMatrix<double>, LocalVector<double>, double > *fp = new FixedPoint<LocalMatrix<double>, LocalVector<double>, double >;
                // fp->SetRelaxation(1.3);

                amgP[i] = getPrecoditioner(preconditioner);
                amgLS[i] = getKrylovSubspaceSolver(solver);
                amgLS[i]->SetPreconditioner(*amgP[i]);
                amgLS[i]->Verbose(0);
            }

            amg->SetOperatorFormat(CSR);
            amg->SetSmoother(amgLS);
            amg->SetSolver(cgs);
            amg->SetSmootherPreIter(1);
            amg->SetSmootherPostIter(2);

            amg->Init(absTol, relTol, 1e8, maxIter);

            ls = amg;
        }

        assert(ls);

        if (linearSystem.verbose() > 1)
        {
            mat_paralution.info();
            rhs_paralution.info();
            sln_paralution.info();

            ls->Verbose(2); // 2
        }
        else
        {
            ls->Verbose(0);
        }

        ls->Build();

        auto timeSolveStart = std::chrono::steady_clock::now();
        ls->Solve(rhs_paralution, &sln_paralution);
        linearSystem.setInfoTimeSolver(elapsedSeconds(timeSolveStart));

        // move to host
        if (moveToAccelerator)
            sln_paralution.MoveToHost();

        sln_paralution.LeaveDataPtr(&linearSystem.system_sln->val);

        // write solution
        linearSystem.writeSolution();

        // check solution
        if (linearSystem.hasReferenceSolution())
            status = linearSystem.compareWithReferenceSolution();

        linearSystem.setInfoTimeTotal(elapsedSeconds(timeStart));
        if (linearSystem.verbose() > 0)
        {
            // Krylov Subspace Solvers
            if (isKrylovSubspaceSolver(solver))
            {

                IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ils
                        = dynamic_cast<IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *>(ls);

                linearSystem.setInfoSolverNumOfIterations(ils->GetIterationCount());
            }

            // Deflated PCG
            if (isDeflatedPCG(solver))
            {
                DPCG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *dpcg =
                        dynamic_cast<DPCG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *>(ls);


                linearSystem.setInfoSolverNumOfIterations(dpcg->GetIterationCount());
            }

            // AMG
            IterativeLinearSolver<LocalMatrix<double>, LocalVector<double>, double > **amgLS = nullptr;
            Preconditioner<LocalMatrix<double>, LocalVector<double>, double > **amgP = nullptr;
            int amgLevels = 0;

            if (linearSystem.multigridArg.getValue())
            {
                AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *amg =
                        dynamic_cast<AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *>(ls);

                linearSystem.setInfoSolverNumOfIterations(amg->GetIterationCount());
            }

            linearSystem.printStatus();

            if (linearSystem.verbose() > 2)
                linearSystem.exportStatusToFile();
        }

        if (ls)
        {
            ls->Clear();
            delete ls;
        }

        if (p)
        {
            p->Clear();
            delete p;
        }

        // AMG
        if (amgP && amgLS)
        {
            for (int i = 0; i < amgLevels - 1; i++)
            {
                delete amgP[i];
                delete amgLS[i];
            }
            delete [] amgP;
            delete [] amgLS;
        }

        rhs_paralution.Clear();
        sln_paralution.Clear();
        mat_paralution.Clear();

        // stop PARALUTION
        paralution::stop_paralution();

        exit(status);
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

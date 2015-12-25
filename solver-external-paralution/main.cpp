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
          maxIterArg(TCLAP::ValueArg<int>("x", "max_iter", "Maximum number of iterations", false, 1000, "int"))
    {
        cmd.add(preconditionerArg);
        cmd.add(solverArg);
        cmd.add(absTolArg);
        cmd.add(relTolArg);
        cmd.add(maxIterArg);
    }

public:
    TCLAP::ValueArg<std::string> preconditionerArg;
    TCLAP::ValueArg<std::string> solverArg;
    TCLAP::ValueArg<double> absTolArg;
    TCLAP::ValueArg<double> relTolArg;
    TCLAP::ValueArg<int> maxIterArg;
};

bool isKrylovSubspaceSolver(const std::string &solver)
{
    return (solver == "BiCGStab" || solver == "CG" || solver == "CR" || solver == "GMRES" || solver == "FGMRES" || solver == "CR" || solver == "IDR");
}

bool isDirectSolver(const std::string &solver)
{
    return (solver == "LU" || solver == "QR" || solver == "Inversion");
}

bool isDeflatedPCG(const std::string &solver)
{
    return (solver == "DPCG");
}

bool isAMG(const std::string &solver)
{
    return (solver == "AMG");
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

        // paralution::set_omp_threads_paralution(1);

        if (linearSystem.isVerbose())
            paralution::info_paralution();

        linearSystem.setInfoNumOfProc(paralution::_get_backend_descriptor()->OpenMP_threads);

        // number of unknowns
        int n = linearSystem.n();

        // number of nonzero elements in matrix
        int nz = linearSystem.nz();

        // info
        paralution::Paralution_Backend_Descriptor *desc = paralution::_get_backend_descriptor();

        //QString backend = QObject::tr("OpenMP");
        //if (desc->accelerator && desc->backend == OCL)
        //    backend = QObject::tr("OpenCL");
        //else if (desc->accelerator && desc->backend == GPU)
        //    backend = QObject::tr("CUDA");
        //else if (desc->accelerator && desc->backend == MIC)
        //    backend = QObject::tr("Xeon MIC");

        LocalVector<ScalarType> sln_paralution;
        LocalVector<ScalarType> rhs_paralution;
        LocalMatrix<ScalarType> mat_paralution;

        mat_paralution.SetDataPtrCSR(&linearSystem.csrRowPtr, &linearSystem.csrColInd, &linearSystem.matA, "matrix", nz, n, n);
        rhs_paralution.SetDataPtr(&linearSystem.system_rhs->val, "rhs", n);
        sln_paralution.SetDataPtr(&linearSystem.system_sln->val, "sln", n);

        // mat_paralution.SetDataPtrCOO(&linearSystem.cooRowInd, &linearSystem.cooColInd, &linearSystem.matA, "matrix", nz, n, n);
        // mat_paralution.ConvertToCSR();

        bool moveToAccelerator = false;
        if (moveToAccelerator)
        {
            mat_paralution.MoveToAccelerator();
            sln_paralution.MoveToAccelerator();
            rhs_paralution.MoveToAccelerator();
        }

        // preconditioner
        std::string preconditioner = linearSystem.preconditionerArg.getValue();
        // default
        if (preconditioner.empty())
            preconditioner = "Jacobi";

        // default
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

        // solver
        std::string solver = linearSystem.solverArg.getValue();
        // default
        if (solver.empty())
            solver = "BiCGStab";

        Solver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ls = nullptr;

        // Direct Solvers
        if (isDirectSolver(solver))
        {
            if (solver == "LU")
                ls = new LU<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
            else if (solver == "QR")
                ls = new QR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
            else if (solver == "Inversion")
                ls = new Inversion<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();

            ls->SetOperator(mat_paralution);
        }

        // Krylov Subspace Solvers
        if (isKrylovSubspaceSolver(solver))
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

            ils->SetOperator(mat_paralution);

            // tolerances
            double absTol = linearSystem.absTolArg.getValue();
            double relTol = linearSystem.relTolArg.getValue();
            int maxIter = linearSystem.maxIterArg.getValue();

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

            dpcg->SetNVectors(2);

            ls = dpcg;
        }

        // AMG
        if (isAMG(solver))
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
            amg->SetManualSolver(true);
            // grid transfer scaling
            amg->SetScaling(true);

            amg->BuildHierarchy();

            int levels = amg->GetNumLevels();

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

            amg->SetOperatorFormat(CSR);
            amg->SetSmoother(sm);
            amg->SetSolver(cgs);
            amg->SetSmootherPreIter(1);
            amg->SetSmootherPostIter(2);

            // tolerances
            double absTol = linearSystem.absTolArg.getValue();
            double relTol = linearSystem.relTolArg.getValue();
            int maxIter = linearSystem.maxIterArg.getValue();

            amg->Init(absTol, relTol, 1e8, maxIter);

            ls = amg;
        }

        assert(ls);

        // AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, double > amg;
        //
        // ls->SetPreconditioner(amg);
        if (linearSystem.isVerbose())
            ls->Verbose(2); // 2
        ls->Build();

        auto timeSolveStart = std::chrono::steady_clock::now();
        ls->Solve(rhs_paralution, &sln_paralution);
        linearSystem.setInfoTimeSolveSystem(elapsedSeconds(timeSolveStart));

        sln_paralution.LeaveDataPtr(&linearSystem.system_sln->val);

        // write solution
        linearSystem.writeSolution();

        // check solution
        if (linearSystem.hasReferenceSolution())
            status = linearSystem.compareWithReferenceSolution();

        linearSystem.setInfoTimeTotal(elapsedSeconds(timeStart));
        if (linearSystem.isVerbose())
            linearSystem.printStatus();

        ls->Clear();
        delete ls;

        p->Clear();
        delete p;

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

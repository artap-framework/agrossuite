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
// GNU General Public License for more details.
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

//typedef float ScalarType;
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

int main(int argc, char *argv[])
{
    try
    {
        int status = 0;

        LinearSystemParalutionArgs linearSystem("External solver - PARALUTION", argc, argv);
        linearSystem.readLinearSystem();
        linearSystem.system_sln->resize(linearSystem.system_rhs->max_len);
        linearSystem.convertToCOO();

        // number of unknowns
        int n = linearSystem.n();

        // number of nonzero elements in matrix
        int nz = linearSystem.nz();

        // init PARALUTION
        paralution::init_paralution();
        // paralution::info_paralution();

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

        mat_paralution.SetDataPtrCOO(&linearSystem.cooIRN, &linearSystem.cooJCN, &linearSystem.cooA, "matrix", nz, n, n);
        mat_paralution.ConvertToCSR();

        // rhs_paralution.Allocate("rhs", n);
        rhs_paralution.SetDataPtr(&linearSystem.system_rhs->val, "rhs", n);
        // sln_paralution.Allocate("sln", n);
        sln_paralution.SetDataPtr(&linearSystem.system_sln->val, "sln", n);

        bool moveToAccelerator = false;
        if (moveToAccelerator)
        {
            mat_paralution.MoveToAccelerator();
            sln_paralution.MoveToAccelerator();
            rhs_paralution.MoveToAccelerator();
        }

        // preconditioner
        std::string preconditioner = linearSystem.preconditionerArg.getValue();
        Preconditioner<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType> *p = nullptr;
        if (preconditioner == "Jacobi" || preconditioner == "") // default
            p = new Jacobi<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
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
        IterativeLinearSolver<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType > *ls = nullptr;
        if (solver == "BiCGStab" || solver == "") // default
            ls = new BiCGStab<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solver == "CG")
            ls = new CG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solver == "GMRES")
            ls = new GMRES<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solver == "FGMRES")
            ls = new FGMRES<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solver == "CR")
            ls = new CR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else if (solver == "IDR")
            ls = new IDR<LocalMatrix<ScalarType>, LocalVector<ScalarType>, ScalarType >();
        else
            std::cerr << "Solver '" << solver << "' not found." << std::endl;

        // tolerances
        double absTol = linearSystem.absTolArg.getValue();
        double relTol = linearSystem.relTolArg.getValue();
        int maxIter = linearSystem.maxIterArg.getValue();

        ls->Init(absTol, relTol, 1e8, maxIter);

        ls->SetOperator(mat_paralution);
        ls->SetPreconditioner(*p);
        // AMG<LocalMatrix<ScalarType>, LocalVector<ScalarType>, double > amg;
        // amg.InitMaxIter(1) ;
        // amg.Verbose(0);
        // ls->SetPreconditioner(amg);
        ls->Verbose(1); // 2
        ls->Build();

        ls->Solve(rhs_paralution, &sln_paralution);
        sln_paralution.LeaveDataPtr(&linearSystem.system_sln->val);

        // write solution
        linearSystem.writeSolution();

        // check solution
        if (linearSystem.hasReferenceSolution())
            status = linearSystem.compareWithReferenceSolution();

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

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

#include <iostream>

#include "../agros-library/util/sparse_io.h"
#include "../agros-library/solver/plugin_solver_interface.h"

#include <amgcl/backend/builtin.hpp>
#include <amgcl/adapter/crs_tuple.hpp>
#include <amgcl/make_solver.hpp>
#include <amgcl/amg.hpp>
#include <amgcl/coarsening/smoothed_aggregation.hpp>
#include <amgcl/relaxation/spai0.hpp>
#include <amgcl/relaxation/ilu0.hpp>
#include <amgcl/solver/cg.hpp>
#include <amgcl/solver/bicgstab.hpp>
#include <amgcl/solver/gmres.hpp>

#include <amgcl/io/mm.hpp>
#include <amgcl/profiler.hpp>

// #define VEXCL_BACKEND_COMPUTE
// #include <vexcl/vexcl.hpp>
// #include <amgcl/backend/vexcl.hpp>

class AMGCLSolverInterface : public QObject, public PluginSolverInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginSolverInterface)
    Q_PLUGIN_METADATA(IID PluginSolverInterface_IID)

public:
    AMGCLSolverInterface() {}
    virtual ~AMGCLSolverInterface() override {}

    virtual QString name() const override { return QString("AMGCL"); }

    virtual void solve(dealii::SparseMatrix<double> &system,
                           dealii::Vector<double> &rhsd,
                           dealii::Vector<double> &sln) override
    {
        solve_cpu_openmp(system, rhsd, sln);
        // solve_opencl(system, rhsd, sln);
    }

    // virtual void solve_opencl(dealii::SparseMatrix<double> &system,
    //                               dealii::Vector<double> &rhsd,
    //                               dealii::Vector<double> &sln)
    // {
    //     vex::Context ctxDevices( vex::Filter::GPU && vex::Filter::DoublePrecision );
    //     if (!ctxDevices) throw std::runtime_error("No devices available.");
    //     // print out list of selected devices:
    //     std::cout << ctxDevices << std::endl;
    //
    //     vex::Context ctx(vex::Filter::Env && vex::Filter::Count(1));
    //
    //     this->prepare_crs(system);
    //
    //     // The profiler:
    //     amgcl::profiler<> prof("agros_profiler");
    //
    //     // prepare RHS
    //     int rows = rhsd.size();
    //     std::vector<double> rhs(rows);
    //     for (size_t i = 0; i < rhsd.size(); ++i)
    //         rhs[i] = rhsd[i];
    //
    //     // Copy the RHS vector to the backend:
    //     vex::vector<double> f(ctx, rhs);
    //
    //     // Solve the system with the previous initial approximation:
    //     vex::vector<double> x(ctx, rows);
    //     for (size_t i = 0; i < sln.size(); ++i)
    //         x[i] = sln[i];
    //
    //     // We use the tuple of CRS arrays to represent the system matrix.
    //     // Note that std::tie creates a tuple of references, so no data is actually
    //     // copied here:
    //     auto A = std::tie(rows,Ap,Ai, Ax);
    //
    //     // Compose the solver type the solver backend:
    //     typedef amgcl::backend::vexcl<double> SBackend;
    //     typedef amgcl::backend::vexcl<double> PBackend;
    //
    //     typedef amgcl::make_solver<amgcl::amg<PBackend,
    //                             amgcl::coarsening::smoothed_aggregation,
    //                             amgcl::relaxation::spai0>,
    //                             amgcl::solver::bicgstab<SBackend> > Solver;
    //
    //     // Solver parameters
    //     Solver::params prm;
    //     prm.solver.maxiter = 500;
    //
    //     // Set the VexCL context in the backend parameters
    //     SBackend::params bprm;
    //     bprm.q = ctx;
    //
    //     Solver::params sprm;
    //     bprm.q = ctx;
    //
    //     Solver solve(A, sprm, bprm);
    //
    //     std::cout << solve.precond() << std::endl;
    //
    //     int iters = 0;
    //     double error = 0.0;
    //
    //     std::tie(iters, error) = solve(f, x);
    //
    //     // Output the number of iterations, the relative error, and the profiling data:
    //     qInfo() << "amgcl solver - iters: " << iters << ", error: " << error;
    //     // qInfo() << prof;
    //
    //     sln = dealii::Vector<double>(rhs.size());
    //     for (size_t row = 0; row < rhs.size(); ++row)
    //         sln[row] = x[row];
    // }

    virtual void solve_cpu_openmp(dealii::SparseMatrix<double> &system,
                                  dealii::Vector<double> &rhsd,
                                  dealii::Vector<double> &sln)
    {
        // #pragma omp parallel
        // {
        //     qInfo() << omp_get_num_threads();
        // }

        this->prepare_crs(system);

        // The profiler:
        amgcl::profiler<> prof("agros_profiler");

        // prepare RHS
        int rows = rhsd.size();
        std::vector<double> rhs(rows);
        for (size_t i = 0; i < rhsd.size(); ++i)
            rhs[i] = rhsd[i];

        // Solve the system with the previous initial approximation:
        std::vector<double> x(rows);
        for (size_t i = 0; i < sln.size(); ++i)
            x[i] = sln[i];

        // We use the tuple of CRS arrays to represent the system matrix.
        // Note that std::tie creates a tuple of references, so no data is actually
        // copied here:
        auto A = std::tie(rows,Ap,Ai, Ax);

        // Compose the solver type
        //   the solver backend:
        typedef amgcl::backend::builtin<double> SBackend;
        typedef amgcl::backend::builtin<double> PBackend;
        // typedef amgcl::make_solver<amgcl::amg<PBackend,
        //                 amgcl::coarsening::smoothed_aggregation,
        //                 amgcl::relaxation::spai0>,
        //                 amgcl::solver::bicgstab<SBackend> > Solver;

        typedef amgcl::make_solver<amgcl::amg<PBackend,
                                amgcl::coarsening::smoothed_aggregation,
                                amgcl::relaxation::spai0>,
                                amgcl::solver::bicgstab<SBackend> > Solver;

        // Initialize the solver with the system matrix:
        // prof.tic("setup");
        Solver solve(A);
        // prof.toc("setup");

        // Show the mini-report on the constructed solver:
        std::cout << solve << std::endl;

        int iters = 0;
        double error = 0.0;

        // prof.tic("solve");
        std::tie(iters, error) = solve(A, rhs, x);
        // prof.toc("solve");

        // Output the number of iterations, the relative error, and the profiling data:
        qInfo() << "amgcl solver - iters: " << iters << ", error: " << error;
        // qInfo() << prof;

        sln = dealii::Vector<double>(rhs.size());
        for (size_t row = 0; row < rhs.size(); ++row)
            sln[row] = x[row];
    }
};

#include "main.moc"

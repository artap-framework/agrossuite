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

#include <Eigen/SparseCholesky>
#include <Eigen/Sparse>

class EigenSolverInterface : public QObject, public PluginSolverInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginSolverInterface)
    Q_PLUGIN_METADATA(IID PluginSolverInterface_IID)

public:
    EigenSolverInterface() {}
    virtual ~EigenSolverInterface() {}

    virtual QString name() const { return QString("Eigen"); }

    virtual void solve(dealii::SparseMatrix<double> &system,
                       dealii::Vector<double> &rhs,
                       dealii::Vector<double> &sln)
    {
        this->prepare_crs(system);

        // number of unknowns
        int n = rhs.size();

        // number of nonzero elements in matrix
        int nz = system.n_nonzero_elements();

        // CSR format
        // fill A
        Eigen::Map<Eigen::SparseMatrix<double, Eigen::RowMajor> > spMap(n, n, nz, Ap.data(),Ai.data(), Ax.data(), 0);

        Eigen::SparseMatrix<double, Eigen::RowMajor> A = spMap.eval();
        A.reserve(nz);

        // fill b
        Eigen::VectorXd b = Eigen::Map<Eigen::VectorXd, Eigen::Unaligned>(rhs.data(), rhs.size());

        // solver
        // Eigen::SparseLU<Eigen::SparseMatrix<double> > solver;
        // Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int> > solver;
        Eigen::SimplicialLDLT<Eigen::SparseMatrix<double> > solver;

        // Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Lower|Eigen::Upper, Eigen::DiagonalPreconditioner<double> > solver;
        // Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>, Eigen::LeastSquareDiagonalPreconditioner<double> > solver;
        // Eigen::BiCGSTAB<Eigen::SparseMatrix<double>, Eigen::DiagonalPreconditioner<double> > solver;
        // solver.setMaxIterations(10000);
        // solver.setTolerance(1e-6);

        // solve Ax = b
        solver.compute(A);
        if(solver.info() != Eigen::Success)
        {
            std::cerr << "Eigen decomposition failed: " << solver.info() << std::endl;
        }

        Eigen::VectorXd x = solver.solve(b);
        if (solver.info() != Eigen::Success)
        {
            std::cerr << "Eigen solver: " << solver.info() << std::endl;
        }
        // std::cout << "#iterations:     " << solver.iterations() << std::endl;
        // std::cout << "estimated error: " << solver.error() << std::endl;

        // copy solution back
        sln = dealii::Vector<double>(x.data(), x.data() + x.size());

        // clear
        Ap.clear();
        Ai.clear();
        Ax.clear();
    }
};

#include "main.moc"

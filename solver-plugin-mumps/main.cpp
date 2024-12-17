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

#include "dmumps_c.h"

#define JOB_INIT -1
#define JOB_END -2
#define JOB_SOLVE 6
#define USE_COMM_WORLD -987654

#define ICNTL(I) icntl[(I)-1] /* macro s.t. indices match documentation */
#define INFOG(I) infog[(I)-1] /* macro s.t. indices match documentation */
#define INFO(I) info[(I)-1]   /* macro s.t. indices match documentation */

class MUMPSSolverInterface : public QObject, public PluginSolverInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginSolverInterface)
    Q_PLUGIN_METADATA(IID PluginSolverInterface_IID)

public:
    MUMPSSolverInterface() {}
    virtual ~MUMPSSolverInterface() override {}

    virtual QString name() const override { return QString("MUMPS"); }

    virtual void solve(dealii::SparseMatrix<double> &system,
                       dealii::Vector<double> &rhs,
                       dealii::Vector<double> &sln) override
    {
        size_t rank = 0;

        // initialize a MUMPS instance. Use MPI_COMM_WORLD
        DMUMPS_STRUC_C id;
        id.job = JOB_INIT;
        id.par = 1;
        id.sym = 0;
        id.comm_fortran = USE_COMM_WORLD;
        id.ICNTL(48) = 1; // use multithreading
        // id.ICNTL(16) = 4; // number of threads

        // ReorderingStrategy::AUTOMATIC:
        // id.ICNTL(28) = 0;
        // id.ICNTL(7) = 7;
        // id.ICNTL(29) = 0;

        // ReorderingStrategy::PORD:
        // id.ICNTL(28) = 1;
        // id.ICNTL(7) = 4;

        // qInfo() << "multithreading" << id.ICNTL(48) << ", number of threads" << id.ICNTL(16);
        dmumps_c(&id);

        // number of unknowns
        id.n = rhs.size();

        // number of nonzero elements in matrix
        id.nz = system.n_nonzero_elements();

        // matrix
        id.a = new double[id.nz];
        id.irn = new int[id.nz];
        id.jcn = new int[id.nz];

        int index = 0;
        for (size_t row = 0; row < system.m(); ++row)
        {
            for (typename dealii::SparseMatrix<double>::const_iterator ptr = system.begin (row); ptr != system.end (row); ++ptr)
                if (std::abs(ptr->value()) > 0.0)
                {
                    id.a[index] = ptr->value ();
                    id.irn[index] = row + 1;
                    id.jcn[index] = ptr->column() + 1;
                    ++index;
                }
        }

        // prepare RHS
        id.rhs = new double[rhs.size()];
        for (size_t i = 0; i < rhs.size(); ++i)
            id.rhs[i] = rhs(i);

        // no outputs
        // id.ICNTL(1) = 6; // -1
        id.ICNTL(1) = -1;
        id.ICNTL(2) = -1;
        id.ICNTL(3) = -1;
        id.ICNTL(4) =  0;

        // call the MUMPS package.
        id.job = JOB_SOLVE;
        dmumps_c(&id);
        id.job = JOB_END;
        dmumps_c(&id); // Terminate instance

        delete [] id.a;
        delete [] id.irn;
        delete [] id.jcn;

        switch (id.infog[1])
        {
        case 0:  // no error
        {
            if (rank == 0)
            {
                sln = dealii::Vector<double>(rhs.size());
                for (size_t row = 0; row < rhs.size(); ++row)
                {
                    sln[row] = id.rhs[row];
                }
                delete [] id.rhs;
            }
        }
            break;
        case -1:
        {
            std::cerr << "Error occured on processor " << id.infog[2] << std::endl;
        }
            break;
        case -2:
        {
            std::cerr << "Number of nonzeros (NNZ) is out of range." << std::endl;
        }
            break;
        case -3:
        {
            std::cerr << "MUMPS called with an invalid option for JOB." << std::endl;
        }
            break;
        case -5:
        {
            std::cerr << "Problem of REAL or COMPLEX workspace allocation of size " << id.infog[2] << " during analysis." << std::endl;
        }
            break;
        case -6:
        {
            std::cerr << "Matrix is singular in structure." << std::endl;
        }
            break;
        case -7:
        {
            std::cerr << "Problem of INTEGER workspace allocation of size " << id.infog[2] << " during analysis."<< std::endl;
        }
            break;
        case -10:
        {
            std::cerr << "Numerically singular matrix." << std::endl;
        }
            break;
        default:
        {
            std::cerr << "Non-detailed exception in MUMPS: INFOG(1) = " << id.infog[1] << std::endl;
        }
            break;
        }
    }
};

#include "main.moc"

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

#include "../3rdparty/tclap/CmdLine.h"
#include "../agros-library/util/sparse_io.h"
#include "../agros-library/solver/plugin_solver_interface.h"

#include <umfpack.h>


class UMFPACKSolverInterface : public QObject, public PluginSolverInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginSolverInterface)
    Q_PLUGIN_METADATA(IID PluginSolverInterface_IID)

public:
    UMFPACKSolverInterface() {}
    virtual ~UMFPACKSolverInterface() {}

    virtual QString name() const { return QString("UMFPACK"); }

    virtual void solve(dealii::SparseMatrix<double> &system,
                       dealii::Vector<double> &rhs,
                       dealii::Vector<double> &sln)
    {
        this->prepare_crs(system);

        // number of unknowns
        int n = rhs.size();

        // number of nonzero elements in matrix
        int nz = system.n_nonzero_elements();

        // reporting
        double Info[UMFPACK_INFO];
        double Control[UMFPACK_CONTROL];


        // factorizing symbolically
        void *symbolic;
        int statusSymbolic = umfpack_di_symbolic(n,
                                                 n,
                                                 Ap.data(),
                                                 Ai.data(),
                                                 Ax.data(),
                                                 &symbolic, Control, Info);

        if (statusSymbolic == UMFPACK_OK)
        {
            // LU factorization of matrix
            void *numeric;
            int statusNumeric = umfpack_di_numeric(Ap.data(),
                                                   Ai.data(),
                                                   Ax.data(),
                                                   symbolic, &numeric, Control, Info);

            //  free the memory associated with the symbolic factorization.
            if (symbolic)
                umfpack_di_free_symbolic (&symbolic);

            if (statusNumeric == UMFPACK_OK)
            {
                sln = dealii::Vector<double>(rhs.size());

                //  solve the linear system.
                int statusSolve = umfpack_di_solve(UMFPACK_A,
                                                   Ap.data(),
                                                   Ai.data(),
                                                   Ax.data(),
                                                   sln.begin(),
                                                   rhs.begin(),
                                                   numeric, Control, Info);


                // free the memory associated with the numeric factorization.
                if (numeric)
                    umfpack_di_free_numeric(&numeric);

                if (statusSolve == UMFPACK_OK)
                {                    
                }
                else
                {
                    std::cerr << "UMFPACK numeric error: " << statusNumeric << std::endl;
                }
            }
            else
            {
                std::cerr << "UMFPACK numeric error: " << statusNumeric << std::endl;
            }
        }
        else
        {
            umfpack_di_report_info(Control, Info);
            umfpack_di_report_status(Control, statusSymbolic);

            std::cerr << "UMFPACK symbolic factorization: " << statusSymbolic << std::endl;
        }

        Ap.clear();
        Ai.clear();
        Ax.clear();
    }
};

#include "main.moc"

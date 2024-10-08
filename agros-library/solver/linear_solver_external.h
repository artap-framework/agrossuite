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

#ifndef SOLVER_EXTERNAL_H
#define SOLVER_EXTERNAL_H

#include "util/util.h"

#undef signals
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#define signals public

class AgrosExternalSolver : public QObject
{
public:
    AgrosExternalSolver(const dealii::SparseMatrix<double> *system_matrix,
                        const dealii::Vector<double> *system_rhs,
                        const dealii::Vector<double> *initial_guess = nullptr);

    void setCommand(const QString &command) { m_command = command; }
    void setCommandEnvironment(const QString &command) { m_commandEnvironment = command; }
    void setCommandParameters(const QString &command) { m_commandParameters = command; }

    void solve();
    void solve(const dealii::Vector<double> *initial_guess);

    dealii::Vector<double> &solution() { return m_solution; }

protected:
    // matrix
    const dealii::SparseMatrix<double> *m_system_matrix;
    // rhs
    const dealii::Vector<double> *m_system_rhs;
    // initial guess (previous solution)
    const dealii::Vector<double> *m_initial_guess;
    // solution
    dealii::Vector<double> m_solution;

    QString m_command;
    QString m_commandEnvironment;
    QString m_commandParameters;
};

#endif // SOLVER_EXTERNAL_H

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

#include "util.h"
#include "util/global.h"
#include "solutiontypes.h"

#undef signals
#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#define signals public

class AgrosExternalSolverExternal : public QObject
{
    Q_OBJECT

public:
    AgrosExternalSolverExternal(const dealii::SparseMatrix<double> *system_matrix,
                                const dealii::Vector<double> *system_rhs);
    void solve();
    void solve(const dealii::Vector<double> *initial_guess);

    dealii::Vector<double> &solution() { return m_solution; }

    virtual void setSolverCommand() = 0;

protected:
    QProcess *m_process;

    QString command;

    QString fileMatrix;
    QString fileMatrixPattern;
    QString fileRHS;
    QString fileInitial;
    QString fileSln;

    const dealii::SparseMatrix<double> *m_system_matrix;
    const dealii::Vector<double> *m_system_rhs;
    dealii::Vector<double> m_solution;
    const dealii::Vector<double> *m_initial_guess;

protected slots:
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode);
};

class AgrosExternalSolverMUMPS : public AgrosExternalSolverExternal
{
public:
    AgrosExternalSolverMUMPS(dealii::SparseMatrix<double> *system_matrix,
                             dealii::Vector<double> *system_rhs)
        : AgrosExternalSolverExternal(system_matrix, system_rhs) {}

    virtual void setSolverCommand();
    virtual void free() {}
};

class AgrosExternalSolverUMFPack : public AgrosExternalSolverExternal
{
public:
    AgrosExternalSolverUMFPack(const dealii::SparseMatrix<double> *system_matrix,
                               const dealii::Vector<double> *system_rhs)
        : AgrosExternalSolverExternal(system_matrix, system_rhs) {}

    virtual void setSolverCommand();
    virtual void free() {}
};

#endif // SOLVER_EXTERNAL_H

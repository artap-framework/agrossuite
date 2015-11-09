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

class AgrosExternalSolver : public QObject
{
    Q_OBJECT

public:
    AgrosExternalSolver(const dealii::SparseMatrix<double> *system_matrix,
                                const dealii::Vector<double> *system_rhs);

    void setCommandTemplate(const QString &commandTemplate) { m_commandTemplate = commandTemplate; }

    void solve();
    void solve(const dealii::Vector<double> *initial_guess);

    dealii::Vector<double> &solution() { return m_solution; }

protected:
    QProcess *m_process;

    const dealii::SparseMatrix<double> *m_system_matrix;
    const dealii::Vector<double> *m_system_rhs;
    dealii::Vector<double> m_solution;
    const dealii::Vector<double> *m_initial_guess;

    QString m_commandTemplate;

protected slots:
    void processError(QProcess::ProcessError error);
    void processFinished(int exitCode);
};


#endif // SOLVER_EXTERNAL_H

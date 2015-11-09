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

#ifndef RESULTSTORE_H
#define RESULTSTORE_H

#include "util.h"

class ProblemComputation;

class AGROS_LIBRARY_API ResultStore : public QObject
{
    Q_OBJECT

public:
    ResultStore(ProblemComputation *parentProblem);
    virtual ~ResultStore();

    inline QMap<QString, double> results() { return m_results; }
    inline void setResult(QString key, double value) { m_results[key] = value; }
    inline void removeResult(QString key) { m_results.remove(key); }

public slots:
    void clear();

private:
    ProblemComputation *m_computation;

    QMap<QString, double> m_results;
};

#endif // RESULTSTORE_H

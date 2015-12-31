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

#ifndef PROBLEM_RESULT_H
#define PROBLEM_RESULT_H

#include "util.h"
#include "problem_config.h"

class ProblemResult
{
public:
    ProblemResult();
    ~ProblemResult() {}

    void clear();

    bool load(const QString &fileName);
    bool save(const QString &fileName);

    inline double resultValue(const QString &key) const { return m_results[key]; }
    inline ParametersType &results() { return m_results; }
    inline bool hasResults() const { return !m_results.isEmpty(); }
    inline void setResult(const QString &key, double value) { m_results[key] = value; }
    inline void removeResult(const QString &key) { m_results.remove(key); }

    inline QMap<QString, QVariant> &info() { return m_info; }

private:
    ParametersType m_results;
    QMap<QString, QVariant> m_info;
};

#endif // PROBLEM_CONFIG_H

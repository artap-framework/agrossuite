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

#ifndef PARAMETER_H
#define PARAMETER_H

#include "util/util.h"

class AGROS_LIBRARY_API Parameter
{
public:
    Parameter(const QString &name = "", double lowerBound = 0.0, double upperBound = 1.0);
    ~Parameter();

    void load(QJsonObject &object);
    void save(QJsonObject &object);
    void clear();

    inline QString name() const { return m_name; }
    inline double lowerBound() const { return m_lowerBound; }
    inline void setLowerBound(double lowerBound) { m_lowerBound = lowerBound; }
    inline double upperBound() const { return m_upperBound; }
    inline void setUpperBound(double upperBound) { m_upperBound = upperBound; }

protected:
    QString m_name;
    double m_lowerBound;
    double m_upperBound;
};

class AGROS_LIBRARY_API ParameterSpace
{
public:
    ParameterSpace(QList<Parameter> parameters);
    ~ParameterSpace();

    void clear();

    QList<QMap<QString, double> > sets() { return m_sets; }

    void random(int count);

private:
    QList<Parameter> m_parameters;
    QList<QMap<QString, double> > m_sets;
};

#endif // PARAMETER_H

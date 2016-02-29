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

#include "util.h"

class Parameter
{
public:
    Parameter(const QString &name = "", double lowerBound = 0.0, double upperBound = 1.0);
    ~Parameter();

    void load(QJsonObject &object);
    void save(QJsonObject &object);
    void clear();

    inline QString name() const { return m_name; }
    inline double lowerBound() const { return m_lowerBound; }
    void setLowerBound(double lowerBound);
    inline double upperBound() const { return m_upperBound; }
    void setUpperBound(double upperBound);

    /*
    inline QList<double> values() { return m_values; }
    void addValue(double value);

    inline double randomValue() { return m_lowerBound + (double) qrand() / RAND_MAX * (m_upperBound - m_lowerBound); }
    double randomValue(double mean, double deviation);
    */
    // static factories
    /*
    static Parameter fromValue(const QString &name, double value);
    static Parameter fromList(const QString &name, QList<double> values);

    static Parameter fromLinspace(const QString &name, int count, double lowerBound, double upperBound);
    static Parameter fromRandom(const QString &name, int count, double lowerBound, double upperBound);
    static Parameter fromRandom(const QString &name, int count, double lowerBound, double upperBound, double mean, double deviation);
    */

protected:
    QString m_name;
    double m_lowerBound;
    double m_upperBound;

    QList<double> m_values;
};

class ParameterSpace
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

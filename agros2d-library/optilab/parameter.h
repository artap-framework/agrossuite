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

#include <QWidget>

#include "util.h"

class Parameter
{
public:
    Parameter(const QString &name = "", double lowerBound = 0.0, double upperBound = 1.0);
    virtual ~Parameter();

    void clear();

    void load(QJsonObject &object);
    void save(QJsonObject &object);

    inline QString name() { return m_name; }

    inline void addValue(double value) { m_values.append(value); }
    inline QList<double> values() { return m_values; }

    inline double lowerBound() const { return m_lowerBound; }
    inline double upperBound() const { return m_upperBound; }

    inline double randomNumber() { return m_lowerBound + (double) qrand() / RAND_MAX * (m_upperBound - m_lowerBound); }

    QString toString() const;

    // static factories
    static Parameter fromValue(const QString &name, double value);
    static Parameter fromList(const QString &name, QList<double> values);
    static Parameter fromLinspace(const QString &name, double low, double high, int count);
    static Parameter fromRandom(const QString &name, double low, double high, int count);

protected:
    QString m_name;
    QList<double> m_values;

    double m_lowerBound;
    double m_upperBound;
};

// Q_DECLARE_METATYPE(Parameters)

class Parameters
{
public:
    // void checkName(const QString &key);

    inline QMap<QString, Parameter> &items() { return m_parameters; }

    Parameter &operator[] (const QString &key) { return m_parameters[key]; }
    const Parameter operator[] (const QString &key) const { return m_parameters[key]; }

private:
    QMap<QString, Parameter> m_parameters;
};

#endif // PARAMETER_H

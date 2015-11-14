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

#include "parameter.h"

#include "optilab.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"

// consts
const QString NAME = "name";
const QString VALUES = "values";
const QString LOWER_BOUND = "lower_bound";
const QString UPPER_BOUND = "upper_bound";

Parameter::Parameter(const QString &name, double lowerBound, double upperBound) :
    m_name(name), m_values(QList<double>()), m_lowerBound(lowerBound), m_upperBound(upperBound)
{
    assert(lowerBound <= upperBound);
}

Parameter::~Parameter()
{
    clear();
}

void Parameter::clear()
{
    m_name = "";
    m_values.clear();
}

void Parameter::load(QJsonObject &object)
{
    m_name = object[NAME].toString();
    m_lowerBound = object[LOWER_BOUND].toDouble();
    m_upperBound = object[UPPER_BOUND].toDouble();

    QJsonArray valuesJson = object[VALUES].toArray();
    for (int i = 0; i < valuesJson.size(); i++)
    {
        m_values.append(valuesJson[i].toDouble());
    }
}

void Parameter::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[LOWER_BOUND] = m_lowerBound;
    object[UPPER_BOUND] = m_upperBound;

    // values
    QJsonArray valuesJson;
    foreach (double value, m_values)
    {
        valuesJson.append(value);
    }
    object[VALUES] = valuesJson;
}

QString Parameter::toString() const
{
    QString str = "";
    foreach (double value, m_values)
        str += QString("%1, ").arg(value);
    if (str.endsWith(", "))
        str = str.left(str.length() - 2);

    return QString("%1: %2").arg(m_name).arg(str);
}

// static factories
Parameter Parameter::fromValue(const QString &name, double value)
{
    Parameter parameter(name, value, value);
    parameter.addValue(value);

    return parameter;
}

Parameter Parameter::fromList(const QString &name, QList<double> values)
{
    assert(values.size() > 0);

    double low =  numeric_limits<double>::max();
    double high = -numeric_limits<double>::max();

    foreach (double value, values)
    {
        if (value > high) high = value;
        if (value < low) low = value;
    }

    Parameter parameter(name, low, high);

    foreach (double value, values)
        parameter.addValue(value);

    return parameter;
}

Parameter Parameter::fromLinspace(const QString &name, double low, double high, int count)
{
    assert(count > 0);
    assert(low <= high);

    Parameter parameter(name, low, high);

    double step = (high - low) / (count - 1);
    for (int i = 0; i < count; i++)
        parameter.addValue(low + i * step);

    return parameter;
}

Parameter Parameter::fromRandom(const QString &name, double low, double high, int count)
{
    assert(count > 0);
    assert(low <= high);

    Parameter parameter(name, low, high);

    for (int i = 0; i < count; i++)
        parameter.addValue(parameter.randomNumber());

    return parameter;
}


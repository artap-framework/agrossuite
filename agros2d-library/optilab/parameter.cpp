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
//#include "solver/problem.h"

#include <random>

// consts
const QString NAME = "name";
const QString VALUES = "values";
const QString LOWER_BOUND = "lower_bound";
const QString UPPER_BOUND = "upper_bound";

Parameter::Parameter(const QString &name, double lowerBound, double upperBound) :
    m_name(name), m_values(QList<double>()), m_lowerBound(lowerBound), m_upperBound(upperBound)
{
    //assert(computation->config()->value(ProblemConfig::Parameters).value<ParametersType>().contains(name));
    assert(lowerBound <= upperBound);
}

Parameter::~Parameter()
{
    clear();
}

void Parameter::clear()
{
    m_values.clear();
}

void Parameter::setLowerBound(double lowerBound)
{
    foreach (double value, m_values)
        assert(lowerBound <= value);

    m_lowerBound = lowerBound;
}

void Parameter::setUpperBound(double upperBound)
{
    foreach (double value, m_values)
        assert(upperBound >= value);

    m_upperBound = upperBound;
}

void Parameter::load(QJsonObject &object)
{
    m_name = object[NAME].toString();
    m_lowerBound = object[LOWER_BOUND].toDouble();
    m_upperBound = object[UPPER_BOUND].toDouble();

    QJsonArray valuesJson = object[VALUES].toArray();
    for (int i = 0; i < valuesJson.size(); i++)
        m_values.append(valuesJson[i].toDouble());
}

void Parameter::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[LOWER_BOUND] = m_lowerBound;
    object[UPPER_BOUND] = m_upperBound;

    QJsonArray valuesJson;
    foreach (double value, m_values)
        valuesJson.append(value);

    object[VALUES] = valuesJson;
}

/*
void Parameter::addValue(double value)
{
    assert(m_lowerBound <= value);
    assert(m_upperBound >= value);
    m_values.append(value);
}

double Parameter::randomValue(double mean, double deviation)
{
    assert(m_lowerBound <= mean);
    assert(m_upperBound >= mean);

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(mean, deviation);

    double value;
    do
    {
        value = distribution(generator);
    } while  ((value < m_lowerBound) || (value > m_upperBound));

    return value;
}


Parameter Parameter::fromValue(const QString &name, double value)
{
    Parameter parameter(name, value, value);
    parameter.addValue(value);

    return parameter;
}

Parameter Parameter::fromList(const QString &name, QList<double> values)
{
    assert(!values.isEmpty());

    double lower = numeric_limits<double>::max();
    double upper = -numeric_limits<double>::max();

    foreach (double value, values)
    {
        if (value > upper) upper = value;
        if (value < lower) lower = value;
    }

    Parameter parameter(name, lower, upper);
    foreach (double value, values)
        parameter.addValue(value);

    return parameter;
}

Parameter Parameter::fromLinspace(const QString &name, int count, double lowerBound, double upperBound)
{
    assert(count > 0);
    Parameter parameter(name, lowerBound, upperBound);

    double step = (upperBound - lowerBound) / (count - 1);
    for (int i = 0; i < count; i++)
        parameter.addValue(lowerBound + i * step);

    return parameter;
}

Parameter Parameter::fromRandom(const QString &name, int count, double lowerBound, double upperBound)
{
    assert(count > 0);
    Parameter parameter(name, lowerBound, upperBound);

    for (int i = 0; i < count; i++)
        parameter.addValue(parameter.randomValue());

    return parameter;
}

Parameter Parameter::fromRandom(const QString &name, int count, double lowerBound, double upperBound, double mean, double deviation)
{
    assert(count > 0);
    Parameter parameter(name, lowerBound, upperBound);

    for (int i = 0; i < count; i++)
        parameter.addValue(parameter.randomValue(mean, deviation));

    return parameter;
}
*/
// *****************************************************************************************************************

ParameterSpace::ParameterSpace(QList<Parameter> parameters) :
    m_parameters(parameters) { }

ParameterSpace::~ParameterSpace()
{
    clear();
}

void ParameterSpace::clear()
{
    m_parameters.clear();
    m_sets.clear();
}

void ParameterSpace::random(int count)
{
    for (int i = 0; i < count; i++)
    {
        QMap<QString, double> set;
        foreach (Parameter parameter, m_parameters)
        {
            // TODO: is it really random?
            // set.insert(parameter.name(), parameter.values()[qrand() % parameter.values().length()]);
        }

        m_sets.append(set);
    }
}

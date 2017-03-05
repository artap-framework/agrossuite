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

#include "problem_parameter.h"

#include "util/global.h"
#include "util/constants.h"

#include "problem.h"
#include "problem_config.h"

const QString PARAMETERS = "Parameters";
const QString NAME = "name";
const QString VALUE = "value";

ProblemParameters::ProblemParameters(const QList<ProblemParameter> parameters) : m_parameters(QMap<QString, ProblemParameter>())
{
    set(parameters);
}

ProblemParameters::~ProblemParameters()
{
    clear();
}

void ProblemParameters::load(QJsonObject &object)
{
    // parameters
    if (object.contains(PARAMETERS))
    {
        QList<ProblemParameter> parameters;

        if (object[PARAMETERS].isString())
        {
            // TODO: remove - old interface
            QString str = object[PARAMETERS].toString();
            QStringList strKeysAndValues = str.split(":");
            QStringList strKeys = (strKeysAndValues[0].size() > 0) ? strKeysAndValues[0].split("|") : QStringList();
            QStringList strValues = (strKeysAndValues[1].size() > 0) ? strKeysAndValues[1].split("|") : QStringList();
            assert(strKeys.count() == strValues.count());

            for (int i = 0; i < strKeys.count(); i++)
                parameters.append(ProblemParameter(strKeys[i], strValues[i].toDouble()));
        }
        else
        {
            // new interface
            QJsonArray parametersJson = object[PARAMETERS].toArray();
            for (int i = 0; i < parametersJson.size(); i++)
            {
                QJsonObject parameterJson = parametersJson[i].toObject();

                parameters.append(ProblemParameter(parameterJson[NAME].toString(), parameterJson[VALUE].toDouble()));
            }
        }

        set(parameters);
    }
}

void ProblemParameters::save(QJsonObject &object)
{
    QJsonArray parametersJson;
    foreach(ProblemParameter parameter, m_parameters.values())
    {
        QJsonObject parameterJson;
        parameterJson[NAME] = parameter.name();
        parameterJson[VALUE] = parameter.value();

        parametersJson.append(parameterJson);
    }
    object[PARAMETERS] = parametersJson;
}

void ProblemParameters::clear()
{
    m_parametersSymbolTable.clear();
    m_parameters.clear();
}

void ProblemParameters::add(const ProblemParameter parameter)
{
    m_parameters[parameter.name()] = parameter;
}

void ProblemParameters::remove(const QString &name)
{
    m_parameters.remove(name);
}

void ProblemParameters::set(const QString &key, double val)
{
    try
    {
        // existing key with same value
        if (m_parameters.keys().contains(key) && fabs(number(key) - val) < EPS_ZERO)
            return;

        Agros::problem()->config()->checkVariableName(key, key);
        m_parameters[key] = ProblemParameter(key, val);

        // create new table - invalidate
        m_parametersSymbolTable = exprtk::symbol_table<double>();
        m_parametersSymbolTable.add_constants();

        foreach (QString k, m_parameters.keys())
        {
            if (k == key)
                m_parametersSymbolTable.add_constant(key.toStdString(), val);
            else
                m_parametersSymbolTable.add_constant(k.toStdString(), number(k));
        }
    }
    catch (AgrosException &e)
    {
        // raise exception
        throw e.toString();
    }
}

void ProblemParameters::set(const QList<ProblemParameter> parameters)
{
    foreach (ProblemParameter parameter, parameters)
        m_parameters[parameter.name()] = parameter;

    // create new table
    m_parametersSymbolTable = exprtk::symbol_table<double>();
    m_parametersSymbolTable.add_constants();

    foreach (ProblemParameter parameter, parameters)
        m_parametersSymbolTable.add_constant(parameter.name().toStdString(), parameter.value());
}

ProblemParameter ProblemParameters::parameter(const QString &name) const
{
    assert(m_parameters.contains(name));
    return m_parameters[name];
}
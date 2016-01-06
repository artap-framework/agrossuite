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

#include "functional.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

// consts
const QString NAME = "name";
const QString EXPRESSION = "expression";

Functional::Functional(const QString &name, Operation operation, const QString &expression) :
    m_name(name), m_operation(operation), m_expression(expression)
{
}

Functional::~Functional()
{
}

void Functional::load(QJsonObject &object)
{
    m_name = object[NAME].toString();
    m_expression = object[EXPRESSION].toString();
}

void Functional::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[EXPRESSION] = m_expression;
}

bool Functional::checkExpression(QSharedPointer<Computation> computation)
{
}

bool Functional::evaluateExpression(QSharedPointer<Computation> computation)
{
    // parameters
    QString commandPre = "";
    StringToDoubleMap parameters = computation->config()->value(ProblemConfig::Parameters).value<StringToDoubleMap>();
    foreach (QString key, parameters.keys())
    {
        if (commandPre.isEmpty())
            commandPre += QString("%1 = %2").arg(key).arg(parameters[key]);
        else
            commandPre += QString("; %1 = %2").arg(key).arg(parameters[key]);
    }

    // resutls
    StringToDoubleMap results = computation->result()->results();
    foreach (QString key, results.keys())
    {
        if (commandPre.isEmpty())
            commandPre += QString("%1 = %2").arg(key).arg(results[key]);
        else
            commandPre += QString("; %1 = %2").arg(key).arg(results[key]);
    }

    double result = 0.0;
    currentPythonEngine()->useTemporaryDict();
    bool successfulRun = currentPythonEngine()->runExpression(m_expression, &result, commandPre);
    currentPythonEngine()->useGlobalDict();

    if (successfulRun)
        computation->result()->setResult(m_name, result);

    return successfulRun;
}

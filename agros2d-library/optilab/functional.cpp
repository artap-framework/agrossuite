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
const QString TYPE = "type";
const QString EXPRESSION = "expression";
const QString WEIGHT = "weight";

Functional::Functional(const QString &name, const QString &expression, int weight) :
    m_name(name), m_expression(expression), m_weight(weight)
{
}

void Functional::load(QJsonObject &object)
{
    m_name = object[NAME].toString();
    m_expression = object[EXPRESSION].toString();
    m_weight = object[WEIGHT].toInt();
}

void Functional::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[EXPRESSION] = m_expression;
    object[WEIGHT] = m_weight;
}

bool Functional::evaluateExpression(QSharedPointer<Computation> computation)
{
    // symbol table
    exprtk::symbol_table<double> parametersSymbolTable = computation->config()->parametersSymbolTable();

    // results
    StringToDoubleMap results = computation->results()->items();
    foreach (QString key, results.keys())
        parametersSymbolTable.add_constant(key.toStdString(), results[key]);

    exprtk::expression<double> expr;
    expr.register_symbol_table(parametersSymbolTable);

    QString error;
    if (compileExpression(m_expression, expr, &error))
    {
        computation->results()->setResult(m_name, expr.value(), ComputationResultType_Functional);
        return true;
    }
    else
    {
        qDebug() << "Functional::evaluateExpression" << error;
        return false;
    }
}

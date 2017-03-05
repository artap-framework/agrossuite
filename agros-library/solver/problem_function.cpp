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

#include "problem_function.h"

#include "util/global.h"
#include "util/constants.h"
#include "util/enums.h"

const QString NAME = "name";
const QString TYPE = "type";
const QString LOWER_BOUND = "lower_bound";
const QString UPPER_BOUND = "upper_bound";
const QString EXPRESSION = "expression";

const QString FUNCTIONS = "Functions";

ProblemFunction::ProblemFunction(const QString &name)
    : m_name(name), m_lowerBound(0.0), m_upperBound(1.0)
{

}

void ProblemFunction::load(QJsonObject &object)
{
    m_name = object[NAME].toString();
    m_lowerBound = object[LOWER_BOUND].toDouble();
    m_upperBound = object[UPPER_BOUND].toDouble();
}

void ProblemFunction::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[TYPE] = problemFunctionTypeToStringKey(type());
    object[LOWER_BOUND] = m_lowerBound;
    object[UPPER_BOUND] = m_upperBound;
}

ProblemFunctionAnalytic::ProblemFunctionAnalytic(const QString &name, const QString &expr)
    : ProblemFunction(name)
{
    setExpression(expr);
}

void ProblemFunctionAnalytic::load(QJsonObject &object)
{
    ProblemFunction::load(object);

    setExpression(object[EXPRESSION].toString());
}

void ProblemFunctionAnalytic::save(QJsonObject &object)
{
    ProblemFunction::save(object);

    object[EXPRESSION] = m_expression;
}

double ProblemFunctionAnalytic::value(double val)
{
    m_value = val;
    return m_exprtkExpr.value();
}

double ProblemFunctionAnalytic::derivative(double val)
{
    // numerical derivative
    double delta = (m_upperBound - m_lowerBound) / 1e9;
    return exprtk::derivative(m_exprtkExpr, "value", delta);
}

void ProblemFunctionAnalytic::setExpression(const QString &expr)
{
    // symbol table
    exprtk::symbol_table<double> localSymbolTable;
    localSymbolTable.add_variable("value", m_value);

    // new expression
    m_exprtkExpr = exprtk::expression<double>();
    m_exprtkExpr.register_symbol_table(localSymbolTable);

    // compile expression
    if (compileExpression(expr, m_exprtkExpr, &m_error))
    {
        m_expression = expr;
        // m_isEvaluated = true;
    }
    else
    {
        // m_isEvaluated = false;
        qDebug() << "ProblemFunctionAnalytic::setExpression" << m_error;
        return;
    }
}

// ******************************************************************************

ProblemFunction *ProblemFunctions::factory(ProblemFunctionType type)
{
    ProblemFunction *function = nullptr;
    if (type == ProblemFunctionType_Analytic)
        function = new ProblemFunctionAnalytic();
    else if (type == ProblemFunctionType_Interpolation)
        assert(0); // function = new ProblemFunctionAnalytic();
    else
        assert(0);

    return function;
}

ProblemFunctions::ProblemFunctions(QList<ProblemFunction *> items) : m_functions(QMap<QString, ProblemFunction *>())
{
    foreach (ProblemFunction *function, items)
        m_functions[function->name()] = function;
}

void ProblemFunctions::load(QJsonObject &object)
{
    QJsonArray functionsJson = object[FUNCTIONS].toArray();
    for (int i = 0; i < functionsJson.size(); i++)
    {
        QJsonObject functionJson = functionsJson[i].toObject();

        ProblemFunction *function = ProblemFunctions::factory(problemFunctionTypeFromStringKey(functionJson[TYPE].toString()));
        function->load(functionJson);

        add(function);
    }
}

void ProblemFunctions::save(QJsonObject &object)
{
    QJsonArray functionsJson;
    foreach(ProblemFunction *function, m_functions.values())
    {
        QJsonObject functionJson;
        function->save(functionJson);

        functionsJson.append(functionJson);
    }
    object[FUNCTIONS] = functionsJson;
}

ProblemFunctions::~ProblemFunctions()
{
    clear();
}

void ProblemFunctions::clear()
{
    foreach (ProblemFunction *function, m_functions)
        delete function;
    m_functions.clear();
}

void ProblemFunctions::add(ProblemFunction *function)
{
    m_functions[function->name()] = function;
}

void ProblemFunctions::remove(const QString &name)
{
    m_functions.remove(name);
}

ProblemFunction *ProblemFunctions::function(const QString &name)
{
    foreach (ProblemFunction *function, m_functions)
        if (function->name() == name)
            return function;

    return nullptr;
}

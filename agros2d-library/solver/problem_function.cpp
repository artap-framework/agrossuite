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

ProblemFunctionAnalytic::ProblemFunctionAnalytic(const QString &name, const QString &expr) : ProblemFunction(name)
{
    setExpression(expr);
}

double ProblemFunctionAnalytic::value(double val)
{
    m_value = val;
    return m_exprtkExpr.value();
}

double ProblemFunctionAnalytic::derivative(double val)
{

}

void ProblemFunctionAnalytic::setExpression(const QString &expr)
{
    // symbol table
    exprtk::symbol_table<double> localSymbolTable;
    localSymbolTable.create_variable("value", m_value);
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

ProblemFunctions::ProblemFunctions(QList<ProblemFunction *> items) : m_functions(QMap<QString, ProblemFunction *>())
{
    foreach (ProblemFunction *function, items)
        m_functions[function->name()] = function;
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

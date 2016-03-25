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

#include <Python.h>

#include "value.h"

#include "util/global.h"
#include "logview.h"
#include "pythonlab/pythonengine_agros.h"
#include "solver/problem_config.h"
#include "parser/lex.h"

static const int LOCAL_SYMBOL_TABLE = 0;
static const int PARAMETERS_SYMBOL_TABLE = 1;

tbb::mutex numberEvaluateMutex;
tbb::mutex numberSetMutex;
tbb::mutex numberAtPointMutex;
tbb::mutex numberAtTimeMutex;
tbb::mutex numberAtTimeAndPointMutex;

Value::Value()
    : m_problem(nullptr),
      m_number(0),
      m_text("0"),
      m_isEvaluated(false),
      m_isTimeDependent(false),
      m_isCoordinateDependent(false),
      m_table(DataTable()),
      m_exprtkExpr(nullptr)
{
}

Value::Value(ProblemBase *problem,
             double value)
    : m_problem(problem),
      m_number(value),
      m_text(QString::number(value)),
      m_isEvaluated(true),
      m_isTimeDependent(false),
      m_isCoordinateDependent(false),
      m_table(DataTable()),
      m_exprtkExpr(nullptr)
{
}

Value::Value(ProblemBase *problem,
             const QString &value,
             const DataTable &table)
    : m_problem(problem),
      m_number(0),
      m_text("0"),
      m_isEvaluated(false),
      m_isTimeDependent(false),
      m_isCoordinateDependent(false),
      m_table(table),
      m_exprtkExpr(nullptr)
{
    if (!value.isEmpty())
    {
        parseFromString(value);
    }
}

Value::Value(ProblemBase *problem,
             const QString &value,
             std::vector<double> x,
             std::vector<double> y,
             DataTableType type,
             bool splineFirstDerivatives,
             bool extrapolateConstant)
    : m_problem(problem),
      m_number(0),
      m_text("0"),
      m_isEvaluated(false),
      m_isTimeDependent(false),
      m_isCoordinateDependent(false),
      m_table(DataTable()),
      m_exprtkExpr(nullptr)
{
    assert(x.size() == y.size());

    parseFromString(value.isEmpty() ? "0" : value);
    m_table.setValues(x, y);
    m_table.setType(type);
    m_table.setSplineFirstDerivatives(splineFirstDerivatives);
    m_table.setExtrapolateConstant(extrapolateConstant);
}

Value::Value(const Value &origin)
{
    *this = origin;
}

Value& Value::operator =(const Value &origin)
{
    m_number = origin.m_number;
    m_text = origin.m_text;
    m_isEvaluated = origin.m_isEvaluated;
    m_isTimeDependent = origin.m_isTimeDependent;
    m_isCoordinateDependent = origin.m_isCoordinateDependent;
    m_table = origin.m_table;
    m_problem = origin.m_problem;
    m_exprtkExpr = origin.m_exprtkExpr ? new exprtk::expression<double>(*origin.m_exprtkExpr) : nullptr;

    return *this;
}

Value::~Value()
{
    delete m_exprtkExpr;
    m_table.clear();
}

bool Value::isNumber() const
{
    return (m_exprtkExpr);
}

bool Value::hasTable() const
{
    return (!m_table.isEmpty());
}

void Value::setNumber(double value)
{
    setText(QString::number(value));
}

bool Value::isEvaluated() const
{
    // try evaluation
    double value = number();

    return m_isEvaluated;
}

double Value::number() const
{
    if (!m_exprtkExpr)
    {
        // qDebug() << "Value::number()" << m_number;
        return m_number;
    }

    if (m_problem)
    {
        {
            tbb::mutex::scoped_lock lock(numberEvaluateMutex);

            if (!(m_exprtkExpr->get_symbol_table(PARAMETERS_SYMBOL_TABLE) == m_problem->config()->parametersSymbolTable()))
            {
                // replace parameters symbol table
                exprtk::symbol_table<double> parametersSymbolTable = m_problem->config()->parametersSymbolTable();
                m_exprtkExpr->get_symbol_table(PARAMETERS_SYMBOL_TABLE) = parametersSymbolTable;

                // compile expression
                if (compileExpression(m_text, *m_exprtkExpr, &m_error))
                {
                    m_isEvaluated = true;
                }
                else
                {
                    m_isEvaluated = false;
                    qDebug() << "Value::number()" << m_error;
                }
            }
        }
    }

    // qDebug() << "m_exprtkExpr->value() - problem" << m_exprtkExpr->value();
    return m_exprtkExpr->value();
}

double Value::numberAtPoint(const Point &point) const
{
    if (!isCoordinateDependent())
        return number();

    {
        tbb::mutex::scoped_lock lock(numberAtPointMutex);

        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("x") = point.x;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("y") = point.y;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("r") = point.x;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("z") = point.y;

        compileExpression(m_text, *m_exprtkExpr);
        return number();
    }
}

double Value::numberAtTime(double time) const
{
    if (!isTimeDependent())
        return number();

    {
        tbb::mutex::scoped_lock lock(numberAtTimeMutex);

        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("time") = time;

        compileExpression(m_text, *m_exprtkExpr);
        return number();
    }
}

double Value::numberAtTimeAndPoint(const double time, const Point &point) const
{
    if (!isTimeDependent() && !isCoordinateDependent())
        return number();

    {
        tbb::mutex::scoped_lock lock(numberAtTimeAndPointMutex);

        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("time") = time;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("x") = point.x;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("y") = point.y;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("r") = point.x;
        m_exprtkExpr->get_symbol_table(LOCAL_SYMBOL_TABLE).variable_ref("z") = point.y;

        compileExpression(m_text, *m_exprtkExpr);
        return number();
    }
}

double Value::numberFromTable(double key) const
{
    if (hasTable())
        return m_table.value(key);
    else
        return number();
}

double Value::derivativeFromTable(double key) const
{
    if (hasTable())
        return m_table.derivative(key);
    else
        return 0.0;
}

void Value::setText(const QString &str)
{
    // speed up - int number
    bool isInt = false;
    double numInt = str.toInt(&isInt);
    if (isInt)
    {
        m_number = numInt;
        m_text = str;
        delete m_exprtkExpr;
        m_isEvaluated = true;

        return;
    }

    // speed up - double number
    bool isDouble = false;
    double numDouble = str.toDouble(&isDouble);
    if (isDouble)
    {
        m_number = numDouble;
        m_text = str;
        delete m_exprtkExpr;
        m_isEvaluated = true;

        return;
    }

    if (m_text != str)
    {
        m_text = str;
        lexicalAnalysis();

        {
            tbb::mutex::scoped_lock lock(numberSetMutex);

            // expression
            if (!m_exprtkExpr)
            {
                m_exprtkExpr = new exprtk::expression<double>();

                // symbol table
                exprtk::symbol_table<double> localSymbolTable;
                localSymbolTable.create_variable("x");
                localSymbolTable.create_variable("y");
                localSymbolTable.create_variable("r");
                localSymbolTable.create_variable("z");
                localSymbolTable.create_variable("time");
                m_exprtkExpr->register_symbol_table(localSymbolTable);

                // problem
                if (m_problem)
                {
                    exprtk::symbol_table<double> parametersSymbolTable = m_problem->config()->parametersSymbolTable();
                    m_exprtkExpr->register_symbol_table(parametersSymbolTable);
                }
            }

            // compile expression
            if (compileExpression(m_text, *m_exprtkExpr, &m_error))
            {
                m_isEvaluated = true;
            }
            else
            {
                m_isEvaluated = false;
                qDebug() << "Value::setText" << m_error;
                return;
            }
        }
    }

    m_isEvaluated = true;
}

QString Value::toString() const
{
    if (m_table.isEmpty())
        return m_text;
    else
        return m_text + ";" + m_table.toString();
}

void Value::parseFromString(const QString &str)
{
    if (str.contains(";"))
    {
        // string and table
        QStringList lst = str.split(";");
        setText(lst.at(0));

        if (lst.size() > 2)
        {
            // try
            {
                m_table.fromString((lst.at(1) + ";" + lst.at(2)));
            }
            // catch (...)
            {
                // do nothing
            }
        }
    }
    else
    {
        // just string
        setText(str);
    }
}

void Value::lexicalAnalysis()
{
    m_isTimeDependent = false;
    m_isCoordinateDependent = false;

    LexicalAnalyser lex;

    // ToDo: Improve
    try
    {
        lex.setExpression(m_text);
    }

    catch (ParserException e)
    {
        // Nothing to do at this point.
    }

    foreach (Token token, lex.tokens())
    {
        if (token.type() == ParserTokenType_VARIABLE)
        {
            if (token.toString() == "time")
                m_isTimeDependent = true;

            if (token.toString() == "x" || token.toString() == "y" || token.toString() == "r" || token.toString() == "z")
                m_isCoordinateDependent = true;
        }
    }
}

// ************************************************************************************************

PointValue::PointValue(ProblemBase *problem, const Point &point)
    : m_x(Value(problem, point.x)),
      m_y(Value(problem, point.y))
{
}

PointValue::PointValue(const Value &x, const Value &y)
    : m_x(x), m_y(y)
{
}

PointValue& PointValue::operator =(const PointValue &origin)
{
    m_x = origin.m_x;
    m_y = origin.m_y;

    return *this;
}

void PointValue::setPoint(double x, double y)
{
    m_x.setNumber(x);
    m_y.setNumber(y);
}

void PointValue::setPoint(const Point &point)
{
    m_x.setNumber(point.x);
    m_y.setNumber(point.y);
}

void PointValue::setPoint(const Value &x, const Value &y)
{
    m_x = x;
    m_y = y;
}

void PointValue::setPoint(const QString &x, const QString &y)
{
    m_x.setText(x);
    m_y.setText(y);
}

QString PointValue::toString() const
{
    return QString("[%1, %2]").arg(m_x.toString()).arg(m_y.toString());
}

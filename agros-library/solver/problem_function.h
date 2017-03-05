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

#ifndef PROBLEM_FUNCTION_H
#define PROBLEM_FUNCTION_H

#include "util/util.h"
#include "value.h"
#include "solutiontypes.h"

class ProblemFunction
{
public:
    ProblemFunction(const QString &name);
    virtual ~ProblemFunction() {}

    virtual ProblemFunctionType type() const = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }
    inline double lowerBound() const { return m_lowerBound; }
    inline void setLowerBound(double lowerBound) { m_lowerBound = lowerBound; }
    inline double upperBound() const { return m_upperBound; }
    inline void setUpperBound(double upperBound) { m_upperBound = upperBound; }

    virtual double value(double val) = 0;
    virtual double derivative(double val) = 0;

protected:
    QString m_name;
    double m_lowerBound;
    double m_upperBound;
};

class ProblemFunctionAnalytic : public ProblemFunction
{
public:
    ProblemFunctionAnalytic(const QString &name = "", const QString &expr = "");
    virtual ~ProblemFunctionAnalytic() {}

    virtual ProblemFunctionType type() const { return ProblemFunctionType_Analytic; }

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString expression() const { return m_expression; }
    void setExpression(const QString &expr);

    virtual double value(double val);
    virtual double derivative(double val);

protected:
    QString m_expression;

    // value
    mutable exprtk::expression<double> m_exprtkExpr;
    mutable QString m_error;
    mutable double m_value;
};

class ProblemFunctions
{
public:
    ProblemFunctions(QList<ProblemFunction *> items = QList<ProblemFunction *>());
    virtual ~ProblemFunctions();

    void load(QJsonObject &object);
    void save(QJsonObject &object);

    void clear();

    void add(ProblemFunction *function);
    void remove(const QString &name);
    ProblemFunction *function(const QString &name);

    QMap<QString, ProblemFunction *> items() const { return m_functions; }

    static ProblemFunction *factory(ProblemFunctionType type);

protected:
    QMap<QString, ProblemFunction *> m_functions;
};

#endif // PROBLEM_CONFIG_H

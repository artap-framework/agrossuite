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

#ifndef STUDY_H
#define STUDY_H

#include <QWidget>

#include "util.h"
#include "parameter.h"

class ProblemComputation;

class Study
{
public:
    Study();
    virtual ~Study();

    void clear();
    virtual void solve() = 0;
    virtual QString name() = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QMap<QString, QString> resultExpressions() { return m_resultExpressions; }
    QList<QSharedPointer<ProblemComputation> > computations() { return m_computations; }

    void addParameter(Parameter parameter) { m_parameters.append(parameter); }
    void addExpression(const QString &name, const QString &expression) { m_resultExpressions[name] = expression; }

protected:
    QMap<QString, QString> m_resultExpressions;
    QList<Parameter> m_parameters;
    QList<QSharedPointer<ProblemComputation> > m_computations;

    void evaluateExpressions();
};

// only one parameter sweep
class StudySweepAnalysis : public Study
{
public:
    StudySweepAnalysis();

    virtual QString name() { return "Sweep analysis"; } // TODO: user defined

    void setParameter(Parameter parameter) { m_parameters.append(parameter); assert(m_parameters.size() == 1); }
    virtual void solve();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);
};

// a strictly unimodal function
class StudyGoldenSectionSearch : public Study
{
public:
    StudyGoldenSectionSearch(double tolerance = 1e-4);

    virtual QString name() { return "Golden section search"; } // TODO: user defined

    void setParameter(Parameter parameter) { m_parameters.append(parameter); assert(m_parameters.size() == 1); }
    // void setFunctional(const QString &functional); // TODO
    void setFunctional() { qDebug() << "not implemented"; }

    void solve();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

protected:
    double m_tolerance;

    double valueForParameter(const QString &name, double value);
};

#endif // STUDY_H

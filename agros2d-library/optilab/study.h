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
#include "util/enums.h"
#include "parameter.h"
#include "functional.h"

class Computation;
class Study;

class Studies : public QObject
{
    Q_OBJECT

public:
    Studies(QObject *parent = 0);
    ~Studies();

    void addStudy(Study *study);
    void removeStudy(Study *study);
    inline QList<Study *> &items() { return m_studies; }

    void loadStudies();
    void saveStudies();

    Study * operator[] (int idx) { return m_studies[idx]; }
    const Study * operator[] (int idx) const { return m_studies[idx]; }

signals:
    void invalidated();

public slots:
    void clear();

private:
    // studies
    QList<Study *> m_studies;
};

Q_DECLARE_METATYPE(Study *)

class Study : public QObject
{
    Q_OBJECT

public:
    Study();
    virtual ~Study();

    virtual StudyType type() = 0;

    void clear();
    virtual void solve() = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    QList<QSharedPointer<Computation> > computations() { return m_computations; }

    void addParameter(Parameter parameter) { m_parameters.append(parameter); }
    QList<Parameter> &parameters() { return m_parameters; }
    void addFunctional(Functional functional) { m_functionals.append(functional); }
    QList<Functional> &functionals() { return m_functionals; }

    QVariant variant();

protected:
    QList<Functional> m_functionals;
    QList<Parameter> m_parameters;
    QList<QSharedPointer<Computation> > m_computations;

    void evaluateExpressions();
};

// only one parameter sweep
class StudySweepAnalysis : public Study
{
public:
    StudySweepAnalysis();

    virtual inline StudyType type() { return StudyType_SweepAnalysis; }

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

    virtual inline StudyType type() { return StudyType_GoldenSectionSearch; }

    void setParameter(Parameter parameter) { m_parameters.append(parameter); assert(m_parameters.size() == 1); }

    void solve();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

protected:
    double m_tolerance;

    double valueForParameter(const QString &name, double value);
};

#endif // STUDY_H

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

    Study * operator[] (int idx) { return m_studies[idx]; }
    const Study * operator[] (int idx) const { return m_studies[idx]; }

signals:
    void invalidated();

public slots:
    void clear();

    bool loadStudies();
    bool saveStudies();

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

    void addParameter(Parameter parameter) { m_parameters.append(parameter); }
    QList<Parameter> &parameters() { return m_parameters; }

    void addFunctional(Functional functional) { m_functionals.append(functional); }
    QList<Functional> &functionals() { return m_functionals; }

    virtual void fillTreeView(QTreeWidget *trvComputations) = 0;
    QVariant variant();

protected:
    QList<Parameter> m_parameters;
    QList<Functional> m_functionals;

    void evaluateExpressions();
};

#endif // STUDY_H

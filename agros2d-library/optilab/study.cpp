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

#include "study.h"
#include "study_sweep.h"
#include "study_genetic.h"
#include "study.h"
#include "parameter.h"

#include "pythonlab/pythonengine.h"

#include "optilab.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include <typeinfo>

// consts
const QString TYPE = "type";

const QString PARAMETERS = "parameters";
const QString FUNCTIONAL = "functionals";

const QString COMPUTATIONSPACE = "computationspace";
const QString COMPUTATIONS = "computations";
const QString COMPUTATION = "computation";

const QString STUDIES = "studies";

Studies::Studies(QObject *parent) : QObject(parent)
{
    connect(this, SIGNAL(invalidated()), this, SLOT(saveStudies()));
}

Studies::~Studies()
{
    QString fn = QString("%1/studies.json").arg(cacheProblemDir());
    if (QFile::exists(fn))
        QFile::remove(fn);
}

void Studies::addStudy(Study *study)
{
    m_studies.append(study);

    emit invalidated();
}

void Studies::removeStudy(Study *study)
{
    m_studies.removeOne(study);

    emit invalidated();
}

void Studies::clear()
{
    for (int i = 0; i < m_studies.size(); i++)
        delete m_studies[i];
    m_studies.clear();

    emit invalidated();
}

bool Studies::loadStudies()
{
    blockSignals(true);

    clear();

    QString fn = QString("%1/studies.json").arg(cacheProblemDir());

    QFile file(fn);
    if (!file.exists())
        return true; // OK - no study

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open study file.");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    QJsonObject rootJson = doc.object();

    QJsonArray studiesJson = rootJson[STUDIES].toArray();
    for (int i = 0; i < studiesJson.size(); i++)
    {
        QJsonObject studyJson = studiesJson[i].toObject();

        // type
        StudyType type = studyTypeFromStringKey(studyJson[TYPE].toString());

        Study *study = nullptr;
        if (type == StudyType_SweepAnalysis)
            study = new StudySweepAnalysis();
        else if (type == StudyType_Genetic)
            study = new StudyGenetic();
        else
            assert(0);

        study->load(studyJson);
        m_studies.append(study);
    }

    blockSignals(false);

    emit invalidated();

    return true;
}

bool Studies::saveStudies()
{
    QString fn = QString("%1/studies.json").arg(cacheProblemDir());

    if (m_studies.isEmpty())
    {
        if (QFile::exists(fn))
            QFile::remove(fn);

        return true;
    }

    QFile file(fn);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open studies file.");
        return false;
    }

    // root object
    QJsonObject rootJson;

    // studies
    QJsonArray studiesJson;
    foreach (Study *study, m_studies)
    {
        QJsonObject studyJson;
        studyJson[TYPE] = studyTypeToStringKey(study->type());
        study->save(studyJson);

        studiesJson.append(studyJson);
    }
    rootJson[STUDIES] = studiesJson;

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson());

    return true;
}

// *****************************************************************************************************************

Study::Study(QList<ComputationSet> computations)
    : m_computations(computations) { }

Study::~Study()
{
    clear();
}

void Study::clear()
{
    m_parameters.clear();
    m_functionals.clear();
    m_computations.clear();
}

void Study::load(QJsonObject &object)
{
    // parameters
    QJsonArray parametersJson = object[PARAMETERS].toArray();
    for (int i = 0; i < parametersJson.size(); i++)
    {
        QJsonObject parameterJson = parametersJson[i].toObject();
        Parameter parameter;
        parameter.load(parameterJson);

        m_parameters.append(parameter);
    }

    // functionals
    QJsonArray functionalsJson = object[FUNCTIONAL].toArray();
    for (int i = 0; i < functionalsJson.size(); i++)
    {
        QJsonObject parameterJson = functionalsJson[i].toObject();
        Functional functional;
        functional.load(parameterJson);

        m_functionals.append(functional);
    }
}

void Study::save(QJsonObject &object)
{    
    // parameters
    QJsonArray parametersJson;
    foreach (Parameter parameter, m_parameters)
    {
        QJsonObject parameterJson;
        parameter.save(parameterJson);

        parametersJson.append(parameterJson);
    }
    object[PARAMETERS] = parametersJson;

    // functionals
    QJsonArray functionalsJson;
    foreach (Functional functional, m_functionals)
    {
        QJsonObject functionalJson;
        functional.save(functionalJson);

        functionalsJson.append(functionalJson);
    }
    object[FUNCTIONAL] = functionalsJson;

    // computations
    QJsonArray computationsJson;
    foreach (ComputationSet computationSet, m_computations)
    {
        QJsonObject computationJson;
        computationSet.save(computationJson);
        computationsJson.append(computationJson);
    }
    object[COMPUTATIONSPACE] = computationsJson;
}

bool Study::evaluateFunctionals(QSharedPointer<Computation> computation)
{
    bool successfulRun = false;
    currentPythonEngine()->useTemporaryDict();
    foreach (Functional functional, m_functionals)
        successfulRun = functional.evaluateExpression(computation);
    currentPythonEngine()->useGlobalDict();

    return successfulRun;
}

void Study::addComputation(QSharedPointer<Computation> computation, bool new_computationSet)
{
    if (m_computations.isEmpty() || new_computationSet)
        m_computations.append(ComputationSet());

    m_computations.last().addComputation(computation);
}

void Study::fillTreeView(QTreeWidget *trvComputations)
{
    for (int i = 0; i < m_computations.size(); i++)
    {
        QTreeWidgetItem *itemComputationSet = new QTreeWidgetItem(trvComputations);
        itemComputationSet->setText(0, tr("Computation set %1 (%2 items)").arg(i + 1).arg(m_computations[i].computations().size()));
        itemComputationSet->setExpanded(true);

        foreach (QSharedPointer<Computation> computation, m_computations[i].computations())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(itemComputationSet);
            item->setText(0, computation->problemDir());
            item->setText(1, QString("%1 / %2").arg(computation->isSolved() ? tr("solved") : tr("not solved")).arg(computation->result()->hasResults() ? tr("results") : tr("no results")));
            item->setData(0, Qt::UserRole, computation->problemDir());
        }
    }
}

QVariant Study::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

ComputationSet::ComputationSet(QList<QSharedPointer<Computation> > set)
    : m_computationSet(set) { }

ComputationSet::~ComputationSet()
{
    m_computationSet.clear();
}

void ComputationSet::load(QJsonObject &object)
{
    QJsonArray computationsJson = object[COMPUTATIONS].toArray();
    for (int i = 0; i < computationsJson.size(); i++)
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[object[COMPUTATION].toString()];
        m_computationSet.append(computation);
    }
}

void ComputationSet::save(QJsonObject &object)
{
    QJsonArray computationsJson;
    foreach (QSharedPointer<Computation> computation, m_computationSet)
    {
        QJsonObject computationJson;
        computationJson[COMPUTATION] = computation->problemDir();
        computationsJson.append(computationJson);
    }
    object[COMPUTATIONS] = computationsJson;
}

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
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "study_sweep.h"
#include "study_genetic.h"

// consts
const QString TYPE = "type";

const QString PARAMETERS = "parameters";
const QString FUNCTIONAL = "functionals";

const QString COMPUTATIONS = "computations";
const QString COMPUTATIONSET = "computationset";

const QString STUDIES = "studies";

ComputationSet::ComputationSet(QList<QSharedPointer<Computation> > set)
    : m_computationSet(set) { }

ComputationSet::~ComputationSet()
{
    m_computationSet.clear();
}

void ComputationSet::load(QJsonObject &object)
{
    QJsonArray computationSetJson = object[COMPUTATIONSET].toArray();
    for (int i = 0; i < computationSetJson.size(); i++)
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[computationSetJson[i].toString()];
        m_computationSet.append(computation);
    }
}

void ComputationSet::save(QJsonObject &object)
{
    QJsonArray computationSetJson;
    foreach (QSharedPointer<Computation> computation, m_computationSet)
        computationSetJson.append(computation->problemDir());
    object[COMPUTATIONSET] = computationSetJson;
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

    // computations
    QJsonArray computationsJson = object[COMPUTATIONS].toArray();
    for (int i = 0; i < computationsJson.size(); i++)
    {
        QJsonObject computationSetJson = computationsJson[i].toObject();
        ComputationSet computationSet;
        computationSet.load(computationSetJson);
        m_computations.append(computationSet);
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
        QJsonObject computationSetJson;
        computationSet.save(computationSetJson);
        computationsJson.append(computationSetJson);
    }
    object[COMPUTATIONS] = computationsJson;
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

        QString computationSetName= tr("Set %1").arg(i + 1);
        if (!m_computations[i].name().isEmpty())
            computationSetName = m_computations[i].name();

        itemComputationSet->setText(0, tr("%1 (%2 computations)").arg(computationSetName).arg(m_computations[i].computations().size()));
        itemComputationSet->setExpanded(true);

        foreach (QSharedPointer<Computation> computation, m_computations[i].computations())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(itemComputationSet);
            item->setText(0, computation->problemDir());
            item->setText(1, QString("%1 / %2").arg(computation->isSolved() ? tr("solved") : tr("not solved")).arg(computation->results()->hasResults() ? tr("results") : tr("no results")));
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

// *****************************************************************************************************************

Studies::Studies(QObject *parent) : QObject(parent)
{
    connect(this, SIGNAL(invalidated()), this, SLOT(save()));
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

bool Studies::load()
{
    blockSignals(true);
    clear();

    QString fn = QString("%1/studies.json").arg(cacheProblemDir());

    QFile file(fn);
    if (!file.exists())
        return true; // no study

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

bool Studies::save()
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

    QJsonObject rootJson;
    QJsonArray studiesJson;

    // studies
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

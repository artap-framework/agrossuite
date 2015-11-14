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
#include "parameter.h"

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
const QString COMPUTATIONS = "computations";
const QString PARAMETERS = "parameters";
const QString FUNCTIONAL = "functionals";
const QString STUDIES = "studies";

Studies::Studies(QObject *parent) : QObject(parent)
{

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

void Studies::loadStudies()
{
    clear();

    QString fn = QString("%1/studies.json").arg(cacheProblemDir());
    QFile file(fn);
    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open study file.");
        return;
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
        else if (type == StudyType_GoldenSectionSearch)
            study = new StudyGoldenSectionSearch();
        else
            assert(0);

        study->load(studyJson);
        m_studies.append(study);
    }

    emit invalidated();
}

void Studies::saveStudies()
{
    QString fn = QString("%1/studies.json").arg(cacheProblemDir());

    QFile file(fn);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open studies file.");
        return;
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
}

// *****************************************************************************************************************

Study::Study()
{
}

Study::~Study()
{
    clear();
}

QVariant Study::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

void Study::clear()
{
    m_parameters.clear();
    m_functionals.clear();
}

void Study::load(QJsonObject &object)
{
    // computations
    QJsonArray computationsJson = object[COMPUTATIONS].toArray();
    for (int i = 0; i < computationsJson.size(); i++)
    {
        QMap<QString, QSharedPointer<ProblemComputation> > computations = Agros2D::computations();
        m_computations.append(computations[computationsJson[i].toString()]);
    }

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
    // computations
    QJsonArray computationsJson;
    foreach (QSharedPointer<ProblemComputation> computation, m_computations)
    {
        computationsJson.append(computation->problemDir());
    }
    object[COMPUTATIONS] = computationsJson;

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
}

// ********************************************************************************

StudySweepAnalysis::StudySweepAnalysis() : Study()
{
}

void StudySweepAnalysis::solve()
{
    // only one parameter
    assert(m_parameters.size() == 1);

    Parameter parameter = m_parameters.first();

    foreach (double value, parameter.values())
    {
        // set parameter
        Agros2D::preprocessor()->config()->setParameter(parameter.name(), value);

        // create computation
        QSharedPointer<ProblemComputation> computation = Agros2D::preprocessor()->createComputation(true, false);
        // store computation
        m_computations.append(computation);

        // solve
        computation->solve();

        // postprocessor
        // TODO: script !!!
        FieldInfo *fieldInfo = computation->fieldInfo("electrostatic");
        computation->scene()->labels->setSelected(true);
        std::shared_ptr<IntegralValue> values = fieldInfo->plugin()->volumeIntegral(computation.data(),
                                                                                    fieldInfo,
                                                                                    0,
                                                                                    0);

        QMap<QString, double> integrals = values->values();
        computation->result()->setResult("We", integrals["electrostatic_energy"]);
        computation->result()->setResult("V", integrals["electrostatic_volume"]);
        // TODO: script !!!

        computation->saveResults();
    }
}

void StudySweepAnalysis::load(QJsonObject &object)
{
    Study::load(object);
}

void StudySweepAnalysis::save(QJsonObject &object)
{
    Study::save(object);
}

// ********************************************************************************

StudyGoldenSectionSearch::StudyGoldenSectionSearch(double tolerance) : Study(),
    m_tolerance(tolerance)
{
}

double StudyGoldenSectionSearch::valueForParameter(const QString &name, double value)
{
    // set parameter
    Agros2D::preprocessor()->config()->setParameter(name, value);
    // create computation
    QSharedPointer<ProblemComputation> computation = Agros2D::preprocessor()->createComputation(true, false);
    // store computation
    m_computations.append(computation);

    // solve
    computation->solve();

    // postprocessor
    // TODO: script !!!
    FieldInfo *fieldInfo = computation->fieldInfo("electrostatic");
    computation->scene()->labels->setSelected(true);
    std::shared_ptr<IntegralValue> values = fieldInfo->plugin()->volumeIntegral(computation.data(),
                                                                                fieldInfo,
                                                                                0,
                                                                                0);

    QMap<QString, double> integrals = values->values();
    computation->result()->setResult("We", integrals["electrostatic_energy"]);

    // "functional"
    double functional = integrals["electrostatic_energy"] / integrals["electrostatic_volume"];
    computation->result()->setResult("fct", functional);

    computation->saveResults();

    return functional;
    // TODO: script !!!
}

void StudyGoldenSectionSearch::solve()
{
    // only one parameter
    assert(m_parameters.size() == 1);

    Parameter parameter = m_parameters.first();

    double goldenRate = (sqrt(5) - 1) / 2;

    double a = parameter.lowerBound();
    double b = parameter.upperBound();

    double xL = b - goldenRate*(b - a);
    double xR = a + goldenRate*(b - a);

    while ((b - a) > m_tolerance)
    {
        double fc = valueForParameter(parameter.name(), xL);
        double fd = valueForParameter(parameter.name(), xR);

        if (fc < fd)
        {
            b = xR;
            xR = xL;
            xL = b - goldenRate * (b - a);
        }
        else
        {
            a = xL;
            xL = xR;
            xR = a + goldenRate * (b - a);
        }

        // qDebug() << fabs(xL-xR);
    }
}

void StudyGoldenSectionSearch::load(QJsonObject &object)
{
    Study::load(object);
}

void StudyGoldenSectionSearch::save(QJsonObject &object)
{
    Study::save(object);
}

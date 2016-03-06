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
#include "gui/lineeditdouble.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "study_sweep.h"
#include "study_nsga2.h"
#include "study_nlopt.h"
#include "study_bayesopt.h"

#include "qcustomplot/qcustomplot.h"

// consts
const QString NAME = "name";
const QString PARAMETERS = "parameters";
const QString FUNCTIONALS = "functionals";
const QString COMPUTATIONS = "computations";
const QString COMPUTATIONSET = "computationset";
const QString CONFIG = "config";

ComputationSet::ComputationSet(QList<QSharedPointer<Computation> > set, const QString &name)
    : m_computations(set), m_name(name)
{
}

ComputationSet::~ComputationSet()
{
    m_computations.clear();
}

void ComputationSet::load(QJsonObject &object)
{
    // computation set
    QJsonObject computationSetJson = object[COMPUTATIONSET].toObject();
    m_name = computationSetJson[NAME].toString();

    // computations
    QJsonArray computationSetArrayJson = computationSetJson[COMPUTATIONS].toArray();
    for (int i = 0; i < computationSetArrayJson.size(); i++)
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[computationSetArrayJson[i].toString()];
        m_computations.append(computation);
    }
}

void ComputationSet::save(QJsonObject &object)
{
    // computations
    QJsonArray computationSetArrayJson;
    foreach (QSharedPointer<Computation> computation, m_computations)
        computationSetArrayJson.append(computation->problemDir());

    // computation set
    QJsonObject computationSetJson;
    computationSetJson[NAME] = m_name;
    computationSetJson[COMPUTATIONS] = computationSetArrayJson;

    object[COMPUTATIONSET] = computationSetJson;
}

void ComputationSet::sort(const QString &parameterName)
{
    // sort individuals
    std::sort(m_computations.begin(), m_computations.end(), ComputationParameterCompare(parameterName));
}

// *****************************************************************************************************************

Study *Study::factory(StudyType type)
{
    Study *study = nullptr;
    if (type == StudyType_SweepAnalysis)
        study = new StudySweepAnalysis();
    else if (type == StudyType_NSGA2)
        study = new StudyNSGA2Analysis();
    else if (type == StudyType_BayesOptAnalysis)
        study = new StudyBayesOptAnalysis();
    else if (type == StudyType_NLoptAnalysis)
        study = new StudyNLoptAnalysis();
    else
        assert(0);

    study->setStringKeys();
    study->clear();

    return study;
}

Study::Study(QList<ComputationSet> computations)
    : m_computationSets(computations), m_name(""), m_abort(false), m_isSolving(false)
{    
}

Study::~Study()
{
    clear();
}

void Study::clear()
{
    // set default values and types
    setDefaultValues();
    m_setting = m_settingDefault;

    m_parameters.clear();
    m_functionals.clear();
    m_computationSets.clear();
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
    QJsonArray functionalsJson = object[FUNCTIONALS].toArray();
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
        m_computationSets.append(computationSet);
    }

    // config
    QJsonObject configJson = object[CONFIG].toObject();
    m_setting = m_settingDefault;
    foreach (Type key, m_settingDefault.keys())
    {
        if (m_settingDefault[key].type() == QVariant::Bool)
            m_setting[key] = configJson[typeToStringKey(key)].toBool();
        else if (m_settingDefault[key].type() == QVariant::String)
            m_setting[key] = configJson[typeToStringKey(key)].toString();
        else if (m_settingDefault[key].type() == QVariant::Double)
            m_setting[key] = configJson[typeToStringKey(key)].toDouble();
        else if (m_settingDefault[key].type() == QVariant::Int)
            m_setting[key] = configJson[typeToStringKey(key)].toInt();
        else
            assert(0);
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
    object[FUNCTIONALS] = functionalsJson;

    // computations
    QJsonArray computationsJson;
    foreach (ComputationSet computationSet, m_computationSets)
    {
        QJsonObject computationSetJson;
        computationSet.save(computationSetJson);
        computationsJson.append(computationSetJson);
    }
    object[COMPUTATIONS] = computationsJson;

    // config
    QJsonObject configJson;
    foreach (Type key, m_settingDefault.keys())
    {
        if (m_settingDefault[key].type() == QVariant::Bool)
            configJson[typeToStringKey(key)] = m_setting[key].toBool();
        else if (m_settingDefault[key].type() == QVariant::String)
            configJson[typeToStringKey(key)] = m_setting[key].toString();
        else if (m_settingDefault[key].type() == QVariant::Double)
            configJson[typeToStringKey(key)] = m_setting[key].toDouble();
        else if (m_settingDefault[key].type() == QVariant::Int)
            configJson[typeToStringKey(key)] = m_setting[key].toInt();
        else
            assert(0);
    }
    object[CONFIG] = configJson;
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

void Study::evaluateStep(QSharedPointer<Computation> computation)
{
    // solve
    computation->solve();

    // TODO: better error handling
    if (!computation->isSolved())
    {
        throw AgrosException(tr("Problem was not solved."));
    }

    // evaluate functionals
    evaluateFunctionals(computation);

    computation->writeProblemToJson();

    updateParameters(m_parameters, computation.data());
    updateChart();
}

double Study::evaluateSingleGoal(QSharedPointer<Computation> computation)
{
    double totalValue = 0.0;
    foreach (double value, evaluateMultiGoal(computation))
        totalValue += value;

    return totalValue;
}

QList<double> Study::evaluateMultiGoal(QSharedPointer<Computation> computation)
{
    QList<double> values;

    // weight functionals
    int totalWeight = 0;
    foreach (Functional functional, m_functionals)
        totalWeight += functional.weight();

    foreach (Functional functional, m_functionals)
    {
        QString name = functional.name();
        double value = computation->results()->resultValue(name);

        values.append(((double) functional.weight() / totalWeight) * value);
    }

    return values;
}

void Study::addComputation(QSharedPointer<Computation> computation, bool newComputationSet)
{
    if (m_computationSets.isEmpty() || newComputationSet)
        m_computationSets.append(ComputationSet());

    m_computationSets.last().addComputation(computation);
}

void Study::removeParameter(const QString &name)
{
    for (int i = 0; i < m_parameters.count(); i++)
    {
        if (m_parameters[i].name() == name)
        {
            m_parameters.removeAt(i);
            break;
        }
    }
}

Parameter &Study::parameter(const QString &name)
{
    for (int i = 0; i < m_parameters.count(); i++)
    {
        if (m_parameters[i].name() == name)
            return m_parameters[i];
    }

    assert(0);
}

void Study::removeFunctional(const QString &name)
{
    for (int i = 0; i < m_functionals.count(); i++)
    {
        if (m_functionals[i].name() == name)
        {
            m_functionals.removeAt(i);
            break;
        }
    }
}

Functional &Study::functional(const QString &name)
{
    for (int i = 0; i < m_functionals.count(); i++)
    {
        if (m_functionals[i].name() == name)
            return m_functionals[i];
    }

    assert(0);
}

QList<QSharedPointer<Computation> > &Study::computations(int index)
{
    if (index == -1)
        return m_computationSets.last().computations();
    else
        return m_computationSets[index].computations();
}

QVariant Study::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

void Study::doAbortSolve()
{
    m_abort = true;
    Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

void Study::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[View_ChartX] = QString();
    m_settingDefault[View_ChartY] = QString();
    m_settingDefault[View_ApplyToAllSets] = true;
    m_settingDefault[View_ChartLogX] = false;
    m_settingDefault[View_ChartLogY] = false;
}

void Study::setStringKeys()
{
    m_settingKey[View_ChartX] = "View_ChartX";
    m_settingKey[View_ChartY] = "View_ChartY";
    m_settingKey[View_ApplyToAllSets] = "View_ApplyToAllSets";
    m_settingKey[View_ChartLogX] = "View_ChartLogX";
    m_settingKey[View_ChartLogY] = "View_ChartLogY";
}

// *****************************************************************************************************************

Studies::Studies(QObject *parent) : QObject(parent)
{
    //connect(this, SIGNAL(invalidated()), this, SLOT(save())); TODO: only GUI
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

void Studies::removeComputation(QSharedPointer<Computation> computation)
{
    for (int i = 0; i < m_studies.size(); i++)
    {
        for (int j = 0; j < m_studies[i]->computationSets().size(); j++)
        {
            m_studies[i]->computationSets()[j].removeComputation(computation);
        }
    }
}

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
#include "solver/problem_parameter.h"
#include "solver/problem_function.h"

#include "study_sweep.h"
#include "study_nsga2.h"
#include "study_nsga3.h"
#include "study_nlopt.h"
#include "study_bayesopt.h"
#include "study_limbo.h"

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
        // TODO: temporary fix (remove bad solution)
        if (!computation.isNull())
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
    if (type == StudyType_Sweep)
        study = new StudySweep();
    else if (type == StudyType_NSGA2)
        study = new StudyNSGA2();
    else if (type == StudyType_NSGA3)
        study = new StudyNSGA3();
    else if (type == StudyType_BayesOpt)
        study = new StudyBayesOpt();
    else if (type == StudyType_NLopt)
        study = new StudyNLopt();
    else if (type == StudyType_Limbo)
        study = new StudyLimbo();
    else
        assert(0);

    study->setStringKeys();
    study->clear();

    return study;
}

Study::Study(QList<ComputationSet> computations)
    : m_computationSets(computations), m_abort(false), m_isSolving(false)
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
    foreach (Functional functional, m_functionals)
        successfulRun = functional.evaluateExpression(computation);

    return successfulRun;
}

void Study::evaluateStep(QSharedPointer<Computation> computation, SolutionUncertainty solutionUncertainty)
{
    if (m_setting.value(Study::General_SolveProblem).toBool())
    {

        try
        {
            // solve problem
            computation->solve();

            // TODO: better error handling
            if (!computation->isSolved())
                throw AgrosSolverException(tr("Problem was not solved."));
        }
        catch (AgrosException &e)
        {
            Agros2D::log()->printError(tr("Problem"), e.toString());
            throw AgrosSolverException(tr("Problem was not solved."));
        }
    }

    // evaluate functionals
    evaluateFunctionals(computation);
    computation->writeProblemToJson();

    // update GUI
    updateParametersAndFunctionals(computation, solutionUncertainty);
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
        double value = computation->results()->value(name);

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

QList<ComputationSet> Study::computationSets(const QString &filter) const
{
    if (filter.isEmpty())
    {
        return m_computationSets;
    }
    else
    {
        QList<ComputationSet> output;

        for (int i = 0; i < m_computationSets.size(); i++)
        {
            output.append(ComputationSet(QList<QSharedPointer<Computation> >(), m_computationSets[i].name()));
            foreach (QSharedPointer<Computation> computation, m_computationSets[i].computations())
            {
                // symbol table
                exprtk::symbol_table<double> parametersSymbolTable = computation->config()->parameters()->symbolTable();

                // results
                StringToDoubleMap results = computation->results()->items();
                foreach (QString key, results.keys())
                    parametersSymbolTable.add_constant(key.toStdString(), results[key]);

                exprtk::expression<double> expr;
                expr.register_symbol_table(parametersSymbolTable);

                QString error;
                if (compileExpression(filter, expr, &error))
                {
                    if (fabs(expr.value()) < EPS_ZERO)
                        continue;
                }
                else
                {
                    qDebug() << QString("Condition '%1' is not valid. ").arg(filter) << error;
                }

                // add computation
                output.last().addComputation(computation);
            }
        }

        return output;
    }
}

void Study::removeEmptyComputationSets()
{
    // remove empty computation sets
    QMutableListIterator<ComputationSet> i(m_computationSets);
    while (i.hasNext())
    {
        ComputationSet *computationSet = &i.next();

        if (computationSet->computations().count() == 0)
            i.remove();
    }
}

void Study::removeComputation(QSharedPointer<Computation> computation)
{
    QMutableListIterator<ComputationSet> i(m_computationSets);
    while (i.hasNext())
    {
        ComputationSet *computationSet = &i.next();
        computationSet->removeComputation(computation);

        if (computationSet->computations().count() == 0)
            i.remove();
    }
}

bool Study::dominateComputations(const Computation *l, const Computation *r)
{
    bool better = false;
    foreach (QString key, l->results()->items().keys())
    {
        if (l->results()->type(key) == ComputationResultType_Functional)
        {
            if (l->results()->value(key) > r->results()->value(key))
                return false;
            else if (l->results()->value(key) < r->results()->value(key))
                better = true;
        }
    }

    return better;
}

QList<QSharedPointer<Computation> > Study::nondominatedSort(QList<ComputationSet> list)
{
    if (list.count() > 0)
    {
        int computationsCount = 0;

        QList<QSharedPointer<Computation> > allComputations;
        foreach (ComputationSet computationSet, list)
        {
            foreach (QSharedPointer<Computation> computation, computationSet.computations())
            {
                allComputations.append(computation);
                computationsCount++;
            }
        }

        int numAssignedIndividuals = 0;
        QList<QSharedPointer<Computation> > curFront;

        while (numAssignedIndividuals < computationsCount)
        {
            for (int i = 0; i < computationsCount; i++)
            {
                bool beDominated = false;

                for (int j = 0; j < curFront.size(); j++)
                {
                    if (dominateComputations(curFront[j].data(), allComputations[i].data())) // i is dominated
                    {
                        beDominated = true;
                        break;
                    }
                    else if (dominateComputations(allComputations[i].data(), curFront[j].data())) // i dominates a member in the current front
                    {
                        curFront.erase(curFront.begin()+j);
                        j -= 1;
                    }
                }
                if (!beDominated)
                {
                    curFront.push_back(allComputations[i]);
                }
            }

            numAssignedIndividuals += curFront.size();
        }

        return curFront;
    }
    else
    {
        return QList<QSharedPointer<Computation> >();
    }
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

    m_settingDefault[General_ClearSolution] = true;
    m_settingDefault[General_SolveProblem] = true;

    m_settingDefault[View_Filter] = QString();
    m_settingDefault[View_ChartHorizontal] = QString();
    m_settingDefault[View_ChartVertical] = QString();
    m_settingDefault[View_ChartLogHorizontal] = false;
    m_settingDefault[View_ChartLogVertical] = false;
    m_settingDefault[View_ChartShowAverageValue] = true;
    m_settingDefault[View_ChartShowTrend] = false;
    m_settingDefault[View_ChartShowParetoFront] = false;
}

void Study::setStringKeys()
{
    m_settingKey[General_ClearSolution] = "General_ClearSolution";
    m_settingKey[General_SolveProblem] = "General_SolveProblem";

    m_settingKey[View_Filter] = "View_Filter";
    m_settingKey[View_ChartHorizontal] = "View_ChartHorizontal";
    m_settingKey[View_ChartVertical] = "View_ChartVertical";
    m_settingKey[View_ChartLogHorizontal] = "View_ChartLogHorizontal";
    m_settingKey[View_ChartLogVertical] = "View_ChartLogVertical";
    m_settingKey[View_ChartShowAverageValue] = "View_ChartShowAverageValue";
    m_settingKey[View_ChartShowTrend] = "View_ChartShowTrend";
    m_settingKey[View_ChartShowParetoFront] = "View_ChartShowParetoFront";
}

QSharedPointer<Computation> Study::findExtreme(ResultType type, const QString &key, bool minimum)
{
    QSharedPointer<Computation> selectedComputation;

    double extreme = (minimum) ? numeric_limits<double>::max() : -numeric_limits<double>::max();

    for (int i = 0; i < m_computationSets.count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = m_computationSets[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            QSharedPointer<Computation> computation = computations[j];

            double val = NAN;
            if (type == ResultType_Parameter)
                val = computation->config()->parameters()->number(key);
            else if (type == ResultType_Recipe || type == ResultType_Functional)
                val = computation->results()->value(key);
            else
                assert(0);

            if (minimum)
            {
                if (val < extreme)
                {
                    extreme = val;
                    selectedComputation = computation;
                }
            }
            else
            {
                if (val > extreme)
                {
                    extreme = val;
                    selectedComputation = computation;
                }
            }
        }
    }

    return selectedComputation;
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
    foreach (Study *study, m_studies)
    {
        study->removeComputation(computation);
    }
}

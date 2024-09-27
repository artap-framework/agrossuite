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

#include "pystudy.h"
#include "solver/problem_result.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"

PyStudy::PyStudy() : m_study(nullptr)
{
}

void PyStudy::addParameter(string name, double lowerBound, double upperBound)
{
    // TODO: check
    m_study->addParameter(Parameter(QString::fromStdString(name), lowerBound, upperBound));
}

void PyStudy::addGoalFunction(string name, string expression, int weight)
{
    // TODO: check
    m_study->addGoalFunction(GoalFunction(QString::fromStdString(name), QString::fromStdString(expression), weight));
}

void PyStudy::solve() const
{
    m_study->solve();
}

void PyStudy::getParameters(std::vector<std::string> &name, std::vector<double> &lowerBound, std::vector<double> &upperBound) const
{
    QList<Parameter> parameters = m_study->parameters();
    for (int i = 0; i < parameters.count(); i++)
    {
        name.push_back(parameters[i].name().toStdString());
        lowerBound.push_back(parameters[i].lowerBound());
        upperBound.push_back(parameters[i].upperBound());
    }
}

void PyStudy::getGoalFunctions(std::vector<std::string> &name, std::vector<std::string> &expression, std::vector<int> &weight) const
{
    QList<GoalFunction> goalFunctions = m_study->goalFunctions();
    for (int i = 0; i < goalFunctions.count(); i++)
    {
        name.push_back(goalFunctions[i].name().toStdString());
        expression.push_back(goalFunctions[i].expression().toStdString());
        weight.push_back(goalFunctions[i].weight());
    }
}

std::string PyStudy::findExtreme(std::string type, std::string key, bool minimum)
{
    // TODO: checks
    QSharedPointer<Computation> computation = m_study->findExtreme(m_study->resultTypeFromStringKey(QString::fromStdString(type)), QString::fromStdString(key), minimum);

    if (!computation.isNull())
        return computation->problemDir().toStdString();
    else
        return "";
}

void PyStudy::steps(vector<int> &steps) const
{   
    // all computations
    QList<ComputationSet> computationSetsAll  = m_study->computationSets();
    QList<Computation *> computationsAll;
    for (int i = 0; i < computationSetsAll.count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = computationSetsAll[i].computations();
        for (int j = 0; j < computations.count(); j++)
            computationsAll.append(computations[j].data());
    }

    // selected computations
    QList<ComputationSet> computationSetsFilter = m_study->computationSets();
    QList<Computation *> computationsFilter;
    for (int i = 0; i < computationSetsFilter.count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = computationSetsFilter[i].computations();
        for (int j = 0; j < computations.count(); j++)
            computationsFilter.append(computations[j].data());
    }

    // selected steps
    for (int i = 0; i < computationsFilter.count(); i++)
        steps.push_back(computationsAll.indexOf(computationsFilter[i]));
}

void PyStudy::values(std::string variable, vector<double> &values) const
{
    QString key = QString::fromStdString(variable);
    QList<ComputationSet> computationSets = m_study->computationSets();

    for (int i = 0; i < computationSets.count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = computationSets[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            QSharedPointer<Computation> computation = computations[j];

            double val = NAN;
            if (computation->config()->parameters()->items().contains(key))
                val = computation->config()->parameters()->number(key);
            else if (computation->results()->items().keys().contains(key))
                val = computation->results()->value(key);

            // add to the list
            if (!std::isnan(val))
                values.push_back(val);
        }
    }
}

void PyStudy::results(vector<int> &steps, vector<std::string> &names, vector<std::string> &types, vector<double> &values) const
{
    QList<ComputationSet> computationSets = m_study->computationSets();

    for (int i = 0; i < computationSets.count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = computationSets[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            QSharedPointer<Computation> computation = computations[j];
            int current_step = steps.size();

            // parameters
            QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
            foreach (Parameter parameter, m_study->parameters())
            {
                steps.push_back(current_step);
                names.push_back(parameter.name().toStdString());
                types.push_back("parameter");
                values.push_back(parameters[parameter.name()].value());
            }

            // results
            StringToDoubleMap results = computation->results()->items();
            foreach (QString key, results.keys())
            {
                steps.push_back(current_step);
                names.push_back(key.toStdString());
                values.push_back(results[key]);

                if (computation->results()->type(key) == ComputationResultType_Functional)
                    types.push_back("goal");
                else if (computation->results()->type(key) == ComputationResultType_Recipe)
                    types.push_back("recipe");
                else
                    assert(0);
            }
        }
    }
}

void PyStudy::getComputationProblemDir(int index, std::string &problemDir) const
{
    int localIndex = 0;
    problemDir = "";
    QList<ComputationSet> computationSets = m_study->computationSets();

    if (computationSets.size() == 0)
        throw out_of_range(QObject::tr("Computations are empty.").toStdString());

    for (int i = 0; i < computationSets.count(); i++)
    {
        QList<QSharedPointer<Computation> > computations = computationSets[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            if (localIndex == index)
            {
                problemDir = computations[j]->problemDir().toStdString();
                return;
            }

            localIndex++;
        }
    }

    throw out_of_range(QObject::tr("Invalid index. Valid range is from 0 to %1").arg(localIndex-1).toStdString());
}

// BayesOpt **************************************************************

PyStudyBayesOpt::PyStudyBayesOpt(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_BayesOpt);
        Agros::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros::problem()->studies()->items().count())
            m_study = Agros::problem()->studies()->items().at(index);
    }
}

void PyStudyBayesOpt::setInitMethod(const std::string &initMethod)
{
    if (study()->initMethodStringKeys().contains(QString::fromStdString(initMethod)))
        m_study->setValue(Study::BayesOpt_init_method, QString::fromStdString(initMethod));
    else
    {
        QStringList list;
        foreach (QString key, study()->initMethodStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyStudyBayesOpt::setSurrName(const std::string &surrName)
{
    if (study()->surrogateStringKeys().contains(QString::fromStdString(surrName)))
        m_study->setValue(Study::BayesOpt_surr_name, QString::fromStdString(surrName));
    else
    {
        QStringList list;
        foreach (QString key, study()->surrogateStringKeys())
            list.append(study()->surrogateToStringKey(key));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyStudyBayesOpt::setScoreType(const std::string &scoreType)
{
    if (study()->scoreTypeStringKeys().contains(QString::fromStdString(scoreType)))
        m_study->setValue(Study::BayesOpt_sc_type, QString::fromStdString(scoreType));
    else
    {
        QStringList list;
        foreach (QString key, study()->scoreTypeStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyStudyBayesOpt::setLearningType(const std::string &learningType)
{
    if (study()->learningTypeStringKeys().contains(QString::fromStdString(learningType)))
        m_study->setValue(Study::BayesOpt_l_type, QString::fromStdString(learningType));
    else
    {
        QStringList list;
        foreach (QString key, study()->learningTypeStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}


// NLopt **************************************************************

PyStudyNLopt::PyStudyNLopt(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_NLopt);
        Agros::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros::problem()->studies()->items().count())
            m_study = Agros::problem()->studies()->items().at(index);
    }
}

void PyStudyNLopt::setAlgorithm(const std::string &algorithm)
{
    if (study()->algorithmStringKeys().contains(QString::fromStdString(algorithm)))
        m_study->setValue(Study::NLopt_algorithm, QString::fromStdString(algorithm));
    else
    {
        QStringList list;
        foreach (QString key, study()->algorithmStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

// NSGA2 **************************************************************

PyStudyNSGA2::PyStudyNSGA2(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_NSGA2);
        Agros::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros::problem()->studies()->items().count())
            m_study = Agros::problem()->studies()->items().at(index);
    }
}

// Sweep **************************************************************

PyStudySweep::PyStudySweep(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_Sweep);
        Agros::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros::problem()->studies()->items().count())
            m_study = Agros::problem()->studies()->items().at(index);
    }
}

void PyStudySweep::setInitMethod(const std::string &initMethod)
{
    if (study()->initMethodStringKeys().contains(QString::fromStdString(initMethod)))
        m_study->setValue(Study::Sweep_init_method, QString::fromStdString(initMethod));
    else
    {
        QStringList list;
        foreach (QString key, study()->initMethodStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}


// Model **************************************************************

PyStudyModel::PyStudyModel(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_Model);
        Agros::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros::problem()->studies()->items().count())
            m_study = Agros::problem()->studies()->items().at(index);
    }
}
//
// void PyStudyModel::setInitMethod(const std::string &initMethod)
// {
//     if (study()->initMethodStringKeys().contains(QString::fromStdString(initMethod)))
//         m_study->setValue(Study::Sweep_init_method, QString::fromStdString(initMethod));
//     else
//     {
//         QStringList list;
//         foreach (QString key, study()->initMethodStringKeys())
//             list.append(key);
//
//         throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
//     }
// }

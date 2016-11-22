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

void PyStudy::addParameter(string name, double lowerBound, double upperBound,
                           bool penaltyEnabled, double scale, double mu, double sigma)
{
    // TODO: check
    m_study->addParameter(Parameter(QString::fromStdString(name), lowerBound, upperBound,
                                    penaltyEnabled, scale, mu, sigma));
}

void PyStudy::addFunctional(string name, string expression, int weight)
{
    // TODO: check
    m_study->addFunctional(Functional(QString::fromStdString(name), QString::fromStdString(expression), weight));
}

void PyStudy::solve()
{
    m_study->solve();
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
    QList<ComputationSet> computationSetsFilter = m_study->computationSets(m_study->value(Study::View_Filter).toString());
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
    QList<ComputationSet> computationSets = m_study->computationSets(m_study->value(Study::View_Filter).toString());

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

// BayesOpt **************************************************************

PyStudyBayesOpt::PyStudyBayesOpt(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_BayesOpt);
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
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

// Limbo **************************************************************

PyStudyLimbo::PyStudyLimbo(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_Limbo);
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
    }
}

void PyStudyLimbo::setMeanType(const std::string &meanType)
{
    if (study()->meanStringKeys().contains(QString::fromStdString(meanType)))
        m_study->setValue(Study::LIMBO_mean, QString::fromStdString(meanType));
    else
    {
        QStringList list;
        foreach (QString key, study()->meanStringKeys())
            list.append(study()->meanToStringKey(key));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyStudyLimbo::setGPType(const std::string &gpType)
{
    if (study()->gpStringKeys().contains(QString::fromStdString(gpType)))
        m_study->setValue(Study::LIMBO_gp, QString::fromStdString(gpType));
    else
    {
        QStringList list;
        foreach (QString key, study()->gpStringKeys())
            list.append(study()->gpToStringKey(key));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyStudyLimbo::setAcquiType(const std::string &acquiType)
{
    if (study()->acquiStringKeys().contains(QString::fromStdString(acquiType)))
        m_study->setValue(Study::LIMBO_acqui, QString::fromStdString(acquiType));
    else
    {
        QStringList list;
        foreach (QString key, study()->acquiStringKeys())
            list.append(study()->acquiToStringKey(key));

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
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
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

// CMAES **************************************************************

PyStudyCMAES::PyStudyCMAES(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_CMAES);
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
    }
}

void PyStudyCMAES::setAlgorithm(const std::string &algorithm)
{
    if (study()->algorithmStringKeys().contains(QString::fromStdString(algorithm)))
        m_study->setValue(Study::CMAES_algorithm, QString::fromStdString(algorithm));
    else
    {
       QStringList list;
        foreach (QString key, study()->algorithmStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyStudyCMAES::setSurrogate(const std::string &surrogate)
{
    if (study()->surrogateStringKeys().contains(QString::fromStdString(surrogate)))
        m_study->setValue(Study::CMAES_surrogate, QString::fromStdString(surrogate));
    else
    {
        QStringList list;
        foreach (QString key, study()->surrogateStringKeys())
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
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
    }
}

// NSGA3 **************************************************************

PyStudyNSGA3::PyStudyNSGA3(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_NSGA3);
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
    }
}

// Sweep **************************************************************

PyStudySweep::PyStudySweep(int index) : PyStudy()
{
    // add study
    if (index == -1)
    {
        m_study = Study::factory(StudyType_Sweep);
        Agros2D::problem()->studies()->addStudy(m_study);
    }
    else
    {
        if (index < Agros2D::problem()->studies()->items().count())
            m_study = Agros2D::problem()->studies()->items().at(index);
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

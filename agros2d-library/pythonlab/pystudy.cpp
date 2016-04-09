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

// BayesOpt **************************************************************

PyStudyBayesOpt::PyStudyBayesOpt() : PyStudy()
{
    m_study = Study::factory(StudyType_BayesOpt);

    // add study
    Agros2D::problem()->studies()->addStudy(m_study);
}

void PyStudyBayesOpt::setInitMethod(const std::string &initMethod)
{
    if (study()->initMethodStringKeys().contains(QString::fromStdString(initMethod)))
        m_study->setValue(Study::BayesOpt_init_method, study()->initMethodFromStringKey(QString::fromStdString(initMethod)));
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
        m_study->setValue(Study::BayesOpt_surr_name, study()->surrogateFromStringKey(QString::fromStdString(surrName)));
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
        m_study->setValue(Study::BayesOpt_sc_type, study()->scoreTypeFromStringKey(QString::fromStdString(scoreType)));
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
        m_study->setValue(Study::BayesOpt_l_type, study()->learningTypeFromStringKey(QString::fromStdString(learningType)));
    else
    {
        QStringList list;
        foreach (QString key, study()->learningTypeStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

// NLopt **************************************************************

PyStudyNLopt::PyStudyNLopt() : PyStudy()
{
    m_study = Study::factory(StudyType_NLopt);

    // add study
    Agros2D::problem()->studies()->addStudy(m_study);
}

void PyStudyNLopt::setAlgorithm(const std::string &algorithm)
{
    if (study()->algorithmStringKeys().contains(QString::fromStdString(algorithm)))
        m_study->setValue(Study::NLopt_algorithm, study()->algorithmFromStringKey(QString::fromStdString(algorithm)));
    else
    {
        QStringList list;
        foreach (QString key, study()->algorithmStringKeys())
            list.append(key);

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

// NSGA2 **************************************************************

PyStudyNSGA2::PyStudyNSGA2() : PyStudy()
{
    m_study = Study::factory(StudyType_NSGA2);

    // add study
    Agros2D::problem()->studies()->addStudy(m_study);
}

// NSGA3 **************************************************************

PyStudyNSGA3::PyStudyNSGA3() : PyStudy()
{
    m_study = Study::factory(StudyType_NSGA3);

    // add study
    Agros2D::problem()->studies()->addStudy(m_study);
}

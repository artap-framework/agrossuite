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

#include "pyproblem.h"
#include "pythonengine_agros.h"
#include "sceneview_geometry.h"
#include "sceneview_post2d.h"
#include "solver/coupling.h"

PyProblem::PyProblem(bool clearProblem)
{
    if (clearProblem)
        clear();
}

void PyProblem::clear()
{
    // m_problem->scene()->clear();
    m_problem->clearFieldsAndConfig();
}

void PyProblem::refresh()
{
    m_problem->scene()->invalidate();
}


std::string PyProblem::getCouplingType(const std::string &sourceField, const std::string &targetField) const
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (m_problem->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = m_problem->couplingInfo(source, target);
        return couplingTypeToStringKey(couplingInfo->couplingType()).toStdString();
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

void PyProblem::checkExistingFields(const QString &sourceField, const QString &targetField) const
{
    if (m_problem->fieldInfos().isEmpty())
        throw logic_error(QObject::tr("No fields are defined.").toStdString());

    if (!m_problem->fieldInfos().contains(sourceField))
        throw logic_error(QObject::tr("Source field '%1' is not defined.").arg(sourceField).toStdString());

    if (!m_problem->fieldInfos().contains(targetField))
        throw logic_error(QObject::tr("Target field '%1' is not defined.").arg(targetField).toStdString());
}

PyPreprocessor::PyPreprocessor(bool clearProblem) : PyProblem(clearProblem)
{
    m_problem = QSharedPointer<Problem>(Agros2D::preprocessor());
}

void PyPreprocessor::setCoordinateType(const std::string &coordinateType)
{
    if (coordinateTypeStringKeys().contains(QString::fromStdString(coordinateType)))
        m_problem->config()->setCoordinateType(coordinateTypeFromStringKey(QString::fromStdString(coordinateType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(coordinateTypeStringKeys())).toStdString());
}

void PyPreprocessor::setMeshType(const std::string &meshType)
{
    if (meshTypeStringKeys().contains(QString::fromStdString(meshType)))
        m_problem->config()->setMeshType(meshTypeFromStringKey(QString::fromStdString(meshType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(meshTypeStringKeys())).toStdString());
}

void PyPreprocessor::setFrequency(double frequency)
{
    if (frequency > 0.0)
        m_problem->config()->setValue(ProblemConfig::Frequency, QString::number(frequency));
    else
        throw out_of_range(QObject::tr("The frequency must be positive.").toStdString());
}

void PyPreprocessor::setTimeStepMethod(const std::string &timeStepMethod)
{
    if (timeStepMethodStringKeys().contains(QString::fromStdString(timeStepMethod)))
        m_problem->config()->setValue(ProblemConfig::TimeMethod, (dealii::TimeStepping::runge_kutta_method) timeStepMethodFromStringKey(QString::fromStdString(timeStepMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(timeStepMethodStringKeys())).toStdString());
}

void PyPreprocessor::setTimeMethodTolerance(double timeMethodTolerance)
{
    if (timeMethodTolerance > 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeMethodTolerance, timeMethodTolerance);
    else
        throw out_of_range(QObject::tr("The time method tolerance must be positive.").toStdString());
}

void PyPreprocessor::setTimeMethodOrder(int timeMethodOrder)
{
    if (timeMethodOrder >= 1 && timeMethodOrder <= 3)
        m_problem->config()->setValue(ProblemConfig::TimeOrder, timeMethodOrder);
    else
        throw out_of_range(QObject::tr("Number of time method order must be greater than 1.").toStdString());
}

void PyPreprocessor::setTimeInitialTimeStep(double timeInitialTimeStep)
{
    if (timeInitialTimeStep > 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeInitialStepSize, timeInitialTimeStep);
    else
        throw out_of_range(QObject::tr("Initial time step must be positive.").toStdString());
}

void PyPreprocessor::setNumConstantTimeSteps(int timeSteps)
{
    if (timeSteps >= 1)
        m_problem->config()->setValue(ProblemConfig::TimeConstantTimeSteps, timeSteps);
    else
        throw out_of_range(QObject::tr("Number of time steps must be greater than 1.").toStdString());
}

void PyPreprocessor::setTimeTotal(double timeTotal)
{
    if (timeTotal >= 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeTotal, timeTotal);
    else
        throw out_of_range(QObject::tr("The total time must be positive.").toStdString());
}

void PyPreprocessor::setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type)
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (m_problem->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = m_problem->couplingInfo(source, target);
        if (couplingTypeStringKeys().contains(QString::fromStdString(type)))
            couplingInfo->setCouplingType(couplingTypeFromStringKey(QString::fromStdString(type)));
        else
            throw invalid_argument(QObject::tr("Invalid coupling type key. Valid keys: %1").arg(stringListToString(couplingTypeStringKeys())).toStdString());
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

PyComputation::PyComputation(bool newComputation) : PyProblem(false)
{    
    Agros2D::preprocessor()->createComputation(newComputation);
    // TODO: better!
    m_computation = Agros2D::computation();
}

void PyComputation::mesh()
{
    // Agros2D::computation()->scene()->invalidate();
    /*
    Agros2D::computation()->scene()->loopsInfo()->processPolygonTriangles(true);
    Agros2D::computation()->mesh(true);

    if (!Agros2D::computation()->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
    */

    //Agros2D::preprocessor()->createComputation(false);
    m_computation->mesh(true);
}

void PyComputation::solve()
{
    // Agros2D::computation()->scene()->invalidate();
    /*
    Agros2D::computation()->scene()->loopsInfo()->processPolygonTriangles(true);
    Agros2D::computation()->solve(false);

    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    */

    //Agros2D::preprocessor()->createComputation(false);
    m_computation->solve(true);
}

double PyComputation::timeElapsed() const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    double time = m_computation->timeElapsed().hour()*3600 + Agros2D::computation()->timeElapsed().minute()*60 +
                  m_computation->timeElapsed().second() + Agros2D::computation()->timeElapsed().msec() * 1e-3;
    return time;
}

void PyComputation::timeStepsLength(vector<double> &steps) const
{
    if (!m_computation->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    QList<double> timeStepLengths = m_computation->timeStepLengths();
    for (int i = 0; i < timeStepLengths.size(); i++)
        steps.push_back(timeStepLengths.at(i));
}

void PyComputation::timeStepsTimes(vector<double> &times) const
{
    if (!m_computation->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    QList<double> timeStepTimes = m_computation->timeStepTimes();
    for (int i = 0; i < timeStepTimes.size(); i++)
        times.push_back(timeStepTimes.at(i));
}

void PyComputation::clearSolution()
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    m_computation->clearSolution();
    m_computation->scene()->invalidate();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();
}

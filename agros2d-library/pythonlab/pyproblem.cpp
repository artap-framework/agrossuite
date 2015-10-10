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
    Agros2D::preprocessor()->scene()->clear();
}

void PyProblem::clearSolution()
{
    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    Agros2D::computation()->clearSolution();
    Agros2D::computation()->scene()->invalidate();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();
}

void PyProblem::refresh()
{
    Agros2D::preprocessor()->scene()->invalidate();
    currentPythonEngineAgros()->postDeal()->refresh();
}

void PyProblem::setCoordinateType(const std::string &coordinateType)
{
    if (coordinateTypeStringKeys().contains(QString::fromStdString(coordinateType)))
        Agros2D::preprocessor()->config()->setCoordinateType(coordinateTypeFromStringKey(QString::fromStdString(coordinateType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(coordinateTypeStringKeys())).toStdString());
}

void PyProblem::setMeshType(const std::string &meshType)
{
    if (meshTypeStringKeys().contains(QString::fromStdString(meshType)))
        Agros2D::preprocessor()->config()->setMeshType(meshTypeFromStringKey(QString::fromStdString(meshType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(meshTypeStringKeys())).toStdString());
}

void PyProblem::setFrequency(double frequency)
{
    if (frequency > 0.0)
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::Frequency, QString::number(frequency));
    else
        throw out_of_range(QObject::tr("The frequency must be positive.").toStdString());
}

void PyProblem::setTimeStepMethod(const std::string &timeStepMethod)
{
    if (timeStepMethodStringKeys().contains(QString::fromStdString(timeStepMethod)))
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeMethod, (dealii::TimeStepping::runge_kutta_method) timeStepMethodFromStringKey(QString::fromStdString(timeStepMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(timeStepMethodStringKeys())).toStdString());
}

void PyProblem::setTimeMethodTolerance(double timeMethodTolerance)
{
    if (timeMethodTolerance > 0.0)
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeMethodTolerance, timeMethodTolerance);
    else
        throw out_of_range(QObject::tr("The time method tolerance must be positive.").toStdString());
}

void PyProblem::setTimeMethodOrder(int timeMethodOrder)
{
    if (timeMethodOrder >= 1 && timeMethodOrder <= 3)
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeOrder, timeMethodOrder);
    else
        throw out_of_range(QObject::tr("Number of time method order must be greater than 1.").toStdString());
}

void PyProblem::setTimeInitialTimeStep(double timeInitialTimeStep)
{
    if (timeInitialTimeStep > 0.0)
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeInitialStepSize, timeInitialTimeStep);
    else
        throw out_of_range(QObject::tr("Initial time step must be positive.").toStdString());
}

void PyProblem::setNumConstantTimeSteps(int timeSteps)
{
    if (timeSteps >= 1)
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeConstantTimeSteps, timeSteps);
    else
        throw out_of_range(QObject::tr("Number of time steps must be greater than 1.").toStdString());
}

void PyProblem::setTimeTotal(double timeTotal)
{
    if (timeTotal >= 0.0)
        Agros2D::preprocessor()->config()->setValue(ProblemConfig::TimeTotal, timeTotal);
    else
        throw out_of_range(QObject::tr("The total time must be positive.").toStdString());
}

std::string PyProblem::getCouplingType(const std::string &sourceField, const std::string &targetField) const
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (Agros2D::preprocessor()->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = Agros2D::preprocessor()->couplingInfo(source, target);
        return couplingTypeToStringKey(couplingInfo->couplingType()).toStdString();
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

void PyProblem::setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type)
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (Agros2D::preprocessor()->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = Agros2D::preprocessor()->couplingInfo(source, target);
        if (couplingTypeStringKeys().contains(QString::fromStdString(type)))
            couplingInfo->setCouplingType(couplingTypeFromStringKey(QString::fromStdString(type)));
        else
            throw invalid_argument(QObject::tr("Invalid coupling type key. Valid keys: %1").arg(stringListToString(couplingTypeStringKeys())).toStdString());
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

void PyProblem::checkExistingFields(const QString &sourceField, const QString &targetField) const
{
    if (Agros2D::preprocessor()->fieldInfos().isEmpty())
        throw logic_error(QObject::tr("No fields are defined.").toStdString());

    if (!Agros2D::preprocessor()->fieldInfos().contains(sourceField))
        throw logic_error(QObject::tr("Source field '%1' is not defined.").arg(sourceField).toStdString());

    if (!Agros2D::preprocessor()->fieldInfos().contains(targetField))
        throw logic_error(QObject::tr("Target field '%1' is not defined.").arg(targetField).toStdString());
}

void PyProblem::mesh()
{
    // Agros2D::computation()->scene()->invalidate();
    Agros2D::computation()->scene()->loopsInfo()->processPolygonTriangles(true);
    Agros2D::computation()->mesh(true);

    if (!Agros2D::computation()->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
}

void PyProblem::solve()
{
    // Agros2D::computation()->scene()->invalidate();
    Agros2D::computation()->scene()->loopsInfo()->processPolygonTriangles(true);
    Agros2D::computation()->solve(false);

    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

double PyProblem::timeElapsed() const
{
    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    double time = Agros2D::computation()->timeElapsed().hour()*3600 + Agros2D::computation()->timeElapsed().minute()*60 +
                  Agros2D::computation()->timeElapsed().second() + Agros2D::computation()->timeElapsed().msec() * 1e-3;
    return time;
}

void PyProblem::timeStepsLength(vector<double> &steps) const
{
    if (!Agros2D::computation()->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    QList<double> timeStepLengths = Agros2D::computation()->timeStepLengths();
    for (int i = 0; i < timeStepLengths.size(); i++)
        steps.push_back(timeStepLengths.at(i));
}

void PyProblem::timeStepsTimes(vector<double> &times) const
{
    if (!Agros2D::computation()->isTransient())
        throw logic_error(QObject::tr("Problem is not transient.").toStdString());

    if (!Agros2D::computation()->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    QList<double> timeStepTimes = Agros2D::computation()->timeStepTimes();
    for (int i = 0; i < timeStepTimes.size(); i++)
        times.push_back(timeStepTimes.at(i));
}

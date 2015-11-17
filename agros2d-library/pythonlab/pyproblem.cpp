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
#include "solver/solutionstore.h"

void PyProblemBase::refresh()
{
    m_problemBase->scene()->invalidate();
}

void PyProblemBase::getParameters(std::vector<std::string> &keys) const
{
    ParametersType parameters = m_problemBase->config()->value(ProblemConfig::Parameters).value<ParametersType>();

    foreach (QString key, parameters.keys())
        keys.push_back(key.toStdString());
}

double PyProblemBase::getParameter(std::string key) const
{
    ParametersType parameters = m_problemBase->config()->value(ProblemConfig::Parameters).value<ParametersType>();

    if (parameters.contains(QString::fromStdString(key)))
    {
        return parameters[QString::fromStdString(key)];
    }
    else
    {
        QString str;
        foreach (QString key, parameters.keys())
            str += key + ", ";
        if (str.length() > 0)
            str = str.left(str.length() - 2);

        throw logic_error(QObject::tr("Invalid argument. Valid keys: %1").arg(str).toStdString());
    }
}

std::string PyProblemBase::getCouplingType(const std::string &sourceField, const std::string &targetField) const
{
    QString source = QString::fromStdString(sourceField);
    QString target = QString::fromStdString(targetField);

    checkExistingFields(source, target);

    if (m_problemBase->hasCoupling(source, target))
    {
        CouplingInfo *couplingInfo = m_problemBase->couplingInfo(source, target);
        return couplingTypeToStringKey(couplingInfo->couplingType()).toStdString();
    }
    else
        throw logic_error(QObject::tr("Coupling '%1' + '%2' doesn't exists.").arg(source).arg(target).toStdString());
}

void PyProblemBase::checkExistingFields(const QString &sourceField, const QString &targetField) const
{
    if (m_problemBase->fieldInfos().isEmpty())
        throw logic_error(QObject::tr("No fields are defined.").toStdString());

    if (!m_problemBase->fieldInfos().contains(sourceField))
        throw logic_error(QObject::tr("Source field '%1' is not defined.").arg(sourceField).toStdString());

    if (!m_problemBase->fieldInfos().contains(targetField))
        throw logic_error(QObject::tr("Target field '%1' is not defined.").arg(targetField).toStdString());
}

PyProblem::PyProblem(bool clearProblem) : PyProblemBase()
{
    m_problem = QSharedPointer<Problem>(Agros2D::problem());
    m_problemBase = m_problem;

    if (clearProblem)
        clear();
}

void PyProblem::clear()
{
    m_problem->clearFieldsAndConfig();
}

void PyProblem::setParameter(std::string key, double value)
{
    ParametersType parameters = m_problem->config()->value(ProblemConfig::Parameters).value<ParametersType>();
    parameters[QString::fromStdString(key)] = value;

    Agros2D::problem()->config()->setValue(ProblemConfig::Parameters, parameters);
}

void PyProblem::setCoordinateType(const std::string &coordinateType)
{
    if (coordinateTypeStringKeys().contains(QString::fromStdString(coordinateType)))
        m_problem->config()->setCoordinateType(coordinateTypeFromStringKey(QString::fromStdString(coordinateType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(coordinateTypeStringKeys())).toStdString());
}

void PyProblem::setMeshType(const std::string &meshType)
{
    if (meshTypeStringKeys().contains(QString::fromStdString(meshType)))
        m_problem->config()->setMeshType(meshTypeFromStringKey(QString::fromStdString(meshType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(meshTypeStringKeys())).toStdString());
}

void PyProblem::setFrequency(double frequency)
{
    if (frequency > 0.0)
        m_problem->config()->setValue(ProblemConfig::Frequency, QString::number(frequency));
    else
        throw out_of_range(QObject::tr("The frequency must be positive.").toStdString());
}

void PyProblem::setTimeStepMethod(const std::string &timeStepMethod)
{
    if (timeStepMethodStringKeys().contains(QString::fromStdString(timeStepMethod)))
        m_problem->config()->setValue(ProblemConfig::TimeMethod, (dealii::TimeStepping::runge_kutta_method) timeStepMethodFromStringKey(QString::fromStdString(timeStepMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(timeStepMethodStringKeys())).toStdString());
}

void PyProblem::setTimeMethodTolerance(double timeMethodTolerance)
{
    if (timeMethodTolerance > 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeMethodTolerance, timeMethodTolerance);
    else
        throw out_of_range(QObject::tr("The time method tolerance must be positive.").toStdString());
}

void PyProblem::setTimeMethodOrder(int timeMethodOrder)
{
    if (timeMethodOrder >= 1 && timeMethodOrder <= 3)
        m_problem->config()->setValue(ProblemConfig::TimeOrder, timeMethodOrder);
    else
        throw out_of_range(QObject::tr("Number of time method order must be greater than 1.").toStdString());
}

void PyProblem::setTimeInitialTimeStep(double timeInitialTimeStep)
{
    if (timeInitialTimeStep > 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeInitialStepSize, timeInitialTimeStep);
    else
        throw out_of_range(QObject::tr("Initial time step must be positive.").toStdString());
}

void PyProblem::setNumConstantTimeSteps(int timeSteps)
{
    if (timeSteps >= 1)
        m_problem->config()->setValue(ProblemConfig::TimeConstantTimeSteps, timeSteps);
    else
        throw out_of_range(QObject::tr("Number of time steps must be greater than 1.").toStdString());
}

void PyProblem::setTimeTotal(double timeTotal)
{
    if (timeTotal >= 0.0)
        m_problem->config()->setValue(ProblemConfig::TimeTotal, timeTotal);
    else
        throw out_of_range(QObject::tr("The total time must be positive.").toStdString());
}

void PyProblem::setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type)
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

PyComputation::PyComputation() : PyProblemBase()
{
    Agros2D::problem()->createComputation(true);
    m_computation = Agros2D::problem()->m_currentComputation;
    m_problemBase = m_computation;
}

PyComputation::PyComputation(std::string computation) : PyProblemBase()
{
    QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
    QString key = QString::fromStdString(computation);
    if (computations.contains(key))
    {
        m_computation = computations[key];
        m_problemBase = m_computation;
    }
    else
    {
        throw logic_error(QObject::tr("Computation '%1' does not exists.").arg(key).toStdString());
    }
}

QSharedPointer<Computation> PyComputation::getComputation()
{
    return m_computation;
}

void PyComputation::clear()
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    m_computation->clearSolution();
    m_computation->scene()->invalidate();

    if (!silentMode())
        currentPythonEngineAgros()->sceneViewPreprocessor()->actSceneModePreprocessor->trigger();
}


void PyComputation::mesh()
{
    m_computation->scene()->invalidate();
    m_computation->scene()->loopsInfo()->processPolygonTriangles(true);
    m_computation->mesh(true);

    if (!m_computation->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());
}

void PyComputation::solve()
{
    m_computation->scene()->invalidate();
    m_computation->scene()->loopsInfo()->processPolygonTriangles(true);
    m_computation->solve();

    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
}

double PyComputation::timeElapsed() const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    double time = m_computation->timeElapsed().hour()*3600 + m_computation->timeElapsed().minute()*60 +
            m_computation->timeElapsed().second() + m_computation->timeElapsed().msec() * 1e-3;
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

void PySolution::setComputation(PyComputation *computation)
{
    m_computation = computation->getComputation();
}

void PySolution::setField(const std::string &fieldId)
{
    QString id = QString::fromStdString(fieldId);
    if (m_computation->hasField(id))
        m_fieldInfo = m_computation->fieldInfo(id);
    else
        throw invalid_argument(QObject::tr("Invalid field id. Field %1 is not defined.").arg(id).toStdString());
}

int PySolution::getTimeStep(int timeStep) const
{
    if (timeStep == -1)
        timeStep = m_computation->solutionStore()->lastTimeStep(m_fieldInfo);
    else if (timeStep < 0 || timeStep >= m_computation->timeStepLengths().length() - 1)
        throw out_of_range(QObject::tr("Time step must be in the range from 0 to %1.").arg(m_computation->timeStepLengths().length() - 1).toStdString());


    return timeStep;
}

int PySolution::getAdaptivityStep(int adaptivityStep, int timeStep) const
{
    if (adaptivityStep == -1)
        adaptivityStep = m_computation->solutionStore()->lastAdaptiveStep(m_fieldInfo, timeStep);
    else if (adaptivityStep < 0 || adaptivityStep > m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() - 1)
        throw out_of_range(QObject::tr("Adaptivity step is out of range. (0 to %1).").arg(m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt() - 1).toStdString());

    return adaptivityStep;
}

void PyComputation::getResults(std::vector<std::string> &keys) const
{
    QMap<QString, double> results = m_computation->result()->results();

    foreach (QString key, results.keys())
        keys.push_back(key.toStdString());
}

double PyComputation::getResult(std::string key) const
{
    QMap<QString, double> results = m_computation->result()->results();

    if (results.contains(QString::fromStdString(key)))
    {
        return results[QString::fromStdString(key)];
    }
    else
    {
        QString str;
        foreach (QString key, results.keys())
            str += key + ", ";
        if (str.length() > 0)
            str = str.left(str.length() - 2);

        throw logic_error(QObject::tr("Invalid argument. Valid keys: %1").arg(str).toStdString());
    }
}

void PyComputation::setResult(std::string key, double value)
{
    QMap<QString, double> results = m_computation->result()->results();
    results[QString::fromStdString(key)] = value;
}

void PySolution::localValues(double x, double y, int timeStep, int adaptivityStep, map<std::string, double> &results) const
{
    map<std::string, double> values;

    if (m_computation->isSolved())
    {
        Point point(x, y);

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

        std::shared_ptr<LocalValue> value = m_fieldInfo->plugin()->localValue(m_computation.data(), m_fieldInfo, timeStep, adaptivityStep, point);
        QMapIterator<QString, LocalPointValue> it(value->values());
        while (it.hasNext())
        {
            it.next();
            Module::LocalVariable variable = m_fieldInfo->localVariable(m_computation->config()->coordinateType(), it.key());

            if (variable.isScalar())
            {
                values[variable.shortname().toStdString()] = it.value().scalar;
            }
            else
            {
                values[variable.shortname().toStdString()] = it.value().vector.magnitude();
                values[variable.shortname().toStdString() + m_computation->config()->labelX().toLower().toStdString()] = it.value().vector.x;
                values[variable.shortname().toStdString() + m_computation->config()->labelY().toLower().toStdString()] = it.value().vector.y;
            }
        }
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PySolution::surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                                  map<std::string, double> &results) const
{
    map<std::string, double> values;

    if (m_computation->isSolved())
    {
        m_computation->scene()->selectNone();

        if (!edges.empty())
        {
            for (vector<int>::const_iterator it = edges.begin(); it != edges.end(); ++it)
            {
                if ((*it >= 0) && (*it < m_computation->scene()->edges->length()))
                {
                    m_computation->scene()->edges->at(*it)->setSelected(true);
                }
                else
                {
                    throw out_of_range(QObject::tr("Edge index must be between 0 and '%1'.").arg(m_computation->scene()->edges->length()-1).toStdString());
                    results = values;
                    return;
                }
            }

            // if (!silentMode() && !m_computation->isSolving())
            //     currentPythonEngineAgros()->sceneViewPost2D()->updateGL();
        }
        else
        {
            m_computation->scene()->selectAll(SceneGeometryMode_OperateOnEdges);
        }

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

        std::shared_ptr<IntegralValue> integral = m_fieldInfo->plugin()->surfaceIntegral(m_computation.data(), m_fieldInfo, timeStep, adaptivityStep);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();
            Module::Integral integral = m_fieldInfo->surfaceIntegral(m_computation->config()->coordinateType(), it.key());
            values[integral.shortname().toStdString()] = it.value();
        }
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PySolution::volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                                 map<std::string, double> &results) const
{
    map<std::string, double> values;

    if (m_computation->isSolved())
    {
        m_computation->scene()->selectNone();

        if (!labels.empty())
        {
            for (vector<int>::const_iterator it = labels.begin(); it != labels.end(); ++it)
            {
                if ((*it >= 0) && (*it < m_computation->scene()->labels->length()))
                {
                    if (m_computation->scene()->labels->at(*it)->marker(m_fieldInfo) != m_computation->scene()->materials->getNone(m_fieldInfo))
                        m_computation->scene()->labels->at(*it)->setSelected(true);
                    else
                        throw out_of_range(QObject::tr("Label with index '%1' is 'none'.").arg(*it).toStdString());
                }
                else
                {
                    throw out_of_range(QObject::tr("Label index must be between 0 and '%1'.").arg(m_computation->scene()->labels->length()-1).toStdString());
                    results = values;
                    return;
                }
            }

            // if (!silentMode() && !m_computation->isSolving())
            //     currentPythonEngineAgros()->sceneViewPost2D()->updateGL();
        }
        else
        {
            m_computation->scene()->selectAll(SceneGeometryMode_OperateOnLabels);
        }

        // set time and adaptivity step if -1 (default parameter - last steps), check steps
        timeStep = getTimeStep(timeStep);
        adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

        std::shared_ptr<IntegralValue> integral = m_fieldInfo->plugin()->volumeIntegral(m_computation.data(), m_fieldInfo, timeStep, adaptivityStep);
        QMapIterator<QString, double> it(integral->values());
        while (it.hasNext())
        {
            it.next();
            Module::Integral integral = m_fieldInfo->volumeIntegral(m_computation->config()->coordinateType(), it.key());
            values[integral.shortname().toStdString()] = it.value();
        }
    }
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }

    results = values;
}

void PySolution::initialMeshInfo(map<std::string, int> &info) const
{
    if (!m_computation->isMeshed())
        throw logic_error(QObject::tr("Problem is not meshed.").toStdString());

    // todo: initial mesh the same for all fields
    info["nodes"] = m_computation->initialMesh().n_used_vertices();
    info["elements"] = m_computation->initialMesh().n_active_cells();
}

void PySolution::solutionMeshInfo(int timeStep, int adaptivityStep, map<std::string, int> &info) const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    // set time and adaptivity step if -1 (default parameter - last steps), check steps
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    // TODO: (Franta) time and adaptivity step in gui vs. implementation
    MultiArray ma = m_computation->solutionStore()->multiArray(FieldSolutionID(m_fieldInfo->fieldId(), timeStep, adaptivityStep));

    info["nodes"] = ma.doFHandler()->get_tria().n_used_vertices();
    info["elements"] = ma.doFHandler()->get_tria().n_active_cells();
    info["dofs"] = ma.doFHandler()->n_dofs();
}

void PySolution::solverInfo(int timeStep, int adaptivityStep,
                            vector<double> &solutionsChange, vector<double> &residual,
                            vector<double> &dampingCoeff, int &jacobianCalculations) const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    // step if -1 (default parameter - last steps)
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    SolutionStore::SolutionRunTimeDetails runTime = m_computation->solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(m_fieldInfo->fieldId(), timeStep, adaptivityStep));

    /*
    for (int i = 0; i < runTime.relativeChangeOfSolutions().size(); i++)
        solutionsChange.push_back(runTime.relativeChangeOfSolutions().at(i));

    for (int i = 0; i < runTime.newtonResidual().size(); i++)
        residual.push_back(runTime.newtonResidual().at(i));

    for (int i = 0; i < runTime.nonlinearDamping().size(); i++)
        dampingCoeff.push_back(runTime.nonlinearDamping().at(i));
    jacobianCalculations = runTime.jacobianCalculations();
     */
}

void PySolution::adaptivityInfo(int timeStep, vector<double> &error, vector<int> &dofs) const
{
    if (!m_computation->isSolved())
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());

    if (m_fieldInfo->adaptivityType() == AdaptivityMethod_None)
        throw logic_error(QObject::tr("Solution is not adaptive.").toStdString());

    // set time step if -1 (default parameter - last steps)
    timeStep = getTimeStep(timeStep);

    int adaptivitySteps = m_computation->solutionStore()->lastAdaptiveStep(m_fieldInfo, timeStep) + 1;
    for (int i = 0; i < adaptivitySteps; i++)
    {
        SolutionStore::SolutionRunTimeDetails runTime = m_computation->solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(m_fieldInfo->fieldId(), timeStep, i));
        /*
        error.push_back(runTime.adaptivityError());
        dofs.push_back(runTime.DOFs());
        */
    }
}

std::string PySolution::filenameMatrix(int timeStep, int adaptivityStep) const
{
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    qDebug() << "TODO: add time and adaptive step";
    // QString name = QString("%1/%2_%3_%4_matrix.mat").arg(cacheProblemDir()).arg(m_fieldInfo->fieldId()).arg(timeStep).arg(adaptivityStep);
    QString name = QString("%1/%2_matrix.mat").arg(cacheProblemDir()).arg(m_fieldInfo->fieldId());
    if (QFile::exists(name))
        return name.toStdString();
    else
        throw logic_error(QObject::tr("Matrix file does not exist.").toStdString());
}

std::string PySolution::filenameRHS(int timeStep, int adaptivityStep) const
{
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    qDebug() << "TODO: add time and adaptive step";
    // QString name = QString("%1/%2_%3_%4_RHS").arg(cacheProblemDir()).arg(m_fieldInfo->fieldId()).arg(timeStep).arg(adaptivityStep);
    QString name = QString("%1/%2_rhs.mat").arg(cacheProblemDir()).arg(m_fieldInfo->fieldId());
    if (QFile::exists(name))
        return name.toStdString();
    else
        throw logic_error(QObject::tr("RHS file does not exist.").toStdString());
}

std::string PySolution::filenameSLN(int timeStep, int adaptivityStep) const
{
    timeStep = getTimeStep(timeStep);
    adaptivityStep = getAdaptivityStep(adaptivityStep, timeStep);

    qDebug() << "TODO: add time and adaptive step";
    // QString name = QString("%1/%2_%3_%4_RHS").arg(cacheProblemDir()).arg(m_fieldInfo->fieldId()).arg(timeStep).arg(adaptivityStep);
    QString name = QString("%1/%2_sln.mat").arg(cacheProblemDir()).arg(m_fieldInfo->fieldId());
    if (QFile::exists(name))
        return name.toStdString();
    else
        throw logic_error(QObject::tr("RHS file does not exist.").toStdString());
}

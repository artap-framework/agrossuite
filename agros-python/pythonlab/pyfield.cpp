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

#include "pyfield.h"
#include "solver/problem.h"
#include "solver/plugin_interface.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"
#include "solver/problem_result.h"

PyField::PyField(std::string fieldId)
{
    QMap<QString, QString> modules = Module::availableModules();
    QString id = QString::fromStdString(fieldId);

    if (modules.keys().contains(id))
    {
        if (Agros::problem()->hasField(id))
        {
            m_fieldInfo = Agros::problem()->fieldInfo(id);
        }
        else
        {
            try
            {
                m_fieldInfo = new FieldInfo(id);
            }
            catch (AgrosPluginException& e)
            {
                throw invalid_argument(QObject::tr("Invalid field id. Plugin %1 cannot be loaded").arg(id).toStdString());
            }

            Agros::problem()->addField(m_fieldInfo);
        }
    }
    else
    {
        throw invalid_argument(QObject::tr("Invalid field id. Valid keys: %1").arg(stringListToString(modules.keys())).toStdString());
    }
}

void PyField::setAnalysisType(const std::string &analysisType)
{
    std::string at = analysisType;

    if (m_fieldInfo->analyses().contains(analysisTypeFromStringKey(QString::fromStdString(at))))
    {
        m_fieldInfo->setAnalysisType(analysisTypeFromStringKey(QString::fromStdString(at)));
    }
    else
    {
        QStringList list;
        foreach (AnalysisType analysis, m_fieldInfo->analyses().keys())
            list.append(analysisTypeToStringKey(analysis));

        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(list)).toStdString());
    }
}

void PyField::setNumberOfRefinements(int numberOfRefinements)
{
    if (numberOfRefinements >= 0 && numberOfRefinements <= 10)
        m_fieldInfo->setValue(FieldInfo::SpaceNumberOfRefinements, numberOfRefinements);
    else
        throw out_of_range(QObject::tr("Number of refinements is out of range (0 - 10).").toStdString());
}

void PyField::setPolynomialOrder(int polynomialOrder)
{
    if (polynomialOrder > 0 && polynomialOrder <= 10)
        m_fieldInfo->setValue(FieldInfo::SpacePolynomialOrder, polynomialOrder);
    else
        throw out_of_range(QObject::tr("Polynomial order is out of range (1 - 10).").toStdString());
}

void PyField::setLinearityType(const std::string &linearityType)
{

    if (linearityTypeStringKeys().contains(QString::fromStdString(linearityType)))
        m_fieldInfo->setLinearityType(linearityTypeFromStringKey(QString::fromStdString(linearityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(linearityTypeStringKeys())).toStdString());
}

void PyField::setNonlinearDampingType(std::string dampingType)
{
    if (dampingTypeStringKeys().contains(QString::fromStdString(dampingType)))
        m_fieldInfo->setValue(FieldInfo::NonlinearDampingType, (DampingType) dampingTypeFromStringKey(QString::fromStdString(dampingType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(dampingTypeStringKeys())).toStdString());
}

void PyField::setAdaptivityType(const std::string &adaptivityType)
{
    if (adaptivityTypeStringKeys().contains(QString::fromStdString(adaptivityType)))
        m_fieldInfo->setAdaptivityType((AdaptivityMethod) adaptivityTypeFromStringKey(QString::fromStdString(adaptivityType)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityTypeStringKeys())).toStdString());
}

void PyField::setMatrixSolver(const std::string &matrixSolver)
{
    if (matrixSolverTypeStringKeys().contains(QString::fromStdString(matrixSolver)))
        m_fieldInfo->setMatrixSolver(matrixSolverTypeFromStringKey(QString::fromStdString(matrixSolver)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(matrixSolverTypeStringKeys())).toStdString());
}

void PyField::setLinearSolverDealIIMethod(const std::string &linearSolverMethod)
{
    if (iterLinearSolverDealIIMethodStringKeys().contains(QString::fromStdString(linearSolverMethod)))
        m_fieldInfo->setValue(FieldInfo::LinearSolverIterDealIIMethod,
                              (IterSolverDealII) iterLinearSolverDealIIMethodFromStringKey(QString::fromStdString(linearSolverMethod)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(iterLinearSolverDealIIMethodStringKeys())).toStdString());
}

void PyField::setLinearSolverDealIIPreconditioner(const std::string &linearSolverPreconditioner)
{
    if (iterLinearSolverDealIIPreconditionerStringKeys().contains(QString::fromStdString(linearSolverPreconditioner)))
        m_fieldInfo->setValue(FieldInfo::LinearSolverIterDealIIPreconditioner,
                              (PreconditionerDealII) iterLinearSolverDealIIPreconditionerFromStringKey(QString::fromStdString(linearSolverPreconditioner)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(iterLinearSolverDealIIPreconditionerStringKeys())).toStdString());
}

void PyField::setExternalMatrixSolver(const std::string &solver)
{
    QStringList solvers;
    QDirIterator itDir(QString("%1/libs/").arg(Agros::dataDir()), QStringList() << "*.ext", QDir::Files);
    while (itDir.hasNext())
        solvers << QFileInfo(itDir.next()).fileName();

    if (solvers.contains(QString::fromStdString(solver)))
        m_fieldInfo->setValue(FieldInfo::LinearSolverExternalName, QString::fromStdString(solver));
    else if (m_fieldInfo->matrixSolver() == SOLVER_EXTERNAL)
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(solvers)).toStdString());
}

void PyField::setAdaptivityEstimator(const std::string &adaptivityEstimator)
{
    if (adaptivityEstimatorStringKeys().contains(QString::fromStdString(adaptivityEstimator)))
        m_fieldInfo->setValue(FieldInfo::AdaptivityEstimator, (AdaptivityEstimator) adaptivityEstimatorFromStringKey(QString::fromStdString(adaptivityEstimator)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityEstimatorStringKeys())).toStdString());
}

void PyField::setAdaptivityStrategy(const std::string &adaptivityStrategy)
{
    if (adaptivityStrategyStringKeys().contains(QString::fromStdString(adaptivityStrategy)))
        m_fieldInfo->setValue(FieldInfo::AdaptivityStrategy, (AdaptivityStrategy) adaptivityStrategyFromStringKey(QString::fromStdString(adaptivityStrategy)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityStrategyStringKeys())).toStdString());
}

void PyField::setAdaptivityStrategyHP(const std::string &adaptivityStrategyHP)
{
    if (adaptivityStrategyHPStringKeys().contains(QString::fromStdString(adaptivityStrategyHP)))
        m_fieldInfo->setValue(FieldInfo::AdaptivityStrategyHP, (AdaptivityStrategyHP) adaptivityStrategyHPFromStringKey(QString::fromStdString(adaptivityStrategyHP)));
    else
        throw invalid_argument(QObject::tr("Invalid argument. Valid keys: %1").arg(stringListToString(adaptivityStrategyHPStringKeys())).toStdString());
}

void PyField::setInitialCondition(double initialCondition)
{
    m_fieldInfo->setValue(FieldInfo::TransientInitialCondition, initialCondition);
}

void PyField::setTimeSkip(double timeSkip)
{
    if (timeSkip >= 0)
        m_fieldInfo->setValue(FieldInfo::TransientTimeSkip, timeSkip);
    else
        throw out_of_range(QObject::tr("Time skip must be greater than or equal to zero.").toStdString());
}

void PyField::addBoundary(const std::string &name, const std::string &type,
                          const map<std::string, std::string> &parameters,
                          const map<std::string, std::string> &expressions)
{
    // check boundaries with same name
    foreach (SceneBoundary *boundary, Agros::problem()->scene()->boundaries->filter(m_fieldInfo->fieldId()).items())
    {
        if (boundary->name() == QString::fromStdString(name))
            throw invalid_argument(QObject::tr("Boundary condition '%1' already exists.").arg(QString::fromStdString(name)).toStdString());
    }

    if (!m_fieldInfo->boundaryTypeContains(QString::fromStdString(type)))
        throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(QString::fromStdString(type)).toStdString());

    Module::BoundaryType boundaryType = m_fieldInfo->boundaryType(QString::fromStdString(type));

    // browse boundary parameters
    QMap<QString, Value> values;
    for (map<std::string, std::string>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    values[variable.id()] = Value(Agros::problem(), QString::fromStdString((*i).second));
                else
                    values[variable.id()] = Value(Agros::problem(), QString::fromStdString(expressions.at((*i).first)));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }

    Agros::problem()->scene()->addBoundary(new SceneBoundary(Agros::problem()->scene(), m_fieldInfo, QString::fromStdString(name), QString::fromStdString(type), values));
    Agros::problem()->scene()->invalidate();
}

void PyField::modifyBoundary(const std::string &name, const std::string &type,
                             const map<std::string, std::string> &parameters,
                             const map<std::string, std::string> &expressions)
{
    SceneBoundary *sceneBoundary = Agros::problem()->scene()->getBoundary(m_fieldInfo, QString::fromStdString(name));
    if (!sceneBoundary)
        throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    // browse boundary types
    foreach (Module::BoundaryType boundaryType, sceneBoundary->fieldInfo()->boundaryTypes())
    {
        if (QString::fromStdString(type) == boundaryType.id())
        {
            sceneBoundary->setType(QString::fromStdString(type));
            break;
        }
        else
            throw invalid_argument(QObject::tr("Wrong boundary type '%1'.").arg(QString::fromStdString(type)).toStdString());
    }

    // browse boundary parameters
    Module::BoundaryType boundaryType = m_fieldInfo->boundaryType(sceneBoundary->type());
    for (map<std::string, std::string>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                assigned = true;
                if (expressions.count((*i).first) == 0)
                    sceneBoundary->modifyValue(QString::fromStdString((*i).first), Value(Agros::problem(), QString::fromStdString((*i).second)));
                else
                    sceneBoundary->modifyValue(QString::fromStdString((*i).first), Value(Agros::problem(), QString::fromStdString(expressions.at((*i).first))));
                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }

    Agros::problem()->scene()->invalidate();
}

void PyField::removeBoundary(const std::string &name)
{
    SceneBoundary *sceneBoundary = Agros::problem()->scene()->getBoundary(m_fieldInfo, QString::fromStdString(name));
    if (sceneBoundary == NULL)
        throw invalid_argument(QObject::tr("Boundary condition '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    Agros::problem()->scene()->removeBoundary(sceneBoundary);
    Agros::problem()->scene()->invalidate();
}

void PyField::addMaterial(const std::string &name, const map<std::string, std::string> &parameters,
                          const map<std::string, std::string> &expressions,
                          const map<std::string, vector<double> > &nonlin_x,
                          const map<std::string, vector<double> > &nonlin_y,
                          const map<string, map<string, string> > &settings_map)
{
    // check materials with same name
    foreach (SceneMaterial *material, Agros::problem()->scene()->materials->filter(m_fieldInfo->fieldId()).items())
    {
        if (material->name() == QString::fromStdString(name))
            throw invalid_argument(QObject::tr("Material '%1' already exists.").arg(QString::fromStdString(name)).toStdString());
    }

    // browse material parameters
    QList<Module::MaterialTypeVariable> variables = m_fieldInfo->materialTypeVariables();
    QMap<QString, Value> values;
    for (map<std::string, std::string>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        bool assigned = false;
        foreach (Module::MaterialTypeVariable variable, variables)
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x.at((*i).first).size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y.at((*i).first).size() : 0);
                if (lenx != leny)
                {
                    if (lenx > leny)
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                    else
                        throw invalid_argument(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());
                }

                DataTableType dataTableType = DataTableType_PiecewiseLinear;
                bool splineFirstDerivatives = true;
                bool extrapolateConstant = true;

                if (settings_map.find((*i).first) != settings_map.end())
                {
                    map<string, string> settings = settings_map.at((*i).first);
                    for (map<std::string, string>::const_iterator is = settings.begin(); is != settings.end(); ++is)
                    {
                        if (QString::fromStdString((*is).first) == "interpolation")
                        {
                            if (!dataTableTypeStringKeys().contains(QString::fromStdString((*is).second)))
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters: %2").arg(QString::fromStdString((*is).second))
                                                       .arg(stringListToString(dataTableTypeStringKeys())).toStdString());
                            dataTableType = dataTableTypeFromStringKey(QString::fromStdString((*is).second));
                            assert(dataTableType != DataTableType_Undefined);
                        }

                        if (QString::fromStdString((*is).first) == "extrapolation")
                        {
                            if (QString::fromStdString((*is).second) == "constant")
                                extrapolateConstant = true;
                            else if (QString::fromStdString((*is).second) == "linear")
                                extrapolateConstant = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'constant' or 'linear'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }

                        if (QString::fromStdString((*is).first) == "derivative_at_endpoints")
                        {
                            if (QString::fromStdString((*is).second) == "first")
                                splineFirstDerivatives = true;
                            else if (QString::fromStdString((*is).second) == "second")
                                splineFirstDerivatives = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'first' or 'second'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }
                    }
                }

                assigned = true;

                try
                {
                    if (expressions.count((*i).first) == 0)
                    {
                        values[variable.id()] = Value(Agros::problem(),
                                                      QString::fromStdString((*i).second),
                                                      (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                      (leny > 0) ? nonlin_y.at((*i).first) : vector<double>(),
                                                      dataTableType, splineFirstDerivatives, extrapolateConstant);
                    }
                    else
                    {
                        values[variable.id()] = Value(Agros::problem(),
                                                      QString::fromStdString(expressions.at((*i).first)),
                                                      (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                      (leny > 0) ? nonlin_y.at((*i).first) : vector<double>(),
                                                      dataTableType, splineFirstDerivatives, extrapolateConstant);
                    }
                }
                catch (AgrosException e)
                {
                    throw invalid_argument(e.toString().toStdString());
                }

                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }

    Agros::problem()->scene()->addMaterial(new SceneMaterial(Agros::problem()->scene(), m_fieldInfo, QString::fromStdString(name), values));
    Agros::problem()->scene()->invalidate();
}

void PyField::modifyMaterial(const std::string &name, const map<string, std::string> &parameters,
                             const map<std::string, std::string> &expressions,
                             const map<std::string, vector<double> > &nonlin_x,
                             const map<std::string, vector<double> > &nonlin_y,
                             const map<string, map<string, string> > &settings_map)
{
    SceneMaterial *sceneMaterial = Agros::problem()->scene()->getMaterial(m_fieldInfo, QString::fromStdString(name));

    if (!sceneMaterial)
        throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    for (map<std::string, std::string>::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
    {
        QList<Module::MaterialTypeVariable> materialVariables = m_fieldInfo->materialTypeVariables();

        bool assigned = false;
        foreach (Module::MaterialTypeVariable variable, materialVariables)
        {
            if (variable.id() == QString::fromStdString((*i).first))
            {
                int lenx = ((nonlin_x.find((*i).first) != nonlin_x.end()) ? nonlin_x.at((*i).first).size() : 0);
                int leny = ((nonlin_y.find((*i).first) != nonlin_y.end()) ? nonlin_y.at((*i).first).size() : 0);
                if (lenx > leny)
                    throw invalid_argument(QObject::tr("Size doesn't match (%1 > %2).").arg(lenx).arg(leny).toStdString());
                else if (lenx < leny)
                    throw invalid_argument(QObject::tr("Size doesn't match (%1 < %2).").arg(lenx).arg(leny).toStdString());

                DataTableType dataTableType = DataTableType_PiecewiseLinear;
                bool splineFirstDerivatives = true;
                bool extrapolateConstant = true;

                if (settings_map.find((*i).first) != settings_map.end())
                {
                    map<string, string> settings = settings_map.at((*i).first);
                    for (map<std::string, string>::const_iterator is = settings.begin(); is != settings.end(); ++is)
                    {
                        if (QString::fromStdString((*is).first) == "interpolation")
                        {
                            dataTableType = dataTableTypeFromStringKey(QString::fromStdString((*is).second));
                            assert(dataTableType != DataTableType_Undefined);
                        }

                        if (QString::fromStdString((*is).first) == "extrapolation")
                        {
                            if (QString::fromStdString((*is).second) == "constant")
                                extrapolateConstant = true;
                            else if (QString::fromStdString((*is).second) == "linear")
                                extrapolateConstant = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'constant' or 'linear'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }

                        if (QString::fromStdString((*is).first) == "derivative_at_endpoints")
                        {
                            if (QString::fromStdString((*is).second) == "first")
                                splineFirstDerivatives = true;
                            else if (QString::fromStdString((*is).second) == "second")
                                splineFirstDerivatives = false;
                            else
                                throw invalid_argument(QObject::tr("Invalid parameter '%1'. Valid parameters are 'first' or 'second'.").arg(QString::fromStdString((*is).second)).toStdString());
                        }
                    }
                }

                assigned = true;

                try
                {
                    if (expressions.count((*i).first) == 0)
                    {
                        sceneMaterial->modifyValue(QString::fromStdString((*i).first), Value(sceneMaterial->valueNakedPtr(QString::fromStdString((*i).second))->problem(),
                                                                                             QString::fromStdString((*i).second),
                                                                                             (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                                                             (leny > 0) ? nonlin_y.at((*i).first) : vector<double>()));
                    }
                    else
                    {
                        sceneMaterial->modifyValue(QString::fromStdString((*i).first), Value(sceneMaterial->valueNakedPtr(QString::fromStdString((*i).second))->problem(),
                                                                                             QString::fromStdString(expressions.at((*i).first)),
                                                                                             (lenx > 0) ? nonlin_x.at((*i).first) : vector<double>(),
                                                                                             (leny > 0) ? nonlin_y.at((*i).first) : vector<double>()));
                    }
                }
                catch (AgrosException e)
                {
                    throw invalid_argument(e.toString().toStdString());
                }

                break;
            }
        }

        if (!assigned)
            throw invalid_argument(QObject::tr("Wrong parameter '%1'.").arg(QString::fromStdString((*i).first)).toStdString());
    }

    Agros::problem()->scene()->invalidate();
}

void PyField::removeMaterial(const std::string &name)
{
    SceneMaterial *sceneMaterial = Agros::problem()->scene()->getMaterial(m_fieldInfo, QString::fromStdString(name));
    if (!sceneMaterial)
        throw invalid_argument(QObject::tr("Material '%1' doesn't exists.").arg(QString::fromStdString(name)).toStdString());

    Agros::problem()->scene()->removeMaterial(sceneMaterial);
    Agros::problem()->scene()->invalidate();
}

void PyField::addRecipeVolumeIntegral(const std::string &name, const std::string &variable, const vector<int> labels, int timeStep, int adaptivityStep)
{
    // TODO: checks
    VolumeIntegralRecipe *recipe = new VolumeIntegralRecipe(QString::fromStdString(name),
                                                            m_fieldInfo->fieldId(),
                                                            QString::fromStdString(variable),
                                                            timeStep,
                                                            adaptivityStep);

    for (int i = 0; i < labels.size(); i++)
        recipe->addLabel(labels[i]);

    Agros::problem()->recipes()->addRecipe(recipe);
}

void PyField::addRecipeSurfaceIntegral(const std::string &name, const std::string &variable, const vector<int> edges, int timeStep, int adaptivityStep)
{
    // TODO: checks
    SurfaceIntegralRecipe *recipe = new SurfaceIntegralRecipe(QString::fromStdString(name),
                                                              m_fieldInfo->fieldId(),
                                                              QString::fromStdString(variable),
                                                              timeStep,
                                                              adaptivityStep);

    for (int i = 0; i < edges.size(); i++)
        recipe->addEdge(edges[i]);

    Agros::problem()->recipes()->addRecipe(recipe);
}

void PyField::addRecipeLocalValue(const std::string &name, const std::string &variable, const std::string &component, double px, double py, int timeStep, int adaptivityStep)
{
    // TODO: checks
    LocalValueRecipe *recipe = new LocalValueRecipe(QString::fromStdString(name),
                                                    m_fieldInfo->fieldId(),
                                                    QString::fromStdString(variable),
                                                    timeStep,
                                                    adaptivityStep);

    recipe->setVariableComponent(physicFieldVariableCompFromStringKey(QString::fromStdString(component)));
    recipe->setPoint(px, py);

    Agros::problem()->recipes()->addRecipe(recipe);
}


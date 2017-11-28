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

#include "field.h"
#include "util/global.h"

#include "problem.h"
#include "solver/problem_config.h"
#include "scene.h"
#include "scenemarker.h"
#include "module.h"
#include "logview.h"

#include "plugin_interface.h"

// #include "../../resources_source/classes/problem_a2d_31_xml.h"

FieldInfo::FieldInfo(QString fieldId)
    : m_plugin(nullptr), m_numberOfSolutions(0), m_hermesMarkerToAgrosLabelConversion(nullptr), m_labelAreas(nullptr)
{    
    assert(!fieldId.isEmpty());
    m_fieldId = fieldId;

    // read plugin
    try
    {
        m_plugin = Agros::loadPlugin(m_fieldId);

    }
    catch (AgrosPluginException &e)
    {
        Agros::log()->printError("Solver", "Cannot load plugin");
        throw;
    }

    assert(m_plugin);

    setStringKeys();
    clear();

    // default analysis
    setAnalysisType(analyses().begin().key());
}

FieldInfo::~FieldInfo()
{
    // delete m_plugin;
}

void FieldInfo::copy(const FieldInfo *origin)
{
    clear();

    m_setting = origin->m_setting;

    // analysis
    setAnalysisType(origin->analysisType());
    setAdaptivityType(origin->adaptivityType());
    setLinearityType(origin->linearityType());
    setMatrixSolver(origin->matrixSolver());
}

double FieldInfo::labelArea(int agrosLabel) const
{
    assert(m_labelAreas);
    return m_labelAreas[agrosLabel];
}

void FieldInfo::setAnalysisType(AnalysisType analysisType)
{
    m_setting[Analysis] = QVariant::fromValue(analysisType);

    foreach (PluginModuleAnalysis an, m_plugin->moduleJson()->analyses)
    {
        if (an.id == analysisTypeToStringKey(analysisType))
            m_numberOfSolutions = an.solutions;

        m_availableLinearityTypes = availableLinearityTypes(analysisType);
    }

    /*
    foreach (XMLXXXModule::analysis an, m_plugin->module()->general_field().analyses().analysis())
    {
        if (an.type() == analysisTypeToStringKey(analysisType).toStdString())
        {
            m_numberOfSolutions = an.solutions();

            if (an.field_config().present())
            {
                for (int i = 0; i < an.field_config().get().field_item().size(); i ++)
                {
                    Type key = stringKeyToType(QString::fromStdString(an.field_config().get().field_item().at(i).field_key()));

                    if (m_settingDefault.keys().contains(key))
                    {
                        if (m_settingDefault[key].type() == QVariant::Double)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toDouble();
                        else if (m_settingDefault[key].type() == QVariant::Int)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toInt();
                        else if (m_settingDefault[key].type() == QVariant::Bool)
                            m_settingDefault[key] = (QString::fromStdString(an.field_config().get().field_item().at(i).field_value()) == "1");
                        else if (m_settingDefault[key].type() == QVariant::String)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                        else if (m_settingDefault[key].type() == QVariant::StringList)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).split("|");
                        else
                            qDebug() << "Key not found" << QString::fromStdString(an.field_config().get().field_item().at(i).field_key()) << QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                    }
                }
            }

            m_availableLinearityTypes = availableLinearityTypes(analysisType);
        }
    }
    */
}

QList<LinearityType> FieldInfo::availableLinearityTypes(AnalysisType at) const
{
    QList<LinearityType> availableLinearityTypes;

    foreach (PluginWeakFormAnalysis analysis, m_plugin->moduleJson()->weakFormAnalysisVolume)
    {
        if (analysis.analysis == at)
        {
            foreach (PluginWeakFormAnalysis::Item item, analysis.items)
            {
                // should be only one
                foreach (PluginWeakFormAnalysis::Item::Solver solver, item.solvers)
                {
                    availableLinearityTypes.append(solver.linearity);
                }
            }
        }
    }

    return availableLinearityTypes;
}

int FieldInfo::labelRefinement(SceneLabel *label) const
{
    QMapIterator<SceneLabel *, int> i(m_labelsRefinement);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return 0;
}


int FieldInfo::labelPolynomialOrder(SceneLabel *label)
{
    QMapIterator<SceneLabel *, int> i(m_labelsPolynomialOrder);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return value(FieldInfo::SpacePolynomialOrder).toInt();
}

void FieldInfo::clear()
{
    // set default values and types
    setDefaultValues();
    m_setting = m_settingDefault;

    m_labelsRefinement.clear();
    m_labelsPolynomialOrder.clear();
}

// xml module
// name
QString FieldInfo::name() const
{
    return m_plugin->localeName(m_plugin->moduleJson()->name);
}

// deformable shape
bool FieldInfo::hasDeformableShape() const
{
    return m_plugin->moduleJson()->deformedShape;
}

// constants
QMap<QString, double> FieldInfo::constants() const
{
    QMap<QString, double> constants;

    foreach (PluginConstant constant, m_plugin->moduleJson()->constants)
        constants[constant.id] = constant.value;

    return constants;
}

// macros
QMap<QString, QString> FieldInfo::macros() const
{
    QMap<QString, QString> macros;

    foreach (PluginMacro macro, m_plugin->moduleJson()->macros)
        macros[macro.id] = macro.expression;

    return macros;
}

QMap<AnalysisType, QString> FieldInfo::analyses() const
{
    QMap<AnalysisType, QString> analyses;

    foreach (PluginModuleAnalysis analysis, m_plugin->moduleJson()->analyses)
        analyses[analysis.type] = m_plugin->localeName(analysis.name);

    return analyses;
}

QList<QString> FieldInfo::allMaterialQuantities() const
{
    QList<QString> result;

    foreach (PluginWeakFormRecipe::Variable variable, m_plugin->moduleJson()->weakFormRecipeVolume.variables)
        result.append(variable.id);

    return result;
}

// material type
QList<Module::MaterialTypeVariable> FieldInfo::materialTypeVariables() const
{
    QList<Module::MaterialTypeVariable> materialTypeVariables;
    foreach (PluginWeakFormAnalysis analysis, m_plugin->moduleJson()->weakFormAnalysisVolume)
    {
        if (analysis.analysis == analysisType())
        {
            foreach (PluginWeakFormAnalysis::Item item, analysis.items)
            {
                foreach (PluginWeakFormAnalysis::Item::Variable variable, item.variables)
                {
                    foreach (PluginWeakFormRecipe::Variable variableRecipe, m_plugin->moduleJson()->weakFormRecipeVolume.variables)
                    {
                        if (variable.id == variableRecipe.id)
                        {
                            bool isTimeDep = false;

                            // time dep
                            if (!variable.dependency.isEmpty())
                            {
                                if (variable.dependency == "time")
                                {
                                    isTimeDep = true;
                                }
                            }

                            // nonlinearity
                            QString nonlinearExpression;
                            if (Agros::problem()->config()->coordinateType() == CoordinateType_Planar && !variable.nonlinearity_planar.isEmpty())
                                nonlinearExpression = variable.nonlinearity_planar;
                            else if (Agros::problem()->config()->coordinateType() == CoordinateType_Axisymmetric && !variable.nonlinearity_axi.isEmpty())
                                nonlinearExpression = variable.nonlinearity_axi;

                            // UI
                            foreach (PluginPreGroup group, m_plugin->moduleJson()->preVolumeGroups)
                            {
                                foreach (PluginPreGroup::Quantity quantity, group.quantities)
                                {
                                    if (quantity.id == variable.id)
                                    {
                                        materialTypeVariables.append(Module::MaterialTypeVariable(variable.id, variableRecipe.shortName, quantity.default_value,
                                                                                                  nonlinearExpression,
                                                                                                  isTimeDep, quantity.isBool, quantity.onlyIf, quantity.onlyIfNot, quantity.isSource));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return materialTypeVariables;
}

// variable by name
bool FieldInfo::materialTypeVariableContains(const QString &id) const
{
    foreach (Module::MaterialTypeVariable var, materialTypeVariables())
        if (var.id() == id)
            return true;

    return false;
}

Module::MaterialTypeVariable FieldInfo::materialTypeVariable(const QString &id) const
{
    foreach (Module::MaterialTypeVariable var, materialTypeVariables())
        if (var.id() == id)
            return var;

    assert(0);
}

QList<Module::BoundaryType> FieldInfo::boundaryTypes() const
{
    QList<Module::BoundaryType> boundaryTypes;
    foreach (PluginWeakFormAnalysis analysis, m_plugin->moduleJson()->weakFormAnalysisSurface)
    {
        if (analysis.analysis == analysisType())
        {
            foreach (PluginWeakFormAnalysis::Item item, analysis.items)
            {
                // variables
                QList<Module::BoundaryTypeVariable> variables;

                foreach (PluginWeakFormAnalysis::Item::Variable variable, item.variables)
                {
                    foreach (PluginWeakFormRecipe::Variable variableRecipe, m_plugin->moduleJson()->weakFormRecipeSurface.variables)
                    {
                        if (variable.id == variableRecipe.id)
                        {
                            bool isTimeDep = false;
                            bool isSpaceDep = false;

                            if (!variable.dependency.isEmpty())
                            {
                                if (variable.dependency == "time")
                                {
                                    isTimeDep = true;
                                }
                                else if (variable.dependency == "space")
                                {
                                    isSpaceDep = true;
                                }
                                else if (variable.dependency == "time-space")
                                {
                                    isTimeDep = true;
                                    isSpaceDep = true;
                                }
                            }

                            variables.append(Module::BoundaryTypeVariable(variable.id, variableRecipe.shortName, isTimeDep, isSpaceDep));
                        }
                    }
                }

                QList<FormInfo> wfMatrix; //  = Module::wfMatrixSurface(&fieldInfo->plugin()->module()->surface(), &variable, fieldInfo->analysisType(), fieldInfo->linearityType());
                QList<FormInfo> wfVector; //  = Module::wfVectorSurface(&fieldInfo->plugin()->module()->surface(), &variable, fieldInfo->analysisType(), fieldInfo->linearityType());
                QList<FormInfo> essential; //  = Module::essential(&fieldInfo->plugin()->module()->surface(), &variable, fieldInfo->analysisType(), fieldInfo->linearityType());

                boundaryTypes.append(Module::BoundaryType(item.id, m_plugin->localeName(item.name), item.equation,
                                                          variables, wfMatrix, wfVector, essential));
            }
        }
    }

    return boundaryTypes;
}

// variable by name
bool FieldInfo::boundaryTypeContains(const QString &id) const
{
    foreach (Module::BoundaryType var, boundaryTypes())
        if (var.id() == id)
            return true;

    return false;
}

Module::BoundaryType FieldInfo::boundaryType(const QString &id) const
{
    foreach (Module::BoundaryType var, boundaryTypes())
        if (var.id() == id)
            return var;

    throw AgrosModuleException(QString("Boundary type %1 not found. Probably using corrupted a2d file or wrong version.").arg(id));
}

// force
Module::Force FieldInfo::force(CoordinateType coordinateType) const
{
    return Module::Force();

    // force
    /*
    XMLXXXModule::force force = m_plugin->module()->postprocessor().force();
    for (unsigned int i = 0; i < force.expression().size(); i++)
    {
        XMLXXXModule::expression exp = force.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            return Module::Force((coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_x().get()) : QString::fromStdString(exp.axi_r().get()),
                                 (coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_y().get()) : QString::fromStdString(exp.axi_z().get()),
                                 (coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_z().get()) : QString::fromStdString(exp.axi_phi().get()));
    }

    assert(0);
    return Module::Force();
    */
}

// material and boundary user interface
Module::DialogUI FieldInfo::materialUI() const
{
    QMap<QString, QList<Module::DialogRow> > groups;

    // preprocessor
    foreach (PluginPreGroup group, m_plugin->moduleJson()->preVolumeGroups)
    {
        QList<Module::DialogRow> materials;

        foreach (PluginPreGroup::Quantity quantity, group.quantities)
        {
            Module::DialogRow row(quantity.id, quantity.name, quantity.shortname, quantity.shortname_html,
                                  quantity.shortname_dependence, quantity.shortname_dependence_html,
                                  quantity.unit, quantity.unit_html,
                                  quantity.default_value, quantity.condition);

            materials.append(row);
        }

        groups[group.name] = materials;
    }

    return Module::DialogUI(groups);
}

Module::DialogUI FieldInfo::boundaryUI() const
{
    QMap<QString, QList<Module::DialogRow> > groups;

    // preprocessor
    foreach (PluginPreGroup group, m_plugin->moduleJson()->preSurfaceGroups)
    {

        QList<Module::DialogRow> materials;

        foreach (PluginPreGroup::Quantity quantity, group.quantities)
        {
            Module::DialogRow row(quantity.id, quantity.name, quantity.shortname, quantity.shortname_html,
                                  quantity.shortname_dependence, quantity.shortname_dependence_html,
                                  quantity.unit, quantity.unit_html,
                                  quantity.default_value, quantity.condition);

            materials.append(row);
        }

        groups[group.name] = materials;
    }

    return Module::DialogUI(groups);
}

// local point variables
QList<Module::LocalVariable> FieldInfo::localPointVariables(CoordinateType coordinateType) const
{
    // local variables
    QList<Module::LocalVariable> variables;

    foreach (PluginPostVariable variable, m_plugin->moduleJson()->postLocalVariables)
    {
        foreach (PluginPostVariable::Expression expresion, variable.expresions)
        {
            if (expresion.analysis == analysisType())
            {
                bool isScalar = (variable.type == "scalar");

                Module::LocalVariable::Expression expr;

                if (coordinateType == CoordinateType_Planar)
                    expr = Module::LocalVariable::Expression(isScalar ? expresion.planar : "",
                                                             isScalar ? "" : expresion.planar_x,
                                                             isScalar ? "" : expresion.planar_y);
                else if (coordinateType == CoordinateType_Axisymmetric)
                    expr = Module::LocalVariable::Expression(isScalar ? expresion.axi : "",
                                                             isScalar ? "" : expresion.axi_r,
                                                             isScalar ? "" : expresion.axi_z);
                else
                    assert(0);

                variables.append(Module::LocalVariable(variable.id, m_plugin->localeName(variable.name),
                                                       variable.shortname, variable.shortname_html,
                                                       variable.unit, variable.unit_html,
                                                       isScalar,
                                                       expr));
            }
        }
    }

    return variables;
}

// view scalar variables
QList<Module::LocalVariable> FieldInfo::viewScalarVariables(CoordinateType coordinateType) const
{
    // scalar variables = local variables
    return localPointVariables(coordinateType);
}

// view vector variables
QList<Module::LocalVariable> FieldInfo::viewVectorVariables(CoordinateType coordinateType) const
{
    // vector variables
    QList<Module::LocalVariable> variables;
    foreach (Module::LocalVariable var, localPointVariables(coordinateType))
        if (!var.isScalar())
            variables.append(var);

    return variables;
}

// surface integrals
QList<Module::Integral> FieldInfo::surfaceIntegrals(CoordinateType coordinateType) const
{
    // surface integrals
    QList<Module::Integral> surfaceIntegrals;

    foreach (PluginPostVariable variable, m_plugin->moduleJson()->postSurfaceIntegrals)
    {
        foreach (PluginPostVariable::Expression expresion, variable.expresions)
        {
            if (expresion.analysis == analysisType())
            {
                QString expr;

                if (coordinateType == CoordinateType_Planar)
                    expr = expresion.planar;
                else if (coordinateType == CoordinateType_Axisymmetric)
                    expr = expresion.axi;
                else
                    assert(0);

                surfaceIntegrals.append(Module::Integral(variable.id, m_plugin->localeName(variable.name),
                                                         variable.shortname, variable.shortname_html,
                                                         variable.unit, variable.unit_html,
                                                         expr,
                                                         false));
            }
        }
    }

    return surfaceIntegrals;
}

// volume integrals
QList<Module::Integral> FieldInfo::volumeIntegrals(CoordinateType coordinateType) const
{
    // volume integrals
    QList<Module::Integral> volumeIntegrals;

    foreach (PluginPostVariable variable, m_plugin->moduleJson()->postVolumeIntegrals)
    {
        foreach (PluginPostVariable::Expression expresion, variable.expresions)
        {
            if (expresion.analysis == analysisType())
            {
                QString expr;

                if (coordinateType == CoordinateType_Planar)
                    expr = expresion.planar;
                else if (coordinateType == CoordinateType_Axisymmetric)
                    expr = expresion.axi;
                else
                    assert(0);

                // TODO: eggshell
                volumeIntegrals.append(Module::Integral(variable.id, m_plugin->localeName(variable.name),
                                                        variable.shortname, variable.shortname_html,
                                                        variable.unit, variable.unit_html,
                                                        expr,
                                                        false));
            }
        }
    }

    return volumeIntegrals;
}

// variable by name
Module::LocalVariable FieldInfo::localVariable(CoordinateType coordinateType, const QString &id) const
{
    foreach (Module::LocalVariable var, localPointVariables(coordinateType))
        if (var.id() == id)
            return var;

    qDebug() << "localVariable: " << id;
    assert(0);

    // qDebug() << "Warning: unable to return local variable: " << id;
    // return Module::LocalVariable();
}

Module::Integral FieldInfo::surfaceIntegral(CoordinateType coordinateType, const QString &id) const
{
    foreach (Module::Integral var, surfaceIntegrals(coordinateType))
        if (var.id() == id)
            return var;

    qDebug() << "surfaceIntegral: " << id;
    assert(0);
}

Module::Integral FieldInfo::volumeIntegral(CoordinateType coordinateType, const QString &id) const
{
    foreach (Module::Integral var, volumeIntegrals(coordinateType))
        if (var.id() == id)
            return var;

    qDebug() << "volumeIntegral: " << id;
    assert(0);
}

// default variables
Module::LocalVariable FieldInfo::defaultViewScalarVariable(CoordinateType coordinateType) const
{
    assert(localPointVariables(coordinateType).size() > 0);

    // return first variable
    return localPointVariables(coordinateType).first();
}

Module::LocalVariable FieldInfo::defaultViewVectorVariable(CoordinateType coordinateType) const
{
    // return first vector variable
    foreach (Module::LocalVariable var, localPointVariables(coordinateType))
        if (!var.isScalar())
            return var;

    assert(0);
}

//void FieldInfo::load(XMLProblem::field_config *configxsd)
//{
//    // default
//    m_setting = m_settingDefault;

//    for (int i = 0; i < configxsd->field_item().size(); i ++)
//    {
//        Type key = stringKeyToType(QString::fromStdString(configxsd->field_item().at(i).field_key()));

//        if (m_settingDefault.keys().contains(key))
//        {
//            if (m_settingDefault[key].type() == QVariant::Double)
//                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).toDouble();
//            else if (m_settingDefault[key].type() == QVariant::Int)
//                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).toInt();
//            else if (m_settingDefault[key].type() == QVariant::Bool)
//                m_setting[key] = (QString::fromStdString(configxsd->field_item().at(i).field_value()) == "1");
//            else if (m_settingDefault[key].type() == QVariant::String)
//                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value());
//            else if (m_settingDefault[key].type() == QVariant::StringList)
//                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).split("|");
//            // else
//            //    qDebug() << "Key not found" << QString::fromStdString(configxsd->field_item().at(i).field_key()) << QString::fromStdString(configxsd->field_item().at(i).field_value());
//        }
//    }
//}

//void FieldInfo::save(XMLProblem::field_config *configxsd)
//{
//    foreach (Type key, m_settingDefault.keys())
//    {
//        if (m_settingDefault[key].type() == QVariant::StringList)
//            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), m_setting[key].toStringList().join("|").toStdString()));
//        else if (m_settingDefault[key].type() == QVariant::Bool)
//            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), QString::number(m_setting[key].toInt()).toStdString()));
//        else
//            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), m_setting[key].toString().toStdString()));

//    }
//}

void FieldInfo::load(QJsonObject &object)
{
    // default
    m_setting = m_settingDefault;

    foreach (Type key, m_settingDefault.keys())
    {
        if (!object.contains(typeToStringKey(key)))
            continue;

        if (m_settingDefault[key].type() == QVariant::StringList)
            m_setting[key] = object[typeToStringKey(key)].toString().split("|");
        else if (m_settingDefault[key].type() == QVariant::Bool)
            m_setting[key] = object[typeToStringKey(key)].toBool();
        else if (m_settingDefault[key].type() == QVariant::String)
            m_setting[key] = object[typeToStringKey(key)].toString();
        else if (m_settingDefault[key].type() == QVariant::Double)
            m_setting[key] = object[typeToStringKey(key)].toDouble();
        else if (m_settingDefault[key].type() == QVariant::Int)
            m_setting[key] = object[typeToStringKey(key)].toInt();
        else
        {
            if (m_settingDefault[key].userType() == qMetaTypeId<AnalysisType>())
                setAnalysisType(analysisTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_settingDefault[key].userType() == qMetaTypeId<LinearityType>())
                setLinearityType(linearityTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_settingDefault[key].userType() == qMetaTypeId<AdaptivityMethod>())
                setAdaptivityType(adaptivityTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_settingDefault[key].userType() == qMetaTypeId<MatrixSolverType>())
                setMatrixSolver(matrixSolverTypeFromStringKey( object[typeToStringKey(key)].toString()));
            else
                assert(0);
        }
    }
}

void FieldInfo::save(QJsonObject &object)
{
    foreach (Type key, m_settingDefault.keys())
    {
        if (m_settingDefault[key].type() == QVariant::StringList)
            object[typeToStringKey(key)] = m_setting[key].toStringList().join("|");
        else if (m_settingDefault[key].type() == QVariant::Bool)
            object[typeToStringKey(key)] = m_setting[key].toBool();
        else if (m_settingDefault[key].type() == QVariant::String)
            object[typeToStringKey(key)] = m_setting[key].toString();
        else if (m_settingDefault[key].type() == QVariant::Double)
            object[typeToStringKey(key)] = m_setting[key].toDouble();
        else if (m_settingDefault[key].type() == QVariant::Int)
            object[typeToStringKey(key)] = m_setting[key].toInt();
        else
        {
            if (m_settingDefault[key].userType() == qMetaTypeId<AnalysisType>())
                object[typeToStringKey(key)] = analysisTypeToStringKey(m_setting[key].value<AnalysisType>());
            else if (m_settingDefault[key].userType() == qMetaTypeId<LinearityType>())
                object[typeToStringKey(key)] = linearityTypeToStringKey(m_setting[key].value<LinearityType>());
            else if (m_settingDefault[key].userType() == qMetaTypeId<AdaptivityMethod>())
                object[typeToStringKey(key)] = adaptivityTypeToStringKey(m_setting[key].value<AdaptivityMethod>());
            else if (m_settingDefault[key].userType() == qMetaTypeId<MatrixSolverType>())
                object[typeToStringKey(key)] = matrixSolverTypeToStringKey(m_setting[key].value<MatrixSolverType>());
            else
                assert(0);
        }
    }
}

void FieldInfo::setStringKeys()
{
    m_settingKey[Analysis] = "Analysis";
    m_settingKey[Linearity] = "Linearity";
    m_settingKey[NonlinearResidualNorm] = "NonlinearResidualNorm";
    m_settingKey[NonlinearRelativeChangeOfSolutions] = "NonlinearRelativeChangeOfSolutions";
    m_settingKey[NonlinearDampingType] = "NonlinearDampingType";
    m_settingKey[NonlinearDampingCoeff] = "NonlinearDampingCoeff";
    m_settingKey[NewtonReuseJacobian] = "NewtonReuseJacobian";
    m_settingKey[NewtonJacobianReuseRatio] = "NewtonJacobianReuseRatio";
    m_settingKey[NonlinearDampingFactorDecreaseRatio] = "NonlinearDampingFactorDecreaseRatio";
    m_settingKey[NewtonMaxStepsReuseJacobian] = "NewtonMaxStepsReuseJacobian";
    m_settingKey[NonlinearStepsToIncreaseDampingFactor] = "NonlinearStepsToIncreaseDampingFactor";
    m_settingKey[PicardAndersonAcceleration] = "PicardAndersonAcceleration";
    m_settingKey[PicardAndersonBeta] = "PicardAndersonBeta";
    m_settingKey[PicardAndersonNumberOfLastVectors] = "PicardAndersonNumberOfLastVectors";
    m_settingKey[SpaceNumberOfRefinements] = "SpaceNumberOfRefinements";
    m_settingKey[SpacePolynomialOrder] = "SpacePolynomialOrder";
    m_settingKey[Adaptivity] = "Adaptivity";
    m_settingKey[AdaptivitySteps] = "AdaptivitySteps";
    m_settingKey[AdaptivityTolerance] = "AdaptivityTolerance";
    m_settingKey[AdaptivityTransientBackSteps] = "AdaptivityTransientBackSteps";
    m_settingKey[AdaptivityTransientRedoneEach] = "AdaptivityTransientRedoneEach";
    m_settingKey[AdaptivityFinePercentage] = "AdaptivityFinePercentage";
    m_settingKey[AdaptivityCoarsePercentage] = "AdaptivityCoarsePercentage";
    m_settingKey[AdaptivityEstimator] = "AdaptivityEstimator";
    m_settingKey[AdaptivityStrategy] = "AdaptivityStrategy";
    m_settingKey[AdaptivityStrategyHP] = "AdaptivityStrategyHP";
    m_settingKey[TransientTimeSkip] = "TransientTimeSkip";
    m_settingKey[TransientInitialCondition] = "TransientInitialCondition";
    m_settingKey[LinearSolver] = "LinearSolver";
    m_settingKey[LinearSolverIterDealIIMethod] = "LinearSolverIterDealIIMethod";
    m_settingKey[LinearSolverIterDealIIPreconditioner] = "LinearSolverIterDealIIPreconditioner";
    m_settingKey[LinearSolverIterToleranceAbsolute] = "LinearSolverIterToleranceAbsolute";
    m_settingKey[LinearSolverIterIters] = "LinearSolverIterIters";
    m_settingKey[LinearSolverExternalName] = "LinearSolverExternalName";
    m_settingKey[LinearSolverExternalCommandEnvironment] = "LinearSolverExternalCommandEnvironment";
    m_settingKey[LinearSolverExternalCommandParameters] = "LinearSolverExternalCommandParameters";
    m_settingKey[TimeUnit] = "TimeUnit";
}

void FieldInfo::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[Analysis] = QVariant::fromValue(AnalysisType_Undefined);
    m_settingDefault[Linearity] = QVariant::fromValue(LinearityType_Linear);
    m_settingDefault[NonlinearResidualNorm] = 0.0;
    m_settingDefault[NonlinearRelativeChangeOfSolutions] = 0.1;
    m_settingDefault[NonlinearDampingType] = DampingType_Automatic;
    m_settingDefault[NonlinearDampingCoeff] = 0.8;
    m_settingDefault[NewtonReuseJacobian] = true;
    m_settingDefault[NewtonJacobianReuseRatio] = 0.8;
    m_settingDefault[NonlinearDampingFactorDecreaseRatio] = 1.2;
    m_settingDefault[NewtonMaxStepsReuseJacobian] = 20;
    m_settingDefault[NonlinearStepsToIncreaseDampingFactor] = 1;
    m_settingDefault[PicardAndersonAcceleration] = false;
    m_settingDefault[PicardAndersonBeta] = 0.2;
    m_settingDefault[PicardAndersonNumberOfLastVectors] = 3;
    m_settingDefault[SpaceNumberOfRefinements] = 1;
    m_settingDefault[SpacePolynomialOrder] = 2;
    m_settingDefault[Adaptivity] = QVariant::fromValue(AdaptivityMethod_None);
    m_settingDefault[AdaptivitySteps] = 10;
    m_settingDefault[AdaptivityTolerance] = 1.0;
    m_settingDefault[AdaptivityEstimator] = AdaptivityEstimator_Kelly;
    m_settingDefault[AdaptivityStrategy] = AdaptivityStrategy_FixedFractionOfTotalError;
    m_settingDefault[AdaptivityStrategyHP] = AdaptivityStrategyHP_FourierSeries;
    m_settingDefault[AdaptivityFinePercentage] = 30;
    m_settingDefault[AdaptivityCoarsePercentage] = 3;
    m_settingDefault[AdaptivityTransientBackSteps] = 3;
    m_settingDefault[AdaptivityTransientRedoneEach] = 5;
    m_settingDefault[TransientTimeSkip] = 0.0;
    m_settingDefault[TransientInitialCondition] = 0.0;
    m_settingDefault[LinearSolver] = QVariant::fromValue(SOLVER_UMFPACK);
    m_settingDefault[LinearSolverIterDealIIMethod] = IterSolverDealII_BiCGStab;
    m_settingDefault[LinearSolverIterDealIIPreconditioner] = PreconditionerDealII_SSOR;
    m_settingDefault[LinearSolverIterToleranceAbsolute] = 1e-16;
    m_settingDefault[LinearSolverIterIters] = 1000;
    m_settingDefault[LinearSolverExternalName] = "";
    m_settingDefault[LinearSolverExternalCommandEnvironment] = "";
    m_settingDefault[LinearSolverExternalCommandParameters] = "";
    m_settingDefault[TimeUnit] = "s";
}

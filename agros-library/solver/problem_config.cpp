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

#include "problem_config.h"

#include "util/global.h"
#include "util/constants.h"

#include "field.h"
#include "solutionstore.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "module.h"
#include "problem_result.h"
#include "problem_function.h"
#include "problem_parameter.h"
#include "coupling.h"
#include "solver.h"
#include "mesh/meshgenerator.h"
#include "logview.h"

ProblemConfig::ProblemConfig(ProblemBase *parentProblem) : QObject(), m_problem(parentProblem),
    m_parameters(new ProblemParameters()), m_functions(new ProblemFunctions())
{
    // qInfo() << "qRegisterMetaType<StringToDoubleMap>" << qRegisterMetaType<StringToDoubleMap>("ParametersType");
    // qInfo() << "qRegisterMetaType<Value>" << qRegisterMetaType<Value>("Value");
    qRegisterMetaType<StringToDoubleMap>("ParametersType");
    qRegisterMetaType<Value>("Value");

    setStringKeys();
    clear();
}

ProblemConfig::~ProblemConfig()
{
    m_parameters->clear();
    delete m_parameters;
    m_functions->clear();
    delete m_functions;
}

void ProblemConfig::copy(const ProblemConfig *origin)
{
    // copy settings
    m_config = origin->m_config;

    // parameters
    m_parameters->clear();
    QList<ProblemParameter> parameters = origin->parameters()->items().values();
    m_parameters->set(parameters);

    // functions
    m_functions->clear();

    // value Problem
    foreach (Type key, m_configDefault.keys())
        if (m_configDefault[key].userType() == qMetaTypeId<Value>())
            setValue(key, Value(m_problem, origin->m_config[key].value<Value>().text()));
}

void ProblemConfig::clear()
{
    // parameters
    m_parameters->clear();

    // functions
    m_functions->clear();

    // set default values and types
    setDefaultValues();
    m_config = m_configDefault;

    // set parameters
    m_parameters->set(m_parameters->items().values());
}

double ProblemConfig::initialTimeStepLength()
{
    return constantTimeStepLength();
}

//void ProblemConfig::load(XMLProblem::problem_config *configxsd)
//{
//    // default
//    m_config = m_configDefault;

//    for (int i = 0; i < configxsd->problem_item().size(); i ++)
//    {
//        Type key = stringKeyToType(QString::fromStdString(configxsd->problem_item().at(i).problem_key()));

//        if (m_configDefault.keys().contains(key))
//        {
//            if (m_configDefault[key].type() == QVariant::Double)
//                m_config[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value()).toDouble();
//            else if (m_configDefault[key].type() == QVariant::Int)
//                m_config[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value()).toInt();
//            else if (m_configDefault[key].type() == QVariant::Bool)
//                m_config[key] = (QString::fromStdString(configxsd->problem_item().at(i).problem_value()) == "1");
//            else if (m_configDefault[key].type() == QVariant::String)
//                m_config[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value());
//            else if (m_configDefault[key].type() == QVariant::StringList)
//                m_config[key] = QString::fromStdString(configxsd->problem_item().at(i).problem_value()).split("|");
//            else
//            {
//                if (m_configDefault[key].userType() == qMetaTypeId<CoordinateType>())
//                    m_config[key] = QVariant::fromValue(coordinateTypeFromStringKey(QString::fromStdString(configxsd->problem_item().at(i).problem_value())));
//                else if (m_configDefault[key].userType() == qMetaTypeId<MeshType>())
//                    m_config[key] = QVariant::fromValue(meshTypeFromStringKey(QString::fromStdString(configxsd->problem_item().at(i).problem_value())));
//                else if (m_configDefault[key].userType() == qMetaTypeId<Value>())
//                    m_config[key] = QVariant::fromValue(Value(m_problem, QString::fromStdString(configxsd->problem_item().at(i).problem_value())));
//                else
//                    qDebug() << "Key not found (XML)" << QString::fromStdString(configxsd->problem_item().at(i).problem_key()) << QString::fromStdString(configxsd->problem_item().at(i).problem_value());
//            }
//        }
//    }
//}

//void ProblemConfig::save(XMLProblem::problem_config *configxsd)
//{
//    foreach (Type key, m_config.keys())
//    {
//        if (m_configDefault[key].type() == QVariant::StringList)
//            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), m_config[key].toStringList().join("|").toStdString()));
//        else if (m_configDefault[key].type() == QVariant::Bool)
//            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), QString::number(m_config[key].toInt()).toStdString()));
//        else if (m_configDefault[key].type() == QVariant::String)
//            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), m_config[key].toString().toStdString()));
//        else if (m_configDefault[key].type() == QVariant::Int)
//            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), QString::number(m_config[key].toInt()).toStdString()));
//        else if (m_configDefault[key].type() == QVariant::Double)
//            configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), QString::number(m_config[key].toDouble()).toStdString()));
//        else
//        {
//            if (m_configDefault[key].userType() == qMetaTypeId<CoordinateType>())
//                configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), coordinateTypeToStringKey(m_config[key].value<CoordinateType>()).toStdString()));
//            else if (m_configDefault[key].userType() == qMetaTypeId<MeshType>())
//                configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), meshTypeToStringKey(m_config[key].value<MeshType>()).toStdString()));
//            else if (m_configDefault[key].userType() == qMetaTypeId<Value>())
//                configxsd->problem_item().push_back(XMLProblem::problem_item(typeToStringKey(key).toStdString(), m_config[key].value<Value>().toString().toStdString()));
//            else
//                assert(0);
//        }
//    }
//}

void ProblemConfig::load(QJsonObject &object)
{
    // default
    m_config = m_configDefault;

    foreach (Type key, m_configDefault.keys())
    {
        if (!object.contains(typeToStringKey(key)))
            continue;

        if (m_configDefault[key].type() == QVariant::StringList)
            m_config[key] = object[typeToStringKey(key)].toString().split("|");
        else if (m_configDefault[key].type() == QVariant::Bool)
            m_config[key] = object[typeToStringKey(key)].toBool();
        else if (m_configDefault[key].type() == QVariant::String)
            m_config[key] = object[typeToStringKey(key)].toString();
        else if (m_configDefault[key].type() == QVariant::Double)
            m_config[key] = object[typeToStringKey(key)].toDouble();
        else if (m_configDefault[key].type() == QVariant::Int)
            m_config[key] = object[typeToStringKey(key)].toInt();
        else
        {
            if (m_configDefault[key].userType() == qMetaTypeId<CoordinateType>())
                m_config[key] = QVariant::fromValue(coordinateTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_configDefault[key].userType() == qMetaTypeId<MeshType>())
                m_config[key] = QVariant::fromValue(meshTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_configDefault[key].userType() == qMetaTypeId<Value>())
                m_config[key] = QVariant::fromValue(Value(m_problem, object[typeToStringKey(key)].toString()));
            else
                assert(0);
        }
    }

    // parameters
    m_parameters->load(object);

    // functions
    m_functions->load(object);
}

void ProblemConfig::save(QJsonObject &object)
{
    foreach (Type key, m_configDefault.keys())
    {
        if (m_configDefault[key].type() == QVariant::StringList)
            object[typeToStringKey(key)] = m_config[key].toStringList().join("|");
        else if (m_configDefault[key].type() == QVariant::Bool)
            object[typeToStringKey(key)] = m_config[key].toBool();
        else if (m_configDefault[key].type() == QVariant::String)
            object[typeToStringKey(key)] = m_config[key].toString();
        else if (m_configDefault[key].type() == QVariant::Double)
            object[typeToStringKey(key)] = m_config[key].toDouble();
        else if (m_configDefault[key].type() == QVariant::Int)
            object[typeToStringKey(key)] = m_config[key].toInt();
        else
        {
            if (m_configDefault[key].userType() == qMetaTypeId<MeshType>())
                object[typeToStringKey(key)] = meshTypeToStringKey(m_config[key].value<MeshType>());
            else if (m_configDefault[key].userType() == qMetaTypeId<CoordinateType>())
                object[typeToStringKey(key)] = coordinateTypeToStringKey(m_config[key].value<CoordinateType>());
            else if (m_configDefault[key].userType() == qMetaTypeId<Value>())
                object[typeToStringKey(key)] = m_config[key].value<Value>().toString();
            else
                assert(0);
        }
    }

    // parameters
    m_parameters->save(object);

    // functions
    m_functions->save(object);
}

void ProblemConfig::setStringKeys()
{
    m_configKey[Frequency] = "Frequency";
    m_configKey[TimeMethod] = "TimeMethod";
    m_configKey[TimeMethodTolerance] = "TimeMethodTolerance";
    m_configKey[TimeInitialStepSize] = "TimeInitialStepSize";
    m_configKey[TimeOrder] = "TimeOrder";
    m_configKey[TimeConstantTimeSteps] = "TimeSteps";
    m_configKey[TimeTotal] = "TimeTotal";
    m_configKey[Coordinate] = "Coordinate";
    m_configKey[Mesh] = "Mesh";

    m_configKey[GridStep] = "GridStep";
    m_configKey[SnapToGrid] = "SnapToGrid";
}

void ProblemConfig::setDefaultValues()
{
    m_configDefault.clear();

    m_configDefault[Frequency] = QVariant::fromValue(Value(m_problem, 50));
    m_configDefault[TimeMethod] = TimeStepMethod_Fixed;
    m_configDefault[TimeMethodTolerance] = 0.05;
    m_configDefault[TimeInitialStepSize] = 0.0;
    m_configDefault[TimeOrder] = 2;
    m_configDefault[TimeConstantTimeSteps] = 10;
    m_configDefault[TimeTotal] = 10.0;
    m_configDefault[Coordinate] = QVariant::fromValue(CoordinateType_Planar);
    m_configDefault[Mesh] = QVariant::fromValue(MeshType_Triangle_QuadFineDivision);

    m_configDefault[GridStep] = 0.05;
    m_configDefault[SnapToGrid] = true;
}

void ProblemConfig::checkVariableName(const QString &keyToCheck, const QString &keyToSkip)
{
    // variable name
    QRegExp expr("(^[a-zA-Z][a-zA-Z0-9_]*)|(^[_][a-zA-Z0-9_]+)");
    if (!expr.exactMatch(keyToCheck))
        throw AgrosException(QObject::tr("Invalid variable name: %1.").arg(keyToCheck));

    // keywords - time, x, y, r, z, value
    if (keyToCheck == "time" || keyToCheck == "value" || keyToCheck == "x" || keyToCheck == "y" || keyToCheck == "r" || keyToCheck == "z")
        throw AgrosException(QObject::tr("Variable is reserved keyword: %1.").arg(keyToCheck));

    // parameters
    foreach (QString k, m_parameters->items().keys())
    {
        if (!keyToSkip.isEmpty() && k == keyToSkip)
            continue;

        if (k == keyToCheck)
            throw AgrosException(QObject::tr("Parameter is already used: %1.").arg(keyToCheck));
    }

    // functions
    foreach (ProblemFunction *function, m_functions->items())
    {
        if (!keyToSkip.isEmpty() && function->name() == keyToSkip)
            continue;

        if (function->name() == keyToCheck)
            throw AgrosException(QObject::tr("Function is already used: %1.").arg(keyToCheck));
    }

    // recipes
    foreach (ResultRecipe *recipe, Agros::problem()->recipes()->items())
    {
        if (!keyToSkip.isEmpty() && recipe->name() == keyToSkip)
            continue;

        if (recipe->name() == keyToCheck)
            throw AgrosException(QObject::tr("Recipe is already used: %1.").arg(keyToCheck));
    }
}

// ********************************************************************************************

PostprocessorSetting::PostprocessorSetting(Computation *computation) : QObject(), m_computation(computation)
{
    setStringKeys();
    clear();
}

void PostprocessorSetting::copy(const PostprocessorSetting *origin)
{
    clear();

    m_setting = origin->m_setting;
}

void PostprocessorSetting::clear()
{
    // set default values and types
    setDefaultValues();
    m_setting = m_settingDefault;
}

void PostprocessorSetting::setStringKeys()
{
    m_settingKey[ScalarView3DMode] = "ScalarView3DMode";
    m_settingKey[ScalarView3DLighting] = "ScalarView3DLighting";
    m_settingKey[ScalarView3DAngle] = "ScalarView3DAngle";
    m_settingKey[ScalarView3DBackground] = "ScalarView3DBackground";
    m_settingKey[ScalarView3DHeight] = "ScalarView3DHeight";
    m_settingKey[ScalarView3DBoundingBox] = "ScalarView3DBoundingBox";
    m_settingKey[ScalarView3DSolidGeometry] = "ScalarView3DSolidGeometry";
    m_settingKey[DeformScalar] = "DeformScalar";
    m_settingKey[DeformContour] = "DeformContour";
    m_settingKey[DeformVector] = "DeformVector";
    m_settingKey[ShowInitialMeshView] = "ShowInitialMeshView";
    m_settingKey[ShowSolutionMeshView] = "ShowSolutionMeshView";
    m_settingKey[ContourVariable] = "ContourVariable";
    m_settingKey[ShowContourView] = "ShowContourView";
    m_settingKey[ContoursCount] = "ContoursCount";
    m_settingKey[ContoursWidth] = "ContoursWidth";
    m_settingKey[ShowScalarView] = "ShowScalarView";
    m_settingKey[ShowScalarColorBar] = "ShowScalarColorBar";
    m_settingKey[ScalarVariable] = "ScalarVariable";
    m_settingKey[ScalarVariableComp] = "ScalarVariableComp";
    m_settingKey[PaletteType] = "PaletteType";
    m_settingKey[PaletteFilter] = "PaletteFilter";
    m_settingKey[PaletteSteps] = "PaletteSteps";
    m_settingKey[ScalarRangeLog] = "ScalarRangeLog";
    m_settingKey[ScalarRangeBase] = "ScalarRangeBase";
    m_settingKey[ScalarDecimalPlace] = "ScalarDecimalPlace";
    m_settingKey[ScalarRangeAuto] = "ScalarRangeAuto";
    m_settingKey[ScalarRangeMin] = "ScalarRangeMin";
    m_settingKey[ScalarRangeMax] = "ScalarRangeMax";
    m_settingKey[ShowVectorView] = "ShowVectorView";
    m_settingKey[VectorVariable] = "VectorVariable";
    m_settingKey[VectorProportional] = "VectorProportional";
    m_settingKey[VectorColor] = "VectorColor";
    m_settingKey[VectorCount] = "VectorCount";
    m_settingKey[VectorScale] = "VectorScale";
    m_settingKey[VectorType] = "VectorType";
    m_settingKey[VectorCenter] = "VectorCenter";
    m_settingKey[ShowOrderView] = "ShowOrderView";
    m_settingKey[ShowErrorView] = "ShowErrorView";
    m_settingKey[OrderComponent] = "OrderComponent";
    m_settingKey[ShowOrderLabel] = "ShowOrderLabel";
    m_settingKey[ShowOrderColorBar] = "ShowOrderColorBar";
    m_settingKey[ShowErrorColorBar] = "ShowErrorColorBar";
    m_settingKey[OrderPaletteOrderType] = "OrderPaletteOrderType";
    m_settingKey[ParticleButcherTableType] = "ParticleButcherTableType";
    m_settingKey[ParticleIncludeRelativisticCorrection] = "ParticleIncludeRelativisticCorrection";
    m_settingKey[ParticleMass] = "ParticleMass";
    m_settingKey[ParticleConstant] = "ParticleConstant";
    m_settingKey[ParticleStartX] = "ParticleStartX";
    m_settingKey[ParticleStartY] = "ParticleStartY";
    m_settingKey[ParticleStartVelocityX] = "ParticleStartVelocityX";
    m_settingKey[ParticleStartVelocityY] = "ParticleStartVelocityY";
    m_settingKey[ParticleNumberOfParticles] = "ParticleNumberOfParticles";
    m_settingKey[ParticleStartingRadius] = "ParticleStartingRadius";
    m_settingKey[ParticleReflectOnDifferentMaterial] = "ParticleReflectOnDifferentMaterial";
    m_settingKey[ParticleReflectOnBoundary] = "ParticleReflectOnBoundary";
    m_settingKey[ParticleCoefficientOfRestitution] = "ParticleCoefficientOfRestitution";
    m_settingKey[ParticleMaximumRelativeError] = "ParticleMaximumRelativeError";
    m_settingKey[ParticleShowPoints] = "ParticleShowPoints";
    m_settingKey[ParticleShowBlendedFaces] = "ParticleShowBlendedFaces";
    m_settingKey[ParticleNumShowParticlesAxi] = "ParticleNumShowParticlesAxi";
    m_settingKey[ParticleColorByVelocity] = "ParticleColorByVelocity";
    m_settingKey[ParticleMaximumNumberOfSteps] = "ParticleMaximumNumberOfSteps";
    m_settingKey[ParticleMaximumStep] = "ParticleMinimumStep";
    m_settingKey[ParticleDragDensity] = "ParticleDragDensity";
    m_settingKey[ParticleDragCoefficient] = "ParticleDragCoefficient";
    m_settingKey[ParticleDragReferenceArea] = "ParticleDragReferenceArea";
    m_settingKey[ParticleCustomForceX] = "ParticleCustomForceX";
    m_settingKey[ParticleCustomForceY] = "ParticleCustomForceY";
    m_settingKey[ParticleCustomForceZ] = "ParticleCustomForceZ";
    m_settingKey[ParticleP2PElectricForce] = "ParticleP2PElectricForce";
    m_settingKey[ParticleP2PMagneticForce] = "ParticleP2PMagneticForce";
    m_settingKey[ChartStartX] = "ChartStartX";
    m_settingKey[ChartStartY] = "ChartStartY";
    m_settingKey[ChartEndX] = "ChartEndX";
    m_settingKey[ChartEndY] = "ChartEndY";
    m_settingKey[ChartTimeX] = "ChartTimeX";
    m_settingKey[ChartTimeY] = "ChartTimeY";
    m_settingKey[ChartHorizontalAxis] = "ChartHorizontalAxis";
    m_settingKey[ChartHorizontalAxisReverse] = "ChartHorizontalAxisReverse";
    m_settingKey[ChartHorizontalAxisPoints] = "ChartHorizontalAxisPoints";
    m_settingKey[ChartVariable] = "ChartVariable";
    m_settingKey[ChartVariableComp] = "ChartVariableComp";
    m_settingKey[ChartMode] = "ChartMode";
    m_settingKey[SolidViewHide] = "SolidViewHide";
}

void PostprocessorSetting::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[ScalarView3DMode] = SceneViewPost3DMode_None;
    m_settingDefault[ScalarView3DLighting] = true;
    m_settingDefault[ScalarView3DAngle] = 240.0;
    m_settingDefault[ScalarView3DBackground] = true;
    m_settingDefault[ScalarView3DHeight] = 4.0;
    m_settingDefault[ScalarView3DBoundingBox] = true;
    m_settingDefault[ScalarView3DSolidGeometry] = true;
    m_settingDefault[DeformScalar] = true;
    m_settingDefault[DeformContour] = true;
    m_settingDefault[DeformVector] = true;
    m_settingDefault[ShowInitialMeshView] = true;
    m_settingDefault[ShowSolutionMeshView] = false;
    m_settingDefault[ContourVariable] = QString();
    m_settingDefault[ShowContourView] = false;
    m_settingDefault[ContoursCount] = 15;
    m_settingDefault[ContoursWidth] = 1.0;
    m_settingDefault[ShowScalarView] = true;
    m_settingDefault[ShowScalarColorBar] = true;
    m_settingDefault[ScalarVariable] = QString();
    m_settingDefault[ScalarVariableComp] = PhysicFieldVariableComp_Undefined;
    m_settingDefault[PaletteType] = Palette_Paruly;
    m_settingDefault[PaletteFilter] = false;
    m_settingDefault[PaletteSteps] = 30;
    m_settingDefault[ScalarRangeLog] = false;
    m_settingDefault[ScalarRangeBase] = 10;
    m_settingDefault[ScalarDecimalPlace] = 4;
    m_settingDefault[ScalarRangeAuto] = true;
    m_settingDefault[ScalarRangeMin] = 0.0;
    m_settingDefault[ScalarRangeMax] = 1.0;
    m_settingDefault[ShowVectorView] = false;
    m_settingDefault[VectorVariable] = QString();
    m_settingDefault[VectorProportional] = true;
    m_settingDefault[VectorColor] = true;
    m_settingDefault[VectorCount] = 50;
    m_settingDefault[VectorScale] = 0.6;
    m_settingDefault[VectorType] = VectorType_Arrow;
    m_settingDefault[VectorCenter] = VectorCenter_Tail;
    m_settingDefault[OrderComponent] = 1;
    m_settingDefault[ShowOrderView] = true;
    m_settingDefault[ShowErrorView] = false;
    m_settingDefault[ShowOrderLabel] = false;
    m_settingDefault[ShowOrderColorBar] = true;
    m_settingDefault[OrderPaletteOrderType] = Palette_Paruly;
    m_settingDefault[ParticleButcherTableType] = Explicit_FEHLBERG_6_45_embedded;
    m_settingDefault[ParticleIncludeRelativisticCorrection] = true;
    m_settingDefault[ParticleMass] = 9.109e-31;
    m_settingDefault[ParticleConstant] = 1.602e-19;
    m_settingDefault[ParticleStartX] = 0.0;
    m_settingDefault[ParticleStartY] = 0.0;
    m_settingDefault[ParticleStartVelocityX] = 0.0;
    m_settingDefault[ParticleStartVelocityY] = 0.0;
    m_settingDefault[ParticleNumberOfParticles] = 1;
    m_settingDefault[ParticleStartingRadius] = 0.0;
    m_settingDefault[ParticleReflectOnDifferentMaterial] = false;
    m_settingDefault[ParticleReflectOnBoundary] = false;
    m_settingDefault[ParticleCoefficientOfRestitution] = 0.0;
    m_settingDefault[ParticleMaximumRelativeError] = 0.01;
    m_settingDefault[ParticleShowPoints] = false;
    m_settingDefault[ParticleShowBlendedFaces] = true;
    m_settingDefault[ParticleNumShowParticlesAxi] = 1;
    m_settingDefault[ParticleColorByVelocity] = true;
    m_settingDefault[ParticleMaximumNumberOfSteps] = 500;
    m_settingDefault[ParticleMaximumStep] = 0.0;
    m_settingDefault[ParticleDragDensity] = 1.2041;
    m_settingDefault[ParticleDragCoefficient] = 0.0;
    m_settingDefault[ParticleDragReferenceArea] = 0.0;
    m_settingDefault[ParticleCustomForceX] = 0.0;
    m_settingDefault[ParticleCustomForceY] = 0.0;
    m_settingDefault[ParticleCustomForceZ] = 0.0;
    m_settingDefault[ParticleP2PElectricForce] = false;
    m_settingDefault[ParticleP2PMagneticForce] = false;
    m_settingDefault[ChartStartX] = 0.0;
    m_settingDefault[ChartStartY] = 0.0;
    m_settingDefault[ChartEndX] = 0.0;
    m_settingDefault[ChartEndY] = 0.0;
    m_settingDefault[ChartTimeX] = 0.0;
    m_settingDefault[ChartTimeY] = 0.0;
    m_settingDefault[ChartHorizontalAxis] = ChartAxis_Length;
    m_settingDefault[ChartHorizontalAxisReverse] = false;
    m_settingDefault[ChartHorizontalAxisPoints] = 200;
    m_settingDefault[ChartVariable] = QString();
    m_settingDefault[ChartVariableComp] = PhysicFieldVariableComp_Undefined;
    m_settingDefault[ChartMode] = ChartMode_Geometry;
    m_settingDefault[SolidViewHide] = QStringList();
}

//void PostprocessorSetting::load(XMLProblem::config *configxsd)
//{
//    // default
//    m_setting = m_settingDefault;

//    for (int i = 0; i < configxsd->item().size(); i ++)
//    {
//        Type key = stringKeyToType(QString::fromStdString(configxsd->item().at(i).key()));

//        if (m_settingDefault.keys().contains(key))
//        {
//            if (m_settingDefault[key].type() == QVariant::Double)
//                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value()).toDouble();
//            else if (m_settingDefault[key].type() == QVariant::Int)
//                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value()).toInt();
//            else if (m_settingDefault[key].type() == QVariant::Bool)
//                m_setting[key] = (QString::fromStdString(configxsd->item().at(i).value()) == "1");
//            else if (m_settingDefault[key].type() == QVariant::String)
//                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value());
//            else if (m_settingDefault[key].type() == QVariant::StringList)
//                m_setting[key] = QString::fromStdString(configxsd->item().at(i).value()).split("|");
//            else
//                qDebug() << "Key not found" << QString::fromStdString(configxsd->item().at(i).key()) << QString::fromStdString(configxsd->item().at(i).value());
//        }
//    }
//}

//void PostprocessorSetting::save(XMLProblem::config *configxsd)
//{
//    foreach (Type key, m_setting.keys())
//    {
//        if (m_settingDefault[key].type() == QVariant::StringList)
//            configxsd->item().push_back(XMLProblem::item(typeToStringKey(key).toStdString(), m_setting[key].toStringList().join("|").toStdString()));
//        else if (m_settingDefault[key].type() == QVariant::Bool)
//            configxsd->item().push_back(XMLProblem::item(typeToStringKey(key).toStdString(), QString::number(m_setting[key].toInt()).toStdString()));
//        else
//            configxsd->item().push_back(XMLProblem::item(typeToStringKey(key).toStdString(), m_setting[key].toString().toStdString()));
//    }
//}

void PostprocessorSetting::load(QJsonObject &object)
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
    }
}

void PostprocessorSetting::save(QJsonObject &object)
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
    }
}

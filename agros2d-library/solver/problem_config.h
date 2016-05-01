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

#ifndef PROBLEM_CONFIG_H
#define PROBLEM_CONFIG_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"

class FieldInfo;
class CouplingInfo;

class ProblemBase;
class ProblemFunctions;
class ProblemParameters;

namespace XMLProblem
{
class config;
class problem_config;
}

typedef QMap<QString, double> StringToDoubleMap;
Q_DECLARE_METATYPE(StringToDoubleMap)

class ProblemConfig : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        // computation
        Unknown,
        Frequency,
        TimeMethod,
        TimeMethodTolerance,
        TimeInitialStepSize,
        TimeOrder,
        TimeConstantTimeSteps,
        TimeTotal,
        Coordinate,
        Mesh,
        // general
        GridStep,
        SnapToGrid
    };

    ProblemConfig(ProblemBase *parentProblem);
    virtual ~ProblemConfig();

    void copy(const ProblemConfig *origin);

    inline QString labelX() { return ((coordinateType() == CoordinateType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((coordinateType() == CoordinateType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((coordinateType() == CoordinateType_Planar) ? "Z" : "a");  }

    void clear();

    // coordinates
    inline CoordinateType coordinateType() const { return m_config[ProblemConfig::Coordinate].value<CoordinateType>(); }
    void setCoordinateType(const CoordinateType coordinateType) { m_config[ProblemConfig::Coordinate] = QVariant::fromValue(coordinateType); emit changed(); }

    // mesh
    inline MeshType meshType() const { return m_config[ProblemConfig::Mesh].value<MeshType>(); }
    void setMeshType(const MeshType meshType) { m_config[ProblemConfig::Mesh] = QVariant::fromValue(meshType); emit changed(); }

    // load and save
    void load(XMLProblem::problem_config *configxsd);
    void save(XMLProblem::problem_config *configxsd);
    void load(QJsonObject &object);
    void save(QJsonObject &object);

    inline QString typeToStringKey(Type type) const { return m_configKey[type]; }
    inline Type stringKeyToType(const QString &key) const { return m_configKey.key(key); }

    inline QVariant value(Type type) const { return m_config[type]; }
    inline void setValue(Type type, int value, bool emitChanged = true) {  m_config[type] = value; if (emitChanged) emit changed(); }
    inline void setValue(Type type, double value, bool emitChanged = true) {  m_config[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, bool value, bool emitChanged = true) {  m_config[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, const QString &value, bool emitChanged = true) { m_config[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, Value value, bool emitChanged = true) { m_config[type] = QVariant::fromValue(value); emit changed(); if (emitChanged) emit changed(); }

    inline QVariant defaultValue(Type type) {  return m_configDefault[type]; }

    // parameters
    inline ProblemParameters *parameters() const { return m_parameters; }
    inline ProblemParameters *parameters() { return m_parameters; }

    // functions
    inline ProblemFunctions *functions() const { return m_functions; }
    inline ProblemFunctions *functions() { return m_functions; }

    inline double constantTimeStepLength() { return value(ProblemConfig::TimeTotal).toDouble() / value(ProblemConfig::TimeConstantTimeSteps).toInt(); }
    double initialTimeStepLength();

    void checkVariableName(const QString &key, const QString &keyToSkip = "");

    void refresh() { emit changed(); }

signals:
    void changed();

private:
    QMap<Type, QVariant> m_config;
    QMap<Type, QVariant> m_configDefault;
    QMap<Type, QString> m_configKey;

    void setDefaultValues();
    void setStringKeys();

    ProblemBase *m_problem;

    // parameters
    ProblemParameters *m_parameters;

    // functions
    ProblemFunctions *m_functions;
};

class PostprocessorSetting : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        Unknown,
        ScalarView3DMode,
        ScalarView3DLighting,
        ScalarView3DAngle,
        ScalarView3DBackground,
        ScalarView3DHeight,
        ScalarView3DBoundingBox,
        ScalarView3DSolidGeometry,
        DeformScalar,
        DeformContour,
        DeformVector,
        ShowInitialMeshView,
        ShowSolutionMeshView,
        ContourVariable,
        ShowContourView,
        ContoursCount,
        ContoursWidth,
        ShowScalarView,
        ShowScalarColorBar,
        ScalarVariable,
        ScalarVariableComp,
        PaletteType,
        PaletteFilter,
        PaletteSteps,
        ScalarRangeLog,
        ScalarRangeBase,
        ScalarDecimalPlace,
        ScalarRangeAuto,
        ScalarRangeMin,
        ScalarRangeMax,
        ShowVectorView,
        VectorVariable,
        VectorProportional,
        VectorColor,
        VectorCount,
        VectorScale,
        VectorType,
        VectorCenter,
        OrderComponent,
        ShowOrderView,
        ShowErrorView,
        ShowOrderColorBar,
        ShowErrorColorBar,
        ShowOrderLabel,
        OrderPaletteOrderType,
        ParticleButcherTableType,
        ParticleIncludeRelativisticCorrection,
        ParticleMass,
        ParticleConstant,
        ParticleStartX,
        ParticleStartY,
        ParticleStartVelocityX,
        ParticleStartVelocityY,
        ParticleNumberOfParticles,
        ParticleStartingRadius,
        ParticleReflectOnDifferentMaterial,
        ParticleReflectOnBoundary,
        ParticleCoefficientOfRestitution,
        ParticleMaximumRelativeError,
        ParticleShowPoints,
        ParticleShowBlendedFaces,
        ParticleNumShowParticlesAxi,
        ParticleColorByVelocity,
        ParticleMaximumNumberOfSteps,
        ParticleMaximumStep,
        ParticleDragDensity,
        ParticleDragCoefficient,
        ParticleDragReferenceArea,
        ParticleCustomForceX,
        ParticleCustomForceY,
        ParticleCustomForceZ,
        ParticleP2PElectricForce,
        ParticleP2PMagneticForce,
        ChartStartX,
        ChartStartY,
        ChartEndX,
        ChartEndY,
        ChartTimeX,
        ChartTimeY,
        ChartHorizontalAxis,
        ChartHorizontalAxisReverse,
        ChartHorizontalAxisPoints,
        ChartVariable,
        ChartVariableComp,
        ChartMode,
        SolidViewHide
    };

    PostprocessorSetting(Computation *computation);

    void copy(const PostprocessorSetting *origin);

    // load and save
    void load(XMLProblem::config *configxsd);
    void save(XMLProblem::config *configxsd);
    void load(QJsonObject &object);
    void save(QJsonObject &object);

    void clear();

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }

    inline QVariant value(Type type) {  return m_setting[type]; }
    inline void setValue(Type type, int value) {  m_setting[type] = value; }
    inline void setValue(Type type, double value) {  m_setting[type] = value; }
    inline void setValue(Type type, bool value) {  m_setting[type] = value; }
    inline void setValue(Type type, const QString &value) { m_setting[type] = value; }
    inline void setValue(Type type, const QStringList &value) { m_setting[type] = value; }

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

private:
    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();

    ProblemBase *m_computation;
};

#endif // PROBLEM_CONFIG_H

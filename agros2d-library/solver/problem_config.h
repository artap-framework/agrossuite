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

namespace XMLProblem
{
    class config;
    class problem_config;
}

typedef QMap<QString, double> ParametersType;
Q_DECLARE_METATYPE(ParametersType)

class ProblemConfig : public QObject
{
    Q_OBJECT
public:
    enum Type
    {
        Unknown,
        Frequency,
        TimeMethod,
        TimeMethodTolerance,
        TimeInitialStepSize,
        TimeOrder,
        TimeConstantTimeSteps,
        TimeTotal,
        Parameters,
        Coordinate,
        Mesh
    };

    ProblemConfig(QWidget *parent = 0);

    inline QString labelX() { return ((coordinateType() == CoordinateType_Planar) ? "X" : "R");  }
    inline QString labelY() { return ((coordinateType() == CoordinateType_Planar) ? "Y" : "Z");  }
    inline QString labelZ() { return ((coordinateType() == CoordinateType_Planar) ? "Z" : "a");  }

    void clear();

    // coordinates
    inline CoordinateType coordinateType() const { return m_setting[ProblemConfig::Coordinate].value<CoordinateType>(); }
    void setCoordinateType(const CoordinateType coordinateType) { m_setting[ProblemConfig::Coordinate] = QVariant::fromValue(coordinateType); emit changed(); }

    // mesh
    inline MeshType meshType() const { return m_setting[ProblemConfig::Mesh].value<MeshType>(); }
    void setMeshType(const MeshType meshType) { m_setting[ProblemConfig::Mesh] = QVariant::fromValue(meshType); emit changed(); }

    // load and save
    void load(XMLProblem::problem_config *configxsd);
    void save(XMLProblem::problem_config *configxsd);
    void load(QJsonObject &object);
    void save(QJsonObject &object);

    inline QString typeToStringKey(Type type) const { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) const { return m_settingKey.key(key); }

    inline QVariant value(Type type) const { return m_setting[type]; }
    inline void setValue(Type type, int value, bool emitChanged = true) {  m_setting[type] = value; if (emitChanged) emit changed(); }
    inline void setValue(Type type, double value, bool emitChanged = true) {  m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, bool value, bool emitChanged = true) {  m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, const QString &value, bool emitChanged = true) { m_setting[type] = value; emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, Value value, bool emitChanged = true) { m_setting[type] = QVariant::fromValue(value); emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, ParametersType parameters, bool emitChanged = true) { m_setting[type] = QVariant::fromValue(parameters); emit changed(); if (emitChanged) emit changed(); }
    inline void setValue(Type type, QMap<QString, QVariant> parameters, bool emitChanged = true) { m_setting[type] = parameters; emit changed(); if (emitChanged) emit changed(); }

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

    // parameters
    void setParameter(const QString &key, double val);
    void checkParameterName(const QString &key);

    inline double constantTimeStepLength() { return value(ProblemConfig::TimeTotal).toDouble() / value(ProblemConfig::TimeConstantTimeSteps).toInt(); }
    double initialTimeStepLength();

    void refresh() { emit changed(); }

signals:
    void changed();

private:
    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();
};

class ProblemSetting : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        Unknown,        
        View_GridStep,
        View_SnapToGrid,
        View_ScalarView3DMode,
        View_ScalarView3DLighting,
        View_ScalarView3DAngle,
        View_ScalarView3DBackground,
        View_ScalarView3DHeight,
        View_ScalarView3DBoundingBox,
        View_ScalarView3DSolidGeometry,
        View_DeformScalar,
        View_DeformContour,
        View_DeformVector,
        View_ShowInitialMeshView,
        View_ShowSolutionMeshView,
        View_ContourVariable,
        View_ShowContourView,
        View_ContoursCount,
        View_ContoursWidth,
        View_ShowScalarView,
        View_ShowScalarColorBar,
        View_ScalarVariable,
        View_ScalarVariableComp,
        View_PaletteType,
        View_PaletteFilter,
        View_PaletteSteps,
        View_ScalarRangeLog,
        View_ScalarRangeBase,
        View_ScalarDecimalPlace,
        View_ScalarRangeAuto,
        View_ScalarRangeMin,
        View_ScalarRangeMax,
        View_ShowVectorView,
        View_VectorVariable,
        View_VectorProportional,
        View_VectorColor,
        View_VectorCount,
        View_VectorScale,
        View_VectorType,
        View_VectorCenter,
        View_OrderComponent,
        View_ShowOrderView,
        View_ShowOrderColorBar,
        View_ShowOrderLabel,
        View_OrderPaletteOrderType,
        View_ParticleButcherTableType,
        View_ParticleIncludeRelativisticCorrection,
        View_ParticleMass,
        View_ParticleConstant,
        View_ParticleStartX,
        View_ParticleStartY,
        View_ParticleStartVelocityX,
        View_ParticleStartVelocityY,
        View_ParticleNumberOfParticles,
        View_ParticleStartingRadius,
        View_ParticleReflectOnDifferentMaterial,
        View_ParticleReflectOnBoundary,
        View_ParticleCoefficientOfRestitution,
        View_ParticleMaximumRelativeError,
        View_ParticleShowPoints,
        View_ParticleShowBlendedFaces,
        View_ParticleNumShowParticlesAxi,
        View_ParticleColorByVelocity,
        View_ParticleMaximumNumberOfSteps,
        View_ParticleMaximumStep,
        View_ParticleDragDensity,
        View_ParticleDragCoefficient,
        View_ParticleDragReferenceArea,
        View_ParticleCustomForceX,
        View_ParticleCustomForceY,
        View_ParticleCustomForceZ,
        View_ParticleP2PElectricForce,
        View_ParticleP2PMagneticForce,
        View_ChartStartX,
        View_ChartStartY,
        View_ChartEndX,
        View_ChartEndY,
        View_ChartTimeX,
        View_ChartTimeY,
        View_ChartHorizontalAxis,
        View_ChartHorizontalAxisReverse,
        View_ChartHorizontalAxisPoints,
        View_ChartVariable,
        View_ChartVariableComp,
        View_ChartMode,
        View_SolidViewHide
    };

    ProblemSetting();
    ~ProblemSetting();

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
};

#endif // PROBLEM_CONFIG_H

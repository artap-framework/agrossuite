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

#ifndef FIELD_H
#define FIELD_H

#include "util/util.h"
#include "value.h"
#include "sceneedge.h"
#include "scenelabel.h"

#undef signals
#include <deal.II/grid/tria.h>
#define signals public

namespace Module
{
    struct ModuleAgros;
}

//namespace XMLProblem
//{
//    class field_config;
//}

class ProblemConfig;
class CouplingInfo;
class LocalForceValue;
class PluginInterface;
class Value;

const int LABEL_OUTSIDE_FIELD = -10000;

class AGROS_LIBRARY_API FieldInfo : public QObject
{
public:
    FieldInfo(QString fieldId);
    ~FieldInfo();

    void copy(const FieldInfo *origin);

    void clear();

    inline PluginInterface *plugin() const { assert(m_plugin); return m_plugin; }

    QString fieldId() const { return m_fieldId; }

    enum Type
    {
        Unknown,
        Analysis,
        Linearity,
        NonlinearResidualNorm,
        NonlinearRelativeChangeOfSolutions,
        NonlinearDampingType,
        NonlinearDampingCoeff,
        NewtonReuseJacobian,
        NewtonJacobianReuseRatio,
        NonlinearDampingFactorDecreaseRatio,
        NewtonMaxStepsReuseJacobian,
        NonlinearStepsToIncreaseDampingFactor,
        PicardAndersonAcceleration,
        PicardAndersonBeta,
        PicardAndersonNumberOfLastVectors,
        SpaceNumberOfRefinements,
        SpacePolynomialOrder,
        Adaptivity,
        AdaptivitySteps,
        AdaptivityTolerance,
        AdaptivityEstimator,
        AdaptivityStrategy,
        AdaptivityStrategyHP,
        AdaptivityFinePercentage,
        AdaptivityCoarsePercentage,
        AdaptivityTransientBackSteps,
        AdaptivityTransientRedoneEach,
        TransientTimeSkip,
        TransientInitialCondition,
        LinearSolver,
        LinearSolverIterDealIIMethod,
        LinearSolverIterDealIIPreconditioner,
        LinearSolverIterToleranceAbsolute,
        LinearSolverIterIters,        
        LinearSolverExternalName,
        LinearSolverExternalCommandEnvironment,
        LinearSolverExternalCommandParameters,
        TimeUnit
    };

    // analysis type
    inline AnalysisType analysisType() const { return m_setting[Analysis].value<AnalysisType>(); }
    void setAnalysisType(const AnalysisType analysisType);

    // linearity
    inline LinearityType linearityType() const {return m_setting[Linearity].value<LinearityType>(); }
    void setLinearityType(const LinearityType linearityType) { m_setting[Linearity] = QVariant::fromValue(linearityType); }

    QList<LinearityType> availableLinearityTypes(AnalysisType at) const;

    // adaptivity
    inline AdaptivityMethod adaptivityType() const { return m_setting[Adaptivity].value<AdaptivityMethod>(); }
    void setAdaptivityType(const AdaptivityMethod adaptivityType) { m_setting[Adaptivity] = QVariant::fromValue(adaptivityType); }

    // matrix
    inline MatrixSolverType matrixSolver() const { return m_setting[LinearSolver].value<MatrixSolverType>(); }
    void setMatrixSolver(const MatrixSolverType matrixSolver) { m_setting[LinearSolver] = QVariant::fromValue(matrixSolver); }

    // number of solutions
    inline int numberOfSolutions() const { return m_numberOfSolutions; }

    const QMap<SceneLabel *, int> labelsRefinement() const { return m_labelsRefinement; }
    int labelRefinement(SceneLabel *label) const;
    void setLabelRefinement(SceneLabel *label, int refinement) { m_labelsRefinement[label] = refinement; }
    void removeLabelRefinement(SceneLabel *label) { m_labelsRefinement.remove(label); }

    const QMap<SceneLabel *, int> labelsPolynomialOrder() { return m_labelsPolynomialOrder; }
    int labelPolynomialOrder(SceneLabel *label);
    void setLabelPolynomialOrder(SceneLabel *label, int order) { m_labelsPolynomialOrder[label] = order; }
    void removeLabelPolynomialOrder(SceneLabel *label) { m_labelsPolynomialOrder.remove(label); }

    // load and save
    // void load(XMLProblem::field_config *configxsd);
    // void save(XMLProblem::field_config *configxsd);
    void load(QJsonObject &object);
    void save(QJsonObject &object);

    inline QString typeToStringKey(Type type) { return m_settingKey[type]; }
    inline Type stringKeyToType(const QString &key) { return m_settingKey.key(key); }
    inline QStringList stringKeys() { return m_settingKey.values(); }

    inline QVariant value(Type type) const { return m_setting[type]; }
    inline void setValue(Type type, int value) {  m_setting[type] = value; }
    inline void setValue(Type type, double value) {  m_setting[type] = value; }
    inline void setValue(Type type, bool value) {  m_setting[type] = value; }
    inline void setValue(Type type, const std::string &value) { setValue(type, QString::fromStdString(value)); }
    inline void setValue(Type type, const QString &value) { m_setting[type] = value; }
    inline void setValue(Type type, const QStringList &value) { m_setting[type] = value; }

    inline QVariant defaultValue(Type type) {  return m_settingDefault[type]; }

    // name
    QString name() const;

    // deformable shape
    bool hasDeformableShape() const;

    // constants
    QMap<QString, double> constants() const;

    // macros
    QMap<QString, QString> macros() const;

    // analyses
    QMap<AnalysisType, QString> analyses() const;

    // material type
    QList<Module::MaterialTypeVariable> materialTypeVariables() const;

    // list of all volume quantities
    QList<QString> allMaterialQuantities() const;

    // variable by name
    bool materialTypeVariableContains(const QString &id) const;
    Module::MaterialTypeVariable materialTypeVariable(const QString &id) const;

    // boundary conditions
    QList<Module::BoundaryType> boundaryTypes() const;
    // variable by name
    bool boundaryTypeContains(const QString &id) const;
    Module::BoundaryType boundaryType(const QString &id) const;
    Module::BoundaryTypeVariable boundaryTypeVariable(const QString &id) const;

    // force
    Module::Force force(CoordinateType coordinateType) const;

    // material and boundary user interface
    Module::DialogUI materialUI() const;
    Module::DialogUI boundaryUI() const;

    // local point variables
    QList<Module::LocalVariable> localPointVariables(CoordinateType coordinateType) const;
    // view scalar and vector variables
    QList<Module::LocalVariable> viewScalarVariables(CoordinateType coordinateType) const;
    QList<Module::LocalVariable> viewVectorVariables(CoordinateType coordinateType) const;
    // surface integrals
    QList<Module::Integral> surfaceIntegrals(CoordinateType coordinateType) const;
    // volume integrals
    QList<Module::Integral> volumeIntegrals(CoordinateType coordinateType) const;

    // variable by name
    Module::LocalVariable localVariable(CoordinateType coordinateType, const QString &id) const;
    Module::Integral surfaceIntegral(CoordinateType coordinateType, const QString &id) const;
    Module::Integral volumeIntegral(CoordinateType coordinateType, const QString &id) const;

    // default variables
    Module::LocalVariable defaultViewScalarVariable(CoordinateType coordinateType) const;
    Module::LocalVariable defaultViewVectorVariable(CoordinateType coordinateType) const;

    QList<LinearityType> availableLinearityTypes() const {return m_availableLinearityTypes;}

    double labelArea(int agrosLabel) const;
private:
    /// plugin
    PluginInterface *m_plugin;

    /// pointer to problem info, whose this object is a "subfield"
    ProblemConfig *m_parent;

    /// unique field info
    QString m_fieldId;

    // number of solutions cache
    int m_numberOfSolutions;

    // linearity
    QList<LinearityType> m_availableLinearityTypes;

    QMap<SceneLabel *, int> m_labelsRefinement;
    QMap<SceneLabel *, int> m_labelsPolynomialOrder;

    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();

    // for speed optimisations
    QMap<QString, QList<QWeakPointer<Value> > > m_valuePointersTable;
    int* m_hermesMarkerToAgrosLabelConversion;
    double* m_labelAreas;
};

ostream& operator<<(ostream& output, FieldInfo& id);

#endif // FIELD_H

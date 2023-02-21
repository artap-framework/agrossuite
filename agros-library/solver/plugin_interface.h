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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtPlugin>

#include "util/util.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"

#include "solver/module.h"
#include "solver/marker.h"
#include "solver/field.h"
#include "solver/solver.h"

#undef signals
#include <deal.II/numerics/data_postprocessor.h>
#define signals public


class PositionInfo;

struct LocalPointValue
{
    LocalPointValue()
    {
        this->scalar = 0.0;
        this->vector = Point();
        this->material = NULL;
    }

    LocalPointValue(double scalar, Point vector, Material *material)
    {
        this->scalar = scalar;
        this->vector = vector;
        this->material = material;
    }

    double scalar;
    Point vector;
    const Material *material;
};

class LocalValue
{
public:
    LocalValue(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, const Point &point)
        : m_computation(computation), m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep), m_point(point) {}
    virtual ~LocalValue()
    {
        m_values.clear();
    }

    // point
    inline Point point() { return m_point; }

    // variables
    QMap<QString, LocalPointValue> values() const { return m_values; }

    virtual void calculate() = 0;

protected:
    // point
    Point m_point;
    // computation
    Computation *m_computation;
    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;

    // variables
    QMap<QString, LocalPointValue> m_values;
};

class IntegralValue
{
public:
    IntegralValue(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
        : m_computation(computation), m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep) {}

    // variables
    inline QMap<QString, double> values() const { return m_values; }

    class AGROS_LIBRARY_API IntegralScratchData
    {
    public:
        IntegralScratchData(const dealii::hp::FECollection<2> &feCollection,
                            const dealii::hp::QCollection<2> &quadratureFormulas,
                            const dealii::hp::QCollection<2-1> &faceQuadratureFormulas);
        IntegralScratchData(const IntegralScratchData &scratch_data);

        dealii::hp::FEValues<2> hp_fe_values;
        dealii::hp::FEFaceValues<2> hp_fe_face_values;
    };

    class AGROS_LIBRARY_API IntegralCopyData
    {
    public:
        IntegralCopyData() : results(QMap<size_t, double>()) {}

        QMap<size_t, double> results;
    };

    virtual void localAssembleSystem(const typename dealii::hp::DoFHandler<2>::active_cell_iterator &cell_int,
                                     IntegralScratchData &scratch_data,
                                     IntegralCopyData &copy_data) = 0;
    virtual void copyLocalToGlobal(const IntegralCopyData &copy_data) = 0;

protected:
    // computation
    Computation *m_computation;

    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;

    AnalysisType m_analysisType;
    CoordinateType m_coordinateType;

    QList<int> surroundings;
    MultiArray ma;

    // variables
    QMap<QString, double> m_values;
};

class ForceValue
{
public:
    ForceValue(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
        : m_computation(computation), m_fieldInfo(fieldInfo), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep) {}

    virtual Point3 force(const Point3 &point, const Point3 &velocity) { return Point3(); }
    virtual bool hasForce() { return false; }

protected:
    // computation
    Computation *m_computation;
    // field info
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
};

// plugin module
class PluginFunctions
{
public:
    // static methods
    static QList<WeakFormKind> weakFormTypeList();
    static QString weakFormTypeStringEnum(WeakFormKind weakformType);

    static QList<CouplingType> couplingFormTypeList();
    static QString couplingTypeStringEnum(CouplingType couplingType);
    static QString couplingTypeToString(QString couplingType);
    static CouplingType couplingTypeFromString(QString couplingType);

    static QList<LinearityType> linearityTypeList();
    static QString linearityTypeStringEnum(LinearityType linearityType);

    static QString physicFieldVariableCompStringEnum(PhysicFieldVariableComp physicFieldVariableComp);

    static QList<CoordinateType> coordinateTypeList();
    static QString coordinateTypeStringEnum(CoordinateType coordinateType);

    static QString analysisTypeStringEnum(AnalysisType analysisType);

    static QString boundaryTypeString(const QString boundaryName);
};

class AGROS_LIBRARY_API PluginWeakFormAnalysis
{
public:
    PluginWeakFormAnalysis() {}

    class AGROS_LIBRARY_API Item
    {
    public:
        Item() : analysis(AnalysisType_Undefined), analysisSource(AnalysisType_Undefined), coupling(CouplingType_Undefined) {}

        class AGROS_LIBRARY_API Variable
        {
        public:
            QString id;
            QString dependency;
            QString nonlinearity_planar;
            QString nonlinearity_axi;
            QString nonlinearity_cart;
        };

        class AGROS_LIBRARY_API Solver
        {
        public:
            class AGROS_LIBRARY_API Matrix
            {
            public:
                QString id;
            };

            class AGROS_LIBRARY_API MatrixTransient
            {
            public:
                QString id;
            };

            class AGROS_LIBRARY_API Vector
            {
            public:
                QString id;
                int coefficient;
                QString variant;
            };

            class AGROS_LIBRARY_API Essential
            {
            public:
                QString id;
            };

            LinearityType linearity;

            QList<Matrix> matrices;
            QList<MatrixTransient> matricesTransient;
            QList<Vector> vectors;
            QList<Essential> essentials;
        };

        QString id;
        QString name;
        QString equation;
        AnalysisType analysis;
        // coupling only
        AnalysisType analysisSource;
        CouplingType coupling;

        QList<Variable> variables;
        QList<Solver> solvers;
    };

    QList<Item> items;
};

class AGROS_LIBRARY_API PluginWeakFormRecipe
{
public:
    class Variable
    {
    public:
        QString id;
        QString shortName;
    };

    class MatrixForm
    {
    public:
        QString id;
        int i, j;
        QString axi;
        QString planar;
        QString cart;
        QString condition;
    };

    class VectorForm
    {
    public:
        QString id;
        int i;
        QString axi;
        QString planar;
        QString cart;
        QString condition;
    };

    // only for surface form
    class EssentialForm
    {
    public:
        QString id;
        int i;
        QString axi;
        QString planar;
        QString cart;
        QString condition;
    };

    QList<Variable> variables;
    QList<MatrixForm> matrixForms;
    QList<VectorForm> vectorForms;
    QList<EssentialForm> essentialForms; // only for surface forms
};

class AGROS_LIBRARY_API PluginPreGroup
{
public:
    class AGROS_LIBRARY_API Quantity
    {
    public:
        QString id;
        QString name;
        QString condition;
        double default_value;
        QString shortname;
        QString shortname_html;
        QString shortname_dependence;
        QString shortname_dependence_html;
        QString unit;
        QString unit_html;
        bool isSource;
        bool isBool;
        QString onlyIf;
        QString onlyIfNot;
    };

    QString name;

    QList<Quantity> quantities;
};

class AGROS_LIBRARY_API PluginPostVariable
{
public:
    class AGROS_LIBRARY_API Expression
    {
    public:
        Expression(AnalysisType analysis = AnalysisType_SteadyState,
                   const QString &planar = "", const QString &planar_x = "", const QString &planar_y = "",
                   const QString &axi = "", const QString &axi_r = "", const QString &axi_z = "",
                   const QString &cart = "", const QString &cart_x = "", const QString &cart_y = "", const QString &cart_z = "")
            : analysis(analysis),
              planar(planar), planar_x(planar_x), planar_y(planar_y),
              axi(axi), axi_r(axi_r), axi_z(axi_z),
              cart(cart), cart_x(cart_x), cart_y(cart_y), cart_z(cart_z) {}

        AnalysisType analysis;
        QString planar, planar_x, planar_y;
        QString axi, axi_r, axi_z;
        QString cart, cart_x, cart_y, cart_z;
    };

    QString id;
    QString name;
    QString type;
    QString shortname;
    QString shortname_html;
    QString unit;
    QString unit_html;

    QList<Expression> expresions;
};

class AGROS_LIBRARY_API PluginConstant
{
public:
    QString id;
    double value;
};

class AGROS_LIBRARY_API PluginMacro
{
public:
    QString id;
    QString expression;
};

class AGROS_LIBRARY_API PluginModuleAnalysis
{
public:
    class Equation
    {
    public:
        QString type;
        int orderIncrease;
    };

    QString id;
    AnalysisType type;
    QString name;
    int solutions;

    QMap<int, Equation> configs;
};

class AGROS_LIBRARY_API PluginModule
{
public:
    QString id;
    QString name;
    bool deformedShape;

    QList<PluginModuleAnalysis> analyses;
    QList<PluginConstant> constants;
    QList<PluginMacro> macros;

    // preprocessor
    QList<PluginPreGroup> preVolumeGroups;
    QList<PluginPreGroup> preSurfaceGroups;

    // postprocessor
    QList<PluginPostVariable> postLocalVariables;
    QList<PluginPostVariable> postVolumeIntegrals;
    QList<PluginPostVariable> postSurfaceIntegrals;

    // processor
    PluginWeakFormRecipe weakFormRecipeVolume;
    PluginWeakFormRecipe weakFormRecipeSurface;

    QList<PluginWeakFormAnalysis> weakFormAnalysisVolume;
    QList<PluginWeakFormAnalysis> weakFormAnalysisSurface;

    void load(const QString &fileName);
    void save(const QString &fileName);
    void read(const QByteArray &content);
    void clear();
};

class AGROS_LIBRARY_API PluginCoupling
{
public:
    QString id;
    QString name;
    QString source;
    QString target;

    QList<PluginConstant> constants;

    // processor
    PluginWeakFormRecipe weakFormRecipeVolume;
    QList<PluginWeakFormAnalysis> weakFormAnalysisVolume;

    void load(const QString &fileName);
    void save(const QString &fileName);
    void read(const QByteArray &content);
    void clear();
};

// plugin interface
class AGROS_LIBRARY_API PluginInterface
{
public:      
    PluginInterface();
    virtual ~PluginInterface();

    virtual QString fieldId() const = 0;
    virtual QStringList couplings() const = 0;

    PluginModule *moduleJson() { return m_moduleJson; }
    inline PluginCoupling *couplingJson(const QString &fieldId) { return m_couplingsJson[fieldId]; }

    // weak forms
    virtual SolverDeal *solverDeal(Computation *computation, const FieldInfo *fieldInfo) = 0;

    // postprocessor
    // filter
    virtual dealii::DataPostprocessorScalar<2> *filter(Computation *computation,
                                                       const FieldInfo *fieldInfo,
                                                       int timeStep,
                                                       int adaptivityStep,
                                                       // MultiArray *ma,
                                                       const QString &variable,
                                                       PhysicFieldVariableComp physicFieldVariableComp) = 0;

    // local values
    virtual std::shared_ptr<LocalValue> localValue(Computation *computation,
                                                   const FieldInfo *fieldInfo,
                                                   int timeStep,
                                                   int adaptivityStep,
                                                   const Point &point) = 0;
    // surface integrals
    virtual std::shared_ptr<IntegralValue> surfaceIntegral(Computation *computation,
                                                           const FieldInfo *fieldInfo,
                                                           int timeStep,
                                                           int adaptivityStep) = 0;
    // volume integrals
    virtual std::shared_ptr<IntegralValue> volumeIntegral(Computation *computation,
                                                          const FieldInfo *fieldInfo,
                                                          int timeStep,
                                                          int adaptivityStep) = 0;
    // force calculation
    virtual std::shared_ptr<ForceValue> force(Computation *computation,
                                              const FieldInfo *fieldInfo,
                                              int timeStep,
                                              int adaptivityStep) = 0;

    // localization
    virtual QString localeName(const QString &name) = 0;
    // description
    virtual QString localeDescription() = 0;

protected:    
    PluginModule *m_moduleJson;
    QMap<QString, PluginCoupling *> m_couplingsJson;
};

QT_BEGIN_NAMESPACE
#define PluginInterface_IID "org.agros.PluginInterface"
Q_DECLARE_INTERFACE(PluginInterface,
                    PluginInterface_IID)
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H

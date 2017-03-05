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
#include "solver/weak_form.h"
#include "solver/solver.h"
#include "solver/solver_nonlinear.h"

#include "../../resources_source/classes/module_xml.h"

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
        IntegralCopyData() : results(QMap<uint, double>()) {}

        QMap<uint, double> results;
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
/*
<module:volume>
  <module:quantity id="electrostatic_permittivity" shortname="el_epsr"/>
  <module:quantity id="electrostatic_charge_density" shortname="el_rho"/>
  <module:matrix_form id="laplace" i="1" j="1" axi="el_epsr * EPS0 * r * (udr * vdr + udz * vdz)"
                                               planar="el_epsr * EPS0 * (udx * vdx + udy * vdy)"
                                               symmetric="1" />
  <module:vector_form id="rhs" i="1" j="1" axi="el_rho * r * vval"
                                           planar="el_rho * vval"
                                           condition="fabs(el_rho) > 0.0" />

  <module:weakforms_volume>
    <module:weakform_volume analysistype="steadystate" equation="-\, \div \left( \varepsilon\,\, \grad \varphi \right) = \rho">
      <module:quantity id="electrostatic_permittivity"/>
      <module:quantity id="electrostatic_charge_density"/>

      <module:linearity_option type="linear">
        <module:matrix_form id="laplace" />
        <module:vector_form id="rhs" />
      </module:linearity_option>

      </module:weakform_volume>
  </module:weakforms_volume>
</module:volume>
*/

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
    QList<EssentialForm> essentialForms;
};

class AGROS_LIBRARY_API PluginWeakFormVolume
{
public:
    class Variable
    {
    public:
        QString id;
        QString dependence;
    };

    class Volume
    {
    public:
        QString equation;
    };

    class Surface
    {
    public:
        QString equation;
    };
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
        bool is_source;
        QString shortname;
        QString shortname_html;
        QString shortname_dependence;
        QString shortname_dependence_html;
        QString unit;
        QString unit_html;
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
        Expression(const QString &analysis = "",
                   const QString &planar = "", const QString &planar_x = "", const QString &planar_y = "",
                   const QString &axi = "", const QString &axi_r = "", const QString &axi_z = "",
                   const QString &cart = "", const QString &cart_x = "", const QString &cart_y = "", const QString &cart_z = "")
            : analysis(analysis),
              planar_x(planar_x), planar_y(planar_y),
              axi_r(axi_r), axi_z(axi_z),
              cart_x(cart_x), cart_y(cart_y), cart_z(cart_z) {}

        QString analysis;
        QString planar, planar_x, planar_y;
        QString axi, axi_r, axi_z;
        QString cart, cart_x, cart_y, cart_z;
    };

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
    QString name;
    int solutions;

    QMap<int, Equation> configs;
};

class AGROS_LIBRARY_API PluginModule
{
public:
    QString id;
    QString name;

    QList<PluginModuleAnalysis> analyses;
    QList<PluginConstant> constants;

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

    void load(const QString &fileName);
    void save(const QString &fileName);
};

// plugin interface
class AGROS_LIBRARY_API PluginInterface
{
public:      
    PluginInterface();
    virtual ~PluginInterface();

    virtual QString fieldId() = 0;

    inline XMLModule::field *module() const { assert(m_module); return m_module; }
    PluginModule *moduleJson() { return m_moduleJson; }

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
    XMLModule::field *m_module;
    PluginModule *m_moduleJson;
};


QT_BEGIN_NAMESPACE
#define PluginInterface_IID "org.agros.PluginInterface"
Q_DECLARE_INTERFACE(PluginInterface,
                    PluginInterface_IID)
QT_END_NAMESPACE

#endif // PLUGIN_INTERFACE_H
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

#ifndef HERMES_FIELD_H
#define HERMES_FIELD_H

#include "form_info.h"

class BDF2Table;
class Computation;

inline double tern(bool condition, double a, double b)
{
    return (condition ? a : b);
}

// to be thrown when string refering to module entity (boundary condition type, etc.) not found
class AgrosModuleException : public AgrosException
{
public:
    AgrosModuleException(const QString &what) : AgrosException(what)
    {
    }
};

class Marker;
class Boundary;
class Material;

struct SceneViewSettings;
template <typename Scalar> struct SolutionArray;

class FieldInfo;
class CouplingInfo;

namespace Module
{
// local variable
class AGROS_LIBRARY_API LocalVariable
{
public:
    struct Expression
    {
        Expression(const QString &scalar = "", const QString &compX = "", const QString &compY = "")
            : m_scalar(scalar), m_compX(compX), m_compY(compY) {}

        // expressions
        inline QString scalar() const { return m_scalar; }
        inline QString compX() const { return m_compX; }
        inline QString compY() const { return m_compY; }

    private:
        // expressions
        QString m_scalar;
        QString m_compX;
        QString m_compY;
    };

    LocalVariable(const QString &id = "",
                  const QString &name = "",
                  const QString &shortname = "",
                  const QString &shortnameHtml = "",
                  const QString &unit = "",
                  const QString &unitHtml = "",
                  bool isScalar = true,
                  const Expression expression = Expression())
        : m_id(id),
          m_name(name),
          m_shortname(shortname),
          m_shortnameHtml(shortnameHtml),
          m_unit(unit),
          m_unitHtml(unitHtml),
          m_isScalar(isScalar),
          m_expression(expression) {}

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }
    // short name
    inline QString shortname() const { return m_shortname; }
    inline QString shortnameHtml() const { return m_shortnameHtml; }
    // unit
    inline QString unit() const { return m_unit; }
    inline QString unitHtml() const { return m_unitHtml; }

    // is scalar variable
    inline bool isScalar() const { return m_isScalar; }

    // expressions
    inline Expression expression() const { return m_expression; }

private:
    // id
    QString m_id;
    // name
    QString m_name;
    // short name
    QString m_shortname;
    QString m_shortnameHtml;
    // unit
    QString m_unit;
    QString m_unitHtml;

    // is scalar variable
    bool m_isScalar;

    // expressions
    Expression m_expression;
};

// force
struct Force
{
    Force(const QString &compX = "", const QString &compY = "", const QString &compZ = "")
        : m_compX(compX), m_compY(compY), m_compZ(compZ) {}

    // expressions
    inline QString compX() const { return m_compX; }
    inline QString compY() const { return m_compY; }
    inline QString compZ() const { return m_compZ; }

private:
    // expressions
    QString m_compX;
    QString m_compY;
    QString m_compZ;
};

// material property
struct MaterialTypeVariable
{    
    MaterialTypeVariable(const QString &id = "", const QString &shortname = "", double defaultValue = 0,
                         const QString &expressionNonlinear = "", bool isTimedep = false, bool isBool = false, QString onlyIf = QString(), QString onlyIfNot = QString(), bool isSource = false)
        : m_id(id), m_shortname(shortname), m_defaultValue(defaultValue),
          m_expressionNonlinear(expressionNonlinear), m_isTimeDep(isTimedep), m_isBool(isBool), m_onlyIf(onlyIf), m_onlyIfNot(onlyIfNot), m_isSource(isSource) {}

    // id
    inline QString id() const { return m_id; }
    // short name
    inline QString shortname() const { return m_shortname; }
    // nonlinear expression
    inline QString expressionNonlinear() const { return m_expressionNonlinear; }
    inline bool isNonlinear() const { return !m_expressionNonlinear.isEmpty(); }
    // timedep
    inline bool isTimeDep() const { return m_isTimeDep; }
    // show as checkbox
    inline bool isBool() const { return m_isBool; }
    // enable only if checkbox with id == m_onlyIf is checked
    inline QString onlyIf() const {return m_onlyIf; }
    // enable only if checkbox with id == m_onlyIf is NOT checked
    inline QString onlyIfNot() const {return m_onlyIfNot; }
    // field source
    inline bool isSource() const { return m_isSource; }

private:
    // id
    QString m_id;
    // short name
    QString m_shortname;
    // default value
    double m_defaultValue;
    // nonlinear expression
    QString m_expressionNonlinear;
    // timedep
    bool m_isTimeDep;
    // bool parameter
    bool m_isBool;
    // only if
    QString m_onlyIf;
    // only if not
    QString m_onlyIfNot;
    // source
    bool m_isSource;
};

// boundary condition type variable
struct BoundaryTypeVariable
{
    BoundaryTypeVariable()
        : m_id(""), m_shortname(""),
          m_isTimeDep(false), m_isSpaceDep(false) {}
    BoundaryTypeVariable(const QString &id, QString shortname,
                         bool isTimedep = false, bool isSpaceDep = false)
        : m_id(id), m_shortname(shortname),
          m_isTimeDep(isTimedep), m_isSpaceDep(isSpaceDep) {}

    // id
    inline QString id() const { return m_id; }
    // short name
    inline QString shortname() const { return m_shortname; }
    // timedep
    inline bool isTimeDep() const { return m_isTimeDep; }
    // spacedep
    inline bool isSpaceDep() const { return m_isSpaceDep; }

private:
    // id
    QString m_id;
    // short name
    QString m_shortname;
    // timedep
    bool m_isTimeDep;
    // spacedep
    bool m_isSpaceDep;
};

// boundary condition type
struct AGROS_LIBRARY_API BoundaryType
{
    BoundaryType(const QString &id, const QString &name, const QString &equation,
                 QList<BoundaryTypeVariable> variables,
                 QList<FormInfo> wfMatrix, QList<FormInfo> wfVector, QList<FormInfo> essential)
        : m_id(id), m_name(name), m_equation(equation), m_variables(variables), m_wfMatrix(wfMatrix), m_wfVector(wfVector), m_essential(essential) {}
    ~BoundaryType();

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }

    // variables
    inline QList<BoundaryTypeVariable> variables() const { return m_variables; }

    // weakform
    inline QList<FormInfo> wfMatrixSurface() const {return m_wfMatrix; }
    inline QList<FormInfo> wfVectorSurface() const {return m_wfVector; }

    // essential
    inline QList<FormInfo> essential() const {return m_essential; }

    // latex equation
    inline QString equation() { return m_equation; }

private:
    // id
    QString m_id;
    // name
    QString m_name;
    // latex equation
    QString m_equation;

    // steady state and harmonic
    QList<FormInfo> m_wfMatrix;
    QList<FormInfo> m_wfVector;
    QList<FormInfo> m_essential;

    // variables
    QList<BoundaryTypeVariable> m_variables;
};

// surface and volume integral value
struct Integral
{
    Integral(const QString &id = "",
             const QString &name = "",
             const QString &shortname = "",
             const QString &shortnameHtml = "",
             const QString &unit = "",
             const QString &unitHtml = "",
             const QString &expression = "",
             const bool isEggShell = false)
        : m_id(id),
          m_name(name),
          m_shortname(shortname),
          m_shortnameHtml(shortnameHtml),
          m_unit(unit),
          m_unitHtml(unitHtml),
          m_expression(expression),
          m_isEggShell(isEggShell) {}

    // id
    inline QString id() const { return m_id; }
    // name
    inline QString name() const { return m_name; }
    // short name
    inline QString shortname() const { return m_shortname; }
    inline QString shortnameHtml() const { return m_shortnameHtml; }
    // unit
    inline QString unit() const { return m_unit; }
    inline QString unitHtml() const { return m_unitHtml; }

    // expressions
    inline QString expression() const { return m_expression; }

    // eggshell
    inline bool isEggShell() const { return m_isEggShell; }

private:
    // id
    QString m_id;
    // name
    QString m_name;
    // short name
    QString m_shortname;
    QString m_shortnameHtml;
    // unit
    QString m_unit;
    QString m_unitHtml;

    // expression
    QString m_expression;

    // eggshell
    bool m_isEggShell;
};

// dialog UI
struct DialogRow
{
    DialogRow(const QString &id = "",
              const QString &name = "",
              const QString &shortname = "",
              const QString &shortnameHtml = "",
              const QString &shortnameDependence = "",
              const QString &shortnameDependenceHtml = "",
              const QString &unit = "",
              const QString &unitHtml = "",
              double defaultValue = 0,
              const QString &condition = "") :
        m_id(id), m_name(name), m_shortname(shortname),
        m_shortnameHtml(shortnameHtml), m_shortnameDependence(shortnameDependence), m_shortnameDependenceHtml(shortnameDependenceHtml),
        m_unit(unit), m_unitHtml(unitHtml), m_defaultValue(defaultValue), m_condition(condition) {}

    inline QString id() const { return m_id; }

    inline QString name() const { return m_name; }
    inline QString shortname() const { return m_shortname; }
    inline QString shortnameHtml() const { return m_shortnameHtml; }
    inline QString shortnameDependence() const { return m_shortnameDependence; }
    inline QString shortnameDependenceHtml() const { return m_shortnameDependenceHtml; }

    inline QString unit() const { return m_unit; }
    inline QString unitHtml() const { return m_unitHtml; }

    inline double defaultValue() const { return m_defaultValue; }
    inline QString condition() const { return m_condition; }

private:
    QString m_id;

    QString m_name;
    QString m_shortname;
    QString m_shortnameHtml;
    QString m_shortnameDependence;
    QString m_shortnameDependenceHtml;

    QString m_unit;
    QString m_unitHtml;

    double m_defaultValue;
    QString m_condition;
};

struct DialogUI
{
    DialogUI() {}
    DialogUI(QMap<QString, QList<Module::DialogRow> > groups) : m_groups(groups) {}

    inline QMap<QString, QList<Module::DialogRow> > groups() const { return m_groups; }
    DialogRow dialogRow(const QString &id);
    void clear();

private:
    QMap<QString, QList<Module::DialogRow> > m_groups;
};

// available modules
AGROS_LIBRARY_API QMap<QString, QString> availableModules();
AGROS_LIBRARY_API QStringList availableCouplings();
}

#endif // HERMES_FIELD_H

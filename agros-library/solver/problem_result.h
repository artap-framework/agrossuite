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

#ifndef PROBLEM_RESULT_H
#define PROBLEM_RESULT_H

#include "util/util.h"
#include "problem_config.h"
#include "util/enums.h"

class AGROS_LIBRARY_API ResultRecipe : public QObject
{
public:
    ResultRecipe(const QString &name, const QString &fieldId, const QString &variable,
                 int timeStep, int adaptivityStep);
    virtual ~ResultRecipe() {}

    virtual ResultRecipeType type() const = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }
    inline QString fieldId() const { return m_fieldId; }
    inline void setFieldId(const QString &fieldId) { m_fieldId = fieldId; }
    inline QString variable() const { return m_variable; }
    inline void setVariable(const QString &variable) { m_variable = variable; }

    int timeStep() const { return m_timeStep; }
    inline void setTimeStep(int step) { m_timeStep = step; }
    int adaptivityStep() const { return m_adaptivityStep; }
    inline void setAdaptivityStep(int step) { m_adaptivityStep = step; }

    virtual double evaluate(Computation *computation) = 0;
    QVariant variant();

    static ResultRecipe *factory(ResultRecipeType type);

protected:
    QString m_name;
    QString m_fieldId;
    QString m_variable;

    int m_timeStep;
    int m_adaptivityStep;

    int timeStep(Computation *computation, FieldInfo *fieldInfo);
    int adaptivityStep(Computation *computation, FieldInfo *fieldInfo);
};

class AGROS_LIBRARY_API LocalValueRecipe : public ResultRecipe
{
public:
    LocalValueRecipe(const QString &name = "", const QString &fieldId = "", const QString &variable = "",
                     int timeStep = -1, int adaptivityStep = -1);
    virtual ~LocalValueRecipe() override {}

    virtual ResultRecipeType type() const override { return ResultRecipeType_LocalPointValue; }

    virtual void load(QJsonObject &object) override;
    virtual void save(QJsonObject &object) override;

    inline Point point() const { return m_point; }
    inline void setPoint(Point point) { m_point = point; }
    inline void setPoint(double x, double y) { m_point = Point(x, y); }

    inline PhysicFieldVariableComp variableComponent() const  { return m_variableComponent; }
    inline void setVariableComponent(PhysicFieldVariableComp component) { m_variableComponent = component; }

    virtual double evaluate(Computation *computation) override;

protected:
    Point m_point;
    PhysicFieldVariableComp m_variableComponent;
};

class AGROS_LIBRARY_API SurfaceIntegralRecipe : public ResultRecipe
{
public:
    SurfaceIntegralRecipe(const QString &name = "", const QString &fieldId = "", const QString &variable = "",
                     int timeStep = -1, int adaptivityStep = -1);
    virtual ~SurfaceIntegralRecipe() override {}
    inline void clear() { m_edges.clear(); }

    virtual ResultRecipeType type() const override { return ResultRecipeType_SurfaceIntegral; }

    virtual void load(QJsonObject &object) override;
    virtual void save(QJsonObject &object) override;

    inline QList<int> edges() const { return m_edges; }
    inline QList<int> &edges() { return m_edges; }
    inline void addEdge(int edge) { m_edges.append(edge); }

    virtual double evaluate(Computation *computation) override;

protected:
    QList<int> m_edges;
};

class AGROS_LIBRARY_API VolumeIntegralRecipe : public ResultRecipe
{
public:
    VolumeIntegralRecipe(const QString &name = "", const QString &fieldId = "", const QString &variable = "",
                     int timeStep = -1, int adaptivityStep = -1);
    virtual ~VolumeIntegralRecipe() override {}
    inline void clear() { m_labels.clear(); }

    virtual ResultRecipeType type() const override { return ResultRecipeType_VolumeIntegral; }

    virtual void load(QJsonObject &object) override;
    virtual void save(QJsonObject &object) override;

    inline QList<int> labels() const { return m_labels; }
    inline QList<int> &labels() { return m_labels; }
    inline void addLabel(int label) { m_labels.append(label); }

    virtual double evaluate(Computation *computation) override;

protected:
    QList<int> m_labels;
};

class AGROS_LIBRARY_API ResultRecipes
{
public:
    ResultRecipes(QList<ResultRecipe *> items = QList<ResultRecipe *>());
    virtual ~ResultRecipes();

    void clear();

    inline void addRecipe(ResultRecipe *recipe) { m_recipes.append(recipe); }
    void removeAll(FieldInfo *field);
    void removeRecipe(ResultRecipe *recipe);
    ResultRecipe *recipe(const QString &name);
    void evaluate(Computation *computation);

    QList<ResultRecipe *> items() const { return m_recipes; }

protected:
    QList<ResultRecipe *> m_recipes;
};

class AGROS_LIBRARY_API ComputationResults
{
public:
    ComputationResults(StringToDoubleMap items = StringToDoubleMap(),
                       QMap<QString, ComputationResultType> types = QMap<QString, ComputationResultType>());
    virtual ~ComputationResults();

    void clear();

    void load(QJsonObject &rootJson);
    void save(QJsonObject &rootJson);

    // results
    inline StringToDoubleMap &items() { return m_results; }
    inline QMap<QString, ComputationResultType> &types() { return m_types; }
    inline double value(const QString &key) const { return m_results[key]; }
    inline ComputationResultType type(const QString &key) const { return m_types[key]; }
    inline bool hasResults() const { return !m_results.isEmpty(); }
    inline void set(const QString &key, double value, ComputationResultType type) { m_results[key] = value; m_types[key] = type; }
    inline void remove(const QString &key) { m_results.remove(key); m_types.remove(key); }

private:
    StringToDoubleMap m_results;
    QMap<QString, ComputationResultType> m_types;
};

#endif // PROBLEM_CONFIG_H

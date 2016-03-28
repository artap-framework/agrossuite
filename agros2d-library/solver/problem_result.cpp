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

#include "problem_result.h"
#include "problem.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

// consts
const QString COMPUTATIONAL_RESULTS = "computational_results";
const QString RESULTS = "results";
const QString TYPES = "types";

const QString TYPE = "type";
const QString NAME = "name";
const QString FIELD = "field";
const QString VARIABLE = "variable";
const QString TIMESTEP = "timestep";
const QString ADAPTIVITYSTEP = "adaptivitystep";

const QString POINT = "point";
const QString POINTX = "x";
const QString POINTY = "y";
const QString COMPONENT = "component";

const QString EDGES = "edges";
const QString LABELS = "labels";

ResultRecipe *ResultRecipe::factory(ResultRecipeType type)
{
    ResultRecipe *recipe = nullptr;
    if (type == ResultRecipeType_LocalValue)
        recipe = new LocalValueRecipe();
    else if (type == ResultRecipeType_SurfaceIntegral)
        recipe = new SurfaceIntegralRecipe();
    else if (type == ResultRecipeType_VolumeIntegral)
        recipe = new VolumeIntegralRecipe();
    else
        assert(0);

    return recipe;
}

ResultRecipe::ResultRecipe(const QString &name, const QString &fieldID, const QString &variable, int timeStep, int adaptivityStep)
    : m_name(name), m_fieldId(fieldID), m_variable(variable), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep) { }

void ResultRecipe::load(QJsonObject &object)
{
    m_name = object[NAME].toString();
    m_fieldId = object[FIELD].toString();
    m_variable = object[VARIABLE].toString();
    m_timeStep = object[TIMESTEP].toInt();
    m_adaptivityStep = object[ADAPTIVITYSTEP].toInt();
}

void ResultRecipe::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[TYPE] = resultRecipeTypeToStringKey(type());
    object[FIELD] = m_fieldId;
    object[VARIABLE] = m_variable;
    object[TIMESTEP] = m_timeStep;
    object[ADAPTIVITYSTEP] = m_adaptivityStep;
}

int ResultRecipe::timeStep(Computation *computation, FieldInfo *fieldInfo)
{
    assert(m_timeStep > 0 || m_timeStep <= computation->solutionStore()->lastTimeStep(fieldInfo));

    if (m_timeStep == -1)
        return computation->solutionStore()->lastTimeStep(fieldInfo);
    else
        return m_timeStep;
}

int ResultRecipe::adaptivityStep(Computation *computation, FieldInfo *fieldInfo)
{
    assert(m_adaptivityStep > 0 || m_adaptivityStep <= computation->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep(computation, fieldInfo)));

    if (m_adaptivityStep == -1)
        return computation->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep(computation, fieldInfo));
    else
        return m_adaptivityStep;
}

QVariant ResultRecipe::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

// *********************************************************************************************************

LocalValueRecipe::LocalValueRecipe(const QString &name, const QString &fieldID, const QString &variable, int timeStep, int adaptivityStep)
    : ResultRecipe(name, fieldID, variable, timeStep, adaptivityStep)
{
    m_point = Point(0, 0);
    m_variableComponent = PhysicFieldVariableComp_Magnitude;
}

void LocalValueRecipe::load(QJsonObject &object)
{
    QJsonObject pointJson = object[POINT].toObject();
    m_point = Point(pointJson[POINTX].toDouble(), pointJson[POINTY].toDouble());
    m_variableComponent = physicFieldVariableCompFromStringKey(object[COMPONENT].toString());

    ResultRecipe::load(object);
}

void LocalValueRecipe::save(QJsonObject &object)
{
    QJsonObject pointJson;
    pointJson[POINTX] = m_point.x;
    pointJson[POINTY] = m_point.y;
    object[POINT] = pointJson;
    object[COMPONENT] = physicFieldVariableCompToStringKey(m_variableComponent);

    ResultRecipe::save(object);
}

double LocalValueRecipe::evaluate(Computation *computation)
{
    if (computation->isSolved() || computation->isSolving())
    {
        FieldInfo *fieldInfo = computation->fieldInfo(m_fieldId);
        std::shared_ptr<LocalValue> localValue = fieldInfo->plugin()->localValue(computation, fieldInfo,
                                                                                 timeStep(computation, fieldInfo),
                                                                                 adaptivityStep(computation, fieldInfo),
                                                                                 m_point);

        QMap<QString, LocalPointValue> values(localValue->values());

        Module::LocalVariable localVariable = fieldInfo->localVariable(computation->config()->coordinateType(), variable());
        if (localVariable.isScalar())
        {
            return values[m_variable].scalar;
        }
        else
        {
            switch (m_variableComponent)
            {
            case PhysicFieldVariableComp_Magnitude:
                return values[m_variable].vector.magnitude();
                break;
            case PhysicFieldVariableComp_X:
                return values[m_variable].vector.x;
                break;
            case PhysicFieldVariableComp_Y:
                return values[m_variable].vector.y;
                break;
            default:
                assert(0);
            }
        }
    }

    return 0;
}

// *********************************************************************************************************

SurfaceIntegralRecipe::SurfaceIntegralRecipe(const QString &name, const QString &fieldID, const QString &variable, int timeStep, int adaptivityStep)
    : ResultRecipe(name, fieldID, variable, timeStep, adaptivityStep)
{
    m_edges = QList<int>();
}

void SurfaceIntegralRecipe::load(QJsonObject &object)
{
    QJsonArray edgesJson = object[EDGES].toArray();
    for (int i = 0; i < edgesJson.size(); i++)
        m_edges.append(edgesJson[i].toInt());

    ResultRecipe::load(object);
}

void SurfaceIntegralRecipe::save(QJsonObject &object)
{
    QJsonArray edgesJson;
    foreach (int label, m_edges)
        edgesJson.append(label);
    object[EDGES] = edgesJson;

    ResultRecipe::save(object);
}

double SurfaceIntegralRecipe::evaluate(Computation *computation)
{
    if (computation->isSolved() || computation->isSolving())
    {
        FieldInfo *fieldInfo = computation->fieldInfo(m_fieldId);
        computation->scene()->selectNone();

        if (!m_edges.isEmpty())
        {
            foreach (int edge, m_edges)
            {
                assert((edge >= 0) && (edge < computation->scene()->faces->length()));
                computation->scene()->faces->at(edge)->setSelected(true);
            }
        }
        else
        {
            computation->scene()->selectAll(SceneGeometryMode_OperateOnEdges);
        }

        std::shared_ptr<IntegralValue> integralValue = fieldInfo->plugin()->surfaceIntegral(computation, fieldInfo,
                                                                                            timeStep(computation, fieldInfo),
                                                                                            adaptivityStep(computation, fieldInfo));

        QMap<QString, double> values(integralValue->values());
        return values[variable()];
    }

    return 0.0;
}

// *********************************************************************************************************

VolumeIntegralRecipe::VolumeIntegralRecipe(const QString &name, const QString &fieldID, const QString &variable, int timeStep, int adaptivityStep)
    : ResultRecipe(name, fieldID, variable, timeStep, adaptivityStep)
{
    m_labels = QList<int>();
}

void VolumeIntegralRecipe::load(QJsonObject &object)
{
    QJsonArray labelsJson = object[LABELS].toArray();
    for (int i = 0; i < labelsJson.size(); i++)
        m_labels.append(labelsJson[i].toInt());

    ResultRecipe::load(object);
}

void VolumeIntegralRecipe::save(QJsonObject &object)
{
    QJsonArray labelsJson;
    foreach (int label, m_labels)
        labelsJson.append(label);
    object[LABELS] = labelsJson;

    ResultRecipe::save(object);
}

double VolumeIntegralRecipe::evaluate(Computation *computation)
{
    if (computation->isSolved() || computation->isSolving())
    {
        FieldInfo *fieldInfo = computation->fieldInfo(m_fieldId);
        computation->scene()->selectNone();

        if (!m_labels.isEmpty())
        {
            foreach (int label, m_labels)
            {
                assert((label >= 0) && (label < computation->scene()->labels->length()));
                assert(computation->scene()->labels->at(label)->marker(fieldInfo) != computation->scene()->materials->getNone(fieldInfo));
                computation->scene()->labels->at(label)->setSelected(true);
            }
        }
        else
        {
            computation->scene()->selectAll(SceneGeometryMode_OperateOnLabels);
        }

        std::shared_ptr<IntegralValue> integralValue = fieldInfo->plugin()->volumeIntegral(computation, fieldInfo,
                                                                                           timeStep(computation, fieldInfo),
                                                                                           adaptivityStep(computation, fieldInfo));

        QMap<QString, double> values(integralValue->values());
        return values[variable()];
    }

    return 0.0;
}

// *****************************************************************************************************************

ResultRecipes::ResultRecipes(QList<ResultRecipe *> recipes)
    : m_recipes(recipes) { }

ResultRecipes::~ResultRecipes()
{
    clear();
}

void ResultRecipes::clear()
{
    for (int i = 0; i < m_recipes.count(); i++)
        delete m_recipes[i];
    m_recipes.clear();
}

void ResultRecipes::removeAll(FieldInfo *field)
{
    foreach (ResultRecipe *recipe, m_recipes)
        if (recipe->fieldId() == field->fieldId())
            removeRecipe(recipe);
}

void ResultRecipes::removeRecipe(ResultRecipe *recipe)
{
    m_recipes.removeAll(recipe);
}

ResultRecipe *ResultRecipes::recipe(const QString &name)
{
    for (int i = 0; i < m_recipes.count(); i++)
        if (m_recipes[i]->name() == name)
            return m_recipes[i];

    assert(0);
}

void ResultRecipes::evaluate(Computation *computation)
{
    foreach (ResultRecipe *recipe, m_recipes)
        computation->results()->setResult(recipe->name(), recipe->evaluate(computation), ComputationResultType_Recipe);
}

// *****************************************************************************************************************

ComputationResults::ComputationResults(StringToDoubleMap results, QMap<QString, ComputationResultType> types)
    : m_results(results), m_types(types) { }

ComputationResults::~ComputationResults()
{
}

void ComputationResults::clear()
{
    m_results.clear();
}

void ComputationResults::load(QJsonObject &rootJson)
{
    // results
    QJsonObject computationResultsJson = rootJson[COMPUTATIONAL_RESULTS].toObject();

    // values
    QJsonObject resultsJson = computationResultsJson[RESULTS].toObject();
    foreach (QString key, resultsJson.keys())
    {
        m_results[key] = resultsJson[key].toDouble();
    }

    // types
    QJsonObject typesJson = computationResultsJson[TYPES].toObject();
    foreach (QString key, typesJson.keys())
    {
        m_types[key] = computationResultTypeFromStringKey(typesJson[key].toString());
    }
}

void ComputationResults::save(QJsonObject &rootJson)
{
    // results
    QJsonObject computationResultsJson;

    // values
    QJsonObject resultsJson;
    for (StringToDoubleMap::const_iterator i = m_results.constBegin(); i != m_results.constEnd(); ++i)
    {
        resultsJson[i.key()] = i.value();
    }
    computationResultsJson[RESULTS] = resultsJson;

    // values
    QJsonObject typesJson;
    for (QMap<QString, ComputationResultType>::const_iterator i = m_types.constBegin(); i != m_types.constEnd(); ++i)
    {
        typesJson[i.key()] = computationResultTypeToStringKey(i.value());
    }
    computationResultsJson[TYPES] = typesJson;


    rootJson[COMPUTATIONAL_RESULTS] = computationResultsJson;
}

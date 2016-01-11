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
const QString RESULTS = "results";
const QString INFO = "info";

const QString RECIPES = "recipes";
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

ResultRecipe::ResultRecipe(const QString &name, const QString &fieldID, const QString &variable, int timeStep, int adaptivityStep)
    : m_name(name), m_fieldID(fieldID), m_variable(variable), m_timeStep(timeStep), m_adaptivityStep(adaptivityStep) { }

void ResultRecipe::load(QJsonObject &object)
{
    m_name= object[NAME].toString();
    m_fieldID = object[FIELD].toString();
    m_variable = object[VARIABLE].toString();
    m_timeStep = object[TIMESTEP].toInt();
    m_adaptivityStep = object[ADAPTIVITYSTEP].toInt();
}

void ResultRecipe::save(QJsonObject &object)
{
    object[NAME] = m_name;
    object[FIELD] = m_fieldID;
    object[VARIABLE] = m_variable;
    object[TIMESTEP] = m_timeStep;
    object[ADAPTIVITYSTEP] = m_adaptivityStep;
}

int ResultRecipe::timeStep(QSharedPointer<Computation> computation, FieldInfo *fieldInfo)
{
    assert(m_timeStep > 0 || m_timeStep <= computation->solutionStore()->lastTimeStep(fieldInfo));

    if (m_timeStep == -1)
        return computation->solutionStore()->lastTimeStep(fieldInfo);
    else
        return m_timeStep;
}

int ResultRecipe::adaptivityStep(QSharedPointer<Computation> computation, FieldInfo *fieldInfo)
{
    assert(m_adaptivityStep > 0 || m_adaptivityStep <= computation->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep(computation, fieldInfo)));

    if (m_adaptivityStep == -1)
        return computation->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep(computation, fieldInfo));
    else
        return m_adaptivityStep;
}

LocalValueRecipe::LocalValueRecipe(const QString &name, const QString &fieldID, const QString &variable, int timeStep, int adaptivityStep)
    : ResultRecipe(name, fieldID, variable, timeStep, adaptivityStep)
{
    m_point = Point(0, 0);
    // TODO: component
}

void LocalValueRecipe::load(QJsonObject &object)
{
    QJsonObject pointJson = object[POINT].toObject();
    m_point = Point(pointJson[POINTX].toDouble(), pointJson[POINTX].toDouble());
    m_component = object[COMPONENT].toString();
}

void LocalValueRecipe::save(QJsonObject &object)
{
    QJsonObject pointJson;
    pointJson[POINTX] = m_point.x;
    pointJson[POINTY] = m_point.y;
    object[POINT] = pointJson;
    object[COMPONENT] = m_component;
}

double LocalValueRecipe::evaluate(QSharedPointer<Computation> computation)
{
    if (computation->isSolved() or computation->isSolving())
    {
        FieldInfo *fieldInfo = computation->fieldInfo(fieldID());
        std::shared_ptr<LocalValue> localValue = fieldInfo->plugin()->localValue(computation.data(), fieldInfo,
                                                                                 timeStep(computation, fieldInfo),
                                                                                 adaptivityStep(computation, fieldInfo),
                                                                                 m_point);

        QMap<QString, LocalPointValue> values(localValue->values());

        Module::LocalVariable localVariable = fieldInfo->localVariable(computation->config()->coordinateType(), variable());
        if (localVariable.isScalar())
        {
            return values[variable()].scalar;
        }
        else
        {
            // TODO: component
            return values[variable()].vector.magnitude();
        }
    }
}

// *****************************************************************************************************************

ResultRecipes::ResultRecipes(QList<ResultRecipe *> recipes)
    : m_recipes(recipes) { }

ResultRecipes::~ResultRecipes()
{
    //delete m_recipes;
}

bool ResultRecipes::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open result file.");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject storeJson = doc.object();

    QJsonArray recipesJson = storeJson[RECIPES].toArray();
    for (int i = 0; i < recipesJson.size(); i++)
    {
        QJsonObject recipeJson = recipesJson[i].toObject();
        ResultRecipeType type = resultRecipeTypeFromStringKey(recipeJson[TYPE].toString());

        ResultRecipe *recipe = nullptr;
        if (type == ResultRecipeType_LocalValue)
            recipe = new LocalValueRecipe();
        else
            assert(0);

        recipe->load(recipeJson);
        m_recipes.append(recipe);
    }

    return true;
}

bool ResultRecipes::save(const QString &fileName)
{
    if (m_recipes.isEmpty())
    {
        if (QFile::exists(fileName))
            QFile::remove(fileName);
        return true;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open result file.");
        return false;
    }

    QJsonObject storeJson;
    QJsonObject recipesJson;
    foreach (ResultRecipe *recipe, m_recipes)
        recipe->save(recipesJson);
    storeJson[RECIPES] = recipesJson;

    // save to file
    QJsonDocument doc(storeJson);
    file.write(doc.toJson());

    return true;
}

void ResultRecipes::evaluate(QSharedPointer<Computation> computation)
{
    foreach (ResultRecipe *recipe, m_recipes)
        computation->results()->setResult(recipe->name(), recipe->evaluate(computation));
}

// *****************************************************************************************************************

ComputationResults::ComputationResults(StringToDoubleMap results, StringToVariantMap info)
    : m_results(results), m_info(info) { }

ComputationResults::~ComputationResults()
{
    //delete m_results;
    //delete m_info;
}

void ComputationResults::clear()
{
    m_results.clear();
    m_info.clear();
}

bool ComputationResults::load(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open result file.");
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject storeJson = doc.object();

    // results
    QJsonObject resultsJson = storeJson[RESULTS].toObject();
    foreach (QString key, resultsJson.keys())
    {
        m_results[key] = resultsJson[key].toDouble();
    }

    // info
    QJsonObject infoJson = storeJson[INFO].toObject();
    foreach (QString key, infoJson.keys())
    {
        m_info[key] = QVariant::fromValue(infoJson[key].toObject());
    }

    return true;
}

bool ComputationResults::save(const QString &fileName)
{
    if (m_results.isEmpty() && m_info.isEmpty())
    {
        if (QFile::exists(fileName))
            QFile::remove(fileName);

        return true;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open result file.");
        return false;
    }

    // root object
    QJsonObject storeJson;

    // results
    QJsonObject resultsJson;
    for (StringToDoubleMap::const_iterator i = m_results.constBegin(); i != m_results.constEnd(); ++i)
    {
        resultsJson[i.key()] = i.value();
    }
    storeJson[RESULTS] = resultsJson;

    // info
    QJsonObject infoJson;
    for (StringToVariantMap::const_iterator i = m_info.constBegin(); i != m_info.constEnd(); ++i)
    {
        infoJson[i.key()] = i.value().toJsonValue();
    }
    storeJson[INFO] = infoJson;

    // save to file
    QJsonDocument doc(storeJson);
    file.write(doc.toJson());

    return true;
}

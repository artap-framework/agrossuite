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
const QString FIELD = "field";
const QString VARIABLE = "variable";

const QString POINT = "point";
const QString COMPONENT = "component";

void ResultRecipe::load(QJsonObject &object)
{
    m_fieldID = object[FIELD].toString();
    m_variable = object[VARIABLE].toString();
}

void ResultRecipe::save(QJsonObject &object)
{
    object[FIELD] = m_fieldID;
    object[VARIABLE] = m_variable;
}

void LocalValueRecipe::load(QJsonObject &object)
{
    QJsonArray pointJson = object[POINT].toArray();
    assert(pointJson.size() == 2);
    m_point = Point(pointJson.first().toDouble(), pointJson.last().toDouble());

    m_component = object[COMPONENT].toString();
}

void LocalValueRecipe::save(QJsonObject &object)
{
    QJsonArray pointJson;
    pointJson.append(m_point.x);
    pointJson.append(m_point.y);
    object[POINT] = pointJson;

    object[COMPONENT] = m_component;
}

double LocalValueRecipe::evaluate(QSharedPointer<Computation> computation)
{
    if (computation->isSolved() or computation->isSolving())
    {
        FieldInfo *fieldInfo = computation->fieldInfo(fieldID());

        // TODO: optional time and adaptivity step
        int timeStep = computation->solutionStore()->lastTimeStep(fieldInfo);
        int adaptivityStep = computation->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

        std::shared_ptr<LocalValue> localValue = fieldInfo->plugin()->localValue(computation.data(), fieldInfo, timeStep, adaptivityStep, m_point);
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
    else
    {
        throw logic_error(QObject::tr("Problem is not solved.").toStdString());
    }
}

// *****************************************************************************************************************

QString ResultRecipes::fileName()
{
    return QString("%1/recipes.json").arg(cacheProblemDir());
}

bool ResultRecipes::load()
{
    QFile file(fileName());
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

        ResultRecipe *recipe= nullptr;
        if (type == ResultRecipeType_LocalValue)
            recipe = new LocalValueRecipe();
        else
            assert(0);

        recipe->load(recipeJson);
        m_recipes.append(recipe);
    }

    return true;
}

bool ResultRecipes::save()
{
    QString fn = fileName();
    if (m_recipes.isEmpty())
    {
        if (QFile::exists(fn))
            QFile::remove(fn);
        return true;
    }

    QFile file(fn);
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

ComputationResults::ComputationResults()
{
    clear();
}

void ComputationResults::clear()
{
    m_results.clear();
    m_info.clear();
}

QString ComputationResults::fileName()
{
    return QString("%1/%2/results.json").arg(cacheProblemDir()).arg(Agros2D::problem()->currentComputation()->problemDir());
}

bool ComputationResults::load()
{
    QFile file(fileName());
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

    //TODO: info

    return true;
}

bool ComputationResults::save()
{
    QString fn = fileName();
    if (m_results.isEmpty() && m_info.isEmpty())
    {
        if (QFile::exists(fn))
            QFile::remove(fn);

        return true;
    }

    QFile file(fn);
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

    //TODO: info

    // save to file
    QJsonDocument doc(storeJson);
    file.write(doc.toJson());

    return true;
}

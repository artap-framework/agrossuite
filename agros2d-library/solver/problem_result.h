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

#include "util.h"
#include "problem_config.h"
#include "util/enums.h"

class ResultRecipe
{
public:
    virtual ResultRecipeType type() = 0;

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline QString name() { return m_name; }
    inline void setName(const QString name) { m_name = name; }
    inline QString fieldID() { return m_fieldID; }
    inline void setFieldID(const QString fieldID) { m_fieldID = fieldID; }
    inline QString variable() { return m_variable; }
    inline void setVariable(const QString variable) { m_variable = variable; }

    virtual double evaluate(QSharedPointer<Computation> computation) = 0;

protected:
    QString m_name;
    QString m_fieldID;
    QString m_variable;
};

class LocalValueRecipe : public ResultRecipe
{
public:
    //LocalValueRecipe(Point point, QString component);
    virtual ResultRecipeType type() { return ResultRecipeType_LocalValue; }

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    inline Point point() { return m_point; }
    inline void setPoint(Point point) { m_point = point; }

    virtual double evaluate(QSharedPointer<Computation> computation);

protected:
    Point m_point;
    QString m_component;
};

class ProblemResults
{
public:
    ProblemResults();
    ~ProblemResults() {}

    void clear();

    bool load(const QString &fileName);
    bool save(const QString &fileName);

    inline double resultValue(const QString &key) const { return m_results[key]; }
    inline StringToDoubleMap &results() { return m_results; }
    inline bool hasResults() const { return !m_results.isEmpty(); }
    inline void setResult(const QString &key, double value) { m_results[key] = value; }
    inline void removeResult(const QString &key) { m_results.remove(key); }

    inline void addRecipe(ResultRecipe *recipe) { m_recipes.append(recipe); }
    void evaluate(QSharedPointer<Computation> computation);

    inline QMap<QString, QVariant> &info() { return m_info; }

private:
    StringToDoubleMap m_results;
    QMap<QString, QVariant> m_info;
    QList<ResultRecipe *> m_recipes;
};

#endif // PROBLEM_CONFIG_H

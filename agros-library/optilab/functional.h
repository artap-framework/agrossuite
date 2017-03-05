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

#ifndef FUNCTIONAL_H
#define FUNCTIONAL_H

#include "util/util.h"
#include "util/enums.h"

class Computation;

class Functional
{
public:
    Functional(const QString &name = "", const QString &expression = "", int weight = 100);

    void load(QJsonObject &object);
    void save(QJsonObject &object);

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }
    inline QString expression() const { return m_expression; }
    inline void setExpression(const QString &expression) { m_expression = expression; }
    inline int weight() const { return m_weight; }
    inline void setWeight(int weight) { m_weight = weight; }

    bool checkExpression(QSharedPointer<Computation> computation);
    bool evaluateExpression(QSharedPointer<Computation> computation);

protected:
    QString m_name;
    QString m_expression;
    int m_weight;
};

#endif // FUNCTIONAL_H

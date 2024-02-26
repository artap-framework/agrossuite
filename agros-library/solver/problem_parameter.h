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

#ifndef PROBLEM_PARAMETER_H
#define PROBLEM_PARAMETER_H

#include "util/util.h"
#include "value.h"
#include "solutiontypes.h"

class FieldInfo;
class CouplingInfo;

class ProblemBase;

class AGROS_LIBRARY_API ProblemParameter
{
public:
    ProblemParameter(const QString &name = "", double value = 0.0) : m_name(name), m_value(value) {}

    inline QString name() const { return m_name; }
    inline void setName(const QString &name) { m_name = name; }
    inline double value() const { return m_value; }
    inline void setValue(double value) { m_value = value; }

protected:
    QString m_name;
    double m_value;
};

class AGROS_LIBRARY_API ProblemParameters
{
public:
    ProblemParameters(const QList<ProblemParameter> parameters = QList<ProblemParameter>());
    virtual ~ProblemParameters();

    void clear();

    void load(QJsonObject &object);
    void save(QJsonObject &object);

    // parameters
    void set(const QString &key, double val);
    void set(const QList<ProblemParameter> parameters);
    inline double number(const QString &name) const { return parameter(name).value(); }

    void add(const ProblemParameter parameter);
    void remove(const QString &name);
    ProblemParameter parameter(const QString &name) const;

    // symbol table
    exprtk::symbol_table<double> &symbolTable() { return m_parametersSymbolTable; }

    QMap<QString, ProblemParameter> items() const { return m_parameters; }

protected:
    QMap<QString, ProblemParameter> m_parameters;
    exprtk::symbol_table<double> m_parametersSymbolTable;
};

#endif // PROBLEM_PARAMETER_H

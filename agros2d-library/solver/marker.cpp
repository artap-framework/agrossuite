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

#include "marker.h"
#include "module.h"
#include "scene.h"
#include "util.h"
#include "solver/field.h"
#include "solver/problem.h"

Marker::Marker(const FieldInfo *fieldInfo, QString name)
    : m_fieldInfo(fieldInfo), m_name(name), m_isNone(false)
{
}

Marker::~Marker()
{
    m_valuesHash.clear();
    m_values.clear();
}

const QSharedPointer<Value> Marker::value(const QString &name) const
{
    assert(!name.isEmpty());
    assert(m_valuesHash.contains(name));

    return value(m_valuesHash[name]);
}

const QSharedPointer<Value> Marker::value(const uint id) const
{
    assert(m_values.contains(id));

    return m_values[id];
}


const Value* Marker::valueNakedPtr(const QString &name) const
{    
    return value(name).data();
}

const Value* Marker::valueNakedPtr(const uint id) const
{
    return value(id).data();
}

const QMap<uint, QSharedPointer<Value> > Marker::values() const
{
    return m_values;
}

bool Marker::contains(const QString &name) const
{    
    return m_valuesHash.contains(name);
}

const QString Marker::valueName(const uint id) const
{
    assert(m_valuesHash.values().contains(id));

    return m_valuesHash.key(id);
}

void Marker::setValue(const QString& name, Value value)
{
#pragma omp critical
    {
        uint hsh = qHash(name);
        m_valuesHash[name] = hsh;
        m_values[hsh] = QSharedPointer<Value>(new Value(value));
    }
}

void Marker::modifyValue(const QString& name, Value value)
{
#pragma omp critical
    {
        if ((m_valuesHash.contains(name) && m_values.contains(m_valuesHash[name])))
        {
            // existing value
            *m_values[m_valuesHash[name]].data() = value;
        }
        else
        {
            // new value
            uint hsh = qHash(name);
            m_valuesHash[name] = hsh;
            m_values[hsh] = QSharedPointer<Value>(new Value(value));
        }
    }
}

bool Marker::evaluate(const QString &name, double time)
{
    return evaluate(m_valuesHash[name], time);
}

bool Marker::evaluate(const uint id, double time)
{
    return m_values[id]->evaluateAtTime(time);
}

bool Marker::evaluateAllVariables()
{
    foreach (uint id, m_values.keys())
        if (!evaluate(id, 0.0))
            return false;

    return true;
}

QString Marker::fieldId()
{
    return m_fieldInfo->fieldId();
}

Boundary::Boundary(const FieldInfo *fieldInfo, QString name, QString type,
                   QMap<QString, Value> values) : Marker(fieldInfo, name)
{
    // type
    setType(type);

    // set values
    foreach (QString id, values.keys())
        setValue(id, values[id]);

    if (!isNone() && !m_type.isEmpty())
    {
        foreach(Module::BoundaryType boundaryType, fieldInfo->boundaryTypes())
        {
            foreach (Module::BoundaryTypeVariable variable, boundaryType.variables())
            {
                if (!this->contains(variable.id()))
                {
                    // default for GUI
                    Module::DialogRow row = fieldInfo->boundaryUI().dialogRow(variable.id());
                    setValue(variable.id(), Value(QString::number(row.defaultValue())));
                }
            }
        }
    }
}

Material::Material(const FieldInfo *fieldInfo, QString name,
                   QMap<QString, Value> values) : Marker(fieldInfo, name)
{
    // set values
    foreach(QString id, values.keys())
        setValue(id, values[id]);

    // set values
    if (name != "none")
    {
        QList<Module::MaterialTypeVariable> materialTypeVariables = fieldInfo->materialTypeVariables();
        foreach (Module::MaterialTypeVariable variable, materialTypeVariables)
        {
            if (!this->contains(variable.id()))
            {
                // default for GUI
                Module::DialogRow row = fieldInfo->materialUI().dialogRow(variable.id());
                setValue(variable.id(), Value(QString::number(row.defaultValue())));
            }
        }

        foreach (QString id, m_fieldInfo->allMaterialQuantities())
        {
            if (!this->contains(id))
            {
                setValue(id, Value(QString::number(0)));
            }
        }
    }
}

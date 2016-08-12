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

#ifndef SCENEMAR_H
#define SCENEMAR_H

#include "util/util.h"
#include "solver/marker.h"

class SceneBoundary;
class SceneMaterial;
class Marker;
class FieldInfo;
class ProblemBase;
class Scene;

Q_DECLARE_METATYPE(SceneMaterial *)
Q_DECLARE_METATYPE(SceneBoundary *)

template <typename MarkerType>
class AGROS_LIBRARY_API MarkerContainer
{
public:
    MarkerContainer() : data(QList<MarkerType* >()) {}
    ~MarkerContainer();

    /// items() should be removed step by step from the code.
    /// more methods operating with list data should be defined here
    QList<MarkerType*> items() { return data; }

    /// add marker.
    virtual void add(MarkerType *marker);

    /// remove marker
    void remove(MarkerType *marker);

    /// remove all markers of this field
    void removeFieldMarkers(const FieldInfo *fieldInfo);

    /// get marker at position i
    MarkerType *at(int i) const;

    /// get marker by name
    MarkerType *get(const QString &name) const;

    /// get none marker
    MarkerType *getNone(const FieldInfo *field);

    /// filter field
    MarkerContainer<MarkerType> filter(const QString &fieldName);
    MarkerContainer<MarkerType> filter(const FieldInfo *fieldInfo);

    /// length of the array
    inline int length() const { return data.length(); }

    /// if contains exactly one element, return it. Otherwise return NULL
    MarkerType* getSingleOrNull();

    /// checks if array is empty
    inline int isEmpty() { return data.isEmpty(); }

    /// bool if contains given marker (pointer)
    bool contains(MarkerType* marker) const {return data.contains(marker);}

    /// clear and delete data
    void clear();

    /// create or delete markers for new or removed field
    void doFieldsChanged(ProblemBase *problem);

protected:
    QList<MarkerType* > data;
    QMap<const FieldInfo*, MarkerType*> noneMarkers;
};

class SceneMaterial : public Material
{
public:
    SceneMaterial(Scene *scene, const FieldInfo *fieldInfo, QString name,
                  QMap<QString, Value> values = (QMap<QString, Value>())) : Material(scene, fieldInfo, name, values) {}

    QVariant variant();
};

class SceneMaterialNone : public SceneMaterial
{
public:
    SceneMaterialNone(Scene *scene) : SceneMaterial(scene, NULL, "none") {}

    QString script() { return ""; }
    QMap<QString, QString> data() { return QMap<QString, QString>(); }
};


class SceneBoundary : public Boundary
{
public:
    SceneBoundary(Scene *scene, const FieldInfo *fieldInfo, QString name = "", QString type = "",
                  QMap<QString, Value> values = (QMap<QString, Value>())) : Boundary(scene, fieldInfo, name, type, values) {}

    QVariant variant();
};

class SceneBoundaryNone : public SceneBoundary
{
public:
    SceneBoundaryNone(Scene *scene) : SceneBoundary(scene, NULL, "none") {}

    QString script() { return ""; }
};

class SceneBoundaryContainer : public MarkerContainer<SceneBoundary>
{

};

class SceneMaterialContainer : public MarkerContainer<SceneMaterial>
{

};


#endif // SCENEMARKER_H

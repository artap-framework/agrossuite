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

#ifndef SCENEBASIC_H
#define SCENEBASIC_H

#include "util/util.h"

struct Point;

class Scene;
class SceneBasic;
template <typename MarkerType> class MarkedSceneBasic;
template <typename MarkerType> class MarkerContainer;
class SceneNode;
class SceneFace;
class SceneLabel;
class Marker;
class FieldInfo;
class ProblemBase;

const int MARKER_IDX_NOT_EXISTING = -1;

class AGROS_LIBRARY_API SceneBasic 
{

public:
    SceneBasic(Scene *scene);
    virtual ~SceneBasic() {}

    void setSelected(bool value = true) { m_isSelected = value; }
    inline bool isSelected() const { return m_isSelected; }

    void setHighlighted(bool value = true) { m_isHighlighted = value; }
    inline bool isHighlighted() const { return m_isHighlighted; }

    // virtual int showDialog(QWidget *parent, bool isNew = false) = 0;

    QVariant variant();

    // parent scene
    inline Scene *scene() { return m_scene; }

protected:
    Scene *m_scene;

private:
    bool m_isSelected;
    bool m_isHighlighted;
};

template <typename BasicType>
class AGROS_LIBRARY_API SceneBasicContainer
{
public:
    SceneBasicContainer() : m_data(QList<BasicType *>()) {}
    virtual ~SceneBasicContainer();

    /// items() should be removed step by step from the code.
    /// more methods operating with list data should be defined here
    QList<BasicType*> items() { return m_data; }

    virtual bool add(BasicType *item);
    virtual bool remove(BasicType *item);
    BasicType *at(int i);
    inline int length() { return m_data.length(); }
    inline int count() {return length(); }
    inline int isEmpty() { return m_data.isEmpty(); }
    void clear();

    /// selects or unselects all items
    void setSelected(bool value = true);

    /// highlights or unhighlights all items
    void setHighlighted(bool value = true);

protected:
    QList<BasicType *> m_data;

    QString containerName;
};

Q_DECLARE_METATYPE(SceneBasic *)

// *************************************************************************************************************************************

template <typename MarkerType>
class MarkedSceneBasic : public SceneBasic
{
public:
    MarkedSceneBasic(Scene *scene) : SceneBasic(scene) {}
    ~MarkedSceneBasic() {}

    /// gets marker that corresponds to the given field
    MarkerType* marker(QString field) const;
    MarkerType* marker(const FieldInfo *fieldInfo) const;

    /// adds marker. If there exists marker with the same field, is overwritten
    void addMarker(MarkerType* marker);

    /// true if has given marker
    bool hasMarker(const MarkerType* marker) const { return m_markers[marker->fieldInfo()] == marker; }
    bool hasMarker(const FieldInfo* fieldInfo) const { return m_markers.contains(fieldInfo); }

    /// returns markers length
    int markersCount();
    QMap<QString, QString> markersKeys() const;

    /// removes marker corresponding to this field from node
    void removeMarker(const FieldInfo* fieldInfo);
    void removeMarker(QString field);
    void removeMarker(MarkerType* marker);

    /// goes through own markers and if they are not yet in the list, adds them there
    void putMarkersToList(MarkerContainer<MarkerType>* list);

    void fieldsChange();

private:
    QMap<const FieldInfo*, MarkerType*> m_markers;    
};


template <typename MarkerType, typename MarkedSceneBasicType>
class MarkedSceneBasicContainer : public SceneBasicContainer<MarkedSceneBasicType>
{
public:
    MarkerContainer<MarkerType> allMarkers();
    void removeMarkerFromAll(MarkerType* marker);
    void addMarkerToAll(MarkerType* marker);

    /// removes markers corresponding to field from all members
    void removeFieldMarkers(const FieldInfo* field);

    /// add missing field markers
    void addMissingFieldMarkers(const FieldInfo* field);

    /// adds none markers for new fields and removes markers from fields that have been deleted
    void fieldsChange();

    /// Filters for elements that has given marker
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> haveMarker(MarkerType *marker);

    //TODO unfortunately, those had to be moved here from SceneBasicContainer
    //TODO if they returned SceneBasicContainer, One would have to cast to use methods of this class to return value...
    //TODO it might be possible to do it differently...
    /// Filters for selected
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> selected();

    /// Filters for highlighted
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> highlighted();
};

#endif // SCENEBASIC_H

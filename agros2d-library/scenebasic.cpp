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

#include "scenebasic.h"

#include "util/global.h"

#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "scene.h"
#include "scenemarker.h"

#include "solver/problem.h"

SceneBasic::SceneBasic(Scene *scene) : m_scene(scene), m_isHighlighted(false)
{
    setSelected(false);   
}

QVariant SceneBasic::variant()
{
    QVariant v;
    v.setValue(this);
    return v;
}

// *************************************************************************************************************************************

template <typename MarkerType>
MarkerType* MarkedSceneBasic<MarkerType>::marker(const FieldInfo* field) const
{
    assert(m_markers.contains(field));
    MarkerType* marker = m_markers[field];
    assert(marker);

    return marker;
}

template <typename MarkerType>
MarkerType* MarkedSceneBasic<MarkerType>::marker(QString fieldId) const
{
    return marker(m_scene->parentProblem()->fieldInfo(fieldId));
}


template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::addMarker(MarkerType* marker)
{
    m_markers[marker->fieldInfo()] = marker;
}

template <typename MarkerType>
int MarkedSceneBasic<MarkerType>::markersCount()
{
    int count = 0;

    foreach (MarkerType* marker, m_markers)
    {
        if (typeid(MarkerType) == typeid(SceneBoundary))
        {
            if (marker != (MarkerType*) m_scene->boundaries->getNone(marker->fieldInfo()))
                count++;
        }
        else if (typeid(MarkerType) == typeid(SceneMaterial))
        {
            if (marker != (MarkerType*) m_scene->materials->getNone(marker->fieldInfo()))
                count++;
        }
        else
            assert(0);

    }

    return count;
}

template <typename MarkerType>
QMap<QString, QString> MarkedSceneBasic<MarkerType>::markersKeys() const
{
    QMap<QString, QString> markers;
    foreach (MarkerType* marker, m_markers)
        markers[marker->fieldId()] = marker->name();

    return markers;
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::putMarkersToList(MarkerContainer<MarkerType>* list)
{
    foreach (MarkerType* marker, m_markers)
        if(!list->contains(marker))
            list->add(marker);
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::removeMarker(QString field)
{
    removeMarker(m_scene->parentProblem()->fieldInfo(field));
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::removeMarker(MarkerType* marker)
{
    foreach (MarkerType* item, m_markers)
    {
        if (item == marker)
        {
            if (typeid(MarkerType) == typeid(SceneBoundary))
                m_markers.insert(marker->fieldInfo(), (MarkerType*) m_scene->boundaries->getNone(marker->fieldInfo()));
            else if (typeid(MarkerType) == typeid(SceneMaterial))
                m_markers.insert(marker->fieldInfo(), (MarkerType*) m_scene->materials->getNone(marker->fieldInfo()));
            else
                assert(0);
        }
    }
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::removeMarker(const FieldInfo *fieldInfo)
{
    // replace marker with none marker
    m_markers.remove(fieldInfo);
}

template <typename MarkerType>
void MarkedSceneBasic<MarkerType>::fieldsChange()
{
    foreach (MarkerType* marker, m_markers)
    {
        if (!m_scene->parentProblem()->fieldInfos().contains(marker->fieldId()))
            removeMarker(marker);
    }

    foreach (FieldInfo* fieldInfo, m_scene->parentProblem()->fieldInfos())
    {
        if (!m_markers.contains(fieldInfo))
        {
            if (typeid(MarkerType) == typeid(SceneBoundary))
                m_markers[fieldInfo] = (MarkerType*) m_scene->boundaries->getNone(fieldInfo);
            else if (typeid(MarkerType) == typeid(SceneMaterial))
                m_markers[fieldInfo] = (MarkerType*) m_scene->materials->getNone(fieldInfo);
            else
                assert(0);
        }
    }
}


template class MarkedSceneBasic<SceneBoundary>;
template class MarkedSceneBasic<SceneMaterial>;

// *************************************************************************************************************************************

template <typename BasicType>
SceneBasicContainer<BasicType>::~SceneBasicContainer()
{
    // clear();
}

template <typename BasicType>
bool SceneBasicContainer<BasicType>::add(BasicType *item)
{
    //TODO add check
    m_data.append(item);

    return true;
}

template <typename BasicType>
bool SceneBasicContainer<BasicType>::remove(BasicType *item)
{
    return m_data.removeOne(item);
}

template <typename BasicType>
BasicType *SceneBasicContainer<BasicType>::at(int i)
{
    return m_data[i];
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::clear()
{
    foreach (BasicType* item, m_data)
        delete item;

    m_data.clear();
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::setSelected(bool value)
{
    foreach (BasicType* item, m_data)
        item->setSelected(value);
}

template <typename BasicType>
void SceneBasicContainer<BasicType>::setHighlighted(bool value)
{
    foreach (BasicType* item, m_data)
        item->setHighlighted(value);
}

template class SceneBasicContainer<SceneNode>;
template class SceneBasicContainer<SceneFace>;
template class SceneBasicContainer<SceneLabel>;

template <typename MarkerType, typename MarkedSceneBasicType>
MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::selected()
{
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> list;
    foreach (MarkedSceneBasicType* item, this->m_data)
    {
        if (item->isSelected())
            list.m_data.push_back(item);
    }

    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::highlighted()
{
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> list;
    foreach (MarkedSceneBasicType* item, this->m_data)
    {
        if (item->isHighlighted())
            list.m_data.push_back(item);
    }

    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::haveMarker(MarkerType *marker)
{
    MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType> list;
    foreach (MarkedSceneBasicType* item, this->m_data)
    {
        if (item->hasMarker(marker))
            list.m_data.push_back(item);
    }

    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::removeFieldMarkers(const FieldInfo *fieldInfo)
{
    foreach(MarkedSceneBasicType* item, this->m_data)
        item->removeMarker(fieldInfo);
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::addMissingFieldMarkers(const FieldInfo *fieldInfo)
{
    foreach(MarkedSceneBasicType* item, this->m_data)
    {
        if (!item->hasMarker(fieldInfo))
        {
            if (typeid(MarkerType) == typeid(SceneBoundary))
                item->addMarker((MarkerType*) item->scene()->boundaries->getNone(fieldInfo));
            else if (typeid(MarkerType) == typeid(SceneMaterial))
                item->addMarker((MarkerType*) item->scene()->materials->getNone(fieldInfo));
            else
                assert(0);
        }
    }
}

template <typename MarkerType, typename MarkedSceneBasicType>
MarkerContainer<MarkerType> MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::allMarkers()
{
    MarkerContainer<MarkerType> list;
    foreach(MarkedSceneBasicType* item, this->m_data)
    {
        item->putMarkersToList(&list);
    }
    return list;
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::removeMarkerFromAll(MarkerType* marker)
{
    foreach(MarkedSceneBasicType* item, this->m_data)
    {
        item->removeMarker(marker);
    }
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::fieldsChange()
{
    foreach(MarkedSceneBasicType* item, this->m_data)
    {
        item->fieldsChange();
    }
}

template <typename MarkerType, typename MarkedSceneBasicType>
void MarkedSceneBasicContainer<MarkerType, MarkedSceneBasicType>::addMarkerToAll(MarkerType* marker)
{
    foreach(MarkedSceneBasicType* item, this->m_data)
    {
        item->addMarker(marker);
    }
}

template class MarkedSceneBasicContainer<SceneBoundary, SceneFace>;
template class MarkedSceneBasicContainer<SceneMaterial, SceneLabel>;


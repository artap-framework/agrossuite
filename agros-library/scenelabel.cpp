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

#include "scenelabel.h"

#include <mesh/meshgenerator_triangle.h>

#include "util/util.h"
#include "util/loops.h"
#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenemarker.h"
#include "mesh/meshgenerator.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

SceneLabel::SceneLabel(Scene *scene, const Point &point, double area)
    : MarkedSceneBasic<SceneMaterial>(scene),
      m_point(scene->parentProblem(), point),
      m_area(area)
{
    foreach (FieldInfo* field, m_scene->parentProblem()->fieldInfos())
    {
        this->addMarker(m_scene->materials->getNone(field));
    }
}

SceneLabel::SceneLabel(Scene *scene, const PointValue &pointValue, double area)
    : MarkedSceneBasic<SceneMaterial>(scene), m_point(pointValue), m_area(area)
{
    foreach (FieldInfo* field, m_scene->parentProblem()->fieldInfos())
    {
        this->addMarker(m_scene->materials->getNone(field));
    }
}

void SceneLabel::setPointValue(const PointValue &point)
{    
    m_point = point;
}

double SceneLabel::distance(const Point &point) const
{
    return (this->point() - point).magnitude();
}

bool SceneLabel::isHole()
{
    foreach (FieldInfo* field, m_scene->parentProblem()->fieldInfos())
        if(hasMarker(field) && !marker(field)->isNone())
            return false;

    return true;
}

SceneLabel *SceneLabel::findLabelAtPoint(Scene *scene, const Point &point)
{
    try
    {
        QMapIterator<SceneLabel*, QList<MeshGeneratorTriangleFast::Triangle> > i(scene->fastMeshInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();

            foreach (MeshGeneratorTriangleFast::Triangle triangle, i.value())
            {
                bool b1 = (point.x - triangle.b.x) * (triangle.a.y - triangle.b.y) - (triangle.a.x - triangle.b.x) * (point.y - triangle.b.y) < 0.0;
                bool b2 = (point.x - triangle.c.x) * (triangle.b.y - triangle.c.y) - (triangle.b.x - triangle.c.x) * (point.y - triangle.c.y) < 0.0;
                bool b3 = (point.x - triangle.a.x) * (triangle.c.y - triangle.a.y) - (triangle.c.x - triangle.a.x) * (point.y - triangle.a.y) < 0.0;

                if ((b1 == b2) && (b2 == b3))
                {
                    // in triangle
                    return i.key();
                }
            }
        }
    }
    catch (AgrosMeshException &ame)
    {
        // do nothing
    }

    return nullptr;
}

SceneLabel *SceneLabel::findClosestLabel(Scene *scene, const Point &point)
{
    // find the nearest label by position
    SceneLabel *labelClosest = findLabelAtPoint(scene, point);

    // find the nearest label by position
    if (!labelClosest)
    {
        double distance = numeric_limits<double>::max();
        foreach (SceneLabel *label, scene->labels->items())
        {
            double labelDistance = label->distance(point);
            if (labelDistance < distance)
            {
                distance = labelDistance;
                labelClosest = label;
            }
        }
    }

    return labelClosest;
}

void SceneLabel::addMarkersFromStrings(QMap<QString, QString> markers)
{
    foreach (QString fieldId, markers.keys())
    {
        if (m_scene->parentProblem()->hasField(fieldId))
        {
            SceneMaterial *material = m_scene->materials->filter(m_scene->parentProblem()->fieldInfo(fieldId)).get(markers[fieldId]);

            if (!material)
                material = m_scene->materials->getNone(m_scene->parentProblem()->fieldInfo(fieldId));

            // add marker
            addMarker(material);
        }
    }

}

//****************************************************************************************************

SceneLabel* SceneLabelContainer::get(SceneLabel *label) const
{
    foreach (SceneLabel *labelCheck, m_data)
    {
        if (labelCheck->point() == label->point())
        {
            return labelCheck;
        }
    }

    return NULL;
}

SceneLabel* SceneLabelContainer::get(const Point& point) const
{
    foreach (SceneLabel *labelCheck, m_data)
    {
        if (labelCheck->point() == point)
            return labelCheck;
    }

    return NULL;
}

RectPoint SceneLabelContainer::boundingBox() const
{
    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    foreach (SceneLabel *label, m_data)
    {
        min.x = qMin(min.x, label->point().x);
        max.x = qMax(max.x, label->point().x);
        min.y = qMin(min.y, label->point().y);
        max.y = qMax(max.y, label->point().y);
    }

    return RectPoint(min, max);
}

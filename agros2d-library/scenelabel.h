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

#ifndef SCENELABEL_H
#define SCENELABEL_H

#include "util/util.h"
#include "value.h"
#include "scenebasic.h"
#include "scenemarker.h"

class FieldInfo;

class AGROS_LIBRARY_API SceneLabel : public MarkedSceneBasic<SceneMaterial>
{
public:
    SceneLabel(Scene *scene, const Point &point, double area);
    SceneLabel(Scene *scene, const PointValue &pointValue, double area);

    inline virtual SceneMaterial* marker(const FieldInfo *fieldInfo) { return MarkedSceneBasic<SceneMaterial>::marker(fieldInfo); }

    inline Point point() const { return m_point.point(); }
    inline PointValue pointValue() const { return m_point; }
    inline PointValue &pointValue() { return m_point; }
    void setPointValue(const PointValue &point);

    inline double area() const { return m_area; }
    inline void setArea(double area) { m_area = area; }

    double distance(const Point &m_point) const;

    // returns true if markers for all fields are noneMarkers
    bool isHole();

    // int showDialog(QWidget *parent, bool isNew = false);

    static SceneLabel *findClosestLabel(Scene *scene, const Point &point);
    static SceneLabel *findLabelAtPoint(Scene *scene, const Point &point);

    void addMarkersFromStrings(QMap<QString, QString> markers);

private:
    PointValue m_point;

    double m_area;
};

Q_DECLARE_METATYPE(SceneLabel *)

class SceneLabelContainer : public MarkedSceneBasicContainer<SceneMaterial, SceneLabel>
{
public:
    /// if container contains object with the same coordinates as label, returns it. Otherwise returns NULL
    SceneLabel* get(SceneLabel* label) const;

    /// returns label with given coordinates or NULL
    SceneLabel* get(const Point& point) const;

    /// returns bounding box, assumes container not empty
    RectPoint boundingBox() const;
};

#endif // SCENELABEL_H

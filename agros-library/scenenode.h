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

#ifndef SCENENODE_H
#define SCENENODE_H

#include "util/util.h"
#include "value.h"
#include "scenebasic.h"

class QDomElement;

class AGROS_LIBRARY_API SceneNode : public SceneBasic
{
public:
    SceneNode(Scene *scene, const Point &point);
    SceneNode(Scene *scene, const PointValue &pointValue);

    inline Point point() const { return m_point.point(); }
    inline PointValue pointValue() const { return m_point; }
    inline PointValue &pointValue() { return m_point; }
    void setPointValue(const PointValue &point);

    // geometry editor
    bool isConnected() const;
    bool isEndNode() const;
    QList<SceneFace *> connectedEdges() const;
    int numberOfConnectedEdges() const;
    bool hasLyingEdges() const;
    QList<SceneFace *> lyingEdges() const;
    bool isOutsideArea() const;
    bool isError();
    double distance(const Point &point) const;

    static SceneNode *findClosestNode(Scene *scene, const Point &point);

private:
    PointValue m_point;
};

class AGROS_LIBRARY_API SceneNodeContainer : public SceneBasicContainer<SceneNode>
{
public:
    /// if container contains object with the same coordinates as node, returns it. Otherwise returns NULL
    SceneNode* get(SceneNode* node) const;

    /// returns node with given coordinates or NULL
    SceneNode* get(const Point& point) const;

    SceneNode* findClosest(const Point& point) const;

    virtual bool remove(SceneNode *item) override;

    /// returns bounding box, assumes container not empty
    RectPoint boundingBox() const;

    //TODO should be in SceneBasicContainer, but I would have to cast the result....
    SceneNodeContainer selected();
    SceneNodeContainer highlighted();
};

#endif // SCENENODE_H

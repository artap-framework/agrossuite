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

#include "scenenode.h"

#include "util/util.h"
#include "util/global.h"

#include "scene.h"
#include "sceneedge.h"
#include "scenemarker.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

SceneNode::SceneNode(Scene *scene, const Point &point) : SceneBasic(scene),
    m_point(scene->parentProblem(), point)
{
}

SceneNode::SceneNode(Scene *scene, const PointValue &pointValue) : SceneBasic(scene),
    m_point(pointValue)
{
}

void SceneNode::setPointValue(const PointValue &point)
{
    m_point = point;

    // refresh cache
    foreach (SceneFace *edge, connectedEdges())
        edge->computeCenterAndRadius();
}

double SceneNode::distance(const Point &point) const
{
    return (this->point() - point).magnitude();
}

SceneNode *SceneNode::findClosestNode(Scene *scene, const Point &point)
{
    SceneNode *nodeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneNode *node, scene->nodes->items())
    {
        double nodeDistance = node->distance(point);
        if (node->distance(point) < distance)
        {
            distance = nodeDistance;
            nodeClosest = node;
        }
    }

    return nodeClosest;
}

bool SceneNode::isConnected() const
{
    foreach (SceneFace *edge, m_scene->faces->items())
        if (edge->nodeStart() == this || edge->nodeEnd() == this)
            return true;

    return false;
}

bool SceneNode::isEndNode() const
{
    int connections = 0;
    foreach (SceneFace *edge, m_scene->faces->items())
        if (edge->nodeStart() == this || edge->nodeEnd() == this)
            connections++;

    return (connections == 1);
}

QList<SceneFace *> SceneNode::connectedEdges() const
{
    QList<SceneFace *> edges;
    edges.reserve(m_scene->faces->count());

    foreach (SceneFace *edge, m_scene->faces->items())
        if (edge->nodeStart() == this || edge->nodeEnd() == this)
            edges.append(edge);

    return edges;
}

int SceneNode::numberOfConnectedEdges() const
{
    return m_scene->numberOfConnectedNodeEdges().value(const_cast<SceneNode *>(this));
}

bool SceneNode::hasLyingEdges() const
{
    return (lyingEdges().length() > 0);
}

QList<SceneFace *> SceneNode::lyingEdges() const
{
    return m_scene->lyingEdgeNodes().keys(const_cast<SceneNode *>(this));
}

bool SceneNode::isOutsideArea() const
{
    return (m_scene->parentProblem()->config()->coordinateType() == CoordinateType_Axisymmetric) &&
            (this->point().x < - EPS_ZERO);
}

bool SceneNode::isError()
{
    return (isOutsideArea() || numberOfConnectedEdges() <= 1 || hasLyingEdges());
}

// *************************************************************************************************************************************

SceneNode* SceneNodeContainer::get(SceneNode *node) const
{
    foreach (SceneNode *nodeCheck, m_data)
    {
        if (nodeCheck->point() == node->point())
        {
            return nodeCheck;
        }
    }

    return NULL;
}

SceneNode* SceneNodeContainer::get(const Point &point) const
{
    foreach (SceneNode *nodeCheck, m_data)
    {
        if (nodeCheck->point() == point)
            return nodeCheck;
    }

    return NULL;
}

bool SceneNodeContainer::remove(SceneNode *item)
{
    // remove all edges connected to this node
    item->scene()->faces->removeConnectedToNode(item);

    return SceneBasicContainer<SceneNode>::remove(item);
}

RectPoint SceneNodeContainer::boundingBox() const
{
    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    foreach (SceneNode *node, m_data)
    {
        min.x = qMin(min.x, node->point().x);
        max.x = qMax(max.x, node->point().x);
        min.y = qMin(min.y, node->point().y);
        max.y = qMax(max.y, node->point().y);
    }

    return RectPoint(min, max);
}

SceneNodeContainer SceneNodeContainer::selected()
{
    SceneNodeContainer list;
    foreach (SceneNode* item, this->m_data)
    {
        if (item->isSelected())
            list.m_data.push_back(item);
    }

    return list;
}

SceneNodeContainer SceneNodeContainer::highlighted()
{
    SceneNodeContainer list;
    foreach (SceneNode* item, this->m_data)
    {
        if (item->isHighlighted())
            list.m_data.push_back(item);
    }

    return list;
}

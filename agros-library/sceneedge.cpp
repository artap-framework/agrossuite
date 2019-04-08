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

#include "sceneedge.h"

#include "util/util.h"
#include "util/global.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "scenemarker.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/field.h"

SceneFace::SceneFace(Scene *scene, SceneNode *nodeStart, SceneNode *nodeEnd, const Value &angle, int segments)
    : MarkedSceneBasic<SceneBoundary>(scene),
      m_nodeStart(nodeStart), m_nodeEnd(nodeEnd),
      m_angle(angle),
      m_segments(segments)
{
    foreach (FieldInfo* field, m_scene->parentProblem()->fieldInfos())
    {
        this->addMarker(m_scene->boundaries->getNone(field));
    }

    m_rightLabelIdx = MARKER_IDX_NOT_EXISTING;
    m_leftLabelIdx = MARKER_IDX_NOT_EXISTING;

    // cache center;
    computeCenterAndRadius();
}

void SceneFace::swapDirection()
{
    SceneNode *tmp = m_nodeStart;

    m_nodeStart = m_nodeEnd;
    m_nodeEnd = tmp;

    // cache center;
    computeCenterAndRadius();
}

bool SceneFace::isLyingOnNode(const SceneNode *node) const
{
    // start or end node
    if ((m_nodeStart == node) || (m_nodeEnd == node))
        return false;

    // general node
    return isLyingOnPoint(node->point());
}

bool SceneFace::isLyingOnPoint(const Point &point) const
{
    if (isStraight())
    {
        double dx = m_vectorCache.x;
        double dy = m_vectorCache.y;

        Point sp = m_nodeStart->point();

        double t = ((point.x - sp.x)*dx + (point.y - sp.y)*dy);

        if (t < 0.0)
            t = 0.0;
        else if (t > m_vectorCache.magnitudeSquared())
            t = 1.0;
        else
            t /= m_vectorCache.magnitudeSquared();

        Point p(sp.x + t*dx, sp.y + t*dy);

        return ((point - p).magnitudeSquared() < EPS_ZERO);
    }
    else
    {
        double dist = (point - center()).magnitudeSquared();

        // point and radius are similar
        if (dist < EPS_ZERO)
            return (distance(point) < EPS_ZERO);
        else
            return false;
    }
}

double SceneFace::distance(const Point &point) const
{
    if (isStraight())
    {
        double dx = m_nodeEnd->point().x - m_nodeStart->point().x;
        double dy = m_nodeEnd->point().y - m_nodeStart->point().y;

        double t = ((point.x - m_nodeStart->point().x)*dx + (point.y - m_nodeStart->point().y)*dy) / (dx*dx + dy*dy);

        if (t > 1.0) t = 1.0;
        if (t < 0.0) t = 0.0;

        Point p(m_nodeStart->point().x + t*dx,
                m_nodeStart->point().y + t*dy);

        return (point - p).magnitude();
    }
    else
    {
        Point c = center();
        double R = radius();
        double distance = (point - c).magnitude();

        // point and radius are similar
        if (distance < EPS_ZERO) return R;

        Point t = (point - c) / distance;
        double l = ((point - c) - t * R).magnitude();

        // double z = (t.angle() - (m_nodeStart->point() - c).angle()) / M_PI*180.0;
        Point p = (m_nodeStart->point() - c);
        double z = (fastatan2(t.y, t.x) - fastatan2(p.y, p.x)) / M_PI*180.0;
        if (z < 0) z = z + 360.0; // interval (0, 360)
        if ((z > 0) && (z < m_angle.number())) return l;

        double a = (point - m_nodeStart->point()).magnitude();
        double b = (point - m_nodeEnd->point()).magnitude();

        return qMin(a, b);
    }
}

bool SceneFace::isCrossed() const
{
    // TODO: copy of crossedEdges() !!!!
    foreach (SceneFace *edgeCheck, m_scene->faces->items())
    {
        if (edgeCheck != this)
        {
            QList<Point> intersects;

            // TODO: improve - add check of crossings of two arcs
            if (isStraight())
                intersects = intersection(m_nodeStart->point(), m_nodeEnd->point(),
                                          edgeCheck->center(), radius(), angle(),
                                          edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle());

            else
                intersects = intersection(edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle(),
                                          m_nodeStart->point(), m_nodeEnd->point(),
                                          center(), radius(), angle());

            if (intersects.count() > 0)
                return true;
        }
    }

    return false;
}

bool SceneFace::hasLyingNode() const
{
    return (lyingNodes().length() > 0);
}

QList<SceneNode *> SceneFace::lyingNodes() const
{
    return m_scene->lyingEdgeNodes().values(const_cast<SceneFace *>(this));
}

void SceneFace::setSegments(int segments)
{
    m_segments = segments;

    // minimum and maximum segments
    if (m_segments < 4)
        m_segments = 4;
    if (m_segments > 20)
        m_segments = 20;
}

double SceneFace::length() const
{
    if (isStraight())
        return (m_nodeEnd->point() - m_nodeStart->point()).magnitude();
    else
        return radius() * m_angle.number() / 180.0 * M_PI;
}

bool SceneFace::isOutsideArea() const
{
    return (m_nodeStart->isOutsideArea() || m_nodeEnd->isOutsideArea());
}

bool SceneFace::isError() const
{
    return (hasLyingNode() || isOutsideArea() || isCrossed());
}

void SceneFace::computeCenterAndRadius()
{
    if (!isStraight())
        m_centerCache = centerPoint(m_nodeStart->point(), m_nodeEnd->point(), m_angle.number());
    else
        m_centerCache = Point();

    m_radiusCache = (m_centerCache - m_nodeStart->point()).magnitude();

    m_vectorCache = m_nodeEnd->point() - m_nodeStart->point();
}

SceneFace *SceneFace::findClosestFace(Scene *scene, const Point &point)
{
    SceneFace *edgeClosest = NULL;

    double distance = numeric_limits<double>::max();
    foreach (SceneFace *edge, scene->faces->items())
    {
        double edgeDistance = edge->distance(point);
        if (edge->distance(point) < distance)
        {
            distance = edgeDistance;
            edgeClosest = edge;
        }
    }

    return edgeClosest;
}

void SceneFace::addMarkersFromStrings(QMap<QString, QString> markers)
{
    foreach (QString fieldId, markers.keys())
    {
        if (m_scene->parentProblem()->hasField(fieldId))
        {
            SceneBoundary *boundary = m_scene->boundaries->filter(m_scene->parentProblem()->fieldInfo(fieldId)).get(markers[fieldId]);

            if (!boundary)
                boundary = m_scene->boundaries->getNone(m_scene->parentProblem()->fieldInfo(fieldId));

            // add marker
            addMarker(boundary);
        }
    }
}

int SceneFace::innerLabelIdx(const FieldInfo *fieldInfo) const
{
    if ((m_leftLabelIdx == MARKER_IDX_NOT_EXISTING) && (m_rightLabelIdx == MARKER_IDX_NOT_EXISTING))
        throw AgrosGeometryException(QObject::tr("right/left label idx not initialized"));

    if ((m_leftLabelIdx == MARKER_IDX_NOT_EXISTING) || (m_scene->labels->at(m_leftLabelIdx)->marker(fieldInfo)->isNone()))
    {
        // on the left is either outside area or label not used for this field, use the right hand side
        if((m_rightLabelIdx == MARKER_IDX_NOT_EXISTING) || (m_scene->labels->at(m_rightLabelIdx)->marker(fieldInfo)->isNone()))
        {
            // also on the right
            return MARKER_IDX_NOT_EXISTING;
        }
        else
        {
            return m_rightLabelIdx;
        }
    }
    else
    {
        return m_leftLabelIdx;
    }
}

int SceneFace::innerLabelIdx() const
{
    int returnIdx = MARKER_IDX_NOT_EXISTING;
    foreach(FieldInfo* fieldInfo, m_scene->parentProblem()->fieldInfos())
    {
        int idx = innerLabelIdx(fieldInfo);
        if(returnIdx == MARKER_IDX_NOT_EXISTING)
        {
            returnIdx = idx;
        }
        else
        {
            if(returnIdx != idx)
            {
                // conflict, return nothing
                return MARKER_IDX_NOT_EXISTING;
            }
        }
    }

    return returnIdx;
}

void SceneFace::addNeighbouringLabel(int idx)
{
    int *first, *second;
    bool turn = false;

    // ensure that if dirrection of edge is swaped, we would switch left and right label
    if (m_nodeStart->point().x > m_nodeEnd->point().x)
        turn = true;
    if (m_nodeStart->point().x == m_nodeEnd->point().x)
        if (m_nodeStart->point().y > m_nodeEnd->point().y)
            turn = true;

    if (turn)
    {
        first = &m_leftLabelIdx;
        second = &m_rightLabelIdx;
    }
    else
    {
        first = &m_rightLabelIdx;
        second = &m_leftLabelIdx;
    }

    if (*first == MARKER_IDX_NOT_EXISTING)
    {
        *first = idx;
    }
    else if (*second == MARKER_IDX_NOT_EXISTING)
    {
        *second = idx;
    }
    else
    {
        throw AgrosGeometryException(QObject::tr("Edge cannot have three adjacent labels"));
    }
}

void SceneFace::unsetRightLeftLabelIdx()
{
    m_rightLabelIdx = MARKER_IDX_NOT_EXISTING;
    m_leftLabelIdx = MARKER_IDX_NOT_EXISTING;
}

SceneLabel *SceneFace::leftLabel() const
{
    return (m_leftLabelIdx == MARKER_IDX_NOT_EXISTING) ? NULL : m_scene->labels->at(m_leftLabelIdx);
}

SceneLabel *SceneFace::rightLabel() const
{
    return (m_rightLabelIdx == MARKER_IDX_NOT_EXISTING) ? NULL : m_scene->labels->at(m_rightLabelIdx);
}

//************************************************************************************************

void SceneFaceContainer::removeConnectedToNode(SceneNode *node)
{
    foreach (SceneFace *edge, m_data)
    {
        if ((edge->nodeStart() == node) || (edge->nodeEnd() == node))
        {
            assert(0);
            /*
            edge->scene()->undoStack()->push(new SceneFaceCommandRemove(edge->nodeStart()->pointValue(),
                                                                        edge->nodeEnd()->pointValue(),
                                                                        edge->markersKeys(),
                                                                        edge->angleValue(),
                                                                        edge->segments()));
            */
        }
    }

}

SceneFace* SceneFaceContainer::get(SceneFace* edge) const
{
    foreach (SceneFace *edgeCheck, m_data)
    {
        if ((((edgeCheck->nodeStart() == edge->nodeStart()) && (edgeCheck->nodeEnd() == edge->nodeEnd())) &&
             (fabs(edgeCheck->angle() - edge->angle()) < EPS_ZERO)) ||
                (((edgeCheck->nodeStart() == edge->nodeEnd()) && (edgeCheck->nodeEnd() == edge->nodeStart())) &&
                 (fabs(edgeCheck->angle() + edge->angle()) < EPS_ZERO)))
        {
            return edgeCheck;
        }
    }

    return NULL;
}

SceneFace* SceneFaceContainer::get(const Point &pointStart, const Point &pointEnd, double angle, int segments) const
{
    foreach (SceneFace *edgeCheck, m_data)
    {
        if (((edgeCheck->nodeStart()->point() == pointStart) && (edgeCheck->nodeEnd()->point() == pointEnd))
                && ((edgeCheck->angle() - angle) < EPS_ZERO) && (edgeCheck->segments() == segments))
            return edgeCheck;
    }

    return NULL;
}

SceneFace* SceneFaceContainer::get(const Point &pointStart, const Point &pointEnd) const
{
    foreach (SceneFace *edgeCheck, m_data)
    {
        if (((edgeCheck->nodeStart()->point() == pointStart) && (edgeCheck->nodeEnd()->point() == pointEnd)))
            return edgeCheck;
    }

    return NULL;
}

RectPoint SceneFaceContainer::boundingBox() const
{
    return SceneFaceContainer::boundingBox(m_data);
}

RectPoint SceneFaceContainer::boundingBox(QList<SceneFace *> edges)
{
    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    foreach (SceneFace *edge, edges)
    {
        // start and end node
        min.x = qMin(min.x, qMin(edge->nodeStart()->point().x, edge->nodeEnd()->point().x));
        max.x = qMax(max.x, qMax(edge->nodeStart()->point().x, edge->nodeEnd()->point().x));
        min.y = qMin(min.y, qMin(edge->nodeStart()->point().y, edge->nodeEnd()->point().y));
        max.y = qMax(max.y, qMax(edge->nodeStart()->point().y, edge->nodeEnd()->point().y));

        if (!edge->isStraight())
        {
            int segments = 4;
            double theta = deg2rad(edge->angle()) / double(segments);
            Point center = edge->center();
            double radius = edge->radius();

            double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                      center.x - edge->nodeStart()->point().x) - M_PI;

            for (int i = 1; i < segments; i++)
            {
                double arc = startAngle + i*theta;

                double x = center.x + radius * cos(arc);
                double y = center.y + radius * sin(arc);

                min.x = qMin(min.x, x);
                max.x = qMax(max.x, x);
                min.y = qMin(min.y, y);
                max.y = qMax(max.y, y);
            }
        }
    }

    return RectPoint(min, max);
}

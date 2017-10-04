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

#include "scene.h"

#include "util/constants.h"
#include "util/global.h"
#include "util/loops.h"
#include "util/dxf_filter.h"

#include "util/util.h"
#include "value.h"
#include "logview.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "solver/problem.h"
#include "solver/plugin_interface.h"

#include "scenemarker.h"

#include "solver/module.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

QString generateSvgGeometry(QList<SceneFace*> edges)
{
    RectPoint boundingBox = SceneFaceContainer::boundingBox(edges);

    double size = 180;
    double stroke_width = max(boundingBox.width(), boundingBox.height()) / size / 2.0;

    // svg
    QString str;
    str += QString("<svg width=\"%1px\" height=\"%2px\" viewBox=\"%3 %4 %5 %6\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n").
            arg(size).
            arg(size).
            arg(boundingBox.start.x).
            arg(0).
            arg(boundingBox.width()).
            arg(boundingBox.height());

    str += QString("<g stroke=\"black\" stroke-width=\"%1\" fill=\"none\">\n").arg(stroke_width);

    foreach (SceneFace *edge, edges)
    {
        if (edge->angle() > 0.0)
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            int segments = edge->angle() / 5.0;
            if (segments < 2) segments = 2;
            double theta = edge->angle() / double(segments - 1);

            for (int i = 0; i < segments-1; i++)
            {
                double arc1 = (startAngle + i*theta)/180.0*M_PI;
                double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                double x1 = radius * fastcos(arc1);
                double y1 = radius * fastsin(arc1);
                double x2 = radius * fastcos(arc2);
                double y2 = radius * fastsin(arc2);

                str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                        arg(center.x + x1).
                        arg(boundingBox.end.y - (center.y + y1)).
                        arg(center.x + x2).
                        arg(boundingBox.end.y - (center.y + y2));
            }
        }
        else
        {
            str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                    arg(edge->nodeStart()->point().x).
                    arg(boundingBox.end.y - edge->nodeStart()->point().y).
                    arg(edge->nodeEnd()->point().x).
                    arg(boundingBox.end.y - edge->nodeEnd()->point().y);
        }
    }
    str += "</g>\n";
    str += "</svg>\n";

    return str;
}

ostream& operator<<(ostream& output, FieldInfo& id)
{
    output << "FieldInfo " << id.fieldId().toStdString();
    return output;
}

// ************************************************************************************************************************

Scene::Scene(ProblemBase *problem) : m_problem(problem),
    m_loopsInfo(new LoopsInfo(this)),        
    boundaries(new SceneBoundaryContainer()), materials(new SceneMaterialContainer()),
    nodes(new SceneNodeContainer()), faces(new SceneFaceContainer()), labels(new SceneLabelContainer())
{    
}

Scene::~Scene()
{
    // delete loop info
    m_loopsInfo->clear();
    delete m_loopsInfo;

    clear();

    // markers (delete None markers)
    boundaries->clear();
    materials->clear();

    // delete m_undoStack;

    delete boundaries;
    delete materials;
    delete nodes;
    delete faces;
    delete labels;
}

void Scene::copy(const Scene *origin)
{
    clear();

    // geometry
    // nodes
    foreach (const SceneNode *originNode, origin->nodes->items())
    {
        addNode(new SceneNode(this, PointValue(Value(m_problem, originNode->pointValue().x().text()),
                                               Value(m_problem, originNode->pointValue().y().text()))));
    }

    // faces
    foreach (const SceneFace *originFace, origin->faces->items())
    {
        SceneNode *nodeStart = nodes->at(origin->nodes->items().indexOf(originFace->nodeStart()));
        SceneNode *nodeEnd = nodes->at(origin->nodes->items().indexOf(originFace->nodeEnd()));

        addFace(new SceneFace(this, nodeStart, nodeEnd,
                              Value(m_problem, originFace->angleValue().text()),
                              originFace->segments(),
                              originFace->isCurvilinear()));
    }

    // labels
    foreach (const SceneLabel *originLabel, origin->labels->items())
    {
        addLabel(new SceneLabel(this, PointValue(Value(m_problem, originLabel->pointValue().x().text()),
                                                 Value(m_problem, originLabel->pointValue().y().text())),
                                originLabel->area()));
    }

    // force recache constraint
    invalidate();
}

SceneNode *Scene::addNode(SceneNode *node)
{
    // check if node doesn't exists
    if (SceneNode* existing = nodes->get(node))
    {
        delete node;
        return existing;
    }

    nodes->add(node);    
    checkNodeConnect(node);

    return node;
}

SceneNode *Scene::getNode(const Point &point)
{
    return nodes->get(point);
}

SceneFace *Scene::addFace(SceneFace *face)
{
    // check if edge doesn't exists
    if (SceneFace* existing = faces->get(face)){
        delete face;
        return existing;
    }

    faces->add(face);

    return face;
}

SceneFace *Scene::getFace(const Point &pointStart, const Point &pointEnd, double angle, int segments, bool isCurvilinear)
{
    return faces->get(pointStart, pointEnd, angle, segments, isCurvilinear);
}

SceneFace *Scene::getFace(const Point &pointStart, const Point &pointEnd)
{
    SceneFace *edge = faces->get(pointStart, pointEnd);
    if(edge)
        return edge;
    return faces->get(pointEnd, pointStart);
}

SceneLabel *Scene::addLabel(SceneLabel *label)
{
    // check if label doesn't exists
    if (SceneLabel *existing = labels->get(label)){
        delete label;
        return existing;
    }

    labels->add(label);    

    return label;
}

SceneLabel *Scene::getLabel(const Point &point)
{
    return labels->get(point);
}

void Scene::addBoundary(SceneBoundary *boundary)
{
    boundaries->add(boundary);    
}

void Scene::removeBoundary(SceneBoundary *boundary)
{
    faces->removeMarkerFromAll(boundary);
    boundaries->remove(boundary);
    // delete boundary;
}

void Scene::setBoundary(SceneBoundary *boundary)
{
    faces->selected().addMarkerToAll(boundary);
    selectNone();
}

SceneBoundary *Scene::getBoundary(FieldInfo *field, const QString &name)
{
    return boundaries->filter(field).get(name);
}

void Scene::addMaterial(SceneMaterial *material)
{
    this->materials->add(material);    
}

SceneMaterial *Scene::getMaterial(FieldInfo *field, const QString &name)
{
    return materials->filter(field).get(name);
}

void Scene::removeMaterial(SceneMaterial *material)
{
    labels->removeMarkerFromAll(material);
    materials->remove(material);
}

void Scene::setMaterial(SceneMaterial *material)
{
    labels->selected().addMarkerToAll(material);
    selectNone();
}

void Scene::checkGeometryAssignement()
{
    if (faces->length() > 2)
    {
        // at least one boundary condition has to be assigned
        int count = 0;
        foreach (SceneFace *edge, faces->items())
            if (edge->markersCount() > 0)
                count++;

        if (count == 0)
            throw AgrosGeometryException(tr("At least one boundary condition has to be assigned"));
    }

    if (labels->length() < 1)
        throw AgrosGeometryException(tr("At least one label has to be created"));
    else
    {
        // at least one material has to be assigned
        int count = 0;
        foreach (SceneLabel *label, labels->items())
            if (label->markersCount() > 0)
                count++;

        if (count == 0)
            throw AgrosGeometryException(tr("At least one material has to be assigned"));
    }

    if (boundaries->length() < 2) // + none marker
        throw AgrosGeometryException(tr("Invalid number of boundary conditions (%1 < 1)").arg(boundaries->length() - 1));

    if (materials->length() < 2) // + none marker
        throw AgrosGeometryException(tr("Invalid number of materials (%1 < 1)").arg(materials->length() - 1));
}

void Scene::clear()
{
    // loops
    if (m_loopsInfo)
        m_loopsInfo->clear();

    // geometry
    nodes->clear();
    faces->clear();
    labels->clear();

    // markers
    boundaries->clear();
    materials->clear();

    // none edge
    boundaries->add(new SceneBoundaryNone(this));
    // none label
    materials->add(new SceneMaterialNone(this));

    // lying nodes
    m_lyingEdgeNodes.clear();
    m_numberOfConnectedNodeEdges.clear();
    m_crossings.clear();

    m_loopsInfo->processPolygonTriangles();

    invalidate();
}

RectPoint Scene::boundingBox() const
{
    if (nodes->isEmpty() && faces->isEmpty() && labels->isEmpty())
    {
        return RectPoint(Point(-0.5, -0.5), Point(0.5, 0.5));
    }
    else
    {
        // nodes bounding box
        RectPoint nodesBoundingBox = nodes->boundingBox();
        // edges bounding box
        RectPoint edgesBoundingBox = faces->boundingBox();
        // labels bounding box
        RectPoint labelsBoundingBox = labels->boundingBox();

        return RectPoint(Point(qMin(qMin(nodesBoundingBox.start.x, edgesBoundingBox.start.x), labelsBoundingBox.start.x),
                               qMin(qMin(nodesBoundingBox.start.y, edgesBoundingBox.start.y), labelsBoundingBox.start.y)),
                         Point(qMax(qMax(nodesBoundingBox.end.x, edgesBoundingBox.end.x), labelsBoundingBox.end.x),
                               qMax(qMax(nodesBoundingBox.end.y, edgesBoundingBox.end.y), labelsBoundingBox.end.y)));
    }
}

void Scene::selectNone()
{
    nodes->setSelected(false);
    faces->setSelected(false);
    labels->setSelected(false);
}

void Scene::selectAll(SceneGeometryMode sceneMode)
{
    selectNone();

    switch (sceneMode)
    {
    case SceneGeometryMode_OperateOnNodes:
        nodes->setSelected();
        break;
    case SceneGeometryMode_OperateOnEdges:
        faces->setSelected();
        break;
    case SceneGeometryMode_OperateOnLabels:
        labels->setSelected();
        break;
    }
}

int Scene::selectedCount()
{
    return nodes->selected().length() +
            faces->selected().length() +
            labels->selected().length();
}

void Scene::highlightNone()
{
    nodes->setHighlighted(false);
    faces->setHighlighted(false);
    labels->setHighlighted(false);
}

int Scene::highlightedCount()
{
    return nodes->highlighted().length() +
            faces->highlighted().length() +
            labels->highlighted().length();
}

void Scene::invalidate()
{
    foreach (SceneFace *edge, faces->items())
        edge->computeCenterAndRadius();

    findLyingEdgeNodes();
    findNumberOfConnectedNodeEdges();
    findCrossings();

    m_loopsInfo->processPolygonTriangles();
}

void Scene::fieldsChange()
{
    boundaries->doFieldsChanged(parentProblem());
    materials->doFieldsChanged(parentProblem());

    faces->fieldsChange();
    labels->fieldsChange();
}

void Scene::exportVTKGeometry(const QString &fileName)
{
    QList<Point> vtkNodes;
    foreach (SceneNode *node, nodes->items())
        vtkNodes.append(Point(node->point().x, node->point().y));

    QMap<int, int> vtkEdges;
    foreach (SceneFace *edge, faces->items())
    {
        if (edge->isStraight())
        {
            // straight line
            vtkEdges.insert(nodes->items().indexOf(edge->nodeStart()),
                            nodes->items().indexOf(edge->nodeEnd()));
        }
        else
        {
            // arc
            // add pseudo nodes
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                      center.x - edge->nodeStart()->point().x) - M_PI;

            int segments = edge->angle() / 15.0;
            double theta = deg2rad(edge->angle()) / double(segments);

            for (int j = 1; j <= segments; j++)
            {
                double arc = startAngle + j*theta;

                double x = radius * cos(arc);
                double y = radius * sin(arc);

                if (j < segments)
                    vtkNodes.append(Point(center.x + x, center.y + y));

                int startIndex = -1;
                int endIndex = -1;
                if (j == 1)
                {
                    startIndex = nodes->items().indexOf(edge->nodeStart());
                    endIndex = vtkNodes.count() - 1;
                }
                else if (j == segments)
                {
                    startIndex = vtkNodes.count() - 1;
                    endIndex = nodes->items().indexOf(edge->nodeEnd());
                }
                else
                {
                    startIndex = vtkNodes.count() - 2;
                    endIndex = vtkNodes.count() - 1;
                }

                vtkEdges.insert(startIndex, endIndex);
            }
        }
    }

    // save current locale
    // char *plocale = setlocale (LC_NUMERIC, "");
    // setlocale (LC_NUMERIC, "C");

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Agros::log()->printError(tr("VTK Geometry export"), tr("Could not create VTK file (%1)").arg(file.errorString()));
        return;
    }
    QTextStream out(&file);

    out << "# vtk DataFile Version 2.0\n";
    out << "\n";
    out << "ASCII\n";
    out << "DATASET POLYDATA\n";
    out << QString("POINTS %1 float\n").arg(vtkNodes.count());
    foreach (Point p, vtkNodes)
        out << QString("%1 %2 0.0\n").arg(p.x).arg(p.y);
    out << QString("LINES %1 %2\n").arg(vtkEdges.count()).arg(vtkEdges.count() * 3);
    for (int i = 0; i < vtkEdges.keys().size(); i++)
        out << QString("2 %1 %2\n").arg(vtkEdges.keys().at(i)).arg(vtkEdges.values().at(i));

    file.waitForBytesWritten(0);
    file.close();

    // set system locale
    // setlocale(LC_NUMERIC, plocale);
}

void Scene::exportToDxf(const QString &fileName)
{
    writeToDXF(this, fileName);
}

void Scene::importFromDxf(const QString &fileName)
{
    readFromDXF(this, fileName);
}

void Scene::checkNodeConnect(SceneNode *node)
{
    bool isConnected = false;
    foreach (SceneNode *nodeCheck, this->nodes->items())
    {
        if ((nodeCheck->distance(node->point()) < EPS_ZERO) && (nodeCheck != node))
        {
            isConnected = true;
            foreach (SceneFace *edgeCheck, node->connectedEdges())
            {
                SceneNode *nodeStart = NULL;
                SceneNode *nodeEnd = NULL;
                if (edgeCheck->nodeStart()->point() == node->point())
                {
                    nodeStart = nodeCheck;
                    nodeEnd = edgeCheck->nodeEnd();
                }
                else if (edgeCheck->nodeEnd()->point() == node->point())
                {
                    nodeStart = edgeCheck->nodeStart();
                    nodeEnd = nodeCheck;
                }

                assert(nodeStart);
                assert(nodeEnd);

                Value edgeAngle = edgeCheck->angleValue();
                faces->remove(edgeCheck);

                SceneFace *edge = new SceneFace(this, nodeStart, nodeEnd, Value(Agros::problem(), 0.0));
                edge->setAngleValue(edgeAngle);
            }
        }
    }

    if (isConnected)
    {
        nodes->remove(node);
    }
}

void Scene::checkTwoNodesSameCoordinates()
{
    for(int nodeIdx1 = 0; nodeIdx1 < nodes->length(); nodeIdx1++)
    {
        SceneNode* node1 = nodes->at(nodeIdx1);
        for(int nodeIdx2 = 0; nodeIdx2 < nodeIdx1; nodeIdx2++)
        {
            SceneNode* node2 = nodes->at(nodeIdx2);
            if(node1->point() == node2->point())
                throw AgrosGeometryException(QObject::tr("Point %1 and %2 has the same coordinates.").arg(nodeIdx1).arg(nodeIdx2));
        }
    }
}

void Scene::checkGeometryResult()
{
    checkTwoNodesSameCoordinates();

    if (parentProblem()->config()->coordinateType() == CoordinateType_Axisymmetric)
    {
        // check for nodes with r < 0
        QSet<int> nodes;
        foreach (SceneNode *node, this->nodes->items())
        {
            if (node->point().x < - EPS_ZERO)
                nodes.insert(this->nodes->items().indexOf(node));
        }

        if (nodes.count() > 0)
        {
            QString indices;
            foreach (int index, nodes)
                indices += QString::number(index) + ", ";
            indices = indices.left(indices.length() - 2);

            nodes.clear();
            throw AgrosGeometryException(tr("There are nodes '%1' with negative radial component.").arg(indices));
        }
        nodes.clear();
    }

    foreach (SceneNode *node, this->nodes->items())
    {
        if (!node->isConnected())
        {
            throw AgrosGeometryException(tr("There are nodes which are not connected to any edge (red highlighted). All nodes should be connected."));
        }

        if (node->isEndNode())
        {
            throw AgrosGeometryException(tr("There are nodes which are connected to one edge only (red highlighted). This is not allowed in Agros."));
        }

        if (node->hasLyingEdges())
        {
            throw AgrosGeometryException(tr("There are nodes which lie on the edge but they are not connected to the edge. Remove these nodes first."));
        }
    }

    if (!m_crossings.isEmpty())
    {
        throw AgrosGeometryException(tr("There are crossings in the geometry (red highlighted). Remove the crossings first."));
    }
}

void Scene::findLyingEdgeNodes()
{
    m_lyingEdgeNodes.clear();

    foreach (SceneFace *edge, faces->items())
    {
        foreach (SceneNode *node, nodes->items())
        {
            if (edge->isLyingOnNode(node))
            {
                if (edge->isLyingOnNode(node))
                    m_lyingEdgeNodes.insert(edge, node);
            }
        }
    }
}

void Scene::findNumberOfConnectedNodeEdges()
{
    m_numberOfConnectedNodeEdges.clear();

    foreach (SceneNode *node, nodes->items())
    {
        int connections = 0;
        foreach (SceneFace *edge, faces->items())
        {
            if (edge->nodeStart() == node || edge->nodeEnd() == node)
                connections++;
        }
        m_numberOfConnectedNodeEdges.insert(node, connections);
    }
}

void Scene::findCrossings()
{
    m_crossings.clear();

    for (int i = 0; i < faces->count(); i++)
    {
        SceneFace *edge = faces->at(i);

        for (int j = i + 1; j < faces->count(); j++)
        {
            SceneFace *edgeCheck = faces->at(j);

            QList<Point> intersects;

            // TODO: improve - add check of crossings of two arcs
            if (edge->isStraight())
                intersects = intersection(edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                          edgeCheck->center(), edge->radius(), edge->angle(),
                                          edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle());

            else
                intersects = intersection(edgeCheck->nodeStart()->point(), edgeCheck->nodeEnd()->point(),
                                          edgeCheck->center(), edgeCheck->radius(), edgeCheck->angle(),
                                          edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                          edge->center(), edge->radius(), edge->angle());

            if (intersects.count() > 0)
            {
                if (!m_crossings.contains(edgeCheck))
                    m_crossings.append(edgeCheck);
                if (!m_crossings.contains((edge)))
                    m_crossings.append(edge);
            }
        }
    }
}

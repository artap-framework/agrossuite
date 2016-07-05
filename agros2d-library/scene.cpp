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

#include "util.h"
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

/*
NewMarkerAction::NewMarkerAction(QIcon icon, QObject* parent, QString field) :
    QAction(icon, Module::availableModules()[field], parent),
    field(field)
{
    connect(this, SIGNAL(triggered()), this, SLOT(doTriggered()));
}

void NewMarkerAction::doTriggered()
{
    emit triggered(field);
}
*/

// ************************************************************************************************************************

Scene::Scene(ProblemBase *problem) : m_problem(problem), m_stopInvalidating(false),
    m_loopsInfo(new LoopsInfo(this)),
    // m_undoStack(new QUndoStack(this)),
    boundaries(new SceneBoundaryContainer()), materials(new SceneMaterialContainer()),
    nodes(new SceneNodeContainer()), faces(new SceneFaceContainer()), labels(new SceneLabelContainer())

{
    createActions();

    connect(this, SIGNAL(invalidated()), this, SLOT(cacheGeometryConstraints()));
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

    // clear actions
    // foreach (QAction *action, actNewBoundaries.values())
    //     delete action;
    // actNewBoundaries.clear();

    // foreach (QAction *action, actNewMaterials.values())
    //     delete action;
    // actNewMaterials.clear();
}

void Scene::copy(const Scene *origin)
{
    clear();

    stopInvalidating(true);
    blockSignals(true);

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

    stopInvalidating(false);
    blockSignals(false);

    // force recache constraint
    cacheGeometryConstraints();

    emit invalidated();
}

void Scene::createActions()
{
    /*
    // scene - add items
    actNewNode = new QAction(icon("scene-node"), tr("New &node..."), this);
    actNewNode->setShortcut(tr("Alt+N"));
    connect(actNewNode, SIGNAL(triggered()), this, SLOT(doNewNode()));

    actNewEdge = new QAction(icon("scene-edge"), tr("New &edge..."), this);
    actNewEdge->setShortcut(tr("Alt+E"));
    connect(actNewEdge, SIGNAL(triggered()), this, SLOT(doNewEdge()));

    actNewLabel = new QAction(icon("scene-label"), tr("New &label..."), this);
    actNewLabel->setShortcut(tr("Alt+L"));
    connect(actNewLabel, SIGNAL(triggered()), this, SLOT(doNewLabel()));

    actDeleteSelected = new QAction(icon("edit-delete"), tr("Delete selected objects"), this);
    connect(actDeleteSelected, SIGNAL(triggered()), this, SLOT(doDeleteSelected()));

    actNewBoundary = new QAction(icon("scene-edgemarker"), tr("New &boundary condition..."), this);
    actNewBoundary->setShortcut(tr("Alt+B"));
    connect(actNewBoundary, SIGNAL(triggered()), this, SLOT(doNewBoundary()));

    // clear actions
    foreach (QAction *action, actNewBoundaries.values())
        delete action;
    actNewBoundaries.clear();

    QMapIterator<QString, QString> iEdge(Module::availableModules());
    while (iEdge.hasNext())
    {
        iEdge.next();

        NewMarkerAction* action = new NewMarkerAction(icon("scene-edgemarker"), this, iEdge.key());
        connect(action, SIGNAL(triggered(QString)), this, SLOT(doNewBoundary(QString)));
        actNewBoundaries[iEdge.key()] = action;
    }

    actNewMaterial = new QAction(icon("scene-labelmarker"), tr("New &material..."), this);
    actNewMaterial->setShortcut(tr("Alt+M"));
    connect(actNewMaterial, SIGNAL(triggered()), this, SLOT(doNewMaterial()));

    // clear actions
    foreach (QAction *action, actNewMaterials.values())
        delete action;
    actNewMaterials.clear();

    QMapIterator<QString, QString> iLabel(Module::availableModules());
    while (iLabel.hasNext())
    {
        iLabel.next();

        NewMarkerAction *action = new NewMarkerAction(icon("scene-labelmarker"), this, iLabel.key());
        connect(action, SIGNAL(triggered(QString)), this, SLOT(doNewMaterial(QString)));
        actNewMaterials[iLabel.key()] = action;
    }

    actTransform = new QAction(icon("scene-transform"), tr("&Transform"), this);
    */
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
    if (!m_stopInvalidating)
        emit invalidated();

    checkNodeConnect(node);

    return node;
}

SceneNode *Scene::getNode(const Point &point)
{
    return nodes->get(point);
}

SceneFace *Scene::addFace(SceneFace *edge)
{
    // check if edge doesn't exists
    if (SceneFace* existing = faces->get(edge)){
        delete edge;
        return existing;
    }

    faces->add(edge);
    if (!m_stopInvalidating)
        emit invalidated();

    return edge;
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
    if(SceneLabel* existing = labels->get(label)){
        delete label;
        return existing;
    }

    labels->add(label);
    if (!m_stopInvalidating)
        emit invalidated();

    return label;
}

SceneLabel *Scene::getLabel(const Point &point)
{
    return labels->get(point);
}

void Scene::addBoundary(SceneBoundary *boundary)
{
    boundaries->add(boundary);
    if (!m_stopInvalidating)
        emit invalidated();
}

void Scene::removeBoundary(SceneBoundary *boundary)
{
    //TODO instead of setting NoneBoundary we now remove... rething
    faces->removeMarkerFromAll(boundary);
    boundaries->remove(boundary);
    // delete boundary;

    if (!m_stopInvalidating)
        emit invalidated();
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
    if (!m_stopInvalidating)
        emit invalidated();
}


SceneMaterial *Scene::getMaterial(FieldInfo *field, const QString &name)
{
    return materials->filter(field).get(name);
}

void Scene::removeMaterial(SceneMaterial *material)
{
    labels->removeMarkerFromAll(material);
    materials->remove(material);

    // delete material;

    if (!m_stopInvalidating)
        emit invalidated();
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
    blockSignals(true);
    stopInvalidating(true);

    // m_undoStack->clear();

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

    stopInvalidating(false);
    blockSignals(false);

    emit cleared();
    emit invalidated();
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

/*
void Scene::deleteSelected()
{
    m_undoStack->beginMacro(tr("Delete selected"));

    // nodes
    QList<PointValue> selectedNodePoints;
    foreach (SceneNode *node, nodes->selected().items())
        selectedNodePoints.append(node->pointValue());
    if (!selectedNodePoints.isEmpty())
        undoStack()->push(new SceneNodeCommandRemoveMulti(selectedNodePoints));

    // edges
    QList<PointValue> selectedEdgePointsStart;
    QList<PointValue> selectedEdgePointsEnd;
    QList<Value> selectedEdgeAngles;
    QList<int> selectedEdgeSegments;
    QList<bool> selectedEdgeIsCurvilinear;
    QList<QMap<QString, QString> > selectedEdgeMarkers;
    foreach (SceneFace *edge, faces->selected().items())
    {
        selectedEdgePointsStart.append(edge->nodeStart()->pointValue());
        selectedEdgePointsEnd.append(edge->nodeEnd()->pointValue());
        selectedEdgeAngles.append(edge->angleValue());
        selectedEdgeSegments.append(edge->segments());
        selectedEdgeIsCurvilinear.append(edge->isCurvilinear());
        selectedEdgeMarkers.append(edge->markersKeys());
    }
    if (!selectedEdgePointsStart.isEmpty())
        undoStack()->push(new SceneEdgeCommandRemoveMulti(selectedEdgePointsStart, selectedEdgePointsEnd,
                                                          selectedEdgeMarkers, selectedEdgeAngles, selectedEdgeSegments, selectedEdgeIsCurvilinear));

    // labels
    QList<PointValue> selectedLabelPointsStart;
    QList<double> selectedLabelAreas;
    QList<QMap<QString, QString> > selectedLabelMarkers;
    foreach (SceneLabel *label, labels->selected().items())
    {
        selectedLabelPointsStart.append(label->pointValue());
        selectedLabelAreas.append(label->area());
        selectedLabelMarkers.append(label->markersKeys());
    }
    if (!selectedLabelPointsStart.isEmpty())
        undoStack()->push(new SceneLabelCommandRemoveMulti(selectedLabelPointsStart, selectedLabelMarkers, selectedLabelAreas));

    m_undoStack->endMacro();

    emit invalidated();
}
*/

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

Point Scene::calculateNewPoint(SceneTransformMode mode, Point originalPoint, Point transformationPoint, double angle, double scaleFactor)
{
    Point newPoint;

    if (mode == SceneTransformMode_Translate)
    {
        newPoint = originalPoint + transformationPoint;
    }
    else if (mode == SceneTransformMode_Rotate)
    {
        double distanceNode = (originalPoint - transformationPoint).magnitude();
        double angleNode = (originalPoint - transformationPoint).angle()/M_PI*180;

        newPoint = transformationPoint + Point(distanceNode * cos((angleNode + angle)/180.0*M_PI), distanceNode * sin((angleNode + angle)/180.0*M_PI));
    }
    else if (mode == SceneTransformMode_Scale)
    {
        newPoint = transformationPoint + (originalPoint - transformationPoint) * scaleFactor;
    }

    return newPoint;
}

bool Scene::moveSelectedNodes(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy)
{
    // select endpoints
    foreach (SceneFace *edge, faces->items())
    {
        if (edge->isSelected())
        {
            edge->nodeStart()->setSelected(true);
            edge->nodeEnd()->setSelected(true);
        }
    }

    QList<PointValue> points;
    QList<PointValue> newPoints;
    QList<PointValue> pointsToSelect;

    foreach (SceneNode *node, nodes->selected().items())
    {
        PointValue newPoint = PointValue(m_problem, calculateNewPoint(mode, node->point(), point, angle, scaleFactor));

        SceneNode *obstructNode = getNode(newPoint.point());
        if (obstructNode && !obstructNode->isSelected())
        {
            Agros2D::log()->printWarning(tr("Geometry"), tr("Cannot perform transformation, existing point would be overwritten"));
            return false;
        }

        points.push_back(node->pointValue());

        // when copying, add only those points, that did not exist
        // when moving, add all, because if poit on place where adding exist, it will be moved away (otherwise it would be obstruct node and function would not reach this point)
        if(copy)
        {
            if (obstructNode)
                pointsToSelect.push_back(newPoint);
            else
                newPoints.push_back(newPoint);
        }
        else
        {
            newPoints.push_back(newPoint);
        }
    }

    if (copy)
    {
        // m_undoStack->push(new SceneNodeCommandAddMulti(newPoints));

        nodes->setSelected(false);

        // select new
        foreach(PointValue point, newPoints)
            getNode(point.point())->setSelected(true);

        foreach(PointValue point, pointsToSelect)
            getNode(point.point())->setSelected(true);
    }
    else
    {
        // m_undoStack->push(new SceneNodeCommandMoveMulti(points, newPoints));
    }

    return true;
}

bool Scene::moveSelectedEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy, bool withMarkers)
{
    QList<SceneFace *> selectedEdges;

    foreach (SceneFace *edge, faces->selected().items())
    {
        selectedEdges.append(edge);
    }

    if(selectedEdges.isEmpty())
        return true;

    nodes->setSelected(false);

    if(!copy)
        return true;

    QList<QPair<Point, Point> > newEdgeEndPoints;
    QList<PointValue> edgeStartPointsToAdd;
    QList<PointValue> edgeEndPointsToAdd;
    QList<Value> edgeAnglesToAdd;
    QList<int> edgeSegmentsToAdd;
    QList<bool> edgeIsCurvilinearToAdd;
    QList<QMap<QString, QString> > edgeMarkersToAdd;

    foreach (SceneFace *edge, selectedEdges)
    {
        PointValue newPointStart = PointValue(m_problem, calculateNewPoint(mode, edge->nodeStart()->point(), point, angle, scaleFactor));
        PointValue newPointEnd = PointValue(m_problem, calculateNewPoint(mode, edge->nodeEnd()->point(), point, angle, scaleFactor));

        // add new edge
        SceneNode *newNodeStart = getNode(newPointStart.point());
        SceneNode *newNodeEnd = getNode(newPointEnd.point());

        assert(newNodeStart && newNodeEnd);

        SceneFace *obstructEdge = getFace(newPointStart.point(), newPointEnd.point());
        if (obstructEdge && !obstructEdge->isSelected())
        {
            Agros2D::log()->printWarning(tr("Geometry"), tr("Cannot perform transformation, existing edge would be overwritten"));
            return false;
        }

        if(! obstructEdge)
        {
            edgeStartPointsToAdd.push_back(newPointStart);
            edgeEndPointsToAdd.push_back(newPointEnd);
            edgeAnglesToAdd.push_back(edge->angleValue());
            edgeSegmentsToAdd.push_back(edge->segments());
            edgeIsCurvilinearToAdd.push_back(edge->isCurvilinear());

            if(withMarkers)
                edgeMarkersToAdd.append(edge->markersKeys());
        }

        newEdgeEndPoints.push_back(QPair<Point, Point>(newPointStart.point(), newPointEnd.point()));
    }

    faces->setSelected(false);

    // m_undoStack->push(new SceneEdgeCommandAddMulti(edgeStartPointsToAdd, edgeEndPointsToAdd,
    //                                                edgeMarkersToAdd, edgeAnglesToAdd, edgeSegmentsToAdd, edgeIsCurvilinearToAdd));

    for(int i = 0; i < newEdgeEndPoints.size(); i++)
    {
        SceneFace* sceneEdge = getFace(newEdgeEndPoints[i].first, newEdgeEndPoints[i].second);
        if (sceneEdge)
            sceneEdge->setSelected(true);
    }

    return true;
}

bool Scene::moveSelectedLabels(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy, bool withMarkers)
{
    QList<PointValue> points;
    QList<PointValue> newPoints;
    QList<PointValue> pointsToSelect;

    QList<double> newAreas;
    QList<QMap<QString, QString> > newMarkers;

    foreach (SceneLabel *label, labels->selected().items())
    {
        PointValue newPoint = PointValue(m_problem, calculateNewPoint(mode, label->point(), point, angle, scaleFactor));

        SceneLabel *obstructLabel = getLabel(newPoint.point());
        if (obstructLabel && !obstructLabel->isSelected())
        {
            Agros2D::log()->printWarning(tr("Geometry"), tr("Cannot perform transformation, existing label would be overwritten"));
            return false;
        }

        points.push_back(label->pointValue());

        // when copying, add only those points, that did not exist
        // when moving, add all, because if poit on place where adding exist, it will be moved away (otherwise it would be obstruct node and function would not reach this point)
        if (copy)
        {
            if(obstructLabel)
            {
                pointsToSelect.push_back(newPoint);
            }
            else
            {
                newPoints.push_back(newPoint);
                newAreas.push_back(label->area());
                if(withMarkers)
                    newMarkers.append(label->markersKeys());

            }
        }
        else
        {
            newPoints.push_back(newPoint);
            newAreas.push_back(label->area());
        }
    }

    if (copy)
    {
        // m_undoStack->push(new SceneLabelCommandAddMulti(newPoints, newMarkers, newAreas));

        labels->setSelected(false);

        // select new
        foreach(PointValue point, newPoints)
            getLabel(point.point())->setSelected(true);

        foreach(PointValue point, pointsToSelect)
            getLabel(point.point())->setSelected(true);
    }
    else
    {
        // m_undoStack->push(new SceneLabelCommandMoveMulti(points, newPoints));
    }

    return true;
}

void Scene::transform(QString name, SceneTransformMode mode, const Point &point, double angle, double scaleFactor, bool copy, bool withMarkers)
{
    // m_undoStack->beginMacro(name);

    bool okNodes, okEdges = true;
    okNodes = moveSelectedNodes(mode, point, angle, scaleFactor, copy);
    if(okNodes)
        okEdges = moveSelectedEdges(mode, point, angle, scaleFactor, copy, withMarkers);
    moveSelectedLabels(mode, point, angle, scaleFactor, copy, withMarkers);

    // m_undoStack->endMacro();

    if(!okNodes || !okEdges)
        nodes->setSelected(false);

    emit invalidated();
}

void Scene::transformTranslate(const Point &point, bool copy, bool withMarkers)
{
    transform(tr("Translation"), SceneTransformMode_Translate, point, 0.0, 0.0, copy, withMarkers);
}

void Scene::transformRotate(const Point &point, double angle, bool copy, bool withMarkers)
{
    transform(tr("Rotation"), SceneTransformMode_Rotate, point, angle, 0.0, copy, withMarkers);
}

void Scene::transformScale(const Point &point, double scaleFactor, bool copy, bool withMarkers)
{
    transform(tr("Scale"), SceneTransformMode_Scale, point, 0.0, scaleFactor, copy, withMarkers);
}

void Scene::cacheGeometryConstraints()
{
    // actNewEdge->setEnabled((nodes->length() >= 2) && (boundaries->length() >= 1));
    // actNewLabel->setEnabled(materials->length() >= 1);

    foreach (SceneFace *edge, faces->items())
        edge->computeCenterAndRadius();

    findLyingEdgeNodes();
    findNumberOfConnectedNodeEdges();
    findCrossings();
}

/*
void Scene::doNewNode(const Point &point)
{
    SceneNode *node = new SceneNode(this, PointValue(m_problem, point));
    if (node->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneNode *nodeAdded = addNode(node);
        if (nodeAdded == node) m_undoStack->push(new SceneNodeCommandAdd(node->pointValue()));
    }
    else
        delete node;
}

void Scene::doNewEdge()
{
    SceneFace *edge = new SceneFace(this, nodes->at(0), nodes->at(1), Value(m_problem, 0.0));
    if (edge->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneFace *edgeAdded = addFace(edge);
        if (edgeAdded == edge)
            m_undoStack->push(edge->getAddCommand());
    }
    else
        delete edge;
}

void Scene::doNewLabel(const Point &point)
{
    SceneLabel *label = new SceneLabel(this, PointValue(m_problem, point), 0.0);
    if (label->showDialog(QApplication::activeWindow(), true) == QDialog::Accepted)
    {
        SceneLabel *labelAdded = addLabel(label);

        if (labelAdded == label)
            m_undoStack->push(label->getAddCommand());
    }
    else
        delete label;
}

void Scene::doDeleteSelected()
{
    deleteSelected();
}

void Scene::doNewBoundary()
{
    doNewBoundary(parentProblem()->fieldInfos().begin().key());
}

void Scene::doNewBoundary(QString field)
{
    SceneBoundary *boundary = new SceneBoundary(this,
                                                parentProblem()->fieldInfo(field),
                                                tr("new boundary"),
                                                parentProblem()->fieldInfo(field)->boundaryTypeDefault().id());

    if (boundary->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addBoundary(boundary);
    else
        delete boundary;
}

void Scene::doNewMaterial()
{
    doNewMaterial(parentProblem()->fieldInfos().begin().key());
}

void Scene::doNewMaterial(QString field)
{
    SceneMaterial *material = new SceneMaterial(this,
                                                parentProblem()->fieldInfo(field),
                                                tr("new material"));

    if (material->showDialog(QApplication::activeWindow()) == QDialog::Accepted)
        addMaterial(material);
    else
        delete material;
}

void Scene::addBoundaryAndMaterialMenuItems(QMenu* menu, QWidget* parent)
{
    if (parentProblem()->fieldInfos().count() == 1)
    {
        // one material and boundary
        menu->addAction(actNewBoundary);
        menu->addAction(actNewMaterial);
    }
    else if (parentProblem()->fieldInfos().count() > 1)
    {
        // multiple materials and boundaries
        QMenu *mnuSubBoundaries = new QMenu(tr("New boundary condition"), parent);
        menu->addMenu(mnuSubBoundaries);
        foreach(FieldInfo* fieldInfo, parentProblem()->fieldInfos())
            mnuSubBoundaries->addAction(actNewBoundaries[fieldInfo->fieldId()]);

        QMenu *mnuSubMaterials = new QMenu(tr("New material"), parent);
        menu->addMenu(mnuSubMaterials);
        foreach(FieldInfo* fieldInfo, parentProblem()->fieldInfos())
            mnuSubMaterials->addAction(actNewMaterials[fieldInfo->fieldId()]);
    }
}
*/

void Scene::doFieldsChanged()
{
    boundaries->doFieldsChanged(parentProblem());
    materials->doFieldsChanged(parentProblem());

    faces->doFieldsChanged();
    labels->doFieldsChanged();
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
        Agros2D::log()->printError(tr("VTK Geometry export"), tr("Could not create VTK file (%1)").arg(file.errorString()));
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

                SceneFace *edge = new SceneFace(this, nodeStart, nodeEnd, Value(Agros2D::problem(), 0.0));
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

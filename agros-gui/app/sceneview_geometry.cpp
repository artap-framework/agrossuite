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

#include "sceneview_geometry.h"

#include "util/util.h"
#include "util/global.h"
#include "util/loops.h"
#include "util/constants.h"
#include "logview.h"

#include "scene.h"
#include "solver/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"
#include "scenebasicselectdialog.h"
#include "scenegeometrydialog.h"
#include "scenetransformdialog.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

SceneFaceCommandAdd* getAddCommand(SceneFace *face)
{
    return new SceneFaceCommandAdd(face->nodeStart()->pointValue(), face->nodeEnd()->pointValue(),
                                   face->markersKeys(), face->angleValue(), face->segments());
}

SceneFaceCommandRemove* getRemoveCommand(SceneFace *face)
{
    return new SceneFaceCommandRemove(face->nodeStart()->pointValue(), face->nodeEnd()->pointValue(), face->markersKeys(),
                                      face->angleValue(), face->segments());
}

SceneLabelCommandAdd* getAddCommand(SceneLabel *label)
{
    return new SceneLabelCommandAdd(label->pointValue(), label->markersKeys(), label->area());
}

SceneLabelCommandRemove* getRemoveCommand(SceneLabel *label)
{
    return new SceneLabelCommandRemove(label->pointValue(), label->markersKeys(), label->area());
}

SceneNodeCommandRemove* getRemoveCommand(SceneNode *node)
{
    return new SceneNodeCommandRemove(node->pointValue());
}

SceneViewProblem::SceneViewProblem(QWidget *parent)
    : SceneViewCommon2D(parent),
      m_sceneMode(SceneGeometryMode_OperateOnNodes),
      m_snapToGrid(true),
      m_selectRegion(false),
      m_selectRegionPos(QPointF())
{
    createActionsGeometry();
    createMenuGeometry();
}

SceneViewProblem::~SceneViewProblem()
{
}

ProblemBase *SceneViewProblem::problem() const
{
    return static_cast<ProblemBase *>(Agros::problem());
}

void SceneViewProblem::createActionsGeometry()
{
    actSceneModeProblem = new QAction(icon("main_problem"), tr("Problem"), this);
    actSceneModeProblem->setShortcut(tr("Ctrl+2"));
    actSceneModeProblem->setCheckable(true);

    // scene - operate on items
    actOperateOnNodes = new QAction(icon("geometry_node"), tr("&Nodes"), this);
    actOperateOnNodes->setShortcut(Qt::Key_F2);
    actOperateOnNodes->setCheckable(true);
    actOperateOnNodes->setChecked(true);

    actOperateOnEdges = new QAction(icon("geometry_edge"), tr("&Edges"), this);
    actOperateOnEdges->setShortcut(Qt::Key_F3);
    actOperateOnEdges->setCheckable(true);

    actOperateOnLabels = new QAction(icon("geometry_label"), tr("&Labels"), this);
    actOperateOnLabels->setShortcut(Qt::Key_F4);
    actOperateOnLabels->setCheckable(true);

    actOperateGroup = new QActionGroup(this);
    actOperateGroup->setExclusive(true);
    actOperateGroup->addAction(actOperateOnNodes);
    actOperateGroup->addAction(actOperateOnEdges);
    actOperateGroup->addAction(actOperateOnLabels);
    connect(actOperateGroup, SIGNAL(triggered(QAction *)), this, SLOT(doSceneGeometryModeSet(QAction *)));

    // select region
    actSceneViewSelectRegion = new QAction(icon("geometry_select"), tr("&Select region"), this);
    actSceneViewSelectRegion->setCheckable(true);

    // object properties
    actSceneObjectProperties = new QAction(icon("gear"), tr("Properties"), this);
    actSceneObjectProperties->setShortcut(Qt::Key_Space);
    connect(actSceneObjectProperties, SIGNAL(triggered()), this, SLOT(doSceneObjectProperties()));

    actSceneObjectDeleteSelected = new QAction(icon("geometry_trash"), tr("Delete"), this);
    connect(actSceneObjectDeleteSelected, SIGNAL(triggered()), this, SLOT(doDeleteSelected()));

    // scene edge swap points
    actSceneEdgeSwapDirection = new QAction(tr("Swap direction"), this);
    connect(actSceneEdgeSwapDirection, SIGNAL(triggered()), this, SLOT(doSceneEdgeSwapDirection()));

    actTransform = new QAction(icon("calculator"), tr("&Transform"), this);
    sceneTransformDialog = new SceneTransformDialog(this, this);
    connect(actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));
}

void SceneViewProblem::createMenuGeometry()
{
    m_mnuScene = new QMenu(this);    
    m_mnuScene->addSeparator();
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        m_mnuScene->addAction(actSceneEdgeSwapDirection);
    m_mnuScene->addSeparator();
    m_mnuScene->addAction(actTransform);
    m_mnuScene->addAction(actSceneObjectDeleteSelected);
    m_mnuScene->addAction(actSceneObjectProperties);
}

void SceneViewProblem::doSceneObjectProperties()
{
    if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
    {
        if (Agros::problem()->scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Agros::problem()->scene()->nodes->length(); i++)
            {
                if (Agros::problem()->scene()->nodes->at(i)->isSelected())
                {
                    SceneNodeDialog *dialog = new SceneNodeDialog(Agros::problem()->scene()->nodes->at(i), this);
                    dialog->exec();
                }
            }
        }
    }
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
    {
        if (Agros::problem()->scene()->selectedCount() > 1)
        {
            SceneEdgeSelectDialog *dialog = new SceneEdgeSelectDialog(Agros::problem()->scene()->faces->selected(), this);
            dialog->exec();
        }
        if (Agros::problem()->scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Agros::problem()->scene()->faces->length(); i++)
            {
                if (Agros::problem()->scene()->faces->at(i)->isSelected())
                {
                    SceneFaceDialog *dialog = new SceneFaceDialog(Agros::problem()->scene()->faces->at(i), this);
                    dialog->exec();
                }
            }
        }
    }
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
    {
        if (Agros::problem()->scene()->selectedCount() > 1)
        {
            SceneLabelSelectDialog *dialog = new SceneLabelSelectDialog(Agros::problem()->scene()->labels->selected(), this);
            dialog->exec();
        }
        if (Agros::problem()->scene()->selectedCount() == 1)
        {
            for (int i = 0; i < Agros::problem()->scene()->labels->length(); i++)
            {
                if (Agros::problem()->scene()->labels->at(i)->isSelected())
                {
                    SceneLabelDialog *dialog = new SceneLabelDialog(Agros::problem()->scene()->labels->at(i), this);
                    dialog->exec();
                }
            }
        }
    }

    Agros::problem()->scene()->selectNone();
}

void SceneViewProblem::doSceneEdgeSwapDirection()
{
    // swap
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        if (Agros::problem()->scene()->selectedCount() == 1)
            for (int i = 0; i < Agros::problem()->scene()->faces->length(); i++)
                if (Agros::problem()->scene()->faces->at(i)->isSelected())
                {
                    Agros::problem()->scene()->faces->at(i)->swapDirection();
                    update();
                }
}

void SceneViewProblem::doSelectBasic()
{
    SceneBasicSelectDialog sceneBasicSelectDialog(this, QApplication::activeWindow());
    sceneBasicSelectDialog.exec();
}

void SceneViewProblem::doDeleteSelected()
{
    undoStack()->beginMacro(tr("Delete selected"));

    // nodes
    QList<PointValue> selectedNodePoints;
    foreach (SceneNode *node, problem()->scene()->nodes->selected().items())
        selectedNodePoints.append(node->pointValue());
    if (!selectedNodePoints.isEmpty())
        undoStack()->push(new SceneNodeCommandRemoveMulti(selectedNodePoints));

    // edges
    QList<PointValue> selectedEdgePointsStart;
    QList<PointValue> selectedEdgePointsEnd;
    QList<Value> selectedEdgeAngles;
    QList<int> selectedEdgeSegments;
    QList<QMap<QString, QString> > selectedEdgeMarkers;
    foreach (SceneFace *edge, problem()->scene()->faces->selected().items())
    {
        selectedEdgePointsStart.append(edge->nodeStart()->pointValue());
        selectedEdgePointsEnd.append(edge->nodeEnd()->pointValue());
        selectedEdgeAngles.append(edge->angleValue());
        selectedEdgeSegments.append(edge->segments());
        selectedEdgeMarkers.append(edge->markersKeys());
    }
    if (!selectedEdgePointsStart.isEmpty())
        undoStack()->push(new SceneEdgeCommandRemoveMulti(selectedEdgePointsStart, selectedEdgePointsEnd,
                                                          selectedEdgeMarkers, selectedEdgeAngles, selectedEdgeSegments));

    // labels
    QList<PointValue> selectedLabelPointsStart;
    QList<double> selectedLabelAreas;
    QList<QMap<QString, QString> > selectedLabelMarkers;
    foreach (SceneLabel *label, problem()->scene()->labels->selected().items())
    {
        selectedLabelPointsStart.append(label->pointValue());
        selectedLabelAreas.append(label->area());
        selectedLabelMarkers.append(label->markersKeys());
    }
    if (!selectedLabelPointsStart.isEmpty())
        undoStack()->push(new SceneLabelCommandRemoveMulti(selectedLabelPointsStart, selectedLabelMarkers, selectedLabelAreas));

    undoStack()->endMacro();

    problem()->scene()->invalidate();
}

void SceneViewProblem::doTransform()
{
    sceneTransformDialog->showDialog();
}

void SceneViewProblem::refresh()
{
    refreshActions();

    SceneViewCommon::refresh();
}

void SceneViewProblem::clear()
{
    SceneViewCommon2D::clear();

    m_selectRegion = false;

    m_sceneMode = SceneGeometryMode_OperateOnNodes;
    doZoomBestFit();

    refreshActions();
}

void SceneViewProblem::refreshActions()
{
    // actions
    actSceneViewSelectRegion->setEnabled(actSceneModeProblem->isChecked());
    actOperateOnNodes->setEnabled(actSceneModeProblem->isChecked());
    actOperateOnEdges->setEnabled(actSceneModeProblem->isChecked());
    actOperateOnLabels->setEnabled(actSceneModeProblem->isChecked());

    // properties and delete
    actSceneObjectProperties->setEnabled(false);
    actSceneObjectDeleteSelected->setEnabled(false);
    actTransform->setEnabled(false);

    // set node context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
    {
        actSceneObjectProperties->setEnabled(Agros::problem()->scene()->selectedCount() == 1);
        actSceneObjectDeleteSelected->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
        actTransform->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
    }

    // set boundary context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
    {
        actSceneObjectProperties->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
        actSceneEdgeSwapDirection->setEnabled(Agros::problem()->scene()->selectedCount() == 1);
        actSceneObjectDeleteSelected->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
        actTransform->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
    }

    // set material context menu
    if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
    {
        actSceneObjectProperties->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
        actSceneObjectDeleteSelected->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
        actTransform->setEnabled(Agros::problem()->scene()->selectedCount() > 0);
    }    
}

void SceneViewProblem::doSceneGeometryModeSet(QAction *action)
{
    if (actOperateOnNodes->isChecked())
        m_sceneMode = SceneGeometryMode_OperateOnNodes;
    else if (actOperateOnEdges->isChecked())
        m_sceneMode = SceneGeometryMode_OperateOnEdges;
    else if (actOperateOnLabels->isChecked())
        m_sceneMode = SceneGeometryMode_OperateOnLabels;
    else
    {
        // set default
        actOperateOnNodes->setChecked(true);
        m_sceneMode = SceneGeometryMode_OperateOnNodes;
    }

    Agros::problem()->scene()->highlightNone();
    Agros::problem()->scene()->selectNone();
    m_nodeLast = NULL;

    update();

    emit sceneGeometryModeChanged(m_sceneMode);
}

void SceneViewProblem::selectRegion(const Point &start, const Point &end)
{
    Agros::problem()->scene()->selectNone();

    switch (m_sceneMode)
    {
    case SceneGeometryMode_OperateOnNodes:
        foreach (SceneNode *node, Agros::problem()->scene()->nodes->items())
            if (node->point().x >= start.x && node->point().x <= end.x && node->point().y >= start.y && node->point().y <= end.y)
                node->setSelected(true);
        break;
    case SceneGeometryMode_OperateOnEdges:
        foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
            if (edge->nodeStart()->point().x >= start.x && edge->nodeStart()->point().x <= end.x && edge->nodeStart()->point().y >= start.y && edge->nodeStart()->point().y <= end.y &&
                    edge->nodeEnd()->point().x >= start.x && edge->nodeEnd()->point().x <= end.x && edge->nodeEnd()->point().y >= start.y && edge->nodeEnd()->point().y <= end.y)
                edge->setSelected(true);
        break;
    case SceneGeometryMode_OperateOnLabels:
        foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
            if (label->point().x >= start.x && label->point().x <= end.x && label->point().y >= start.y && label->point().y <= end.y)
                label->setSelected(true);
        break;
    }
}

void SceneViewProblem::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    SceneViewCommon2D::mouseMoveEvent(event);

    m_lastPos = event->pos();

    setToolTip("");

    Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

    // highlight scene objects + hints
    if ((event->modifiers() == 0)
            || ((event->modifiers() & Qt::ControlModifier)
                && (event->modifiers() & Qt::ShiftModifier)
                && (Agros::problem()->scene()->selectedCount() == 0)))
    {
        // highlight scene objects
        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            // highlight the closest node
            SceneNode *node = SceneNode::findClosestNode(Agros::problem()->scene(), p);
            if (node)
            {
                Agros::problem()->scene()->highlightNone();
                node->setHighlighted(true);
                setToolTip(tr("<h3>Node</h3>Point: [%1; %2]<br/>Index: %3").
                           arg(node->point().x, 0, 'g', 3).
                           arg(node->point().y, 0, 'g', 3).
                           arg(Agros::problem()->scene()->nodes->items().indexOf(node)));
                update();
            }
        }
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // highlight the closest edge
            SceneFace *edge = SceneFace::findClosestFace(Agros::problem()->scene(), p);
            if (edge)
            {
                // assigned boundary conditions
                QString str;
                foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
                {
                    str = str + QString("%1 (%2), ").
                            arg(edge->hasMarker(fieldInfo) ? edge->marker(fieldInfo)->name() : "-").
                            arg(fieldInfo->name());
                }
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                Agros::problem()->scene()->highlightNone();
                edge->setHighlighted(true);
                setToolTip(tr("<h3>Edge</h3><b>Point:</b> [%1; %2] - [%3; %4]<br/><b>Boundary conditions:</b> %5<br/><b>Angle:</b> %7 deg.<br/><b>Index:</b> %8").
                           arg(edge->nodeStart()->point().x, 0, 'g', 3).
                           arg(edge->nodeStart()->point().y, 0, 'g', 3).
                           arg(edge->nodeEnd()->point().x, 0, 'g', 3).
                           arg(edge->nodeEnd()->point().y, 0, 'g', 3).
                           arg(str).
                           arg(edge->angle(), 0, 'f', 0).
                           arg(Agros::problem()->scene()->faces->items().indexOf(edge)));
                update();
            }
        }
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // highlight the closest label
            Agros::problem()->scene()->highlightNone();

            SceneLabel *label = SceneLabel::findClosestLabel(Agros::problem()->scene(), p);
            if (label)
            {
                // assigned materials
                QString str, polynomial_order;
                foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
                {
                    str = str + QString("%1 (%2), ").
                            arg(label->hasMarker(fieldInfo) ? label->marker(fieldInfo)->name() : "-").
                            arg(fieldInfo->name());
                    polynomial_order = polynomial_order + QString("%1 (%2), ").
                            arg(fieldInfo->labelPolynomialOrder(label)).
                            arg(fieldInfo->name());
                }
                if (str.length() > 0)
                    str = str.left(str.length() - 2);

                QList<LoopsInfo::Triangle> triangles = Agros::problem()->scene()->loopsInfo()->polygonTriangles()[label];

                RectPoint boundingBox(Point(numeric_limits<double>::max(),  numeric_limits<double>::max()),
                                      Point(-numeric_limits<double>::max(), -numeric_limits<double>::max()));
                double area = 0.0;
                foreach (LoopsInfo::Triangle triangle, triangles)
                {
                    boundingBox.start.x = qMin(boundingBox.start.x, triangle.a.x);
                    boundingBox.end.x = qMax(boundingBox.end.x, triangle.a.x);
                    boundingBox.start.y = qMin(boundingBox.start.y, triangle.a.y);
                    boundingBox.end.y = qMax(boundingBox.end.y, triangle.a.y);

                    boundingBox.start.x = qMin(boundingBox.start.x, triangle.b.x);
                    boundingBox.end.x = qMax(boundingBox.end.x, triangle.b.x);
                    boundingBox.start.y = qMin(boundingBox.start.y, triangle.b.y);
                    boundingBox.end.y = qMax(boundingBox.end.y, triangle.b.y);

                    boundingBox.start.x = qMin(boundingBox.start.x, triangle.c.x);
                    boundingBox.end.x = qMax(boundingBox.end.x, triangle.c.x);
                    boundingBox.start.y = qMin(boundingBox.start.y, triangle.c.y);
                    boundingBox.end.y = qMax(boundingBox.end.y, triangle.c.y);

                    area += (triangle.a.x * (triangle.b.y - triangle.c.y) +
                             triangle.b.x * (triangle.c.y - triangle.a.y) +
                             triangle.c.x * (triangle.a.y - triangle.b.y)) / 2.0;

                }

                label->setHighlighted(true);
                setToolTip(tr("<h3>Label</h3><b>Point:</b> [%1; %2]<br/><b>Area:</b> %3<br/><b>Material:</b> %4<br/><b>Polynomial order:</b> %5<br/><b>Index:</b> %6").
                           arg(label->point().x, 0, 'g', 3).
                           arg(label->point().y, 0, 'g', 3).
                           arg(QString("[%1; %2], [%3; %4], %5").arg(boundingBox.start.x).arg(boundingBox.start.y).arg(boundingBox.end.x).arg(boundingBox.end.y).arg(area)).
                           arg(str).
                           arg(polynomial_order).
                           arg(Agros::problem()->scene()->labels->items().indexOf(label)));
                update();
            }
        }
    }

    // add edge by mouse - draw line
    if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
    {
        // add edge directly by mouse click - highlight
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // add edge directly by mouse click
            SceneNode *node = SceneNode::findClosestNode(Agros::problem()->scene(), p);
            if (node)
            {
                Agros::problem()->scene()->highlightNone();
                node->setHighlighted(true);
                update();
            }
        }
    }

    // snap to grid - nodes
    m_snapToGrid = ((Agros::problem()->config()->value(ProblemConfig::SnapToGrid).toBool())
                    && ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
                    && (m_sceneMode == SceneGeometryMode_OperateOnNodes));

    if (m_snapToGrid && !(event->modifiers() & Qt::ControlModifier))
    {
        m_snapToGrid = false;
        update();
    }
    if (m_snapToGrid && (event->modifiers() & Qt::ControlModifier))
        update();

    // move nodes and labels directly by mouse - left mouse + control + shift
    if ((event->buttons() & Qt::LeftButton)
            && ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)))
    {
        Point dp = Point(2.0/width() * dx/m_scale2d*aspect(), -2.0/height() * dy/m_scale2d);

        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            // mouse move length memory
            static Point len;
            len = len + dp;

            if (Agros::problem()->config()->value(ProblemConfig::SnapToGrid).toBool())
            {
                if (fabs(len.x) > Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble())
                {
                    dp.x = (len.x > 0) ? Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() : -Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
                    dp.y = 0;
                    len.x = 0;

                    undoStack()->beginMacro(tr("Translation"));
                    transformTranslate(dp, false, true);
                    undoStack()->endMacro();
                }

                if (fabs(len.y) > Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble())
                {
                    dp.x = 0;
                    dp.y = (len.y > 0) ? Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() : -Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
                    len.y = 0;

                    undoStack()->beginMacro(tr("Translation"));
                    transformTranslate(dp, false, true);
                    undoStack()->endMacro();
                }
            }
            else
            {
                undoStack()->beginMacro(tr("Translation"));
                transformTranslate(dp, false, true);
                undoStack()->endMacro();
            }
        }
        else if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            static Point len;
            len = len + dp;

            if (Agros::problem()->config()->value(ProblemConfig::SnapToGrid).toBool())
            {
                if (fabs(len.x) > Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble())
                {
                    dp.x = (len.x > 0) ? Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() : -Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
                    dp.y = 0;
                    len.x = 0;

                    undoStack()->beginMacro(tr("Translation"));
                    transformTranslate(dp, false, true);
                    undoStack()->endMacro();
                }

                if (fabs(len.y) > Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble())
                {
                    dp.x = 0;
                    dp.y = (len.y > 0) ? Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() : -Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
                    len.y = 0;

                    undoStack()->beginMacro(tr("Translation"));
                    transformTranslate(dp, false, true);
                    undoStack()->endMacro();
                }
            }
            else
            {
                undoStack()->beginMacro(tr("Translation"));
                transformTranslate(dp, false, true);
                undoStack()->endMacro();
            }
        }
        else if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            undoStack()->beginMacro(tr("Translation"));
            transformTranslate(dp, false, true);
            undoStack()->endMacro();
        }

        update();
    }

    if (m_snapToGrid)
    {
        Point snapPoint;
        snapPoint.x = floor(p.x / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
        snapPoint.y = floor(p.y / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();

        emit mouseMoved(snapPoint);
    }
    else
    {
        emit mouseMoved(p);
    }
}

void SceneViewProblem::mousePressEvent(QMouseEvent *event)
{
    // select region
    if ((event->button() & Qt::LeftButton)
            && !(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier))
    {
        // select region
        if (actSceneViewSelectRegion->isChecked())
        {
            m_selectRegionPos = m_lastPos;
            actSceneViewSelectRegion->setChecked(false);
            actSceneViewSelectRegion->setData(true);
            m_selectRegion = true;

            return;
        }
    }

    m_lastPos = event->pos();
    Point p = transform(Point(event->pos().x(), event->pos().y()));

    // add node, edge or label by mouse click
    // control + left mouse
    if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
    {
        // add node directly by mouse click
        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            Point pointNode;

            // snap to grid
            if (m_snapToGrid)
            {
                Point snapPoint = transform(Point(m_lastPos.x(), m_lastPos.y()));

                pointNode.x = floor(snapPoint.x / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
                pointNode.y = floor(snapPoint.y / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
            }
            else
            {
                pointNode = p;
            }

            // coordinates must be greater than or equal to 0 (axisymmetric case)
            if ((Agros::problem()->config()->coordinateType() == CoordinateType_Axisymmetric) &&
                    (pointNode.x < 0))
            {
                QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater than or equal to zero."));

            }
            else
            {
                SceneNode *node = new SceneNode(Agros::problem()->scene(), pointNode);
                SceneNode *nodeAdded = Agros::problem()->scene()->addNode(node);
                if (nodeAdded == node)
                    undoStack()->push(new SceneNodeCommandAdd(node->pointValue()));
                update();
            }
        }
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // add edge directly by mouse click
            SceneNode *node = SceneNode::findClosestNode(Agros::problem()->scene(), p);
            if (node)
            {
                if (m_nodeLast == NULL)
                {
                    m_nodeLast = node;
                    m_nodeLast->setSelected(true);
                }
                else
                {
                    if (node != m_nodeLast)
                    {
                        SceneFace *edge = new SceneFace(Agros::problem()->scene(), m_nodeLast, node, 0);
                        SceneFace *edgeAdded = Agros::problem()->scene()->addFace(edge);

                        if (edgeAdded == edge)
                            undoStack()->push(getAddCommand(edge));
                    }

                    m_nodeLast->setSelected(false);
                    m_nodeLast = NULL;
                }

                update();
            }
        }
        // add label directly by mouse click
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // coordinates must be greater then or equal to 0 (axisymmetric case)
            if ((Agros::problem()->config()->coordinateType() == CoordinateType_Axisymmetric) &&
                    (p.x < 0))
            {
                QMessageBox::warning(this, tr("Node"), tr("Radial component must be greater then or equal to zero."));
            }
            else
            {
                SceneLabel *label = new SceneLabel(Agros::problem()->scene(), p, 0);
                SceneLabel *labelAdded = Agros::problem()->scene()->addLabel(label);

                if (labelAdded == label)
                    undoStack()->push(getAddCommand(label));

                update();
            }
        }
    }

    // multiple select or just one node or label due to movement
    // nothing or (shift + control) + left mouse
    if ((event->button() & Qt::LeftButton) && ((event->modifiers() == 0)
                                               || ((event->modifiers() & Qt::ControlModifier)
                                                   && (event->modifiers() & Qt::ShiftModifier)
                                                   && (Agros::problem()->scene()->selectedCount() == 0))))
    {
        // select scene objects
        if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
        {
            // select the closest node
            SceneNode *node = SceneNode::findClosestNode(Agros::problem()->scene(), p);
            if (node)
            {
                node->setSelected(!node->isSelected());
                update();
            }
        }

        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // select the closest edge
            SceneFace *edge = SceneFace::findClosestFace(Agros::problem()->scene(), p);
            if (edge)
            {
                edge->setSelected(!edge->isSelected());
                update();
            }
        }

        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            // select the closest label
            SceneLabel *label = SceneLabel::findClosestLabel(Agros::problem()->scene(), p);
            if (label)
            {
                label->setSelected(!label->isSelected());
                update();
            }
        }
    }

    refreshActions();

    SceneViewCommon2D::mousePressEvent(event);
}

void SceneViewProblem::mouseReleaseEvent(QMouseEvent *event)
{
    actSceneViewSelectRegion->setChecked(false);

    if (m_selectRegion)
    {
        Point posStart = transform(Point(m_selectRegionPos.x(), m_selectRegionPos.y()));
        Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

        if (actSceneViewSelectRegion->data().value<bool>())
            selectRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));

        actSceneViewSelectRegion->setData(false);
    }

    m_selectRegion = false;
    update();

    // move by mouse - select none
    if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
    {
        Agros::problem()->scene()->selectNone();
    }

    refreshActions();

    SceneViewCommon2D::mouseReleaseEvent(event);
}

void SceneViewProblem::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier))
    {
        Point p = transform(Point(event->pos().x(), event->pos().y()));

        if (event->button() & Qt::LeftButton)
        {
            // select scene objects
            Agros::problem()->scene()->selectNone();
            if (m_sceneMode == SceneGeometryMode_OperateOnNodes)
            {
                // select the closest node
                SceneNode *node = SceneNode::findClosestNode(Agros::problem()->scene(), p);
                if (node)
                {
                    node->setSelected(true);
                    update();

                    SceneNodeDialog *dialog = new SceneNodeDialog(node, this);
                    if (dialog->exec() == QDialog::Accepted)
                    {
                        update();
                    }
                }
            }
            if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
            {
                // select the closest label
                SceneFace *edge = SceneFace::findClosestFace(Agros::problem()->scene(), p);
                if (edge)
                {
                    edge->setSelected(true);
                    update();

                    SceneFaceDialog *dialog = new SceneFaceDialog(edge, this);
                    if (dialog->exec() == QDialog::Accepted)
                    {
                        update();
                    }
                }
            }
            if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
            {
                // select the closest label
                SceneLabel *label = SceneLabel::findClosestLabel(Agros::problem()->scene(), p);
                if (label)
                {
                    label->setSelected(true);
                    update();

                    SceneLabelDialog *dialog = new SceneLabelDialog(label, this);
                    if (dialog->exec() == QDialog::Accepted)
                    {
                        update();
                    }
                }
            }
            Agros::problem()->scene()->selectNone();
            update();
        }
    }

    refreshActions();

    SceneViewCommon2D::mouseDoubleClickEvent(event);
}

void SceneViewProblem::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_Delete:
    {
        doDeleteSelected();
        break;
    }
    case Qt::Key_Space:
    {
        doSceneObjectProperties();
        break;
    }
    case Qt::Key_A:
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {
            Agros::problem()->scene()->selectAll(m_sceneMode);
            update();
        }
        break;
    }
    default:
        QWidget::keyPressEvent(event);
    }

    refreshActions();

    SceneViewCommon2D::keyPressEvent(event);

    // snap to grid
    m_snapToGrid = ((Agros::problem()->config()->value(ProblemConfig::SnapToGrid).toBool())
                    && (event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier)
                    && (m_sceneMode == SceneGeometryMode_OperateOnNodes));
}

void SceneViewProblem::keyReleaseEvent(QKeyEvent *event)
{
    m_snapToGrid = false;
    update();

    refreshActions();

    SceneViewCommon2D::keyReleaseEvent(event);
}

void SceneViewProblem::contextMenuEvent(QContextMenuEvent *event)
{
    refreshActions();

    m_mnuScene->exec(event->globalPos());
}

void SceneViewProblem::paintRulersHintsEdges()
{
    loadProjection2d(true);

    Point cornerMin = transform(Point(0, 0));
    Point cornerMax = transform(Point(width(), height()));

    glColor3d(0.0, 0.53, 0.0);

    Point p = transform(m_lastPos.x(), m_lastPos.y());
    Point rulersAreaScreen = rulersAreaSize();
    Point rulersArea(2.0/width()*rulersAreaScreen.x/m_scale2d*aspect(),
                     2.0/height()*rulersAreaScreen.y/m_scale2d);

    double tickSize = rulersArea.y / 3.0;

    Point snapPoint = p;
    if (m_snapToGrid)
    {
        snapPoint.x = floor(p.x / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
        snapPoint.y = floor(p.y / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();

        // hint line
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(1, 0x8FFF);

        glLineWidth(1.0);
        glBegin(GL_LINES);
        glVertex2d(snapPoint.x, cornerMax.y - rulersArea.y);
        glVertex2d(snapPoint.x, cornerMin.y);
        glVertex2d(cornerMin.x + rulersArea.x, snapPoint.y);
        glVertex2d(cornerMax.x, snapPoint.y);
        glEnd();

        glDisable(GL_LINE_STIPPLE);
    }

    // ticks
    glLineWidth(3.0);
    glBegin(GL_TRIANGLES);
    // horizontal
    glVertex2d(p.x, cornerMax.y + rulersArea.y);
    glVertex2d(p.x + tickSize / 2.0, cornerMax.y + rulersArea.y - tickSize);
    glVertex2d(p.x - tickSize / 2.0, cornerMax.y + rulersArea.y - tickSize);

    // vertical
    glVertex2d(cornerMin.x + rulersArea.x, p.y);
    glVertex2d(cornerMin.x + rulersArea.x - tickSize, p.y + tickSize / 2.0);
    glVertex2d(cornerMin.x + rulersArea.x - tickSize, p.y - tickSize / 2.0);
    glEnd();

    // snap to grid text
    if (m_snapToGrid)
    {
        loadProjectionViewPort();

        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

        Point scr = untransform(snapPoint.x, snapPoint.y);
        printRulersAt(scr.x + 1.5*m_labelRulersSize,
                      scr.y + 1.5*m_labelRulersSize * 0.7,
                      QString("%1, %2").arg(snapPoint.x).arg(snapPoint.y));
    }
}

void SceneViewProblem::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    paintGrid();

    // geometry
    paintGeometry();

    // rulers
    if (Agros::configComputer()->value(Config::Config_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHintsEdges();
    }

    // axes
    if (Agros::configComputer()->value(Config::Config_ShowAxes).toBool()) paintAxes();

    paintSelectRegion();
    paintZoomRegion();
    paintSnapToGrid();
    paintEdgeLine();
}

void SceneViewProblem::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
    {
        if (m_sceneMode == SceneGeometryMode_OperateOnEdges)
        {
            // edge without marker
            if (edge->markersCount() == 0)
            {
                glEnable(GL_LINE_STIPPLE);
                glLineStipple(1, 0x8FFF);
            }
        }

        glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
        glLineWidth(EDGEWIDTH);

        if (edge->hasLyingNode() || edge->isOutsideArea() || Agros::problem()->scene()->crossings().contains(edge))
        {
            glColor3d(COLORCROSSED[0], COLORCROSSED[1], COLORCROSSED[2]);
            glLineWidth(EDGEWIDTH);
        }
        if (edge->isHighlighted())
        {
            glColor3d(COLORHIGHLIGHTED[0], COLORHIGHLIGHTED[1], COLORHIGHLIGHTED[2]);
            glLineWidth(EDGEWIDTH + 2.0);
        }
        if (edge->isSelected())
        {
            glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
            glLineWidth(EDGEWIDTH + 2.0);
        }

        if (fabs(edge->angle()) < 5) // if (edge->isStraight())
        {
            glBegin(GL_LINES);
            glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
            glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
            glEnd();
        }
        else
        {
            Point center = edge->center();
            double radius = edge->radius();
            double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                      center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle());
        }

        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0);
    }

    // nodes
    foreach (SceneNode *node, Agros::problem()->scene()->nodes->items())
    {
        glColor3d(COLORNODE[0], COLORNODE[1], COLORNODE[2]);
        glPointSize(NODESIZE);

        glBegin(GL_POINTS);
        glVertex2d(node->point().x, node->point().y);
        glEnd();

        glColor3d(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2]);
        glPointSize(NODESIZE - 2.0);
        glBegin(GL_POINTS);
        glVertex2d(node->point().x, node->point().y);
        glEnd();

        bool isError = false;
        if ((node->isSelected()) || (node->isHighlighted()) || (isError = node->isError()) )
        {
            if (isError)
            {
                glColor3d(COLORCROSSED[0], COLORCROSSED[1], COLORCROSSED[2]);
                glPointSize(EDGEWIDTH + 2.0);
            }

            if (node->isHighlighted())
            {
                glColor3d(COLORHIGHLIGHTED[0], COLORHIGHLIGHTED[1], COLORHIGHLIGHTED[2]);
                glPointSize(EDGEWIDTH - 2.0);
            }

            if (node->isSelected())
            {
                glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
                glPointSize(EDGEWIDTH - 2.0);
            }

            glBegin(GL_POINTS);
            glVertex2d(node->point().x, node->point().y);
            glEnd();
        }
    }

    // labels
    double dxlabel = 1.3/width() * LABELSIZE/m_scale2d*aspect();
    double dylabel = 1.3/height() * LABELSIZE/m_scale2d*aspect();
    foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
    {
        glColor3d(COLORLABEL[0], COLORLABEL[1], COLORLABEL[2]);
        glPointSize(LABELSIZE);
        glBegin(GL_POINTS);
        glVertex2d(label->point().x, label->point().y);
        glEnd();

        // glLineWidth(1.0);
        // glBegin(GL_POLYGON);
        // for (int i = 0; i<360; i = i + 20) glVertex2d(label->point().x + dxlabel*fastcos(i/180.0*M_PI), label->point().y + dylabel*fastsin(i/180.0*M_PI));
        // glEnd();

        // glBegin(GL_TRIANGLES);
        // glVertex2d(label->point().x + dxlabel/2.0, label->point().y - dylabel/3.0);
        // glVertex2d(label->point().x - dxlabel/2.0, label->point().y - dylabel/3.0);
        // glVertex2d(label->point().x, label->point().y + dylabel*2.0/3.0);
        // glEnd();

        glColor3d(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2]);
        glPointSize(LABELSIZE - 2.0);
        glBegin(GL_POINTS);
        glVertex2d(label->point().x, label->point().y);
        glEnd();

        // glBegin(GL_POLYGON);
        // for (int i = 0; i<360; i = i + 20) glVertex2d(label->point().x + 0.6*dxlabel*fastcos(i/180.0*M_PI), label->point().y + 0.6*dylabel*fastsin(i/180.0*M_PI));
        // glEnd();
        // glBegin(GL_TRIANGLES);
        // glVertex2d(label->point().x + dxlabel/2.0 * 0.4, label->point().y - dylabel/3.0 * 0.4);
        // glVertex2d(label->point().x - dxlabel/2.0 * 0.4, label->point().y - dylabel/3.0 * 0.4);
        // glVertex2d(label->point().x, label->point().y + dylabel*2.0/3.0 * 0.4);
        // glEnd();

        if ((label->isSelected()) || (label->isHighlighted()))
        {
            if (label->isHighlighted())
                glColor3d(COLORHIGHLIGHTED[0], COLORHIGHLIGHTED[1], COLORHIGHLIGHTED[2]);
            if (label->isSelected())
                glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);

            glPointSize(LABELSIZE);
            glBegin(GL_POINTS);
            glVertex2d(label->point().x, label->point().y);
            glEnd();

            // glBegin(GL_POLYGON);
            // for (int i = 0; i<360; i = i + 20) glVertex2d(label->point().x + 0.6*dxlabel*fastcos(i/180.0*M_PI), label->point().y + 0.6*dylabel*fastsin(i/180.0*M_PI));
            // glEnd();

            // glBegin(GL_TRIANGLES);
            // glVertex2d(label->point().x + dxlabel/2.0 * 0.4, label->point().y - dylabel/3.0 * 0.4);
            // glVertex2d(label->point().x - dxlabel/2.0 * 0.4, label->point().y - dylabel/3.0 * 0.4);
            // glVertex2d(label->point().x, label->point().y + dylabel*2.0/3.0 * 0.4);
            // glEnd();
        }

        // area size
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            double radius = sqrt(label->area()/M_PI);
            if (radius > 0)
            {
                glColor3d(0, 0.95, 0.9);

                glLineWidth(1.0);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i<360; i = i + 10)
                {
                    glVertex2d(label->point().x + radius*fastcos(i/180.0*M_PI),
                               label->point().y + radius*fastsin(i/180.0*M_PI));
                }
                glEnd();
            }
        }
    }

    try
    {
        if (Agros::problem()->scene()->crossings().isEmpty())
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            // blended rectangle
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glBegin(GL_TRIANGLES);
            QMapIterator<SceneLabel *, QList<LoopsInfo::Triangle> > i(Agros::problem()->scene()->loopsInfo()->polygonTriangles());
            while (i.hasNext())
            {
                i.next();

                if (i.key()->isSelected() && i.key()->isHole())
                    glColor4f(0.7f, 0.1f, 0.3f, 0.55f);
                else if (i.key()->isSelected())
                    glColor4f(0.3f, 0.1f, 0.7f, 0.55f);
                else if (i.key()->isHighlighted() && i.key()->isHole())
                    glColor4f(0.7f, 0.1, 0.3f, 0.10);
                else if (i.key()->isHighlighted())
                    glColor4f(0.3f, 0.1, 0.7f, 0.18f);
                else if (i.key()->isHole())
                    glColor4f(0.3f, 0.1f, 0.7f, 0.00f);
                else
                    glColor4f(0.3f, 0.1f, 0.7, 0.10f);

                foreach (LoopsInfo::Triangle triangle, i.value())
                {
                    glVertex2d(triangle.a.x, triangle.a.y);
                    glVertex2d(triangle.b.x, triangle.b.y);
                    glVertex2d(triangle.c.x, triangle.c.y);
                }
            }
            glEnd();

            glDisable(GL_BLEND);
            glDisable(GL_POLYGON_OFFSET_FILL);
        }

        // FIX: temp
        /*
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        QMapIterator<SceneLabel*, QList<Triangle> > j(labels);
        while (j.hasNext())
        {
            j.next();

            glColor4f(0.3, 0.1, 0.7, 0.55);

            glBegin(GL_TRIANGLES);
            foreach (Triangle triangle, j.value())
            {
                glVertex2d(triangle.a.x, triangle.a.y);
                glVertex2d(triangle.b.x, triangle.b.y);
                glVertex2d(triangle.c.x, triangle.c.y);
            }
            glEnd();
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        */
    }
    catch (AgrosException& ame)
    {
        // This is a function responsible for painting geometry in process of its creation by the user.
        // Situations, which do not represent valid geometry, are acceptable here
        // Therefore catch exceptions and do nothing
    }

    // labels hints
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
    {
        if (m_sceneMode == SceneGeometryMode_OperateOnLabels)
        {
            glColor3d(0.1, 0.1, 0.1);

            // assigned materials
            QString str;
            foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
                str = str + QString("%1, ").
                        arg(label->hasMarker(fieldInfo) ? label->marker(fieldInfo)->name() : tr(""));
            if (str.length() > 0)
                str = str.left(str.length() - 2);

            Point scr = untransform(label->point().x, label->point().y);

            printRulersAt(scr.x - 1.5*m_labelRulersSize * str.length() / 2.0,
                          scr.y - 1.5*m_labelRulersSize * 1.2, str);
        }
    }
}

void SceneViewProblem::paintSnapToGrid()
{
    if (m_snapToGrid)
    {
        loadProjection2d(true);

        Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

        Point snapPoint;
        snapPoint.x = floor(p.x / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();
        snapPoint.y = floor(p.y / Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble() + 0.5) * Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble();

        glColor3d(COLORHIGHLIGHTED[0], COLORHIGHLIGHTED[1], COLORHIGHLIGHTED[2]);
        glPointSize(EDGEWIDTH - 1.0);
        glBegin(GL_POINTS);
        glVertex2d(snapPoint.x, snapPoint.y);
        glEnd();
    }
}

void SceneViewProblem::paintEdgeLine()
{
    if (m_nodeLast)
    {
        if (m_nodeLast->isSelected())
        {
            loadProjection2d(true);

            Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

            glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);

            // check for crossing
            foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
            {
                QList<Point> intersects = intersection(p, m_nodeLast->point(),
                                                       m_nodeLast->point(), 0, 0,
                                                       edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                                       edge->center(), edge->radius(), edge->angle());

                foreach (Point intersect, intersects)
                {
                    // red line and point
                    glColor3d(1.0, 0.0, 0.0);

                    glPointSize(5.0);

                    glBegin(GL_POINTS);
                    glVertex2d(intersect.x, intersect.y);
                    glEnd();
                }
            }

            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1, 0x8FFF);

            glLineWidth(EDGEWIDTH);

            glBegin(GL_LINES);
            glVertex2d(m_nodeLast->point().x, m_nodeLast->point().y);
            glVertex2d(p.x, p.y);
            glEnd();

            glDisable(GL_LINE_STIPPLE);
            glLineWidth(1.0);
        }
    }
}

void SceneViewProblem::paintSelectRegion()
{
    // zoom region
    if (m_selectRegion)
    {
        loadProjection2d(true);

        Point posStart = transform(Point(m_selectRegionPos.x(), m_selectRegionPos.y()));
        Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

        drawBlend(posStart, posEnd,
                  COLORHIGHLIGHTED[0], COLORHIGHLIGHTED[1], COLORHIGHLIGHTED[2]);
    }
}

void SceneViewProblem::saveGeometryToSvg(const QString &fileName)
{
    QString geometry = generateSvgGeometry(Agros::problem()->scene()->faces->items());
    writeStringContent(fileName, geometry);
}



bool SceneViewProblem::moveSelectedNodes(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy)
{
    // select endpoints
    foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
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

    foreach (SceneNode *node, Agros::problem()->scene()->nodes->selected().items())
    {
        PointValue newPoint = PointValue(Agros::problem(), calculateNewPoint(mode, node->point(), point, angle, scaleFactor));

        SceneNode *obstructNode = Agros::problem()->scene()->getNode(newPoint.point());
        if (obstructNode && !obstructNode->isSelected())
        {
            Agros::log()->printWarning(tr("Geometry"), tr("Cannot perform transformation, existing point would be overwritten"));
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
        undoStack()->push(new SceneNodeCommandAddMulti(newPoints));

        Agros::problem()->scene()->nodes->setSelected(false);

        // select new
        foreach(PointValue point, newPoints)
            Agros::problem()->scene()->getNode(point.point())->setSelected(true);

        foreach(PointValue point, pointsToSelect)
            Agros::problem()->scene()->getNode(point.point())->setSelected(true);
    }
    else
    {
        undoStack()->push(new SceneNodeCommandMoveMulti(points, newPoints));
    }

    return true;
}

bool SceneViewProblem::moveSelectedEdges(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy, bool withMarkers)
{
    QList<SceneFace *> selectedEdges;

    foreach (SceneFace *edge, Agros::problem()->scene()->faces->selected().items())
    {
        selectedEdges.append(edge);
    }

    if(selectedEdges.isEmpty())
        return true;

    Agros::problem()->scene()->nodes->setSelected(false);

    if(!copy)
        return true;

    QList<QPair<Point, Point> > newEdgeEndPoints;
    QList<PointValue> edgeStartPointsToAdd;
    QList<PointValue> edgeEndPointsToAdd;
    QList<Value> edgeAnglesToAdd;
    QList<int> edgeSegmentsToAdd;
    QList<QMap<QString, QString> > edgeMarkersToAdd;

    foreach (SceneFace *edge, selectedEdges)
    {
        PointValue newPointStart = PointValue(Agros::problem(), calculateNewPoint(mode, edge->nodeStart()->point(), point, angle, scaleFactor));
        PointValue newPointEnd = PointValue(Agros::problem(), calculateNewPoint(mode, edge->nodeEnd()->point(), point, angle, scaleFactor));

        // add new edge
        SceneNode *newNodeStart = Agros::problem()->scene()->getNode(newPointStart.point());
        SceneNode *newNodeEnd = Agros::problem()->scene()->getNode(newPointEnd.point());

        assert(newNodeStart && newNodeEnd);

        SceneFace *obstructEdge = Agros::problem()->scene()->getFace(newPointStart.point(), newPointEnd.point());
        if (obstructEdge && !obstructEdge->isSelected())
        {
            Agros::log()->printWarning(tr("Geometry"), tr("Cannot perform transformation, existing edge would be overwritten"));
            return false;
        }

        if(! obstructEdge)
        {
            edgeStartPointsToAdd.push_back(newPointStart);
            edgeEndPointsToAdd.push_back(newPointEnd);
            edgeAnglesToAdd.push_back(edge->angleValue());
            edgeSegmentsToAdd.push_back(edge->segments());            

            if(withMarkers)
                edgeMarkersToAdd.append(edge->markersKeys());
        }

        newEdgeEndPoints.push_back(QPair<Point, Point>(newPointStart.point(), newPointEnd.point()));
    }

    Agros::problem()->scene()->faces->setSelected(false);

    undoStack()->push(new SceneEdgeCommandAddMulti(edgeStartPointsToAdd, edgeEndPointsToAdd,
                                                   edgeMarkersToAdd, edgeAnglesToAdd, edgeSegmentsToAdd));

    for(int i = 0; i < newEdgeEndPoints.size(); i++)
    {
        SceneFace* sceneEdge = Agros::problem()->scene()->getFace(newEdgeEndPoints[i].first, newEdgeEndPoints[i].second);
        if (sceneEdge)
            sceneEdge->setSelected(true);
    }

    return true;
}

bool SceneViewProblem::moveSelectedLabels(SceneTransformMode mode, Point point, double angle, double scaleFactor, bool copy, bool withMarkers)
{
    QList<PointValue> points;
    QList<PointValue> newPoints;
    QList<PointValue> pointsToSelect;

    QList<double> newAreas;
    QList<QMap<QString, QString> > newMarkers;

    foreach (SceneLabel *label, Agros::problem()->scene()->labels->selected().items())
    {
        PointValue newPoint = PointValue(Agros::problem(), calculateNewPoint(mode, label->point(), point, angle, scaleFactor));

        SceneLabel *obstructLabel = Agros::problem()->scene()->getLabel(newPoint.point());
        if (obstructLabel && !obstructLabel->isSelected())
        {
            Agros::log()->printWarning(tr("Geometry"), tr("Cannot perform transformation, existing label would be overwritten"));
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
        undoStack()->push(new SceneLabelCommandAddMulti(newPoints, newMarkers, newAreas));

        Agros::problem()->scene()->labels->setSelected(false);

        // select new
        foreach(PointValue point, newPoints)
            Agros::problem()->scene()->getLabel(point.point())->setSelected(true);

        foreach(PointValue point, pointsToSelect)
            Agros::problem()->scene()->getLabel(point.point())->setSelected(true);
    }
    else
    {
        undoStack()->push(new SceneLabelCommandMoveMulti(points, newPoints));
    }

    return true;
}

void SceneViewProblem::transformPosition(SceneTransformMode mode, const Point &point, double angle, double scaleFactor, bool copy, bool withMarkers)
{
    bool okNodes, okEdges = true;
    okNodes = moveSelectedNodes(mode, point, angle, scaleFactor, copy);
    if(okNodes)
        okEdges = moveSelectedEdges(mode, point, angle, scaleFactor, copy, withMarkers);
    moveSelectedLabels(mode, point, angle, scaleFactor, copy, withMarkers);

    if(!okNodes || !okEdges)
        Agros::problem()->scene()->nodes->setSelected(false);

    Agros::problem()->scene()->invalidate();
}

void SceneViewProblem::transformTranslate(const Point &point, bool copy, bool withMarkers)
{
    transformPosition(SceneTransformMode_Translate, point, 0.0, 0.0, copy, withMarkers);
}

void SceneViewProblem::transformRotate(const Point &point, double angle, bool copy, bool withMarkers)
{
    transformPosition(SceneTransformMode_Rotate, point, angle, 0.0, copy, withMarkers);
}

void SceneViewProblem::transformScale(const Point &point, double scaleFactor, bool copy, bool withMarkers)
{
    transformPosition(SceneTransformMode_Scale, point, 0.0, scaleFactor, copy, withMarkers);
}

Point SceneViewProblem::calculateNewPoint(SceneTransformMode mode, Point originalPoint, Point transformationPoint, double angle, double scaleFactor)
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

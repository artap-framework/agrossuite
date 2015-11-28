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

#include "util.h"
#include "util/global.h"
#include "gui/valuelineedit.h"

#include "scene.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/field.h"

SceneFace::SceneFace(Scene *scene, SceneNode *nodeStart, SceneNode *nodeEnd, const Value &angle, int segments, bool isCurvilinear)
    : MarkedSceneBasic<SceneBoundary>(scene),
      m_nodeStart(nodeStart), m_nodeEnd(nodeEnd), m_angle(angle), m_segments(segments), m_isCurvilinear(isCurvilinear)
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

void SceneFace::setCurvilinear(bool isCurvilinear)
{
    m_isCurvilinear = isCurvilinear;
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

int SceneFace::showDialog(QWidget *parent, bool isNew)
{
    SceneFaceDialog *dialog = new SceneFaceDialog(this, parent, isNew);
    return dialog->exec();
}

SceneFaceCommandAdd* SceneFace::getAddCommand()
{
    return new SceneFaceCommandAdd(m_nodeStart->point(), m_nodeEnd->point(), markersKeys(), m_angle, m_segments, m_isCurvilinear);
}

SceneFaceCommandRemove* SceneFace::getRemoveCommand()
{
    return new SceneFaceCommandRemove(m_nodeStart->point(), m_nodeEnd->point(), markersKeys(), m_angle, m_segments, m_isCurvilinear);
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
            edge->scene()->undoStack()->push(new SceneFaceCommandRemove(edge->nodeStart()->point(),
                                                                           edge->nodeEnd()->point(),
                                                                           edge->markersKeys(),
                                                                           edge->angle(),
                                                                           edge->segments(),
                                                                           edge->isCurvilinear()));
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

SceneFace* SceneFaceContainer::get(const Point &pointStart, const Point &pointEnd, double angle, int segments, bool isCurvilinear) const
{
    foreach (SceneFace *edgeCheck, m_data)
    {
        if (((edgeCheck->nodeStart()->point() == pointStart) && (edgeCheck->nodeEnd()->point() == pointEnd))
                && ((edgeCheck->angle() - angle) < EPS_ZERO) && (edgeCheck->segments() == segments) && (edgeCheck->isCurvilinear() == isCurvilinear))
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

// *************************************************************************************************************************************

SceneFaceMarker::SceneFaceMarker(SceneFace *edge, FieldInfo *fieldInfo, QWidget *parent)
    : QGroupBox(parent), m_fieldInfo(fieldInfo), m_face(edge)

{
    setTitle(fieldInfo->name());

    cmbBoundary = new QComboBox();
    connect(cmbBoundary, SIGNAL(currentIndexChanged(int)), this, SLOT(doBoundaryChanged(int)));

    btnBoundary = new QPushButton(icon("three-dots"), "");
    btnBoundary->setMaximumSize(btnBoundary->sizeHint());
    connect(btnBoundary, SIGNAL(clicked()), this, SLOT(doBoundaryClicked()));

    QHBoxLayout *layoutBoundary = new QHBoxLayout();
    layoutBoundary->addWidget(cmbBoundary, 1);
    layoutBoundary->addWidget(btnBoundary);

    QFormLayout *layoutBoundaries = new QFormLayout();
    layoutBoundaries->addRow(tr("Boundary condition:"), layoutBoundary);

    setLayout(layoutBoundaries);
}

void SceneFaceMarker::load()
{
    if (m_face->hasMarker(m_fieldInfo))
        cmbBoundary->setCurrentIndex(cmbBoundary->findData(m_face->marker(m_fieldInfo)->variant()));
}

bool SceneFaceMarker::save()
{
    m_face->addMarker(cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>());

    return true;
}

void SceneFaceMarker::fillComboBox()
{
    cmbBoundary->clear();

    // none marker
    cmbBoundary->addItem(m_face->scene()->boundaries->getNone(m_fieldInfo)->name(),
                         m_face->scene()->boundaries->getNone(m_fieldInfo)->variant());

    // real markers
    foreach (SceneBoundary *boundary, m_face->scene()->boundaries->filter(m_fieldInfo).items())
    {
        cmbBoundary->addItem(boundary->name(),
                             boundary->variant());
    }
}

void SceneFaceMarker::doBoundaryChanged(int index)
{
    btnBoundary->setEnabled(cmbBoundary->currentIndex() > 0);
}

void SceneFaceMarker::doBoundaryClicked()
{
    SceneBoundary *marker = cmbBoundary->itemData(cmbBoundary->currentIndex()).value<SceneBoundary *>();
    if (marker->showDialog(this) == QDialog::Accepted)
    {
        cmbBoundary->setItemText(cmbBoundary->currentIndex(), marker->name());
        m_face->scene()->invalidate();
    }
}

SceneFaceDialog::SceneFaceDialog(SceneFace *edge, QWidget *parent, bool isNew) : SceneBasicDialog(parent, isNew)
{
    m_object = edge;

    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edge"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    // setMaximumSize(sizeHint());
}

QLayout* SceneFaceDialog::createContent()
{
    // layout
    QFormLayout *layout = new QFormLayout();

    foreach (FieldInfo *fieldInfo, m_object->scene()->parentProblem()->fieldInfos())
    {
        SceneFaceMarker *sceneEdge = new SceneFaceMarker(dynamic_cast<SceneFace *>(m_object), fieldInfo, this);
        layout->addRow(sceneEdge);

        m_faceMarkers.append(sceneEdge);
    }

    txtAngle = new ValueLineEdit();
    txtAngle->setMinimum(0.0);
    txtAngle->setMaximum(90.0);
    txtSegments = new QSpinBox();
    txtSegments->setMinimum(3);
    txtSegments->setMaximum(20);
    chkIsCurvilinear = new QCheckBox(tr("Curvilinear"));
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(evaluated(bool)));
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(angleChanged()));

    lblLength = new QLabel();

    cmbNodeStart = new QComboBox();
    cmbNodeEnd = new QComboBox();
    connect(cmbNodeStart, SIGNAL(currentIndexChanged(int)), this, SLOT(nodeChanged()));
    connect(cmbNodeEnd, SIGNAL(currentIndexChanged(int)), this, SLOT(nodeChanged()));

    // coordinates
    QFormLayout *layoutEdgeParameters = new QFormLayout();
    layoutEdgeParameters->addRow(tr("Angle (deg.):"), txtAngle);
    layoutEdgeParameters->addRow(tr("Segments (-):"), txtSegments);
    layoutEdgeParameters->addRow(tr(""), chkIsCurvilinear);
    layoutEdgeParameters->addRow(tr("Start node:"), cmbNodeStart);
    layoutEdgeParameters->addRow(tr("End node:"), cmbNodeEnd);
    layoutEdgeParameters->addRow(tr("Length:"), lblLength);

    QGroupBox *grpEdgeParameters = new QGroupBox(tr("Edge parameters"));
    grpEdgeParameters->setLayout(layoutEdgeParameters);

    layout->addRow(grpEdgeParameters);

    fillComboBox();

    QPushButton *btnSwap = new QPushButton();
    btnSwap->setText(tr("Swap direction"));
    connect(btnSwap, SIGNAL(clicked()), this, SLOT(swap()));
    buttonBox->addButton(btnSwap, QDialogButtonBox::ActionRole);

    return layout;
}

void SceneFaceDialog::fillComboBox()
{
    // markers
    foreach (SceneFaceMarker *edgeMarker, m_faceMarkers)
        edgeMarker->fillComboBox();

    // start and end nodes
    cmbNodeStart->blockSignals(true);
    cmbNodeEnd->blockSignals(true);
    cmbNodeStart->clear();
    cmbNodeEnd->clear();
    for (int i = 0; i < m_object->scene()->nodes->count(); i++)
    {
        cmbNodeStart->addItem(QString("%1 - [%2; %3]").
                              arg(i).
                              arg(m_object->scene()->nodes->at(i)->point().x, 0, 'e', 2).
                              arg(m_object->scene()->nodes->at(i)->point().y, 0, 'e', 2),
                              m_object->scene()->nodes->at(i)->variant());
        cmbNodeEnd->addItem(QString("%1 - [%2; %3]").
                            arg(i).
                            arg(m_object->scene()->nodes->at(i)->point().x, 0, 'e', 2).
                            arg(m_object->scene()->nodes->at(i)->point().y, 0, 'e', 2),
                            m_object->scene()->nodes->at(i)->variant());
    }
    cmbNodeStart->blockSignals(false);
    cmbNodeEnd->blockSignals(false);
}

bool SceneFaceDialog::load()
{
    SceneFace *sceneEdge = dynamic_cast<SceneFace *>(m_object);

    cmbNodeStart->blockSignals(true);
    cmbNodeEnd->blockSignals(true);

    cmbNodeStart->setCurrentIndex(cmbNodeStart->findData(sceneEdge->nodeStart()->variant()));
    cmbNodeEnd->setCurrentIndex(cmbNodeEnd->findData(sceneEdge->nodeEnd()->variant()));
    txtAngle->setValue(sceneEdge->angleValue());
    connect(txtAngle, SIGNAL(evaluated(bool)), this, SLOT(angleChanged()));
    txtSegments->setValue(sceneEdge->segments());
    chkIsCurvilinear->setChecked(sceneEdge->isCurvilinear());

    foreach (SceneFaceMarker *edgeMarker, m_faceMarkers)
        edgeMarker->load();

    nodeChanged();

    cmbNodeStart->blockSignals(false);
    cmbNodeEnd->blockSignals(false);

    return true;
}

bool SceneFaceDialog::save()
{
    if (!txtAngle->evaluate(false)) return false;

    SceneFace *sceneEdge = dynamic_cast<SceneFace *>(m_object);

    if (cmbNodeStart->currentIndex() == cmbNodeEnd->currentIndex())
    {
        QMessageBox::warning(this, tr("Edge"), tr("Start and end node are same."));
        return false;
    }

    // check if edge doesn't exists
    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    SceneFace *edgeCheck = m_object->scene()->getFace(nodeStart->point(), nodeEnd->point());
    if ((edgeCheck) && ((sceneEdge != edgeCheck) || m_isNew))
    {
        QMessageBox::warning(this, tr("Edge"), tr("Edge already exists."));
        return false;
    }

    if (!m_isNew)
    {
        m_object->scene()->undoStack()->push(new SceneEdgeCommandEdit(sceneEdge->nodeStart()->point(), sceneEdge->nodeEnd()->point(),
                                                                     nodeStart->point(), nodeEnd->point(),
                                                                     sceneEdge->angleValue(),
                                                                     txtAngle->value(),
                                                                     sceneEdge->segments(),
                                                                     txtSegments->value(),
                                                                     sceneEdge->isCurvilinear(),
                                                                     chkIsCurvilinear->checkState() == Qt::Checked));
    }

    sceneEdge->setNodeStart(nodeStart);
    sceneEdge->setNodeEnd(nodeEnd);
    sceneEdge->setAngleValue(txtAngle->value());
    sceneEdge->setSegments(txtSegments->value());
    sceneEdge->setCurvilinear(chkIsCurvilinear->checkState() == Qt::Checked);

    foreach (SceneFaceMarker *edgeMarker, m_faceMarkers)
        edgeMarker->save();

    m_object->scene()->invalidate();
    return true;
}

void SceneFaceDialog::nodeChanged()
{
    SceneNode *nodeStart = dynamic_cast<SceneNode *>(cmbNodeStart->itemData(cmbNodeStart->currentIndex()).value<SceneBasic *>());
    SceneNode *nodeEnd = dynamic_cast<SceneNode *>(cmbNodeEnd->itemData(cmbNodeEnd->currentIndex()).value<SceneBasic *>());

    if (nodeStart && nodeEnd)
    {
        SceneFace *sceneEdge = dynamic_cast<SceneFace *>(m_object);

        SceneFace *edgeCheck = m_object->scene()->getFace(nodeStart->point(), nodeEnd->point());
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!((nodeStart == nodeEnd) || ((edgeCheck) && ((sceneEdge != edgeCheck) || m_isNew))));

        SceneFace edge(m_object->scene(), nodeStart, nodeEnd, txtAngle->number(), txtSegments->value(), chkIsCurvilinear->checkState() == Qt::Checked);
        lblLength->setText(QString("%1 m").arg(edge.length()));
    }
}

void SceneFaceDialog::swap()
{
    // swap nodes
    int startIndex = cmbNodeStart->currentIndex();
    cmbNodeStart->setCurrentIndex(cmbNodeEnd->currentIndex());
    cmbNodeEnd->setCurrentIndex(startIndex);
}

void SceneFaceDialog::angleChanged()
{
    txtSegments->setEnabled(txtAngle->value().isEvaluated() && txtAngle->value().number() > 0.0);
    chkIsCurvilinear->setEnabled(txtAngle->value().isEvaluated() && txtAngle->value().number() > 0.0);
}

SceneEdgeSelectDialog::SceneEdgeSelectDialog(MarkedSceneBasicContainer<SceneBoundary, SceneFace> edges, QWidget *parent)
    : QDialog(parent), m_edges(edges)
{
    setWindowIcon(icon("scene-edge"));
    setWindowTitle(tr("Edges"));

    // markers
    QFormLayout *layoutBoundaries = new QFormLayout();

    QGroupBox *grpBoundaries = new QGroupBox(tr("Boundary conditions"));
    grpBoundaries->setLayout(layoutBoundaries);

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        QComboBox *cmbBoundary = new QComboBox();
        cmbBoundaries[fieldInfo] = cmbBoundary;

        layoutBoundaries->addRow(fieldInfo->name(), cmbBoundary);
    }

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpBoundaries);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    load();

    setMinimumSize(sizeHint());
}

void SceneEdgeSelectDialog::load()
{
    // markers
    foreach (FieldInfo *fieldInfo, cmbBoundaries.keys())
    {
        cmbBoundaries[fieldInfo]->clear();

        // none marker
        cmbBoundaries[fieldInfo]->addItem(Agros2D::problem()->scene()->boundaries->getNone(fieldInfo)->name(),
                                          Agros2D::problem()->scene()->boundaries->getNone(fieldInfo)->variant());

        // real markers
        foreach (SceneBoundary *boundary, Agros2D::problem()->scene()->boundaries->filter(fieldInfo).items())
            cmbBoundaries[fieldInfo]->addItem(boundary->name(),
                                              boundary->variant());
    }

    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        SceneBoundary* boundary = NULL;
        bool match = true;
        foreach (SceneFace *edge, m_edges.items())
        {
            if (boundary)
                match = match && (boundary == edge->marker(fieldInfo));
            else
                boundary = edge->marker(fieldInfo);
        }
        if (match)
            cmbBoundaries[fieldInfo]->setCurrentIndex(cmbBoundaries[fieldInfo]->findData(boundary->variant()));
        else
            cmbBoundaries[fieldInfo]->setCurrentIndex(-1);
    }
}

bool SceneEdgeSelectDialog::save()
{
    foreach (SceneFace* edge, m_edges.items())
    {
        foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
        {
            if (cmbBoundaries[fieldInfo]->currentIndex() != -1)
                edge->addMarker(cmbBoundaries[fieldInfo]->itemData(cmbBoundaries[fieldInfo]->currentIndex()).value<SceneBoundary *>());

        }
    }

    Agros2D::problem()->scene()->invalidate();
    return true;
}

void SceneEdgeSelectDialog::doAccept()
{
    if (save())
        accept();
}

void SceneEdgeSelectDialog::doReject()
{
    reject();
}

// **********************************************************************************************************************************

SceneFaceCommandAdd::SceneFaceCommandAdd(const Point &pointStart, const Point &pointEnd, const QMap<QString, QString> &markers,
                                         const Value &angle, int segments, bool isCurvilinear, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markers = markers;
    m_angle = angle;
    m_segments = segments;
    m_isCurvilinear = isCurvilinear;
}

void SceneFaceCommandAdd::undo()
{
    Agros2D::problem()->scene()->faces->remove(Agros2D::problem()->scene()->getFace(m_pointStart, m_pointEnd, m_angle.number(), m_segments, m_isCurvilinear));
    Agros2D::problem()->scene()->invalidate();
}

void SceneFaceCommandAdd::redo()
{
    // new edge
    SceneNode *nodeStart = new SceneNode(Agros2D::problem()->scene(), m_pointStart);
    nodeStart = Agros2D::problem()->scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Agros2D::problem()->scene(), m_pointEnd);
    nodeEnd = Agros2D::problem()->scene()->addNode(nodeEnd);
    SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_angle, m_segments, m_isCurvilinear);

    edge->addMarkersFromStrings(m_markers);

    // add edge to the list
    Agros2D::problem()->scene()->addFace(edge);
    Agros2D::problem()->scene()->invalidate();
}

SceneEdgeCommandAddOrRemoveMulti::SceneEdgeCommandAddOrRemoveMulti(QList<Point> pointStarts, QList<Point> pointEnds,
                                                                   QList<Value> angles, QList<int> segments, QList<bool> isCurvilinear, QList<QMap<QString, QString> > markers, QUndoCommand *parent) : QUndoCommand(parent)
{
    assert(pointStarts.size() == pointEnds.size());
    assert(pointStarts.size() == angles.size());
    m_pointStarts = pointStarts;
    m_pointEnds = pointEnds;
    m_angles = angles;
    m_segments = segments;
    m_isCurvilinear = isCurvilinear;
    m_markers = markers;
}

void SceneEdgeCommandAddOrRemoveMulti::remove()
{
    Agros2D::problem()->scene()->stopInvalidating(true);

    for(int i = 0; i < m_pointStarts.size(); i++)
    {
        Agros2D::problem()->scene()->faces->remove(Agros2D::problem()->scene()->getFace(m_pointStarts[i], m_pointEnds[i], m_angles[i].number(), m_segments[i], m_isCurvilinear[i]));
    }

    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

void SceneEdgeCommandAddOrRemoveMulti::add()
{
    Agros2D::problem()->scene()->stopInvalidating(true);
    for(int i = 0; i < m_pointStarts.size(); i++)
    {
        SceneNode *nodeStart = Agros2D::problem()->scene()->getNode(m_pointStarts[i]);
        SceneNode *nodeEnd = Agros2D::problem()->scene()->getNode(m_pointEnds[i]);
        assert(nodeStart && nodeEnd);
        if(nodeStart && nodeEnd)
        {
            SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_angles[i], m_segments[i], m_isCurvilinear[i]);

            // if markers are not empty, we were deleting or copying "withMarkers = True"
            if(!m_markers.empty())
            {
                edge->addMarkersFromStrings(m_markers[i]);
            }
            // add edge to the list
            Agros2D::problem()->scene()->addFace(edge);
        }
    }
    Agros2D::problem()->scene()->stopInvalidating(false);
    Agros2D::problem()->scene()->invalidate();
}

SceneFaceCommandRemove::SceneFaceCommandRemove(const Point &pointStart, const Point &pointEnd, const QMap<QString, QString> &markers,
                                               const Value &angle, int segments, bool isCurvilinear, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_markers = markers;
    m_angle = angle;
    m_segments = segments;
    m_isCurvilinear = isCurvilinear;
}

void SceneFaceCommandRemove::undo()
{
    // new edge
    SceneNode *nodeStart = new SceneNode(Agros2D::problem()->scene(), m_pointStart);
    nodeStart = Agros2D::problem()->scene()->addNode(nodeStart);
    SceneNode *nodeEnd = new SceneNode(Agros2D::problem()->scene(), m_pointEnd);
    nodeEnd = Agros2D::problem()->scene()->addNode(nodeEnd);
    SceneFace *edge = new SceneFace(Agros2D::problem()->scene(), nodeStart, nodeEnd, m_angle, m_segments, m_isCurvilinear);

    edge->addMarkersFromStrings(m_markers);

    // add edge to the list
    Agros2D::problem()->scene()->addFace(edge);
    Agros2D::problem()->scene()->invalidate();
}

void SceneFaceCommandRemove::redo()
{
    Agros2D::problem()->scene()->faces->remove(Agros2D::problem()->scene()->getFace(m_pointStart, m_pointEnd, m_angle.number(), m_segments, m_isCurvilinear));
    Agros2D::problem()->scene()->invalidate();
}

SceneEdgeCommandEdit::SceneEdgeCommandEdit(const Point &pointStart, const Point &pointEnd, const Point &pointStartNew, const Point &pointEndNew,
                                           const Value &angle, const Value &angleNew, int segments, int segmentsNew, bool isCurvilinear, bool isCurvilinearNew, QUndoCommand *parent) : QUndoCommand(parent)
{
    m_pointStart = pointStart;
    m_pointEnd = pointEnd;
    m_pointStartNew = pointStartNew;
    m_pointEndNew = pointEndNew;
    m_angle = angle;
    m_angleNew = angleNew;
    m_segments = segments;
    m_segmentsNew = segmentsNew;
    m_isCurvilinear = isCurvilinear;
    m_isCurvilinearNew = isCurvilinearNew;
}

void SceneEdgeCommandEdit::undo()
{
    SceneFace *edge = Agros2D::problem()->scene()->getFace(m_pointStartNew, m_pointEndNew, m_angleNew.number(), m_segmentsNew, m_isCurvilinearNew);
    if (edge)
    {
        edge->setNodeStart(Agros2D::problem()->scene()->getNode(m_pointStart));
        edge->setNodeEnd(Agros2D::problem()->scene()->getNode(m_pointEnd));
        edge->setAngleValue(m_angle);
        edge->setSegments(m_segments);
        Agros2D::problem()->scene()->invalidate();
    }
}

void SceneEdgeCommandEdit::redo()
{
    SceneFace *edge = Agros2D::problem()->scene()->getFace(m_pointStart, m_pointEnd, m_angle.number(), m_segments, m_isCurvilinear);
    if (edge)
    {
        edge->setNodeStart(Agros2D::problem()->scene()->getNode(m_pointStartNew));
        edge->setNodeEnd(Agros2D::problem()->scene()->getNode(m_pointEndNew));
        edge->setAngleValue(m_angleNew);
        edge->setSegments(m_segmentsNew);
        Agros2D::problem()->scene()->invalidate();
    }
}


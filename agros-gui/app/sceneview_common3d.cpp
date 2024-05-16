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

#include "sceneview_common3d.h"

#include "util/global.h"

#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/module.h"

#include "solver/problem.h"
#include "solver/problem_config.h"

SceneViewCommon3D::SceneViewCommon3D(QWidget *parent)
    : SceneViewCommon(parent)
{
    createActions();
}

SceneViewCommon3D::~SceneViewCommon3D()
{
}

void SceneViewCommon3D::createActions()
{
    // projection
    actSetProjectionXY = new QAction(this);
    connect(actSetProjectionXY, SIGNAL(triggered()), this, SLOT(doSetProjectionXY()));

    actSetProjectionXZ = new QAction(this);
    connect(actSetProjectionXZ, SIGNAL(triggered()), this, SLOT(doSetProjectionXZ()));

    actSetProjectionYZ = new QAction(this);
    connect(actSetProjectionYZ, SIGNAL(triggered()), this, SLOT(doSetProjectionYZ()));    
}

void SceneViewCommon3D::clear()
{
    // 3d
    m_scale3d = 0.6;
    m_offset3d = Point();
    m_rotation3d = Point3();
    m_rotation3d.x = 66.0;
    m_rotation3d.y = -35.0;
    m_rotation3d.z = 0.0;
}

void SceneViewCommon3D::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO)
        return;

    double sceneWidth = end.x - start.x;
    double sceneHeight = end.y - start.y;

    double maxScene = ((width() / height()) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale3d = 0.9/maxScene;

    m_offset3d.x = 0.0;
    m_offset3d.y = 0.0;

    setZoom(0);
}

void SceneViewCommon3D::paintAxes()
{
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 2.0 / height());
    glTranslated(- width() / 2.0 + 30, -height() / 2.0 + 30, 0.0);

    glRotated(m_rotation3d.x, 1.0, 0.0, 0.0);
    glRotated(m_rotation3d.z, 0.0, 1.0, 0.0);
    glRotated(m_rotation3d.y, 0.0, 0.0, 1.0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // x-axis
    glColor3d(0.9, 0.0, 0.0);

    glBegin(GL_QUADS);
    glVertex3d(0, -1, 0);
    glVertex3d(16, -1, 0);
    glVertex3d(16, 1, 0);
    glVertex3d(0, 1, 0);

    glVertex3d(0, 0, -1);
    glVertex3d(16, 0, -1);
    glVertex3d(16, 0, 1);
    glVertex3d(0, 0, 1);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex3d(16, -5, 0);
    glVertex3d(16, 5, 0);
    glVertex3d(35, 0, 0);

    glVertex3d(16, 0, -5);
    glVertex3d(16, 0, 5);
    glVertex3d(35, 0, 0);
    glEnd();

    // y-axis
    glColor3d(0.0, 0.9, 0.0);

    glBegin(GL_QUADS);
    glVertex3d(-1, 0, 0);
    glVertex3d(-1, 16, 0);
    glVertex3d(1, 16, 0);
    glVertex3d(1, 0, 0);

    glVertex3d(0, 0, -1);
    glVertex3d(0, 16, -1);
    glVertex3d(0, 16, 1);
    glVertex3d(0, 0, 1);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex3d(-5, 16, 0);
    glVertex3d(5, 16, 0);
    glVertex3d(0, 35, 0);

    glVertex3d(0, 16, -5);
    glVertex3d(0, 16, 5);
    glVertex3d(0, 35, 0);
    glEnd();

    // z-axis
    glColor3d(0.0, 0.0, 0.9);

    glBegin(GL_QUADS);
    glVertex3d(-1, 0, 0);
    glVertex3d(-1, 0, -16);
    glVertex3d(1, 0, -16);
    glVertex3d(1, 0, 0);

    glVertex3d(0, -1, 0);
    glVertex3d(0, -1, -16);
    glVertex3d(0, 1, -16);
    glVertex3d(0, 1, 0);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex3d(-5, 0, -16);
    glVertex3d(5, 0, -16);
    glVertex3d(0, 0, -35);

    glVertex3d(0, -5, -16);
    glVertex3d(0, 5, -16);
    glVertex3d(0, 0, -35);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneViewCommon3D::loadProjection3d(bool setScene, bool plane)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 0.5, -0.5, 0.5, 4.0, 15.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (setScene)
    {
        // set max and min zoom
        if (m_scale3d < 1e-9) m_scale3d = 1e-9;
        if (m_scale3d > 1e6) m_scale3d = 1e6;

        glScaled(1.0/aspect(), 1.0, 0.1);

        // move to origin
        glTranslated(-m_offset3d.x, -m_offset3d.y, 1.0);

        glRotated(m_rotation3d.x, 1.0, 0.0, 0.0);
        glRotated(m_rotation3d.z, 0.0, 1.0, 0.0);
        glRotated(m_rotation3d.y, 0.0, 0.0, 1.0);

        RectPoint rect = problem()->scene()->boundingBox();
        if (plane)
        {
            glTranslated(- m_scale3d * (rect.start.x + rect.end.x) / 2.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
        }
        else
        {
            if (problem()->config()->coordinateType() == CoordinateType_Planar)
            {
                glTranslated(- m_scale3d * (rect.start.x + rect.end.x) / 2.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
            }
            else
            {
                glTranslated(0.0, - m_scale3d * (rect.start.y + rect.end.y) / 2.0, 0.0);
            }
        }

        glScaled(m_scale3d, m_scale3d, m_scale3d);
    }
}

void SceneViewCommon3D::setZoom(double power)
{
    m_scale3d = m_scale3d * pow(1.2, power);

    update();
}

// events

void SceneViewCommon3D::keyPressEvent(QKeyEvent *event)
{
}

void SceneViewCommon3D::keyReleaseEvent(QKeyEvent *event)
{
    setToolTip("");

    QWidget::keyReleaseEvent(event);
    emit mouseSceneModeChanged(MouseSceneMode_Nothing);
}

void SceneViewCommon3D::mousePressEvent(QMouseEvent *event)
{
}

void SceneViewCommon3D::mouseReleaseEvent(QMouseEvent *event)
{
}

void SceneViewCommon3D::mouseDoubleClickEvent(QMouseEvent * event)
{
}

void SceneViewCommon3D::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    setToolTip("");

    // pan
    if ((event->buttons() & Qt::MiddleButton)
            || ((event->buttons() & Qt::LeftButton)
                && (((event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))))

    {
        setCursor(Qt::PointingHandCursor);

        m_offset3d.x -= 2.0/width() * dx*aspect();
        m_offset3d.y += 2.0/height() * dy;

        emit mouseSceneModeChanged(MouseSceneMode_Pan);

        update();
    }

    // rotate
    if ((event->buttons() & Qt::LeftButton)
            && (!(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        m_rotation3d.x -= dy;
        m_rotation3d.y += dx;

        emit mouseSceneModeChanged(MouseSceneMode_Rotate);

        update();
    }
    if ((event->buttons() & Qt::LeftButton)
            && (!(event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        m_rotation3d.z -= dy;

        emit mouseSceneModeChanged(MouseSceneMode_Rotate);

        update();
    }
}

void SceneViewCommon3D::wheelEvent(QWheelEvent *event)
{
    setZoom(event->angleDelta().y()/150.0);
}

void SceneViewCommon3D::contextMenuEvent(QContextMenuEvent *event)
{
    actSetProjectionXY->setText(tr("Projection to %1%2").arg(problem()->config()->labelX()).arg(problem()->config()->labelY()));
    actSetProjectionXZ->setText(tr("Projection to %1%2").arg(problem()->config()->labelX()).arg(problem()->config()->labelZ()));
    actSetProjectionYZ->setText(tr("Projection to %1%2").arg(problem()->config()->labelY()).arg(problem()->config()->labelZ()));

    QMenu *mnuView = new QMenu(tr("View"), this);

    mnuView->addAction(actSetProjectionXY);
    mnuView->addAction(actSetProjectionXZ);
    mnuView->addAction(actSetProjectionYZ);

    QMenu *mnuView3D = new QMenu(this);
    mnuView3D->addMenu(mnuView);

    mnuView3D->exec(event->globalPos());
}

void SceneViewCommon3D::doSetProjectionXY()
{
    m_rotation3d.x = m_rotation3d.y = m_rotation3d.z = 0.0;
    update();
}

void SceneViewCommon3D::doSetProjectionXZ()
{
    m_rotation3d.y = m_rotation3d.z = 0.0;
    m_rotation3d.x = 90.0;
    update();
}

void SceneViewCommon3D::doSetProjectionYZ()
{
    m_rotation3d.x = m_rotation3d.y = 90.0;
    m_rotation3d.z = 0.0;
    update();
}

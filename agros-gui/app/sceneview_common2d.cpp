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

#include "sceneview_common2d.h"

#include "util/global.h"
#include "util/constants.h"

#include "scene.h"

#include "solver/problem.h"
#include "solver/problem_config.h"


SceneViewCommon2D::SceneViewCommon2D(QWidget *parent) : SceneViewCommon(parent)
{
    clear();
}

SceneViewCommon2D::~SceneViewCommon2D()
{
}

void SceneViewCommon2D::clear()
{
    m_zoomRegion = false;

    // 2d
    m_scale2d = 1.0;
    m_offset2d = Point();

    m_nodeLast = NULL;
}

Point SceneViewCommon2D::transform(const Point &point) const
{
    return Point((2.0 / width() * point.x - 1) / m_scale2d*aspect() + m_offset2d.x,
                 - (2.0 / height() * point.y - 1) / m_scale2d + m_offset2d.y);
}

Point SceneViewCommon2D::untransform(const Point &point) const
{
    return Point((1.0 + (point.x - m_offset2d.x) * m_scale2d/aspect()) * width() / 2.0,
                 (1.0 + (point.y - m_offset2d.y) * m_scale2d) * height() / 2.0);
}

void SceneViewCommon2D::loadProjection2d(bool setScene)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (setScene)
    {
        // set max and min zoom
        if (m_scale2d < 1e-9) m_scale2d = 1e-9;
        if (m_scale2d > 1e6) m_scale2d = 1e6;

        glScaled(m_scale2d/aspect(), m_scale2d, 1.0);
        glTranslated(-m_offset2d.x, -m_offset2d.y, 0.0);
    }
}

void SceneViewCommon2D::paintGrid()
{
    if (!problem())
        return;

    loadProjection2d(true);

    Point cornerMin = transform(Point(0, 0));
    Point cornerMax = transform(Point(width(), height()));

    glDisable(GL_DEPTH_TEST);

    // heavy line
    int heavyLine = 5;

    glLineWidth(1.0);
    glBegin(GL_LINES);

    if ((((cornerMax.x-cornerMin.x)/problem()->config()->value(ProblemConfig::GridStep).toDouble() + (cornerMin.y-cornerMax.y)/problem()->config()->value(ProblemConfig::GridStep).toDouble()) < 200) &&
            ((cornerMax.x-cornerMin.x)/problem()->config()->value(ProblemConfig::GridStep).toDouble() > 0) && ((cornerMin.y-cornerMax.y)/problem()->config()->value(ProblemConfig::GridStep).toDouble() > 0))
    {
        // vertical lines
        for (int i = cornerMin.x/problem()->config()->value(ProblemConfig::GridStep).toDouble() - 1; i < cornerMax.x/problem()->config()->value(ProblemConfig::GridStep).toDouble() + 1; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(COLORGRID[0] * 5.0/6.0, COLORGRID[1] * 5.0/6.0, COLORGRID[2] * 5.0/6.0);
            else
                glColor3d(COLORGRID[0], COLORGRID[1], COLORGRID[2]);

            glVertex2d(i*problem()->config()->value(ProblemConfig::GridStep).toDouble(), cornerMin.y);
            glVertex2d(i*problem()->config()->value(ProblemConfig::GridStep).toDouble(), cornerMax.y);
        }

        // horizontal lines
        for (int i = cornerMax.y/problem()->config()->value(ProblemConfig::GridStep).toDouble() - 1; i < cornerMin.y/problem()->config()->value(ProblemConfig::GridStep).toDouble() + 1; i++)
        {
            if (i % heavyLine == 0)
                glColor3d(COLORGRID[0] * 5.0/6.0, COLORGRID[1] * 5.0/6.0, COLORGRID[2] * 5.0/6.0);
            else
                glColor3d(COLORGRID[0], COLORGRID[1], COLORGRID[2]);

            glVertex2d(cornerMin.x, i*problem()->config()->value(ProblemConfig::GridStep).toDouble());
            glVertex2d(cornerMax.x, i*problem()->config()->value(ProblemConfig::GridStep).toDouble());
        }
    }
    glEnd();

    if (problem()->config()->coordinateType() == CoordinateType_Axisymmetric)
    {
        drawBlend(cornerMin,
                  Point(0, cornerMax.y),
                  COLORGRID[0] * 2.0/3.0, COLORGRID[1] * 2.0/3.0, COLORGRID[2] * 2.0/3.0, 0.25);
    }

    glLineWidth(1.5);
    glBegin(GL_LINES);
    // y axis
    glVertex2d(0, cornerMin.y);
    glVertex2d(0, cornerMax.y);
    // x axis
    glVertex2d(((problem()->config()->coordinateType() == CoordinateType_Axisymmetric) ? 0 : cornerMin.x), 0);
    glVertex2d(cornerMax.x, 0);
    glEnd();
}

void SceneViewCommon2D::paintAxes()
{
    if (!problem())
        return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glColor3d(COLORCROSS[0], COLORCROSS[1], COLORCROSS[2]);

    Point rulersArea = rulersAreaSize();
    Point border = (Agros::configComputer()->value(Config::Config_ShowRulers).toBool())
            ? Point(rulersArea.x + 10.0, rulersArea.y + 10.0) : Point(10.0, 10.0);

    // x-axis
    glBegin(GL_QUADS);
    glVertex2d(border.x, border.y);
    glVertex2d(border.x + 16, border.y);
    glVertex2d(border.x + 16, border.y + 2);
    glVertex2d(border.x, border.y + 2);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(border.x + 16, border.y - 4);
    glVertex2d(border.x + 16, border.y + 6);
    glVertex2d(border.x + 35, border.y + 1);
    glEnd();

    // y-axis
    glBegin(GL_QUADS);
    glVertex2d(border.x, border.y);
    glVertex2d(border.x, border.y + 16);
    glVertex2d(border.x + 2, border.y + 16);
    glVertex2d(border.x + 2, border.y);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2d(border.x - 4, border.y + 16);
    glVertex2d(border.x + 6, border.y + 16);
    glVertex2d(border.x + 1, border.y + 35);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    const int labelRulersSize = Agros::configComputer()->value(Config::Config_RulersFontPointSize).toInt();
    printRulersAt(border.x + 38, border.y + 1 - 0.5*labelRulersSize, problem()->config()->labelX());
    printRulersAt(border.x + 1 - 0.5*labelRulersSize, border.y + 38, problem()->config()->labelY());
}

void SceneViewCommon2D::paintRulers()
{
    if (!problem())
        return;

    loadProjection2d(true);

    Point cornerMin = transform(Point(0, 0));
    Point cornerMax = transform(Point(width(), height()));

    const int labelRulersSize = Agros::configComputer()->value(Config::Config_RulersFontPointSize).toInt();
    double gridStep = problem()->config()->value(ProblemConfig::GridStep).toDouble();
    if (gridStep < EPS_ZERO)
        return;

    while (((cornerMax.x-cornerMin.x)/gridStep + (cornerMin.y-cornerMax.y)/gridStep) > 200)
        gridStep *= 2.0;
    while (((cornerMax.x-cornerMin.x)/gridStep + (cornerMin.y-cornerMax.y)/gridStep) < 80)
        gridStep /= 2.0;

    if (((cornerMax.x-cornerMin.x)/gridStep > 0) && ((cornerMin.y-cornerMax.y)/gridStep > 0))
    {
        int heavyLine = 5;

        // labels
        Point rulersAreaScreen = rulersAreaSize();
        Point rulersArea(2.0/width()*rulersAreaScreen.x/m_scale2d*aspect(),
                         2.0/height()*rulersAreaScreen.y/m_scale2d);

        double tickSize = rulersArea.y / 3.0;

        // area background
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glColor3d(m_windowColor[0], m_windowColor[1], m_windowColor[2]);

        glBegin(GL_QUADS);
        glVertex2d(cornerMin.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMax.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMax.x, cornerMax.y);
        glVertex2d(cornerMin.x, cornerMax.y);

        glVertex2d(cornerMin.x + rulersArea.x, cornerMax.y);
        glVertex2d(cornerMin.x, cornerMax.y);
        glVertex2d(cornerMin.x, cornerMin.y);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMin.y);
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);

        // area lines
        glColor3d(0.5, 0.5, 0.5);
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMax.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMax.y + rulersArea.y);
        glVertex2d(cornerMin.x + rulersArea.x, cornerMin.y);
        glEnd();

        // lines
        glLineWidth(1.0);
        glBegin(GL_LINES);

        // horizontal ticks
        for (int i = cornerMin.x/gridStep - 1; i < cornerMax.x/gridStep + 1; i++)
        {
            if ((i*gridStep > cornerMin.x + rulersArea.x) && (i*gridStep < cornerMax.x))
            {
                if (i % heavyLine == 0)
                {
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y);
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y - 2.0 * tickSize);
                }
                else
                {
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y);
                    glVertex2d(i*gridStep, cornerMax.y + rulersArea.y - tickSize);
                }
            }
        }

        // vertical ticks
        for (int i = cornerMax.y/gridStep - 1; i < cornerMin.y/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMax.y + rulersArea.y) || (i*gridStep > cornerMin.y))
                continue;

            if (i % heavyLine == 0)
            {
                glVertex2d(cornerMin.x + rulersArea.x - 2.0 * tickSize, i*gridStep);
                glVertex2d(cornerMin.x + rulersArea.x, i*gridStep);
            }
            else
            {
                glVertex2d(cornerMin.x + rulersArea.x - tickSize, i*gridStep);
                glVertex2d(cornerMin.x + rulersArea.x, i*gridStep);
            }
        }
        glEnd();

        // zero axes
        glColor3d(COLORGRID[0], COLORGRID[1], COLORGRID[2]);

        glLineWidth(1.5);
        glBegin(GL_LINES);

        glVertex2d(0.0, cornerMax.y + rulersArea.y);
        glVertex2d(0.0, cornerMax.y + rulersArea.y - 2.0 * tickSize);

        glVertex2d(cornerMin.x + rulersArea.x - tickSize, 0.0);
        glVertex2d(cornerMin.x + rulersArea.x, 0.0);

        glEnd();

        // labels
        loadProjectionViewPort();

        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

        // horizontal labels
        for (int i = cornerMin.x/gridStep - 1; i < cornerMax.x/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMin.x + rulersArea.x) || (i*gridStep > cornerMax.x))
                continue;

            if (i % heavyLine == 0)
            {
                QString text;
                if ((abs(gridStep) > 1e3 || abs(gridStep) < 1e-3) && i != 0)
                    text = QString::number(i*gridStep, 'e', 2);
                else
                    text = QString::number(i*gridStep, 'f', 6);

                Point scr = untransform(i*gridStep, cornerMax.y);
                printRulersAt(scr.x + 1.2*labelRulersSize,
                              scr.y + 2, QString(text + "        ").left(9));
            }
        }

        // vertical labels
        for (int i = cornerMax.y/gridStep - 1; i < cornerMin.y/gridStep + 1; i++)
        {
            if ((i*gridStep < cornerMax.y + rulersArea.y) || (i*gridStep > cornerMin.y))
                continue;

            if (i % heavyLine == 0)
            {
                QString text;
                if ((abs(gridStep) > 1e3 || abs(gridStep) < 1e-3) && i != 0)
                    text = QString::number(i*gridStep, 'e', 2);
                else
                    text = QString::number(i*gridStep, 'f', 7);

                Point scr = untransform(cornerMin.x + rulersArea.x / 20.0, i*gridStep);
                printRulersAt(scr.x, scr.y - 1.6*labelRulersSize,
                              QString(((i >= 0) ? " " : "") + text + "        ").left(9));
            }
        }
    }

    // paint axes
    paintAxes();
}

void SceneViewCommon2D::paintRulersHints()
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
}

void SceneViewCommon2D::paintZoomRegion()
{
    // zoom region
    if (m_zoomRegion)
    {
        loadProjection2d(true);

        Point posStart = transform(Point(m_zoomRegionPos.x(), m_zoomRegionPos.y()));
        Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

        drawBlend(posStart, posEnd,
                  COLORHIGHLIGHTED[0], COLORHIGHLIGHTED[1], COLORHIGHLIGHTED[2]);
    }
}

// events

void SceneViewCommon2D::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() & Qt::ControlModifier) && !(event->modifiers() & Qt::ShiftModifier))
        Q_EMIT mouseSceneModeChanged(MouseSceneMode_Add);
    if (!(event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
        Q_EMIT mouseSceneModeChanged(MouseSceneMode_Pan);
    if ((event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier))
        Q_EMIT mouseSceneModeChanged(MouseSceneMode_Move);

    Point stepTemp = transform(Point(width(), height()));
    stepTemp.x = stepTemp.x - m_offset2d.x;
    stepTemp.y = stepTemp.y - m_offset2d.y;
    double step = qMin(stepTemp.x, stepTemp.y) / 10.0;

    switch (event->key())
    {
    case Qt::Key_Up:
    {
        m_offset2d.y += step;
        update();
    }
        break;
    case Qt::Key_Down:
    {
        m_offset2d.y -= step;
        update();
    }
        break;
    case Qt::Key_Left:
    {
        m_offset2d.x -= step;
        update();
    }
        break;
    case Qt::Key_Right:
    {
        m_offset2d.x += step;
        update();
    }
        break;
    case Qt::Key_Plus:
    {
        doZoomIn();
    }
        break;
    case Qt::Key_Minus:
    {
        doZoomOut();
    }
        break;
    case Qt::Key_Escape:
    {
        m_nodeLast = NULL;
        if (problem())
        {
            problem()->scene()->selectNone();
            Q_EMIT mousePressed();
            update();
        }
    }
        break;
    case Qt::Key_N:
    {
        // add node with coordinates under mouse pointer
        if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
        {
            if (problem())
            {
                Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));
                assert(0);
                // problem()->scene()->doNewNode(p);
            }
        }
    }
        break;
    case Qt::Key_L:
    {
        // add label with coordinates under mouse pointer
        if ((event->modifiers() & Qt::ShiftModifier) && (event->modifiers() & Qt::ControlModifier))
        {
            if (problem())
            {
                Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));
                assert(0);
                // problem()->scene()->doNewLabel(p);
            }
        }
    }
        break;
    default:
        ; //
    }

    QWidget::keyPressEvent(event);
}

void SceneViewCommon2D::keyReleaseEvent(QKeyEvent *event)
{
    setToolTip("");

    if (!(event->modifiers() & Qt::ControlModifier))
    {
        m_nodeLast = NULL;
        update();
    }
    QWidget::keyReleaseEvent(event);

    Q_EMIT mouseSceneModeChanged(MouseSceneMode_Nothing);
}

// rulers
Point SceneViewCommon2D::rulersAreaSize() const
{
    const int labelRulersSize = Agros::configComputer()->value(Config::Config_RulersFontPointSize).toInt();
    return Point(0.8*labelRulersSize * 11, 1.0*labelRulersSize * 3);
}

void SceneViewCommon2D::setZoom(double power)
{
    m_scale2d = m_scale2d * pow(1.2, power);

    update();
}

void SceneViewCommon2D::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO)
        return;

    Point rulersAreaScreen = rulersAreaSize();

    double sceneWidth = end.x - start.x;
    double sceneHeight = end.y - start.y;

    auto showRulers = Agros::configComputer()->value(Config::Config_ShowRulers).toBool();

    double w = (showRulers) ? width() - rulersAreaScreen.x : width();
    double h = (showRulers) ? height() - rulersAreaScreen.y : height();
    double maxScene = ((w / h) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale2d = 1.8/maxScene;

    Point rulersArea(2.0/width()*rulersAreaScreen.x/m_scale2d*aspect(),
                     2.0/height()*rulersAreaScreen.y/m_scale2d);

    m_offset2d.x = ((showRulers) ? start.x + end.x - rulersArea.x : start.x + end.x) / 2.0;
    m_offset2d.y = ((showRulers) ? start.y + end.y - rulersArea.y : start.y + end.y) / 2.0;

    setZoom(0);
}

void SceneViewCommon2D::mousePressEvent(QMouseEvent *event)
{
    // zoom region
    if ((event->button() & Qt::LeftButton)
            && !(event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier))
    {
        // zoom region
        if (actSceneZoomRegion)
        {
            if (actSceneZoomRegion->isChecked())
            {
                m_zoomRegionPos = m_lastPos;
                actSceneZoomRegion->setChecked(false);
                m_zoomRegion = true;

                return;
            }
        }
    }
}

void SceneViewCommon2D::mouseDoubleClickEvent(QMouseEvent * event)
{

    if (!(event->modifiers() & Qt::ControlModifier))
    {
        // zoom best fit
        if ((event->buttons() & Qt::MiddleButton)
                || ((event->buttons() & Qt::LeftButton)
                    && ((!(event->modifiers() & Qt::ControlModifier) && (event->modifiers() & Qt::ShiftModifier)))))
        {
            doZoomBestFit();
            return;
        }
    }
}

void SceneViewCommon2D::mouseReleaseEvent(QMouseEvent *event)
{
    setCursor(Qt::ArrowCursor);

    // zoom region
    if (actSceneZoomRegion)
    {
        actSceneZoomRegion->setChecked(false);
    }

    if (m_zoomRegion)
    {
        Point posStart = transform(Point(m_zoomRegionPos.x(), m_zoomRegionPos.y()));
        Point posEnd = transform(Point(m_lastPos.x(), m_lastPos.y()));

        doZoomRegion(Point(qMin(posStart.x, posEnd.x), qMin(posStart.y, posEnd.y)), Point(qMax(posStart.x, posEnd.x), qMax(posStart.y, posEnd.y)));
    }

    m_zoomRegion = false;
    update();

    Q_EMIT mouseSceneModeChanged(MouseSceneMode_Nothing);
}

void SceneViewCommon2D::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_lastPos = event->pos();

    setToolTip("");

    Point p = transform(Point(m_lastPos.x(), m_lastPos.y()));

    // zoom or select region
    if (m_zoomRegion)
        update();

    // pan - middle button or shift + left mouse
    if ((event->buttons() & Qt::MiddleButton)
            || ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::ShiftModifier) && !(event->modifiers() & Qt::ControlModifier)))
    {
        setCursor(Qt::PointingHandCursor);

        m_offset2d.x -= 2.0/width() * dx/m_scale2d*aspect();
        m_offset2d.y += 2.0/height() * dy/m_scale2d;

        Q_EMIT mouseSceneModeChanged(MouseSceneMode_Pan);
        update();
    }

    Q_EMIT mouseMoved(p);

    if (Agros::configComputer()->value(Config::Config_ShowRulers).toBool())
        update();
}

void SceneViewCommon2D::wheelEvent(QWheelEvent *event)
{
    Point posMouse;
    posMouse = Point((2.0/width()*(event->position().x() - width()/2.0))/m_scale2d*aspect(),
                     -(2.0/height()*(event->position().y() - height()/2.0))/m_scale2d);

    m_offset2d.x += posMouse.x;
    m_offset2d.y += posMouse.y;

    m_scale2d = m_scale2d * qPow(1.2, event->angleDelta().y()/150.0);

    posMouse = Point((2.0/width()*(event->position().x() - width()/2.0))/m_scale2d*aspect(),
                     -(2.0/height()*(event->position().y() - height()/2.0))/m_scale2d);

    m_offset2d.x -= posMouse.x;
    m_offset2d.y -= posMouse.y;

    update();
}


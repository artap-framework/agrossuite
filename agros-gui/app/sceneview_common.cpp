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

#include "sceneview_common.h"

#include "util/global.h"
#include "logview.h"

#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "solver/module.h"

#include "solver/problem.h"
#include "solver/problem_config.h"

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

static const int TEXTURE_SIZE = 512;


inline void transformPoint(GLdouble out[4], const GLdouble m[16], const GLdouble in[4])
{
#define M(row,col)  m[col*4+row]
    out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
    out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
    out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
    out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

inline GLint project(GLdouble objx, GLdouble objy, GLdouble objz,
                     const GLdouble model[16], const GLdouble proj[16],
const GLint viewport[4], GLdouble * winx, GLdouble * winy, GLdouble * winz)
{
    GLdouble in[4], out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;
    transformPoint(out, model, in);
    transformPoint(in, proj, out);

    if (in[3] == 0.0)
        return GL_FALSE;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    *winx = viewport[0] + (1 + in[0]) * viewport[2] / 2;
    *winy = viewport[1] + (1 + in[1]) * viewport[3] / 2;

    *winz = (1 + in[2]) / 2;
    return GL_TRUE;
}

SceneViewCommon::SceneViewCommon(QWidget *parent)
    : QOpenGLWidget(parent),
      actSceneZoomRegion(NULL),
      m_labelRulersSize(0),
      m_labelPostSize(0),
      m_lastPos(QPoint()),
      m_zoomRegion(false),
      m_zoomRegionPos(QPointF()),
      m_windowColor{0, 0, 0}
{
    createActions();

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    QPalette p;
    m_windowColor[0] = p.window().color().red() / 255.0;
    m_windowColor[1] = p.window().color().green() / 255.0;
    m_windowColor[2] = p.window().color().blue() / 255.0;

    setMinimumSize(200, 200);
}

SceneViewCommon::~SceneViewCommon()
{
}

void SceneViewCommon::createActions()
{
    // scene - zoom
    actSceneZoomIn = new QAction(tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(actSceneZoomIn, SIGNAL(triggered()), this, SLOT(doZoomIn()));

    actSceneZoomOut = new QAction(tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(actSceneZoomOut, SIGNAL(triggered()), this, SLOT(doZoomOut()));

    actSceneZoomBestFit = new QAction(tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));
    connect(actSceneZoomBestFit, SIGNAL(triggered()), this, SLOT(doZoomBestFit()));

    actSceneZoomRegion = new QAction(tr("Zoom region"), this);
    actSceneZoomRegion->setCheckable(true);
}

void SceneViewCommon::initializeGL()
{
    glShadeModel(GL_SMOOTH);
    glDisable(GL_MULTISAMPLE);
    glEnable(GL_NORMALIZE);
}

#if QT_VERSION > 0x050100
void SceneViewCommon::messageLogged(QOpenGLDebugMessage message)
{
    qDebug() << message;
}
#endif

void SceneViewCommon::resizeGL(int w, int h)
{
    setupViewport(w, h);
    QOpenGLWidget::resizeGL(w, h);
}

void SceneViewCommon::setupViewport(int w, int h)
{
    glViewport(0, 0, w, h);
}

void SceneViewCommon::printRulersAt(int penX, int penY, const QString &text)
{    
    m_labelRulersSize = Agros::configComputer()->value(Config::Config_RulersFontPointSize).toInt();
    printAt(penX, penY, text, m_labelRulersSize);
}

void SceneViewCommon::printPostAt(int penX, int penY, const QString &text)
{
    m_labelPostSize = Agros::configComputer()->value(Config::Config_PostFontPointSize).toInt();
    printAt(penX, penY, text, m_labelPostSize);
}

QPixmap SceneViewCommon::renderScenePixmap(int w, int h, bool useContext)
{
    // QPixmap p = QPixmap::fromImage(grabFrameBuffer(false));
    // p = renderPixmap(width() * 3, height() * 3);
    // return p;
    return QPixmap::fromImage(grabFramebuffer());
}

void SceneViewCommon::loadProjectionViewPort()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void SceneViewCommon::printAt(int penX, int penY, const QString &text, int fontSize)
{
    int width = this->width();
    int height = this->height();

    GLdouble model[4][4], proj[4][4];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, &model[0][0]);
    glGetDoublev(GL_PROJECTION_MATRIX, &proj[0][0]);
    glGetIntegerv(GL_VIEWPORT, &view[0]);

    GLdouble textPosX = 0;
    GLdouble textPosY = 0;
    GLdouble textPosZ = 0;

    project(penX, penY, 0, &model[0][0], &proj[0][0], &view[0], &textPosX, &textPosY, &textPosZ);

    textPosY = height - textPosY; // y is inverted

    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setFont(defaultFixedFont(fontSize));
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter.drawText(textPosX, textPosY, text); // z = pointT4.z + distOverOp / 4
    painter.end();
}

// events *****************************************************************************************************************************

void SceneViewCommon::closeEvent(QCloseEvent *event)
{
    event->ignore();
}

// slots *****************************************************************************************************************************

void SceneViewCommon::doZoomBestFit()
{
    if (problem())
    {
        RectPoint rect = problem()->scene()->boundingBox();
        doZoomRegion(rect.start, rect.end);
    }
    else
    {
        doZoomRegion(Point(-0.5, -0.5), Point(0.5, 0.5));
    }
}

void SceneViewCommon::doZoomIn()
{
    setZoom(1.2);
}

void SceneViewCommon::doZoomOut()
{
    setZoom(-1/1.2);
}

void SceneViewCommon::refresh()
{
    paintGL();
    update();
}

void SceneViewCommon::drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments) const
{
    if (segments == -1)
    {
        if (arcAngle < 10)
            segments = 10;
        else
            segments = arcAngle / 3;
    }

    if (segments < 2) segments = 2;
    double theta = arcAngle / double(segments - 1);

    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < segments; i++)
    {
        double arc = (startAngle + i*theta)/180.0*M_PI;

        double x = r * fastcos(arc);
        double y = r * fastsin(arc);

        glVertex3d(point.x + x, point.y + y, 0.0);
    }
    glEnd();
}

void SceneViewCommon::drawBlend(Point start, Point end, double red, double green, double blue, double alpha) const
{
    // store color
    double color[4];
    glGetDoublev(GL_CURRENT_COLOR, color);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // blended rectangle
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(red, green, blue, alpha);

    glBegin(GL_QUADS);
    glVertex2d(start.x, start.y);
    glVertex2d(end.x, start.y);
    glVertex2d(end.x, end.y);
    glVertex2d(start.x, end.y);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_OFFSET_FILL);

    // retrieve color
    glColor4d(color[0], color[1], color[2], color[3]);
}

void SceneViewCommon::saveImageToFile(const QString &fileName, int w, int h)
{
    QPixmap pixmap = renderScenePixmap(w, h);
    if (!pixmap.save(fileName, "PNG"))
        Agros::log()->printError(tr("Problem"), tr("Image cannot be saved to the file '%1'.").arg(fileName));
}

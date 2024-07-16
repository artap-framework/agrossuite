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

#include <deal.II/numerics/fe_field_function.h>

#include "util/util.h"
#include "util/global.h"
#include "util/constants.h"
#include "util/loops.h"

#include "scene.h"
#include "solver/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "postprocessorview.h"
#include "scenemarkerselectdialog.h"

#include "gui/resultsview.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/plugin_interface.h"

#include "sceneview_post2d.h"

SceneViewPost2D::SceneViewPost2D(PostprocessorWidget *postprocessorWidget)
    : SceneViewCommon2D(postprocessorWidget), SceneViewPostInterface(postprocessorWidget), m_postprocessorWidget(postprocessorWidget),
      m_listContours(-1),
      m_listVectors(-1),
      m_listScalarField(-1),
      m_selectedPoint(Point())
{
    createActionsPost2D();

    connect(this, SIGNAL(mousePressed(Point)), this, SLOT(selectedPoint(Point)));
}

SceneViewPost2D::~SceneViewPost2D()
{
}

ProblemBase *SceneViewPost2D::problem() const
{
    return static_cast<ProblemBase *>(m_postprocessorWidget->currentComputation().data());
}

void SceneViewPost2D::createActionsPost2D()
{
    // point
    actSelectPoint = new QAction(icon("results_other_local"), tr("Local point value"), this);
    actSelectPoint->setIconVisibleInMenu(true);
    connect(actSelectPoint, SIGNAL(triggered()), this, SLOT(selectPoint()));

    // marker
    actSelectByMarker = new QAction(icon("results_other_marker"), tr("Select by marker"), this);
    actSelectByMarker->setIconVisibleInMenu(true);
    connect(actSelectByMarker, SIGNAL(triggered()), this, SLOT(selectByMarker()));

    // postprocessor group
    actPostprocessorModeNothing = new QAction(icon("geometry_clear"), tr("Nothing"), this);
    actPostprocessorModeNothing->setCheckable(true);
    actPostprocessorModeNothing->setChecked(true);

    actPostprocessorModeLocalPointValue = new QAction(icon("results_point"), tr("Point"), this);
    actPostprocessorModeLocalPointValue->setCheckable(true);

    actPostprocessorModeSurfaceIntegral = new QAction(icon("results_surface"), tr("Surface int."), this);
    actPostprocessorModeSurfaceIntegral->setCheckable(true);

    actPostprocessorModeVolumeIntegral = new QAction(icon("results_volume"), tr("Volume int."), this);
    actPostprocessorModeVolumeIntegral->setCheckable(true);

    actPostprocessorModeGroup = new QActionGroup(this);
    actPostprocessorModeGroup->addAction(actPostprocessorModeNothing);
    actPostprocessorModeGroup->addAction(actPostprocessorModeLocalPointValue);
    actPostprocessorModeGroup->addAction(actPostprocessorModeSurfaceIntegral);
    actPostprocessorModeGroup->addAction(actPostprocessorModeVolumeIntegral);
    connect(actPostprocessorModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostprocessorModeGroup(QAction*)));

    actExportVTKScalar = new QAction(tr("Export VTK scalar..."), this);
    connect(actExportVTKScalar, SIGNAL(triggered()), this, SLOT(exportVTKScalarView()));

    actExportVTKContours = new QAction(tr("Export VTK contours..."), this);
    connect(actExportVTKContours, SIGNAL(triggered()), this, SLOT(exportVTKContourView()));
}

void SceneViewPost2D::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_A)
    {
        // select all
        if (event->modifiers() & Qt::ControlModifier)
        {
            // select volume integral area
            if (actPostprocessorModeVolumeIntegral->isChecked())
            {
                m_postprocessorWidget->currentComputation()->scene()->selectAll(SceneGeometryMode_OperateOnLabels);
                // remove holes selection
                foreach (SceneLabel *label, m_postprocessorWidget->currentComputation()->scene()->labels->items())
                    if (label->isHole())
                        label->setSelected(false);

                emit mousePressed();
            }

            // select surface integral area
            if (actPostprocessorModeSurfaceIntegral->isChecked())
            {
                m_postprocessorWidget->currentComputation()->scene()->selectAll(SceneGeometryMode_OperateOnEdges);

                emit mousePressed();
            }

            update();
        }
    }

    SceneViewCommon2D::keyPressEvent(event);
}

void SceneViewPost2D::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon2D::mousePressEvent(event);

    m_lastPos = event->pos();
    Point p = transform(Point(event->pos().x(), event->pos().y()));

    if (event->buttons() & Qt::LeftButton)
    {
        // local point value
        if (actPostprocessorModeLocalPointValue->isChecked())
        {
            m_selectedPoint = p;
            emit mousePressed(p);

            update();
        }

        // select volume integral area
        if (actPostprocessorModeVolumeIntegral->isChecked())
        {
            // find marker
            SceneLabel *label = SceneLabel::findLabelAtPoint(m_postprocessorWidget->currentComputation()->scene(), p);
            if (label)
            {
                label->setSelected(!label->isSelected());

                update();
                emit mousePressed();
            }
        }

        // select surface integral area
        if (actPostprocessorModeSurfaceIntegral->isChecked())
        {
            //  find edge marker
            SceneFace *edge = SceneFace::findClosestFace(m_postprocessorWidget->currentComputation()->scene(), p);

            edge->setSelected(!edge->isSelected());
            update();

            emit mousePressed();
        }
    }
}

void SceneViewPost2D::paintGL()
{
    if (!isVisible() || m_postprocessorWidget->currentComputation().isNull()) return;
    this->makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    // paintGrid();

    // view
    if (m_postprocessorWidget->currentComputation()->isSolved() && m_postprocessorWidget->currentComputation()->postDeal()->isProcessed())
    {
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowScalarView).toBool()) paintScalarField();
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowContourView).toBool()) paintContours();
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowVectorView).toBool()) paintVectors();
    }

    // geometry
    paintGeometry();

    if (m_postprocessorWidget->currentComputation()->isSolved() && m_postprocessorWidget->currentComputation()->postDeal()->isProcessed())
    {
        if (actPostprocessorModeLocalPointValue->isChecked()) paintPostprocessorSelectedPoint();
        if (actPostprocessorModeVolumeIntegral->isChecked()) paintPostprocessorSelectedVolume();
        if (actPostprocessorModeSurfaceIntegral->isChecked()) paintPostprocessorSelectedSurface();

        // bars
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowScalarView).toBool()
                && m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowScalarColorBar).toBool())
            paintScalarFieldColorBar(this,
                                     m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeMin).toDouble(),
                                     m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeMax).toDouble());
    }

    // rulers
    if (Agros::configComputer()->value(Config::Config_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Agros::configComputer()->value(Config::Config_ShowAxes).toBool()) paintAxes();

    paintZoomRegion();

    if (m_postprocessorWidget->currentComputation()->isSolved() && m_postprocessorWidget->currentComputation()->postDeal()->isProcessed())
    {
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowScalarView).toBool())
        {
            Module::LocalVariable localVariable = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localVariable(m_postprocessorWidget->currentComputation()->config()->coordinateType(),
                                                                                                                                            m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString());
            QString text = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString() != "" ? localVariable.name() : "";
            if ((PhysicFieldVariableComp) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariableComp).toInt() != PhysicFieldVariableComp_Scalar)
                text += " - " + physicFieldVariableCompString((PhysicFieldVariableComp) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariableComp).toInt());

            // emit labelCenter(text);
        }
    }
}

void SceneViewPost2D::resizeGL(int w, int h)
{
    if (m_postprocessorWidget->currentComputation()->isSolved())
        paletteCreate();

    SceneViewCommon::resizeGL(w, h);
}

void SceneViewPost2D::paintGeometry()
{
    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items())
    {
        glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
        glLineWidth(EDGEWIDTH);

        if (fabs(edge->angle()) < EPS_ZERO)
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
            double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

            drawArc(center, radius, startAngle, edge->angle());
        }

        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintScalarField()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    loadProjection2d(true);

    if (m_postprocessorWidget->currentComputation()->postDeal()->scalarValues().isEmpty()) {
        std::cerr << "Scalar values are empty." << std::endl;
        return;
    }

    paletteCreate();

    // range
    double rangeMin = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeMin).toDouble();
    double rangeMax = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeMax).toDouble();
    double irange = 1.0 / (rangeMax - rangeMin);
    // special case: constant solution
    if (fabs(rangeMax - rangeMin) < EPS_ZERO)
        irange = 1.0;

    // set texture for coloring
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);

    // Kontrola, zda je textura platná
    GLint textureBinding;
    glGetIntegerv(GL_TEXTURE_BINDING_1D, &textureBinding);
    if (textureBinding != m_textureScalar) {
        std::cerr << "Texture binding failed." << std::endl;
        return;
    }

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // set texture transformation matrix
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslated(m_texShift, 0.0, 0.0);
    glScaled(m_texScale, 1.0, 1.0); // opravený scaling

    // Create or regenerate display list
    if (m_listScalarField == -1)
    {
        m_listScalarField = glGenLists(1);
        glNewList(m_listScalarField, GL_COMPILE);

        glBegin(GL_TRIANGLES);
        foreach (PostTriangle triangle, m_postprocessorWidget->currentComputation()->postDeal()->scalarValues())
        {
            if (!m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeAuto).toBool())
            {
                double avgValue = (triangle.values[0] + triangle.values[1] + triangle.values[2]) / 3.0;
                if (avgValue < rangeMin || avgValue > rangeMax)
                    continue;
            }

            for (int j = 0; j < 3; j++)
            {
                double texCoord;
                if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeLog).toBool())
                    texCoord = log10((1 + (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeBase).toInt() - 1)) * (triangle.values[j] - rangeMin) * irange) / log10(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeBase).toInt());
                else
                    texCoord = (triangle.values[j] - rangeMin) * irange;

                glTexCoord1d(texCoord);
                glVertex2d(triangle.vertices[j][0], triangle.vertices[j][1]);
            }
        }
        glEnd();

        glEndList();
    }

    // Call the display list
    glCallList(m_listScalarField);

    // Disable texture and reset texture transform
    glDisable(GL_TEXTURE_1D);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    // Ensure all OpenGL commands are executed
    glFlush();
    glFinish();

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error: " << error << std::endl;
    }
}


void SceneViewPost2D::paintContours()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    loadProjection2d(true);

    if (m_listContours == -1)
    {
        if (m_postprocessorWidget->currentComputation()->postDeal()->contourValues().isEmpty()) return;

        m_listContours = glGenLists(1);
        glNewList(m_listContours, GL_COMPILE);

        // min max
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();

        foreach (PostTriangle triangle, m_postprocessorWidget->currentComputation()->postDeal()->contourValues())
        {
            for (int i = 0; i < 3; i++)
            {
                if (triangle.values[i] > rangeMax) rangeMax = triangle.values[i];
                if (triangle.values[i] < rangeMin) rangeMin = triangle.values[i];
            }
        }

        // draw contours
        if ((rangeMax-rangeMin) > EPS_ZERO)
        {
            // value range
            double step = (rangeMax-rangeMin) / m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ContoursCount).toInt();

            glLineWidth(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ContoursWidth).toInt());
            glColor3d(COLORCONTOURS[0], COLORCONTOURS[1], COLORCONTOURS[2]);

            glBegin(GL_LINES);
            foreach (PostTriangle triangle, m_postprocessorWidget->currentComputation()->postDeal()->contourValues())
            {
                paintContoursTri(triangle, step);
            }
            glEnd();
        }
        glEndList();

        glCallList(m_listContours);
    }
    else
    {
        glCallList(m_listContours);
    }
}

void SceneViewPost2D::paintContoursTri(const PostTriangle &triangle, double step)
{
    // sort the vertices by their value, keep track of the permutation sign.
    int i, idx[3] = { 0, 1, 2 }, perm = 0;
    for (i = 0; i < 2; i++)
    {
        if (triangle.values[idx[0]] > triangle.values[idx[1]])
        {
            std::swap(idx[0], idx[1]);
            perm++;
        }
        if (triangle.values[idx[1]] > triangle.values[idx[2]])
        {
            std::swap(idx[1], idx[2]);
            perm++;
        }
    }
    if (fabs(triangle.values[idx[0]] - triangle.values[idx[2]]) < 1e-3 * fabs(step))
        return;

    // get the first (lowest) contour value
    double val = triangle.values[idx[0]];

    double y = ceil(val / step);
    val = y * step;

    int l1 = 0, l2 = 1;
    int r1 = 0, r2 = 2;

    while (val < triangle.values[idx[r2]])
    {
        double ld = triangle.values[idx[l2]] - triangle.values[idx[l1]];
        double rd = triangle.values[idx[r2]] - triangle.values[idx[r1]];

        // draw a slice of the triangle
        while (val < triangle.values[idx[l2]])
        {
            double lt = (val - triangle.values[idx[l1]]) / ld;
            double rt = (val - triangle.values[idx[r1]]) / rd;

            double x1 = (1.0 - lt) * triangle.vertices[idx[l1]][0] + lt * triangle.vertices[idx[l2]][0];
            double y1 = (1.0 - lt) * triangle.vertices[idx[l1]][1] + lt * triangle.vertices[idx[l2]][1];
            double x2 = (1.0 - rt) * triangle.vertices[idx[r1]][0] + rt * triangle.vertices[idx[r2]][0];
            double y2 = (1.0 - rt) * triangle.vertices[idx[r1]][1] + rt * triangle.vertices[idx[r2]][1];

            if (perm & 1)
            {
                glVertex2d(x1, y1);
                glVertex2d(x2, y2);
            }
            else
            {
                glVertex2d(x2, y2);
                glVertex2d(x1, y1);
            }

            val += step;
        }
        l1 = 1;
        l2 = 2;
    }
}

void SceneViewPost2D::paintVectors()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    loadProjection2d(true);

    if (m_listVectors == -1)
    {
        // if (!m_postprocessorWidget->currentComputation()->postDeal()->vecVectorView()) return;

        m_listVectors = glGenLists(1);
        glNewList(m_listVectors, GL_COMPILE);

        RectPoint rect = m_postprocessorWidget->currentComputation()->scene()->boundingBox();
        double gs = (rect.width() + rect.height()) / m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorCount).toInt();

        MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();
        dealii::Functions::FEFieldFunction<2> localvalues(ma.doFHandler(), ma.solution());

        QList<PostTriangle> triangleX = m_postprocessorWidget->currentComputation()->postDeal()->vectorXValues();
        QList<PostTriangle> triangleY = m_postprocessorWidget->currentComputation()->postDeal()->vectorYValues();

        // min max
        double rangeMin =  numeric_limits<double>::max();
        double rangeMax = -numeric_limits<double>::max();
        for (int i = 0; i < m_postprocessorWidget->currentComputation()->postDeal()->vectorXValues().size(); i++)
        {
            double valueX = (triangleX[i].values[0] + triangleX[i].values[1] + triangleX[i].values[2]) / 3.0;
            double valueY = (triangleY[i].values[0] + triangleY[i].values[1] + triangleY[i].values[2]) / 3.0;
            double valueSqr = valueX*valueX + valueY*valueY;

            if (valueSqr < rangeMin) rangeMin = valueSqr;
            if (valueSqr > rangeMax) rangeMax = valueSqr;
        }
        rangeMin = sqrt(rangeMin);
        rangeMax = sqrt(rangeMax);

        // range
        double irange = 1.0 / (rangeMax - rangeMin);
        // special case: constant solution
        if (fabs(rangeMax - rangeMin) < EPS_ZERO)
            irange = 1.0;

        int countX = rect.width() / gs;
        int countY = rect.height() / gs;

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_TRIANGLES);
        for (int i = 0; i < m_postprocessorWidget->currentComputation()->postDeal()->vectorXValues().size(); i++)
        {
            // triangleX and triangleY have same coordinates (+ swap 1 <-> 2)
            dealii::Point<2> vertices[3];
            vertices[0] = triangleX[i].vertices[0];
            vertices[1] = triangleX[i].vertices[2];
            vertices[2] = triangleX[i].vertices[1];

            Point a(vertices[0][0], vertices[0][1]);
            Point b(vertices[1][0], vertices[1][1]);
            Point c(vertices[2][0], vertices[2][1]);

            RectPoint r;
            r.start = Point(qMin(qMin(a.x, b.x), c.x), qMin(qMin(a.y, b.y), c.y));
            r.end = Point(qMax(qMax(a.x, b.x), c.x), qMax(qMax(a.y, b.y), c.y));

            // double area
            double area2 = a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y);

            // plane equation
            double aa = b.x*c.y - c.x*b.y;
            double ab = c.x*a.y - a.x*c.y;
            double ac = a.x*b.y - b.x*a.y;
            double ba = b.y - c.y;
            double bb = c.y - a.y;
            double bc = a.y - b.y;
            double ca = c.x - b.x;
            double cb = a.x - c.x;
            double cc = b.x - a.x;
            double ax = (aa * triangleX[i].values[0] + ab * triangleX[i].values[1] + ac * triangleX[i].values[2]) / area2;
            double bx = (ba * triangleX[i].values[0] + bb * triangleX[i].values[1] + bc * triangleX[i].values[2]) / area2;
            double cx = (ca * triangleX[i].values[0] + cb * triangleX[i].values[1] + cc * triangleX[i].values[2]) / area2;
            double ay = (aa * triangleY[i].values[0] + ab * triangleY[i].values[1] + ac * triangleY[i].values[2]) / area2;
            double by = (ba * triangleY[i].values[0] + bb * triangleY[i].values[1] + bc * triangleY[i].values[2]) / area2;
            double cy = (ca * triangleY[i].values[0] + cb * triangleY[i].values[1] + cc * triangleY[i].values[2]) / area2;

            for (int j = floor(r.start.x / gs); j < ceil(r.end.x / gs); j++)
            {
                for (int k = floor(r.start.y / gs); k < ceil(r.end.y / gs); k++)
                {
                    Point point(j*gs, k*gs);
                    if (k % 2 == 0) point.x += gs/2.0;

                    // find in triangle
                    bool inTriangle = true;
                    for (int l = 0; l < 3; l++)
                    {
                        int p = l + 1;
                        if (p == 3)
                            p = 0;
                        double z = (vertices[p][0] - vertices[l][0]) * (point.y - vertices[l][1]) - (vertices[p][1] - vertices[l][1]) *
                                (point.x - vertices[l][0]);
                        if (z < 0)
                        {
                            inTriangle = false;
                            break;
                        }
                    }

                    if (inTriangle)
                    {
                        // view
                        double dx = ax + bx * point.x + cx * point.y;
                        double dy = ay + by * point.x + cy * point.y;
                        double value = sqrt(dx*dx + dy*dy);
                        double angle = atan2(dy, dx);
                        if ((m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorProportional).toBool()) &&
                                (fabs(rangeMin - rangeMax) > EPS_ZERO))
                        {
                            if ((value / rangeMax) < 1e-6)
                            {
                                dx = 0.0;
                                dy = 0.0;
                            }
                            else
                            {
                                dx = ((value - rangeMin) * irange) * m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * cos(angle);
                                dy = ((value - rangeMin) * irange) * m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * sin(angle);
                            }
                        }
                        else
                        {
                            dx = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * cos(angle);
                            dy = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * sin(angle);
                        }
                        double dm = sqrt(dx*dx + dy*dy);
                        // color
                        if ((m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorColor).toBool())
                                && (fabs(rangeMin - rangeMax) > EPS_ZERO))
                        {
                            double color = 0.7 - 0.7 * (value - rangeMin) * irange;
                            glColor3d(color, color, color);
                        }
                        else
                        {
                            glColor3d(COLORVECTORS[0], COLORVECTORS[1], COLORVECTORS[2]);
                        }
                        // tail
                        Point shiftCenter(0.0, 0.0);
                        if ((VectorCenter) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorCenter).toInt() == VectorCenter_Head)
                            shiftCenter = Point(- 2.0*dm * cos(angle), - 2.0*dm * sin(angle)); // head
                        if ((VectorCenter) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorCenter).toInt() == VectorCenter_Center)
                            shiftCenter = Point(- dm * cos(angle), - dm * sin(angle)); // center
                        if ((VectorType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorType).toInt() == VectorType_Arrow)
                        {
                            // arrow and shaft
                            // head for an arrow
                            double vh1x = point.x + dm/5.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vh1y = point.y + dm/5.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vh2x = point.x + dm/5.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vh2y = point.y + dm/5.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                            double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;
                            glVertex2d(vh1x, vh1y);
                            glVertex2d(vh2x, vh2y);
                            glVertex2d(vh3x, vh3y);

                            // shaft for an arrow
                            double vs1x = point.x + dm/15.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vs1y = point.y + dm/15.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vs2x = point.x + dm/15.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                            double vs2y = point.y + dm/15.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                            double vs3x = vs1x - dm * cos(angle);
                            double vs3y = vs1y - dm * sin(angle);
                            double vs4x = vs2x - dm * cos(angle);
                            double vs4y = vs2y - dm * sin(angle);

                            glVertex2d(vs1x, vs1y);
                            glVertex2d(vs2x, vs2y);
                            glVertex2d(vs3x, vs3y);
                            glVertex2d(vs4x, vs4y);
                            glVertex2d(vs3x, vs3y);
                            glVertex2d(vs2x, vs2y);
                        }
                        else if ((VectorType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorType).toInt() == VectorType_Cone)
                        {
                            // cone
                            double vh1x = point.x + dm/3.5 * cos(angle - M_PI/2.0) + shiftCenter.x;
                            double vh1y = point.y + dm/3.5 * sin(angle - M_PI/2.0) + shiftCenter.y;
                            double vh2x = point.x + dm/3.5 * cos(angle + M_PI/2.0) + shiftCenter.x;
                            double vh2y = point.y + dm/3.5 * sin(angle + M_PI/2.0) + shiftCenter.y;
                            double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                            double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;
                            glVertex2d(vh1x, vh1y);
                            glVertex2d(vh2x, vh2y);
                            glVertex2d(vh3x, vh3y);
                        }
                    }
                }
            }
        }
        glEnd();

        /*
        QList<dealii::Point<2> > points;
        QList<dealii::Tensor<1, 2> > gradients;
        for (int i = 0; i < countX; i++)
        {
            for (int j = 0; j < countY; j++)
            {
                try
                {
                    dealii::Point<2> point(rect.start.x + i * gs + ((j % 2 == 0) ? 0 : gs / 2.0), rect.start.y + j * gs);
                    dealii::Tensor<1, 2> grad = localvalues.gradient(point);

                    points.append(point);
                    gradients.append(grad);

                    if (grad.norm() > rangeMax) rangeMax = grad.norm();
                    if (grad.norm() < rangeMin) rangeMin = grad.norm();
                }
                catch (const dealii::GridTools::ExcPointNotFound<2> &e)
                {
                    continue;
                }
            }
        }

        //Add 20% margin to the range
        double vectorRange = rangeMax - rangeMin;
        rangeMin = rangeMin - 0.2*vectorRange;
        rangeMax = rangeMax + 0.2*vectorRange;

        // qDebug() << "SceneViewCommon::paintVectors(), min = " << vectorRangeMin << ", max = " << vectorRangeMax;

        double irange = 1.0 / (rangeMax - rangeMin);
        // if (fabs(vectorRangeMin - vectorRangeMax) < EPS_ZERO) return;

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        for (int i = 0; i < points.size(); i++)
        {
            Point point = Point(points[i][0], points[i][1]);

            double dx = gradients[i][0];
            double dy = gradients[i][1];

            double value = sqrt(dx*dx + dy*dy);
            double angle = atan2(dy, dx);

            if ((m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorProportional).toBool()) && (fabs(rangeMin - rangeMax) > EPS_ZERO))
            {
                if ((value / rangeMax) < 1e-6)
                {
                    dx = 0.0;
                    dy = 0.0;
                }
                else
                {
                    dx = ((value - rangeMin) * irange) * m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * cos(angle);
                    dy = ((value - rangeMin) * irange) * m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * sin(angle);
                }
            }
            else
            {
                dx = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * cos(angle);
                dy = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble() * gs * sin(angle);
            }

            double dm = sqrt(dx*dx + dy*dy);
            // qDebug() << dx << dy << dm;

            // color
            if ((m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorColor).toBool())
                    && (fabs(rangeMin - rangeMax) > EPS_ZERO))
            {
                double color = 0.7 - 0.7 * (value - rangeMin) * irange;
                glColor3d(color, color, color);
            }
            else
            {
                glColor3d(COLORVECTORS[0], COLORVECTORS[1], COLORVECTORS[2]);
            }

            // tail
            Point shiftCenter(0.0, 0.0);
            if ((VectorCenter) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorCenter).toInt() == VectorCenter_Head)
                shiftCenter = Point(- 2.0*dm * cos(angle), - 2.0*dm * sin(angle)); // head
            if ((VectorCenter) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorCenter).toInt() == VectorCenter_Center)
                shiftCenter = Point(- dm * cos(angle), - dm * sin(angle)); // center

            if ((VectorType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorType).toInt() == VectorType_Arrow)
            {
                // arrow and shaft
                // head for an arrow
                double vh1x = point.x + dm/5.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vh1y = point.y + dm/5.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vh2x = point.x + dm/5.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vh2y = point.y + dm/5.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;

                glVertex2d(vh1x, vh1y);
                glVertex2d(vh2x, vh2y);
                glVertex2d(vh3x, vh3y);

                // shaft for an arrow
                double vs1x = point.x + dm/15.0 * cos(angle + M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vs1y = point.y + dm/15.0 * sin(angle + M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vs2x = point.x + dm/15.0 * cos(angle - M_PI/2.0) + dm * cos(angle) + shiftCenter.x;
                double vs2y = point.y + dm/15.0 * sin(angle - M_PI/2.0) + dm * sin(angle) + shiftCenter.y;
                double vs3x = vs1x - dm * cos(angle);
                double vs3y = vs1y - dm * sin(angle);
                double vs4x = vs2x - dm * cos(angle);
                double vs4y = vs2y - dm * sin(angle);

                glVertex2d(vs1x, vs1y);
                glVertex2d(vs2x, vs2y);
                glVertex2d(vs3x, vs3y);
                glVertex2d(vs4x, vs4y);
                glVertex2d(vs3x, vs3y);
                glVertex2d(vs2x, vs2y);
            }
            else if ((VectorType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::VectorType).toInt() == VectorType_Cone)
            {
                // cone
                double vh1x = point.x + dm/3.5 * cos(angle - M_PI/2.0) + shiftCenter.x;
                double vh1y = point.y + dm/3.5 * sin(angle - M_PI/2.0) + shiftCenter.y;
                double vh2x = point.x + dm/3.5 * cos(angle + M_PI/2.0) + shiftCenter.x;
                double vh2y = point.y + dm/3.5 * sin(angle + M_PI/2.0) + shiftCenter.y;
                double vh3x = point.x + 2.0 * dm * cos(angle) + shiftCenter.x;
                double vh3y = point.y + 2.0 * dm * sin(angle) + shiftCenter.y;

                glVertex2d(vh1x, vh1y);
                glVertex2d(vh2x, vh2y);
                glVertex2d(vh3x, vh3y);
            }
        }
        glEnd();
        */

        glDisable(GL_POLYGON_OFFSET_FILL);

        glEndList();

        glCallList(m_listVectors);
    }
    else
    {
        glCallList(m_listVectors);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedVolume()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    if (m_postprocessorWidget->currentComputation()->scene()->crossings().isEmpty())
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // blended rectangle
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2], 0.5);

        QMapIterator<SceneLabel*, QList<MeshGeneratorTriangleFast::Triangle> > i(m_postprocessorWidget->currentComputation()->scene()->fastMeshInfo()->polygonTriangles());
        glBegin(GL_TRIANGLES);
        while (i.hasNext())
        {
            i.next();

            if (i.key()->isSelected())
            {
                foreach (MeshGeneratorTriangleFast::Triangle triangle, i.value())
                {
                    glVertex2d(triangle.a.x, triangle.a.y);
                    glVertex2d(triangle.b.x, triangle.b.y);
                    glVertex2d(triangle.c.x, triangle.c.y);
                }
            }
        }
        glEnd();

        glDisable(GL_BLEND);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedSurface()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    // edges
    foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items()) {
        glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
        glLineWidth(3.0);

        if (edge->isSelected())
        {
            if (edge->isStraight())
            {
                glBegin(GL_LINES);
                glVertex2d(edge->nodeStart()->point().x, edge->nodeStart()->point().y);
                glVertex2d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y);
                glEnd();

                // connect with inner label, outer normal should be in opposite dirrection, but not allways, depends on geometry!
                /*
                glLineWidth(2.0);
                glBegin(GL_LINES);
                glVertex2d((edge->nodeStart()->point().x + edge->nodeEnd()->point().x) / 2., (edge->nodeStart()->point().y + edge->nodeEnd()->point().y) / 2.);
                SceneLabel* label = Agros::m_postprocessorWidget->currentComputation()->scene()->labels->at(edge->innerLabelIdx());
                glVertex2d(label->point().x, label->point().y);

                glEnd();
                */
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180 - 180;

                drawArc(center, radius, startAngle, edge->angle());
            }
        }
        glLineWidth(1.0);
    }
}

void SceneViewPost2D::paintPostprocessorSelectedPoint()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
    glPointSize(8.0);

    glBegin(GL_POINTS);
    glVertex2d(m_selectedPoint.x, m_selectedPoint.y);
    glEnd();
}

void SceneViewPost2D::clearGLLists()
{
    if (m_listContours != -1) glDeleteLists(m_listContours, 1);
    if (m_listVectors != -1) glDeleteLists(m_listVectors, 1);
    if (m_listScalarField != -1) glDeleteLists(m_listScalarField, 1);

    m_listContours = -1;
    m_listVectors = -1;
    m_listScalarField = -1;
}

void SceneViewPost2D::refresh()
{
    clearGLLists();
    setControls();

    // if (!m_postprocessorWidget->currentComputation().isNull() && m_postprocessorWidget->currentComputation()->isSolved())
    SceneViewCommon::refresh();
}

void SceneViewPost2D::setControls()
{
    if (!m_postprocessorWidget->currentComputation().isNull())
    {
        actPostprocessorModeGroup->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
        actSelectByMarker->setEnabled(m_postprocessorWidget->currentComputation()->isSolved() && (actPostprocessorModeSurfaceIntegral->isChecked() || actPostprocessorModeVolumeIntegral->isChecked()));
        actSelectPoint->setEnabled(m_postprocessorWidget->currentComputation()->isSolved() && actPostprocessorModeLocalPointValue->isChecked());
        actExportVTKScalar->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
        actExportVTKContours->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
    }
}

void SceneViewPost2D::clear()
{
    actPostprocessorModeNothing->setChecked(true);
    actPostprocessorModeNothing->trigger();

    SceneViewCommon2D::clear();

    refresh();
    doZoomBestFit();
}

void SceneViewPost2D::exportVTKScalarView(const QString &fileName)
{
    exportVTK(fileName,
              m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString(),
              (PhysicFieldVariableComp) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariableComp).toInt());
}

void SceneViewPost2D::exportVTKContourView(const QString &fileName)
{
    Module::LocalVariable variable = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localVariable(m_postprocessorWidget->currentComputation()->config()->coordinateType(),
                                                                                                                               m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ContourVariable).toString());
    PhysicFieldVariableComp comp = variable.isScalar() ? PhysicFieldVariableComp_Scalar : PhysicFieldVariableComp_Magnitude;

    exportVTK(fileName,
              m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ContourVariable).toString(),
              comp);
}

void SceneViewPost2D::exportVTK(const QString &fileName, const QString &variable, PhysicFieldVariableComp physicFieldVariableComp)
{
    if (!m_postprocessorWidget->currentComputation().isNull() && m_postprocessorWidget->currentComputation()->isSolved())
    {
        QString fn = fileName;

        if (fn.isEmpty())
        {
            // file dialog
            QSettings settings;
            QString dir = settings.value("General/LastVTKDir").toString();

            fn = QFileDialog::getSaveFileName(this, tr("Export VTK file"), dir, tr("VTK files (*.vtk)"));
            if (fn.isEmpty())
                return;

            if (!fn.endsWith(".vtk"))
                fn.append(".vtk");

            // remove existing file
            if (QFile::exists(fn))
                QFile::remove(fn);
        }

        QSharedPointer<Computation> computation = m_postprocessorWidget->currentComputation();
        dealii::DataPostprocessorScalar<2> *post = computation->postDeal()->activeViewField()->plugin()->filter(computation.data(),
                                                                                                                computation->postDeal()->activeViewField(),
                                                                                                                computation->postDeal()->activeTimeStep(),
                                                                                                                computation->postDeal()->activeAdaptivityStep(),
                                                                                                                computation->postDeal()->activeViewField()->localVariable(computation->config()->coordinateType(),
                                                                                                                                                                          variable).id(),
                                                                                                                physicFieldVariableComp);

        MultiArray ma = computation->postDeal()->activeMultiSolutionArray();

        PostDataOut *data_out = new PostDataOut(computation->postDeal()->activeViewField(), computation.data());
        data_out->attach_dof_handler(ma.doFHandler());
        data_out->add_data_vector(ma.solution(), *post);
        data_out->build_patches(2);

        std::ofstream output (fn.toStdString());
        data_out->write_vtk(output);

        // release data object
        delete data_out;

        // release post object
        delete post;

        if (!fn.isEmpty())
        {
            QFileInfo fileInfo(fn);
            if (fileInfo.absoluteDir() != tempProblemDir())
            {
                QSettings settings;
                settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
            }
        }
    }
}

void SceneViewPost2D::selectByMarker()
{
    SceneModePostprocessor mode = (actPostprocessorModeSurfaceIntegral->isChecked()) ? SceneModePostprocessor_SurfaceIntegral : SceneModePostprocessor_VolumeIntegral;

    SceneMarkerSelectDialog sceneMarkerSelectDialog(this, mode, m_postprocessorWidget->currentComputation().data());
    if (sceneMarkerSelectDialog.exec() == QDialog::Accepted)
        emit mousePressed();
}

void SceneViewPost2D::selectPoint()
{
    LocalPointValueDialog localPointValueDialog(m_selectedPoint, m_postprocessorWidget->currentComputation().data());
    if (localPointValueDialog.exec() == QDialog::Accepted)
    {
        emit mousePressed(localPointValueDialog.point());
        update();
    }
}

void SceneViewPost2D::doPostprocessorModeGroup(QAction *action)
{
    if (actPostprocessorModeNothing->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_Empty);
    if (actPostprocessorModeLocalPointValue->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_LocalValue);
    if (actPostprocessorModeSurfaceIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_SurfaceIntegral);
    if (actPostprocessorModeVolumeIntegral->isChecked())
        emit postprocessorModeGroupChanged(SceneModePostprocessor_VolumeIntegral);

    if (!m_postprocessorWidget->currentComputation().isNull())
        m_postprocessorWidget->currentComputation()->scene()->selectNone();

    setControls();
    update();
}

void SceneViewPost2D::selectedPoint(const Point &p)
{
    m_selectedPoint = p;
    update();
}

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

#include "sceneview_geometry_simple.h"

#include <mesh/meshgenerator_triangle.h>

#include "util/util.h"
#include "util/global.h"
#include "util/constants.h"
#include "util/loops.h"
#include "logview.h"

#include "scene.h"
#include "solver/problem.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"


SceneViewSimpleGeometry::SceneViewSimpleGeometry(QWidget *parent)
    : SceneViewCommon2D(parent)
{
    setMinimumSize(150, 250);
}

void SceneViewSimpleGeometry::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO)
        return;

    double sceneWidth = end.x - start.x;
    double sceneHeight = end.y - start.y;

    double maxScene = ((width() / height()) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale2d = 1.8/maxScene;

    m_offset2d.x = (start.x + end.x) / 2.0;
    m_offset2d.y = (start.y + end.y) / 2.0;

    setZoom(0);
}

void SceneViewSimpleGeometry::refresh()
{
    if (!m_problem.isNull())
        SceneViewCommon::refresh();
}

void SceneViewSimpleGeometry::clear()
{
    doZoomBestFit();
}

void SceneViewSimpleGeometry::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // geometry
    paintGeometry();
}

void SceneViewSimpleGeometry::paintGeometry()
{
    if (m_problem.isNull())
        return;

    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, m_problem->scene()->faces->items())
    {
        glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
        glLineWidth(EDGEWIDTH);

        if (edge->isStraight())
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
    }

    try
    {
        if (m_problem->scene()->crossings().isEmpty())
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // blended rectangle
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glBegin(GL_TRIANGLES);
            QMapIterator<SceneLabel*, QList<MeshGeneratorTriangleFast::Triangle> > i(m_problem->scene()->fastMeshInfo()->polygonTriangles());
            while (i.hasNext())
            {
                i.next();

                if (i.key()->isHole())
                    glColor4f(0.3, 0.1, 0.7, 0.00);
                else
                    glColor4f(0.3, 0.1, 0.7, 0.10);

                foreach (MeshGeneratorTriangleFast::Triangle triangle, i.value())
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
    }
    catch (AgrosException& ame)
    {
        // therefore catch exceptions and do nothing
    }
}

// *********************************************************************************************************

SceneViewChartSimpleGeometry::SceneViewChartSimpleGeometry(PostprocessorWidget *postprocessorWidget)
    : SceneViewCommon2D(postprocessorWidget), m_postprocessorWidget(postprocessorWidget)
{
}

void SceneViewChartSimpleGeometry::setChartLine(const ChartLine &chartLine)
{
    m_chartLine = chartLine;

    update();
}

void SceneViewChartSimpleGeometry::doZoomRegion(const Point &start, const Point &end)
{
    if (fabs(end.x-start.x) < EPS_ZERO || fabs(end.y-start.y) < EPS_ZERO)
        return;

    double sceneWidth = end.x - start.x;
    double sceneHeight = end.y - start.y;

    double maxScene = ((width() / height()) < (sceneWidth / sceneHeight)) ? sceneWidth/aspect() : sceneHeight;

    if (maxScene > 0.0)
        m_scale2d = 1.8/maxScene;

    m_offset2d.x = (start.x + end.x) / 2.0;
    m_offset2d.y = (start.y + end.y) / 2.0;

    setZoom(0);
}

void SceneViewChartSimpleGeometry::paintChartLine()
{
    if (m_postprocessorWidget->currentComputation().isNull())
        return;

    loadProjection2d(true);

    RectPoint rect = problem()->scene()->boundingBox();
    double dm = qMax(rect.width(), rect.height()) / 25.0;

    glColor3d(0.9, 0.1, 0.1);
    glLineWidth(1.0);

    // line
    if (m_chartLine.start == m_chartLine.end)
    {
        glPointSize(5.0);
        glBegin(GL_POINTS);
        glVertex2d(m_chartLine.start.x, m_chartLine.start.y);
        glEnd();
    }
    else
    {
        glBegin(GL_LINES);
        glVertex2d(m_chartLine.start.x, m_chartLine.start.y);
        glVertex2d(m_chartLine.end.x, m_chartLine.end.y);
        glEnd();

        double angle = atan2(m_chartLine.end.y - m_chartLine.start.y,
                             m_chartLine.end.x - m_chartLine.start.x);

        // shaft for an arrow
        double vs1x = m_chartLine.end.x + dm / 2.5 * cos(angle + M_PI/2.0) - dm * cos(angle);
        double vs1y = m_chartLine.end.y + dm / 2.5 * sin(angle + M_PI/2.0) - dm * sin(angle);
        double vs2x = m_chartLine.end.x + dm / 2.5 * cos(angle - M_PI/2.0) - dm * cos(angle);
        double vs2y = m_chartLine.end.y + dm / 2.5 * sin(angle - M_PI/2.0) - dm * sin(angle);
        double vs3x = m_chartLine.end.x;
        double vs3y = m_chartLine.end.y;        

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBegin(GL_TRIANGLES);
        glVertex2d(vs1x, vs1y);
        glVertex2d(vs2x, vs2y);
        glVertex2d(vs3x, vs3y);
        glEnd();

        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void SceneViewChartSimpleGeometry::clear()
{
    doZoomBestFit();
}

void SceneViewChartSimpleGeometry::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // geometry
    paintGeometry();

    // chart line
    paintChartLine();
}

void SceneViewChartSimpleGeometry::paintGeometry()
{
    if (m_postprocessorWidget->currentComputation().isNull())
        return;

    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, problem()->scene()->faces->items())
    {
        glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
        glLineWidth(EDGEWIDTH);

        if (edge->isStraight())
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
    }

    try
    {
        if (problem()->scene()->crossings().isEmpty())
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // blended rectangle
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glBegin(GL_TRIANGLES);
            QMapIterator<SceneLabel*, QList<MeshGeneratorTriangleFast::Triangle> > i(m_postprocessorWidget->currentComputation()->scene()->fastMeshInfo()->polygonTriangles());
            while (i.hasNext())
            {
                i.next();

                if (i.key()->isHole())
                    glColor4f(0.3, 0.1, 0.7, 0.00);
                else
                    glColor4f(0.3, 0.1, 0.7, 0.10);

                foreach (MeshGeneratorTriangleFast::Triangle triangle, i.value())
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
    }
    catch (AgrosException& ame)
    {
        // therefore catch exceptions and do nothing
    }
}

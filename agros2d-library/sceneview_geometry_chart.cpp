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

#include "sceneview_geometry_chart.h"

#include "util.h"
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

#include "gui/physicalfield.h"

#include "solver/module.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

SceneViewSimpleGeometry::SceneViewSimpleGeometry(QWidget *parent, PhysicalFieldWidget *fieldWidget)
    : SceneViewCommon2D(parent), m_fieldWidget(fieldWidget)
{
    setMinimumSize(100, 50);
}

SceneViewSimpleGeometry::~SceneViewSimpleGeometry()
{
}

ProblemBase *SceneViewSimpleGeometry::problem() const
{
    return static_cast<ProblemBase *>(m_fieldWidget->selectedComputation().data());
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
    if (!m_fieldWidget->selectedComputation().isNull() && m_fieldWidget->selectedComputation()->isSolved())
        SceneViewCommon::refresh();
}

void SceneViewSimpleGeometry::clear()
{
    doZoomBestFit();
}

void SceneViewSimpleGeometry::setChartLine(ChartLine chartLine)
{
    m_chartLine = chartLine;

    updateGL();
}

void SceneViewSimpleGeometry::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // geometry
    paintGeometry();
    paintChartLine();
}

void SceneViewSimpleGeometry::paintGeometry()
{
    if (m_fieldWidget->selectedComputation().isNull())
        return;

    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, m_fieldWidget->selectedComputation()->scene()->faces->items())
    {
        glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
        glLineWidth(1.0);

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
}

void SceneViewSimpleGeometry::paintChartLine()
{
    if (m_fieldWidget->selectedComputation().isNull())
        return;

    loadProjection2d(true);

    RectPoint rect = m_fieldWidget->selectedComputation()->scene()->boundingBox();
    double dm = qMax(rect.width(), rect.height()) / 25.0;

    glColor3d(1.0, 0.1, 0.1);
    glLineWidth(2.0);

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

        glLineWidth(1.0);

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

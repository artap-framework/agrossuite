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

#include "sceneview_study.h"

#include <mesh/meshgenerator_triangle.h>
#include <optilab/parameter.h>

#include "util/util.h"
#include "util/global.h"
#include "util/constants.h"
#include "util/loops.h"
#include "logview.h"

#include "scene.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarkerdialog.h"


SceneViewStudy::SceneViewStudy(QWidget *parent, bool showRulers)
    : SceneViewCommon2D(parent), m_currentComputation(nullptr), m_recipe(nullptr), m_parameter(nullptr), m_showRulers(showRulers), m_userMode(NoInteraction)
{
    setMinimumSize(300, 300);

    m_currentComputation = QSharedPointer<Computation>(new Computation());
    m_currentComputation->readFromProblem();
}

ProblemBase *SceneViewStudy::problem() const
{
    return static_cast<ProblemBase *>(m_currentComputation.data());
}

void SceneViewStudy::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon2D::mousePressEvent(event);

    if (m_userMode == NoInteraction)
        return;

    m_lastPos = event->pos();
    Point p = transform(Point(event->pos().x(), event->pos().y()));

    if (event->buttons() & Qt::LeftButton)
    {
        // local point value
        if (m_userMode == SelectPoint)
        {
            m_selectedPoint = p;
            Q_EMIT mousePressed(p);

            update();
        }

        // select volume integral area
        if (m_userMode == SelectVolume)
        {
            // find marker
            SceneLabel *label = SceneLabel::findLabelAtPoint(m_currentComputation->scene(), p);
            if (label)
            {
                label->setSelected(!label->isSelected());

                update();
                Q_EMIT mousePressed();
            }
        }

        // select surface integral area
        if (m_userMode == SelectSurface)
        {
            //  find edge marker
            SceneFace *edge = SceneFace::findClosestFace(m_currentComputation->scene(), p);

            edge->setSelected(!edge->isSelected());
            update();

            Q_EMIT mousePressed();
        }
    }
}

void SceneViewStudy::doZoomRegion(const Point &start, const Point &end)
{
    if (m_userMode == NoInteraction)
    {
        SceneViewCommon2D::doZoomRegion(start, end);
    }
    else
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
}

void SceneViewStudy::refresh()
{
    SceneViewCommon::refresh();

    m_currentComputation->readFromProblem();
}

void SceneViewStudy::clear()
{
    doZoomBestFit();
}

void SceneViewStudy::paintGL()
{
    if (!isVisible()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    paintGrid();

    // geometry
    paintGeometryStudy();

    // rulers
    if (m_showRulers)
    {
        if (Agros::configComputer()->value(Config::Config_ShowRulers).toBool())
        {
            paintRulers();
        }
    }
}

void SceneViewStudy::paintGeometryStudy()
{
    if (m_userMode == NoInteraction)
    {
        m_currentComputation->scene()->selectNone();

        if (m_parameter)
        {
            constexpr double colorLower[3] = { 86 / 255.0, 92 / 255.0, 12 / 255.0 };
            constexpr double colorUpper[3] = { 100 / 255.0, 0 / 255.0, 100 / 255.0 };

            auto parameters = Agros::problem()->config()->parameters()->items();

            parameters[m_parameter->name()].setValue(m_parameter->lowerBound());
            // parameters[m_parameter->name()].setValue(Agros::problem()->config()->parameters()->number(m_parameter->name()) * 1.05);
            m_currentComputation->checkAndApplyParameters(parameters);
            paintGeometryStudyEdges(colorLower);

            parameters[m_parameter->name()].setValue(m_parameter->upperBound());
            // parameters[m_parameter->name()].setValue(Agros::problem()->config()->parameters()->number(m_parameter->name()) * 0.95);
            m_currentComputation->checkAndApplyParameters(parameters);
            paintGeometryStudyEdges(colorUpper);

            // paint original geometry
            m_currentComputation->checkAndApplyParameters(Agros::problem()->config()->parameters()->items());
            paintGeometryStudyEdges(COLOREDGE);

            // paint volume
            paintGeometryStudyVolume();
        }

        // recipes
        if (m_recipe)
        {
            // paint volume
            paintGeometryStudyVolume();

            // paint original geometry
            m_currentComputation->checkAndApplyParameters(Agros::problem()->config()->parameters()->items());
            paintGeometryStudyEdges(COLOREDGE);

            if (auto *localRecipe = dynamic_cast<LocalValueRecipe *>(m_recipe))
            {
                // localRecipe
                paintGeometryStudySelectedPoint(localRecipe->point());
            }
            else if (auto *surfaceRecipe = dynamic_cast<SurfaceIntegralRecipe *>(m_recipe))
            {
                // surfaceRecipe
                foreach (int edgeIndex, surfaceRecipe->edges())
                    m_currentComputation->scene()->faces->items().at(edgeIndex)->setSelected(true);
                paintGeometryStudySelectedEdges();
            }
            else if (auto *volumeRecipe = dynamic_cast<VolumeIntegralRecipe *>(m_recipe))
            {
                // volumeRecipe
                foreach (int labelIndex, volumeRecipe->labels())
                    m_currentComputation->scene()->labels->items().at(labelIndex)->setSelected(true);
                paintGeometryStudySelectedVolume();
            }
        }

        if (!m_parameter && !m_recipe)
        {
            // paint original geometry
            m_currentComputation->checkAndApplyParameters(Agros::problem()->config()->parameters()->items());
            paintGeometryStudyEdges(COLOREDGE);

            // paint volume
            paintGeometryStudyVolume();
        }
    }
    else if (m_userMode == SelectPoint)
    {
        paintGeometryStudyEdges(COLOREDGE);
        paintGeometryStudyVolume();
        paintGeometryStudySelectedPoint(m_selectedPoint);
    }
    else if (m_userMode == SelectSurface)
    {
        paintGeometryStudySelectedEdges();
        paintGeometryStudyVolume();
    }
    else if (m_userMode == SelectVolume)
    {
        paintGeometryStudyEdges(COLOREDGE);
        paintGeometryStudySelectedVolume();
    }
}

void SceneViewStudy::paintGeometryStudySelectedPoint(const Point &selectedPoint) const
{
    glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
    glPointSize(8.0);

    glBegin(GL_POINTS);
    glVertex2d(selectedPoint.x, selectedPoint.y);
    glEnd();
}

void SceneViewStudy::paintGeometryStudySelectedEdges() const
{
    for (int i = 0; i < m_currentComputation->scene()->faces->items().count(); i++)
    {
        auto *edge = m_currentComputation->scene()->faces->items().at(i);

        if (edge->isSelected())
        {
            glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
            glLineWidth(EDGEWIDTH + 2.0);
        }
        else
        {
            glColor3d(COLOREDGE[0], COLOREDGE[1], COLOREDGE[2]);
            glLineWidth(EDGEWIDTH);
        }

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

void SceneViewStudy::paintGeometryStudySelectedVolume() const
{
    if (m_currentComputation->scene()->crossings().isEmpty())
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // blended rectangle
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_TRIANGLES);
        QMapIterator<SceneLabel *, QList<MeshGeneratorTriangleFast::Triangle> > i(m_currentComputation->scene()->fastMeshInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();

            auto *label = i.key();
            if (label->isSelected())
                glColor4f(0.3f, 0.1f, 0.7f, 0.55f);
            else
                glColor4f(0.3f, 0.1f, 0.7, 0.10f);

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

void SceneViewStudy::paintGeometryStudyEdges(const double color[])
{
    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, m_currentComputation->scene()->faces->items())
    {
        glColor3d(color[0], color[1], color[2]);
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
}

void SceneViewStudy::paintGeometryStudyVolume() const
{
    paintGeometryStudySelectedVolume();
}

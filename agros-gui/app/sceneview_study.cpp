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


SceneViewStudy::SceneViewStudy(QWidget *parent)
    : SceneViewProblem(parent), m_currentComputation(nullptr), m_recipe(nullptr), m_parameter(nullptr)
{
    setMinimumSize(150, 200);
}

void SceneViewStudy::refresh()
{
    SceneViewCommon::refresh();

    m_currentComputation = QSharedPointer<Computation>(new Computation());
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
    if (Agros::configComputer()->value(Config::Config_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHintsEdges();
    }

    // axes
    if (Agros::configComputer()->value(Config::Config_ShowAxes).toBool())
    {
        paintAxes();
    }
}

void SceneViewStudy::paintGeometryStudy()
{
    m_currentComputation->scene()->selectNone();

    if (m_parameter)
    {
        const double colorLower[3] = { 86 / 255.0, 92 / 255.0, 12 / 255.0 };
        const double colorUpper[3] = { 86 / 255.0, 92 / 255.0, 12 / 255.0 };

        auto parameters = Agros::problem()->config()->parameters()->items();

        parameters[m_parameter->name()].setValue(m_parameter->lowerBound());
        m_currentComputation->checkAndApplyParameters(parameters);
        paintGeometryStudyEdges(colorLower);

        parameters[m_parameter->name()].setValue(m_parameter->upperBound());
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
            paintGeometryStudySelectedEdges(surfaceRecipe->edges());
        }
        else if (auto *volumeRecipe = dynamic_cast<VolumeIntegralRecipe *>(m_recipe))
        {
            // volumeRecipe
            paintGeometryStudySelectedVolume(volumeRecipe->labels());
        }
    }

    if (!m_recipe && !m_parameter)
    {
        // paint original geometry
        m_currentComputation->checkAndApplyParameters(Agros::problem()->config()->parameters()->items());
        paintGeometryStudyEdges(COLOREDGE);

        // paint volume
        paintGeometryStudyVolume();
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

void SceneViewStudy::paintGeometryStudySelectedEdges(QList<int> selectedEdges) const
{
    for (int i = 0; i < m_currentComputation->scene()->faces->items().count(); i++)
    {
        auto *edge = m_currentComputation->scene()->faces->items().at(i);

        if (selectedEdges.contains(i))
        {
            glColor3d(COLORSELECTED[0], COLORSELECTED[1], COLORSELECTED[2]);
            glLineWidth(EDGEWIDTH + 2.0);

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
}

void SceneViewStudy::paintGeometryStudySelectedVolume(QList<int> selectedLabels) const
{
    if (m_currentComputation->scene()->crossings().isEmpty())
    {
        qInfo() << selectedLabels;

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
            int index = m_currentComputation->scene()->labels->items().indexOf(label);
            qInfo() << index;
            if (selectedLabels.contains(index))
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

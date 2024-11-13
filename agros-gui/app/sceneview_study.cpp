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


SceneViewStudy::SceneViewStudy(QWidget *parent)
    : SceneViewProblem(parent)
{
    setMinimumSize(150, 200);
}

void SceneViewStudy::refresh()
{
    SceneViewCommon::refresh();
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
    loadProjection2d(true);

    // edges
    foreach (SceneFace *edge, Agros::problem()->scene()->faces->items())
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
        if (Agros::problem()->scene()->crossings().isEmpty())
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // blended rectangle
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glBegin(GL_TRIANGLES);
            QMapIterator<SceneLabel*, QList<MeshGeneratorTriangleFast::Triangle> > i(Agros::problem()->scene()->fastMeshInfo()->polygonTriangles());
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

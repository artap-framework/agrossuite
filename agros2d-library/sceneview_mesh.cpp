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

#include "sceneview_mesh.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"

#include "scene.h"
#include "solver/field.h"
#include "logview.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "postprocessorview.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/module.h"

// deal.ii
#include <deal.II/grid/tria.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/hp/dof_handler.h>
#include <deal.II/base/quadrature_lib.h>
#include <deal.II/numerics/error_estimator.h>

const int QUADRATURE_ORDER_INCREASE = 2;

SceneViewMesh::SceneViewMesh(PostprocessorWidget *postprocessorWidget)
    : SceneViewCommon2D(postprocessorWidget), SceneViewPostInterface(postprocessorWidget), m_postprocessorWidget(postprocessorWidget)
{
    createActionsMesh();
}

SceneViewMesh::~SceneViewMesh()
{
}

ProblemBase *SceneViewMesh::problem() const
{
    return static_cast<ProblemBase *>(m_postprocessorWidget->currentComputation().data());
}

void SceneViewMesh::createActionsMesh()
{
    actExportVTKOrder = new QAction(tr("Export VTK order..."), this);
    connect(actExportVTKOrder, SIGNAL(triggered()), this, SLOT(exportVTKOrderView()));

    actExportVTKMesh = new QAction(tr("Export VTK mesh..."), this);
    connect(actExportVTKMesh, SIGNAL(triggered()), this, SLOT(exportVTKMesh()));
}

void SceneViewMesh::refresh()
{
    clearGLLists();

    setControls();

    SceneViewCommon::refresh();
}

void SceneViewMesh::clearGLLists()
{
    m_arrayInitialMesh.clear();
    m_arraySolutionMesh.clear();
    m_arrayOrderMesh.clear();
    m_arrayOrderMeshColor.clear();
}

void SceneViewMesh::setControls()
{
    if (!m_postprocessorWidget->currentComputation().isNull())
    {
        actExportVTKMesh->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
        actExportVTKOrder->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
    }
}

void SceneViewMesh::clear()
{
    setControls();

    SceneViewCommon2D::clear();
    doZoomBestFit();
}

void SceneViewMesh::exportVTKMesh(const QString &fileName)
{
    exportVTK(fileName, true);
}

void SceneViewMesh::exportVTKOrderView(const QString &fileName)
{
    exportVTK(fileName, false);
}


void SceneViewMesh::exportVTK(const QString &fileName, bool exportMeshOnly)
{
    if (m_postprocessorWidget->currentComputation()->isSolved())
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

        assert(0); // IMPLEMENT dealii
        /*
        Views::Orderizer orderView;
        if (exportMeshOnly)
            orderView.save_mesh_vtk(postDeal()->activeMultiSolutionArray().spaces().at(0),
                                    fn.toLatin1().data());
        else
            orderView.save_orders_vtk(postDeal()->activeMultiSolutionArray().spaces().at(0),
                                      fn.toLatin1().data());
        */

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

void SceneViewMesh::paintGL()
{
    if (!isVisible() || m_postprocessorWidget->currentComputation().isNull()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    // grid
    if (Agros2D::configComputer()->value(Config::Config_ShowGrid).toBool()) paintGrid();

    // view
    if (m_postprocessorWidget->currentComputation()->isSolved() && m_postprocessorWidget->currentComputation()->postDeal()->isProcessed())
    {
        // order
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderView).toBool()) paintOrder();
        // error
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowErrorView).toBool()) paintError();
        // solution mesh
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowSolutionMeshView).toBool()) paintSolutionMesh();
        // initial mesh
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowInitialMeshView).toBool()) paintInitialMesh();
    }

    // geometry
    paintGeometry();

    if (m_postprocessorWidget->currentComputation()->isSolved() && m_postprocessorWidget->currentComputation()->postDeal()->isProcessed())
    {
        // bars
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderView).toBool()
                && m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderColorBar).toBool())
            paintOrderColorBar();

        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowErrorView).toBool()
                && m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderColorBar).toBool())
            paintErrorColorBar();

    }

    // rulers
    if (Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool())
    {
        paintRulers();
        paintRulersHints();
    }

    // axes
    if (Agros2D::configComputer()->value(Config::Config_ShowAxes).toBool()) paintAxes();

    paintZoomRegion();
}

void SceneViewMesh::paintGeometry()
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

void SceneViewMesh::paintInitialMesh()
{
    if (!m_postprocessorWidget->currentComputation()->isMeshed()) return;

    if (m_arrayInitialMesh.isEmpty())
    {
        if (m_postprocessorWidget->currentComputation()->initialMesh().n_active_cells() == 0)
            return;

        // vertices
        const std::vector<dealii::Point<2> > &vertices = m_postprocessorWidget->currentComputation()->initialMesh().get_vertices();

        // faces
        dealii::Triangulation<2>::active_face_iterator ti = m_postprocessorWidget->currentComputation()->initialMesh().begin_face();
        while (ti != m_postprocessorWidget->currentComputation()->initialMesh().end_face())
        {
            m_arrayInitialMesh.append(QVector2D(vertices[ti->vertex_index(0)][0], vertices[ti->vertex_index(0)][1]));
            m_arrayInitialMesh.append(QVector2D(vertices[ti->vertex_index(1)][0], vertices[ti->vertex_index(1)][1]));

            ++ti;
        }
    }
    else
    {
        loadProjection2d(true);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3d(COLORINITIALMESH[0], COLORINITIALMESH[1], COLORINITIALMESH[2]);
        glLineWidth(1.3);

        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arrayInitialMesh.constData());
        glDrawArrays(GL_LINES, 0, m_arrayInitialMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void SceneViewMesh::paintSolutionMesh()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    if (m_arraySolutionMesh.isEmpty())
    {
        MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();
        for (int level = 0; level <= ma.doFHandler().get_tria().n_levels() - 1; level++)
        {
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end_active(level);
            for (; cell_int != endc_int; ++cell_int)
            {
                if (cell_int->active_fe_index() == 0)
                    continue;

                // coordinates
                dealii::Point<2> point0 = cell_int->vertex(0);
                dealii::Point<2> point1 = cell_int->vertex(1);
                dealii::Point<2> point2 = cell_int->vertex(2);
                dealii::Point<2> point3 = cell_int->vertex(3);

                m_arraySolutionMesh.append(QVector2D(point0[0], point0[1]));
                m_arraySolutionMesh.append(QVector2D(point1[0], point1[1]));
                m_arraySolutionMesh.append(QVector2D(point3[0], point3[1]));
                m_arraySolutionMesh.append(QVector2D(point2[0], point2[1]));
            }
        }
    }
    else
    {
        loadProjection2d(true);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3d(COLORSOLUTIONMESH[0], COLORSOLUTIONMESH[1], COLORSOLUTIONMESH[2]);
        glLineWidth(1.3);

        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arraySolutionMesh.constData());
        glDrawArrays(GL_QUADS, 0, m_arraySolutionMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
    }
}

void SceneViewMesh::paintOrder()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    if (m_arrayOrderMesh.isEmpty())
    {
        MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();

        for (int level = 0; level <= ma.doFHandler().get_tria().n_levels() - 1; level++)
        {
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end_active(level);
            for (; cell_int != endc_int; ++cell_int)
            {
                if (cell_int->active_fe_index() == 0)
                    continue;

                // coordinates
                dealii::Point<2> point0 = cell_int->vertex(0);
                dealii::Point<2> point1 = cell_int->vertex(1);
                dealii::Point<2> point2 = cell_int->vertex(2);
                dealii::Point<2> point3 = cell_int->vertex(3);

                // polynomial degree
                int degree = cell_int->get_fe().degree;

                QVector3D colorVector = QVector3D(paletteColorOrder(degree)[0], paletteColorOrder(degree)[1], paletteColorOrder(degree)[2]);

                m_arrayOrderMesh.append(QVector2D(point0[0], point0[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point1[0], point1[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point2[0], point2[1]));
                m_arrayOrderMeshColor.append(colorVector);

                m_arrayOrderMesh.append(QVector2D(point1[0], point1[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point3[0], point3[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point2[0], point2[1]));
                m_arrayOrderMeshColor.append(colorVector);
            }
        }
    }
    else
    {
        loadProjection2d(true);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arrayOrderMesh.constData());
        glColorPointer(3, GL_FLOAT, 0, m_arrayOrderMeshColor.constData());
        glDrawArrays(GL_TRIANGLES, 0, m_arrayOrderMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // paint labels
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderLabel).toBool())
    {
        loadProjectionViewPort();

        glColor3d(1.0, 1.0, 1.0);
        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

        MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();

        for (int level = 0; level <= ma.doFHandler().get_tria().n_levels() - 1; level++)
        {
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end_active(level);
            for (; cell_int != endc_int; ++cell_int)
            {
                if (cell_int->active_fe_index() == 0)
                    continue;

                // coordinates
                dealii::Point<2> point0 = cell_int->vertex(0);
                dealii::Point<2> point1 = cell_int->vertex(1);
                dealii::Point<2> point2 = cell_int->vertex(2);
                dealii::Point<2> point3 = cell_int->vertex(3);

                // polynomial degree
                int degree = cell_int->get_fe().degree;

                // average value
                dealii::Point<2> point = point0 + point1 + point2 + point3;

                point[0] /= 4;
                point[1] /= 4;

                Point scr = untransform(point[0], point[1]);

                printPostAt(scr.x - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) / 2.0,
                            scr.y - (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0) / 2.0,
                            QString::number(degree));
            }
        }
    }
}

void SceneViewMesh::paintError()
{    

    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    if (m_arrayOrderMesh.isEmpty())
    {

        MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();
        dealii::hp::QCollection<2-1> quadratureFormulasFace;
        dealii::hp::QCollection<2> quadratureFormulas;

        quadratureFormulas.push_back(dealii::QGauss<2>(1));
        quadratureFormulasFace.push_back(dealii::QGauss<2 - 1>(1));

        // Gauss quadrature
        for (unsigned int degree = 1; degree <= DEALII_MAX_ORDER + 1; degree++)
        {
            quadratureFormulas.push_back(dealii::QGauss<2>(degree + QUADRATURE_ORDER_INCREASE));
            quadratureFormulasFace.push_back(dealii::QGauss<2-1>(degree + QUADRATURE_ORDER_INCREASE));
        }
        // estimated error per cell

        m_estimated_error_per_cell = m_postprocessorWidget->currentComputation()->calculationMesh().n_active_cells();

        // estimator
        // AdaptivityEstimator estimator = (AdaptivityEstimator)  m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt();

        dealii::KellyErrorEstimator<2>::estimate(ma.doFHandler(),
                                                 quadratureFormulasFace,
                                                 TYPENAME dealii::FunctionMap<2>::type(),
                                                 ma.solution(),
                                                 m_estimated_error_per_cell);


        for (int level = 0; level <= ma.doFHandler().get_tria().n_levels() - 1; level++)
        {
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end_active(level);
            for (int i = 0; cell_int != endc_int; ++cell_int, ++i)
            {
                if (cell_int->active_fe_index() == 0)
                    continue;

                // coordinates
                dealii::Point<2> point0 = cell_int->vertex(0);
                dealii::Point<2> point1 = cell_int->vertex(1);
                dealii::Point<2> point2 = cell_int->vertex(2);
                dealii::Point<2> point3 = cell_int->vertex(3);

                // error
                int error = (int) (log10(m_estimated_error_per_cell[i]) + 15) / 2;

                QVector3D colorVector = QVector3D(paletteColorOrder(error)[0], paletteColorOrder(error)[1], paletteColorOrder(error)[2]);

                m_arrayOrderMesh.append(QVector2D(point0[0], point0[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point1[0], point1[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point2[0], point2[1]));
                m_arrayOrderMeshColor.append(colorVector);

                m_arrayOrderMesh.append(QVector2D(point1[0], point1[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point3[0], point3[1]));
                m_arrayOrderMeshColor.append(colorVector);
                m_arrayOrderMesh.append(QVector2D(point2[0], point2[1]));
                m_arrayOrderMeshColor.append(colorVector);
            }
        }
    }
    else
    {
        loadProjection2d(true);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);

        glVertexPointer(2, GL_FLOAT, 0, m_arrayOrderMesh.constData());
        glColorPointer(3, GL_FLOAT, 0, m_arrayOrderMeshColor.constData());
        glDrawArrays(GL_TRIANGLES, 0, m_arrayOrderMesh.size());

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // paint labels
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderLabel).toBool())
    {
        loadProjectionViewPort();

        glColor3d(1.0, 1.0, 1.0);
        glScaled(2.0 / width(), 2.0 / height(), 1.0);
        glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

        MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();

        for (int level = 0; level <= ma.doFHandler().get_tria().n_levels() - 1; level++)
        {
            dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end_active(level);
            for (int i = 0; cell_int != endc_int; ++cell_int, ++i)
            {
                if (cell_int->active_fe_index() == 0)
                    continue;

                // coordinates
                dealii::Point<2> point0 = cell_int->vertex(0);
                dealii::Point<2> point1 = cell_int->vertex(1);
                dealii::Point<2> point2 = cell_int->vertex(2);
                dealii::Point<2> point3 = cell_int->vertex(3);

                // polynomial degree
                double error = m_estimated_error_per_cell[i];

                // average value
                dealii::Point<2> point = point0 + point1 + point2 + point3;

                point[0] /= 4;
                point[1] /= 4;

                Point scr = untransform(point[0], point[1]);

                printPostAt(scr.x - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) / 2.0,
                            scr.y - (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0) / 2.0,
                            QString::number(error));
            }
        }
    }
}

void SceneViewMesh::paintErrorColorBar()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved() || !m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderColorBar).toBool()) return;

    int minError = 100;
    int maxError = 0;
    MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();

    int level = 0;
    dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end();
    for (int i = 0; cell_int != endc_int; ++cell_int, ++i)
    {
        // polynomial degree
        int error = (int) (log10(m_estimated_error_per_cell[i]) + 15) / 2;

        if (error < minError) minError = error;
        if (error > maxError) maxError = error;
    }

    // order color map
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = 11 * (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(20 + textWidth, (20 + maxError * (2 * textHeight) - textHeight / 2.0 + 2));
    Point scaleBorder = Point(10.0, (Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (20 + textWidth));

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // bars
    glBegin(GL_QUADS);
    for (int i = 1; i < maxError + 1; i++)
    {
        glColor3d(0.0, 0.0, 0.0);
        glVertex2d(scaleLeft + 10,                             scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - scaleBorder.x, scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - scaleBorder.x, scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 10,                             scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);

        glColor3d(paletteColorOrder(i)[0], paletteColorOrder(i)[1], paletteColorOrder(i)[2]);
        glVertex2d(scaleLeft + 12,                                 scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - 2 - scaleBorder.x, scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - 2 - scaleBorder.x, scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 12,                                 scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
    }
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // labels
    glColor3d(0.0, 0.0, 0.0);
    for (int i = 1; i < maxError + 1; i++)
    {
        printPostAt(scaleLeft + 10 + textWidth / 2.0 - (QString::number(i).size() - 1) * (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) / 2.0 - scaleBorder.x,
                    scaleBorder.y + 10.0 + (i-1)*(2.0 * textHeight) + textHeight / 2.0,
                    QString::number(pow10((i*2-15))));
    }
}

void SceneViewMesh::paintOrderColorBar()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved() || !m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowOrderColorBar).toBool()) return;

    int minDegree = 100;
    int maxDegree = 1;
    MultiArray ma = m_postprocessorWidget->currentComputation()->postDeal()->activeMultiSolutionArray();

    int level = 0;
    dealii::hp::DoFHandler<2>::active_cell_iterator cell_int = ma.doFHandler().begin_active(level), endc_int = ma.doFHandler().end();
    for (; cell_int != endc_int; ++cell_int)
    {
        // polynomial degree
        int degree = cell_int->get_fe().degree;

        if (degree < minDegree) minDegree = degree;
        if (degree > maxDegree) maxDegree = degree;
    }

    // order color map
    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(- width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = 6 * (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(20 + textWidth, (20 + maxDegree * (2 * textHeight) - textHeight / 2.0 + 2));
    Point scaleBorder = Point(10.0, (Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (20 + textWidth));

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // bars
    glBegin(GL_QUADS);
    for (int i = 1; i < maxDegree + 1; i++)
    {
        glColor3d(0.0, 0.0, 0.0);
        glVertex2d(scaleLeft + 10,                             scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - scaleBorder.x, scaleBorder.y + 10 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - scaleBorder.x, scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 10,                             scaleBorder.y + 12 + (i )*(2 * textHeight) - textHeight / 2.0);

        glColor3d(paletteColorOrder(i)[0], paletteColorOrder(i)[1], paletteColorOrder(i)[2]);
        glVertex2d(scaleLeft + 12,                                 scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - 2 - scaleBorder.x, scaleBorder.y + 12 + (i-1)*(2 * textHeight));
        glVertex2d(scaleLeft + 10 + textWidth - 2 - scaleBorder.x, scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
        glVertex2d(scaleLeft + 12,                                 scaleBorder.y + 10 + (i  )*(2 * textHeight) - textHeight / 2.0);
    }
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // labels
    glColor3d(0.0, 0.0, 0.0);
    for (int i = 1; i < maxDegree + 1; i++)
    {
        printPostAt(scaleLeft + 10 + textWidth / 2.0 - (QString::number(i).size() - 1) * (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) / 2.0 - scaleBorder.x,
                    scaleBorder.y + 10.0 + (i-1)*(2.0 * textHeight) + textHeight / 2.0,
                    QString::number(i));
    }
}

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

#include "sceneview_particle.h"

#include "util/util.h"
#include "util/global.h"
#include "util/constants.h"
#include "util/loops.h"

#include "gui/lineeditdouble.h"
#include "gui/common.h"

#include "particle/particle_tracing.h"

#include "scene.h"
#include "solver/problem.h"
#include "logview.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "postprocessorview.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem_config.h"

/*
    cmbParticleButcherTableType->setCurrentIndex(cmbParticleButcherTableType->findData(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleButcherTableType).toInt()));
    txtParticleNumberOfParticles->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleNumberOfParticles).toInt());
    txtParticleStartingRadius->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleStartingRadius).toDouble());
    chkParticleIncludeRelativisticCorrection->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleIncludeRelativisticCorrection).toBool());
    txtParticleMass->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleMass).toDouble());
    txtParticleConstant->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleConstant).toDouble());
    txtParticlePointX->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleStartX).toDouble());
    txtParticlePointY->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleStartY).toDouble());
    txtParticleVelocityX->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleStartVelocityX).toDouble());
    txtParticleVelocityY->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleStartVelocityY).toDouble());
    chkParticleReflectOnDifferentMaterial->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleReflectOnDifferentMaterial).toBool());
    chkParticleReflectOnBoundary->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleReflectOnBoundary).toBool());
    txtParticleCoefficientOfRestitution->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleCoefficientOfRestitution).toDouble());
    txtParticleCustomForceX->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleCustomForceX).toDouble());
    txtParticleCustomForceY->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleCustomForceY).toDouble());
    txtParticleCustomForceZ->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleCustomForceZ).toDouble());
    txtParticleMaximumRelativeError->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleMaximumRelativeError).toDouble());
    txtParticleMaximumSteps->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleMaximumStep).toDouble());
    txtParticleMaximumNumberOfSteps->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleMaximumNumberOfSteps).toInt());
    chkParticleColorByVelocity->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleColorByVelocity).toBool());
    chkParticleShowPoints->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleShowPoints).toBool());
    chkParticleShowBlendedFaces->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleShowBlendedFaces).toBool());
    txtParticleNumShowParticleAxi->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleNumShowParticlesAxi).toInt());
    txtParticleDragDensity->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleDragDensity).toDouble());
    txtParticleDragReferenceArea->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleDragReferenceArea).toDouble());
    txtParticleDragCoefficient->setValue(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleDragCoefficient).toDouble());
    chkParticleP2PElectricForce->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleP2PElectricForce).toBool());
    chkParticleP2PMagneticForce->setChecked(m_postprocessorWidget->currentComputation()->setting()->defaultValue(ProblemSetting::ParticleP2PMagneticForce).toBool());
    */



// *************************************************************************************************

SceneViewParticleTracing::SceneViewParticleTracing(PostprocessorWidget *postprocessorWidget)
    : SceneViewCommon3D(postprocessorWidget), SceneViewPostInterface(postprocessorWidget), m_postprocessorWidget(postprocessorWidget),
      m_listParticleTracing(-1)
{
    createActionsParticleTracing();
}

SceneViewParticleTracing::~SceneViewParticleTracing()
{
}

ProblemBase *SceneViewParticleTracing::problem() const
{
    return static_cast<ProblemBase *>(m_postprocessorWidget->currentComputation().data());
}

void SceneViewParticleTracing::createActionsParticleTracing()
{
    actSceneModeParticleTracing = new QAction(tr("Particle\nTracing"), this);
    actSceneModeParticleTracing->setShortcut(tr("Ctrl+6"));
    actSceneModeParticleTracing->setCheckable(true);
}

void SceneViewParticleTracing::mousePressEvent(QMouseEvent *event)
{
    SceneViewCommon3D::mousePressEvent(event);
}

void SceneViewParticleTracing::paintGL()
{
    if (!isVisible() || m_postprocessorWidget->currentComputation().isNull()) return;
    makeCurrent();

    glClearColor(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2], 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // gradient background
    paintBackground();

    if (m_postprocessorWidget->currentComputation()->isSolved())
    {
        // todo: what is better?
        //paintGeometrySurface(Agros2D::problem()->configView()->particleShowBlendedFaces);
        if(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleShowBlendedFaces).toBool())
            paintGeometrySurface(true);

        paintGeometryOutline();
        paintParticleTracing();

        // bars
        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleColorByVelocity).toInt())
            paintParticleTracingColorBar(m_velocityMin, m_velocityMax);
    }

    emit labelCenter(tr("Particle tracing"));

    if (Agros2D::configComputer()->value(Config::Config_ShowAxes).toBool()) paintAxes();
}

void SceneViewParticleTracing::resizeGL(int w, int h)
{
    SceneViewCommon::resizeGL(w, h);
}


void SceneViewParticleTracing::paintGeometryOutline()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;
    if (!particleTracingIsPrepared()) return;

    loadProjection3d(true, false);

    RectPoint rect = m_postprocessorWidget->currentComputation()->scene()->boundingBox();
    double max = qMax(rect.width(), rect.height());
    double depth = max / m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarView3DHeight).toDouble();

    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPopMatrix();

    // geometry
    glDisable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);

    glColor3d(0.0, 0.0, 0.0);
    glLineWidth(1.3);

    if (m_postprocessorWidget->currentComputation()->config()->coordinateType() == CoordinateType_Planar)
    {
        // depth
        foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items())
        {
            glBegin(GL_LINES);
            if (edge->isStraight())
            {
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);

                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
            }
            glEnd();
        }

        // length
        foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items())
        {
            glBegin(GL_LINES);
            if (edge->isStraight())
            {
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);

                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                          center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                int segments = edge->angle() / 5.0;
                if (segments < 2) segments = 2;

                double theta = edge->angle() / double(segments);

                for (int i = 0; i < segments; i++)
                {
                    double arc1 = (startAngle + i*theta)/180.0*M_PI;
                    double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                    double x1 = radius * cos(arc1);
                    double y1 = radius * sin(arc1);
                    double x2 = radius * cos(arc2);
                    double y2 = radius * sin(arc2);

                    glVertex3d(center.x + x1, center.y + y1, depth/2.0);
                    glVertex3d(center.x + x2, center.y + y2, depth/2.0);

                    glVertex3d(center.x + x1, center.y + y1, -depth/2.0);
                    glVertex3d(center.x + x2, center.y + y2, -depth/2.0);
                }
            }
            glEnd();
        }
    }
    else
    {
        // top
        foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items())
        {
            for (int j = 0; j <= 360; j = j + 90)
            {
                if (edge->isStraight())
                {
                    glBegin(GL_LINES);
                    glVertex3d(edge->nodeStart()->point().x * cos(j/180.0*M_PI),
                               edge->nodeStart()->point().y,
                               edge->nodeStart()->point().x * sin(j/180.0*M_PI));
                    glVertex3d(edge->nodeEnd()->point().x * cos(j/180.0*M_PI),
                               edge->nodeEnd()->point().y,
                               edge->nodeEnd()->point().x * sin(j/180.0*M_PI));
                    glEnd();
                }
                else
                {
                    Point center = edge->center();
                    double radius = edge->radius();
                    double startAngle = atan2(center.y - edge->nodeStart()->point().y, center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                    double theta = edge->angle() / double(edge->angle()/2 - 1);

                    glBegin(GL_LINE_STRIP);
                    for (int i = 0; i < edge->angle()/2; i++)
                    {
                        double arc = (startAngle + i*theta)/180.0*M_PI;

                        double x = radius * cos(arc);
                        double y = radius * sin(arc);

                        glVertex3d((center.x + x) * cos(j/180.0*M_PI),
                                   center.y + y,
                                   (center.x + x) * sin(j/180.0*M_PI));
                    }
                    glEnd();
                }
            }
        }

        // side
        foreach (SceneNode *node, m_postprocessorWidget->currentComputation()->scene()->nodes->items())
        {
            int count = 29.0;
            double step = 360.0/count;

            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < count; j++)
            {
                glVertex3d(node->point().x * cos((j+0)*step/180.0*M_PI),
                           node->point().y,
                           node->point().x * sin((j+0)*step/180.0*M_PI));
                glVertex3d(node->point().x * cos((j+1)*step/180.0*M_PI),
                           node->point().y,
                           node->point().x * sin((j+1)*step/180.0*M_PI));
            }
            glEnd();
        }
    }

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_DEPTH_TEST);
}

void SceneViewParticleTracing::paintGeometrySurface(bool blend)
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;
    if (!particleTracingIsPrepared()) return;

    loadProjection3d(true, false);

    RectPoint rect = m_postprocessorWidget->currentComputation()->scene()->boundingBox();
    double max = qMax(rect.width(), rect.height());
    double depth = max / m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarView3DHeight).toDouble();

    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);

    if (blend)
    {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4d(0.2, 0.4, 0.1, 0.3);
    }
    else
    {
        glColor3d(0.2, 0.4, 0.1);
    }

    // surfaces
    if (m_postprocessorWidget->currentComputation()->config()->coordinateType() == CoordinateType_Planar)
    {
        glBegin(GL_TRIANGLES);
        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(m_postprocessorWidget->currentComputation()->scene()->loopsInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();
            if (i.key()->isHole())
                continue;

            foreach (LoopsInfo::Triangle triangle, i.value())
            {
                // z = - depth / 2.0
                glVertex3d(triangle.a.x, triangle.a.y, -depth/2.0);
                glVertex3d(triangle.b.x, triangle.b.y, -depth/2.0);
                glVertex3d(triangle.c.x, triangle.c.y, -depth/2.0);

                // z = + depth / 2.0
                glVertex3d(triangle.a.x, triangle.a.y, depth/2.0);
                glVertex3d(triangle.c.x, triangle.c.y, depth/2.0);
                glVertex3d(triangle.b.x, triangle.b.y, depth/2.0);
            }
        }
        glEnd();

        // length
        foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items())
        {
            glBegin(GL_TRIANGLE_STRIP);
            if (edge->isStraight())
            {
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, -depth/2.0);
                glVertex3d(edge->nodeStart()->point().x, edge->nodeStart()->point().y, depth/2.0);

                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, -depth/2.0);
                glVertex3d(edge->nodeEnd()->point().x, edge->nodeEnd()->point().y, depth/2.0);
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                          center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                int segments = edge->angle() / 5.0;
                if (segments < 2) segments = 2;

                double theta = edge->angle() / double(segments);

                for (int i = 0; i < segments + 1; i++)
                {
                    double arc = (startAngle + i*theta)/180.0*M_PI;

                    double x = radius * cos(arc);
                    double y = radius * sin(arc);

                    glVertex3d(center.x + x, center.y + y, -depth/2.0);
                    glVertex3d(center.x + x, center.y + y, depth/2.0);
                }
            }
            glEnd();
        }
    }
    else
    {
        glBegin(GL_TRIANGLES);
        QMapIterator<SceneLabel*, QList<LoopsInfo::Triangle> > i(m_postprocessorWidget->currentComputation()->scene()->loopsInfo()->polygonTriangles());
        while (i.hasNext())
        {
            i.next();
            if (i.key()->isHole())
                continue;

            foreach (LoopsInfo::Triangle triangle, i.value())
            {
                for (int j = 0; j <= 360; j = j + 90)
                {
                    glVertex3d(triangle.a.x * cos(j/180.0*M_PI), triangle.a.y, triangle.a.x * sin(j/180.0*M_PI));
                    glVertex3d(triangle.b.x * cos(j/180.0*M_PI), triangle.b.y, triangle.b.x * sin(j/180.0*M_PI));
                    glVertex3d(triangle.c.x * cos(j/180.0*M_PI), triangle.c.y, triangle.c.x * sin(j/180.0*M_PI));
                }
            }
        }
        glEnd();

        // length
        foreach (SceneFace *edge, m_postprocessorWidget->currentComputation()->scene()->faces->items())
        {
            int count = 29.0;
            double step = 360.0/count;

            glBegin(GL_TRIANGLE_STRIP);
            if (edge->isStraight())
            {
                for (int j = 0; j < count + 1; j++)
                {
                    glVertex3d(edge->nodeStart()->point().x * cos((j+0)*step/180.0*M_PI), edge->nodeStart()->point().y, edge->nodeStart()->point().x * sin((j+0)*step/180.0*M_PI));
                    glVertex3d(edge->nodeEnd()->point().x * cos((j+0)*step/180.0*M_PI), edge->nodeEnd()->point().y, edge->nodeEnd()->point().x * sin((j+0)*step/180.0*M_PI));
                }
            }
            else
            {
                Point center = edge->center();
                double radius = edge->radius();
                double startAngle = atan2(center.y - edge->nodeStart()->point().y,
                                          center.x - edge->nodeStart()->point().x) / M_PI*180.0 - 180.0;

                int segments = edge->angle() / 5.0;
                if (segments < 2) segments = 2;

                double theta = edge->angle() / double(segments);

                for (int i = 0; i < segments; i++)
                {
                    double arc1 = (startAngle + i*theta)/180.0*M_PI;
                    double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                    double x1 = radius * cos(arc1);
                    double y1 = radius * sin(arc1);
                    double x2 = radius * cos(arc2);
                    double y2 = radius * sin(arc2);

                    for (int j = 0; j < count + 1; j++)
                    {
                        glVertex3d((center.x + x1) * cos((j+0)*step/180.0*M_PI), (center.y + y1), (center.x + x1) * sin((j+0)*step/180.0*M_PI));
                        glVertex3d((center.x + x2) * cos((j+0)*step/180.0*M_PI), (center.y + y2), (center.x + x2) * sin((j+0)*step/180.0*M_PI));
                    }
                }
            }
            glEnd();
        }
    }

    if (blend)
    {
        glDisable(GL_BLEND);
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}

void SceneViewParticleTracing::paintParticleTracing()
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;
    if (!particleTracingIsPrepared()) return;

    loadProjection3d(true, false);

    if (m_listParticleTracing == -1)
    {
        m_listParticleTracing = glGenLists(1);
        glNewList(m_listParticleTracing, GL_COMPILE);

        RectPoint rect = m_postprocessorWidget->currentComputation()->scene()->boundingBox();
        double max = qMax(rect.width(), rect.height());
        double depth = max / m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarView3DHeight).toDouble();

        glPushMatrix();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPopMatrix();

        double velocityMin = m_velocityMin;
        double velocityMax = m_velocityMax;

        double positionMin = m_positionMin;
        double positionMax = m_positionMax;

        if ((positionMax - positionMin) < EPS_ZERO)
        {
            positionMin = -1.0;
            positionMax = +1.0;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POINT_SMOOTH);

        // particle visualization
        for (int k = 0; k < m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt(); k++)
        {
            // starting point
            /*
            glPointSize(Agros2D::problem()->configView()->value(ProblemConfigView::NodeSize).toInt() * 1.2);
            glColor3d(0.0, 0.0, 0.0);
            glBegin(GL_POINTS);
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                glVertex3d(m_positionsList[k][0].x, m_positionsList[k][0].y, -depth/2.0 + (m_positionsList[k][0].z - positionMin) * depth/(positionMax - positionMin));
            else
                glVertex3d(m_positionsList[k][0].x * cos(m_positionsList[k][0].z), m_positionsList[k][0].y, m_positionsList[k][0].x * sin(m_positionsList[k][0].z));
            glEnd();
            */

            glColor3d(rand() / double(RAND_MAX),
                      rand() / double(RAND_MAX),
                      rand() / double(RAND_MAX));

            // lines
            if (m_postprocessorWidget->currentComputation()->config()->coordinateType() == CoordinateType_Planar)
            {
                glLineWidth(1.5 * EDGEWIDTH);

                glBegin(GL_LINES);
                for (int i = 0; i < m_positionsList[k].length() - 1; i++)
                {
                    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleColorByVelocity).toBool())
                        glColor3d(1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                  1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                  1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin));

                    glVertex3d(m_positionsList[k][i].x,
                               m_positionsList[k][i].y,
                               0.0);
                    glVertex3d(m_positionsList[k][i+1].x,
                            m_positionsList[k][i+1].y,
                            0.0);
                }
                glEnd();

                // points
                if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleShowPoints).toBool())
                {
                    glColor3d(0.0, 0.0, 0.0);
                    glPointSize(NODESIZE);

                    glBegin(GL_POINTS);
                    for (int i = 0; i < m_positionsList[k].length(); i++)
                    {
                        glVertex3d(m_positionsList[k][i].x,
                                   m_positionsList[k][i].y,
                                   0.0);
                    }
                    glEnd();
                }
            }
            else
            {
                double stepAngle = 360.0 / m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleNumShowParticlesAxi).toInt();

                for (int l = 0; l < m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleNumShowParticlesAxi).toInt(); l++)
                {
                    glLineWidth(1.5 * EDGEWIDTH);

                    glBegin(GL_LINES);
                    for (int i = 0; i < m_positionsList[k].length() - 1; i++)
                    {
                        if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleColorByVelocity).toBool())
                            glColor3d(1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                      1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin),
                                      1.0 - 0.8 * (m_velocitiesList[k][i].magnitude() - velocityMin) / (velocityMax - velocityMin));

                        glVertex3d(m_positionsList[k][i].x * cos(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI),
                                   m_positionsList[k][i].y,
                                   m_positionsList[k][i].x * sin(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI));
                        glVertex3d(m_positionsList[k][i+1].x * cos(m_positionsList[k][i+1].z + l * stepAngle/180.0 * M_PI),
                                m_positionsList[k][i+1].y,
                                m_positionsList[k][i+1].x * sin(m_positionsList[k][i+1].z + l * stepAngle/180.0 * M_PI));

                    }
                    glEnd();

                    // points
                    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleShowPoints).toBool())
                    {
                        glColor3d(0.0, 0.0, 0.0);
                        glPointSize(NODESIZE);

                        glBegin(GL_POINTS);
                        for (int i = 0; i < m_positionsList[k].length(); i++)
                        {
                            glVertex3d(m_positionsList[k][i].x * cos(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI),
                                       m_positionsList[k][i].y,
                                       m_positionsList[k][i].x * sin(m_positionsList[k][i].z + l * stepAngle/180.0 * M_PI));
                        }
                        glEnd();
                    }
                }
            }
        }

        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_POINT_SMOOTH);

        glEndList();

        glCallList(m_listParticleTracing);
    }
    else
    {
        glCallList(m_listParticleTracing);
    }
}

void SceneViewParticleTracing::paintParticleTracingColorBar(double min, double max)
{
    if (!m_postprocessorWidget->currentComputation()->isSolved()) return;

    loadProjectionViewPort();

    glScaled(2.0 / width(), 2.0 / height(), 1.0);
    glTranslated(-width() / 2.0, -height() / 2.0, 0.0);

    // dimensions
    int textWidth = (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) * (QString::number(-1.0, 'e', m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarDecimalPlace).toInt()).length() + 1);
    int textHeight = 2 * (m_charDataPost[GLYPH_M].y1 - m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // contextHeight() - 20.0
    Point scaleBorder = Point(10.0, (Agros2D::configComputer()->value(Config::Config_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (width() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
              0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glBegin(GL_QUADS);
    glColor3d(0.0, 0.0, 0.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glColor3d(0.8, 0.8, 0.8);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    // ticks
    glColor3d(0.0, 0.0, 0.0);
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    glColor3d(0.0, 0.0, 0.0);
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = min + (double) (i-1) / (numTicks-1) * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0) : 0.0),
                    scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                    QString::number(value, 'e', m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarDecimalPlace).toInt()));
    }

    // variable
    QString str = QString("%1 (m/s)").arg(tr("Vel."));

    printPostAt(scaleLeft + scaleSize.x / 2.0 - (m_charDataPost[GLYPH_M].x1 - m_charDataPost[GLYPH_M].x0)  * str.count() / 2.0,
                scaleBorder.y + scaleSize.y - 20.0,
                str);
}

void SceneViewParticleTracing::clearGLLists()
{
    if (m_listParticleTracing != -1) glDeleteLists(m_listParticleTracing, 1);

    m_listParticleTracing = -1;

    clearParticleLists();
}

void SceneViewParticleTracing::refresh()
{
    clearGLLists();

    setControls();

    if (!m_postprocessorWidget->currentComputation().isNull() && m_postprocessorWidget->currentComputation()->isSolved())
        SceneViewCommon::refresh();
}

void SceneViewParticleTracing::setControls()
{
    if (!m_postprocessorWidget->currentComputation().isNull())
    {
        actSceneModeParticleTracing->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
        actSetProjectionXY->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
        actSetProjectionXZ->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
        actSetProjectionYZ->setEnabled(m_postprocessorWidget->currentComputation()->isSolved());
    }
}

void SceneViewParticleTracing::clear()
{
    clearParticleLists();

    setControls();

    SceneViewCommon3D::clear();
    doZoomBestFit();
}

void SceneViewParticleTracing::clearParticleLists()
{
    // clear lists
    foreach (QList<Point3> list, m_positionsList)
        list.clear();
    m_positionsList.clear();

    foreach (QList<Point3> list, m_velocitiesList)
        list.clear();
    m_velocitiesList.clear();

    foreach (QList<double> list, m_timesList)
        list.clear();
    m_timesList.clear();

    m_velocityMin = 0.0;
    m_velocityMax = 0.0;
}

void SceneViewParticleTracing::processParticleTracing()
{
    QTime cpuTime;
    cpuTime.start();

    clearParticleLists();

    if (!m_postprocessorWidget->currentComputation().isNull() && m_postprocessorWidget->currentComputation()->isSolved())
    {
        Agros2D::log()->printMessage(tr("Post View"), tr("Particle view"));

        m_velocityMin =  numeric_limits<double>::max();
        m_velocityMax = -numeric_limits<double>::max();

        QList<Point3> initialPositions;
        QList<Point3> initialVelocities;
        QList<double> particleCharges;
        QList<double> particleMasses;

        try
        {
            for (int k = 0; k < m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt(); k++)
            {
                // initial position
                Point3 initialPosition;
                initialPosition.x = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartX).toDouble();
                initialPosition.y = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartY).toDouble();
                initialPosition.z = 0.0;

                // initial velocity
                Point3 initialVelocity;
                initialVelocity.x = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartVelocityX).toDouble();
                initialVelocity.y = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartVelocityY).toDouble();
                initialVelocity.z = 0.0;

                // random point
                if (k > 0)
                {
                    Point3 dp(rand() * (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartingRadius).toDouble()) / RAND_MAX,
                              rand() * (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartingRadius).toDouble()) / RAND_MAX,
                              (m_postprocessorWidget->currentComputation()->config()->coordinateType() == CoordinateType_Planar) ? 0.0 : rand() * 2.0*M_PI / RAND_MAX);

                    initialPosition = Point3(-m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartingRadius).toDouble() / 2,
                                             -m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleStartingRadius).toDouble() / 2,
                                             (m_postprocessorWidget->currentComputation()->config()->coordinateType() == CoordinateType_Planar) ? 0.0 : -1.0*M_PI) + initialPosition + dp;
                }

                initialPositions.append(initialPosition);
                initialVelocities.append(initialVelocity);
                particleCharges.append(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleConstant).toDouble());
                particleMasses.append(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleMass).toDouble());
            }

            // position and velocity cache
            ParticleTracing particleTracing(m_postprocessorWidget->currentComputation().data(), particleMasses);
            ParticleTracingForceCustom forceCustom(&particleTracing);
            ParticleTracingForceDrag forceDrag(&particleTracing);
            ParticleTracingForceField forceField(&particleTracing, particleCharges);
            ParticleTracingForceFieldP2P forceFieldP2P(&particleTracing, particleCharges, particleMasses);

            particleTracing.computeTrajectoryParticles(initialPositions, initialVelocities);

            m_positionsList = particleTracing.positions();
            m_velocitiesList = particleTracing.velocities();
            m_timesList = particleTracing.times();

            // velocity min and max value
            m_velocityMin = particleTracing.velocityModuleMin();
            m_velocityMax = particleTracing.velocityModuleMax();
        }
        catch (AgrosException& e)
        {
            Agros2D::log()->printWarning(tr("Particle tracing"), tr("Particle tracing failed (%1)").arg(e.what()));
            m_velocityMin = 0.0;
            m_velocityMax = 0.0;

            return;
        }
        catch (...)
        {
            Agros2D::log()->printWarning(tr("Particle tracing"), tr("Catched unknown exception in particle tracing"));
            m_velocityMin = 0.0;
            m_velocityMax = 0.0;

            return;
        }

        for (int k = 0; k < m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt(); k++)
            Agros2D::log()->printMessage(tr("Particle Tracing"), tr("Particle %1: %2 steps, final time %3 s").
                                         arg(k + 1).
                                         arg(m_timesList[k].count()).
                                         arg(m_timesList[k].last()));
    }
    Agros2D::log()->printDebug(tr("Particle Tracing"), tr("Total cpu time %1 ms").arg(cpuTime.elapsed()));

    refresh();
}

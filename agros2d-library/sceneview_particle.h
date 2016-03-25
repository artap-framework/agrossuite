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

#ifndef SCENEVIEWPARTICLETRACING_H
#define SCENEVIEWPARTICLETRACING_H

#include "util.h"
#include "sceneview_common3d.h"

class PostprocessorWidget;

class SceneViewParticleTracing : public SceneViewCommon3D, public SceneViewPostInterface
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();
    void processParticleTracing();

public:
    SceneViewParticleTracing(PostprocessorWidget *postprocessorWidget);
    ~SceneViewParticleTracing();

    QAction *actSceneModeParticleTracing;

    virtual QIcon iconView() { return icon("scene-particle"); }
    virtual QString labelView() { return tr("Particle Tracing"); }

    void setControls();

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual ProblemBase *problem() const;

    void paintGeometryOutline();
    void paintGeometrySurface(bool blend = false);
    void paintParticleTracing();
    void paintParticleTracingColorBar(double min, double max);

private:
    PostprocessorWidget *m_postprocessorWidget;

    int m_listParticleTracing;

    void createActionsParticleTracing();

    // particle tracing
    ParticleTracing *particleTracing;
    QList<QList<Point3> > m_positionsList;
    QList<QList<Point3> > m_velocitiesList;
    QList<QList<double> > m_timesList;
    double m_positionMin;
    double m_positionMax;
    double m_velocityMin;
    double m_velocityMax;

    inline bool particleTracingIsPrepared() { return !m_positionsList.isEmpty(); }

private slots:
    virtual void clearGLLists();
    void clearParticleLists();
};

#endif // SCENEVIEWPARTICLETRACING_H

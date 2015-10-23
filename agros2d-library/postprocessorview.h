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

#ifndef POSTPROCESSORVIEW_H
#define POSTPROCESSORVIEW_H

#include "util.h"

class PostDeal;
class SceneViewPreprocessor;
class SceneViewMesh;
class SceneViewPost2D;
class SceneViewPost3D;
class SceneViewParticleTracing;
class SceneViewChart;
class LineEditDouble;
class CollapsableGroupBoxButton;
class FieldInfo;
class ValueLineEdit;
class PhysicalFieldWidget;
class ProblemComputation;

class PostprocessorSceneMeshWidget;
class PostprocessorScenePost2DWidget;
class PostprocessorSceneChartWidget;
class PostprocessorSceneParticleTracingWidget;

class PostprocessorSceneWidget;

enum PostprocessorWidgetMode
{
    PostprocessorWidgetMode_Mesh,
    PostprocessorWidgetMode_Post2D,
    PostprocessorWidgetMode_Post3D,
    PostprocessorWidgetMode_Chart,
    PostprocessorWidgetMode_ParticleTracing
};

class PostprocessorWidget : public QWidget
{
    Q_OBJECT

public:
    PostprocessorWidget();

    inline PhysicalFieldWidget *fieldWidget() { return m_fieldWidget; }

    PostprocessorWidgetMode mode();

    inline SceneViewMesh *sceneViewMesh() { return m_sceneViewMesh; }
    inline SceneViewPost2D *sceneViewPost2D() { return m_sceneViewPost2D; }
    inline SceneViewParticleTracing *sceneViewParticleTracing() { return m_sceneViewParticleTracing; }
    inline SceneViewChart *sceneViewChart() { return m_sceneViewChart; }

    inline QSharedPointer<ProblemComputation> computation() { return m_computation; }

    QAction *actSceneModePost;

signals:
    void apply();
    void modeChanged();

public slots:
    void refresh();
    void doApply();

private:
    PhysicalFieldWidget *m_fieldWidget;
    QSharedPointer<ProblemComputation> m_computation;

    QTabWidget *tabWidget;
    QPushButton *btnApply;

    // mesh and polynomial info
    QLabel *lblMeshInitial;
    QLabel *lblMeshSolution;
    QLabel *lblDOFs;

    PostprocessorSceneMeshWidget *m_meshWidget;
    PostprocessorScenePost2DWidget *m_post2DWidget;
    PostprocessorSceneChartWidget *m_chartWidget;
    PostprocessorSceneParticleTracingWidget *m_particleTracingWidget;

    SceneViewMesh *m_sceneViewMesh;
    SceneViewPost2D *m_sceneViewPost2D;
    SceneViewParticleTracing *m_sceneViewParticleTracing;
    SceneViewChart *m_sceneViewChart;

    void createControls();

    friend class PostprocessorSceneWidget;

private slots:
    void connectComputation(QSharedPointer<ProblemComputation> computation);

    void doCalculationFinished();
};

class PostprocessorSceneWidget : public QWidget
{
    Q_OBJECT

public:
    PostprocessorSceneWidget(PostprocessorWidget *postprocessorWidget)
        : QWidget(postprocessorWidget), m_postprocessorWidget(postprocessorWidget)
    {
    }

protected:
    PostprocessorWidget *m_postprocessorWidget;
};


#endif // POSTPROCESSORVIEW_H

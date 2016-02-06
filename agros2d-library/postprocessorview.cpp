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

#include "postprocessorview.h"
#include "postprocessorview_mesh.h"
#include "postprocessorview_post2d.h"
#include "postprocessorview_post3d.h"
#include "postprocessorview_chart.h"
#include "postprocessorview_particletracing.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "chartdialog.h"

#include "pythonlab/pythonengine_agros.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorWidget::PostprocessorWidget()
{
    // scene mode
    actSceneModeResults = new QAction(icon("results"), tr("Results"), this);
    actSceneModeResults->setShortcut(tr("Ctrl+3"));
    actSceneModeResults->setCheckable(true);
    actSceneModeResults->setEnabled(false);

    m_fieldWidget = new PhysicalFieldWidget(this);
    connect(m_fieldWidget, SIGNAL(fieldChanged()), this, SLOT(refresh()));

    m_sceneViewMesh = new SceneViewMesh(this);
    m_meshWidget = new PostprocessorSceneMeshWidget(this, m_sceneViewMesh);

    m_sceneViewPost2D = new SceneViewPost2D(this);
    m_post2DWidget = new PostprocessorScenePost2DWidget(this, m_sceneViewPost2D);

    m_sceneViewPost3D = new SceneViewPost3D(this);
    m_post3DWidget = new PostprocessorScenePost3DWidget(this, m_sceneViewPost3D);

    m_sceneViewChart = new SceneViewChart(this);
    m_chartWidget = new PostprocessorSceneChartWidget(this, m_sceneViewChart);

    m_sceneViewParticleTracing = new SceneViewParticleTracing(this);
    m_particleTracingWidget = new PostprocessorSceneParticleTracingWidget(this, m_sceneViewParticleTracing);

    createControls();
}

void PostprocessorWidget::createControls()
{    
    // dialog buttons
    btnApply = new QPushButton(tr("Apply"));
    connect(btnApply, SIGNAL(clicked()), SLOT(updateSettings()));

    tabWidget = new QTabWidget();
    tabWidget->addTab(m_meshWidget, tr("Mesh"));
    tabWidget->addTab(m_post2DWidget, tr("2D view"));
    tabWidget->addTab(m_post3DWidget, tr("3D view"));
    tabWidget->addTab(m_particleTracingWidget, tr("Part. tracing"));
    tabWidget->addTab(m_chartWidget, tr("Chart"));
    connect(tabWidget, SIGNAL(currentChanged(int)), SIGNAL(modeChanged()));

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(m_fieldWidget);
    layoutMain->addWidget(tabWidget);
    layoutMain->addWidget(btnApply, 0, Qt::AlignRight);

    setLayout(layoutMain);
}

void PostprocessorWidget::updateSettings()
{
    // update field widget
    m_fieldWidget->updateControls();

    // PostprocessorSceneWidget *widget = qobject_cast<PostprocessorSceneWidget>(tabWidget->currentWidget());
    // widget->save();

    // save settings
    if (tabWidget->currentWidget() == m_meshWidget)
    {
        m_meshWidget->save();
    }
    else if (tabWidget->currentWidget() == m_post2DWidget)
    {
        m_post2DWidget->save();
    }
    else if (tabWidget->currentWidget() == m_post3DWidget)
    {
        m_post3DWidget->save();
    }
    else if (tabWidget->currentWidget() == m_chartWidget)
    {
        m_chartWidget->save();
    }
    else if (tabWidget->currentWidget() == m_particleTracingWidget)
    {
        m_particleTracingWidget->save();
    }

    // refresh view
    refresh();

    // refresh view
    m_computation->postDeal()->refresh();

    emit changed();
}

void PostprocessorWidget::refresh()
{
    if (currentPythonEngine()->isScriptRunning())
        return;

    actSceneModeResults->setEnabled(Agros2D::computations().count() > 0);
    if (Agros2D::computations().count() == 0)
    {
        if (!m_computation.isNull())
        {
            m_computation = QSharedPointer<Computation>(nullptr);
            // connect computation
            emit connectComputation(m_computation);
        }

        return;
    }

    // update field widget
    m_fieldWidget->updateControls();

    // set current computation
    if (m_computation != m_fieldWidget->selectedComputation())
    {
        m_computation = m_fieldWidget->selectedComputation();
        // connect computation
        emit connectComputation(m_computation);

        if (!m_computation.isNull())
        {
            // refresh widgets
            m_meshWidget->refresh();
            m_post2DWidget->refresh();
            m_post3DWidget->refresh();
            m_chartWidget->refresh();
            m_particleTracingWidget->refresh();

            // default widget
            if (m_computation->isMeshed() && !m_computation->isSolved())
                tabWidget->setCurrentWidget(m_meshWidget);
            else if (m_computation->isSolved())
                tabWidget->setCurrentWidget(m_post2DWidget);

            // refresh views
            m_sceneViewMesh->setControls();
            m_sceneViewPost2D->setControls();
            m_sceneViewPost3D->setControls();
            m_sceneViewParticleTracing->setControls();
            m_sceneViewChart->refresh();
        }
    }

    if (m_computation.isNull())
        return;

    tabWidget->setTabEnabled(0, m_computation->isMeshed());
    tabWidget->setTabEnabled(1, m_computation->isSolved());
    tabWidget->setTabEnabled(2, m_computation->isSolved());
    tabWidget->setTabEnabled(3, m_computation->isSolved());
    tabWidget->setTabEnabled(4, m_computation->isSolved());

    m_meshWidget->load();
    m_post2DWidget->load();
    m_post3DWidget->load();
    m_chartWidget->load();
    m_particleTracingWidget->load();

    if (m_computation->isSolved())
    {
        m_computation->postDeal()->setActiveViewField(m_fieldWidget->selectedField());
        m_computation->postDeal()->setActiveTimeStep(m_fieldWidget->selectedTimeStep());
        m_computation->postDeal()->setActiveAdaptivityStep(m_fieldWidget->selectedAdaptivityStep());
    }
}

PostprocessorWidgetMode PostprocessorWidget::mode()
{
    if (tabWidget->currentWidget() == m_meshWidget)
        return PostprocessorWidgetMode_Mesh;
    else if (tabWidget->currentWidget() == m_post2DWidget)
        return PostprocessorWidgetMode_Post2D;
    else if (tabWidget->currentWidget() == m_post3DWidget)
        return PostprocessorWidgetMode_Post3D;
    else if (tabWidget->currentWidget() == m_chartWidget)
        return PostprocessorWidgetMode_Chart;
    else if (tabWidget->currentWidget() == m_particleTracingWidget)
        return PostprocessorWidgetMode_ParticleTracing;
    else
        assert(0);
}

PostprocessorSceneWidget::PostprocessorSceneWidget(PostprocessorWidget *postprocessorWidget)
    : QWidget(postprocessorWidget), m_postprocessorWidget(postprocessorWidget)
{
    connect(postprocessorWidget->fieldWidget(), SIGNAL(fieldChanged()), this, SLOT(refresh()));
}

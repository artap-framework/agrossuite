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
    m_meshWidget = new PostprocessorSceneMeshWidget(m_fieldWidget, m_sceneViewMesh);

    m_sceneViewPost2D = new SceneViewPost2D(this);
    m_post2DWidget = new PostprocessorScenePost2DWidget(m_fieldWidget, m_sceneViewPost2D);

    m_sceneViewPost3D = new SceneViewPost3D(this);
    m_post3DWidget = new PostprocessorScenePost3DWidget(m_fieldWidget, m_sceneViewPost3D);

    m_sceneViewChart = new SceneViewChart(this);
    m_chartWidget = new PostprocessorSceneChartWidget(m_fieldWidget, m_sceneViewChart);

    m_sceneViewParticleTracing = new SceneViewParticleTracing(this);
    m_particleTracingWidget = new PostprocessorSceneParticleTracingWidget(m_fieldWidget, m_sceneViewParticleTracing);

    createControls();
}

void PostprocessorWidget::createControls()
{    
    // dialog buttons
    btnApply = new QPushButton(tr("Apply"));
    connect(btnApply, SIGNAL(clicked()), SLOT(apply()));

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

void PostprocessorWidget::solvedWithGUI()
{
    // clear commputation
    m_currentComputation.clear();

    refresh();

    actSceneModeResults->trigger();

    m_sceneViewMesh->doZoomBestFit();
    m_sceneViewPost2D->doZoomBestFit();
    m_sceneViewPost3D->doZoomBestFit();
    m_sceneViewParticleTracing->doZoomBestFit();
}

void PostprocessorWidget::apply()
{
    if (m_currentComputation->isSolved())
    {
        // save settings
        if (tabWidget->currentWidget() == m_meshWidget)
            m_meshWidget->save();
        else if (tabWidget->currentWidget() == m_post2DWidget)
            m_post2DWidget->save();
        else if (tabWidget->currentWidget() == m_post3DWidget)
            m_post3DWidget->save();
        else if (tabWidget->currentWidget() == m_chartWidget)
            m_chartWidget->save();
        else if (tabWidget->currentWidget() == m_particleTracingWidget)
            m_particleTracingWidget->save();

        // refresh view
        m_currentComputation->postDeal()->setActiveViewField(m_fieldWidget->selectedField());
        m_currentComputation->postDeal()->setActiveTimeStep(m_fieldWidget->selectedTimeStep());
        m_currentComputation->postDeal()->setActiveAdaptivityStep(m_fieldWidget->selectedAdaptivityStep());
        m_currentComputation->postDeal()->refresh();

        // refresh view
        m_sceneViewMesh->refresh();
        m_sceneViewPost2D->refresh();
        m_sceneViewPost3D->refresh();
        m_sceneViewChart->refresh();
        m_sceneViewParticleTracing->refresh();

        emit changed();
    }
}

void PostprocessorWidget::refresh()
{
    if (currentPythonEngine()->isScriptRunning()) return;

    // enable widget
    bool enabled = false;
    foreach (QSharedPointer<Computation> computation, Agros2D::computations().values())
    {
        if (computation->isSolved())
        {
            enabled = true;
            break;
        }
    }
    actSceneModeResults->setEnabled(enabled);

    // reset computations
    if (Agros2D::computations().isEmpty())
        m_currentComputation.clear();

    // update field widget
    m_fieldWidget->updateControls();

    // set current computation
    bool computationChanged = m_currentComputation != m_fieldWidget->selectedComputation();
    if (computationChanged)
    {
        m_currentComputation = m_fieldWidget->selectedComputation();

        if (!m_currentComputation.isNull())
        {
            // refresh widgets
            m_meshWidget->refresh();
            m_post2DWidget->refresh();
            m_post3DWidget->refresh();
            m_chartWidget->refresh();
            m_particleTracingWidget->refresh();

            // default widget
            tabWidget->setCurrentWidget(m_post2DWidget);

            // clear post view
            m_currentComputation->postDeal()->clear();

            // refresh views
            m_sceneViewMesh->setControls();
            m_sceneViewPost2D->setControls();
            m_sceneViewPost3D->setControls();
            m_sceneViewParticleTracing->setControls();
            m_sceneViewChart->refresh();

            tabWidget->setTabEnabled(0, m_currentComputation->isSolved());
            tabWidget->setTabEnabled(1, m_currentComputation->isSolved());
            tabWidget->setTabEnabled(2, m_currentComputation->isSolved());
            tabWidget->setTabEnabled(3, m_currentComputation->isSolved());
            tabWidget->setTabEnabled(4, m_currentComputation->isSolved());
        }
    }

    if (m_currentComputation.isNull())
        return;

    m_meshWidget->load();
    m_post2DWidget->load();
    m_post3DWidget->load();
    m_chartWidget->load();
    m_particleTracingWidget->load();

    if (computationChanged)
        apply();
}

void PostprocessorWidget::clearedComputation()
{
    m_currentComputation.clear();
    refresh();
}

void PostprocessorWidget::processed()
{
    // refresh widgets
    m_meshWidget->refresh();
    m_post2DWidget->refresh();
    m_post3DWidget->refresh();
    m_chartWidget->refresh();
    m_particleTracingWidget->refresh();

    m_sceneViewMesh->refresh();
    m_sceneViewPost2D->refresh();
    m_sceneViewPost3D->refresh();
    m_sceneViewChart->refresh();
    m_sceneViewParticleTracing->refresh();
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

PostprocessorSceneWidget::PostprocessorSceneWidget(PhysicalFieldWidget *fieldWidget)
    : QWidget(fieldWidget), m_fieldWidget(fieldWidget)
{
    connect(fieldWidget, SIGNAL(fieldChanged()), this, SLOT(refresh()));
}

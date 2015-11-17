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
    actSceneModeResults = new QAction(icon("scene-post2d"), tr("Results"), this);
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

    // sceneViewPost3D = new SceneViewPost3D(postDeal, this);

    createControls();

    // connect(currentPythonEngine(), SIGNAL(executedScript()), this, SLOT(doCalculationFinished()));

    // reconnect computation slots
    connect(Agros2D::singleton(), SIGNAL(connectComputation(QSharedPointer<Computation>)), this, SLOT(connectComputation(QSharedPointer<Computation>)));
}

void PostprocessorWidget::createControls()
{    
    // dialog buttons
    btnApply = new QPushButton(tr("Apply"));
    connect(btnApply, SIGNAL(clicked()), SLOT(doApply()));

    // mesh and polynomial info
    lblMeshInitial = new QLabel();
    lblMeshSolution = new QLabel();
    lblDOFs = new QLabel();

    QGridLayout *layoutInfo = new QGridLayout();
    layoutInfo->addWidget(new QLabel(tr("Initial mesh:")), 0, 0);
    layoutInfo->addWidget(lblMeshInitial, 0, 1);
    layoutInfo->addWidget(new QLabel(tr("Solution mesh:")), 1, 0);
    layoutInfo->addWidget(lblMeshSolution, 1, 1);
    layoutInfo->addWidget(new QLabel(tr("Number of DOFs:")), 2, 0);
    layoutInfo->addWidget(lblDOFs, 2, 1);

    tabWidget = new QTabWidget();
    tabWidget->addTab(m_meshWidget, icon("scene-mesh"), tr("Mesh"));
    tabWidget->addTab(m_post2DWidget, icon("scene-post2d"), tr("2D"));
    tabWidget->addTab(m_post3DWidget, icon("scene-post3d"), tr("3D"));
    tabWidget->addTab(m_chartWidget, icon("chart"), tr("Chart"));
    tabWidget->addTab(m_particleTracingWidget, icon("scene-particle"), tr("PT"));
    connect(tabWidget, SIGNAL(currentChanged(int)), SIGNAL(modeChanged()));

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(m_fieldWidget);
    layoutMain->addWidget(tabWidget);
    // layoutMain->addStretch(1);
    layoutMain->addLayout(layoutInfo);
    layoutMain->addWidget(btnApply, 0, Qt::AlignRight);

    setLayout(layoutMain);
}

void PostprocessorWidget::doApply()
{
    if (m_computation->isSolved())
    {
        m_computation->postDeal()->setActiveViewField(fieldWidget()->selectedField());
        m_computation->postDeal()->setActiveTimeStep(fieldWidget()->selectedTimeStep());
        m_computation->postDeal()->setActiveAdaptivityStep(fieldWidget()->selectedAdaptivityStep());
    }

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

    // update field widget
    m_fieldWidget->updateControls();

    // refresh
    emit apply();

    activateWindow();
}

void PostprocessorWidget::refresh()
{
    actSceneModeResults->setEnabled(m_computation && m_computation->isMeshed());

    if (m_computation.isNull())
        return;

    tabWidget->setTabEnabled(0, m_computation->isMeshed());
    tabWidget->setTabEnabled(1, m_computation->isSolved());
    tabWidget->setTabEnabled(2, m_computation->isSolved());
    tabWidget->setTabEnabled(3, m_computation->isSolved());
    tabWidget->setTabEnabled(4, m_computation->isSolved());

    // mesh and polynomial info
    int dofs = 0;
    if (m_computation->isMeshed())
    {
        lblMeshInitial->setText(QString(tr("%1 nodes, %2 elements").
                                        arg(m_computation->initialMesh().n_used_vertices()).
                                        arg(m_computation->initialMesh().n_active_cells())));
        lblMeshSolution->setText(QString(tr("%1 nodes, %2 elements").
                                         arg(m_computation->calculationMesh().n_used_vertices()).
                                         arg(m_computation->calculationMesh().n_active_cells())));
    }

    if (m_computation->isSolved() && m_fieldWidget->selectedField())
    {
        MultiArray ma = m_computation->solutionStore()->multiArray(FieldSolutionID(m_fieldWidget->selectedField()->fieldId(),
                                                                                   m_fieldWidget->selectedTimeStep(),
                                                                                   m_fieldWidget->selectedAdaptivityStep()));

        dofs = ma.doFHandler()->n_dofs();
    }
    lblDOFs->setText(tr("%1 DOFs").arg(dofs));

    m_meshWidget->load();
    m_post2DWidget->load();
    m_post3DWidget->load();
    m_chartWidget->load();
    m_particleTracingWidget->load();
}

void PostprocessorWidget::connectComputation(QSharedPointer<Computation> computation)
{
    if (!m_computation.isNull())
    {
        disconnect(m_computation.data(), SIGNAL(meshed()), this, SLOT(doCalculationFinished()));
        disconnect(m_computation.data(), SIGNAL(solved()), this, SLOT(doCalculationFinished()));
        disconnect(this, SIGNAL(apply()), m_computation.data()->postDeal(), SLOT(refresh()));
    }

    m_computation = computation;

    if (!m_computation.isNull())
    {
        connect(m_computation.data(), SIGNAL(meshed()), this, SLOT(doCalculationFinished()));
        connect(m_computation.data(), SIGNAL(solved()), this, SLOT(doCalculationFinished()));
        connect(this, SIGNAL(apply()), m_computation.data()->postDeal(), SLOT(refresh()));        
    }    

    refresh();
}

void PostprocessorWidget::doCalculationFinished()
{
    if (!m_computation)
        return;

    if (currentPythonEngine()->isScriptRunning())
        return;

    actSceneModeResults->setEnabled(m_computation->isMeshed());

    if (m_computation->isSolved() && m_computation->postDeal()->isProcessed())
    {
        m_fieldWidget->selectField(m_computation->postDeal()->activeViewField());
        m_fieldWidget->selectTimeStep(m_computation->postDeal()->activeTimeStep());
        m_fieldWidget->selectAdaptivityStep(m_computation->postDeal()->activeAdaptivityStep());
    }

    m_meshWidget->refresh();
    m_meshWidget->load();
    m_post2DWidget->refresh();
    m_post2DWidget->load();
    m_post3DWidget->refresh();
    m_post3DWidget->load();
    m_chartWidget->refresh();
    m_chartWidget->load();
    m_particleTracingWidget->refresh();
    m_particleTracingWidget->load();

    // default widget
    if (m_computation->isMeshed() && !m_computation->isSolved())
        tabWidget->setCurrentWidget(m_meshWidget);
    else if (m_computation->isSolved())
        tabWidget->setCurrentWidget(m_post2DWidget);

    if (m_computation->isMeshed() || m_computation->isSolved())
        emit apply();
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

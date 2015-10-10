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
#include "pythonlab/pythonengine_agros.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorWidget::PostprocessorWidget(PostDeal *postDeal,
                                         SceneViewMesh *sceneMesh,
                                         SceneViewPost2D *scenePost2D)
    : m_postDeal(postDeal)
{
    // scene mode
    actSceneModePost = new QAction(icon("scene-post2d"), tr("Postprocessor"), this);
    actSceneModePost->setShortcut(tr("Ctrl+3"));
    actSceneModePost->setCheckable(true);
    actSceneModePost->setEnabled(false);

    m_fieldWidget = new PhysicalFieldWidget(this);
    // connect(m_fieldWidget, SIGNAL(fieldChanged()), this, SLOT(doField()));

    meshWidget = new PostprocessorSceneMeshWidget(this, sceneMesh, this);
    post2DWidget = new PostprocessorScenePost2DWidget(this, scenePost2D, this);

    createControls();

    connect(currentPythonEngine(), SIGNAL(executedScript()), this, SLOT(doCalculationFinished()));

    // reconnect computation slots
    connect(Agros2D::singleton(), SIGNAL(reconnectSlots()), this, SLOT(reconnectActions()));
}

void PostprocessorWidget::createControls()
{    
    // dialog buttons
    btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

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
    tabWidget->addTab(meshWidget, icon("scene-mesh"), tr("Mesh"));
    tabWidget->addTab(post2DWidget, icon("scene-post2d"), tr("2D"));
    connect(tabWidget, SIGNAL(currentChanged(int)), SIGNAL(modeChanged()));

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);    
    layoutMain->addWidget(m_fieldWidget);
    layoutMain->addWidget(tabWidget);
    // layoutMain->addStretch(1);
    layoutMain->addLayout(layoutInfo);
    layoutMain->addWidget(btnOK, 0, Qt::AlignRight);

    setLayout(layoutMain);
}

void PostprocessorWidget::doApply()
{
    m_fieldWidget->updateControls();

    m_postDeal->setActiveViewField(m_fieldWidget->selectedField());
    m_postDeal->setActiveTimeStep(m_fieldWidget->selectedTimeStep());
    m_postDeal->setActiveAdaptivityStep(m_fieldWidget->selectedAdaptivityStep());

    if (tabWidget->currentWidget() == meshWidget)
    {
        meshWidget->save();
        meshWidget->updateControls();
    }
    else if (tabWidget->currentWidget() == post2DWidget)
    {
        post2DWidget->save();
        post2DWidget->updateControls();
    }

    // refresh
    emit apply();

    activateWindow();
}

void PostprocessorWidget::reconnectActions()
{
    connect(Agros2D::computation(), SIGNAL(meshed()), this, SLOT(doCalculationFinished()));
    connect(Agros2D::computation(), SIGNAL(solved()), this, SLOT(doCalculationFinished()));
}

void PostprocessorWidget::doCalculationFinished()
{
    if (!Agros2D::computation())
        return;

    if (currentPythonEngine()->isScriptRunning())
        return;

    actSceneModePost->setEnabled(Agros2D::computation()->isMeshed());

    if (Agros2D::computation()->isSolved())
    {
        m_fieldWidget->selectField(m_postDeal->activeViewField());
        m_fieldWidget->selectTimeStep(m_postDeal->activeTimeStep());
        m_fieldWidget->selectAdaptivityStep(m_postDeal->activeAdaptivityStep());
    }

    // mesh and polynomial info
    int dofs = 0;
    if (Agros2D::computation()->isMeshed())
    {
        lblMeshInitial->setText(QString(tr("%1 nodes, %2 elements").
                                        arg(Agros2D::computation()->initialMesh().n_used_vertices()).
                                        arg(Agros2D::computation()->initialMesh().n_active_cells())));
        lblMeshSolution->setText(QString(tr("%1 nodes, %2 elements").
                                         arg(Agros2D::computation()->calculationMesh().n_used_vertices()).
                                         arg(Agros2D::computation()->calculationMesh().n_active_cells())));
    }
    if (Agros2D::computation()->isSolved())
    {
        MultiArray ma = Agros2D::solutionStore()->multiArray(FieldSolutionID(m_fieldWidget->selectedField(),
                                                                             m_fieldWidget->selectedTimeStep(),
                                                                             m_fieldWidget->selectedAdaptivityStep()));

        dofs = ma.doFHandler()->n_dofs();
    }
    lblDOFs->setText(tr("%1 DOFs").arg(dofs));

    if ((Agros2D::computation()->isMeshed() && !Agros2D::computation()->isSolving()) || Agros2D::computation()->isSolved())
        emit apply();
}

PostprocessorWidgetMode PostprocessorWidget::mode()
{
    if (tabWidget->currentWidget() == meshWidget)
        return PostprocessorWidgetMode_Mesh;
    else if (tabWidget->currentWidget() == post2DWidget)
        return PostprocessorWidgetMode_Post2D;
}

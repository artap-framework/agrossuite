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

#include "postprocessorview_mesh.h"
#include "postprocessorview.h"

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

PostprocessorSceneMeshWidget::PostprocessorSceneMeshWidget(PostprocessorWidget *postprocessorWidget, SceneViewMesh *sceneMesh)
    : PostprocessorSceneWidget(postprocessorWidget), m_sceneMesh(sceneMesh)
{
    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorMeshWidget");

    createControls();
}

void PostprocessorSceneMeshWidget::createControls()
{
    // layout mesh
    chkShowInitialMeshView = new QCheckBox(tr("Initial mesh"));
    chkShowSolutionMeshView = new QCheckBox(tr("Solution mesh"));
    chkShowOrderView = new QCheckBox(tr("Polynomial order"));
    connect(chkShowOrderView, SIGNAL(clicked()), this, SLOT(refresh()));

    txtOrderComponent = new QSpinBox(this);
    txtOrderComponent->setMinimum(1);
    txtOrderComponent->setVisible(false); // NOT IMPLEMENTED

    QGridLayout *gridLayoutMesh = new QGridLayout();
    gridLayoutMesh->addWidget(chkShowInitialMeshView, 0, 0, 1, 2);
    gridLayoutMesh->addWidget(chkShowSolutionMeshView, 1, 0, 1, 2);
    gridLayoutMesh->addWidget(chkShowOrderView, 2, 0, 1, 2);
    // gridLayoutMesh->addWidget(new QLabel(tr("Component:")), 3, 0);
    // gridLayoutMesh->addWidget(txtOrderComponent, 3, 1);

    QGroupBox *grpShowMesh = new QGroupBox(tr("Mesh"));
    grpShowMesh->setLayout(gridLayoutMesh);

    // layout order
    cmbOrderPaletteOrder = new QComboBox();
    foreach (QString key, paletteTypeStringKeys())
        cmbOrderPaletteOrder->addItem(paletteTypeString(paletteTypeFromStringKey(key)), paletteTypeFromStringKey(key));

    chkShowOrderColorbar = new QCheckBox(tr("Show colorbar"), this);
    chkOrderLabel = new QCheckBox(tr("Show labels"), this);

    QGridLayout *gridLayoutOrder = new QGridLayout();
    gridLayoutOrder->setColumnStretch(1, 1);
    gridLayoutOrder->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutOrder->addWidget(cmbOrderPaletteOrder, 0, 1);
    gridLayoutOrder->addWidget(chkShowOrderColorbar, 0, 2);
    gridLayoutOrder->addWidget(chkOrderLabel, 2, 2);

    QGroupBox *grpShowOrder = new QGroupBox(tr("Polynomial order"));
    grpShowOrder->setLayout(gridLayoutOrder);

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

    QGroupBox *grpInfo = new QGroupBox(tr("Mesh and polynomial order"));
    grpInfo->setLayout(layoutInfo);

    QVBoxLayout *widgetsLayout = new QVBoxLayout();
    widgetsLayout->addWidget(grpShowMesh);
    widgetsLayout->addWidget(grpShowOrder);
    widgetsLayout->addStretch(1);
    widgetsLayout->addWidget(grpInfo);

    setLayout(widgetsLayout);
}

void PostprocessorSceneMeshWidget::refresh()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    // mesh and order
    chkShowInitialMeshView->setEnabled(m_postprocessorWidget->computation()->isMeshed());
    chkShowSolutionMeshView->setEnabled(m_postprocessorWidget->computation()->isSolved());
    chkShowOrderView->setEnabled(m_postprocessorWidget->computation()->isSolved());
    txtOrderComponent->setEnabled(m_postprocessorWidget->computation()->isSolved() && (chkShowOrderView->isChecked() || chkShowSolutionMeshView->isChecked()));
    txtOrderComponent->setMaximum(m_postprocessorWidget->fieldWidget()->selectedField()->numberOfSolutions());
}

void PostprocessorSceneMeshWidget::load()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    // show
    chkShowInitialMeshView->setChecked(m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_ShowInitialMeshView).toBool());
    chkShowSolutionMeshView->setChecked(m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_ShowSolutionMeshView).toBool());
    chkShowOrderView->setChecked(m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_ShowOrderView).toBool());
    txtOrderComponent->setValue(m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_OrderComponent).toInt());

    // order view
    chkShowOrderColorbar->setChecked(m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_ShowOrderColorBar).toBool());
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteType) m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_OrderPaletteOrderType).toInt()));
    chkOrderLabel->setChecked(m_postprocessorWidget->computation()->setting()->value(PostprocessorSetting::View_ShowOrderLabel).toBool());

    // mesh and polynomial info
    int dofs = 0;
    if (m_postprocessorWidget->computation()->isMeshed())
    {
        lblMeshInitial->setText(QString(tr("%1 nodes, %2 elements").
                                        arg(m_postprocessorWidget->computation()->initialMesh().n_used_vertices()).
                                        arg(m_postprocessorWidget->computation()->initialMesh().n_active_cells())));
        lblMeshSolution->setText(QString(tr("%1 nodes, %2 elements").
                                         arg(m_postprocessorWidget->computation()->calculationMesh().n_used_vertices()).
                                         arg(m_postprocessorWidget->computation()->calculationMesh().n_active_cells())));
    }

    if (m_postprocessorWidget->computation()->isSolved() && m_postprocessorWidget->fieldWidget()->selectedField())
    {
        MultiArray ma = m_postprocessorWidget->computation()->solutionStore()->multiArray(FieldSolutionID(m_postprocessorWidget->fieldWidget()->selectedField()->fieldId(),
                                                                                                          m_postprocessorWidget->fieldWidget()->selectedTimeStep(),
                                                                                                          m_postprocessorWidget->fieldWidget()->selectedAdaptivityStep()));

        dofs = ma.doFHandler().n_dofs();
    }
    lblDOFs->setText(tr("%1 DOFs").arg(dofs));
}

void PostprocessorSceneMeshWidget::save()
{
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_ShowInitialMeshView, chkShowInitialMeshView->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_ShowSolutionMeshView, chkShowSolutionMeshView->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_ShowOrderView, chkShowOrderView->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_OrderComponent, txtOrderComponent->value());

    // order view
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_ShowOrderColorBar, chkShowOrderColorbar->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_OrderPaletteOrderType, (PaletteType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt());
    m_postprocessorWidget->computation()->setting()->setValue(PostprocessorSetting::View_ShowOrderLabel, chkOrderLabel->isChecked());
}

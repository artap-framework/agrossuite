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

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorSceneMeshWidget::PostprocessorSceneMeshWidget(PhysicalFieldWidget *fieldWidget, SceneViewMesh *sceneMesh)
    : PostprocessorSceneWidget(fieldWidget), m_sceneMesh(sceneMesh)
{
    setObjectName("PostprocessorMeshWidget");

    createControls();
}

void PostprocessorSceneMeshWidget::createControls()
{
    // layout mesh
    QGroupBox *groupBoxMesh = new QGroupBox(tr("Mesh"));
    rbShowNone = new QRadioButton(tr("None"));
    rbShowInitialMeshView = new QRadioButton(tr("Initial mesh"));
    rbShowSolutionMeshView = new QRadioButton(tr("Solution mesh"));

    QVBoxLayout *vboxMesh = new QVBoxLayout;
    vboxMesh->addWidget(rbShowNone);
    vboxMesh->addWidget(rbShowInitialMeshView);
    vboxMesh->addWidget(rbShowSolutionMeshView);
    vboxMesh->addStretch(1);
    groupBoxMesh->setLayout(vboxMesh);

    QGroupBox *groupBoxVolume = new QGroupBox(tr("View"));
    rbShowOrderView = new QRadioButton(tr("Polynomial order"));
    rbShowErrorView = new QRadioButton(tr("Error estimate"));

    QVBoxLayout *vboxVolume = new QVBoxLayout;
    vboxVolume->addWidget(rbShowOrderView);
    vboxVolume->addWidget(rbShowErrorView);
    vboxVolume->addStretch(1);
    groupBoxVolume->setLayout(vboxVolume);

    connect(rbShowOrderView, SIGNAL(clicked(bool)), this, SLOT(refresh()));
    connect(rbShowErrorView, SIGNAL(clicked(bool)), this, SLOT(refresh()));

    txtOrderComponent = new QSpinBox(this);
    txtOrderComponent->setMinimum(1);
    txtOrderComponent->setVisible(false); // NOT IMPLEMENTED

    QGridLayout *gridLayoutMesh = new QGridLayout();
    gridLayoutMesh->addWidget(groupBoxMesh, 0, 0, 1, 2);
    gridLayoutMesh->addWidget(groupBoxVolume, 1, 0, 1, 2);
    // gridLayoutMesh->addWidget(new QLabel(tr("Component:")), 3, 0);
    // gridLayoutMesh->addWidget(txtOrderComponent, 3, 1);

    QGroupBox *grpShowMesh = new QGroupBox(tr("Mesh"));
    grpShowMesh->setLayout(gridLayoutMesh);

    // layout order
    cmbOrderPaletteOrder = new QComboBox();
    foreach (QString key, paletteTypeStringKeys())
        cmbOrderPaletteOrder->addItem(paletteTypeString(paletteTypeFromStringKey(key)), paletteTypeFromStringKey(key));

    chkShowOrderColorbar = new QCheckBox(tr("Show colorbar"), this);    

    QGridLayout *gridLayoutOrder = new QGridLayout();
    gridLayoutOrder->setColumnStretch(1, 1);
    gridLayoutOrder->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutOrder->addWidget(cmbOrderPaletteOrder, 0, 1);
    gridLayoutOrder->addWidget(chkShowOrderColorbar, 0, 2);

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
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;

    // mesh
    rbShowInitialMeshView->setEnabled(m_fieldWidget->selectedComputation()->isSolved());
    rbShowSolutionMeshView->setEnabled(m_fieldWidget->selectedComputation()->isSolved());

    // order
    rbShowOrderView->setEnabled(m_fieldWidget->selectedComputation()->isSolved());
    rbShowErrorView->setEnabled(m_fieldWidget->selectedComputation()->isSolved());

    txtOrderComponent->setEnabled(m_fieldWidget->selectedComputation()->isSolved() && (rbShowOrderView->isChecked() || rbShowSolutionMeshView->isChecked()));
    txtOrderComponent->setMaximum(m_fieldWidget->selectedField()->numberOfSolutions());
}

void PostprocessorSceneMeshWidget::load()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;

    // show mesh
    // default
    rbShowNone->setChecked(true);
    rbShowInitialMeshView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowInitialMeshView).toBool());
    rbShowSolutionMeshView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowSolutionMeshView).toBool());

    // show order
    rbShowOrderView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowOrderView).toBool());
    rbShowErrorView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowErrorView).toBool());
    txtOrderComponent->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::OrderComponent).toInt());

    // order view
    chkShowOrderColorbar->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowOrderColorBar).toBool());
    cmbOrderPaletteOrder->setCurrentIndex(cmbOrderPaletteOrder->findData((PaletteType) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::OrderPaletteOrderType).toInt()));

    // mesh and polynomial info
    int dofs = 0;
    if (m_fieldWidget->selectedComputation()->isMeshed())
    {
        lblMeshInitial->setText(QString(tr("%1 nodes, %2 elements").
                                        arg(m_fieldWidget->selectedComputation()->initialMesh().n_used_vertices()).
                                        arg(m_fieldWidget->selectedComputation()->initialMesh().n_active_cells())));
        lblMeshSolution->setText(QString(tr("%1 nodes, %2 elements").
                                         arg(m_fieldWidget->selectedComputation()->calculationMesh().n_used_vertices()).
                                         arg(m_fieldWidget->selectedComputation()->calculationMesh().n_active_cells())));
    }

    if (m_fieldWidget->selectedComputation()->isSolved() && m_fieldWidget->selectedField())
    {
        MultiArray ma = m_fieldWidget->selectedComputation()->solutionStore()->multiArray(FieldSolutionID(m_fieldWidget->selectedField()->fieldId(),
                                                                                                          m_fieldWidget->selectedTimeStep(),
                                                                                                          m_fieldWidget->selectedAdaptivityStep()));

        dofs = ma.doFHandler().n_dofs();
    }
    lblDOFs->setText(tr("%1 DOFs").arg(dofs));
}

void PostprocessorSceneMeshWidget::save()
{
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowInitialMeshView, rbShowInitialMeshView->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowSolutionMeshView, rbShowSolutionMeshView->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowOrderView, rbShowOrderView->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowErrorView, rbShowErrorView->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::OrderComponent, txtOrderComponent->value());

    // order view
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowOrderColorBar, chkShowOrderColorbar->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::OrderPaletteOrderType, (PaletteType) cmbOrderPaletteOrder->itemData(cmbOrderPaletteOrder->currentIndex()).toInt());
}


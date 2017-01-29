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

#include "postprocessorview_post3d.h"
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

PostprocessorScenePost3DWidget::PostprocessorScenePost3DWidget(PhysicalFieldWidget *fieldWidget, SceneViewPost3D *scenePost3D)
    : PostprocessorSceneWidget(fieldWidget), m_scenePost3D(scenePost3D)
{
    setObjectName("PostprocessorPost3DWidget");

    createControls();
}

void PostprocessorScenePost3DWidget::createControls()
{
    // layout post3d
    radPost3DNone = new QRadioButton(tr("None"), this);
    radPost3DScalarField3D = new QRadioButton(tr("Scalar view"), this);
    radPost3DScalarField3DSolid = new QRadioButton(tr("Scalar view solid"), this);
    radPost3DModel = new QRadioButton("Model", this);

    butPost3DGroup = new QButtonGroup(this);
    butPost3DGroup->addButton(radPost3DNone);
    butPost3DGroup->addButton(radPost3DScalarField3D);
    butPost3DGroup->addButton(radPost3DScalarField3DSolid);
    butPost3DGroup->addButton(radPost3DModel);

    QGridLayout *layoutPost3D = new QGridLayout();
    layoutPost3D->addWidget(radPost3DNone, 0, 0);
    layoutPost3D->addWidget(radPost3DScalarField3D, 1, 0);
    layoutPost3D->addWidget(radPost3DScalarField3DSolid, 2, 0);
    layoutPost3D->addWidget(radPost3DModel, 3, 0);

    QGroupBox *grpShowPost3D = new QGroupBox(tr("Postprocessor 3D"));
    grpShowPost3D->setLayout(layoutPost3D);

    lstSolidMaterials = new QListWidget();

    QGridLayout *gridLayoutSolid = new QGridLayout();
    gridLayoutSolid->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutSolid->setColumnStretch(0, 1);
    gridLayoutSolid->addWidget(lstSolidMaterials, 0, 0);

    QGroupBox *solidWidget = new QGroupBox(tr("Solid view"));
    solidWidget->setLayout(gridLayoutSolid);

    // layout 3d
    chkView3DLighting = new QCheckBox(tr("Ligthing"), this);
    txtView3DAngle = new QDoubleSpinBox(this);
    txtView3DAngle->setDecimals(1);
    txtView3DAngle->setSingleStep(1);
    txtView3DAngle->setMinimum(30);
    txtView3DAngle->setMaximum(360);
    chkView3DBackground = new QCheckBox(tr("Gradient back."), this);
    txtView3DHeight = new QDoubleSpinBox(this);
    txtView3DHeight->setDecimals(1);
    txtView3DHeight->setSingleStep(0.1);
    txtView3DHeight->setMinimum(0.2);
    txtView3DHeight->setMaximum(10.0);
    chkView3DBoundingBox = new QCheckBox(tr("Bounding box"), this);
    chkView3DSolidGeometry = new QCheckBox(tr("Show edges"), this);

    QGridLayout *layout3D = new QGridLayout();
    layout3D->setColumnMinimumWidth(0, columnMinimumWidth());
    layout3D->addWidget(new QLabel(tr("Angle:")), 0, 0);
    layout3D->addWidget(txtView3DAngle, 0, 1);
    layout3D->addWidget(new QLabel(tr("Height:")), 1, 0);
    layout3D->addWidget(txtView3DHeight, 1, 1);
    layout3D->addWidget(chkView3DLighting, 2, 0);
    layout3D->addWidget(chkView3DBackground, 3, 0);
    layout3D->addWidget(chkView3DBoundingBox, 2, 1);
    layout3D->addWidget(chkView3DSolidGeometry, 3, 1);

    QGroupBox *viewWidget = new QGroupBox(tr("Settings"));
    viewWidget->setLayout(layout3D);

    QVBoxLayout *layoutAll = new QVBoxLayout();
    layoutAll->addWidget(grpShowPost3D);
    layoutAll->addWidget(viewWidget);
    layoutAll->addWidget(solidWidget, 1);

    refresh();

    setLayout(layoutAll);
}

void PostprocessorScenePost3DWidget::refresh()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;
}

void PostprocessorScenePost3DWidget::load()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;

    radPost3DNone->setChecked((SceneViewPost3DMode) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DMode).toInt() == SceneViewPost3DMode_None);
    radPost3DScalarField3D->setChecked((SceneViewPost3DMode) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DMode).toInt() == SceneViewPost3DMode_ScalarView3D);
    radPost3DScalarField3DSolid->setChecked((SceneViewPost3DMode) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DMode).toInt() == SceneViewPost3DMode_ScalarView3DSolid);
    radPost3DModel->setChecked((SceneViewPost3DMode) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DMode).toInt() == SceneViewPost3DMode_Model);

    chkView3DLighting->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DLighting).toBool());
    txtView3DAngle->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DAngle).toDouble());
    chkView3DBackground->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DBackground).toBool());
    txtView3DHeight->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DHeight).toDouble());
    chkView3DBoundingBox->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DBoundingBox).toBool());
    chkView3DSolidGeometry->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarView3DSolidGeometry).toBool());

    // solid
    lstSolidMaterials->clear();
    if (m_fieldWidget->selectedComputation()->isSolved() && m_fieldWidget->selectedComputation()->postDeal()->activeViewField())
    {
        foreach (SceneMaterial *material, m_fieldWidget->selectedComputation()->scene()->materials->filter(m_fieldWidget->selectedComputation()->postDeal()->activeViewField()).items())
        {
            QListWidgetItem *item = new QListWidgetItem(lstSolidMaterials);
            item->setText(material->name());
            item->setData(Qt::UserRole, material->variant());
            if (m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::SolidViewHide).toStringList().contains(material->name()))
                item->setCheckState(Qt::Unchecked);
            else
                item->setCheckState(Qt::Checked);

            lstSolidMaterials->addItem(item);
        }
    }
}

void PostprocessorScenePost3DWidget::save()
{
    if (radPost3DNone->isChecked()) m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DMode, SceneViewPost3DMode_None);
    if (radPost3DScalarField3D->isChecked()) m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DMode, SceneViewPost3DMode_ScalarView3D);
    if (radPost3DScalarField3DSolid->isChecked()) m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DMode, SceneViewPost3DMode_ScalarView3DSolid);
    if (radPost3DModel->isChecked()) m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DMode, SceneViewPost3DMode_Model);

    // solid
    QStringList hideList;
    for (int i = 0; i < lstSolidMaterials->count(); i++)
    {
        if (lstSolidMaterials->item(i)->checkState() == Qt::Unchecked)
        {
            SceneMaterial *material = lstSolidMaterials->item(i)->data(Qt::UserRole).value<SceneMaterial *>();
            hideList.append(material->name());
        }
    }
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::SolidViewHide, hideList);

    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DLighting, chkView3DLighting->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DAngle, txtView3DAngle->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DBackground, chkView3DBackground->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DHeight, txtView3DHeight->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DBoundingBox, chkView3DBoundingBox->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarView3DSolidGeometry, chkView3DSolidGeometry->isChecked());
}

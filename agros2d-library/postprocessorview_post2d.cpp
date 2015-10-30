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

#include "postprocessorview_post2d.h"
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

PostprocessorScenePost2DWidget::PostprocessorScenePost2DWidget(PostprocessorWidget *postprocessorWidget, SceneViewPost2D *scenePost2D)
    : PostprocessorSceneWidget(postprocessorWidget), m_scenePost2D(scenePost2D)
{
    setWindowIcon(icon("scene-properties"));
    setObjectName("PostprocessorPost2DWidget");

    createControls();
}

void PostprocessorScenePost2DWidget::createControls()
{
    // main toolbar
    toolBar = new QToolBar();
    toolBar->addAction(m_scenePost2D->actPostprocessorModeNothing);
    toolBar->addAction(m_scenePost2D->actPostprocessorModeLocalPointValue);
    toolBar->addAction(m_scenePost2D->actPostprocessorModeSurfaceIntegral);
    toolBar->addAction(m_scenePost2D->actPostprocessorModeVolumeIntegral);
    toolBar->addSeparator();
    toolBar->addAction(m_scenePost2D->actSelectPoint);
    toolBar->addAction(m_scenePost2D->actSelectByMarker);

    groupPost2d = post2DWidget();
    groupPostScalar = postScalarWidget();
    groupPostContour = postContourWidget();
    groupPostVector = postVectorWidget();

    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->addWidget(groupPost2d);
    layoutArea->addWidget(groupPostScalar);
    layoutArea->addWidget(groupPostContour);
    layoutArea->addWidget(groupPostVector);
    layoutArea->addStretch(1);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutArea);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(toolBar);
    layoutMain->addWidget(widgetArea, 1);

    refresh();

    groupPostScalarAdvanced->setVisible(false);
    groupPostContourAdvanced->setVisible(false);
    groupPostVectorAdvanced->setVisible(false);

    setLayout(layoutMain);
}

QWidget *PostprocessorScenePost2DWidget::post2DWidget()
{
    // layout post2d
    chkShowPost2DContourView = new QCheckBox(tr("Contours"));
    connect(chkShowPost2DContourView, SIGNAL(clicked()), this, SLOT(refresh()));
    chkShowPost2DVectorView = new QCheckBox(tr("Vectors"));
    connect(chkShowPost2DVectorView, SIGNAL(clicked()), this, SLOT(refresh()));
    chkShowPost2DScalarView = new QCheckBox(tr("Scalar view"));
    connect(chkShowPost2DScalarView, SIGNAL(clicked()), this, SLOT(refresh()));

    QGridLayout *layoutPost2D = new QGridLayout();
    layoutPost2D->addWidget(chkShowPost2DScalarView, 0, 0);
    layoutPost2D->addWidget(chkShowPost2DContourView, 1, 0);
    layoutPost2D->addWidget(chkShowPost2DVectorView, 2, 0);
    layoutPost2D->setRowStretch(50, 1);

    QGroupBox *grpShowPost2D = new QGroupBox(tr("Postprocessor 2D"));
    grpShowPost2D->setLayout(layoutPost2D);

    return grpShowPost2D;
}

CollapsableGroupBoxButton *PostprocessorScenePost2DWidget::postScalarWidget()
{
    // layout scalar field
    cmbPostScalarFieldVariable = new QComboBox();
    connect(cmbPostScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));
    cmbPostScalarFieldVariableComp = new QComboBox();

    chkScalarFieldRangeAuto = new QCheckBox(tr("Auto range"));
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    groupPostScalarAdvanced = postScalarAdvancedWidget();

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutScalarField->setColumnStretch(1, 1);
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariable, 0, 1);
    layoutScalarField->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariableComp, 1, 1);
    layoutScalarField->addWidget(groupPostScalarAdvanced, 2, 0, 1, 2);

    CollapsableGroupBoxButton *grpScalarField = new CollapsableGroupBoxButton(tr("Scalar field"));
    connect(grpScalarField, SIGNAL(collapseEvent(bool)), this, SLOT(doScalarFieldExpandCollapse(bool)));
    grpScalarField->setCollapsed(true);
    grpScalarField->setLayout(layoutScalarField);

    return grpScalarField;
}

CollapsableGroupBoxButton *PostprocessorScenePost2DWidget::postContourWidget()
{
    // contour field
    cmbPost2DContourVariable = new QComboBox();

    groupPostContourAdvanced = postContourAdvancedWidget();

    QGridLayout *layoutContourField = new QGridLayout();
    layoutContourField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutContourField->setColumnStretch(1, 1);
    layoutContourField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutContourField->addWidget(cmbPost2DContourVariable, 0, 1);
    layoutContourField->addWidget(groupPostContourAdvanced, 1, 0, 1, 2);

    CollapsableGroupBoxButton *grpContourField = new CollapsableGroupBoxButton(tr("Contour field"));
    connect(grpContourField, SIGNAL(collapseEvent(bool)), this, SLOT(doContourFieldExpandCollapse(bool)));
    grpContourField->setCollapsed(true);
    grpContourField->setLayout(layoutContourField);

    return grpContourField;
}

CollapsableGroupBoxButton *PostprocessorScenePost2DWidget::postVectorWidget()
{
    // vector field
    cmbPost2DVectorFieldVariable = new QComboBox();

    groupPostVectorAdvanced = postVectorAdvancedWidget();

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutVectorField->setColumnStretch(1, 1);
    layoutVectorField->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutVectorField->addWidget(cmbPost2DVectorFieldVariable, 0, 1);
    layoutVectorField->addWidget(groupPostVectorAdvanced, 1, 0, 1, 2);

    CollapsableGroupBoxButton *grpVectorField = new CollapsableGroupBoxButton(tr("Vector field"));
    connect(grpVectorField, SIGNAL(collapseEvent(bool)), this, SLOT(doVectorFieldExpandCollapse(bool)));
    grpVectorField->setCollapsed(true);
    grpVectorField->setLayout(layoutVectorField);

    return grpVectorField;
}

QWidget *PostprocessorScenePost2DWidget::postScalarAdvancedWidget()
{
    // scalar field
    // palette
    cmbPalette = new QComboBox();
    foreach (QString key, paletteTypeStringKeys())
        cmbPalette->addItem(paletteTypeString(paletteTypeFromStringKey(key)), paletteTypeFromStringKey(key));

    chkPaletteFilter = new QCheckBox(tr("Filter"));
    connect(chkPaletteFilter, SIGNAL(stateChanged(int)), this, SLOT(doPaletteFilter(int)));

    // steps
    txtPaletteSteps = new QSpinBox(this);
    txtPaletteSteps->setMinimum(PALETTESTEPSMIN);
    txtPaletteSteps->setMaximum(PALETTESTEPSMAX);

    // log scale
    chkScalarFieldRangeLog = new QCheckBox(tr("Log. scale"));
    txtScalarFieldRangeBase = new LineEditDouble(1);
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    txtScalarFieldRangeMin = new LineEditDouble(0.1);
    connect(txtScalarFieldRangeMin, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMinChanged()));
    lblScalarFieldRangeMinError = new QLabel("");
    lblScalarFieldRangeMinError->setPalette(palette);
    lblScalarFieldRangeMinError->setVisible(false);
    txtScalarFieldRangeMax = new LineEditDouble(0.1);
    connect(txtScalarFieldRangeMax, SIGNAL(textChanged(QString)), this, SLOT(doScalarFieldRangeMaxChanged()));
    lblScalarFieldRangeMaxError = new QLabel("");
    lblScalarFieldRangeMaxError->setPalette(palette);
    lblScalarFieldRangeMaxError->setVisible(false);

    QGridLayout *layoutScalarFieldRange = new QGridLayout();
    lblScalarFieldRangeMin = new QLabel(tr("Minimum:"));
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMin, 0, 0);
    layoutScalarFieldRange->addWidget(txtScalarFieldRangeMin, 0, 1);
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMinError, 0, 2);
    layoutScalarFieldRange->addWidget(chkScalarFieldRangeAuto, 0, 3);
    lblScalarFieldRangeMax = new QLabel(tr("Maximum:"));
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMax, 1, 0);
    layoutScalarFieldRange->addWidget(txtScalarFieldRangeMax, 1, 1);
    layoutScalarFieldRange->addWidget(lblScalarFieldRangeMaxError, 1, 2);

    QGroupBox *grpScalarFieldRange = new QGroupBox(tr("Range"));
    grpScalarFieldRange->setLayout(layoutScalarFieldRange);

    QGridLayout *gridLayoutScalarFieldPalette = new QGridLayout();
    gridLayoutScalarFieldPalette->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutScalarFieldPalette->setColumnStretch(1, 1);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutScalarFieldPalette->addWidget(cmbPalette, 0, 1, 1, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Steps:")), 2, 0);
    gridLayoutScalarFieldPalette->addWidget(txtPaletteSteps, 2, 1);
    gridLayoutScalarFieldPalette->addWidget(chkPaletteFilter, 2, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Base:")), 3, 0);
    gridLayoutScalarFieldPalette->addWidget(txtScalarFieldRangeBase, 3, 1);
    gridLayoutScalarFieldPalette->addWidget(chkScalarFieldRangeLog, 3, 2);

    QGroupBox *grpScalarFieldPalette = new QGroupBox(tr("Palette"));
    grpScalarFieldPalette->setLayout(gridLayoutScalarFieldPalette);

    // decimal places
    txtScalarDecimalPlace = new QSpinBox(this);
    txtScalarDecimalPlace->setMinimum(SCALARDECIMALPLACEMIN);
    txtScalarDecimalPlace->setMaximum(SCALARDECIMALPLACEMAX);

    // color bar
    chkShowScalarColorBar = new QCheckBox(tr("Show colorbar"), this);

    QGridLayout *gridLayoutScalarFieldColorbar = new QGridLayout();
    gridLayoutScalarFieldColorbar->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutScalarFieldColorbar->setColumnStretch(1, 1);
    gridLayoutScalarFieldColorbar->addWidget(new QLabel(tr("Decimal places:")), 0, 0);
    gridLayoutScalarFieldColorbar->addWidget(txtScalarDecimalPlace, 0, 1);
    gridLayoutScalarFieldColorbar->addWidget(chkShowScalarColorBar, 0, 2);

    QGroupBox *grpScalarFieldColorbar = new QGroupBox(tr("Colorbar"));
    grpScalarFieldColorbar->setLayout(gridLayoutScalarFieldColorbar);

    chkScalarDeform = new QCheckBox(tr("Deform shape"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkScalarDeform, 0, 0);

    QGroupBox *grpScalarDeformShape = new QGroupBox(tr("Displacement"));
    grpScalarDeformShape->setLayout(layoutDeformShape);

    QVBoxLayout *layoutScalarFieldAdvanced = new QVBoxLayout();
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldPalette);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldColorbar);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldRange);
    layoutScalarFieldAdvanced->addWidget(grpScalarDeformShape);

    QWidget *scalarWidget = new QWidget();
    scalarWidget->setLayout(layoutScalarFieldAdvanced);

    return scalarWidget;
}

QWidget *PostprocessorScenePost2DWidget::postContourAdvancedWidget()
{
    // contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(CONTOURSCOUNTMIN);
    txtContoursCount->setMaximum(CONTOURSCOUNTMAX);
    txtContourWidth = new QDoubleSpinBox(this);
    txtContourWidth->setMinimum(CONTOURSWIDTHMIN);
    txtContourWidth->setMaximum(CONTOURSWIDTHMAX);
    txtContourWidth->setSingleStep(0.1);

    chkContourDeform = new QCheckBox(tr("Deform shape"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkContourDeform, 0, 0);

    QGroupBox *grpContourDeformShape = new QGroupBox(tr("Displacement"));
    grpContourDeformShape->setLayout(layoutDeformShape);

    QGridLayout *gridLayoutContours = new QGridLayout();
    gridLayoutContours->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutContours->setColumnStretch(1, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Number of contours:")), 0, 0);
    gridLayoutContours->addWidget(txtContoursCount, 0, 1);
    gridLayoutContours->addWidget(new QLabel(tr("Contour width:")), 1, 0);
    gridLayoutContours->addWidget(txtContourWidth, 1, 1);
    gridLayoutContours->addWidget(grpContourDeformShape, 2, 0, 1, 2);

    QWidget *contourWidget = new QWidget();
    contourWidget->setLayout(gridLayoutContours);

    return contourWidget;
}

QWidget *PostprocessorScenePost2DWidget::postVectorAdvancedWidget()
{
    // vectors
    chkVectorProportional = new QCheckBox(tr("Proportional"), this);
    chkVectorColor = new QCheckBox(tr("Color (b/w)"), this);
    txtVectorCount = new QSpinBox(this);
    txtVectorCount->setMinimum(VECTORSCOUNTMIN);
    txtVectorCount->setMaximum(VECTORSCOUNTMAX);
    txtVectorScale = new QDoubleSpinBox(this);
    txtVectorScale->setDecimals(2);
    txtVectorScale->setSingleStep(0.1);
    txtVectorScale->setMinimum(VECTORSSCALEMIN);
    txtVectorScale->setMaximum(VECTORSSCALEMAX);
    cmbVectorType = new QComboBox();
    foreach (QString key, vectorTypeStringKeys())
        cmbVectorType->addItem(vectorTypeString(vectorTypeFromStringKey(key)), vectorTypeFromStringKey(key));
    cmbVectorCenter = new QComboBox();
    foreach (QString key, vectorCenterStringKeys())
        cmbVectorCenter->addItem(vectorCenterString(vectorCenterFromStringKey(key)), vectorCenterFromStringKey(key));

    chkVectorDeform = new QCheckBox(tr("Deform shape"), this);

    QGridLayout *layoutDeformShape = new QGridLayout();
    layoutDeformShape->addWidget(chkVectorDeform, 0, 0);

    QGroupBox *grpVectorDeformShape = new QGroupBox(tr("Displacement"));
    grpVectorDeformShape->setLayout(layoutDeformShape);

    QGridLayout *gridLayoutVectors = new QGridLayout();
    gridLayoutVectors->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutVectors->setColumnStretch(1, 1);
    gridLayoutVectors->addWidget(new QLabel(tr("Number of vec.:")), 0, 0);
    gridLayoutVectors->addWidget(txtVectorCount, 0, 1);
    gridLayoutVectors->addWidget(chkVectorProportional, 0, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Scale:")), 1, 0);
    gridLayoutVectors->addWidget(txtVectorScale, 1, 1);
    gridLayoutVectors->addWidget(chkVectorColor, 1, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Type:")), 2, 0);
    gridLayoutVectors->addWidget(cmbVectorType, 2, 1, 1, 2);
    gridLayoutVectors->addWidget(new QLabel(tr("Center:")), 3, 0);
    gridLayoutVectors->addWidget(cmbVectorCenter, 3, 1, 1, 2);
    gridLayoutVectors->addWidget(grpVectorDeformShape, 4, 0, 1, 2);

    QWidget *vectorWidget = new QWidget();
    vectorWidget->setLayout(gridLayoutVectors);

    return vectorWidget;
}

void PostprocessorScenePost2DWidget::doScalarFieldExpandCollapse(bool collapsed)
{
    groupPostScalarAdvanced->setVisible(!collapsed);
}

void PostprocessorScenePost2DWidget::doContourFieldExpandCollapse(bool collapsed)
{
    groupPostContourAdvanced->setVisible(!collapsed);
}

void PostprocessorScenePost2DWidget::doVectorFieldExpandCollapse(bool collapsed)
{
    groupPostVectorAdvanced->setVisible(!collapsed);
}

/*
void PostprocessorScenePost2DWidget::doScalarFieldDefault()
{
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) computation()->setting()->defaultValue(ProblemSetting::View_PaletteType).toInt()));
    chkPaletteFilter->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_PaletteFilter).toBool());
    txtPaletteSteps->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_PaletteSteps).toInt());
    chkShowScalarColorBar->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_ShowScalarColorBar).toBool());
    chkScalarFieldRangeLog->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_ScalarRangeLog).toBool());
    txtScalarFieldRangeBase->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_ScalarRangeBase).toInt());
    txtScalarDecimalPlace->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_ScalarDecimalPlace).toInt());
    chkScalarDeform->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_DeformScalar).toBool());
}

void PostprocessorScenePost2DWidget::doContoursVectorsDefault()
{
    txtContoursCount->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_ContoursCount).toInt());
    chkContourDeform->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_DeformContour).toBool());

    chkVectorProportional->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_VectorProportional).toBool());
    chkVectorColor->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_VectorColor).toBool());
    txtVectorCount->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_VectorCount).toInt());
    txtVectorScale->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_VectorScale).toDouble());
    cmbVectorType->setCurrentIndex(cmbVectorType->findData((VectorType) computation()->setting()->defaultValue(ProblemSetting::View_VectorType).toInt()));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData((VectorCenter) computation()->setting()->defaultValue(ProblemSetting::View_VectorCenter).toInt()));
    chkVectorDeform->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::View_DeformVector).toBool());
}
*/

void PostprocessorScenePost2DWidget::doScalarFieldRangeMinChanged()
{
    lblScalarFieldRangeMinError->clear();
    lblScalarFieldRangeMinError->setVisible(false);
    lblScalarFieldRangeMaxError->clear();
    lblScalarFieldRangeMaxError->setVisible(false);

    if (txtScalarFieldRangeMin->value() > txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("> %1").arg(txtScalarFieldRangeMax->value()));
        lblScalarFieldRangeMinError->setVisible(true);
    }
    /*
    else if (txtScalarFieldRangeMin->value() == txtScalarFieldRangeMax->value())
    {
        lblScalarFieldRangeMinError->setText(QString("= %1").arg(txtScalarFieldRangeMax->value()));
        btnOK->setDisabled(true);
    }
    */
}

void PostprocessorScenePost2DWidget::doScalarFieldRangeMaxChanged()
{
    lblScalarFieldRangeMaxError->clear();
    lblScalarFieldRangeMinError->clear();

    if (txtScalarFieldRangeMax->value() < txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("< %1").arg(txtScalarFieldRangeMin->value()));
        //btnOK->setDisabled(true);
    }
    /*
    else if (txtScalarFieldRangeMax->value() == txtScalarFieldRangeMin->value())
    {
        lblScalarFieldRangeMaxError->setText(QString("= %1").arg(txtScalarFieldRangeMin->value()));
        btnOK->setDisabled(true);
    }
    */
}

void PostprocessorScenePost2DWidget::doScalarFieldVariable(int index)
{
    if (!m_postprocessorWidget->fieldWidget()->selectedField())
        return;

    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    if (cmbPostScalarFieldVariable->currentIndex() != -1)
    {
        QString variableName(cmbPostScalarFieldVariable->itemData(index).toString());

        QString fieldName = m_postprocessorWidget->fieldWidget()->selectedField()->fieldId();
        Module::LocalVariable physicFieldVariable = m_postprocessorWidget->computation()->fieldInfo(fieldName)->localVariable(m_postprocessorWidget->computation()->config()->coordinateType(), variableName);

        // component
        cmbPostScalarFieldVariableComp->clear();
        if (physicFieldVariable.isScalar())
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
        }
        else
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
            cmbPostScalarFieldVariableComp->addItem(m_postprocessorWidget->computation()->config()->labelX(), PhysicFieldVariableComp_X);
            cmbPostScalarFieldVariableComp->addItem(m_postprocessorWidget->computation()->config()->labelY(), PhysicFieldVariableComp_Y);
        }

        cmbPostScalarFieldVariableComp->setCurrentIndex(cmbPostScalarFieldVariableComp->findData(scalarFieldVariableComp));
        if (cmbPostScalarFieldVariableComp->currentIndex() == -1)
            cmbPostScalarFieldVariableComp->setCurrentIndex(0);
    }
}

void PostprocessorScenePost2DWidget::doScalarFieldLog(int state)
{
    txtScalarFieldRangeBase->setEnabled(chkScalarFieldRangeLog->isChecked());
}

void PostprocessorScenePost2DWidget::doScalarFieldRangeAuto(int state)
{
    txtScalarFieldRangeMin->setEnabled(!chkScalarFieldRangeAuto->isChecked());
    txtScalarFieldRangeMax->setEnabled(!chkScalarFieldRangeAuto->isChecked());
}

void PostprocessorScenePost2DWidget::doPaletteFilter(int state)
{
    txtPaletteSteps->setEnabled(!chkPaletteFilter->isChecked());
}

void PostprocessorScenePost2DWidget::refresh()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    fillComboBoxScalarVariable(m_postprocessorWidget->computation()->config()->coordinateType(), m_postprocessorWidget->fieldWidget()->selectedField(), cmbPostScalarFieldVariable);
    fillComboBoxContourVariable(m_postprocessorWidget->computation()->config()->coordinateType(), m_postprocessorWidget->fieldWidget()->selectedField(), cmbPost2DContourVariable);
    fillComboBoxVectorVariable(m_postprocessorWidget->computation()->config()->coordinateType(), m_postprocessorWidget->fieldWidget()->selectedField(), cmbPost2DVectorFieldVariable);
    doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());
}

void PostprocessorScenePost2DWidget::load()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;

    chkShowPost2DContourView->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ShowContourView).toBool());
    chkShowPost2DVectorView->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ShowVectorView).toBool());
    chkShowPost2DScalarView->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ShowScalarView).toBool());

    // contour field
    cmbPost2DContourVariable->setCurrentIndex(cmbPost2DContourVariable->findData(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ContourVariable).toString()));
    if (cmbPost2DContourVariable->count() > 0 && cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()) != QVariant::Invalid)
    {
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ContourVariable,
                                                    cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString());
    }
    if (cmbPost2DContourVariable->currentIndex() == -1 && cmbPost2DContourVariable->count() > 0)
    {
        // set first variable
        cmbPost2DContourVariable->setCurrentIndex(0);
    }

    // scalar field
    cmbPostScalarFieldVariable->setCurrentIndex(cmbPostScalarFieldVariable->findData(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarVariable).toString()));
    if (cmbPostScalarFieldVariable->count() > 0 && cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarVariable, cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());
        doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());

        cmbPostScalarFieldVariableComp->setCurrentIndex(cmbPostScalarFieldVariableComp->findData((PhysicFieldVariableComp) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarVariableComp).toInt()));
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt());
    }
    if (cmbPostScalarFieldVariable->currentIndex() == -1 && cmbPostScalarFieldVariable->count() > 0)
    {
        // set first variable
        cmbPostScalarFieldVariable->setCurrentIndex(0);
    }

    // vector field
    if (cmbPost2DVectorFieldVariable->count() > 0 && cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        cmbPost2DVectorFieldVariable->setCurrentIndex(cmbPost2DVectorFieldVariable->findData(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorVariable).toString()));
        m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorVariable,
                                                                  cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString());
    }
    if (cmbPost2DVectorFieldVariable->currentIndex() == -1 && cmbPost2DVectorFieldVariable->count() > 0)
    {
        // set first variable
        cmbPost2DVectorFieldVariable->setCurrentIndex(0);
    }

    // scalar field
    chkShowScalarColorBar->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ShowScalarColorBar).toBool());
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_PaletteType).toInt()));
    chkPaletteFilter->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_PaletteFilter).toBool());
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_PaletteSteps).toInt());
    chkScalarDeform->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_DeformScalar).toBool());

    // contours
    txtContoursCount->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ContoursCount).toInt());
    txtContourWidth->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ContoursWidth).toDouble());
    chkContourDeform->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_DeformContour).toBool());

    // vector field
    chkVectorProportional->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorProportional).toBool());
    chkVectorColor->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorColor).toBool());
    txtVectorCount->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorCount).toInt());
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorScale).toDouble());
    cmbVectorType->setCurrentIndex(cmbVectorType->findData((VectorType) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorType).toInt()));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData((VectorCenter) m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_VectorCenter).toInt()));
    chkVectorDeform->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_DeformVector).toBool());

    // advanced
    // scalar field
    chkScalarFieldRangeLog->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarRangeLog).toBool());
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarRangeBase).toDouble());
    txtScalarDecimalPlace->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarDecimalPlace).toInt());
    chkScalarFieldRangeAuto->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarRangeAuto).toBool());
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarRangeMin).toDouble());
    txtScalarFieldRangeMax->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ScalarRangeMax).toDouble());
}

void PostprocessorScenePost2DWidget::save()
{
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ShowContourView, chkShowPost2DContourView->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ShowScalarView, chkShowPost2DScalarView->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ShowVectorView, chkShowPost2DVectorView->isChecked());

    // contour field
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ContourVariable, cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString());

    // scalar field
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarVariable, cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarVariableComp, (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarRangeAuto, chkScalarFieldRangeAuto->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarRangeMin, txtScalarFieldRangeMin->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarRangeMax, txtScalarFieldRangeMax->value());

    // vector field
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorVariable, cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString());

    // scalar field
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ShowScalarColorBar, chkShowScalarColorBar->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_PaletteType, (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_PaletteFilter, chkPaletteFilter->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_PaletteSteps, txtPaletteSteps->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_DeformScalar, chkScalarDeform->isChecked());

    // contours
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ContoursCount, txtContoursCount->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ContoursWidth, txtContourWidth->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_DeformContour, chkContourDeform->isChecked());

    // vector field
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorProportional, chkVectorProportional->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorColor, chkVectorColor->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorCount, txtVectorCount->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorScale, txtVectorScale->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorType, (VectorType) cmbVectorType->itemData(cmbVectorType->currentIndex()).toInt());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_VectorCenter, (VectorCenter) cmbVectorCenter->itemData(cmbVectorCenter->currentIndex()).toInt());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_DeformVector, chkVectorDeform->isChecked());

    // scalar view
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarRangeLog, chkScalarFieldRangeLog->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarRangeBase, txtScalarFieldRangeBase->text().toDouble());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ScalarDecimalPlace, txtScalarDecimalPlace->value());
}

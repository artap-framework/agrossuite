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

#include "gui/resultsview.h"

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

PostprocessorScenePost2DWidget::PostprocessorScenePost2DWidget(PhysicalFieldWidget *fieldWidget, SceneViewPost2D *scenePost2D)
    : PostprocessorSceneWidget(fieldWidget), m_scenePost2D(scenePost2D)
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

    resultsView = new ResultsView(m_scenePost2D);
    connect(m_scenePost2D, SIGNAL(mousePressed()), resultsView, SLOT(doShowResults()));
    connect(m_scenePost2D, SIGNAL(mousePressed(const Point &)), resultsView, SLOT(showPoint(const Point &)));
    connect(m_scenePost2D, SIGNAL(postprocessorModeGroupChanged(SceneModePostprocessor)), resultsView, SLOT(doPostprocessorModeGroupChanged(SceneModePostprocessor)));

    QVBoxLayout *layoutResults = new QVBoxLayout();
    layoutResults->addWidget(toolBar);
    layoutResults->addWidget(resultsView);

    QWidget *widResults = new QWidget();
    widResults->setLayout(layoutResults);

    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->addTab(postScalarAdvancedWidget(), tr("Scalar field"));
    tabWidget->addTab(postContourAdvancedWidget(), tr("Contours"));
    tabWidget->addTab(postVectorAdvancedWidget(), tr("Vector field"));

    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->addWidget(tabWidget);
    layoutArea->addWidget(widResults);

    refresh();

    setLayout(layoutArea);
}

QWidget *PostprocessorScenePost2DWidget::postScalarAdvancedWidget()
{
    // scalar field
    chkShowPost2DScalarView = new QCheckBox(tr("Show scalar field"));
    connect(chkShowPost2DScalarView, SIGNAL(clicked()), this, SLOT(refresh()));

    chkScalarDeform = new QCheckBox(tr("Deform shape"), this);

    // layout scalar field
    cmbPostScalarFieldVariable = new QComboBox();
    connect(cmbPostScalarFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doScalarFieldVariable(int)));
    cmbPostScalarFieldVariableComp = new QComboBox();

    QGridLayout *layoutScalarField = new QGridLayout();
    layoutScalarField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutScalarField->setColumnStretch(1, 1);
    layoutScalarField->addWidget(chkShowPost2DScalarView, 0, 1);
    layoutScalarField->addWidget(chkScalarDeform, 0, 2);
    layoutScalarField->addWidget(new QLabel(tr("Variable:")), 1, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariable, 1, 1, 1, 2);
    layoutScalarField->addWidget(new QLabel(tr("Component:")), 2, 0);
    layoutScalarField->addWidget(cmbPostScalarFieldVariableComp, 2, 1, 1, 2);

    QGroupBox *grpScalarField = new QGroupBox(tr("Variable"));
    grpScalarField->setLayout(layoutScalarField);

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

    // decimal places
    txtScalarDecimalPlace = new QSpinBox(this);
    txtScalarDecimalPlace->setMinimum(SCALARDECIMALPLACEMIN);
    txtScalarDecimalPlace->setMaximum(SCALARDECIMALPLACEMAX);

    // color bar
    chkShowScalarColorBar = new QCheckBox(tr("Show colorbar"), this);

    // log scale
    chkScalarFieldRangeLog = new QCheckBox(tr("Log. scale"));
    txtScalarFieldRangeBase = new LineEditDouble(1);
    connect(chkScalarFieldRangeLog, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldLog(int)));

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::red);

    txtScalarFieldRangeMin = new LineEditDouble(0.1);
    txtScalarFieldRangeMax = new LineEditDouble(0.1);
    chkScalarFieldRangeAuto = new QCheckBox(tr("Auto range"));
    connect(chkScalarFieldRangeAuto, SIGNAL(stateChanged(int)), this, SLOT(doScalarFieldRangeAuto(int)));

    QGridLayout *gridLayoutScalarFieldPalette = new QGridLayout();
    gridLayoutScalarFieldPalette->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutScalarFieldPalette->setColumnStretch(1, 1);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Palette:")), 0, 0);
    gridLayoutScalarFieldPalette->addWidget(cmbPalette, 0, 1);
    gridLayoutScalarFieldPalette->addWidget(chkShowScalarColorBar, 0, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Steps:")), 2, 0);
    gridLayoutScalarFieldPalette->addWidget(txtPaletteSteps, 2, 1);
    gridLayoutScalarFieldPalette->addWidget(chkPaletteFilter, 2, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Decimal places:")), 3, 0);
    gridLayoutScalarFieldPalette->addWidget(txtScalarDecimalPlace, 3, 1);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Base:")), 4, 0);
    gridLayoutScalarFieldPalette->addWidget(txtScalarFieldRangeBase, 4, 1);
    gridLayoutScalarFieldPalette->addWidget(chkScalarFieldRangeLog, 4, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Minimum range:")), 5, 0);
    gridLayoutScalarFieldPalette->addWidget(txtScalarFieldRangeMin, 5, 1);
    gridLayoutScalarFieldPalette->addWidget(chkScalarFieldRangeAuto, 5, 2);
    gridLayoutScalarFieldPalette->addWidget(new QLabel(tr("Maximum range:")), 6, 0);
    gridLayoutScalarFieldPalette->addWidget(txtScalarFieldRangeMax, 6, 1);

    QGroupBox *grpScalarFieldPalette = new QGroupBox(tr("Palette and colorbar"));
    grpScalarFieldPalette->setLayout(gridLayoutScalarFieldPalette);

    QVBoxLayout *layoutScalarFieldAdvanced = new QVBoxLayout();
    layoutScalarFieldAdvanced->addWidget(grpScalarField);
    layoutScalarFieldAdvanced->addWidget(grpScalarFieldPalette);
    layoutScalarFieldAdvanced->addStretch(1);

    QWidget *scalarWidget = new QWidget();
    scalarWidget->setLayout(layoutScalarFieldAdvanced);

    return scalarWidget;
}

QWidget *PostprocessorScenePost2DWidget::postContourAdvancedWidget()
{
    // contours field
    chkShowPost2DContourView = new QCheckBox(tr("Show contours"));
    connect(chkShowPost2DContourView, SIGNAL(clicked()), this, SLOT(refresh()));

    chkContourDeform = new QCheckBox(tr("Deform shape"), this);

    // contour field
    cmbPost2DContourVariable = new QComboBox();

    QGridLayout *layoutContourField = new QGridLayout();
    layoutContourField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutContourField->setColumnStretch(1, 1);
    layoutContourField->addWidget(chkShowPost2DContourView, 0, 1);
    layoutContourField->addWidget(chkContourDeform, 0, 2);
    layoutContourField->addWidget(new QLabel(tr("Variable:")), 1, 0);
    layoutContourField->addWidget(cmbPost2DContourVariable, 1, 1, 1, 2);

    QGroupBox *grpContourField = new QGroupBox(tr("Variable"));
    grpContourField->setLayout(layoutContourField);

    // contours
    txtContoursCount = new QSpinBox(this);
    txtContoursCount->setMinimum(CONTOURSCOUNTMIN);
    txtContoursCount->setMaximum(CONTOURSCOUNTMAX);
    txtContourWidth = new QDoubleSpinBox(this);
    txtContourWidth->setMinimum(CONTOURSWIDTHMIN);
    txtContourWidth->setMaximum(CONTOURSWIDTHMAX);
    txtContourWidth->setSingleStep(0.1);

    QGridLayout *layoutAdvancedContours = new QGridLayout();
    layoutAdvancedContours->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdvancedContours->setColumnStretch(1, 1);
    layoutAdvancedContours->addWidget(new QLabel(tr("Number of contours:")), 1, 0);
    layoutAdvancedContours->addWidget(txtContoursCount, 1, 1);
    layoutAdvancedContours->addWidget(new QLabel(tr("Contour width:")), 2, 0);
    layoutAdvancedContours->addWidget(txtContourWidth, 2, 1);

    QGroupBox *grpAdvancedContour = new QGroupBox(tr("Advanced"));
    grpAdvancedContour->setLayout(layoutAdvancedContours);

    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->addWidget(grpContourField);
    layoutArea->addWidget(grpAdvancedContour);
    layoutArea->addStretch(1);

    QWidget *contourWidget = new QWidget();
    contourWidget->setLayout(layoutArea);

    return contourWidget;
}

QWidget *PostprocessorScenePost2DWidget::postVectorAdvancedWidget()
{
    chkShowPost2DVectorView = new QCheckBox(tr("Show vectors"));
    connect(chkShowPost2DVectorView, SIGNAL(clicked()), this, SLOT(refresh()));

    chkVectorDeform = new QCheckBox(tr("Deform shape"), this);

    // vector field
    cmbPost2DVectorFieldVariable = new QComboBox();

    QGridLayout *layoutVectorField = new QGridLayout();
    layoutVectorField->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutVectorField->setColumnStretch(1, 1);
    layoutVectorField->addWidget(chkShowPost2DVectorView, 0, 1);
    layoutVectorField->addWidget(chkVectorDeform, 0, 2);
    layoutVectorField->addWidget(new QLabel(tr("Variable:")), 1, 0);
    layoutVectorField->addWidget(cmbPost2DVectorFieldVariable, 1, 1, 1, 2);

    QGroupBox *grpVectorField = new QGroupBox(tr("Variable"));
    grpVectorField->setLayout(layoutVectorField);

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

    QGridLayout *layoutAdvancedVectors = new QGridLayout();
    layoutAdvancedVectors->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdvancedVectors->setColumnStretch(1, 1);
    layoutAdvancedVectors->addWidget(new QLabel(tr("Number of vectors:")), 0, 0);
    layoutAdvancedVectors->addWidget(txtVectorCount, 0, 1);
    layoutAdvancedVectors->addWidget(chkVectorProportional, 0, 2);
    layoutAdvancedVectors->addWidget(new QLabel(tr("Scale:")), 1, 0);
    layoutAdvancedVectors->addWidget(txtVectorScale, 1, 1);
    layoutAdvancedVectors->addWidget(chkVectorColor, 1, 2);
    layoutAdvancedVectors->addWidget(new QLabel(tr("Type:")), 2, 0);
    layoutAdvancedVectors->addWidget(cmbVectorType, 2, 1);
    layoutAdvancedVectors->addWidget(new QLabel(tr("Center:")), 3, 0);
    layoutAdvancedVectors->addWidget(cmbVectorCenter, 3, 1);
    layoutAdvancedVectors->setRowStretch(50, 1);

    QGroupBox *grpAdvancedVector = new QGroupBox(tr("Advanced"));
    grpAdvancedVector->setLayout(layoutAdvancedVectors);

    QVBoxLayout *layoutArea = new QVBoxLayout();
    layoutArea->addWidget(grpVectorField);
    layoutArea->addWidget(grpAdvancedVector);
    layoutArea->addStretch(1);

    QWidget *vectorWidget = new QWidget();
    vectorWidget->setLayout(layoutArea);

    return vectorWidget;
}

/*
void PostprocessorScenePost2DWidget::doScalarFieldDefault()
{
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) computation()->setting()->defaultValue(ProblemSetting::PaletteType).toInt()));
    chkPaletteFilter->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::PaletteFilter).toBool());
    txtPaletteSteps->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::PaletteSteps).toInt());
    chkShowScalarColorBar->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::ShowScalarColorBar).toBool());
    chkScalarFieldRangeLog->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::ScalarRangeLog).toBool());
    txtScalarFieldRangeBase->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::ScalarRangeBase).toInt());
    txtScalarDecimalPlace->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::ScalarDecimalPlace).toInt());
    chkScalarDeform->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::DeformScalar).toBool());
}

void PostprocessorScenePost2DWidget::doContoursVectorsDefault()
{
    txtContoursCount->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::ContoursCount).toInt());
    chkContourDeform->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::DeformContour).toBool());

    chkVectorProportional->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::VectorProportional).toBool());
    chkVectorColor->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::VectorColor).toBool());
    txtVectorCount->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::VectorCount).toInt());
    txtVectorScale->setValue(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::VectorScale).toDouble());
    cmbVectorType->setCurrentIndex(cmbVectorType->findData((VectorType) computation()->setting()->defaultValue(ProblemSetting::VectorType).toInt()));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData((VectorCenter) computation()->setting()->defaultValue(ProblemSetting::VectorCenter).toInt()));
    chkVectorDeform->setChecked(m_postprocessorWidget->computation()->setting()->defaultValue(ProblemSetting::DeformVector).toBool());
}
*/

void PostprocessorScenePost2DWidget::doScalarFieldVariable(int index)
{
    if (!m_fieldWidget->selectedField())
        return;

    PhysicFieldVariableComp scalarFieldVariableComp = (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt();

    if (cmbPostScalarFieldVariable->currentIndex() != -1)
    {
        QString variableName(cmbPostScalarFieldVariable->itemData(index).toString());

        QString fieldName = m_fieldWidget->selectedField()->fieldId();
        Module::LocalVariable physicFieldVariable = m_fieldWidget->selectedComputation()->fieldInfo(fieldName)->localVariable(m_fieldWidget->selectedComputation()->config()->coordinateType(), variableName);

        // component
        cmbPostScalarFieldVariableComp->clear();
        if (physicFieldVariable.isScalar())
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
        }
        else
        {
            cmbPostScalarFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
            cmbPostScalarFieldVariableComp->addItem(m_fieldWidget->selectedComputation()->config()->labelX(), PhysicFieldVariableComp_X);
            cmbPostScalarFieldVariableComp->addItem(m_fieldWidget->selectedComputation()->config()->labelY(), PhysicFieldVariableComp_Y);
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
    // show empty results
    resultsView->showRecipe();

    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;

    fillComboBoxScalarVariable(m_fieldWidget->selectedComputation()->config()->coordinateType(), m_fieldWidget->selectedField(), cmbPostScalarFieldVariable);
    fillComboBoxContourVariable(m_fieldWidget->selectedComputation()->config()->coordinateType(), m_fieldWidget->selectedField(), cmbPost2DContourVariable);
    fillComboBoxVectorVariable(m_fieldWidget->selectedComputation()->config()->coordinateType(), m_fieldWidget->selectedField(), cmbPost2DVectorFieldVariable);
    doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());
}

void PostprocessorScenePost2DWidget::load()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;

    chkShowPost2DContourView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowContourView).toBool());
    chkShowPost2DVectorView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowVectorView).toBool());
    chkShowPost2DScalarView->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowScalarView).toBool());

    // contour field
    cmbPost2DContourVariable->setCurrentIndex(cmbPost2DContourVariable->findData(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ContourVariable).toString()));
    if (cmbPost2DContourVariable->count() > 0 && cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()) != QVariant::Invalid)
    {
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ContourVariable,
                                                                  cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString());
    }
    if (cmbPost2DContourVariable->currentIndex() == -1 && cmbPost2DContourVariable->count() > 0)
    {
        // set first variable
        cmbPost2DContourVariable->setCurrentIndex(0);
    }

    // scalar field
    cmbPostScalarFieldVariable->setCurrentIndex(cmbPostScalarFieldVariable->findData(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString()));
    if (cmbPostScalarFieldVariable->count() > 0 && cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarVariable, cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());
        doScalarFieldVariable(cmbPostScalarFieldVariable->currentIndex());

        cmbPostScalarFieldVariableComp->setCurrentIndex(cmbPostScalarFieldVariableComp->findData((PhysicFieldVariableComp) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarVariableComp).toInt()));
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarVariableComp, (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt());
    }
    if (cmbPostScalarFieldVariable->currentIndex() == -1 && cmbPostScalarFieldVariable->count() > 0)
    {
        // set first variable
        cmbPostScalarFieldVariable->setCurrentIndex(0);
    }

    // vector field
    if (cmbPost2DVectorFieldVariable->count() > 0 && cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        cmbPost2DVectorFieldVariable->setCurrentIndex(cmbPost2DVectorFieldVariable->findData(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorVariable).toString()));
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorVariable,
                                                                  cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString());
    }
    if (cmbPost2DVectorFieldVariable->currentIndex() == -1 && cmbPost2DVectorFieldVariable->count() > 0)
    {
        // set first variable
        cmbPost2DVectorFieldVariable->setCurrentIndex(0);
    }

    // scalar field
    chkShowScalarColorBar->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ShowScalarColorBar).toBool());
    cmbPalette->setCurrentIndex(cmbPalette->findData((PaletteType) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::PaletteType).toInt()));
    chkPaletteFilter->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::PaletteFilter).toBool());
    doPaletteFilter(chkPaletteFilter->checkState());
    txtPaletteSteps->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::PaletteSteps).toInt());
    chkScalarDeform->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::DeformScalar).toBool());
    chkScalarDeform->setVisible(m_fieldWidget->selectedField()->hasDeformableShape());

    // contours
    txtContoursCount->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ContoursCount).toInt());
    txtContourWidth->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ContoursWidth).toDouble());
    chkContourDeform->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::DeformContour).toBool());
    chkContourDeform->setVisible(m_fieldWidget->selectedField()->hasDeformableShape());

    // vector field
    chkVectorProportional->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorProportional).toBool());
    chkVectorColor->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorColor).toBool());
    txtVectorCount->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorCount).toInt());
    txtVectorCount->setToolTip(tr("Width and height of bounding box over vector count."));
    txtVectorScale->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorScale).toDouble());
    cmbVectorType->setCurrentIndex(cmbVectorType->findData((VectorType) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorType).toInt()));
    cmbVectorCenter->setCurrentIndex(cmbVectorCenter->findData((VectorCenter) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::VectorCenter).toInt()));
    chkVectorDeform->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::DeformVector).toBool());
    chkVectorDeform->setVisible(m_fieldWidget->selectedField()->hasDeformableShape());

    // advanced
    // scalar field
    chkScalarFieldRangeLog->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarRangeLog).toBool());
    doScalarFieldLog(chkScalarFieldRangeLog->checkState());
    txtScalarFieldRangeBase->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarRangeBase).toDouble());
    txtScalarDecimalPlace->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarDecimalPlace).toInt());
    chkScalarFieldRangeAuto->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarRangeAuto).toBool());
    doScalarFieldRangeAuto(chkScalarFieldRangeAuto->checkState());
    txtScalarFieldRangeMin->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarRangeMin).toDouble());
    txtScalarFieldRangeMax->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ScalarRangeMax).toDouble());
}

void PostprocessorScenePost2DWidget::save()
{
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowContourView, chkShowPost2DContourView->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowScalarView, chkShowPost2DScalarView->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowVectorView, chkShowPost2DVectorView->isChecked());

    // contour field
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ContourVariable, cmbPost2DContourVariable->itemData(cmbPost2DContourVariable->currentIndex()).toString());

    // scalar field
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarVariable, cmbPostScalarFieldVariable->itemData(cmbPostScalarFieldVariable->currentIndex()).toString());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarVariableComp, (PhysicFieldVariableComp) cmbPostScalarFieldVariableComp->itemData(cmbPostScalarFieldVariableComp->currentIndex()).toInt());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarRangeAuto, chkScalarFieldRangeAuto->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarRangeMin, txtScalarFieldRangeMin->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarRangeMax, txtScalarFieldRangeMax->value());

    // vector field
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorVariable, cmbPost2DVectorFieldVariable->itemData(cmbPost2DVectorFieldVariable->currentIndex()).toString());

    // scalar field
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ShowScalarColorBar, chkShowScalarColorBar->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::PaletteType, (PaletteType) cmbPalette->itemData(cmbPalette->currentIndex()).toInt());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::PaletteFilter, chkPaletteFilter->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::PaletteSteps, txtPaletteSteps->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::DeformScalar, chkScalarDeform->isChecked());

    // contours
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ContoursCount, txtContoursCount->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ContoursWidth, txtContourWidth->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::DeformContour, chkContourDeform->isChecked());

    // vector field
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorProportional, chkVectorProportional->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorColor, chkVectorColor->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorCount, txtVectorCount->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorScale, txtVectorScale->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorType, (VectorType) cmbVectorType->itemData(cmbVectorType->currentIndex()).toInt());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::VectorCenter, (VectorCenter) cmbVectorCenter->itemData(cmbVectorCenter->currentIndex()).toInt());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::DeformVector, chkVectorDeform->isChecked());

    // scalar view
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarRangeLog, chkScalarFieldRangeLog->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarRangeBase, txtScalarFieldRangeBase->text().toDouble());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ScalarDecimalPlace, txtScalarDecimalPlace->value());
}

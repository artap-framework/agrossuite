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

#include "postprocessorview_chart.h"
#include "postprocessorview.h"

#include "chartdialog.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry_simple.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

#include "qcustomplot/qcustomplot.h"

QList<Point> ChartLine::getPoints()
{
    if (numberOfPoints == 0)
        return QList<Point>();

    QList<Point> points;
    points.reserve(numberOfPoints);

    double dx = (end.x - start.x) / (numberOfPoints - 1);
    double dy = (end.y - start.y) / (numberOfPoints - 1);

    for (int i = 0; i < numberOfPoints; i++)
        if (reverse)
            points.insert(0, Point(start.x + i*dx, start.y + i*dy));
        else
            points.append(Point(start.x + i*dx, start.y + i*dy));

    return points;
}

// **************************************************************************************************

PostprocessorSceneChartWidget::PostprocessorSceneChartWidget(PhysicalFieldWidget *fieldWidget, SceneViewChart *sceneChart)
    : PostprocessorSceneWidget(fieldWidget), sceneChart(sceneChart)
{
    setObjectName("PostprocessorChartWidget");

    createControls();
}

void PostprocessorSceneChartWidget::createControls()
{
    // variable
    cmbFieldVariable = new QComboBox();
    connect(cmbFieldVariable, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldVariable(int)));

    // component
    cmbFieldVariableComp = new QComboBox();

    QGridLayout *layoutVariable = new QGridLayout();
    layoutVariable->addWidget(new QLabel(tr("Variable:")), 0, 0);
    layoutVariable->addWidget(cmbFieldVariable, 0, 1);
    layoutVariable->addWidget(new QLabel(tr("Component:")), 1, 0);
    layoutVariable->addWidget(cmbFieldVariableComp, 1, 1);

    // viewer
    geometryViewer = new SceneViewSimpleGeometryChart(this);
    geometryViewer->setMinimumHeight(100);
    
    btnSaveImage = new QPushButton();
    btnSaveImage->setDefault(false);
    btnSaveImage->setEnabled(false);
    btnSaveImage->setText(tr("Save image"));
    connect(btnSaveImage, SIGNAL(clicked()), sceneChart, SLOT(doSaveImage()));

    btnExportData = new QPushButton();
    btnExportData->setDefault(false);
    btnExportData->setEnabled(false);
    btnExportData->setText(tr("Export"));
    connect(btnExportData, SIGNAL(clicked()), sceneChart, SLOT(doExportData()));

    // geometry
    lblStartX = new QLabel("X:");
    lblStartY = new QLabel("Y:");
    lblEndX = new QLabel("X:");
    lblEndY = new QLabel("Y:");

    txtStartX = new LineEditDouble(0.0);
    txtStartY = new LineEditDouble(0.0);
    connect(txtStartX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtStartY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    txtEndX = new LineEditDouble(0.0);
    txtEndY = new LineEditDouble(0.0);
    connect(txtEndX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtEndY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    // start
    QGridLayout *layoutStart = new QGridLayout();
    layoutStart->addWidget(lblStartX, 0, 0);
    layoutStart->addWidget(txtStartX, 0, 1);
    layoutStart->addWidget(lblStartY, 1, 0);
    layoutStart->addWidget(txtStartY, 1, 1);

    QGroupBox *grpStart = new QGroupBox(tr("Start"));
    grpStart->setLayout(layoutStart);

    // end
    QGridLayout *layoutEnd = new QGridLayout();
    layoutEnd->addWidget(lblEndX, 0, 0);
    layoutEnd->addWidget(txtEndX, 0, 1);
    layoutEnd->addWidget(lblEndY, 1, 0);
    layoutEnd->addWidget(txtEndY, 1, 1);

    QGroupBox *grpEnd = new QGroupBox(tr("End"));
    grpEnd->setLayout(layoutEnd);

    QHBoxLayout *layoutStartEnd = new QHBoxLayout();
    layoutStartEnd->addWidget(grpStart);
    layoutStartEnd->addWidget(grpEnd);

    // x - axis
    radHorizontalAxisLength = new QRadioButton(tr("Length"));
    radHorizontalAxisLength->setChecked(true);
    radHorizontalAxisX = new QRadioButton("X");
    radHorizontalAxisY = new QRadioButton("Y");

    QButtonGroup *axisGroup = new QButtonGroup(this);
    axisGroup->addButton(radHorizontalAxisLength);
    axisGroup->addButton(radHorizontalAxisX);
    axisGroup->addButton(radHorizontalAxisY);

    // axis
    QHBoxLayout *layoutAxis = new QHBoxLayout();
    layoutAxis->addWidget(radHorizontalAxisLength);
    layoutAxis->addWidget(radHorizontalAxisX);
    layoutAxis->addWidget(radHorizontalAxisY);

    QGroupBox *grpAxis = new QGroupBox(tr("Horizontal axis"));
    grpAxis->setLayout(layoutAxis);

    // axis points and time step
    txtHorizontalAxisPoints = new QSpinBox(this);
    txtHorizontalAxisPoints->setMinimum(2);
    txtHorizontalAxisPoints->setMaximum(500);
    txtHorizontalAxisPoints->setValue(200);
    chkHorizontalAxisReverse = new QCheckBox(tr("Reverse"));

    // timestep
    QGridLayout *layoutAxisPointsAndTimeStep = new QGridLayout();
    layoutAxisPointsAndTimeStep->setColumnStretch(1, 1);
    layoutAxisPointsAndTimeStep->addWidget(new QLabel(tr("Points:")), 0, 0);
    layoutAxisPointsAndTimeStep->addWidget(txtHorizontalAxisPoints, 0, 1);
    layoutAxisPointsAndTimeStep->addWidget(chkHorizontalAxisReverse, 0, 2);
    layoutAxisPointsAndTimeStep->setColumnStretch(1, 1);

    QGroupBox *grpAxisPointsAndTimeStep = new QGroupBox(tr("Points and time step"), this);
    grpAxisPointsAndTimeStep->setLayout(layoutAxisPointsAndTimeStep);

    // time
    lblPointX = new QLabel("X:");
    lblPointY = new QLabel("Y:");

    txtTimeX = new LineEditDouble(0.0);
    txtTimeY = new LineEditDouble(0.0);
    connect(txtTimeX, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));
    connect(txtTimeY, SIGNAL(textChanged(QString)), this, SLOT(createChartLine()));

    QGridLayout *layoutTime = new QGridLayout();
    layoutTime->addWidget(lblPointX, 0, 0);
    layoutTime->addWidget(txtTimeX, 0, 1);
    layoutTime->addWidget(lblPointY, 1, 0);
    layoutTime->addWidget(txtTimeY, 1, 1);

    QGroupBox *grpTime = new QGroupBox(tr("Point"));
    grpTime->setLayout(layoutTime);

    // button bar
    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->setContentsMargins(0, 0, 0, 0);
    layoutButton->addStretch();
    layoutButton->addWidget(btnSaveImage);
    layoutButton->addWidget(btnExportData);

    QVBoxLayout *layoutChart = new QVBoxLayout();
    layoutChart->setContentsMargins(0, 0, 0, 0);
    layoutChart->addWidget(geometryViewer, 1);
    layoutChart->addLayout(layoutButton);

    // controls geometry
    QVBoxLayout *controlsGeometryLayout = new QVBoxLayout();
    controlsGeometryLayout->addLayout(layoutStartEnd);
    controlsGeometryLayout->addWidget(grpAxis);
    controlsGeometryLayout->addWidget(grpAxisPointsAndTimeStep);
    controlsGeometryLayout->addStretch();

    widGeometry = new QWidget();
    widGeometry->setLayout(controlsGeometryLayout);

    // controls time
    QVBoxLayout *controlsTimeLayout = new QVBoxLayout();
    controlsTimeLayout->addWidget(grpTime);
    controlsTimeLayout->addStretch();

    widTime = new QWidget();
    widTime->setLayout(controlsTimeLayout);

    tbxAnalysisType = new QTabWidget();
    tbxAnalysisType->addTab(widGeometry, tr("Geometry"));
    tbxAnalysisType->addTab(widTime, tr("Time"));

    // controls
    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addLayout(layoutVariable);
    layoutMain->addWidget(tbxAnalysisType);
    layoutMain->addLayout(layoutChart, 1);

    setLayout(layoutMain);
}

void PostprocessorSceneChartWidget::doFieldVariable(int index)
{
    if (!(index >= 0 && m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;

    Module::LocalVariable physicFieldVariable = m_fieldWidget->selectedField()->localVariable(m_fieldWidget->selectedComputation()->config()->coordinateType(), cmbFieldVariable->itemData(index).toString());

    cmbFieldVariableComp->clear();
    if (physicFieldVariable.isScalar())
    {
        cmbFieldVariableComp->addItem(tr("Scalar"), PhysicFieldVariableComp_Scalar);
    }
    else
    {
        cmbFieldVariableComp->addItem(tr("Magnitude"), PhysicFieldVariableComp_Magnitude);
        cmbFieldVariableComp->addItem(m_fieldWidget->selectedComputation()->config()->labelX(), PhysicFieldVariableComp_X);
        cmbFieldVariableComp->addItem(m_fieldWidget->selectedComputation()->config()->labelY(), PhysicFieldVariableComp_Y);
    }

    if (cmbFieldVariableComp->currentIndex() == -1)
        cmbFieldVariableComp->setCurrentIndex(0);
}

void PostprocessorSceneChartWidget::createChartLine()
{
    ChartLine line;

    if (tbxAnalysisType->currentWidget() == widGeometry)
    {
        line = ChartLine(Point(txtStartX->value(), txtStartY->value()),
                         Point(txtEndX->value(), txtEndY->value()),
                         txtHorizontalAxisPoints->value(),
                         chkHorizontalAxisReverse->isChecked());
    }
    if (tbxAnalysisType->currentWidget() == widTime)
    {
        line = ChartLine(Point(txtTimeX->value(), txtTimeY->value()),
                         Point(txtTimeX->value(), txtTimeY->value()),
                         0.0,
                         0);
    }

    geometryViewer->setChartLine(line);
    geometryViewer->doZoomBestFit();
}

void PostprocessorSceneChartWidget::refresh()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget && m_fieldWidget->selectedField()))
        return;

    // set computation
    if (!m_fieldWidget->selectedComputation().isNull())
        geometryViewer->setProblem(static_cast<QSharedPointer<ProblemBase> >(m_fieldWidget->selectedComputation()));

    fillComboBoxScalarVariable(m_fieldWidget->selectedComputation()->config()->coordinateType(), m_fieldWidget->selectedField(), cmbFieldVariable);
    doFieldVariable(cmbFieldVariable->currentIndex());

    // correct labels
    lblStartX->setText(m_fieldWidget->selectedComputation()->config()->labelX() + ":");
    lblStartY->setText(m_fieldWidget->selectedComputation()->config()->labelY() + ":");
    lblEndX->setText(m_fieldWidget->selectedComputation()->config()->labelX() + ":");
    lblEndY->setText(m_fieldWidget->selectedComputation()->config()->labelY() + ":");
    lblPointX->setText(m_fieldWidget->selectedComputation()->config()->labelX() + ":");
    lblPointY->setText(m_fieldWidget->selectedComputation()->config()->labelY() + ":");
    radHorizontalAxisX->setText(m_fieldWidget->selectedComputation()->config()->labelX());
    radHorizontalAxisY->setText(m_fieldWidget->selectedComputation()->config()->labelY());

    if (m_fieldWidget->selectedComputation()->isTransient())
    {
        widTime->setEnabled(true);
    }
    else
    {
        widTime->setEnabled(false);
        widGeometry->setEnabled(true);
        tbxAnalysisType->setCurrentWidget(widGeometry);
    }
}

void PostprocessorSceneChartWidget::load()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget && m_fieldWidget->selectedField()))
        return;

    txtStartX->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartStartX).toDouble());
    txtStartY->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartStartY).toDouble());
    txtEndX->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartEndX).toDouble());
    txtEndY->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartEndY).toDouble());
    txtTimeX->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartTimeX).toDouble());
    txtTimeY->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartTimeY).toDouble());
    radHorizontalAxisX->setChecked((ChartAxisType) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxis).toInt() == ChartAxis_X);
    radHorizontalAxisY->setChecked((ChartAxisType) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxis).toInt() == ChartAxis_Y);
    radHorizontalAxisLength->setChecked((ChartAxisType) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxis).toInt() == ChartAxis_Length);
    txtHorizontalAxisPoints->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxisPoints).toInt());
    chkHorizontalAxisReverse->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxisReverse).toBool());

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString()));
    if (cmbFieldVariable->count() > 0 && cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()) != QVariant::Invalid)
    {
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartVariable, cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
        doFieldVariable(cmbFieldVariable->currentIndex());

        cmbFieldVariableComp->setCurrentIndex(cmbFieldVariableComp->findData((PhysicFieldVariableComp) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartVariableComp).toInt()));
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartVariableComp, (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt());
    }
    if (cmbFieldVariable->currentIndex() == -1 && cmbFieldVariable->count() > 0)
    {
        // set first variable
        cmbFieldVariable->setCurrentIndex(0);
    }

    if ((ChartMode) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartMode).toInt() == ChartMode_Geometry)
        tbxAnalysisType->setCurrentWidget(widGeometry);
    else if ((ChartMode) m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ChartMode).toInt() == ChartMode_Time)
        tbxAnalysisType->setCurrentWidget(widTime);

    btnSaveImage->setEnabled(sceneChart->chart()->graph()->data()->size() > 0);
    btnExportData->setEnabled(sceneChart->chart()->graph()->data()->size() > 0);
}

void PostprocessorSceneChartWidget::save()
{
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartStartX, txtStartX->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartStartY, txtStartY->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartEndX, txtEndX->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartEndY, txtEndY->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartTimeX, txtTimeX->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartTimeY, txtTimeY->value());
    if (radHorizontalAxisX->isChecked())
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartHorizontalAxis, ChartAxis_X);
    else if (radHorizontalAxisY->isChecked())
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartHorizontalAxis, ChartAxis_Y);
    else if (radHorizontalAxisLength->isChecked())
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartHorizontalAxis, ChartAxis_Length);
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartHorizontalAxisReverse, chkHorizontalAxisReverse->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartHorizontalAxisPoints, txtHorizontalAxisPoints->value());
    if (tbxAnalysisType->currentWidget() == widGeometry)
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartMode, ChartMode_Geometry);
    else if (tbxAnalysisType->currentWidget() == widTime)
        m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartMode, ChartMode_Time);
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartVariable, cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ChartVariableComp, (PhysicFieldVariableComp) cmbFieldVariableComp->itemData(cmbFieldVariableComp->currentIndex()).toInt());

    createChartLine();
}

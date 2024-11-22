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

#include "optilab.h"
#include "optilab_study.h"
#include "optilab/parameter.h"
#include "optilab/study.h"
#include "optilab/study_dialog.h"
#include "util/global.h"

#include "app/sceneview_data.h"

#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/plugin_interface.h"

#include "app/sceneview_geometry_simple.h"

#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/normal.hpp>


class StatisticsLinReg
{
public:
    StatisticsLinReg(QVector<double> x, QVector<double> y) : x(x), y(y), m_slope(0.0), m_shift(0.0), m_correlation_coef(0.0)
    {
    }

    bool computeLinReg()
    {
        assert(x.size() == y.size());
        int n = x.size();

        double sumx = 0.0;
        double sumx2 = 0.0;
        double sumxy = 0.0;
        double sumy = 0.0;
        double sumy2 = 0.0;

        for (int i = 0; i < n; i++)
        {
            sumx  += x[i];
            sumx2 += x[i] * x[i];
            sumxy += x[i] * y[i];
            sumy  += y[i];
            sumy2 += y[i] * y[i];
        }

        double denom = (n * sumx2 - sumx*sumx);
        if (denom == 0)
        {
            // singular matrix. can't solve the problem.
            return false;
        }

        // slope
        m_slope = (n * sumxy  -  sumx * sumy) / denom;
        // shift
        m_shift = (sumy * sumx2  -  sumx * sumxy) / denom;

        // correlation coef
        m_correlation_coef = (sumxy - sumx * sumy / n) / sqrt((sumx2 - sumx*sumx/n) * (sumy2 - sumy*sumy/n));

        return true;
    }

    inline double slope() const { return m_slope; }
    inline double shift() const { return m_shift; }
    inline double correlation_coef() const { return m_correlation_coef; }

private:
    QVector<double> x;
    QVector<double> y;

    double m_slope;
    double m_shift;
    double m_correlation_coef;
};

// ***************************************************************************************************************************************

OptiLab::OptiLab(QWidget *parent) : QWidget(parent), m_selectedComputation(nullptr)
{
    actSceneModeOptiLab = new QAction(icon("menu_study"), tr("OptiLab"), this);
    actSceneModeOptiLab->setShortcut(Qt::Key_F5);
    actSceneModeOptiLab->setCheckable(true);
    actSceneModeOptiLab->setEnabled(false);

    createControls();
}

OptiLab::~OptiLab()
{
}

QWidget *OptiLab::createControlsChart()
{
    actExportToCsv = new QAction(tr("Export data to CSV..."), this);
    actExportToCsv->setIcon(icon("geometry_zoom"));
    connect(actExportToCsv, SIGNAL(triggered(bool)), this, SLOT(exportData()));

    actChartRescale = new QAction(tr("Rescale chart"), this);
    connect(actChartRescale, SIGNAL(triggered(bool)), this, SLOT(chartRescale(bool)));

    actChartShowParetoFront = new QAction(icon("optilab_pareto"), tr("Show Pareto front"), this);
    actChartShowParetoFront->setCheckable(true);
    QObject::connect(actChartShowParetoFront, &QAction::triggered, this, &OptiLab::doChartRefreshed);

    actChartShowTrend = new QAction(icon("optilab_trend"), tr("Show trend line"), this);
    actChartShowTrend->setCheckable(true);
    QObject::connect(actChartShowTrend, &QAction::triggered, this, &OptiLab::doChartRefreshed);

    actChartShowAverageValue = new QAction(icon("optilab_avg"), tr("Show average value"), this);
    actChartShowAverageValue->setCheckable(true);
    QObject::connect(actChartShowAverageValue, &QAction::triggered, this, &OptiLab::doChartRefreshed);

    mnuChart = new QMenu(this);
    mnuChart->addAction(actChartRescale);
    mnuChart->addSeparator();
    mnuChart->addAction(actChartShowTrend);
    mnuChart->addAction(actChartShowAverageValue);
    mnuChart->addAction(actChartShowParetoFront);

    auto chart = new QChart();
    chart->legend()->hide();

    // axis x
    axisX = new QValueAxis;
    axisX->setLabelFormat("%g");
    axisX->setGridLineVisible(true);
    axisX->setTitleText(tr("x"));
    chart->addAxis(axisX, Qt::AlignBottom);

    // axis y
    axisY = new QValueAxis;
    axisY->setLabelFormat("%g");
    axisY->setGridLineVisible(true);
    axisY->setTitleText(tr("y"));
    chart->addAxis(axisY, Qt::AlignLeft);

    valueSelectedSeries = new QScatterSeries();
    valueSelectedSeries->setUseOpenGL(true);
    chart->addSeries(valueSelectedSeries);
    valueSelectedSeries->setPointsVisible(true);
    valueSelectedSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    valueSelectedSeries->setMarkerSize(17.0);
    valueSelectedSeries->setColor(QColor(219, 13, 58));
    valueSelectedSeries->attachAxis(axisX);
    valueSelectedSeries->attachAxis(axisY);

    valueHoverSeries = new QScatterSeries();
    valueHoverSeries->setUseOpenGL(true);
    chart->addSeries(valueHoverSeries);
    valueHoverSeries->setPointsVisible(true);
    valueHoverSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    valueHoverSeries->setMarkerSize(15.0);
    valueHoverSeries->setColor(QColor(94, 3, 23));
    valueHoverSeries->attachAxis(axisX);
    valueHoverSeries->attachAxis(axisY);

    valueSeries = new QScatterSeries();
    valueSeries->setUseOpenGL(true);
    chart->addSeries(valueSeries);
    valueSeries->setPointsVisible(true);
    valueSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    valueSeries->setMarkerSize(10.0);
    valueSeries->setColor(QColor(80, 120, 160));
    valueSeries->attachAxis(axisX);
    valueSeries->attachAxis(axisY);
    QObject::connect(valueSeries, &QScatterSeries::clicked, this, &OptiLab::chartClicked);
    QObject::connect(valueSeries, &QScatterSeries::hovered, this, &OptiLab::chartHovered);

    // chart trend line
    trendLineSeries = new QLineSeries();
    trendLineSeries->setUseOpenGL(true);
    chart->addSeries(trendLineSeries);
    trendLineSeries->setPen(QPen(QBrush(QColor(90, 175, 90)), 1, Qt::DashLine));
    trendLineSeries->attachAxis(axisX);
    trendLineSeries->attachAxis(axisY);

    // mean value
    averageValueSeries = new QLineSeries();
    averageValueSeries->setUseOpenGL(true);
    chart->addSeries(averageValueSeries);
    averageValueSeries->setPen(QPen(QBrush(QColor(140, 140, 140)), 1, Qt::DotLine));
    averageValueSeries->attachAxis(axisX);
    averageValueSeries->attachAxis(axisY);

    averageValueLowerSeries = new QLineSeries();
    averageValueLowerSeries->setUseOpenGL(true);
    averageValueLowerSeries->setPen(QPen(QBrush(QColor(180, 180, 180)), 2, Qt::DotLine));

    averageValueUpperSeries = new QLineSeries();
    averageValueUpperSeries->setUseOpenGL(true);
    averageValueUpperSeries->setPen(QPen(QBrush(QColor(180, 180, 180)), 2, Qt::DotLine));

    averageValueAreaSeries = new QAreaSeries(averageValueLowerSeries, averageValueUpperSeries);
    chart->addSeries(averageValueAreaSeries);
    averageValueSeries->setBrush(QBrush(QColor(255, 50, 30, 10)));
    averageValueAreaSeries->setOpacity(0.05);
    averageValueAreaSeries->attachAxis(axisX);
    averageValueAreaSeries->attachAxis(axisY);

    // // Pareto front
    paretoFrontSeries = new QScatterSeries();
    paretoFrontSeries->setUseOpenGL(true);
    chart->addSeries(paretoFrontSeries);
    paretoFrontSeries->setMarkerSize(10.0);
    paretoFrontSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    paretoFrontSeries->setColor(QColor(150, 110, 110));
    paretoFrontSeries->attachAxis(axisX);
    paretoFrontSeries->attachAxis(axisY);
    QObject::connect(paretoFrontSeries, &QScatterSeries::clicked, this, &OptiLab::chartClicked);
    QObject::connect(paretoFrontSeries, &QScatterSeries::hovered, this, &OptiLab::chartHovered);

    chartView = new ChartView(chart, false);
    chartView->setContentsMargins(0, 0, 0, 0);
    chartView->setChart(chart);
    // QObject::connect(chartView, &QChartView::contextMenuEvent, this, &OptiLab::contextMenuEvent);

    actResultsFindMinimum = new QAction(icon("optilab_min"), tr("Find minimum"), this);
    actResultsFindMinimum->setIconVisibleInMenu(true);
    connect(actResultsFindMinimum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMinimum(bool)));
    actResultsFindMaximum = new QAction(icon("optilab_max"), tr("Find maximum"), this);
    actResultsFindMaximum->setIconVisibleInMenu(true);
    connect(actResultsFindMaximum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMaximum(bool)));

    // scene - zoom
    actSceneZoomIn = new QAction(tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(actSceneZoomIn, SIGNAL(triggered()), this, SLOT(doZoomIn()));

    actSceneZoomOut = new QAction(tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(actSceneZoomOut, SIGNAL(triggered()), this, SLOT(doZoomOut()));

    actSceneZoomBestFit = new QAction(tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));
    connect(actSceneZoomBestFit, SIGNAL(triggered()), this, SLOT(doZoomBestFit()));

    // zoom
    auto *mnuZoom = new QMenu(this);
    mnuZoom->addAction(actSceneZoomBestFit);
    mnuZoom->addAction(actSceneZoomIn);
    mnuZoom->addAction(actSceneZoomOut);

    auto *zoomButton = new QToolButton();
    zoomButton->setText(tr("Zoom"));
    zoomButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    zoomButton->setIconSize(QSize(24, 24));
    zoomButton->setMenu(mnuZoom);
    zoomButton->setAutoRaise(true);
    zoomButton->setIcon(icon("geometry_zoom"));
    zoomButton->setPopupMode(QToolButton::InstantPopup);

    // export
    auto *mnuExport = new QMenu(this);
    mnuExport->addAction(actExportToCsv);

    auto *exportButton = new QToolButton();
    exportButton->setText(tr("Export"));
    exportButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    exportButton->setIconSize(QSize(24, 24));
    exportButton->setMenu(mnuExport);
    exportButton->setAutoRaise(true);
    exportButton->setIcon(icon("export"));
    exportButton->setPopupMode(QToolButton::InstantPopup);

    // right toolbar
    toolBarRight = new QToolBar();
    toolBarRight->setProperty("modulebar", true);
    toolBarRight->setProperty("os", operatingSystem());
    toolBarRight->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBarRight->addAction(actResultsFindMinimum);
    toolBarRight->addAction(actResultsFindMaximum);
    toolBarRight->addSeparator();
    toolBarRight->addAction(actChartShowAverageValue);
    toolBarRight->addAction(actChartShowTrend);
    toolBarRight->addAction(actChartShowParetoFront);
    toolBarRight->addSeparator();
    toolBarRight->addWidget(zoomButton);
    toolBarRight->addSeparator();
    toolBarRight->addWidget(exportButton);

    auto layoutRight = new QVBoxLayout();
    layoutRight->setContentsMargins(0, 0, 0, 0);
    layoutRight->addWidget(toolBarRight);
    layoutRight->addWidget(chartView);

    auto *widRight = new QWidget();
    widRight->setContentsMargins(0, 0, 0, 0);
    widRight->setLayout(layoutRight);

    return widRight;
}

QWidget *OptiLab::createControlsResults()
{
    // treeview
    trvResults = new QTreeWidget(this);
    trvResults->setExpandsOnDoubleClick(false);
    trvResults->setHeaderHidden(true);
    trvResults->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trvResults->setContextMenuPolicy(Qt::CustomContextMenu);
    trvResults->setMouseTracking(true);
    trvResults->setUniformRowHeights(true);
    trvResults->setColumnCount(2);
    trvResults->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trvResults->header()->resizeSection(0, 220);
    trvResults->header()->setStretchLastSection(true);
    trvResults->setIndentation(trvResults->indentation() - 2);

    geometryViewer = new SceneViewSimpleGeometry(this);

    auto *layoutResults = new QVBoxLayout();
    layoutResults->setContentsMargins(2, 2, 2, 2);
    layoutResults->addWidget(trvResults, 2);
    layoutResults->addWidget(geometryViewer, 1);

    auto *widgetSolution = new QWidget();
    widgetSolution->setLayout(layoutResults);

    return widgetSolution;
}

void OptiLab::createControls()
{
    actComputationSolve = new QAction(icon("main_solve"), tr("Show selected solution"), this);
    // actComputationSolve->setShortcut(QKeySequence("Alt+A"));
    connect(actComputationSolve, &QAction::triggered, this, &OptiLab::doSolveCurrentComputation);

    // right toolbar
    auto *toolBarApply = new QToolBar();
    toolBarApply->setContentsMargins(0, 0, 10, 0);
    toolBarApply->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBarApply->addAction(actComputationSolve);

    auto *layoutButtons = new QHBoxLayout();
    layoutButtons->setContentsMargins(0, 0, 0, 0);
    layoutButtons->addStretch();
    layoutButtons->addWidget(toolBarApply);

    cmbStudies = new QComboBox(this);
    connect(cmbStudies, SIGNAL(currentIndexChanged(int)), this, SLOT(doStudyChanged(int)));

    cmbAxisX = new QComboBox(this);
    connect(cmbAxisX, SIGNAL(currentIndexChanged(int)), this, SLOT(axisXChanged(int)));
    cmbAxisY = new QComboBox(this);
    connect(cmbAxisY, SIGNAL(currentIndexChanged(int)), this, SLOT(axisYChanged(int)));

    auto *layoutCombo = new QFormLayout();
    layoutCombo->addRow(tr("Studies:"), cmbStudies);
    layoutCombo->addRow(tr("Horizontal axis:"), cmbAxisX);
    layoutCombo->addRow(tr("Vertical axis:"), cmbAxisY);

    auto layoutLeft = new QVBoxLayout();
    layoutLeft->addLayout(layoutCombo);
    layoutLeft->addLayout(layoutButtons);
    layoutLeft->addWidget(createControlsResults(), 3);

    auto *widLeft = new QWidget();
    widLeft->setMinimumWidth(400);
    widLeft->setMaximumWidth(400);
    widLeft->setLayout(layoutLeft);

    auto *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(widLeft);
    layoutMain->addWidget(createControlsChart());
    layoutMain->setStretch(2, 1);

    setLayout(layoutMain);
}

void OptiLab::refresh()
{
    bool enabled = false;
    for (int i = 0; i < Agros::problem()->studies()->items().count(); i++)
    {
        Study *study = Agros::problem()->studies()->items().at(i);

        if (study->computationsCount() > 0)
        {
            enabled = true;
            break;
        }
    }

    actSceneModeOptiLab->setEnabled(enabled);

    Study *selectedStudy = nullptr;
    if (cmbStudies->currentIndex() != -1)
        selectedStudy = cmbStudies->itemData(cmbStudies->currentIndex()).value<Study *>();

    cmbStudies->clear();
    for (int i = 0; i < Agros::problem()->studies()->items().count(); i++)
    {
        Study *study = Agros::problem()->studies()->items().at(i);

        // only studies with computations
        if (study->computationsCount() > 0)
            cmbStudies->addItem(studyTypeString(study->type()), QVariant::fromValue(study));
    }

    if (selectedStudy)
    {
        if (cmbStudies->count() > 0)
        {
            for (int i = 0; i < Agros::problem()->studies()->items().count(); i++)
            {
                Study *study = Agros::problem()->studies()->items().at(i);

                if (study == selectedStudy)
                {
                    cmbStudies->setCurrentIndex(i);
                    break;
                }
            }
        }
    }
}

void OptiLab::chartContextMenu(const QPoint &pos)
{
    mnuChart->exec(QCursor::pos());
}

void OptiLab::chartRescale(bool checked)
{
    doChartRefreshed();
    chartView->fitToData();
}

void OptiLab::chartShowTrend(int state)
{
    actChartShowTrend->setChecked(state);
    doChartRefreshed();
}

void OptiLab::chartShowAverageValue(int state)
{
    actChartShowAverageValue->setChecked(state);
    doChartRefreshed();
}

void OptiLab::chartShowParetoFront(int state)
{
    actChartShowParetoFront->setChecked(state);
    doChartRefreshed();
}

void OptiLab::doStudySolved(Study *study)
{
    refresh();
    doStudySelected(study);

    actSceneModeOptiLab->trigger();
}

void OptiLab::doStudyChanged(int index)
{
    if (index < 0 || index >= cmbStudies->count())
        return;

    Study *study = cmbStudies->itemData(index).value<Study *>();
    doStudySelected(study);
}

void OptiLab::doStudySelected(Study *study)
{
    // computations
    QSharedPointer<Computation> currentComputation = nullptr;
    m_studyComputations.clear();
    bool selectedComputationIsInStudy = false;

    // study
    if (study)
    {
        double min = numeric_limits<double>::max();

        // fill tree view
        QList<ComputationSet> computationSets = study->computationSets();
        for (int i = 0; i < computationSets.size(); i++)
        {
            foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
            {
                m_studyComputations.append(computation);

                // selected computation is in study
                if (computation == m_selectedComputation)
                    selectedComputationIsInStudy = true;

                // select minimum
                double localMin = study->evaluateSingleGoal(computation);
                if (localMin < min)
                {
                    min = localMin;
                    currentComputation = computation;
                }
            }
        }

        // parameters, goal functions and recipes
        QString currentNameX = "";
        if (cmbAxisX->count() > 0)
            currentNameX = cmbAxisX->currentText();
        cmbAxisX->clear();

        QString currentNameY = "";
        if (cmbAxisY->count() > 0)
            currentNameY = cmbAxisY->currentText();
        else if (study->goalFunctions().count() > 0)
        {
            // default
            GoalFunction goal = study->goalFunctions().at(0);
            currentNameY = tr("%1 (goal function)").arg(goal.name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Goal, goal.name()));
        }
        cmbAxisY->clear();

        // steps
        cmbAxisX->addItem(tr("steps"), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Steps, "")));
        cmbAxisY->addItem(tr("steps"), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Steps, "")));

        // parameters
        foreach (Parameter parameter, study->parameters())
        {
            cmbAxisX->addItem(tr("%1 (parameter)").arg(parameter.name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Parameter, parameter.name())));
            cmbAxisY->addItem(tr("%1 (parameter)").arg(parameter.name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Parameter, parameter.name())));
        }

        // goal
        foreach (GoalFunction goal, study->goalFunctions())
        {
            cmbAxisX->addItem(tr("%1 (goal function)").arg(goal.name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Goal, goal.name())));
            cmbAxisY->addItem(tr("%1 (goal function)").arg(goal.name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Goal, goal.name())));
        }

        // recipes
        foreach (ResultRecipe *recipe, Agros::problem()->recipes()->items())
        {
            cmbAxisX->addItem(tr("%1 (recipe)").arg(recipe->name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Recipe, recipe->name())));
            cmbAxisY->addItem(tr("%1 (recipe)").arg(recipe->name()), QVariant::fromValue(QPair<Study::ResultType, QString>(Study::ResultType::ResultType_Recipe, recipe->name())));
        }

        // select combobox
        if (!currentNameX.isEmpty())
            cmbAxisX->setCurrentText(currentNameX);
        if (!currentNameY.isEmpty())
            cmbAxisY->setCurrentText(currentNameY);

        if (study->computationSets().count() == 0)
        {
            trvResults->clear();
            geometryViewer->setProblem(nullptr);
        }
    }
    else
    {
        cmbAxisX->clear();
        cmbAxisY->clear();

        trvResults->clear();
        geometryViewer->setProblem(nullptr);
    }

    // actions
    actComputationSolve->setEnabled(false);
    actResultsFindMinimum->setEnabled(false);
    actResultsFindMaximum->setEnabled(false);
    actChartRescale->setEnabled(false);
    actChartShowTrend->setEnabled(false);
    actChartShowAverageValue->setEnabled(false);
    actChartShowParetoFront->setEnabled(false);

    actSceneZoomIn->setEnabled(false);
    actSceneZoomOut->setEnabled(false);
    actSceneZoomBestFit->setEnabled(false);

    cmbAxisX->setEnabled(study != nullptr);
    cmbAxisY->setEnabled(study != nullptr);
    trvResults->setEnabled(study != nullptr);
    chartView->setEnabled(study != nullptr);

    // select current computation
    if (m_selectedComputation.isNull() || !selectedComputationIsInStudy)
    {
        if (!currentComputation.isNull())
            m_selectedComputation = currentComputation;
    }
    else if (!selectedComputationIsInStudy)
    {
        m_selectedComputation = nullptr;
    }

    if (m_studyComputations.count() > 0 && !m_selectedComputation.isNull())
    {
        doComputationSelected(m_selectedComputation);

        // replot chart
        chartView->fitToData();
        geometryViewer->doZoomBestFit();
    }
}

void OptiLab::doChartRefreshed()
{
    m_computationMap.clear();

    // series
    valueSeries->clear();
    valueSelectedSeries->clear();
    averageValueSeries->clear();
    averageValueLowerSeries->clear();
    averageValueUpperSeries->clear();
    trendLineSeries->clear();
    paretoFrontSeries->clear();

    if (cmbStudies->count() == 0)
        return;

    Study *study = cmbStudies->itemData(cmbStudies->currentIndex()).value<Study *>();

    QList<ComputationSet> computationSets = study->computationSets();
    if (computationSets.count() == 0)
    {
        // fit and replot chart
        axisX->setRange(-1, 1);
        axisY->setRange(-1, 1);

        return;
    }



    // axis x label
    axisX->setTitleText(cmbAxisX->currentText());
    auto currentDataAxisX = cmbAxisX->currentData().value<QPair<Study::ResultType, QString> >();
    Study::ResultType currentDataResultAxisX = currentDataAxisX.first;
    QString currentDataNameX = currentDataAxisX.second;

    // axis y label
    axisY->setTitleText(cmbAxisY->currentText());
    auto currentDataAxisY = cmbAxisY->currentData().value<QPair<Study::ResultType, QString> >();
    Study::ResultType currentDataResultAxisY = currentDataAxisY.first;
    QString currentDataNameY = currentDataAxisY.second;

    // linear regression
    QVector<double> linRegDataX;
    QVector<double> linRegDataY;

    // data
    QList<QSharedPointer<Computation> > paretoFront = study->nondominatedSort(computationSets);
    QVector<double> paretoDataX;
    QVector<double> paretoDataY;

    int paletteStep = computationSets.count() > 1 ? (PALETTEENTRIES - 1) / (computationSets.count() - 1) : 0;
    int step = 0;
    for (int i = 0; i < computationSets.count(); i++)
    {
        QVector<double> dataSetX;
        QVector<double> dataSetY;

        QList<QSharedPointer<Computation> > computations = computationSets[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            QSharedPointer<Computation> computation = computations[j];

            step++;

            // steps
            if (currentDataResultAxisX == Study::ResultType::ResultType_Steps)
                dataSetX.append(step);
            if (currentDataResultAxisY == Study::ResultType::ResultType_Steps)
                dataSetY.append(step);

            // parameters
            if (currentDataResultAxisX == Study::ResultType::ResultType_Parameter)
                dataSetX.append(computation->config()->parameters()->number(currentDataNameX));
            if (currentDataResultAxisY == Study::ResultType::ResultType_Parameter)
                dataSetY.append(computation->config()->parameters()->number(currentDataNameY));

            // functionals
            if (currentDataResultAxisX == Study::ResultType::ResultType_Goal)
                dataSetX.append(computation->results()->value(currentDataNameX));
            if (currentDataResultAxisY == Study::ResultType::ResultType_Goal)
                dataSetY.append(computation->results()->value(currentDataNameY));

            // recipes
            if (currentDataResultAxisX == Study::ResultType::ResultType_Recipe)
                dataSetX.append(computation->results()->value(currentDataNameX));
            if (currentDataResultAxisY == Study::ResultType::ResultType_Recipe)
                dataSetY.append(computation->results()->value(currentDataNameY));

            // selected point
            if (m_selectedComputation)
            {
                if (computation->problemDir() == m_selectedComputation->problemDir())
                {
                    valueSelectedSeries->clear();
                    valueSelectedSeries->append(dataSetX.last(), dataSetY.last());
                }
            }

            // computation map
            m_computationMap[i].insert(QPair<double, double>(dataSetX.last(), dataSetY.last()), computation);

            // Pareto front
            if (paretoFront.contains(computation))
            {
                paretoDataX.append(dataSetX.last());
                paretoDataY.append(dataSetY.last());
            }
        }

        int colorFillIndex = i * paletteStep;
        const QColor colorFill(paletteDataAgros[colorFillIndex][0] * 255, paletteDataAgros[colorFillIndex][1] * 255, paletteDataAgros[colorFillIndex][2] * 255);

        // add data to global array (for min and max computation)
        linRegDataX << dataSetX;
        linRegDataY << dataSetY;

        // add data
        for (int k = 0; k < dataSetX.size(); k++)
            valueSeries->append(dataSetX[k], dataSetY[k]);
    }

    // Pareto front
    if (actChartShowParetoFront->isChecked())
    {
        for (int i; i < paretoDataX.size(); i++)
        {
            paretoFrontSeries->append(paretoDataX[i], paretoDataY[i]);
        }
    }

    // mean value and standard deviation
    Statistics statsX(linRegDataX);
    Statistics statsY(linRegDataY);

    // bounding box
    double delta = qMin((statsX.max() - statsX.min()) * 0.1, (statsY.max() - statsY.min()) * 0.1);

    if (actChartShowAverageValue->isChecked())
    {
        averageValueSeries->append(statsX.min() - delta, statsY.mean());
        averageValueSeries->append(statsX.max() + delta, statsY.mean());
        averageValueLowerSeries->append(statsX.min() - delta, statsY.mean() - statsY.stdDev());
        averageValueLowerSeries->append(statsX.max() + delta, statsY.mean() - statsY.stdDev());
        averageValueUpperSeries->append(statsX.min() - delta, statsY.mean() + statsY.stdDev());
        averageValueUpperSeries->append(statsX.max() + delta, statsY.mean() + statsY.stdDev());
    }

    // linear regression
    if (actChartShowTrend->isChecked())
    {
        StatisticsLinReg linReg(linRegDataX, linRegDataY);
        if (linReg.computeLinReg())
        {
            // trend arrow
            QPointF start((statsX.min() - delta), (statsX.min() - delta) * linReg.slope() + linReg.shift());
            QPointF end((statsX.max() + delta), (statsX.max() + delta) * linReg.slope() + linReg.shift());

            trendLineSeries->append(start.x(), start.y());
            trendLineSeries->append(end.x(), end.y());
        }
    }
}

void OptiLab::chartClicked(const QPointF &point)
{
    int index = findPointIndex(point);
    if (index != -1)
        doComputationSelected(m_studyComputations[index]);
}

void OptiLab::chartHovered(const QPointF &point, bool state)
{
    if (!state)
    {
        valueHoverSeries->clear();
        return;
    }

    int index = findPointIndex(point);
    if (index != -1)
    {
        valueHoverSeries->clear();
        valueHoverSeries->append(valueSeries->points().at(index));
    }
}

void OptiLab::doComputationSelected(const QSharedPointer<Computation> computation)
{
    m_selectedComputation = computation;
    if (!m_selectedComputation.isNull())
    {
        Study *study = cmbStudies->itemData(cmbStudies->currentIndex()).value<Study *>();

        // cache value
        auto currentDataAxisY = cmbAxisY->currentData().value<QPair<Study::ResultType, QString> >();
        Study::ResultType selectedType = currentDataAxisY.first;
        QString selectedKey = currentDataAxisY.second;

        trvResults->clear();

        geometryViewer->setProblem(static_cast<QSharedPointer<ProblemBase> >(m_selectedComputation));

        // fill treeview
        QFont fnt = trvResults->font();
        fnt.setBold(true);

        auto *statisticsNode = new QTreeWidgetItem(trvResults);
        statisticsNode->setText(0, tr("Statistics"));
        statisticsNode->setFont(0, fnt);
        statisticsNode->setIcon(0, icon("field_properties"))  ;
        statisticsNode->setExpanded(true);

        auto *solutionsNode = new QTreeWidgetItem(statisticsNode);
        solutionsNode->setText(0, tr("Number of computations"));
        solutionsNode->setText(1, QString::number(study->computationsCount()));

        // parameters
        auto *parametersNode = new QTreeWidgetItem(trvResults);
        parametersNode->setText(0, tr("Parameters"));
        parametersNode->setFont(0, fnt);
        parametersNode->setIcon(0, icon("menu_parameter"))  ;
        parametersNode->setExpanded(true);

        QMap<QString, ProblemParameter> parameters = m_selectedComputation->config()->parameters()->items();
        foreach (Parameter parameter, study->parameters())
        {
            auto *parameterNode = new QTreeWidgetItem(parametersNode);
            parameterNode->setText(0, parameter.name());
            parameterNode->setText(1, QString::number(parameters[parameter.name()].value()));
            parameterNode->setData(0, Qt::UserRole, parameter.name());
            parameterNode->setData(1, Qt::UserRole, Study::ResultType::ResultType_Parameter);
        }

        // goal functions
        auto *goalNode = new QTreeWidgetItem(trvResults);
        goalNode->setText(0, tr("Goal Functions"));
        goalNode->setFont(0, fnt);
        goalNode->setIcon(0, icon("menu_function"));
        goalNode->setExpanded(true);

        // recipes
        auto *recipesNode = new QTreeWidgetItem(trvResults);
        recipesNode->setText(0, tr("Recipes"));
        recipesNode->setFont(0, fnt);
        recipesNode->setIcon(0, icon("menu_recipe"));
        recipesNode->setExpanded(true);

        StringToDoubleMap results = m_selectedComputation->results()->items();
        foreach (QString key, results.keys())
        {
            QTreeWidgetItem *item = nullptr;
            if (m_selectedComputation->results()->type(key) == ComputationResultType_Functional)
            {
                item = new QTreeWidgetItem(goalNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Goal);
            }
            else if (m_selectedComputation->results()->type(key) == ComputationResultType_Recipe)
            {
                item = new QTreeWidgetItem(recipesNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Recipe);
            }
            else
                assert(0);

            item->setText(0, key);
            item->setText(1, QString::number(results[key]));
            item->setData(0, Qt::UserRole, key);
        }

        // paint chart
        doChartRefreshed();

        // actions
        actComputationSolve->setEnabled(true);
        actResultsFindMinimum->setEnabled(true);
        actResultsFindMaximum->setEnabled(true);
        actChartRescale->setEnabled(true);
        actChartShowTrend->setEnabled(true);
        actChartShowAverageValue->setEnabled(true);
        actChartShowParetoFront->setEnabled(true);

        actSceneZoomIn->setEnabled(true);
        actSceneZoomOut->setEnabled(true);
        actSceneZoomBestFit->setEnabled(true);

        trvResults->setEnabled(study != nullptr);
    }
}

void OptiLab::doSolveCurrentComputation()
{
    if (m_selectedComputation)
    {
        emit solveCurrentComputation(m_selectedComputation);
    }
}

void OptiLab::doResultChanged()
{
    if (cmbStudies->count() > 0)
    {
        Study *study = cmbStudies->itemData(cmbStudies->currentIndex()).value<Study *>();

        QList<ComputationSet> computationSets = study->computationSets();

        auto currentDataAxisY = cmbAxisY->currentData().value<QPair<Study::ResultType, QString> >();
        Study::ResultType type = currentDataAxisY.first;
        QString key = currentDataAxisY.second;

        QVector<double> step;
        QVector<double> data;

        for (int i = 0; i < computationSets.count(); i++)
        {
            QList<QSharedPointer<Computation> > computations = computationSets[i].computations();

            for (int j = 0; j < computations.count(); j++)
            {
                QSharedPointer<Computation> computation = computations[j];

                if (type == Study::ResultType_Parameter)
                    data.append(computation->config()->parameters()->number(key));
                else if (type == Study::ResultType_Recipe || type == Study::ResultType_Goal)
                    data.append(computation->results()->value(key));


                step.append(step.count());
            }
        }
    }
}

void OptiLab::resultsFindMinimum(bool checked)
{
    resultsFindExtrem(true);
}

void OptiLab::resultsFindMaximum(bool checked)
{
    resultsFindExtrem(false);
}

void OptiLab::resultsFindExtrem(bool minimum)
{
    if (cmbStudies->count() > 0)
    {
        Study *study = cmbStudies->itemData(cmbStudies->currentIndex()).value<Study *>();

        if (cmbAxisY->currentIndex() != -1)
        {
            auto currentDataAxisY = cmbAxisY->currentData().value<QPair<Study::ResultType, QString> >();
            Study::ResultType currentDataResultAxisY = currentDataAxisY.first;
            QString currentDataNameY = currentDataAxisY.second;

            // extreme
            QSharedPointer<Computation> selectedComputation = study->findExtreme(currentDataResultAxisY, currentDataNameY, minimum);

            if (!selectedComputation.isNull())
                doComputationSelected(selectedComputation);
        }
    }
}

void OptiLab::axisXChanged(int index)
{
    if (index != -1)
    {
        doChartRefreshed();
        chartView->fitToData();
    }
}

void OptiLab::axisYChanged(int index)
{
    if (index != -1)
    {
        doResultChanged();
        doChartRefreshed();
        chartView->fitToData();
    }
}

void OptiLab::doZoomIn()
{
    chartView->chart()->zoomIn();
}

void OptiLab::doZoomOut()
{
    chartView->chart()->zoomOut();
}

void OptiLab::doZoomBestFit()
{
    chartView->fitToData();
}

int OptiLab::findPointIndex(const QPointF &point)
{
    double tolX = (axisX->max()-axisX->min()) / 100.0;
    double tolY = (axisY->max()-axisY->min()) / 100.0;

    int index = -1;
    for (int i = 0; i < valueSeries->points().count(); i++)
    {
        QPointF p = valueSeries->points().at(i);
        if ((abs(p.x() - point.x()) < tolX) && (abs(p.y() - point.y()) < tolY))
        {
            index = i;
            break;
        }
    }

    return index;
}

void OptiLab::exportData()
{
    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data"), dir, tr("CSV files (*.csv)"));
    if (fileName.isEmpty())
    {
        cerr << "Incorrect file name." << endl;
        return;
    }

    QFileInfo fileInfo(fileName);

    // open file for write
    if (fileInfo.suffix().isEmpty())
        fileName = fileName + ".csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cerr << "Could not create " + fileName.toStdString() + " file." << endl;
        return;
    }

    settings.setValue("General/LastDataDir", fileInfo.absolutePath());

    QTextStream out(&file);

    auto *study = cmbStudies->itemData(cmbStudies->currentIndex()).value<Study *>();
    QList<ComputationSet> computationSets = study->computationSets();

    // headers
    for (int i = 0; i < computationSets.size(); i++)
    {
        foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
        {
            QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
            foreach (Parameter parameter, study->parameters())
            {
                out << parameter.name() + ";";
            }

            StringToDoubleMap results = computation->results()->items();
            foreach (QString key, results.keys())
            {
                out << key + ";";
            }

            out << "\n";

            break;
        }
        break;
    }

    // values
    for (int i = 0; i < computationSets.size(); i++)
    {
        foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
        {
            QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
            foreach (Parameter parameter, study->parameters())
            {
                out << QString::number(parameters[parameter.name()].value()) + ";";
            }

            StringToDoubleMap results = computation->results()->items();
            foreach (QString key, results.keys())
            {
                out << QString::number(results[key]) + ";";
            }

            out << "\n";
        }
    }
}
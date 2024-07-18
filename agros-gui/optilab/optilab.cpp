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
#include "optilab_widget.h"
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

OptiLab::OptiLab(QWidget *parent) : QWidget(parent), m_selectedStudy(nullptr)
{
    actSceneModeOptiLab = new QAction(icon("main_optilab"), tr("OptiLab"), this);
    actSceneModeOptiLab->setShortcut(Qt::Key_F8);
    actSceneModeOptiLab->setCheckable(true);

    m_optiLabWidget = new OptiLabWidget(this);

    createControls();

    connect(m_optiLabWidget, SIGNAL(studySelected(Study *)), this, SLOT(doStudySelected(Study *)));
}

OptiLab::~OptiLab()
{
    QSettings settings;
    settings.setValue("OptiLab/ResultsTreeColumnWidth0", trvResults->columnWidth(0));
    settings.setValue("OptiLab/ResultsTreeColumnWidth1", trvResults->columnWidth(1));
}

QWidget *OptiLab::createControlsGeometryAndStats()
{
    lblResultMin = new QLabel();
    lblResultMin->setMinimumWidth(90);
    lblResultMax = new QLabel();
    lblResultMean = new QLabel();
    lblResultMedian = new QLabel();
    lblResultVariance = new QLabel();
    lblResultStdDev = new QLabel();
    // lblResultNormalCovariance = new QLabel(); // normal distribution
    // lblResultNormalCorrelation = new QLabel(); // normal distribution

    auto *layoutStatistics = new QGridLayout();
    layoutStatistics->addWidget(new QLabel(tr("Minimum:")), 0, 0);
    layoutStatistics->addWidget(lblResultMin, 0, 1);
    layoutStatistics->addWidget(new QLabel(tr("Maximum:")), 1, 0);
    layoutStatistics->addWidget(lblResultMax, 1, 1);
    layoutStatistics->addWidget(new QLabel(tr("Mean value:")), 3, 0);
    layoutStatistics->addWidget(lblResultMean, 3, 1);
    layoutStatistics->addWidget(new QLabel(tr("Median:")), 4, 0);
    layoutStatistics->addWidget(lblResultMedian, 4, 1);
    layoutStatistics->addWidget(new QLabel(tr("Variance:")), 5, 0);
    layoutStatistics->addWidget(lblResultVariance, 5, 1);
    layoutStatistics->addWidget(new QLabel(tr("Std. deviation:")), 6, 0);
    layoutStatistics->addWidget(lblResultStdDev, 6, 1);
    layoutStatistics->setRowStretch(10, 1);
    // layoutStatistics->addWidget(new QLabel(tr("Covariance:")), 21, 0);
    // layoutStatistics->addWidget(lblResultNormalCovariance, 21, 1);
    // layoutStatistics->addWidget(new QLabel(tr("Correlation:")), 22, 0);
    // layoutStatistics->addWidget(lblResultNormalCorrelation, 22, 1);

    auto *widStatistics = new QWidget();
    // widDist->setContentsMargins(0, 0, 0, 0);
    widStatistics->setLayout(layoutStatistics);

    resultsStatChart = new QChart();
    resultsStatChart->legend()->setVisible(true);
    resultsStatChart->legend()->setInteractive(true);
    resultsStatChart->legend()->setAlignment(Qt::AlignBottom);
    resultsStatChart->legend()->attachToChart();

    axisStatX = new QValueAxis;
    axisStatX->setLabelFormat("%g");
    axisStatX->setGridLineVisible(false);
    axisStatX->setLabelsVisible(false);
    resultsStatChart->addAxis(axisStatX, Qt::AlignBottom);

    // axis y
    axisStatY = new QValueAxis;
    axisStatY->setLabelFormat("%g");
    axisStatY->setGridLineVisible(true);
    // axisY->setTitleText(tr("objective"));
    resultsStatChart->addAxis(axisStatY, Qt::AlignLeft);

    resultsStatMinMaxSeries = createSeries("Min and max");
    resultsStatMeanSeries = createSeries("Mean");
    resultsStatMedianSeries = createSeries("Median");

    geometryViewer = new SceneViewSimpleGeometry(this);

    auto *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(resultsStatChart);

    auto *tabStats = new QTabWidget();
    tabStats->setMinimumHeight(250);
    tabStats->setMaximumHeight(250);
    tabStats->addTab(geometryViewer, tr("Geometry"));
    tabStats->addTab(chartView, tr("Statistics"));
    tabStats->addTab(widStatistics, tr("Values"));

    return tabStats;
}

QScatterSeries *OptiLab::createSeries(const QString &name)
{
    auto series = new QScatterSeries();
    series->setName(name);
    series->setMarkerSize(12.0);
    resultsStatChart->addSeries(series);
    series->attachAxis(axisStatX);
    series->attachAxis(axisStatY);

    return series;
}

QWidget *OptiLab::createControlsChart()
{
    actChartRescale = new QAction(tr("Rescale chart"), this);
    connect(actChartRescale, SIGNAL(triggered(bool)), this, SLOT(chartRescale(bool)));

    actChartLogHorizontal = new QAction(tr("Logarithmic scale (x-axis)"), this);
    actChartLogHorizontal->setCheckable(true);
    connect(actChartLogHorizontal, SIGNAL(triggered(bool)), this, SLOT(chartLogHorizontal(bool)));

    actChartLogVertical = new QAction(tr("Logarithmic scale (y-axis)"), this);
    actChartLogVertical->setCheckable(true);
    connect(actChartLogVertical, SIGNAL(triggered(bool)), this, SLOT(chartLogVertical(bool)));

    actChartShowTrend = new QAction(tr("Show trend line"), this);
    actChartShowTrend->setCheckable(true);
    connect(actChartShowTrend, SIGNAL(triggered(bool)), this, SLOT(chartShowTrend(bool)));

    actChartShowAverageValue = new QAction(tr("Show average value"), this);
    actChartShowAverageValue->setCheckable(true);
    connect(actChartShowAverageValue, SIGNAL(triggered(bool)), this, SLOT(chartShowAverageValue(bool)));

    actChartParetoFront = new QAction(tr("Show Pareto front"), this);
    actChartParetoFront->setCheckable(true);
    connect(actChartParetoFront, SIGNAL(triggered(bool)), this, SLOT(chartShowParetoFront(bool)));

    mnuChart = new QMenu(this);
    mnuChart->addAction(actChartRescale);
    mnuChart->addSection(tr("Chart properties"));
    mnuChart->addAction(actChartLogHorizontal);
    mnuChart->addAction(actChartLogVertical);
    mnuChart->addAction(actChartShowTrend);
    mnuChart->addAction(actChartShowAverageValue);
    mnuChart->addAction(actChartParetoFront);

    auto chart = new QChart();
    chart->legend()->hide();
    // connect(chart, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(chartContextMenu(const QPoint &)));
    // connect(chart, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, int, QMouseEvent*)));

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

    valueSeries = new QScatterSeries();
    valueSeries->setUseOpenGL(true);
    chart->addSeries(valueSeries);
    valueSeries->setMarkerSize(10.0);
    valueSeries->setSelectedColor(QColor(60, 180, 180));
    valueSeries->attachAxis(axisX);
    valueSeries->attachAxis(axisY);
    QObject::connect(valueSeries, &QScatterSeries::clicked, this, &OptiLab::chartClicked);

    // chart trend line
    trendLineSeries = new QLineSeries();
    trendLineSeries->setUseOpenGL(true);
    // trendLineSeries->setPen(QPen(QBrush(QColor(60, 75, 90)), 2, Qt::DashLine));
    trendLineSeries->setColor(QColor(60, 75, 90));
    chart->addSeries(trendLineSeries);
    trendLineSeries->attachAxis(axisX);
    trendLineSeries->attachAxis(axisY);

    // mean value
    averageValueSeries = new QLineSeries();
    averageValueSeries->setUseOpenGL(true);
    averageValueSeries->setPen(QPen(QBrush(QColor(140, 140, 140)), 2, Qt::DotLine));
    chart->addSeries(averageValueSeries);
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
    averageValueAreaSeries->setOpacity(0.1);
    averageValueAreaSeries->attachAxis(axisX);
    averageValueAreaSeries->attachAxis(axisY);

    // // Pareto front
    paretoFrontSeries = new QLineSeries();
    paretoFrontSeries->setUseOpenGL(true);
    chart->addSeries(paretoFrontSeries);
    paretoFrontSeries->attachAxis(axisX);
    paretoFrontSeries->attachAxis(axisY);
    // chartGraphParetoFront = chart->addGraph();
    // // chartGraphParetoFront->setName(tr("Pareto front"));
    // // chartGraphParetoFront->addToLegend();
    // chartGraphParetoFront->removeFromLegend();
    // chartGraphParetoFront->setPen(QPen(QBrush(QColor(150, 110, 110, 120)), 8, Qt::SolidLine));
    // chartGraphParetoFront->setLineStyle(QCPGraph::lsLine);

    chartView = new ChartView(chart, false);
    chartView->setChart(chart);

    return chartView;
}

QWidget *OptiLab::createControlsResults()
{
    QSettings settings;

    // computations
    cmbComputations = new QComboBox(this);
    connect(cmbComputations, SIGNAL(currentIndexChanged(int)), this, SLOT(doComputationChanged(int)));

    actComputationSolve = new QAction(tr("Solve problem"), this);
    connect(actComputationSolve, SIGNAL(triggered(bool)), this, SLOT(doComputationSolve(bool)));

    // treeview
    trvResults = new QTreeWidget(this);
    trvResults->setExpandsOnDoubleClick(false);
    // trvResults->setHeaderHidden(false);
    trvResults->setHeaderLabels(QStringList() << tr("Name") << tr("Value"));
    // trvResults->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trvResults->setContextMenuPolicy(Qt::CustomContextMenu);
    trvResults->setMouseTracking(true);
    trvResults->setUniformRowHeights(true);
    trvResults->setColumnCount(2);
    trvResults->setColumnWidth(0, settings.value("OptiLab/ResultsTreeColumnWidth0", 80).toInt());
    trvResults->setColumnWidth(1, settings.value("OptiLab/ResultsTreeColumnWidth1", 180).toInt());
    trvResults->setIndentation(trvResults->indentation() - 2);
    connect(trvResults, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(resultsContextMenu(const QPoint &)));
    connect(trvResults, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doResultChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    actResultsDependenceOnSteps = new QAction(tr("Dependence on the number of steps"), this);
    connect(actResultsDependenceOnSteps, SIGNAL(triggered(bool)), this, SLOT(resultsDependenceOnSteps(bool)));

    actResultsSetHorizontal = new QAction(tr("Set on horizontal axis"), this);
    connect(actResultsSetHorizontal, SIGNAL(triggered(bool)), this, SLOT(resultsSetHorizontal(bool)));

    actResultsSetVertical = new QAction(tr("Set on vertical axis"), this);
    connect(actResultsSetVertical, SIGNAL(triggered(bool)), this, SLOT(resultsSetVertical(bool)));

    actResultsFindMinimum = new QAction(tr("Find minimum"), this);
    connect(actResultsFindMinimum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMinimum(bool)));
    actResultsFindMaximum = new QAction(tr("Find maximum"), this);
    connect(actResultsFindMaximum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMaximum(bool)));

    mnuResults = new QMenu(trvResults);
    mnuResults->addAction(actResultsDependenceOnSteps);
    mnuResults->addAction(actResultsSetHorizontal);
    mnuResults->addAction(actResultsSetVertical);
    mnuResults->addSection(tr("Statistics"));
    mnuResults->addAction(actResultsFindMinimum);
    mnuResults->addAction(actResultsFindMaximum);

    auto *layoutResults = new QVBoxLayout();
    layoutResults->setContentsMargins(0, 0, 0, 0);
    layoutResults->addWidget(cmbComputations);
    layoutResults->addWidget(trvResults, 2);

    auto *widResults = new QWidget();
    widResults->setContentsMargins(0, 0, 0, 0);
    widResults->setLayout(layoutResults);

    return widResults;
}

void OptiLab::createControls()
{
    auto *layoutCharts = new QVBoxLayout();
    layoutCharts->addWidget(createControlsChart(), 2);

    auto *widCharts = new QWidget();
    widCharts->setContentsMargins(0, 0, 0, 0);
    widCharts->setLayout(layoutCharts);

    auto layoutResults = new QVBoxLayout();
    layoutResults->addWidget(createControlsResults(), 2);
    layoutResults->addWidget(createControlsGeometryAndStats(), 0);

    auto *widResults = new QWidget();
    widResults->setContentsMargins(0, 0, 0, 0);
    widResults->setMaximumWidth(300);
    widResults->setLayout(layoutResults);

    auto layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(m_optiLabWidget);
    layoutMain->addWidget(widResults);
    layoutMain->addWidget(widCharts);
    layoutMain->setStretch(2, 1);

    setLayout(layoutMain);
}

void OptiLab::refresh()
{
    optiLabWidget()->refresh();
}

void OptiLab::chartContextMenu(const QPoint &pos)
{
    mnuChart->exec(QCursor::pos());
}

void OptiLab::chartRescale(bool checked)
{
    chartView->fitToData();
}

void OptiLab::chartLogHorizontal(bool checked)
{
    // force rescale
    bool rescale = (m_selectedStudy->value(Study::View_ChartLogHorizontal).toBool() != checked);

    m_selectedStudy->setValue(Study::View_ChartLogHorizontal, checked);
    actChartLogHorizontal->setChecked(checked);

    // if (checked)
    //     chart->xAxis->setScaleType(QCPAxis::stLogarithmic);
    // else
    //     chart->xAxis->setScaleType(QCPAxis::stLinear);

    if (rescale) chartRescale(true);
}

void OptiLab::chartLogVertical(bool checked)
{
    // force rescale
    bool rescale = (m_selectedStudy->value(Study::View_ChartLogVertical).toBool() != checked);

    m_selectedStudy->setValue(Study::View_ChartLogVertical, checked);
    actChartLogVertical->setChecked(checked);

    // if (checked)
    //     chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    // else
    //     chart->yAxis->setScaleType(QCPAxis::stLinear);

    if (rescale) chartRescale(true);
}

void OptiLab::chartShowTrend(bool checked)
{
    m_selectedStudy->setValue(Study::View_ChartShowTrend, checked);
    actChartShowTrend->setChecked(checked);

    // chartTrendLine->setVisible(checked);
    // chart->replot(QxCustomPlot::rpQueuedRefresh);
}

void OptiLab::chartShowAverageValue(bool checked)
{
    m_selectedStudy->setValue(Study::View_ChartShowAverageValue, checked);
    actChartShowAverageValue->setChecked(checked);

    // chartGraphAverageValue->setVisible(checked);
    // chartGraphAverageValueChannelLower->setVisible(checked);
    // chartGraphAverageValueChannelUpper->setVisible(checked);
    //
    // chart->replot(QxCustomPlot::rpQueuedRefresh);
}

void OptiLab::chartShowParetoFront(bool checked)
{
    m_selectedStudy->setValue(Study::View_ChartShowParetoFront, checked);
    actChartParetoFront->setChecked(checked);

    // chartGraphParetoFront->setVisible(checked);
    // chart->replot(QxCustomPlot::rpQueuedRefresh);
}

void OptiLab::resultsDependenceOnSteps(bool checked)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        m_selectedStudy->setValue(Study::View_ChartHorizontal, QString("step:"));
        m_selectedStudy->setValue(Study::View_ChartVertical, QString("%1:%2").
                          arg(m_selectedStudy->resultTypeToStringKey((Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt())).
                          arg(trvResults->currentItem()->data(0, Qt::UserRole).toString()));

        doChartRefreshed();
    }
}

void OptiLab::resultsSetHorizontal(bool checked)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        m_selectedStudy->setValue(Study::View_ChartHorizontal, QString("%1:%2").
                          arg(m_selectedStudy->resultTypeToStringKey((Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt())).
                          arg(trvResults->currentItem()->data(0, Qt::UserRole).toString()));

        doChartRefreshed();
    }
}

void OptiLab::resultsSetVertical(bool checked)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        m_selectedStudy->setValue(Study::View_ChartVertical, QString("%1:%2").
                          arg(m_selectedStudy->resultTypeToStringKey((Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt())).
                          arg(trvResults->currentItem()->data(0, Qt::UserRole).toString()));

        doChartRefreshed();
    }
}

void OptiLab::resultsContextMenu(const QPoint &pos)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        mnuResults->exec(QCursor::pos());
    }
}

void OptiLab::doStudySelected(Study *study)
{
    m_selectedStudy = study;

    doChartRefreshed();
    geometryViewer->doZoomBestFit();
    cmbComputations->blockSignals(true);

    // computations
    QString selectedComputationProblemDir = "";
    if (cmbComputations->currentIndex() >= 0)
        selectedComputationProblemDir = cmbComputations->currentData().toString();
    cmbComputations->clear();

    // study
    if (m_selectedStudy)
    {
        double min = selectedComputationProblemDir.isEmpty() ? numeric_limits<double>::max() : 0.0;

        // fill tree view
        QList<ComputationSet> computationSets = m_selectedStudy->computationSets(m_selectedStudy->value(Study::View_Filter).toString());
        for (int i = 0; i < computationSets.size(); i++)
        {
            int currentComputationSetCount = 0;
            foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
            {
                QString item = QString("%1 - %2 / %3 - %4 / %5")
                    .arg(computationSets[i].name().at(0))
                    .arg(i+1)
                    .arg(currentComputationSetCount+1)
                    .arg(computation->isSolved() ? tr("solved") : tr("not solved"))
                    .arg(computation->results()->hasResults() ? tr("results") : tr("no results"));

                cmbComputations->addItem(item, computation->problemDir());

                currentComputationSetCount++;

                // select minimum
                double localMin = m_selectedStudy->evaluateSingleGoal(computation);
                if (localMin < min)
                {
                    min = localMin;
                    selectedComputationProblemDir = computation->problemDir();
                }
            }
        }
    }

    cmbComputations->blockSignals(false);

    // select current computation
    if (cmbComputations->count() > 0 && !selectedComputationProblemDir.isEmpty())
    {
        doComputationSelected(selectedComputationProblemDir);
    }
}

void OptiLab::doChartRefreshed(const QString &problemDir)
{
    m_computationMap.clear();
    valueSeries->clear();
    averageValueSeries->clear();
    averageValueLowerSeries->clear();
    averageValueUpperSeries->clear();

    if (!m_selectedStudy)
    {
        // fit and replot chart
        chartView->fitToData();

        return;
    }

    // block signals
    valueSeries->blockSignals(true);

    QList<ComputationSet> computationSets = m_selectedStudy->computationSets(m_selectedStudy->value(Study::View_Filter).toString());

    QString chartX = m_selectedStudy->value(Study::View_ChartHorizontal).toString();
    QString chartY = m_selectedStudy->value(Study::View_ChartVertical).toString();

    if (chartX.isEmpty() || chartY.isEmpty())
        return;

    QString labelX = "-";
    QString labelY = "-";

    if (chartX.contains("step:"))
        labelX = "step";
    else if (chartX.contains("parameter:"))
        labelX = tr("%1 (parameter)").arg(chartX.right(chartX.count() - 10));
    else if (chartX.contains("functional:"))
        labelX = tr("%1 (functional)").arg(chartX.right(chartX.count() - 11));
    else if (chartX.contains("recipe:"))
        labelX = tr("%1 (recipe)").arg(chartX.right(chartX.count() - 7));
    else if (chartX.contains("other:"))
        labelX = tr("%1 (other)").arg(chartX.right(chartX.count() - 6));
    else
        assert(0);

    if (chartY.contains("step:"))
        labelY = "step";
    else if (chartY.contains("parameter:"))
        labelY = tr("%1 (parameter)").arg(chartY.right(chartY.count() - 10));
    else if (chartY.contains("functional:"))
        labelY = tr("%1 (functional)").arg(chartY.right(chartY.count() - 11));
    else if (chartY.contains("recipe:"))
        labelY = tr("%1 (recipe)").arg(chartY.right(chartY.count() - 7));
    else if (chartY.contains("other:"))
        labelY = tr("%1 (other)").arg(chartY.right(chartY.count() - 6));
    else
        assert(0);

    // linear regression
    QVector<double> linRegDataX;
    QVector<double> linRegDataY;

    // Pareto front
    QList<QSharedPointer<Computation> > paretoFront = m_selectedStudy->nondominatedSort(computationSets);
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
            if (chartX.contains("step:"))
            {
                dataSetX.append(step);
            }
            if (chartY.contains("step:"))
            {
                dataSetY.append(step);
            }

            // parameters
            if (chartX.contains("parameter:"))
            {
                QString name = chartX.right(chartX.count() - 10);
                double value = computation->config()->parameters()->number(name);
                dataSetX.append(value);
            }
            if (chartY.contains("parameter:"))
            {
                QString name = chartY.right(chartY.count() - 10);
                double value = computation->config()->parameters()->number(name);
                dataSetY.append(value);
            }

            // functionals
            if (chartX.contains("functional:"))
            {
                QString name = chartX.right(chartX.count() - 11);
                double value = computation->results()->value(name);
                dataSetX.append(value);
            }
            if (chartY.contains("functional:"))
            {
                QString name = chartY.right(chartY.count() - 11);
                double value = computation->results()->value(name);
                dataSetY.append(value);
            }

            // recipes
            if (chartX.contains("recipe:"))
            {
                QString name = chartX.right(chartX.count() - 7);
                double value = computation->results()->value(name);
                dataSetX.append(value);
            }
            if (chartY.contains("recipe:"))
            {
                QString name = chartY.right(chartY.count() - 7);
                double value = computation->results()->value(name);
                dataSetY.append(value);
            }

            // other
            if (chartX.contains("other:"))
            {
                QString name = chartX.right(chartX.count() - 6);
                double value = computation->results()->value(name);
                dataSetX.append(value);
            }
            if (chartY.contains("other:"))
            {
                QString name = chartY.right(chartY.count() - 6);
                double value = computation->results()->value(name);
                dataSetY.append(value);
            }

            // selected point
            if (computation->problemDir() == problemDir)
            {
                QVector<double> selectedX; selectedX << dataSetX.last();
                QVector<double> selectedY; selectedY << dataSetY.last();

                // chartGraphSelectedComputation->setData(selectedX, selectedY);
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
        // int colorBrushIndex = abs(goal - minGoal) / (maxGoal - minGoal) * 255;
        // const QColor colorBrush(paletteDataParuly[colorBrushIndexp][0] * 255, paletteDataParuly[colorBrushIndex][1] * 255, paletteDataParuly[colorBrushIndex][2] * 255);

        // add data to global array (for min and max computation)
        linRegDataX << dataSetX;
        linRegDataY << dataSetY;

        // chartGraphChart->setData(dataSetX, dataSetY);
        // chartGraphChart->setProperty("computationset_index", i);
        // chartGraphChart->setName(computationSets[i].name());
        // chartGraphChart->addToLegend();
        // chartGraphChart->setLineStyle(QCPGraph::lsNone);
        // chartGraphChart->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(140, 140, 140), colorFill, 9));

        // add data
        for (int i = 0; i < dataSetX.size(); i++)
            valueSeries->append(dataSetX[i], dataSetY[i]);
    }

    // Pareto front
    // chartGraphParetoFront->setData(paretoDataX, paretoDataY);

    // mean value and standard deviation
    Statistics statsX(linRegDataX);
    Statistics statsY(linRegDataY);

    // bounding box
    double delta = qMin((statsX.max() - statsX.min()) * 0.1, (statsY.max() - statsY.min()) * 0.1);

    averageValueSeries->append(statsX.min() - delta, statsY.mean());
    averageValueSeries->append(statsX.max() + delta, statsY.mean());
    averageValueLowerSeries->append(statsX.min() - delta, statsY.mean() - statsY.stdDev());
    averageValueLowerSeries->append(statsX.max() + delta, statsY.mean() - statsY.stdDev());
    averageValueUpperSeries->append(statsX.min() - delta, statsY.mean() + statsY.stdDev());
    averageValueUpperSeries->append(statsX.max() + delta, statsY.mean() + statsY.stdDev());

    // linear regression
    StatisticsLinReg linReg(linRegDataX, linRegDataY);
    if (linReg.computeLinReg())
    {
        // trend arrow
        QPointF start((statsX.min() - delta), (statsX.min() - delta) * linReg.slope() + linReg.shift());
        QPointF end((statsX.max() + delta), (statsX.max() + delta) * linReg.slope() + linReg.shift());

        trendLineSeries->append(start.x(), start.y());
        trendLineSeries->append(end.x(), end.y());
        // chartTrendLine->setHead(QCPLineEnding::esSpikeArrow);
        // chartTrendLine->setPen(QPen(QBrush(QColor(200, 200, 200)), 5));
    }

    // unblock signals
    valueSeries->blockSignals(false);

    // replot chart
    chartView->fitToData();

    // set actions
    chartLogHorizontal(m_selectedStudy->value(Study::View_ChartLogHorizontal).toBool());
    chartLogHorizontal(m_selectedStudy->value(Study::View_ChartLogVertical).toBool());
    chartShowTrend(m_selectedStudy->value(Study::View_ChartShowTrend).toBool());
    chartShowAverageValue(m_selectedStudy->value(Study::View_ChartShowAverageValue).toBool());
    chartShowParetoFront(m_selectedStudy->value(Study::View_ChartShowParetoFront).toBool());
}

// QPair<double, double> OptiLab::findClosestData(QCPGraph *graph, const Point &pos)
// {
//     // find min and max
//     RectPoint bound;
//     bound.start.x = numeric_limits<double>::max();
//     bound.end.x = -numeric_limits<double>::max();
//     bound.start.y = numeric_limits<double>::max();
//     bound.end.y = -numeric_limits<double>::max();
//
//     for (QCPGraphDataContainer::const_iterator it = graph->data()->begin(); it != graph->data()->end(); ++it)
//     {
//         if (it->key < bound.start.x) bound.start.x = it->key;
//         if (it->key > bound.end.x) bound.end.x = it->key;
//         if (it->value < bound.start.y) bound.start.y = it->value;
//         if (it->value > bound.end.y) bound.end.y = it->value;
//     }
//
//     // find closest point
//     QPair<double, double> selectedData;
//     double dist = numeric_limits<double>::max();
//     for (QCPGraphDataContainer::const_iterator it = graph->data()->begin(); it != graph->data()->end(); ++it)
//     {
//         double mag = Point((it->key - pos.x) / bound.width(),
//                            (it->value - pos.y) / bound.height()).magnitudeSquared();
//
//         if (mag <= dist)
//         {
//             dist = mag;
//             selectedData.first = it->key;
//             selectedData.second = it->value;
//         }
//     }
//
//     return selectedData;
// }

// void OptiLab::graphClicked(QCPAbstractPlottable *plottable, int code, QMouseEvent *event)
// {
//     if (QCPGraph *graph = dynamic_cast<QCPGraph *>(plottable))
//     {
//         // find closest point
//         QPair<double, double> selectedData = findClosestData(graph, Point(chart->xAxis->pixelToCoord(event->pos().x()),
//                                                                           chart->yAxis->pixelToCoord(event->pos().y())));
//
//         QSharedPointer<Computation> selectedComputation = m_computationMap[graph->property("computationset_index").toInt()][selectedData];
//         if (!selectedComputation.isNull())
//         {
//             QToolTip::hideText();
//             QToolTip::showText(event->globalPos(),
//                                tr("<table>"
//                                   "<tr><th colspan=\"2\">%L1</th></tr>"
//                                   "<tr><td>X:</td>" "<td>%L2</td></tr>"
//                                   "<tr><td>Y:</td>" "<td>%L3</td></tr>"
//                                   "</table>").
//                                arg(graph->name().isEmpty() ? "..." : graph->name()).arg(selectedData.first).arg(selectedData.second), chart, chart->rect());
//
//             emit computationSelected(selectedComputation->problemDir());
//         }
//     }
// }

void OptiLab::chartClicked(const QPointF &point)
{
    int index = valueSeries->points().indexOf(point);
    qInfo() << index;
}

void OptiLab::doComputationSelected(const QString &problemDir)
{
    for (int i = 0; i < cmbComputations->count(); i++)
    {
        QString dir = cmbComputations->itemData(i).toString();
        if (dir == problemDir)
        {
            cmbComputations->setCurrentIndex(i);
            break;
        }
    }
}

void OptiLab::doComputationChanged(int index)
{
    if (cmbComputations->count() == 0)
    {
        geometryViewer->setProblem(QSharedPointer<ProblemBase>(nullptr));
        trvResults->clear();
        return;
    }

    qInfo() << "doComputationChanged " << index;
    if (!m_selectedStudy)
    {
        geometryViewer->setProblem(QSharedPointer<ProblemBase>(nullptr));
        trvResults->clear();
        return;
    }

    QString problemDir = cmbComputations->currentData().toString();

    // cache value
    Study::ResultType selectedType = Study::ResultType_Parameter;
    QString selectedKey = "";
    QTreeWidgetItem *selectedItem = nullptr;
    if (!problemDir.isEmpty() && trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        selectedType = (Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt();
        selectedKey = trvResults->currentItem()->data(0, Qt::UserRole).toString();
    }

    trvResults->clear();

    QMap<QString, QSharedPointer<Computation> > computations = Agros::computations();
    if (computations.count() > 0)
    {
        QSharedPointer<Computation> computation = computations[problemDir];

        geometryViewer->setProblem(static_cast<QSharedPointer<ProblemBase> >(computation));

        // fill treeview
        QFont fnt = trvResults->font();
        fnt.setBold(true);

        // parameters
        QTreeWidgetItem *parametersNode = new QTreeWidgetItem(trvResults);
        parametersNode->setText(0, tr("Parameters"));
        parametersNode->setFont(0, fnt);
        parametersNode->setIcon(0, iconAlphabet('P', AlphabetColor_Brown));
        parametersNode->setExpanded(true);

        QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
        foreach (Parameter parameter, m_selectedStudy->parameters())
        {
            QTreeWidgetItem *parameterNode = new QTreeWidgetItem(parametersNode);
            parameterNode->setText(0, parameter.name());
            parameterNode->setText(1, QString::number(parameters[parameter.name()].value()));
            parameterNode->setData(0, Qt::UserRole, parameter.name());
            parameterNode->setData(1, Qt::UserRole, Study::ResultType::ResultType_Parameter);

            if (selectedType == Study::ResultType::ResultType_Parameter && selectedKey == parameter.name())
                selectedItem = parameterNode;
        }

        // functionals
        QTreeWidgetItem *functionalsNode = new QTreeWidgetItem(trvResults);
        functionalsNode->setText(0, tr("Goal Functions"));
        functionalsNode->setFont(0, fnt);
        functionalsNode->setIcon(0, iconAlphabet('F', AlphabetColor_Blue));
        functionalsNode->setExpanded(true);

        // recipes
        QTreeWidgetItem *recipesNode = new QTreeWidgetItem(trvResults);
        recipesNode->setText(0, tr("Recipes"));
        recipesNode->setFont(0, fnt);
        recipesNode->setIcon(0, iconAlphabet('R', AlphabetColor_Green));
        recipesNode->setExpanded(true);

        // other
        QTreeWidgetItem *otherNode = new QTreeWidgetItem(trvResults);
        otherNode->setText(0, tr("Other"));
        otherNode->setFont(0, fnt);
        otherNode->setIcon(0, iconAlphabet('O', AlphabetColor_Purple));
        otherNode->setExpanded(true);

        StringToDoubleMap results = computation->results()->items();
        foreach (QString key, results.keys())
        {
            QTreeWidgetItem *item = nullptr;
            if (computation->results()->type(key) == ComputationResultType_Functional)
            {
                item = new QTreeWidgetItem(functionalsNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Functional);

                if (selectedType == Study::ResultType::ResultType_Functional && selectedKey == key)
                    selectedItem = item;
            }
            else if (computation->results()->type(key) == ComputationResultType_Recipe)
            {
                item = new QTreeWidgetItem(recipesNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Recipe);

                if (selectedType == Study::ResultType::ResultType_Recipe && selectedKey == key)
                    selectedItem = item;
            }
            else if (computation->results()->type(key) == ComputationResultType_Other)
            {
                item = new QTreeWidgetItem(otherNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Other);

                if (selectedType == Study::ResultType::ResultType_Other && selectedKey == key)
                    selectedItem = item;
            }
            else
                assert(0);

            item->setText(0, key);
            item->setText(1, QString::number(results[key]));
            item->setData(0, Qt::UserRole, key);
        }

        // paint chart
        doChartRefreshed(problemDir);

        // select item
        if (selectedItem)
            trvResults->setCurrentItem(selectedItem);
    }
}

void OptiLab::doComputationSolve(bool ok)
{
    if (cmbComputations->count() > 0)
    {
        QString key = cmbComputations->itemData(cmbComputations->currentIndex()).toString();
        if (!key.isEmpty() && Agros::computations().contains(key))
        {
            SolveThread *solveThread = new SolveThread(Agros::computations()[key].data());
            solveThread->startCalculation();
        }
    }
}

void OptiLab::doResultChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    bool showStats = trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid();

    lblResultMin->setText("-");
    lblResultMax->setText("-");
    lblResultMean->setText("-");
    lblResultMedian->setText("-");
    lblResultVariance->setText("-");
    lblResultStdDev->setText("-");
    // lblResultNormalCovariance->setText("-");
    // lblResultNormalCorrelation->setText("-");

    if (m_selectedStudy && showStats)
    {
        QList<ComputationSet> computationSets = m_selectedStudy->computationSets(m_selectedStudy->value(Study::View_Filter).toString());

        Study::ResultType type = (Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt();
        QString key = trvResults->currentItem()->data(0, Qt::UserRole).toString();

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
                else if (type == Study::ResultType_Recipe || type == Study::ResultType_Functional || type == Study::ResultType_Other)
                    data.append(computation->results()->value(key));

                step.append(step.count());
            }
        }

        if (data.size() > 0)
        {
            Statistics stats(data);
            if (stats.stdDev() > 0.0)
            {
                boost::math::normal_distribution<double> normalDistribution(stats.mean(), stats.stdDev());

                // data distribution
                int dataCount = 15;
                double dataStep = (stats.max() - stats.min()) / (dataCount - 1);
                QVector<double> dataPDF(dataCount);
                QVector<double> dataCDF(dataCount);
                QVector<double> normalCDFCorrelation(dataCount);
                QVector<double> dataSteps(dataCount);

                if ((stats.max() - stats.min()) > EPS_ZERO)
                {
                    for (int i = 0; i < dataCount; i++)
                        dataSteps[i] = stats.min() + dataStep * i;

                    // construct probability density function (PDF)
                    foreach (double val, data)
                    {
                        int bin = round(((dataCount - 1) * ((val - stats.min()) / (stats.max() - stats.min()))));
                        dataPDF[bin] += 1;
                    }

                    // construct cumulative distribution function (CDF)
                    for (int i = 0; i < dataCount; i++)
                        for (int j = 0; j <= i; j++)
                            dataCDF[i] += dataPDF[j];

                    // correlation with normal distribution
                    for (int i = 0; i < dataCount; i++)
                        normalCDFCorrelation[i] = boost::math::cdf(normalDistribution, dataSteps[i]);
                }

                // labels
                lblResultMin->setText(QString::number(stats.min()));
                lblResultMax->setText(QString::number(stats.max()));
                lblResultMean->setText(QString::number(stats.mean()));
                lblResultMedian->setText(QString::number(stats.median()));
                lblResultVariance->setText(QString::number(stats.variance()));
                lblResultStdDev->setText(QString::number(stats.stdDev()));

                // correlation and covariance
                // StatisticsCorrelation statsCorrelation(dataCDF, normalCDFCorrelation);
                // lblResultNormalCovariance->setText(QString::number(statsCorrelation.covariance()));
                // lblResultNormalCorrelation->setText(QString::number(statsCorrelation.correlation()));

                // chart
                resultsStatMinMaxSeries->clear();
                resultsStatMinMaxSeries->append(0, stats.min());
                resultsStatMinMaxSeries->append(0, stats.max());
                resultsStatMeanSeries->clear();
                resultsStatMeanSeries->append(0, stats.mean());
                resultsStatMedianSeries->clear();
                resultsStatMedianSeries->append(0, stats.median());
                axisStatX->setRange(-1, 1);
                axisStatY->setRange(stats.min(), stats.max());
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
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        Study::ResultType type = (Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt();
        QString key = trvResults->currentItem()->data(0, Qt::UserRole).toString();

        // extreme
        QSharedPointer<Computation> selectedComputation = m_selectedStudy->findExtreme(type, key, minimum);

        if (!selectedComputation.isNull())
            doComputationSelected(selectedComputation->problemDir());
    }
}

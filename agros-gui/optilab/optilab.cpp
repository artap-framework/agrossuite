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

OptiLab::OptiLab(QWidget *parent) : QWidget(parent), m_selectedStudy(nullptr), m_selectedComputation(nullptr)
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

    groupBoxStatistics = new QGroupBox(tr("Statistics"));
    groupBoxStatistics->setLayout(layoutStatistics);
    groupBoxStatistics->setMaximumHeight(groupBoxStatistics->minimumSizeHint().height());

    geometryViewer = new SceneViewSimpleGeometry(this);

    auto *layoutGeometry = new QVBoxLayout();
    layoutGeometry->setContentsMargins(0, 0, 0, 0);
    layoutGeometry->addWidget(groupBoxStatistics);
    layoutGeometry->addWidget(geometryViewer);

    auto *widget = new QWidget();
    widget->setLayout(layoutGeometry);

    return widget;
}

QWidget *OptiLab::createControlsChart()
{
    actChartRescale = new QAction(tr("Rescale chart"), this);
    connect(actChartRescale, SIGNAL(triggered(bool)), this, SLOT(chartRescale(bool)));

    actChartParetoFront = new QAction(tr("Show Pareto front"), this);
    actChartParetoFront->setCheckable(true);
    connect(actChartParetoFront, SIGNAL(triggered(bool)), this, SLOT(chartShowParetoFront(bool)));

    mnuChart = new QMenu(this);
    mnuChart->addAction(actChartRescale);
    mnuChart->addSection(tr("Chart properties"));
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

    valueSelectedSeries = new QScatterSeries();
    valueSelectedSeries->setUseOpenGL(true);
    chart->addSeries(valueSelectedSeries);
    valueSelectedSeries->setPointsVisible(true);
    valueSelectedSeries->setMarkerSize(17.0);
    valueSelectedSeries->setColor(QColor(219, 13, 58));
    valueSelectedSeries->attachAxis(axisX);
    valueSelectedSeries->attachAxis(axisY);

    valueHoverSeries = new QScatterSeries();
    valueHoverSeries->setUseOpenGL(true);
    chart->addSeries(valueHoverSeries);
    valueHoverSeries->setPointsVisible(true);
    valueHoverSeries->setMarkerSize(15.0);
    valueHoverSeries->setColor(QColor(94, 3, 23));
    valueHoverSeries->attachAxis(axisX);
    valueHoverSeries->attachAxis(axisY);

    valueSeries = new QScatterSeries();
    valueSeries->setUseOpenGL(true);
    chart->addSeries(valueSeries);
    valueSeries->setPointsVisible(true);
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
    // QObject::connect(chartView, &QChartView::contextMenuEvent, this, &OptiLab::contextMenuEvent);

    return chartView;
}

QWidget *OptiLab::createControlsResults()
{
    // treeview
    trvResults = new QTreeWidget(this);
    trvResults->setExpandsOnDoubleClick(false);
    // trvResults->setHeaderHidden(false);
    trvResults->setHeaderLabels(QStringList() << tr("Name") << tr("Value"));
    trvResults->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trvResults->setContextMenuPolicy(Qt::CustomContextMenu);
    trvResults->setMouseTracking(true);
    trvResults->setUniformRowHeights(true);
    trvResults->setColumnCount(2);
    trvResults->setIndentation(trvResults->indentation() - 2);
    // trvResults->resizeColumnToContents(0);
    // trvResults->resizeColumnToContents(1);

    actComputationSolve = new QAction(icon("main_solve"), tr("Solve"), this);
    connect(actComputationSolve, SIGNAL(triggered(bool)), this, SLOT(doSolveCurrentComputation(bool)));

    actResultsFindMinimum = new QAction(icon("geometry_rectangle"), tr("Find minimum"), this);
    actResultsFindMinimum->setIconVisibleInMenu(true);
    connect(actResultsFindMinimum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMinimum(bool)));
    actResultsFindMaximum = new QAction(icon("geometry_circle"), tr("Find maximum"), this);
    actResultsFindMaximum->setIconVisibleInMenu(true);
    connect(actResultsFindMaximum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMaximum(bool)));

    // left toolbar
    auto toolBarComputation = new QToolBar();
    toolBarComputation->setOrientation(Qt::Horizontal);
    toolBarComputation->setProperty("modulebar", true);
    toolBarComputation->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolBarComputation->addAction(actComputationSolve);
    toolBarComputation->addSeparator();
    toolBarComputation->addAction(actResultsFindMinimum);
    toolBarComputation->addAction(actResultsFindMaximum);

    cmbAxisX = new QComboBox(this);
    connect(cmbAxisX, SIGNAL(currentIndexChanged(int)), this, SLOT(axisXChanged(int)));
    cmbAxisY = new QComboBox(this);
    connect(cmbAxisY, SIGNAL(currentIndexChanged(int)), this, SLOT(axisYChanged(int)));

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("Horizontal axis:"), cmbAxisX);
    formLayout->addRow(tr("Vertical axis and statistics:"), cmbAxisY);

    auto *layoutResults = new QVBoxLayout();
    layoutResults->setContentsMargins(0, 0, 0, 0);
    layoutResults->addLayout(formLayout);
    layoutResults->addWidget(trvResults, 2);
    layoutResults->addWidget(toolBarComputation);

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
    widResults->setMaximumWidth(370);
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

void OptiLab::chartShowParetoFront(bool checked)
{
    m_selectedStudy->setValue(Study::View_ChartShowParetoFront, checked);
    actChartParetoFront->setChecked(checked);

    // chartGraphParetoFront->setVisible(checked);
    // chart->replot(QxCustomPlot::rpQueuedRefresh);
}

void OptiLab::doStudySelected(Study *study)
{
    m_selectedStudy = study;

    // computations
    QString selectedComputationProblemDir = "";
    if (m_selectedComputation)
        selectedComputationProblemDir = m_selectedComputation->problemDir();
    m_selectedStudyProblemDirs.clear();

    // study
    if (m_selectedStudy)
    {
        double min = selectedComputationProblemDir.isEmpty() ? numeric_limits<double>::max() : 0.0;

        // fill tree view
        QList<ComputationSet> computationSets = m_selectedStudy->computationSets(m_selectedStudy->value(Study::View_Filter).toString());
        for (int i = 0; i < computationSets.size(); i++)
        {
            foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
            {
                m_selectedStudyProblemDirs.append(computation->problemDir());

                // select minimum
                double localMin = m_selectedStudy->evaluateSingleGoal(computation);
                if (localMin < min)
                {
                    min = localMin;
                    selectedComputationProblemDir = computation->problemDir();
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
        foreach (Parameter parameter, m_selectedStudy->parameters())
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
    }

    // actions
    actComputationSolve->setEnabled(false);
    actResultsFindMinimum->setEnabled(false);
    actResultsFindMaximum->setEnabled(false);

    cmbAxisX->setEnabled(m_selectedStudy != nullptr);
    cmbAxisY->setEnabled(m_selectedStudy != nullptr);
    trvResults->setEnabled(m_selectedStudy != nullptr);
    chartView->setEnabled(m_selectedStudy != nullptr);

    // select current computation
    if (m_selectedStudyProblemDirs.count() > 0 && !selectedComputationProblemDir.isEmpty())
    {
        doComputationSelected(selectedComputationProblemDir);

        // replot chart
        chartView->fitToData();
        geometryViewer->doZoomBestFit();
    }
}

void OptiLab::doChartRefreshed(bool fitToData)
{
    m_computationMap.clear();

    // series
    valueSeries->clear();
    valueSelectedSeries->clear();
    averageValueSeries->clear();
    averageValueLowerSeries->clear();
    averageValueUpperSeries->clear();
    trendLineSeries->clear();

    QList<ComputationSet> computationSets = m_selectedStudy->computationSets(m_selectedStudy->value(Study::View_Filter).toString());
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
    }

    // update chart
    if (fitToData)
        chartView->fitToData();

    // set actions
    chartShowParetoFront(m_selectedStudy->value(Study::View_ChartShowParetoFront).toBool());
}

void OptiLab::chartClicked(const QPointF &point)
{
    int index = findPointIndex(point);
    if (index != -1)
        doComputationSelected(m_selectedStudyProblemDirs[index]);
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

void OptiLab::doComputationSelected(const QString &problemDir)
{
    QMap<QString, QSharedPointer<Computation> > computations = Agros::computations();
    if (computations.count() > 0)
    {
        m_selectedComputation = computations[problemDir];

        // cache value
        auto currentDataAxisY = cmbAxisY->currentData().value<QPair<Study::ResultType, QString> >();
        Study::ResultType selectedType = currentDataAxisY.first;
        QString selectedKey = currentDataAxisY.second;

        trvResults->clear();

        geometryViewer->setProblem(static_cast<QSharedPointer<ProblemBase> >(m_selectedComputation));

        // fill treeview
        QFont fnt = trvResults->font();
        fnt.setBold(true);

        // parameters
        auto *parametersNode = new QTreeWidgetItem(trvResults);
        parametersNode->setText(0, tr("Parameters"));
        parametersNode->setFont(0, fnt);
        parametersNode->setIcon(0, iconAlphabet('P', AlphabetColor_Brown));
        parametersNode->setExpanded(true);

        QMap<QString, ProblemParameter> parameters = m_selectedComputation->config()->parameters()->items();
        foreach (Parameter parameter, m_selectedStudy->parameters())
        {
            auto *parameterNode = new QTreeWidgetItem(parametersNode);
            parameterNode->setText(0, parameter.name());
            parameterNode->setText(1, QString::number(parameters[parameter.name()].value()));
            parameterNode->setData(0, Qt::UserRole, parameter.name());
            parameterNode->setData(1, Qt::UserRole, Study::ResultType::ResultType_Parameter);
        }

        // functionals
        auto *functionalsNode = new QTreeWidgetItem(trvResults);
        functionalsNode->setText(0, tr("Goal Functions"));
        functionalsNode->setFont(0, fnt);
        functionalsNode->setIcon(0, iconAlphabet('F', AlphabetColor_Blue));
        functionalsNode->setExpanded(true);

        // recipes
        auto *recipesNode = new QTreeWidgetItem(trvResults);
        recipesNode->setText(0, tr("Recipes"));
        recipesNode->setFont(0, fnt);
        recipesNode->setIcon(0, iconAlphabet('R', AlphabetColor_Green));
        recipesNode->setExpanded(true);

        StringToDoubleMap results = m_selectedComputation->results()->items();
        foreach (QString key, results.keys())
        {
            QTreeWidgetItem *item = nullptr;
            if (m_selectedComputation->results()->type(key) == ComputationResultType_Functional)
            {
                item = new QTreeWidgetItem(functionalsNode);
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
    }
}

void OptiLab::doSolveCurrentComputation(bool ok)
{
    if (m_selectedComputation)
    {
        Agros::problem()->setCurrentComputation(m_selectedComputation);

        emit doSolveCurrentComputation(m_selectedComputation.data());
    }
}

void OptiLab::doResultChanged()
{
    bool showStats = cmbAxisY->currentIndex() != -1;

    groupBoxStatistics->setTitle(tr("Statistics - %1").arg(cmbAxisY->currentText()));
    lblResultMin->setText("-");
    lblResultMax->setText("-");
    lblResultMean->setText("-");
    lblResultMedian->setText("-");
    lblResultVariance->setText("-");
    lblResultStdDev->setText("-");

    if (m_selectedStudy && showStats)
    {
        QList<ComputationSet> computationSets = m_selectedStudy->computationSets(m_selectedStudy->value(Study::View_Filter).toString());

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
    if (cmbAxisY->currentIndex() != -1)
    {
        auto currentDataAxisY = cmbAxisY->currentData().value<QPair<Study::ResultType, QString> >();
        Study::ResultType currentDataResultAxisY = currentDataAxisY.first;
        QString currentDataNameY = currentDataAxisY.second;

        // extreme
        QSharedPointer<Computation> selectedComputation = m_selectedStudy->findExtreme(currentDataResultAxisY, currentDataNameY, minimum);

        if (!selectedComputation.isNull())
            doComputationSelected(selectedComputation->problemDir());
    }
}

void OptiLab::axisXChanged(int index)
{
    if (index != -1)
    {
        doChartRefreshed(true);
    }
}

void OptiLab::axisYChanged(int index)
{
    if (index != -1)
    {
        doResultChanged();
        doChartRefreshed(true);
    }
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
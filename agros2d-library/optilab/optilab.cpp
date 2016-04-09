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
#include "parameter.h"
#include "study.h"
#include "parameter.h"
#include "study_sweep.h"
#include "study_nsga2.h"
#include "study_bayesopt.h"
#include "study_nlopt.h"
#include "util/global.h"
#include "pythonlab/pythonengine.h"

#include "sceneview_data.h"
#include "logview.h"

#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"

#include "gui/infowidget.h"

#include "sceneview_geometry_simple.h"

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

class Statistics
{
public:
    Statistics(const QVector<double> x) : m_values(x), m_min(0.0), m_max(0.0), m_sum(0.0), m_mean(0.0), m_meanError(0.0), m_median(0.0), m_variance(0.0), m_stdDev(0.0)
    {
        if (x.isEmpty())
            return;

        // min and max
        auto mm = std::minmax_element(x.begin(), x.end());
        m_min = *mm.first;
        m_max = *mm.second;

        // sum
        m_sum = std::accumulate(x.begin(), x.end(), 0.0);

        // mean value
        m_mean = m_sum / x.size();

        // mean error
        m_meanError = m_mean / sqrt(x.size());

        // variance
        m_variance = 0.0;
        foreach (double val, x)
            m_variance += (m_mean - val)*(m_mean - val);
        m_variance = m_variance / x.size();

        // standard deviation
        m_stdDev = sqrt(m_variance);

        // median
        m_sortedValues = x;
        std::sort(m_sortedValues.begin(), m_sortedValues.end());
        if (m_sortedValues.size() % 2 == 0)
            m_median = (m_sortedValues[(m_sortedValues.size() / 2) - 1] + m_sortedValues[m_sortedValues.size() / 2]) / 2.0;
        else
            m_median = m_sortedValues[m_sortedValues.size() / 2];
    }

    inline double min() const { return m_min; }
    inline double max() const { return m_max; }
    inline double sum() const { return m_sum; }
    inline double mean() const { return m_mean; }
    inline double meanError() const { return m_meanError; }
    inline double median() const { return m_median; }
    inline double variance() const { return m_variance; }
    inline double stdDev() const { return m_stdDev; }

    inline int const size() { return m_values.size(); }
    inline QVector<double> const values() { return m_values; }
    inline QVector<double> const sortedValues() { return m_sortedValues; }

private:
    QVector<double> m_values;
    QVector<double> m_sortedValues;

    double m_min;
    double m_max;
    double m_sum;
    double m_mean;
    double m_meanError;
    double m_median;
    double m_variance;
    double m_stdDev;
};

class StatisticsCorrelation
{
public:
    StatisticsCorrelation(const Statistics &x, const Statistics &y) : m_statsX(x), m_statsY(y), m_correlation(0.0), m_covariance(0.0)
    {
        compute();
    }

    StatisticsCorrelation(const QVector<double> x, const QVector<double> y) : m_statsX(Statistics(x)), m_statsY(Statistics(y)), m_correlation(0.0), m_covariance(0.0)
    {
        compute();
    }

    inline double covariance() const { return m_covariance; }
    inline double correlation() const { return m_correlation; }

private:
    Statistics m_statsX;
    Statistics m_statsY;

    void compute()
    {
        assert(m_statsX.size() == m_statsY.size());

        // Covariance
        m_covariance = 0.0;
        for (int i = 0; i < m_statsX.size(); i++)
            m_covariance += (m_statsX.values()[i] - m_statsX.mean()) * (m_statsY.values()[i] - m_statsY.mean());

        m_covariance /= m_statsX.size();

        // Pearson product-moment correlation coefficient
        m_correlation = m_covariance / (m_statsX.stdDev() * m_statsY.stdDev());
    }

    double m_covariance;
    double m_correlation;
};

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent), m_optilab(parent)
{
    createControls();

    actRunStudy = new QAction(icon("run"), tr("Run study"), this);
    actRunStudy->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actRunStudy, SIGNAL(triggered()), this, SLOT(solveStudy()));

    connect(Agros2D::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(Agros2D::problem()->scene(), SIGNAL(invalidated()), this, SLOT(refresh()));

    connect(currentPythonEngine(), SIGNAL(executedScript()), this, SLOT(refresh()));
}

OptiLabWidget::~OptiLabWidget()
{

}

void OptiLabWidget::createControls()
{
    cmbStudies = new QComboBox(this);
    connect(cmbStudies, SIGNAL(currentIndexChanged(int)), this, SLOT(studyChanged(int)));

    lblNumberOfSets = new QLabel("");
    lblNumberOfComputations = new QLabel("");

    QGridLayout *layoutStudies = new QGridLayout();
    layoutStudies->addWidget(new QLabel(tr("Study:")), 0, 0);
    layoutStudies->addWidget(cmbStudies, 0, 1);
    layoutStudies->addWidget(new QLabel(tr("Number of sets:")), 1, 0);
    layoutStudies->addWidget(lblNumberOfSets, 1, 1);
    layoutStudies->addWidget(new QLabel(tr("Number of computations:")), 2, 0);
    layoutStudies->addWidget(lblNumberOfComputations, 2, 1);

    QWidget *widgetStudies = new QWidget(this);
    widgetStudies->setLayout(layoutStudies);

    // parameters
    trvComputations = new QTreeWidget(this);
    trvComputations->setMouseTracking(true);
    trvComputations->setColumnCount(2);
    trvComputations->setMinimumWidth(220);
    trvComputations->setColumnWidth(0, 220);
    trvComputations->setContextMenuPolicy(Qt::CustomContextMenu);

    QStringList headers;
    headers << tr("Computation") << tr("State");
    trvComputations->setHeaderLabels(headers);
    connect(trvComputations, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doComputationContextMenu(const QPoint &)));
    connect(trvComputations, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doComputationChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    actComputationSolve = new QAction(icon(""), tr("Solve problem"), this);
    connect(actComputationSolve, SIGNAL(triggered(bool)), this, SLOT(doComputationSolve(bool)));

    actComputationDelete = new QAction(icon(""), tr("Delete solution"), this);
    connect(actComputationDelete, SIGNAL(triggered(bool)), this, SLOT(doComputationDelete(bool)));

    mnuComputations = new QMenu(this);
    mnuComputations->addAction(actComputationSolve);
    mnuComputations->addSeparator();
    mnuComputations->addAction(actComputationDelete);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(widgetStudies);
    layout->addWidget(trvComputations);

    setLayout(layout);
}

void OptiLabWidget::refresh()
{
    actRunStudy->setEnabled(false);

    // fill studies
    cmbStudies->blockSignals(true);

    QString selectedItem = "";
    if (cmbStudies->currentIndex() != -1)
        selectedItem = cmbStudies->currentText();

    cmbStudies->clear();
    trvComputations->clear();
    foreach (Study *study, Agros2D::problem()->studies()->items())
        cmbStudies->addItem(studyTypeString(study->type()));

    cmbStudies->blockSignals(false);

    if (cmbStudies->count() > 0)
    {
        if (!selectedItem.isEmpty())
            cmbStudies->setCurrentText(selectedItem);
        else
            cmbStudies->setCurrentIndex(0);

        studyChanged(cmbStudies->currentIndex());
    }
}

void OptiLabWidget::studyChanged(int index)
{
    // study
    Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

    trvComputations->blockSignals(true);

    // computations
    QString selectedItem = "";
    if (trvComputations->currentItem())
        selectedItem = trvComputations->currentItem()->data(0, Qt::UserRole).toString();

    trvComputations->clear();

    double min = selectedItem.isEmpty() ? numeric_limits<double>::max() : 0.0;

    // fill tree view
    for (int i = 0; i < study->computationSets().size(); i++)
    {
        QList<ComputationSet> computationSets = study->computationSets();

        QTreeWidgetItem *itemComputationSet = new QTreeWidgetItem(trvComputations);
        itemComputationSet->setIcon(0, (computationSets[i].name().size() > 0) ? iconAlphabet(computationSets[i].name().at(0), AlphabetColor_Brown) : QIcon());
        // itemComputationSet->setCheckState(0, Qt::Checked);
        itemComputationSet->setText(0, tr("%1 (%2 computations)").arg(computationSets[i].name()).arg(computationSets[i].computations().size()));
        if (i == computationSets.size() - 1)
            itemComputationSet->setExpanded(true);

        foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(itemComputationSet);
            item->setText(0, computation->problemDir());
            item->setText(1, QString("%1 / %2").arg(computation->isSolved() ? tr("solved") : tr("not solved")).arg(computation->results()->hasResults() ? tr("results") : tr("no results")));
            item->setData(0, Qt::UserRole, computation->problemDir());

            // select minimum
            double localMin = study->evaluateSingleGoal(computation);
            if (localMin < min)
            {
                min = localMin;
                selectedItem = computation->problemDir();
            }
        }
    }

    trvComputations->blockSignals(false);

    // set stats
    lblNumberOfSets->setText(QString::number(study->computationSets().count()));
    lblNumberOfComputations->setText(QString::number(study->computationsCount()));

    // set study to optilab view
    m_optilab->setStudy(study);

    // select current computation
    if (!selectedItem.isEmpty())
        doComputationSelected(selectedItem);

    // if not selected -> select first
    if (trvComputations->topLevelItemCount() > 0 && !trvComputations->currentItem())
        trvComputations->setCurrentItem(trvComputations->topLevelItem(0));

    // enable buttons
    actRunStudy->setEnabled(true);
}

void OptiLabWidget::solveStudy()
{
    if (cmbStudies->count() == 0)
        return;

    // study
    Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

    LogOptimizationDialog *log = new LogOptimizationDialog(study);
    log->show();

    // solve
    study->solve();

    refresh();
}

void OptiLabWidget::doComputationSelected(const QString &key)
{
    for (int i = 0; i < trvComputations->topLevelItemCount(); i++)
    {
        for (int j = 0; j < trvComputations->topLevelItem(i)->childCount(); j++)
        {
            QTreeWidgetItem *item = trvComputations->topLevelItem(i)->child(j);
            if (item->data(0, Qt::UserRole).toString() == key)
            {
                trvComputations->setCurrentItem(item);
                trvComputations->scrollToItem(item);

                // refresh chart
                emit chartRefreshed(Agros2D::computations()[key]->problemDir());

                return;
            }
        }
    }
}

void OptiLabWidget::doComputationChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    if (trvComputations->currentItem())
    {
        QString key = trvComputations->currentItem()->data(0, Qt::UserRole).toString();
        actComputationDelete->setEnabled(!key.isEmpty());

        emit computationSelected(key);
    }
}

void OptiLabWidget::doComputationDelete(bool)
{
    if (trvComputations->currentItem())
    {
        QString key = trvComputations->currentItem()->data(0, Qt::UserRole).toString();
        if (!key.isEmpty() && Agros2D::computations().contains(key))
        {
            Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

            // remove empty computation sets
            QMutableListIterator<ComputationSet> i(study->computationSets());
            while (i.hasNext())
            {
                ComputationSet *computationSet = &i.next();
                computationSet->removeComputation(Agros2D::computations()[key]);

                if (computationSet->computations().count() == 0)
                    i.remove();
            }

            refresh();
        }
    }
}

void OptiLabWidget::doComputationSolve(bool)
{
    if (trvComputations->currentItem())
    {
        QString key = trvComputations->currentItem()->data(0, Qt::UserRole).toString();
        if (!key.isEmpty() && Agros2D::computations().contains(key))
        {
            Agros2D::computations()[key]->solveWithGUI();
        }
    }
}


void OptiLabWidget::doComputationContextMenu(const QPoint &pos)
{
    mnuComputations->exec(QCursor::pos());
}

// ***************************************************************************************************************************************

OptiLab::OptiLab(QWidget *parent) : QWidget(parent), m_study(nullptr)
{
    actSceneModeOptiLab = new QAction(icon("optilab"), tr("OptiLab"), this);
    actSceneModeOptiLab->setShortcut(Qt::Key_F8);
    actSceneModeOptiLab->setCheckable(true);

    m_optiLabWidget = new OptiLabWidget(this);

    createControls();

    connect(Agros2D::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));

    connect(m_optiLabWidget, SIGNAL(computationSelected(QString)), this, SLOT(doComputationSelected(QString)));
    connect(m_optiLabWidget, SIGNAL(chartRefreshed(QString)), this, SLOT(doChartRefreshed(QString)));
    connect(this, SIGNAL(computationSelected(QString)), m_optiLabWidget, SLOT(doComputationSelected(QString)));
}

OptiLab::~OptiLab()
{
    QSettings settings;
    settings.setValue("OptiLab/TreeColumnWidth0", trvResults->columnWidth(0));
    settings.setValue("OptiLab/TreeColumnWidth1", trvResults->columnWidth(1));
    settings.setValue("OptiLab/SplitterState", splitter->saveState());
    settings.setValue("OptiLab/SplitterGeometry", splitter->saveGeometry());
}

void OptiLab::setStudy(Study *study)
{
    m_study = study;

    doChartRefreshed("");
    geometryViewer->doZoomBestFit();
}

QWidget *OptiLab::createControlsDistChart()
{
    pdfChart = new QCustomPlot(this);
    // pdfChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QCPBars *pdfDataBars = new QCPBars(pdfChart->xAxis, pdfChart->yAxis);
    pdfDataBars->setPen(QPen(QColor(255, 131, 0)));
    pdfDataBars->setBrush(QColor(255, 131, 0, 50));
    pdfDataBars->setName(tr("Data"));
    pdfDataBars->addToLegend();
    pdfChart->addPlottable(pdfDataBars);

    QCPItemStraightLine *pdfDataMeanLine = new QCPItemStraightLine(pdfChart);
    pdfDataMeanLine->setPen(QPen(QBrush(QColor(40, 40, 140, 100)), 1, Qt::DashLine));
    pdfChart->addItem(pdfDataMeanLine);

    QCPGraph *pdfNormalGraph = pdfChart->addGraph(pdfChart->xAxis, pdfChart->yAxis2);
    pdfNormalGraph->setName(tr("Normal"));
    pdfNormalGraph->addToLegend();

    pdfChart->xAxis->setTickLabelRotation(60);
    pdfChart->xAxis->setLabel(tr("Samples"));
    pdfChart->yAxis->setLabel(tr("Number of occurrences"));
    pdfChart->yAxis2->setVisible(false);
    // pdfChart->legend->setVisible(true);
    // histogramChart->setContextMenuPolicy(Qt::CustomContextMenu);

    cdfChart = new QCustomPlot(this);
    // cdfChart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QCPGraph *cdfDataGraph = cdfChart->addGraph(cdfChart->xAxis, cdfChart->yAxis);
    cdfDataGraph->setName(tr("Data"));
    cdfDataGraph->addToLegend();
    // cdfGraph->setLineStyle(QCPGraph::lsNone);
    QPen cdfPen;
    cdfPen.setColor(QColor(255, 131, 0));
    cdfPen.setStyle(Qt::DotLine);
    cdfPen.setWidthF(2);
    cdfDataGraph->setPen(cdfPen);
    cdfDataGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(255, 131, 0), QColor(255, 131, 0, 50), 7));

    QCPGraph *cdfNormalGraph = cdfChart->addGraph(cdfChart->xAxis, cdfChart->yAxis2);
    cdfNormalGraph->setName(tr("Normal"));
    cdfNormalGraph->addToLegend();

    cdfChart->xAxis->setTickLabelRotation(60);
    cdfChart->xAxis->setLabel(tr("Samples"));
    cdfChart->yAxis->setLabel(tr("Count"));
    cdfChart->yAxis2->setVisible(false);
    // cdfChart->legend->setVisible(true);
    cdfChart->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

    lblResultMin = new QLabel();
    lblResultMin->setMinimumWidth(90);
    lblResultMax = new QLabel();
    lblResultSum = new QLabel();
    lblResultMean = new QLabel();
    lblResultMedian = new QLabel();
    lblResultVariance = new QLabel();
    lblResultStdDev = new QLabel();
    lblResultNormalCovariance = new QLabel(); // normal distribution
    lblResultNormalCorrelation = new QLabel(); // normal distribution

    QGridLayout *layoutStatistics = new QGridLayout();
    layoutStatistics->addWidget(new QLabel(tr("Minimum:")), 0, 0);
    layoutStatistics->addWidget(lblResultMin, 0, 1);
    layoutStatistics->addWidget(new QLabel(tr("Maximum:")), 1, 0);
    layoutStatistics->addWidget(lblResultMax, 1, 1);
    layoutStatistics->addWidget(new QLabel(tr("Sum:")), 2, 0);
    layoutStatistics->addWidget(lblResultSum, 2, 1);
    layoutStatistics->addWidget(new QLabel(tr("Mean value:")), 3, 0);
    layoutStatistics->addWidget(lblResultMean, 3, 1);
    layoutStatistics->addWidget(new QLabel(tr("Median:")), 4, 0);
    layoutStatistics->addWidget(lblResultMedian, 4, 1);
    layoutStatistics->addWidget(new QLabel(tr("Variance:")), 5, 0);
    layoutStatistics->addWidget(lblResultVariance, 5, 1);
    layoutStatistics->addWidget(new QLabel(tr("Std. deviation:")), 6, 0);
    layoutStatistics->addWidget(lblResultStdDev, 6, 1);
    layoutStatistics->setRowStretch(10, 1);
    layoutStatistics->addWidget(new QLabel(tr("Covariance:")), 21, 0);
    layoutStatistics->addWidget(lblResultNormalCovariance, 21, 1);
    layoutStatistics->addWidget(new QLabel(tr("Correlation:")), 22, 0);
    layoutStatistics->addWidget(lblResultNormalCorrelation, 22, 1);

    QHBoxLayout *layoutDetails = new QHBoxLayout();
    layoutDetails->setContentsMargins(0, 0, 0, 0);
    layoutDetails->addWidget(pdfChart, 1);
    layoutDetails->addWidget(cdfChart, 1);
    layoutDetails->addLayout(layoutStatistics, 0);

    QWidget *widDist = new QWidget();
    widDist->setContentsMargins(0, 0, 0, 0);
    widDist->setLayout(layoutDetails);

    return widDist;
}

QWidget *OptiLab::createControlsChart()
{
    actChartRescale = new QAction(icon(""), tr("Rescale chart"), this);
    connect(actChartRescale, SIGNAL(triggered(bool)), this, SLOT(chartRescale(bool)));

    actChartLogHorizontal = new QAction(icon(""), tr("Logarithmic scale (x-axis)"), this);
    actChartLogHorizontal->setCheckable(true);
    connect(actChartLogHorizontal, SIGNAL(triggered(bool)), this, SLOT(chartLogHorizontal(bool)));

    actChartLogVertical = new QAction(icon(""), tr("Logarithmic scale (y-axis)"), this);
    actChartLogVertical->setCheckable(true);
    connect(actChartLogVertical, SIGNAL(triggered(bool)), this, SLOT(chartLogVertical(bool)));

    actChartShowTrend = new QAction(icon(""), tr("Show trend line"), this);
    actChartShowTrend->setCheckable(true);
    connect(actChartShowTrend, SIGNAL(triggered(bool)), this, SLOT(chartShowTrend(bool)));

    actChartShowAverageValue = new QAction(icon(""), tr("Show average value"), this);
    actChartShowAverageValue->setCheckable(true);
    connect(actChartShowAverageValue, SIGNAL(triggered(bool)), this, SLOT(chartShowAverageValue(bool)));

    mnuChart = new QMenu(this);
    mnuChart->addAction(actChartRescale);
    mnuChart->addSection(tr("Chart properties"));
    mnuChart->addAction(actChartLogHorizontal);
    mnuChart->addAction(actChartLogVertical);
    mnuChart->addAction(actChartShowTrend);
    mnuChart->addAction(actChartShowAverageValue);

    chart = new QCustomPlot(this);
    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    chart->setContextMenuPolicy(Qt::CustomContextMenu);
    chart->legend->setVisible(true);
    connect(chart, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(chartContextMenu(const QPoint &)));
    connect(chart, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, QMouseEvent*)));
    connect(chart, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(graphMouseDoubleClick(QMouseEvent*)));

    // chart trend line
    chartTrendLine = new QCPItemLine(chart);
    chart->addItem(chartTrendLine);
    chartTrendLine->setVisible(false);

    // mean value
    chartGraphAverageValue = chart->addGraph();
    chartGraphAverageValue->removeFromLegend();
    chartGraphAverageValue->setPen(QPen(QBrush(QColor(140, 140, 140)), 3, Qt::DashLine));
    // graph->setBrush(QBrush(Qt::red));
    chartGraphAverageValue->setLineStyle(QCPGraph::lsLine);

    chartGraphAverageValueChannelLower = chart->addGraph();
    chartGraphAverageValueChannelLower->removeFromLegend();
    chartGraphAverageValueChannelLower->setPen(QPen(QBrush(QColor(180, 180, 180)), 1, Qt::DotLine));

    chartGraphAverageValueChannelUpper = chart->addGraph();
    chartGraphAverageValueChannelUpper->removeFromLegend();
    chartGraphAverageValueChannelUpper->setPen(QPen(QBrush(QColor(180, 180, 180)), 1, Qt::DotLine));
    chartGraphAverageValueChannelUpper->setBrush(QBrush(QColor(255, 50, 30, 10)));
    chartGraphAverageValueChannelUpper->setChannelFillGraph(chartGraphAverageValueChannelLower);

    // selected computation
    chartGraphSelectedComputation = chart->addGraph();
    chartGraphSelectedComputation->removeFromLegend();
    chartGraphSelectedComputation->setLineStyle(QCPGraph::lsNone);
    chartGraphSelectedComputation->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Qt::darkGray, 17));

    return chart;
}

QWidget *OptiLab::createControlsResults()
{
    QSettings settings;

    // treeview
    trvResults = new QTreeWidget(this);
    trvResults->setExpandsOnDoubleClick(false);
    trvResults->setHeaderHidden(false);
    trvResults->setHeaderLabels(QStringList() << tr("Name") << tr("Value"));
    // trvResults->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    trvResults->setContextMenuPolicy(Qt::CustomContextMenu);
    trvResults->setMouseTracking(true);
    trvResults->setUniformRowHeights(true);
    trvResults->setColumnCount(2);
    trvResults->setColumnWidth(0, settings.value("OptiLab/TreeColumnWidth0", 80).toInt());
    trvResults->setColumnWidth(1, settings.value("OptiLab/TreeColumnWidth1", 120).toInt());
    trvResults->setIndentation(trvResults->indentation() - 2);
    connect(trvResults, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(resultsContextMenu(const QPoint &)));
    connect(trvResults, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doResultChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    actResultsDependenceOnSteps = new QAction(icon(""), tr("Dependence on the number of steps"), this);
    connect(actResultsDependenceOnSteps, SIGNAL(triggered(bool)), this, SLOT(resultsDependenceOnSteps(bool)));

    actResultsSetHorizontal = new QAction(icon(""), tr("Set on horizontal axis"), this);
    connect(actResultsSetHorizontal, SIGNAL(triggered(bool)), this, SLOT(resultsSetHorizontal(bool)));

    actResultsSetVertical = new QAction(icon(""), tr("Set on vertical axis"), this);
    connect(actResultsSetVertical, SIGNAL(triggered(bool)), this, SLOT(resultsSetVertical(bool)));

    actResultsFindMinimum = new QAction(icon(""), tr("Find minimum"), this);
    connect(actResultsFindMinimum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMinimum(bool)));
    actResultsFindMaximum = new QAction(icon(""), tr("Find maximum"), this);
    connect(actResultsFindMaximum, SIGNAL(triggered(bool)), this, SLOT(resultsFindMaximum(bool)));

    mnuResults = new QMenu(trvResults);
    mnuResults->addAction(actResultsDependenceOnSteps);
    mnuResults->addAction(actResultsSetHorizontal);
    mnuResults->addAction(actResultsSetVertical);
    mnuResults->addSection(tr("Statistics"));
    mnuResults->addAction(actResultsFindMinimum);
    mnuResults->addAction(actResultsFindMaximum);

    QVBoxLayout *layoutResults = new QVBoxLayout();
    layoutResults->addWidget(trvResults, 2);
    layoutResults->addWidget(geometryViewer, 1);

    QWidget *widResults = new QWidget();
    widResults->setContentsMargins(0, 0, 0, 0);
    widResults->setLayout(layoutResults);

    return widResults;
}

void OptiLab::createControls()
{
    geometryViewer = new SceneViewSimpleGeometry(this);

    QVBoxLayout *layoutCharts = new QVBoxLayout();
    layoutCharts->addWidget(createControlsChart(), 2);
    layoutCharts->addWidget(createControlsDistChart(), 1);

    QWidget *widCharts = new QWidget();
    widCharts->setContentsMargins(0, 0, 0, 0);
    widCharts->setLayout(layoutCharts);

    QSettings settings;

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(createControlsResults());
    splitter->addWidget(widCharts);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 1);
    splitter->restoreState(settings.value("OptiLab/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("OptiLab/SplitterGeometry").toByteArray());

    QHBoxLayout *layoutLab = new QHBoxLayout();
    layoutLab->addWidget(splitter);

    setLayout(layoutLab);
}

void OptiLab::refresh()
{
    actSceneModeOptiLab->setEnabled(!Agros2D::problem()->studies()->items().isEmpty());
}

void OptiLab::chartContextMenu(const QPoint &pos)
{
    mnuChart->exec(QCursor::pos());
}

void OptiLab::chartRescale(bool checked)
{
    chart->rescaleAxes();
    chart->replot(QCustomPlot::rpQueued);
}

void OptiLab::chartLogHorizontal(bool checked)
{
    // force rescale
    bool rescale = (m_study->value(Study::View_ChartLogHorizontal).toBool() != checked);

    m_study->setValue(Study::View_ChartLogHorizontal, checked);
    actChartLogHorizontal->setChecked(checked);

    if (checked)
        chart->xAxis->setScaleType(QCPAxis::stLogarithmic);
    else
        chart->xAxis->setScaleType(QCPAxis::stLinear);

    if (rescale) chartRescale(true);
}

void OptiLab::chartLogVertical(bool checked)
{
    // force rescale
    bool rescale = (m_study->value(Study::View_ChartLogVertical).toBool() != checked);

    m_study->setValue(Study::View_ChartLogVertical, checked);
    actChartLogVertical->setChecked(checked);

    if (checked)
        chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    else
        chart->yAxis->setScaleType(QCPAxis::stLinear);

    if (rescale) chartRescale(true);
}

void OptiLab::chartShowTrend(bool checked)
{
    m_study->setValue(Study::View_ChartShowTrend, checked);
    actChartShowTrend->setChecked(checked);

    chartTrendLine->setVisible(checked);
    chart->replot(QCustomPlot::rpQueued);
}

void OptiLab::chartShowAverageValue(bool checked)
{
    m_study->setValue(Study::View_ChartShowAverageValue, checked);
    actChartShowAverageValue->setChecked(checked);

    chartGraphAverageValue->setVisible(checked);
    chartGraphAverageValueChannelLower->setVisible(checked);
    chartGraphAverageValueChannelUpper->setVisible(checked);

    chart->replot(QCustomPlot::rpQueued);
}

void OptiLab::resultsDependenceOnSteps(bool checked)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        m_study->setValue(Study::View_ChartHorizontal, QString("step:"));
        m_study->setValue(Study::View_ChartVertical, QString("%1:%2").
                          arg(m_study->resultTypeToStringKey((Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt())).
                          arg(trvResults->currentItem()->data(0, Qt::UserRole).toString()));

        doChartRefreshed();
    }
}

void OptiLab::resultsSetHorizontal(bool checked)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        m_study->setValue(Study::View_ChartHorizontal, QString("%1:%2").
                          arg(m_study->resultTypeToStringKey((Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt())).
                          arg(trvResults->currentItem()->data(0, Qt::UserRole).toString()));

        doChartRefreshed();
    }
}

void OptiLab::resultsSetVertical(bool checked)
{
    if (trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        m_study->setValue(Study::View_ChartVertical, QString("%1:%2").
                          arg(m_study->resultTypeToStringKey((Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt())).
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

void OptiLab::doComputationSelected(const QString &key)
{
    // cache value
    Study::ResultType selectedType = Study::ResultType_Parameter;
    QString selectedKey = "";
    QTreeWidgetItem *selectedItem = nullptr;
    if (!key.isEmpty() && trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid())
    {
        selectedType = (Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt();
        selectedKey = trvResults->currentItem()->data(0, Qt::UserRole).toString();
    }

    trvResults->clear();

    if (key.isEmpty())
    {
        geometryViewer->setProblem(QSharedPointer<ProblemBase>(nullptr));
    }
    else
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[key];

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

        StringToDoubleMap parameters = computation->config()->parameters();
        foreach (Parameter parameter, m_study->parameters())
        {
            QTreeWidgetItem *parameterNode = new QTreeWidgetItem(parametersNode);
            parameterNode->setText(0, parameter.name());
            parameterNode->setText(1, QString::number(parameters[parameter.name()]));
            parameterNode->setData(0, Qt::UserRole, parameter.name());
            parameterNode->setData(1, Qt::UserRole, Study::ResultType::ResultType_Parameter);

            if (selectedType == Study::ResultType::ResultType_Parameter && selectedKey == parameter.name())
                selectedItem = parameterNode;
        }

        // functionals
        QTreeWidgetItem *functionalsNode = new QTreeWidgetItem(trvResults);
        functionalsNode->setText(0, tr("Functionals"));
        functionalsNode->setFont(0, fnt);
        functionalsNode->setIcon(0, iconAlphabet('F', AlphabetColor_Blue));
        functionalsNode->setExpanded(true);

        // recipes
        QTreeWidgetItem *recipesNode = new QTreeWidgetItem(trvResults);
        recipesNode->setText(0, tr("Recipes"));
        recipesNode->setFont(0, fnt);
        recipesNode->setIcon(0, iconAlphabet('R', AlphabetColor_Green));
        recipesNode->setExpanded(true);

        StringToDoubleMap results = computation->results()->items();
        foreach (QString key, results.keys())
        {
            QTreeWidgetItem *item = nullptr;
            if (computation->results()->resultType(key) == ComputationResultType_Functional)
            {
                item = new QTreeWidgetItem(functionalsNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Functional);

                if (selectedType == Study::ResultType::ResultType_Functional && selectedKey == key)
                    selectedItem = item;
            }
            else if (computation->results()->resultType(key) == ComputationResultType_Recipe)
            {
                item = new QTreeWidgetItem(recipesNode);
                item->setData(1, Qt::UserRole, Study::ResultType::ResultType_Recipe);

                if (selectedType == Study::ResultType::ResultType_Recipe && selectedKey == key)
                    selectedItem = item;
            }
            else
                assert(0);

            item->setText(0, key);
            item->setText(1, QString::number(results[key]));
            item->setData(0, Qt::UserRole, key);
        }

        // paint chart
        doChartRefreshed(key);

        // select item
        if (selectedItem)
            trvResults->setCurrentItem(selectedItem);
    }
}

void OptiLab::doChartRefreshed(const QString &key)
{
    m_computationMap.clear();
    chartGraphSelectedComputation->clearData();

    foreach (QCPGraph *graph, chartGraphCharts)
        chart->removeGraph(graph);
    chartGraphCharts.clear();

    if (!m_study) return;

    QString chartX = m_study->value(Study::View_ChartHorizontal).toString();
    QString chartY = m_study->value(Study::View_ChartVertical).toString();

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
    else
        assert(0);

    // linear regression
    QVector<double> linRegDataX;
    QVector<double> linRegDataY;

    int paletteStep = m_study->computationSets().count() > 1 ? (PALETTEENTRIES - 1) / (m_study->computationSets().count() - 1) : 0;
    int step = 0;
    for (int i = 0; i < m_study->computationSets().count(); i++)
    {
        QVector<double> dataSetX;
        QVector<double> dataSetY;

        QList<QSharedPointer<Computation> > computations = m_study->computationSets()[i].computations();

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
                double value = computation->config()->parameter(name);
                dataSetX.append(value);
            }
            if (chartY.contains("parameter:"))
            {
                QString name = chartY.right(chartY.count() - 10);
                double value = computation->config()->parameter(name);
                dataSetY.append(value);
            }

            // functionals
            if (chartX.contains("functional:"))
            {
                QString name = chartX.right(chartX.count() - 11);
                double value = computation->results()->resultValue(name);
                dataSetX.append(value);
            }
            if (chartY.contains("functional:"))
            {
                QString name = chartY.right(chartY.count() - 11);
                double value = computation->results()->resultValue(name);
                dataSetY.append(value);
            }

            // recipes
            if (chartX.contains("recipe:"))
            {
                QString name = chartX.right(chartX.count() - 7);
                double value = computation->results()->resultValue(name);
                dataSetX.append(value);
            }
            if (chartY.contains("recipe:"))
            {
                QString name = chartY.right(chartY.count() - 7);
                double value = computation->results()->resultValue(name);
                dataSetY.append(value);
            }

            // selected point
            if (computation->problemDir() == key)
            {
                QVector<double> selectedX; selectedX << dataSetX.last();
                QVector<double> selectedY; selectedY << dataSetY.last();

                chartGraphSelectedComputation->setData(selectedX, selectedY);
            }

            // computation map
            m_computationMap[i].insert(QPair<double, double>(dataSetX.last(), dataSetY.last()), computation);
        }

        int colorFillIndex = i * paletteStep;
        const QColor colorFill(paletteDataAgros[colorFillIndex][0] * 255, paletteDataAgros[colorFillIndex][1] * 255, paletteDataAgros[colorFillIndex][2] * 255);
        // int colorBrushIndex = abs(goal - minGoal) / (maxGoal - minGoal) * 255;
        // const QColor colorBrush(paletteDataParuly[colorBrushIndexp][0] * 255, paletteDataParuly[colorBrushIndex][1] * 255, paletteDataParuly[colorBrushIndex][2] * 255);

        // add data to global array (for min and max computation)
        linRegDataX << dataSetX;
        linRegDataY << dataSetY;

        QCPGraph *chartGraphChart = chart->addGraph();
        chartGraphCharts.append(chartGraphChart);
        chartGraphChart->setData(dataSetX, dataSetY);
        chartGraphChart->setProperty("computationset_index", i);
        chartGraphChart->setName(m_study->computationSets()[i].name());
        chartGraphChart->addToLegend();

        chartGraphChart->setLineStyle(QCPGraph::lsNone);
        chartGraphChart->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(140, 140, 140), colorFill, 9));
    }

    // mean value and standard deviation
    Statistics statsX(linRegDataX);
    Statistics statsY(linRegDataY);

    // bounding box
    double delta = qMin((statsX.max() - statsX.min()) * 0.1, (statsY.max() - statsY.min()) * 0.1);

    QVector<double> meanX; meanX << statsX.min() - delta << statsX.max() + delta;
    QVector<double> meanY; meanY << statsY.mean() << statsY.mean();
    QVector<double> devUpper; devUpper << statsY.mean() + statsY.stdDev() << statsY.mean() + statsY.stdDev();
    QVector<double> devLower; devLower << statsY.mean() - statsY.stdDev() << statsY.mean() - statsY.stdDev();
    chartGraphAverageValue->setData(meanX, meanY);
    chartGraphAverageValueChannelLower->setData(meanX, devLower);
    chartGraphAverageValueChannelUpper->setData(meanX, devUpper);

    // linear regression
    StatisticsLinReg linReg(linRegDataX, linRegDataY);
    if (linReg.computeLinReg())
    {
        // trend arrow
        QPointF start((statsX.min() - delta), (statsX.min() - delta) * linReg.slope() + linReg.shift());
        QPointF end((statsX.max() + delta), (statsX.max() + delta) * linReg.slope() + linReg.shift());

        chartTrendLine->start->setCoords(start);
        chartTrendLine->end->setCoords(end);
        chartTrendLine->setHead(QCPLineEnding::esSpikeArrow);
        chartTrendLine->setPen(QPen(QBrush(QColor(200, 200, 200)), 5));
    }

    // plot main chart
    chart->xAxis->setLabel(labelX);
    chart->yAxis->setLabel(labelY);

    // replot chart
    chart->replot(QCustomPlot::rpQueued);

    // set actions
    chartLogHorizontal(m_study->value(Study::View_ChartLogHorizontal).toBool());
    chartLogHorizontal(m_study->value(Study::View_ChartLogVertical).toBool());
    chartShowTrend(m_study->value(Study::View_ChartShowTrend).toBool());
    chartShowAverageValue(m_study->value(Study::View_ChartShowAverageValue).toBool());
}

QCPData OptiLab::findClosestData(QCPGraph *graph, const Point &pos)
{    
    // find min and max
    RectPoint bound;
    bound.start.x = numeric_limits<double>::max();
    bound.end.x = -numeric_limits<double>::max();
    bound.start.y = numeric_limits<double>::max();
    bound.end.y = -numeric_limits<double>::max();

    foreach (const QCPData data, graph->data()->values())
    {
        if (data.key < bound.start.x) bound.start.x = data.key;
        if (data.key > bound.end.x) bound.end.x = data.key;
        if (data.value < bound.start.y) bound.start.y = data.value;
        if (data.value > bound.end.y) bound.end.y = data.value;
    }

    // find closest point
    QCPData selectedData(0, 0);
    double dist = numeric_limits<double>::max();
    foreach (const QCPData data, graph->data()->values())
    {
        double mag = Point((data.key - pos.x) / bound.width(),
                           (data.value - pos.y) / bound.height()).magnitudeSquared();

        if (mag <= dist)
        {
            dist = mag;
            selectedData = data;
        }
    }

    return selectedData;
}

void OptiLab::graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event)
{
    if (QCPGraph *graph = dynamic_cast<QCPGraph *>(plottable))
    {
        // find closest point
        QCPData selectedData = findClosestData(graph, Point(chart->xAxis->pixelToCoord(event->pos().x()),
                                                            chart->yAxis->pixelToCoord(event->pos().y())));

        QSharedPointer<Computation> selectedComputation = m_computationMap[graph->property("computationset_index").toInt()][QPair<double, double>(selectedData.key, selectedData.value)];
        if (!selectedComputation.isNull())
        {
            QToolTip::hideText();
            QToolTip::showText(event->globalPos(),
                               tr("<table>"
                                  "<tr><th colspan=\"2\">%L1</th></tr>"
                                  "<tr><td>X:</td>" "<td>%L2</td></tr>"
                                  "<tr><td>Y:</td>" "<td>%L3</td></tr>"
                                  "</table>").
                               arg(graph->name().isEmpty() ? "..." : graph->name()).arg(selectedData.key).arg(selectedData.value), chart, chart->rect());

            emit computationSelected(selectedComputation->problemDir());
        }
    }
}

void OptiLab::graphMouseDoubleClick(QMouseEvent *event)
{
    // rescale chart
    if (event->buttons() & Qt::MidButton)
    {
        chartRescale(true);
        return;
    }
}

void OptiLab::doResultChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    bool showStats = trvResults->currentItem() && trvResults->currentItem()->data(1, Qt::UserRole).isValid();

    lblResultMin->setText("-");
    lblResultMax->setText("-");
    lblResultSum->setText("-");
    lblResultMean->setText("-");
    lblResultMedian->setText("-");
    lblResultVariance->setText("-");
    lblResultStdDev->setText("-");
    lblResultNormalCovariance->setText("-");
    lblResultNormalCorrelation->setText("-");

    pdfChart->graph(0)->setVisible(showStats);
    pdfChart->plottable(0)->setVisible(showStats);
    pdfChart->item(0)->setVisible(showStats);

    cdfChart->graph(0)->setVisible(showStats);
    cdfChart->graph(1)->setVisible(showStats);

    if (showStats)
    {
        Study::ResultType type = (Study::ResultType) trvResults->currentItem()->data(1, Qt::UserRole).toInt();
        QString key = trvResults->currentItem()->data(0, Qt::UserRole).toString();

        QVector<double> step;
        QVector<double> data;

        for (int i = 0; i < m_study->computationSets().count(); i++)
        {
            QList<QSharedPointer<Computation> > computations = m_study->computationSets()[i].computations();

            for (int j = 0; j < computations.count(); j++)
            {
                QSharedPointer<Computation> computation = computations[j];

                if (type == Study::ResultType_Parameter)
                    data.append(computation->config()->parameter(key));
                else if (type == Study::ResultType_Recipe || type == Study::ResultType_Functional)
                    data.append(computation->results()->resultValue(key));

                step.append(step.count());
            }
        }

        if (data.size() > 0)
        {
            Statistics stats(data);

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

            QCPBars *pdfDataBars = dynamic_cast<QCPBars *>(pdfChart->plottable(0));
            pdfDataBars->setWidth(dataStep * 0.9);
            pdfDataBars->setData(dataSteps, dataPDF);
            QCPItemStraightLine *pdfDataMeanLine = dynamic_cast<QCPItemStraightLine *>(pdfChart->item(0));
            pdfDataMeanLine->point1->setCoords(QPointF(stats.mean(), 0));
            pdfDataMeanLine->point2->setCoords(QPointF(stats.mean(), 1));

            // normal distribution
            int normalCount = 100;
            QVector<double> normalPDF(normalCount);
            QVector<double> normalCDF(normalCount);
            QVector<double> normalSteps(normalCount);
            for (int i = 0; i < normalCount; i++)
            {
                normalSteps[i] = stats.min() + (stats.max() - stats.min()) / (100 - 1) * i;
                normalPDF[i] = boost::math::pdf(normalDistribution, normalSteps[i]);
                normalCDF[i] = boost::math::cdf(normalDistribution, normalSteps[i]);
            }
            pdfChart->graph(0)->setData(normalSteps, normalPDF);

            pdfChart->xAxis->setLabel(key);
            pdfChart->rescaleAxes();

            // cdf
            cdfChart->xAxis->setLabel(key);
            cdfChart->graph(0)->setData(dataSteps, dataCDF);
            cdfChart->graph(1)->setData(normalSteps, normalCDF);
            cdfChart->rescaleAxes();

            // labels
            lblResultMin->setText(QString::number(stats.min()));
            lblResultMax->setText(QString::number(stats.max()));
            lblResultSum->setText(QString::number(stats.sum()));
            lblResultMean->setText(QString::number(stats.mean()));
            lblResultMedian->setText(QString::number(stats.median()));
            lblResultVariance->setText(QString::number(stats.variance()));
            lblResultStdDev->setText(QString::number(stats.stdDev()));

            // correlation and covariance
            StatisticsCorrelation statsCorrelation(dataCDF, normalCDFCorrelation);
            lblResultNormalCovariance->setText(QString::number(statsCorrelation.covariance()));
            lblResultNormalCorrelation->setText(QString::number(statsCorrelation.correlation()));
        }
    }

    pdfChart->replot(QCustomPlot::rpQueued);
    cdfChart->replot(QCustomPlot::rpQueued);
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
        QSharedPointer<Computation> selectedComputation = m_study->findExtreme(type, key, minimum);

        if (!selectedComputation.isNull())
            emit computationSelected(selectedComputation->problemDir());
    }
}

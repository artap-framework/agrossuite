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
#include "optilab/parameter.h"
#include "optilab/study.h"
#include "optilab/parameter.h"
#include "optilab/study_sweep.h"
#include "optilab/study_nsga2.h"
#include "optilab/study_bayesopt.h"
#include "optilab/study_nlopt.h"
#include "optilab/study_dialog.h"
#include "util/global.h"

#include "app/sceneview_data.h"
#include "logview.h"

#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"

#include "gui/infowidget.h"
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

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent), m_optilab(parent)
{
    createControls();

    actRunStudy = new QAction(icon("main_solve"), tr("Run study"), this);
    actRunStudy->setShortcut(QKeySequence("Alt+S"));
    connect(actRunStudy, SIGNAL(triggered()), this, SLOT(solveStudy()));
}

OptiLabWidget::~OptiLabWidget()
{
}

void OptiLabWidget::createControls()
{
    cmbStudies = new QComboBox(this);
    connect(cmbStudies, SIGNAL(currentIndexChanged(int)), this, SLOT(studyChanged(int)));

    lblNumberOfComputations = new QLabel("");

    // filter
    txtFilter = new QLineEdit();

    QGridLayout *layoutStudies = new QGridLayout();
    layoutStudies->addWidget(new QLabel(tr("Study:")), 0, 0);
    layoutStudies->addWidget(cmbStudies, 0, 1);
    layoutStudies->addWidget(new QLabel(tr("Filter:")), 1, 0);
    layoutStudies->addWidget(txtFilter, 1, 1);
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

    actComputationSolve = new QAction(tr("Solve problem"), this);
    connect(actComputationSolve, SIGNAL(triggered(bool)), this, SLOT(doComputationSolve(bool)));

    actComputationDelete = new QAction(tr("Delete solution"), this);
    connect(actComputationDelete, SIGNAL(triggered(bool)), this, SLOT(doComputationDelete(bool)));

    mnuComputations = new QMenu(this);
    mnuComputations->addAction(actComputationSolve);
    mnuComputations->addSeparator();
    mnuComputations->addAction(actComputationDelete);

    QPushButton *btnApply = new QPushButton(tr("Apply"));
    connect(btnApply, SIGNAL(clicked(bool)), this, SLOT(refresh()));

    QPushButton *btnExport = new QPushButton(tr("Export"));
    connect(btnExport, SIGNAL(clicked(bool)), this, SLOT(exportData()));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnExport);
    layoutButton->addWidget(btnApply);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(widgetStudies);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutButton);

    setLayout(layout);
}

void OptiLabWidget::refresh()
{    
    actRunStudy->setEnabled(false);

    // fill studies
    cmbStudies->blockSignals(true);

    QString selectedItem = "";
    if (cmbStudies->currentIndex() != -1)
    {
        selectedItem = cmbStudies->currentText();

        // study - set filter
        if (Agros::problem()->studies()->items().count() > 0)
        {
            Study *study = Agros::problem()->studies()->items().at(cmbStudies->currentIndex());
            study->setValue(Study::View_Filter, txtFilter->text());
        }
    }

    // clear filter
    txtFilter->clear();

    cmbStudies->clear();
    trvComputations->clear();
    foreach (Study *study, Agros::problem()->studies()->items())
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

void OptiLabWidget::exportData()
{
    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), dir, tr("CSV files (*.csv)"), &selectedFilter);
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

    // study
    Study *study = Agros::problem()->studies()->items().at(cmbStudies->currentIndex());

    if (study)
    {
        QList<ComputationSet> computationSets = study->computationSets(study->value(Study::View_Filter).toString());

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
}

void OptiLabWidget::studyChanged(int index)
{
    trvComputations->blockSignals(true);

    // computations
    QString selectedItem = "";
    if (trvComputations->currentItem())
        selectedItem = trvComputations->currentItem()->data(0, Qt::UserRole).toString();

    trvComputations->clear();

    // study
    Study *study = Agros::problem()->studies()->items().at(cmbStudies->currentIndex());

    if (study)
    {
        txtFilter->setText(study->value(Study::View_Filter).toString());
        double min = selectedItem.isEmpty() ? numeric_limits<double>::max() : 0.0;

        // fill tree view
        QList<ComputationSet> computationSets = study->computationSets(study->value(Study::View_Filter).toString());
        for (int i = 0; i < computationSets.size(); i++)
        {
            QTreeWidgetItem *itemComputationSet = new QTreeWidgetItem(trvComputations);
            itemComputationSet->setIcon(0, (computationSets[i].name().size() > 0) ? iconAlphabet(computationSets[i].name().at(0), AlphabetColor_Brown) : QIcon());
            // itemComputationSet->setCheckState(0, Qt::Checked);
            if (i == computationSets.size() - 1)
                itemComputationSet->setExpanded(true);

            int currentComputationSetCount = 0;
            foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(itemComputationSet);
                item->setText(0, computation->problemDir());
                item->setText(1, QString("%1 / %2").arg(computation->isSolved() ? tr("solved") : tr("not solved")).arg(computation->results()->hasResults() ? tr("results") : tr("no results")));
                item->setData(0, Qt::UserRole, computation->problemDir());

                currentComputationSetCount++;

                // select minimum
                double localMin = study->evaluateSingleGoal(computation);
                if (localMin < min)
                {
                    min = localMin;
                    selectedItem = computation->problemDir();
                }
            }

            itemComputationSet->setText(0, tr("%1 (%2 computations)").arg(computationSets[i].name()).arg(currentComputationSetCount));
        }

        // set stats
        int computationCount = 0;
        foreach (ComputationSet computationSet, computationSets)
            computationCount += computationSet.computations().count();

        lblNumberOfComputations->setText(QString::number(computationCount));
    }
    // set study to optilab view
    m_optilab->setStudy(study);

    trvComputations->blockSignals(false);

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
    Study *study = Agros::problem()->studies()->items().at(cmbStudies->currentIndex());

    LogOptimizationDialog *log = new LogOptimizationDialog(study);
    log->show();

    // solve
    study->solve();

    // image
    /*
    QDateTime currentTime(QDateTime::currentDateTime());
    QString fn = QString("%1/log/%2.png").arg(tempProblemDir()).arg(currentTime.toString("yyyy-MM-dd-hh-mm-ss-zzz"));

    const int width = 650;
    const int height = 400;

    totalChart->savePng(fn, width, height);
    Agros::log()->appendImage(fn);
    */

    // close dialog
    log->closeLog();

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
                if (!key.isEmpty() && Agros::computations().contains(key))
                    emit chartRefreshed(Agros::computations()[key]->problemDir());

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
        if (!key.isEmpty() && Agros::computations().contains(key))
        {
            Study *study = Agros::problem()->studies()->items().at(cmbStudies->currentIndex());
            study->removeComputation(Agros::computations()[key]);

            refresh();
        }
    }
}

void OptiLabWidget::doComputationSolve(bool)
{
    if (trvComputations->currentItem())
    {
        QString key = trvComputations->currentItem()->data(0, Qt::UserRole).toString();
        if (!key.isEmpty() && Agros::computations().contains(key))
        {
            SolveThread *solveThread = new SolveThread(Agros::computations()[key].data());
            solveThread->startCalculation();
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
    actSceneModeOptiLab = new QAction(icon("main_optilab"), tr("OptiLab"), this);
    actSceneModeOptiLab->setShortcut(Qt::Key_F8);
    actSceneModeOptiLab->setCheckable(true);

    m_optiLabWidget = new OptiLabWidget(this);

    createControls();

    // connect(Agros::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));

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

    // clear selection
    if (m_study->computationSets().count() == 0)
        doComputationSelected("");

    doChartRefreshed("");
    geometryViewer->doZoomBestFit();
}

QWidget *OptiLab::createControlsDistChart()
{
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
    layoutDetails->addLayout(layoutStatistics, 0);

    QWidget *widDist = new QWidget();
    widDist->setContentsMargins(0, 0, 0, 0);
    widDist->setLayout(layoutDetails);

    return widDist;
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

    chart = new QCustomPlot(this);
    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    chart->setContextMenuPolicy(Qt::CustomContextMenu);
    chart->legend->setVisible(true);
    connect(chart, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(chartContextMenu(const QPoint &)));
    connect(chart, SIGNAL(plottableClick(QCPAbstractPlottable*, int, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, int, QMouseEvent*)));
    connect(chart, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(graphMouseDoubleClick(QMouseEvent*)));

    // chart trend line
    chartTrendLine = new QCPItemLine(chart);
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

    // Pareto front
    chartGraphParetoFront = chart->addGraph();
    // chartGraphParetoFront->setName(tr("Pareto front"));
    // chartGraphParetoFront->addToLegend();
    chartGraphParetoFront->removeFromLegend();
    chartGraphParetoFront->setPen(QPen(QBrush(QColor(150, 110, 110, 120)), 8, Qt::SolidLine));
    chartGraphParetoFront->setLineStyle(QCPGraph::lsLine);

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
    actSceneModeOptiLab->setEnabled(!Agros::problem()->studies()->items().isEmpty());

    optiLabWidget()->refresh();
}

void OptiLab::chartContextMenu(const QPoint &pos)
{
    mnuChart->exec(QCursor::pos());
}

void OptiLab::chartRescale(bool checked)
{
    chart->rescaleAxes();
    chart->replot(QCustomPlot::rpQueuedRefresh);
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
    chart->replot(QCustomPlot::rpQueuedRefresh);
}

void OptiLab::chartShowAverageValue(bool checked)
{
    m_study->setValue(Study::View_ChartShowAverageValue, checked);
    actChartShowAverageValue->setChecked(checked);

    chartGraphAverageValue->setVisible(checked);
    chartGraphAverageValueChannelLower->setVisible(checked);
    chartGraphAverageValueChannelUpper->setVisible(checked);

    chart->replot(QCustomPlot::rpQueuedRefresh);
}

void OptiLab::chartShowParetoFront(bool checked)
{
    m_study->setValue(Study::View_ChartShowParetoFront, checked);
    actChartParetoFront->setChecked(checked);

    chartGraphParetoFront->setVisible(checked);
    chart->replot(QCustomPlot::rpQueuedRefresh);
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
        QMap<QString, QSharedPointer<Computation> > computations = Agros::computations();
        if (computations.count() > 0)
        {
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

            QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
            foreach (Parameter parameter, m_study->parameters())
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
            doChartRefreshed(key);

            // select item
            if (selectedItem)
                trvResults->setCurrentItem(selectedItem);
        }
    }
}

void OptiLab::doChartRefreshed(const QString &key)
{
    QList<ComputationSet> computationSets = m_study->computationSets(m_study->value(Study::View_Filter).toString());

    m_computationMap.clear();
    chartGraphSelectedComputation->data()->clear();

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
    QList<QSharedPointer<Computation> > paretoFront = m_study->nondominatedSort(computationSets);
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
            if (computation->problemDir() == key)
            {
                QVector<double> selectedX; selectedX << dataSetX.last();
                QVector<double> selectedY; selectedY << dataSetY.last();

                chartGraphSelectedComputation->setData(selectedX, selectedY);
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

        QCPGraph *chartGraphChart = chart->addGraph();
        chartGraphCharts.append(chartGraphChart);
        chartGraphChart->setData(dataSetX, dataSetY);
        chartGraphChart->setProperty("computationset_index", i);
        chartGraphChart->setName(computationSets[i].name());
        chartGraphChart->addToLegend();
        chartGraphChart->setLineStyle(QCPGraph::lsNone);
        chartGraphChart->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(140, 140, 140), colorFill, 9));
    }

    // Pareto front
    chartGraphParetoFront->setData(paretoDataX, paretoDataY);

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
    chart->replot(QCustomPlot::rpQueuedRefresh);

    // set actions
    chartLogHorizontal(m_study->value(Study::View_ChartLogHorizontal).toBool());
    chartLogHorizontal(m_study->value(Study::View_ChartLogVertical).toBool());
    chartShowTrend(m_study->value(Study::View_ChartShowTrend).toBool());
    chartShowAverageValue(m_study->value(Study::View_ChartShowAverageValue).toBool());
    chartShowParetoFront(m_study->value(Study::View_ChartShowParetoFront).toBool());
}

QPair<double, double> OptiLab::findClosestData(QCPGraph *graph, const Point &pos)
{    
    // find min and max
    RectPoint bound;
    bound.start.x = numeric_limits<double>::max();
    bound.end.x = -numeric_limits<double>::max();
    bound.start.y = numeric_limits<double>::max();
    bound.end.y = -numeric_limits<double>::max();

    for (QCPGraphDataContainer::const_iterator it = graph->data()->begin(); it != graph->data()->end(); ++it)
    {
        if (it->key < bound.start.x) bound.start.x = it->key;
        if (it->key > bound.end.x) bound.end.x = it->key;
        if (it->value < bound.start.y) bound.start.y = it->value;
        if (it->value > bound.end.y) bound.end.y = it->value;
    }

    // find closest point
    QPair<double, double> selectedData;
    double dist = numeric_limits<double>::max();
    for (QCPGraphDataContainer::const_iterator it = graph->data()->begin(); it != graph->data()->end(); ++it)
    {
        double mag = Point((it->key - pos.x) / bound.width(),
                           (it->value - pos.y) / bound.height()).magnitudeSquared();

        if (mag <= dist)
        {
            dist = mag;
            selectedData.first = it->key;
            selectedData.second = it->value;
        }
    }

    return selectedData;
}

void OptiLab::graphClicked(QCPAbstractPlottable *plottable, int code, QMouseEvent *event)
{
    if (QCPGraph *graph = dynamic_cast<QCPGraph *>(plottable))
    {
        // find closest point
        QPair<double, double> selectedData = findClosestData(graph, Point(chart->xAxis->pixelToCoord(event->pos().x()),
                                                                          chart->yAxis->pixelToCoord(event->pos().y())));

        QSharedPointer<Computation> selectedComputation = m_computationMap[graph->property("computationset_index").toInt()][selectedData];
        if (!selectedComputation.isNull())
        {
            QToolTip::hideText();
            QToolTip::showText(event->globalPos(),
                               tr("<table>"
                                  "<tr><th colspan=\"2\">%L1</th></tr>"
                                  "<tr><td>X:</td>" "<td>%L2</td></tr>"
                                  "<tr><td>Y:</td>" "<td>%L3</td></tr>"
                                  "</table>").
                               arg(graph->name().isEmpty() ? "..." : graph->name()).arg(selectedData.first).arg(selectedData.second), chart, chart->rect());

            emit computationSelected(selectedComputation->problemDir());
        }
    }
}

void OptiLab::graphMouseDoubleClick(QMouseEvent *event)
{
    // rescale chart
    if (event->buttons() & Qt::MiddleButton)
    {
        chartRescale(true);
        return;
    }
}

void OptiLab::doResultChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    QList<ComputationSet> computationSets = m_study->computationSets(m_study->value(Study::View_Filter).toString());

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

    if (showStats)
    {
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
        QSharedPointer<Computation> selectedComputation = m_study->findExtreme(type, key, minimum);

        if (!selectedComputation.isNull())
            emit computationSelected(selectedComputation->problemDir());
    }
}

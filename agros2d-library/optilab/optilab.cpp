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
#include "study_sweep.h"
#include "study_nsga2.h"
#include "study_bayesopt.h"
#include "study_nlopt.h"
#include "util/global.h"

#include "sceneview_data.h"
#include "logview.h"

#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"

#include "gui/infowidget.h"

int linreg(QVector<double> x, QVector<double> y, double *m, double *b, double *r)
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
        *m = 0;
        *b = 0;
        if (r) *r = 0;
        return 1;
    }

    // slope
    *m = (n * sumxy  -  sumx * sumy) / denom;
    // shift
    *b = (sumy * sumx2  -  sumx * sumxy) / denom;

    // correlation coef
    if (r)
        *r = (sumxy - sumx * sumy / n) / sqrt((sumx2 - sumx*sumx/n) * (sumy2 - sumy*sumy/n));

    return 0;
}

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent), m_optilab(parent)
{
    createControls();

    actRunStudy = new QAction(icon("run"), tr("Run study"), this);
    actRunStudy->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actRunStudy, SIGNAL(triggered()), this, SLOT(solveStudy()));

    connect(Agros2D::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(Agros2D::problem()->scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
}

OptiLabWidget::~OptiLabWidget()
{

}

void OptiLabWidget::createControls()
{
    cmbStudies = new QComboBox(this);
    connect(cmbStudies, SIGNAL(currentIndexChanged(int)), this, SLOT(studyChanged(int)));

    QGridLayout *layoutStudies = new QGridLayout();
    layoutStudies->addWidget(new QLabel(tr("Studies:")), 0, 0);
    layoutStudies->addWidget(cmbStudies, 0, 1);

    QWidget *widgetStudies = new QWidget(this);
    widgetStudies->setLayout(layoutStudies);

    // parameters
    trvComputations = new QTreeWidget(this);
    trvComputations->setMouseTracking(true);
    trvComputations->setColumnCount(2);
    trvComputations->setMinimumWidth(220);
    trvComputations->setColumnWidth(0, 220);

    QStringList headers;
    headers << tr("Computation") << tr("State");
    trvComputations->setHeaderLabels(headers);

    connect(trvComputations, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doComputationChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    cmbChartX = new QComboBox(this);
    cmbChartY = new QComboBox(this);
    chkChartLogX = new QCheckBox(tr("Logarithmic scale (x-axis)"), this);
    chkChartLogY = new QCheckBox(tr("Logarithmic scale (y-axis)"), this);

    QGridLayout *layoutChartXYControls = new QGridLayout();
    layoutChartXYControls->addWidget(chkChartLogX, 1, 0);
    layoutChartXYControls->addWidget(chkChartLogY, 2, 0);
    layoutChartXYControls->addWidget(new QLabel(tr("Variable X:")), 5, 0);
    layoutChartXYControls->addWidget(cmbChartX, 5, 1);
    layoutChartXYControls->addWidget(new QLabel(tr("Variable Y:")), 6, 0);
    layoutChartXYControls->addWidget(cmbChartY, 6, 1);

    btnPlotChart = new QPushButton(tr("Apply"));
    connect(btnPlotChart, SIGNAL(clicked()), this, SLOT(plotChart()));

    QHBoxLayout *layoutParametersButton = new QHBoxLayout();
    layoutParametersButton->addStretch();
    layoutParametersButton->addWidget(btnPlotChart);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(widgetStudies);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutChartXYControls);
    layout->addLayout(layoutParametersButton);

    setLayout(layout);
}

void OptiLabWidget::refresh()
{
    emit chartRefreshed(nullptr, QSharedPointer<Computation>());

    actRunStudy->setEnabled(false);
    btnPlotChart->setEnabled(false);

    // fill studies
    cmbStudies->blockSignals(true);

    QString selectedItem = "";
    if (cmbStudies->currentIndex() != -1)
        selectedItem = cmbStudies->currentText();

    cmbStudies->clear();
    trvComputations->clear();
    foreach (Study *study, Agros2D::problem()->studies()->items())
    {
        if (study->name().isEmpty())
            cmbStudies->addItem(studyTypeString(study->type()));
        else
            cmbStudies->addItem(study->name());

    }

    cmbStudies->blockSignals(false);

    if (cmbStudies->count() > 0)
    {
        if (!selectedItem.isEmpty())
            cmbStudies->setCurrentText(selectedItem);
        else
            cmbStudies->setCurrentIndex(0);

        studyChanged(cmbStudies->currentIndex());
    }

    // fill combo boxes
    cmbChartX->clear();
    cmbChartY->clear();

    // TODO: chart - first iteration
    if (cmbStudies->currentIndex() != -1)
    {
        Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

        // step
        cmbChartX->addItem(tr("step"), QString("step:"));
        cmbChartY->addItem(tr("step"), QString("step:"));

        // parameters
        foreach (Parameter parameter, study->parameters())
        {
            cmbChartX->addItem(tr("%1 (parameter)").arg(parameter.name()), QString("parameter:%1").arg(parameter.name()));
            cmbChartY->addItem(tr("%1 (parameter)").arg(parameter.name()), QString("parameter:%1").arg(parameter.name()));
        }

        // functionals
        foreach (Functional functional, study->functionals())
        {
            cmbChartX->addItem(tr("%1 (functional)").arg(functional.name()), QString("functional:%1").arg(functional.name()));
            cmbChartY->addItem(tr("%1 (functional)").arg(functional.name()), QString("functional:%1").arg(functional.name()));
        }

        // recipes
        foreach (ResultRecipe *recipe, Agros2D::problem()->recipes()->items())
        {
            cmbChartX->addItem(tr("%1 (recipe)").arg(recipe->name()), QString("recipe:%1").arg(recipe->name()));
            cmbChartY->addItem(tr("%1 (recipe)").arg(recipe->name()), QString("recipe:%1").arg(recipe->name()));
        }
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
        QTreeWidgetItem *itemComputationSet = new QTreeWidgetItem(trvComputations);

        QList<ComputationSet> computationSets = study->computationSets();

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

    // select current computation
    if (!selectedItem.isEmpty())
        doComputationSelected(selectedItem);

    // if not selected -> select first
    if (trvComputations->topLevelItemCount() > 0 && !trvComputations->currentItem())
        trvComputations->setCurrentItem(trvComputations->topLevelItem(0));

    // set controls
    chkChartLogX->setChecked(study->value(Study::View_ChartLogX).toBool());
    chkChartLogY->setChecked(study->value(Study::View_ChartLogY).toBool());
    cmbChartX->setCurrentIndex(cmbChartX->findData(study->value(Study::View_ChartX).toString()));
    cmbChartY->setCurrentIndex(cmbChartY->findData(study->value(Study::View_ChartY).toString()));

    // enable buttons
    actRunStudy->setEnabled(true);
    btnPlotChart->setEnabled(true);
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

void OptiLabWidget::plotChart()
{
    if (cmbStudies->count() == 0)
        return;

    // study
    Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

    study->setValue(Study::View_ChartX, cmbChartX->currentData().toString());
    study->setValue(Study::View_ChartY, cmbChartY->currentData().toString());
    study->setValue(Study::View_ChartLogX, chkChartLogX->checkState() == Qt::Checked);
    study->setValue(Study::View_ChartLogY, chkChartLogY->checkState() == Qt::Checked);

    QSharedPointer<Computation> computation;
    if (trvComputations->currentItem())
    {
        computation = Agros2D::computations()[trvComputations->currentItem()->data(0, Qt::UserRole).toString()];
    }

    emit chartRefreshed(study, computation);
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
        emit computationSelected(key);

        // study
        Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());
        emit chartRefreshed(study, Agros2D::computations()[key]);
    }
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
    connect(m_optiLabWidget, SIGNAL(chartRefreshed(Study *, QSharedPointer<Computation>)), this, SLOT(doChartRefreshed(Study *, QSharedPointer<Computation>)));
    connect(this, SIGNAL(computationSelected(QString)), m_optiLabWidget, SLOT(doComputationSelected(QString)));
}

OptiLab::~OptiLab()
{    
}

void OptiLab::createControls()
{
    m_infoWidget = new InfoWidgetGeneral(this);

    actRescale = new QAction(icon(""), tr("Rescale chart"), this);
    connect(actRescale, SIGNAL(triggered(bool)), this, SLOT(chartRescale(bool)));

    mnuChart = new QMenu(this);
    mnuChart->addAction(actRescale);

    chart = new QCustomPlot(this);
    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    chart->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(chart, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(chartContextMenu(const QPoint &)));
    connect(chart, SIGNAL(plottableClick(QCPAbstractPlottable*, QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*, QMouseEvent*)));

    QHBoxLayout *layoutLab = new QHBoxLayout();
    layoutLab->addWidget(m_infoWidget, 1);
    layoutLab->addWidget(chart, 1);

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

void OptiLab::doComputationSelected(const QString &key)
{
    if (key.isEmpty())
    {
        m_infoWidget->clear();
    }
    else
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[key];

        m_infoWidget->showProblemInfo(computation.data());
    }
}

void OptiLab::doChartRefreshed(Study *study, QSharedPointer<Computation> selectedComputation)
{
    m_study = study;

    chart->clearGraphs();
    chart->clearItems();

    m_computationMap.clear();

    if (!m_study)
        return;

    QString chartX = m_study->value(Study::View_ChartX).toString();
    QString chartY = m_study->value(Study::View_ChartY).toString();

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
            if (computation == selectedComputation)
            {
                QVector<double> selectedX; selectedX << dataSetX.last();
                QVector<double> selectedY; selectedY << dataSetY.last();
                QCPGraph *graph = chart->addGraph();
                graph->setData(selectedX, selectedY);
                graph->removeFromLegend();

                graph->setLineStyle(QCPGraph::lsNone);
                graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, Qt::darkGray, 17));
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

        QCPGraph *graph = chart->addGraph();
        graph->setData(dataSetX, dataSetY);
        graph->setProperty("computationset_index", i);
        graph->setName(m_study->computationSets()[i].name());
        graph->addToLegend();

        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, QColor(140, 140, 140), colorFill, 9));
    }

    // bounding box
    auto mmx = std::minmax_element(linRegDataX.begin(), linRegDataX.end());
    auto mmy = std::minmax_element(linRegDataY.begin(), linRegDataY.end());
    double delta = qMin((*mmx.second - *mmx.first) * 0.1, (*mmy.second - *mmy.first) * 0.1);
    QPointF boundStart(*mmx.first - delta, *mmy.first - delta);
    QPointF boundEnd(*mmx.second + delta, *mmy.second + delta);

    // QCPItemRect *boundRect = new QCPItemRect(chart);
    // chart->addItem(boundRect);
    // boundRect->topLeft->setCoords(boundStart);
    // boundRect->bottomRight->setCoords(boundEnd);

    // linear regression
    double m = 0.0;
    double b = 0.0;
    double r = 0.0;
    if (linreg(linRegDataX, linRegDataY, &m, &b, &r) == 0)
    {
        // trend arrow
        QPointF start((*mmx.first - delta), (*mmx.first - delta) * m + b);
        QPointF end((*mmx.second + delta), (*mmx.second + delta) * m + b);

        QCPItemLine *arrow = new QCPItemLine(chart);
        chart->addItem(arrow);
        arrow->start->setCoords(start);
        arrow->end->setCoords(end);
        arrow->setHead(QCPLineEnding::esSpikeArrow);
        arrow->setPen(QPen(QBrush(QColor(200, 200, 200)), 5));
    }

    // mean value and standard deviation
    double sum = std::accumulate(linRegDataY.begin(), linRegDataY.end(), 0.0);
    double mean = sum / linRegDataY.size();

    std::vector<double> diff(linRegDataY.size());
    std::transform(linRegDataY.begin(), linRegDataY.end(), diff.begin(), [mean](double x) { return x - mean; });
    double sqSum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdev = std::sqrt(sqSum / linRegDataY.size());

    QVector<double> meanX; meanX << *mmx.first - delta << *mmx.second + delta;
    QVector<double> meanY; meanY << mean << mean;
    QVector<double> devUpper; devUpper << mean + stdev << mean + stdev;
    QVector<double> devLower; devLower << mean - stdev << mean - stdev;
    QCPGraph *graph = chart->addGraph();
    graph->setData(meanX, meanY);
    graph->removeFromLegend();

    graph->setPen(QPen(QBrush(QColor(140, 140, 140)), 3, Qt::DashLine));
    // graph->setBrush(QBrush(Qt::red));
    graph->setLineStyle(QCPGraph::lsLine);

    QCPGraph *graphChannelLower = chart->addGraph();
    graphChannelLower->removeFromLegend();
    graphChannelLower->setData(meanX, devLower);
    graphChannelLower->setPen(QPen(QBrush(QColor(180, 180, 180)), 1, Qt::DotLine));

    QCPGraph *graphChannelUpper = chart->addGraph();
    graphChannelUpper->removeFromLegend();
    graphChannelUpper->setData(meanX, devUpper);
    graphChannelUpper->setPen(QPen(QBrush(QColor(180, 180, 180)), 1, Qt::DotLine));
    graphChannelUpper->setBrush(QBrush(QColor(255, 50, 30, 10)));
    graphChannelUpper->setChannelFillGraph(graphChannelLower);

    // plot main chart
    chart->xAxis->setLabel(labelX);
    chart->yAxis->setLabel(labelY);
    chart->legend->setVisible(true);
    if (m_study->value(Study::View_ChartLogX).toBool())
        chart->xAxis->setScaleType(QCPAxis::stLogarithmic);
    else
        chart->xAxis->setScaleType(QCPAxis::stLinear);
    if (study->value(Study::View_ChartLogY).toBool())
        chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    else
        chart->yAxis->setScaleType(QCPAxis::stLinear);

    // replot chart
    chart->replot(QCustomPlot::rpQueued);
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

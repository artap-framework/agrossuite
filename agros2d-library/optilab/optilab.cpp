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
#include "study_genetic.h"
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
#include "qcustomplot.h"

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent), m_optilab(parent)
{
    createControls();

    connect(Agros2D::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));
}

OptiLabWidget::~OptiLabWidget()
{

}

void OptiLabWidget::createControls()
{
    cmbStudies = new QComboBox(this);
    connect(cmbStudies, SIGNAL(currentIndexChanged(int)), this, SLOT(studyChanged(int)));

    QHBoxLayout *layoutStudies = new QHBoxLayout();
    layoutStudies->addWidget(new QLabel(tr("Studies")));
    layoutStudies->addWidget(cmbStudies);

    // parameters
    trvComputations = new QTreeWidget(this);
    trvComputations->setMouseTracking(true);
    trvComputations->setColumnCount(2);
    trvComputations->setMinimumWidth(220);
    trvComputations->setColumnWidth(0, 220);

    QStringList headers;
    headers << tr("Computation") << tr("State");
    trvComputations->setHeaderLabels(headers);

    connect(trvComputations, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(computationChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    cmbChartX = new QComboBox(this);
    cmbChartY = new QComboBox(this);
    chkShowAllSets = new QCheckBox(tr("Apply to all sets"), this);
    chkChartLogY = new QCheckBox(tr("Logarithmic scale"), this);

    QGridLayout *layoutChartXYControls = new QGridLayout();
    layoutChartXYControls->addWidget(chkShowAllSets, 0, 0);
    layoutChartXYControls->addWidget(chkChartLogY, 1, 0);
    layoutChartXYControls->addWidget(new QLabel(tr("Variable X:")), 2, 0);
    layoutChartXYControls->addWidget(cmbChartX, 2, 1);
    layoutChartXYControls->addWidget(new QLabel(tr("Variable Y:")), 3, 0);
    layoutChartXYControls->addWidget(cmbChartY, 3, 1);

    btnSolveStudy = new QPushButton(tr("Solve"));
    connect(btnSolveStudy, SIGNAL(clicked()), this, SLOT(solveStudy()));
    btnPlotChart = new QPushButton(tr("Plot chart"));
    connect(btnPlotChart, SIGNAL(clicked()), this, SLOT(plotChart()));

    QHBoxLayout *layoutParametersButton = new QHBoxLayout();
    layoutParametersButton->addWidget(btnSolveStudy);
    layoutParametersButton->addStretch();
    layoutParametersButton->addWidget(btnPlotChart);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addLayout(layoutStudies);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutChartXYControls);
    layout->addLayout(layoutParametersButton);

    setLayout(layout);
}

void OptiLabWidget::refresh()
{
    btnSolveStudy->setEnabled(false);
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

    // fill tree view
    study->fillTreeView(trvComputations);

    trvComputations->blockSignals(false);

    // select current computation
    if (!selectedItem.isEmpty())
        computationSelect(selectedItem);

    // if not selected -> select first
    if (trvComputations->topLevelItemCount() > 0 && !trvComputations->currentItem())
        trvComputations->setCurrentItem(trvComputations->topLevelItem(0));

    // set controls
    chkShowAllSets->setChecked(study->value(Study::View_ApplyToAllSets).toBool());
    cmbChartX->setCurrentIndex(cmbChartX->findData(study->value(Study::View_ChartX).toString()));
    cmbChartY->setCurrentIndex(cmbChartY->findData(study->value(Study::View_ChartY).toString()));

    // enable buttons
    btnSolveStudy->setEnabled(true);
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

    study->setValue(Study::View_ApplyToAllSets, chkShowAllSets->checkState() == Qt::Checked);
    study->setValue(Study::View_ChartX, cmbChartX->currentData().toString());
    study->setValue(Study::View_ChartY, cmbChartY->currentData().toString());
    study->setValue(Study::View_ChartLogY, chkChartLogY->checkState() == Qt::Checked);

    emit chartRefreshed(study);
}

/*
void OptiLabWidget::testSweep()
{
    // sweep analysis
    StudySweepAnalysis *analysis = new StudySweepAnalysis();

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // result recipes
    LocalValueRecipe *localValueRecipe = new LocalValueRecipe("V", "electrostatic", "electrostatic_potential");
    localValueRecipe->setPoint(0.02, 0.05);
    Agros2D::problem()->recipes()->addRecipe(localValueRecipe);

    SurfaceIntegralRecipe *surfaceIntegralRecipe = new SurfaceIntegralRecipe("Q", "electrostatic", "electrostatic_charge");
    surfaceIntegralRecipe->addEdge(1);
    surfaceIntegralRecipe->addEdge(12);
    Agros2D::problem()->recipes()->addRecipe(surfaceIntegralRecipe);

    VolumeIntegralRecipe *volumeIntegralRecipe = new VolumeIntegralRecipe("We", "electrostatic", "electrostatic_energy");
    Agros2D::problem()->recipes()->addRecipe(volumeIntegralRecipe);

    // parameters
    QList<double> params; params << 0.005 << 0.01 << 0.015;
    analysis->addParameter(Parameter::fromList("R1", params));
    //analysis->addParameter(Parameter::fromRandom("R2", 4, 0.05, 0.07));
    //analysis->addParameter(Parameter::fromLinspace("R3", 3, 0.05, 0.07));
    //analysis->addParameter(Parameter::fromRandom("C", 10, 1, 5));

    // functionals
    analysis->addFunctional(Functional("C", FunctionalType_Result, "2*We/U**2"));

    // solve
    analysis->solve();

    refresh();
}
*/

void OptiLabWidget::computationSelect(const QString &key)
{

}

void OptiLabWidget::computationChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    if (trvComputations->currentItem())
    {
        QString key = trvComputations->currentItem()->data(0, Qt::UserRole).toString();
        emit computationSelected(key);
    }
}

OptiLab::OptiLab(QWidget *parent) : QWidget(parent)
{
    actSceneModeOptiLab = new QAction(icon("optilab"), tr("OptiLab"), this);
    actSceneModeOptiLab->setShortcut(Qt::Key_F8);
    actSceneModeOptiLab->setCheckable(true);

    m_optiLabWidget = new OptiLabWidget(this);

    createControls();

    connect(m_optiLabWidget, SIGNAL(computationSelected(QString)), this, SLOT(computationSelected(QString)));
    connect(m_optiLabWidget, SIGNAL(chartRefreshed(Study *)), this, SLOT(chartRefreshed(Study *)));
}

OptiLab::~OptiLab()
{    
}

void OptiLab::createControls()
{
    m_infoWidget = new InfoWidgetGeneral(this);

    m_chart = new QCustomPlot(this);
    m_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QHBoxLayout *layoutLab = new QHBoxLayout();
    layoutLab->addWidget(m_infoWidget, 1);
    layoutLab->addWidget(m_chart, 1);

    setLayout(layoutLab);
}

void OptiLab::computationSelected(const QString &key)
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

void OptiLab::chartRefreshed(Study *study)
{
    QString chartX = study->value(Study::View_ChartX).toString();
    QString chartY = study->value(Study::View_ChartY).toString();

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

    m_chart->clearGraphs();

    int paletteStep = study->computationSets().count() > 1 ? (PALETTEENTRIES - 1) / (study->computationSets().count() - 1) : 0;
    int step = 0;
    for (int i = 0; i < study->computationSets().count(); i++)
    {
        QVector<double> dataX;
        QVector<double> dataY;

        QList<QSharedPointer<Computation> > computations = study->computationSets()[i].computations();

        for (int j = 0; j < computations.count(); j++)
        {
            QSharedPointer<Computation> computation = computations[j];

            step++;

            // steps
            if (chartX.contains("step:"))
            {
                dataX.append(step);
            }
            if (chartY.contains("step:"))
            {
                dataY.append(step);
            }

            // parameters
            if (chartX.contains("parameter:"))
            {
                QString name = chartX.right(chartX.count() - 10);
                double value = computation->config()->parameter(name);
                dataX.append(value);
            }
            if (chartY.contains("parameter:"))
            {
                QString name = chartY.right(chartY.count() - 10);
                double value = computation->config()->parameter(name);
                dataY.append(value);
            }

            // functionals
            if (chartX.contains("functional:"))
            {
                QString name = chartX.right(chartX.count() - 11);
                double value = computation->results()->resultValue(name);
                dataX.append(value);
            }
            if (chartY.contains("functional:"))
            {
                QString name = chartY.right(chartY.count() - 11);
                double value = computation->results()->resultValue(name);
                dataY.append(value);
            }

            // recipes
            if (chartX.contains("recipe:"))
            {
                QString name = chartX.right(chartX.count() - 7);
                double value = computation->results()->resultValue(name);
                dataX.append(value);
            }
            if (chartY.contains("recipe:"))
            {
                QString name = chartY.right(chartY.count() - 7);
                double value = computation->results()->resultValue(name);
                dataY.append(value);
            }
        }

        const QColor color(paletteDataAgros[i * paletteStep][0] * 255, paletteDataAgros[i * paletteStep][1] * 255, paletteDataAgros[i * paletteStep][2] * 255);

        QCPGraph *graph = m_chart->addGraph();
        graph->setData(dataX, dataY);
        graph->setName(study->computationSets()[i].name());
        graph->addToLegend();

        graph->setLineStyle(QCPGraph::lsNone);
        QCPScatterStyle scatterStyle;
        scatterStyle.setSize(8);
        scatterStyle.setShape(QCPScatterStyle::ssDisc);
        scatterStyle.setPen(QPen(color));
        graph->setScatterStyle(scatterStyle);
    }

    m_chart->xAxis->setLabel(labelX);
    m_chart->yAxis->setLabel(labelY);
    m_chart->legend->setVisible(true);
    if (study->value(Study::View_ChartLogY).toBool())
        m_chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    else
        m_chart->yAxis->setScaleType(QCPAxis::stLinear);
    m_chart->rescaleAxes();
    m_chart->replot(QCustomPlot::rpQueued);
}

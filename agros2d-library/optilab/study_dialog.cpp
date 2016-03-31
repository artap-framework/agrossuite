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

#include "study.h"
#include "study_dialog.h"
#include "util/global.h"
#include "gui/lineeditdouble.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "study_sweep.h"
#include "study_nsga2.h"
#include "study_nsga3.h"
#include "study_nlopt.h"
#include "study_bayesopt.h"

#include "qcustomplot/qcustomplot.h"

double logVal(double val)
{
    return (val > 0) ? log10(fabs(val)) : -log10(fabs(val));
}

LogOptimizationDialog::LogOptimizationDialog(Study *study) : QDialog(QApplication::activeWindow()),
    m_study(study), m_progress(nullptr), m_computationSetsCount(1)
{
    setModal(true);
    
    setWindowIcon(icon("run"));
    setWindowTitle(studyTypeString(study->type()));
    setAttribute(Qt::WA_DeleteOnClose);
    
    createControls();
    
    connect(btnAbort, SIGNAL(clicked()), m_study, SLOT(doAbortSolve()));
    connect(btnAbort, SIGNAL(clicked()), this, SLOT(aborted()));
    connect(m_study, SIGNAL(updateChart(int, QList<double>, double, SolutionUncertainty)), this, SLOT(updateChart(int, QList<double>, double, SolutionUncertainty)));
    connect(m_study, SIGNAL(solved()), this, SLOT(solved()));
    
    int w = 2.0/3.0 * QApplication::desktop()->screenGeometry().width();
    int h = 2.0/3.0 * QApplication::desktop()->screenGeometry().height();
    
    setMinimumSize(w, h);
    setMaximumSize(w, h);
    
    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

LogOptimizationDialog::~LogOptimizationDialog()
{
}

void LogOptimizationDialog::closeEvent(QCloseEvent *e)
{
    if (m_study->isSolving())
        e->ignore();
}

void LogOptimizationDialog::reject()
{
    if (m_study->isSolving())
        m_study->doAbortSolve();
    else
        close();
}

void LogOptimizationDialog::tryClose()
{
    if (m_study->isSolving())
    {
        Agros2D::log()->printError(tr("Solver"), tr("Stydy is being aborted."));
    }
    else
    {
        close();
    }
}

void LogOptimizationDialog::createControls()
{
    // m_logWidget = new LogWidget(this);
    
#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif
    
    QFont fontProgress = font();
    fontProgress.setPointSize(fontSize);
    
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(tryClose()));
    btnClose->setEnabled(false);
    
    btnAbort = new QPushButton(tr("Abort"));
    
    QHBoxLayout *layoutStatus = new QHBoxLayout();
    layoutStatus->addStretch();
    layoutStatus->addWidget(btnAbort, 0, Qt::AlignRight);
    layoutStatus->addWidget(btnClose, 0, Qt::AlignRight);
    
    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setWidth(2);
    
    QPen penError;
    penError.setColor(Qt::darkRed);
    penError.setWidth(2);
    
    QFont fontTitle(font());
    fontTitle.setBold(true);
    
    QFont fontChart(font());
    fontChart.setPointSize(fontSize);
    
    // objective functions
    QVBoxLayout *layoutCharts = nullptr;
    if (m_study->functionals().count() > 1)
    {
        layoutCharts = new QVBoxLayout();
        foreach (Functional functional, m_study->functionals())
        {
            QCustomPlot *chart = new QCustomPlot(this);
            m_charts.append(chart);
            
            QCPPlotTitle *title = new QCPPlotTitle(chart, functional.name());
            title->setFont(fontTitle);
            chart->plotLayout()->insertRow(0);
            chart->plotLayout()->addElement(0, 0, title);
            
            chart->xAxis->setTickLabelFont(fontChart);
            chart->xAxis->setLabelFont(fontChart);
            // chart->xAxis->setTickStep(1.0);
            chart->xAxis->setAutoTickStep(true);
            chart->xAxis->setLabel(tr("number of steps"));
            
            // chart->yAxis->setScaleType(QCPAxis::stLogarithmic);
            chart->yAxis->setTickLabelFont(fontChart);
            chart->yAxis->setLabelFont(fontChart);
            chart->yAxis->setLabel(functional.name());
            
            QCPGraph *objectiveGraph = chart->addGraph(chart->xAxis, chart->yAxis);
            objectiveGraph->setLineStyle(QCPGraph::lsLine);
            objectiveGraph->setPen(pen);
            objectiveGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
            objectiveGraph->setName(functional.name());
            
            layoutCharts->addWidget(chart);
        }
    }

    // total objective function
    m_totalChart = new QCustomPlot(this);

    QCPPlotTitle *title = new QCPPlotTitle(m_totalChart, tr("Total objective function"));
    title->setFont(fontTitle);
    m_totalChart->plotLayout()->insertRow(0);
    m_totalChart->plotLayout()->addElement(0, 0, title);

    m_totalChart->xAxis->setTickLabelFont(fontChart);
    m_totalChart->xAxis->setLabelFont(fontChart);
    // chart->xAxis->setTickStep(1.0);
    m_totalChart->xAxis->setAutoTickStep(true);
    m_totalChart->xAxis->setLabel(tr("number of steps"));
    // m_totalChart->yAxis->setScaleType(QCPAxis::stLogarithmic);
    m_totalChart->yAxis->setTickLabelFont(fontChart);
    m_totalChart->yAxis->setLabelFont(fontChart);
    m_totalChart->yAxis->setLabel(tr("Objective"));
    m_totalChart->yAxis2->setLabel(tr("Uncertainty"));
    m_totalChart->yAxis2->setVisible(true);
    m_totalChart->legend->setVisible(true);

    QCPGraph *totalObjectiveGraph = m_totalChart->addGraph(m_totalChart->xAxis, m_totalChart->yAxis);
    totalObjectiveGraph->setLineStyle(QCPGraph::lsLine);
    totalObjectiveGraph->setPen(pen);
    totalObjectiveGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
    totalObjectiveGraph->setName(tr("Value"));
    totalObjectiveGraph->addToLegend();

    QCPGraph *totalObjectiveGraphLower = m_totalChart->addGraph();
    totalObjectiveGraphLower->removeFromLegend();
    totalObjectiveGraphLower->setPen(QPen(QBrush(QColor(180, 180, 180)), 1, Qt::DotLine));

    QCPGraph *totalObjectiveGraphUpper = m_totalChart->addGraph();
    totalObjectiveGraphUpper->setPen(QPen(QBrush(QColor(180, 180, 180)), 1, Qt::DotLine));
    totalObjectiveGraphUpper->setBrush(QBrush(QColor(255, 50, 30, 20)));
    totalObjectiveGraphUpper->setChannelFillGraph(totalObjectiveGraphLower);
    totalObjectiveGraphUpper->setName(tr("Bounds (std. dev.)"));
    totalObjectiveGraphUpper->addToLegend();

    QCPGraph *totalObjectiveUncertainty = m_totalChart->addGraph(m_totalChart->xAxis, m_totalChart->yAxis);
    totalObjectiveUncertainty->setLineStyle(QCPGraph::lsLine);
    totalObjectiveUncertainty->setPen(QPen(QBrush(QColor(30, 10, 20)), 2, Qt::DashLine));
    totalObjectiveUncertainty->setName(tr("Uncertainty"));
    totalObjectiveUncertainty->addToLegend();

    m_progress = new QProgressBar(this);
    m_progress->setMaximum(m_study->estimatedNumberOfSteps());
    
    QVBoxLayout *layoutObjective = new QVBoxLayout();
    layoutObjective->addWidget(m_totalChart, 3);
    if (layoutCharts) layoutObjective->addLayout(layoutCharts, 5);
    layoutObjective->addWidget(m_progress, 1);
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutObjective, 2);
    // layout->addWidget(m_logWidget, 1);
    // layout->addStretch();
    layout->addLayout(layoutStatus);
    
    setLayout(layout);
}

void LogOptimizationDialog::updateChart(int step, QList<double> values, double totalValue, SolutionUncertainty solutionUncertainty)
{
    int computationSetsCount = m_study->computationSets().count();

    // local objective functions
    if (values.count() > 1)
    {
        for (int i = 0; i < values.count(); i++)
        {
            QCustomPlot *chart = m_charts[i];

            // dashed line (populations)
            if (m_computationSetsCount < computationSetsCount)
            {
                QCPItemStraightLine *line = new QCPItemStraightLine(chart);
                chart->addItem(line);

                line->point1->setCoords(QPointF(step - 0.5, 0));
                line->point2->setCoords(QPointF(step - 0.5, 1));
                line->setPen(QPen(QBrush(QColor(140, 40, 40)), 2, Qt::DashLine));
            }

            chart->graph(0)->addData(step, values[i]);
            chart->rescaleAxes();
            chart->replot(QCustomPlot::rpImmediate);
        }
    }

    // total objective function
    m_totalChart->graph(0)->addData(step, (totalValue));
    if (fabs(solutionUncertainty.lowerBound) > 0.0 && fabs(solutionUncertainty.upperBound) > 0.0)
    {
        m_totalChart->legend->setVisible(true);
        m_totalChart->graph(1)->setVisible(true);
        m_totalChart->graph(2)->setVisible(true);
        m_totalChart->graph(3)->setVisible(true);

        m_totalChart->graph(1)->addData(step, (solutionUncertainty.lowerBound));
        m_totalChart->graph(2)->addData(step, (solutionUncertainty.upperBound));
        m_totalChart->graph(3)->addData(step, (solutionUncertainty.uncertainty));
    }
    else
    {
        m_totalChart->legend->setVisible(false);
        m_totalChart->graph(1)->setVisible(false);
        m_totalChart->graph(2)->setVisible(false);
        m_totalChart->graph(3)->setVisible(false);
    }

    // dashed line (populations)
    if (m_computationSetsCount < computationSetsCount)
    {
        QCPItemStraightLine *line = new QCPItemStraightLine(m_totalChart);
        m_totalChart->addItem(line);

        line->point1->setCoords(QPointF(step - 0.5, 0));
        line->point2->setCoords(QPointF(step - 0.5, 1));
        line->setPen(QPen(QBrush(QColor(140, 40, 40)), 2, Qt::DashLine));
    }

    m_totalChart->rescaleAxes();
    m_totalChart->replot(QCustomPlot::rpImmediate);
    
    m_computationSetsCount = m_study->computationSets().count();
    
    m_progress->setValue(step);
    QApplication::processEvents();
}

void LogOptimizationDialog::solved()
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);
    
    // TODO: move from GUI
    QDateTime currentTime(QDateTime::currentDateTime());
    QString fn = QString("%1/log/%2.png").arg(tempProblemDir()).arg(currentTime.toString("yyyy-MM-dd-hh-mm-ss-zzz"));

    const int width = 650;
    const int height = 400;

    m_totalChart->savePng(fn, width, height);
    Agros2D::log()->appendImage(fn);

    tryClose();
}

void LogOptimizationDialog::aborted()
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);
}

void LogOptimizationDialog::updateParameters(QList<Parameter> parameters, const Computation *computation)
{
    QString params = "";
    foreach (Parameter parameter, parameters)
    {
        params += QString("%1 = %2, ").arg(parameter.name()).arg(computation->config()->parameter(parameter.name()));
    }
    if (params.size() > 0)
        params = params.left(params.size() - 2);
    
    QString res = "";
    foreach (QString name, computation->results()->items().keys())
    {
        res += QString("%1 = %2, ").arg(name).arg(computation->results()->resultValue(name));
    }
    if (res.size() > 0)
        res = res.left(res.size() - 2);
    
    Agros2D::log()->printMessage(tr("Study"), tr("Parameters: %1, results: %2").arg(params).arg(res));
}

// ******************************************************************************************************************

StudySelectDialog::StudySelectDialog(QWidget *parent) : QDialog(parent), m_selectedStudyType(StudyType_Undefined)
{
    setWindowTitle(tr("Add study"));
    setModal(true);
    
    lstStudies = new QListWidget(this);
    lstStudies->setIconSize(QSize(24, 24));
    lstStudies->setMinimumHeight(28*10);
    
    foreach (QString name, studyTypeStringKeys())
    {
        QListWidgetItem *item = new QListWidgetItem(lstStudies);
        item->setIcon(iconAlphabet(studyTypeString(studyTypeFromStringKey(name)).at(0), AlphabetColor_Bluegray));
        item->setText(studyTypeString(studyTypeFromStringKey(name)));
        item->setData(Qt::UserRole, studyTypeFromStringKey(name));
        
        lstStudies->addItem(item);
    }
    
    connect(lstStudies, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstStudies, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstStudies, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(doItemSelected(QListWidgetItem *)));
    
    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstStudies);
    
    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);
    
    setLayout(layout);
    
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstStudies->count() > 0)
    {
        lstStudies->setCurrentRow(0);
        doItemSelected(lstStudies->currentItem());
    }
    
    int w = sizeHint().width() + 20;
    int h = 1.5/5.0 * QApplication::desktop()->screenGeometry().height();
    
    setMinimumSize(w, h);
    setMaximumSize(w, h);
    
    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

void StudySelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_selectedStudyType = (StudyType) item->data(Qt::UserRole).toInt();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void StudySelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstStudies->currentItem())
    {
        m_selectedStudyType = (StudyType) item->data(Qt::UserRole).toInt();
        accept();
    }
}

// *******************************************************************************************************************

StudyDialog *StudyDialog::factory(Study *study, QWidget *parent)
{    
    if (study->type() == StudyType_NSGA2)
        return new StudyNSGA2Dialog(study, parent);
    else if (study->type() == StudyType_NSGA3)
        return new StudyNSGA3Dialog(study, parent);
    else if (study->type() == StudyType_BayesOpt)
        return new StudyBayesOptDialog(study, parent);
    else if (study->type() == StudyType_NLopt)
        return new StudyNLoptDialog(study, parent);
    else if (study->type() == StudyType_Sweep)
        return new StudySweepDialog(study, parent);
    else
        assert(0);
    
    return nullptr;
}

StudyDialog::StudyDialog(Study *study, QWidget *parent) : QDialog(parent),
    m_study(study)
{
    setWindowTitle(tr("%1").arg(studyTypeString(study->type())));
    setAttribute(Qt::WA_DeleteOnClose);
}

int StudyDialog::showDialog()
{
    createControls();
    load();
    
    return exec();
}

void StudyDialog::createControls()
{
    // dialog buttons
    QPushButton *btnApply = new QPushButton(tr("OK"));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnApply->setDefault(true);
    
    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    
    QPushButton *btnDuplicate = new QPushButton(tr("Duplicate"));
    connect(btnDuplicate, SIGNAL(clicked()), this, SLOT(doDuplicate()));
    
    QHBoxLayout *layoutButtonBox = new QHBoxLayout();
    layoutButtonBox->addStretch();
    layoutButtonBox->addWidget(btnDuplicate);
    layoutButtonBox->addWidget(btnApply);
    layoutButtonBox->addWidget(btnClose);
    
    chkClearSolution = new QCheckBox(tr("Clear solution after solving the problem"));
    chkSolveProblem = new QCheckBox(tr("Solve problem"));
    
    QGridLayout *layoutGeneral = new QGridLayout(this);
    layoutGeneral->addWidget(chkClearSolution, 0, 0);
    layoutGeneral->addWidget(chkSolveProblem, 0, 1);
    
    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);
    
    QVBoxLayout *layoutStudy = new QVBoxLayout(this);
    layoutStudy->addWidget(grpGeneral);
    layoutStudy->addLayout(createStudyControls());
    layoutStudy->addStretch();
    
    QWidget *widgetStudy = new QWidget(this);
    widgetStudy->setLayout(layoutStudy);
    
    tabStudy = new QTabWidget(this);
    tabStudy->addTab(widgetStudy, tr("Study"));
    tabStudy->addTab(createParametersAndFunctionals(), tr("Parameters and functionals"));
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tabStudy);
    layout->addLayout(layoutButtonBox);
    
    setLayout(layout);
    
    readParameters();
    readFunctionals();
}

QWidget *StudyDialog::createParametersAndFunctionals()
{
    QVBoxLayout *layoutParameters = new QVBoxLayout(this);
    layoutParameters->addWidget(createParameters());
    
    QGroupBox *grpParameters = new QGroupBox(tr("Parameters"));
    grpParameters->setLayout(layoutParameters);
    
    QVBoxLayout *layoutFunctionals = new QVBoxLayout(this);
    layoutFunctionals->addWidget(createFunctionals());
    
    QGroupBox *grpFunctionals = new QGroupBox(tr("Functionals"));
    grpFunctionals->setLayout(layoutFunctionals);
    
    QVBoxLayout *layoutPF = new QVBoxLayout(this);
    layoutPF->addWidget(grpParameters);
    layoutPF->addWidget(grpFunctionals);
    
    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutPF);
    
    return widget;
}

QWidget *StudyDialog::createParameters()
{
    trvParameterWidget = new QTreeWidget(this);
    trvParameterWidget->setExpandsOnDoubleClick(false);
    trvParameterWidget->setHeaderHidden(false);
    trvParameterWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Lower bound") << tr("Upper bound"));
    trvParameterWidget->setColumnCount(3);
    trvParameterWidget->setIndentation(2);
    trvParameterWidget->setColumnWidth(0, 200);
    trvParameterWidget->headerItem()->setTextAlignment(1, Qt::AlignRight);
    trvParameterWidget->headerItem()->setTextAlignment(2, Qt::AlignRight);
    
    // connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvParameterWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doParameterItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvParameterWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doParameterItemDoubleClicked(QTreeWidgetItem *, int)));
    
    btnParameterAdd = new QPushButton(tr("Add"), this);
    connect(btnParameterAdd, SIGNAL(clicked(bool)), this, SLOT(doParameterAdd(bool)));
    btnParameterEdit = new QPushButton(tr("Edit"), this);
    connect(btnParameterEdit, SIGNAL(clicked(bool)), this, SLOT(doParameterEdit(bool)));
    btnParameterRemove = new QPushButton(tr("Remove"), this);
    connect(btnParameterRemove, SIGNAL(clicked(bool)), this, SLOT(doParameterRemove(bool)));
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnParameterAdd);
    buttonsLayout->addWidget(btnParameterEdit);
    buttonsLayout->addWidget(btnParameterRemove);
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvParameterWidget);
    layout->addLayout(buttonsLayout);
    
    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    
    return widget;
}

void StudyDialog::readParameters()
{
    trvParameterWidget->clear();
    
    foreach (Parameter parameter, m_study->parameters())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvParameterWidget);
        
        item->setText(0, QString("%1").arg(parameter.name()));
        item->setData(0, Qt::UserRole, parameter.name());
        item->setText(1, QString("%1").arg(parameter.lowerBound()));
        item->setText(2, QString("%1").arg(parameter.upperBound()));
        item->setTextAlignment(1, Qt::AlignRight);
        item->setTextAlignment(2, Qt::AlignRight);
    }
    
    btnParameterAdd->setEnabled(m_study->parameters().count() < Agros2D::problem()->config()->parameters().count());
    
    doParameterItemChanged(nullptr, nullptr);
}

void StudyDialog::doParameterItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    btnParameterEdit->setEnabled(trvParameterWidget->currentItem());
    btnParameterRemove->setEnabled(trvParameterWidget->currentItem());
    
    if (trvParameterWidget->currentItem())
    {
        
    }
}

void StudyDialog::doParameterItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    if (trvParameterWidget->currentItem())
    {
        doParameterEdit(true);
    }
}

void StudyDialog::doParameterAdd(bool checked)
{
    // select parameter dialog
    ParameterSelectDialog dialog(m_study, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        // add parameter
        QString name = dialog.selectedParameterName();
        if (!name.isEmpty())
        {
            Parameter parameter(name);
            StudyParameterDialog dialog(m_study, &parameter);
            if (dialog.exec() == QDialog::Accepted)
            {
                m_study->addParameter(parameter);
                readParameters();
            }
        }
    }
}

void StudyDialog::doParameterEdit(bool checked)
{
    if (trvParameterWidget->currentItem())
    {
        StudyParameterDialog dialog(m_study, &m_study->parameter(trvParameterWidget->currentItem()->data(0, Qt::UserRole).toString()));
        if (dialog.exec() == QDialog::Accepted)
        {
            readParameters();
        }
    }
}

void StudyDialog::doParameterRemove(bool checked)
{
    if (trvParameterWidget->currentItem())
    {
        m_study->removeParameter(trvParameterWidget->currentItem()->data(0, Qt::UserRole).toString());
        
        readParameters();
    }
}

QWidget *StudyDialog::createFunctionals()
{
    trvFunctionalWidget = new QTreeWidget(this);
    trvFunctionalWidget->setExpandsOnDoubleClick(false);
    trvFunctionalWidget->setHeaderHidden(false);
    trvFunctionalWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Weight") << tr("Expression"));
    trvFunctionalWidget->setColumnCount(3);
    trvFunctionalWidget->setIndentation(2);
    
    // connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvFunctionalWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doFunctionalItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvFunctionalWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doFunctionalItemDoubleClicked(QTreeWidgetItem *, int)));
    
    btnFunctionalAdd = new QPushButton(tr("Add"), this);
    connect(btnFunctionalAdd, SIGNAL(clicked(bool)), this, SLOT(doFunctionalAdd(bool)));
    btnFunctionalEdit = new QPushButton(tr("Edit"), this);
    connect(btnFunctionalEdit, SIGNAL(clicked(bool)), this, SLOT(doFunctionalEdit(bool)));
    btnFunctionalRemove = new QPushButton(tr("Remove"), this);
    connect(btnFunctionalRemove, SIGNAL(clicked(bool)), this, SLOT(doFunctionalRemove(bool)));
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(btnFunctionalAdd);
    buttonsLayout->addWidget(btnFunctionalEdit);
    buttonsLayout->addWidget(btnFunctionalRemove);
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvFunctionalWidget);
    layout->addLayout(buttonsLayout);
    
    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    
    
    return widget;
}

void StudyDialog::readFunctionals()
{
    trvFunctionalWidget->clear();
    
    foreach (Functional functional, m_study->functionals())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvFunctionalWidget);
        
        item->setText(0, QString("%1").arg(functional.name()));
        item->setData(0, Qt::UserRole, functional.name());
        item->setText(1, QString("%1 \%").arg(functional.weight()));
        item->setText(2, QString("%1").arg((functional.expression().count() < 45) ? functional.expression() : functional.expression().left(45) + "..."));
    }
    
    doFunctionalItemChanged(nullptr, nullptr);
}

void StudyDialog::doFunctionalItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    btnFunctionalEdit->setEnabled(trvFunctionalWidget->currentItem());
    btnFunctionalRemove->setEnabled(trvFunctionalWidget->currentItem());
    
    if (trvFunctionalWidget->currentItem())
    {
        
    }
}

void StudyDialog::doFunctionalItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    if (trvFunctionalWidget->currentItem())
    {
        doFunctionalEdit(true);
    }
}

void StudyDialog::doFunctionalAdd(bool checked)
{
    Functional functional;
    
    StudyFunctionalDialog dialog(m_study, &functional);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_study->addFunctional(functional);
        readFunctionals();
    }
}

void StudyDialog::doFunctionalEdit(bool checked)
{
    if (trvFunctionalWidget->currentItem())
    {
        StudyFunctionalDialog dialog(m_study, &m_study->functional(trvFunctionalWidget->currentItem()->data(0, Qt::UserRole).toString()));
        if (dialog.exec() == QDialog::Accepted)
        {
            readFunctionals();
        }
    }
}

void StudyDialog::doFunctionalRemove(bool checked)
{
    if (trvFunctionalWidget->currentItem())
    {
        m_study->removeFunctional(trvFunctionalWidget->currentItem()->data(0, Qt::UserRole).toString());
        
        readFunctionals();
    }
}

void StudyDialog::doAccept()
{
    save();
    accept();
}

void StudyDialog::load()
{
    chkClearSolution->setChecked(m_study->value(Study::General_ClearSolution).toBool());
    chkSolveProblem->setChecked(m_study->value(Study::General_SolveProblem).toBool());
}

void StudyDialog::save()
{
    m_study->setValue(Study::General_ClearSolution, chkClearSolution->isChecked());
    m_study->setValue(Study::General_SolveProblem, chkSolveProblem->isChecked());
}

void StudyDialog::doDuplicate()
{
    // select study dialog
    StudySelectDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        // add study
        if (dialog.selectedStudyType() != StudyType_Undefined)
        {
            Study *study = Study::factory(dialog.selectedStudyType());
            
            // copy parameters
            foreach (Parameter parameter, m_study->parameters())
                study->addParameter(Parameter(parameter.name(), parameter.lowerBound(), parameter.upperBound()));
            
            // copy functionals
            foreach (Functional functional, m_study->functionals())
                study->addFunctional(Functional(functional.name(), functional.expression(), functional.weight()));
            
            StudyDialog *studyDialog = StudyDialog::factory(study, this);
            if (studyDialog->showDialog() == QDialog::Accepted)
            {
                Agros2D::problem()->studies()->addStudy(study);
                
                close();
            }
            else
            {
                delete study;
            }
        }
    }
}

// **************************************************************************************************************

StudyFunctionalDialog::StudyFunctionalDialog(Study *study, Functional *functional, QWidget *parent)
    : m_study(study), m_functional(functional)
{
    createControls();
}

void StudyFunctionalDialog::createControls()
{
    setWindowTitle(tr("Functional: %1").arg(m_functional->name()));
    
    lblError = new QLabel();
    
    txtName = new QLineEdit(m_functional->name());
    connect(txtName, SIGNAL(textChanged(QString)), this, SLOT(functionalNameTextChanged(QString)));
    txtExpression = new QLineEdit(m_functional->expression());
    txtWeight = new QSpinBox();
    txtWeight->setRange(0, 100);
    txtWeight->setValue(m_functional->weight());
    
    QGridLayout *layoutEdit = new QGridLayout();
    layoutEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutEdit->addWidget(txtName, 0, 1);
    layoutEdit->addWidget(new QLabel(tr("Expression")), 1, 0);
    layoutEdit->addWidget(txtExpression, 1, 1);
    layoutEdit->addWidget(new QLabel(tr("Weight")), 2, 0);
    layoutEdit->addWidget(txtWeight, 2, 1);
    
    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);
    lblError->setVisible(false);
    
    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    
    QVBoxLayout *layoutWidget = new QVBoxLayout();
    layoutWidget->addLayout(layoutEdit);
    layoutWidget->addWidget(lblError);
    layoutWidget->addStretch();
    layoutWidget->addWidget(buttonBox);
    
    setLayout(layoutWidget);
    
    if (!m_functional->name().isEmpty())
        txtName->setFocus();
}

void StudyFunctionalDialog::functionalNameTextChanged(const QString &str)
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checkFunctional(str));
}

bool StudyFunctionalDialog::checkFunctional(const QString &str)
{
    try
    {
        Agros2D::problem()->config()->checkVariableName(str);
    }
    catch (AgrosException &e)
    {
        lblError->setText(e.toString());
        lblError->setVisible(true);
        
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    }
    
    foreach (Functional functional, m_study->functionals())
    {
        if (str == m_functional->name())
            continue;
        
        if (str == functional.name())
        {
            lblError->setText(tr("Functional already exists."));
            lblError->setVisible(true);
            
            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
            return false;
        }
    }
    
    lblError->setVisible(false);
    
    return true;
}

void StudyFunctionalDialog::doAccept()
{
    if (checkFunctional(txtName->text()))
    {
        m_functional->setName(txtName->text());
        m_functional->setExpression(txtExpression->text());
        m_functional->setWeight(txtWeight->value());
        
        accept();
    }
}

// **************************************************************************************************************

StudyParameterDialog::StudyParameterDialog(Study *study, Parameter *parameter, QWidget *parent)
    : m_study(study), m_parameter(parameter)
{
    createControls();
}

void StudyParameterDialog::createControls()
{
    setWindowTitle(tr("Parameter: %1").arg(m_parameter->name()));
    
    lblError = new QLabel();
    
    lblName = new QLabel(m_parameter->name(), this);
    txtLowerBound = new LineEditDouble(m_parameter->lowerBound(), this);
    connect(txtLowerBound, SIGNAL(editingFinished()), this, SLOT(checkRange()));
    txtUpperBound = new LineEditDouble(m_parameter->upperBound(), this);
    connect(txtUpperBound, SIGNAL(editingFinished()), this, SLOT(checkRange()));
    
    QGridLayout *layoutEdit = new QGridLayout();
    layoutEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutEdit->addWidget(lblName, 0, 1);
    layoutEdit->addWidget(new QLabel(tr("Lower bound")), 1, 0);
    layoutEdit->addWidget(txtLowerBound, 1, 1);
    layoutEdit->addWidget(new QLabel(tr("Upper bound")), 2, 0);
    layoutEdit->addWidget(txtUpperBound, 2, 1);
    
    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);
    lblError->setVisible(false);
    
    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
    
    QVBoxLayout *layoutWidget = new QVBoxLayout();
    layoutWidget->addLayout(layoutEdit);
    layoutWidget->addWidget(lblError);
    layoutWidget->addStretch();
    layoutWidget->addWidget(buttonBox);
    
    setLayout(layoutWidget);
}

bool StudyParameterDialog::checkRange()
{
    if (txtLowerBound->value() > txtUpperBound->value())
    {
        lblError->setText(tr("Lower bound is higher then upper bound."));
        lblError->setVisible(true);
        
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    }
    
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    lblError->setVisible(false);
    
    return true;
}

void StudyParameterDialog::doAccept()
{
    if (checkRange())
    {
        m_parameter->setLowerBound(txtLowerBound->value());
        m_parameter->setUpperBound(txtUpperBound->value());
        
        accept();
    }
}

ParameterSelectDialog::ParameterSelectDialog(Study *study, QWidget *parent) : QDialog(parent), m_study(study), m_selectedParameterName(QString())
{
    setWindowTitle(tr("Add parameter"));
    setModal(true);
    
    lstParameters = new QListWidget(this);
    lstParameters->setIconSize(QSize(24, 24));
    lstParameters->setMinimumHeight(26*3);
    
    // remaining parameters
    foreach (QString name, Agros2D::problem()->config()->parameters().keys())
    {
        bool skip = false;
        foreach (Parameter parameter, m_study->parameters())
        {
            if (parameter.name() == name)
            {
                skip = true;
                break;
            }
        }
        
        if (!skip)
        {
            QListWidgetItem *item = new QListWidgetItem(lstParameters);
            item->setIcon(iconAlphabet(name.at(0), AlphabetColor_Lightgray));
            item->setText(name);
            item->setData(Qt::UserRole, name);
            
            lstParameters->addItem(item);
        }
    }
    
    connect(lstParameters, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(doItemDoubleClicked(QListWidgetItem *)));
    connect(lstParameters, SIGNAL(itemActivated(QListWidgetItem *)), this, SLOT(doItemSelected(QListWidgetItem *)));
    connect(lstParameters, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(doItemSelected(QListWidgetItem *)));
    
    QGridLayout *layoutSurface = new QGridLayout();
    layoutSurface->addWidget(lstParameters);
    
    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);
    
    setLayout(layout);
    
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    if (lstParameters->count() > 0)
    {
        lstParameters->setCurrentRow(0);
        doItemSelected(lstParameters->currentItem());
    }
    
    int w = sizeHint().width();
    int h = 1.0/3.0 * QApplication::desktop()->screenGeometry().height();
    
    setMinimumSize(w, h);
    setMaximumSize(w, h);
    
    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

void ParameterSelectDialog::doItemSelected(QListWidgetItem *item)
{
    m_selectedParameterName = item->data(Qt::UserRole).toString();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void ParameterSelectDialog::doItemDoubleClicked(QListWidgetItem *item)
{
    if (lstParameters->currentItem())
    {
        m_selectedParameterName = item->data(Qt::UserRole).toString();
        accept();
    }
}

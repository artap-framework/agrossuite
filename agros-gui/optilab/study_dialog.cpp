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

#include "study_dialog.h"
#include "util/global.h"
#include "gui/lineeditdouble.h"
#include "gui/chart.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "optilab/study.h"

#include "optilab/study_sweep.h"
#include "optilab/study_nsga2.h"
#include "optilab/study_nlopt.h"
#include "optilab/study_bayesopt.h"
#include "optilab/study_methoddialog.h"

#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/normal.hpp>

double logVal(double val)
{
    return (val > 0) ? log10(fabs(val)) : -log10(fabs(val));
}

LogOptimizationDialog::LogOptimizationDialog(Study *study) : QDialog(QApplication::activeWindow()),
    m_study(study), progressBar(nullptr), m_computationSetsCount(1), m_step(1), m_totalValue(std::numeric_limits<double>::max())
{
    setModal(true);
    
    setWindowTitle(studyTypeString(study->type()));
    setAttribute(Qt::WA_DeleteOnClose);
    
    createControls();
    
    connect(btnAbort, SIGNAL(clicked()), this, SLOT(aborted()));
    m_study->updateParametersAndFunctionals = std::bind(&LogOptimizationDialog::updateParametersAndFunctionals, this, std::placeholders::_1, std::placeholders::_2);
    
    int w = 1.0/3.0 * QGuiApplication::primaryScreen()->availableGeometry().width();
    int h = 1.0/2.0 * QGuiApplication::primaryScreen()->availableGeometry().height();
    
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
        m_study->abortSolving();
    else
        close();
}

void LogOptimizationDialog::closeLog()
{
    if (m_study->isSolving())
    {
        Agros::log()->printError(tr("Solver"), tr("Study is being aborted."));
    }
    else
    {
        close();
    }
}

void LogOptimizationDialog::createControls()
{
#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif
    
    QFont fontProgress = font();
    fontProgress.setPointSize(fontSize);
    
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(closeLog()));
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

    trvProgress = new QTreeWidget(this);
    trvProgress->setIndentation(trvProgress->indentation() - 4);
    trvProgress->setHeaderHidden(true);
    trvProgress->setColumnCount(2);
    trvProgress->setColumnWidth(0, 150);
    trvProgress->setColumnWidth(1, 130);
    trvProgress->setMinimumWidth(300);

    // current and best parameters and functionals
    auto *currentNode = new QTreeWidgetItem(trvProgress);
    currentNode->setText(0, tr("Current"));
    currentNode->setFont(0, fontTitle);
    currentNode->setExpanded(true);

    currentStepNode = new QTreeWidgetItem(currentNode);
    currentStepNode->setIcon(0, iconAlphabet('S', AlphabetColor_Brown));
    currentStepNode->setText(0, tr("Step"));

    currentParametersNode = new QTreeWidgetItem(currentNode);
    currentParametersNode->setIcon(0, iconAlphabet('P', AlphabetColor_Green));
    currentParametersNode->setText(0, tr("Parameters"));
    currentParametersNode->setExpanded(true);

    currentFunctionalsNode = new QTreeWidgetItem(currentNode);
    currentFunctionalsNode->setIcon(0, iconAlphabet('F', AlphabetColor_Blue));
    currentFunctionalsNode->setText(0, tr("Goal Functions"));
    currentFunctionalsNode->setExpanded(true);

    auto *optimalNode = new QTreeWidgetItem(trvProgress);
    optimalNode->setText(0, tr("Optimal"));
    optimalNode->setFont(0, fontTitle);
    optimalNode->setExpanded(true);

    optimalStepNode = new QTreeWidgetItem(optimalNode);
    optimalStepNode->setIcon(0, iconAlphabet('S', AlphabetColor_Brown));
    optimalStepNode->setText(0, tr("Step"));

    optimalParametersNode = new QTreeWidgetItem(optimalNode);
    optimalParametersNode->setIcon(0, iconAlphabet('P', AlphabetColor_Green));
    optimalParametersNode->setText(0, tr("Parameters"));
    optimalParametersNode->setExpanded(true);

    optimalFunctionalsNode = new QTreeWidgetItem(optimalNode);
    optimalFunctionalsNode->setIcon(0, iconAlphabet('F', AlphabetColor_Blue));
    optimalFunctionalsNode->setText(0, tr("Goal Functions"));
    optimalFunctionalsNode->setExpanded(true);

    foreach (Parameter parameter, m_study->parameters())
    {
        auto *itemCurrent = new QTreeWidgetItem(currentParametersNode);
        itemCurrent->setText(0, parameter.name());

        auto *itemBest = new QTreeWidgetItem(optimalParametersNode);
        itemBest->setText(0, parameter.name());
    }

    foreach (Functional functional, m_study->functionals())
    {
        auto *itemCurrent = new QTreeWidgetItem(currentFunctionalsNode);
        itemCurrent->setText(0, functional.name());

        auto *itemBest = new QTreeWidgetItem(optimalFunctionalsNode);
        itemBest->setText(0, functional.name());
    }

    // chart
    totalChart = new QChart();
    totalChart->legend()->hide();
    totalChart->setTitle(tr("Solution"));

    // axis x
    axisX = new QValueAxis;
    axisX->setLabelFormat("%g");
    axisX->setGridLineVisible(true);
    axisX->setTitleText(tr("number of steps"));
    totalChart->addAxis(axisX, Qt::AlignBottom);

    // axis y
    axisObjective = new QValueAxis;
    axisObjective->setLabelFormat("%g");
    axisObjective->setGridLineVisible(true);
    axisObjective->setTitleText(tr("objective"));
    totalChart->addAxis(axisObjective, Qt::AlignLeft);

    axisUncertainty = new QValueAxis;
    axisUncertainty->setLabelFormat("%g");
    axisUncertainty->setGridLineVisible(true);
    axisUncertainty->setTitleText(tr("uncertainty"));
    totalChart->addAxis(axisUncertainty, Qt::AlignRight);

    // attach axis
    totalObjectiveSeries = new QScatterSeries();
    totalObjectiveSeries->setMarkerSize(12.0);
    totalChart->addSeries(totalObjectiveSeries);
    totalObjectiveSeries->attachAxis(axisX);
    totalObjectiveSeries->attachAxis(axisObjective);

    totalObjectiveUncertaintySeries = new QLineSeries();
    totalChart->addSeries(totalObjectiveUncertaintySeries);
    totalObjectiveUncertaintySeries->attachAxis(axisX);
    totalObjectiveUncertaintySeries->attachAxis(axisUncertainty);

    totalObjectiveUncertaintyLowerSeries = new QLineSeries();
    totalChart->addSeries(totalObjectiveUncertaintyLowerSeries);
    totalObjectiveUncertaintyLowerSeries->attachAxis(axisX);
    totalObjectiveUncertaintyLowerSeries->attachAxis(axisUncertainty);

    totalObjectiveUncertaintyUpperSeries = new QLineSeries();
    totalChart->addSeries(totalObjectiveUncertaintyUpperSeries);
    totalObjectiveUncertaintyUpperSeries->attachAxis(axisX);
    totalObjectiveUncertaintyUpperSeries->attachAxis(axisUncertainty);

    totalObjectiveUncertaintyArea = new QAreaSeries(totalObjectiveUncertaintyLowerSeries, totalObjectiveUncertaintyUpperSeries);
    totalObjectiveUncertaintyArea->setOpacity(0.5);
    totalObjectiveUncertaintyArea->setPen(pen);
    totalChart->addSeries(totalObjectiveUncertaintyArea);

    progressBar = new QProgressBar(this);
    progressBar->setMaximum(m_study->estimatedNumberOfSteps());

    auto *chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setChart(totalChart);

    auto *layoutParametersAndFunctionalsAndChart = new QHBoxLayout();
    layoutParametersAndFunctionalsAndChart->addWidget(trvProgress, 1);
    layoutParametersAndFunctionalsAndChart->addWidget(chartView, 10);

    auto *layoutObjective = new QVBoxLayout();
    layoutObjective->addLayout(layoutParametersAndFunctionalsAndChart, 1);
    layoutObjective->addWidget(progressBar, 1);
    
    auto *layout = new QVBoxLayout();
    layout->addLayout(layoutObjective, 2);
    layout->addLayout(layoutStatus);
    
    setLayout(layout);
}

void LogOptimizationDialog::updateParametersAndFunctionals(QSharedPointer<Computation> computation, SolutionUncertainty solutionUncertainty)
{
    int computationSetsCount = m_study->computationSets(m_study->value(Study::View_Filter).toString()).count();
    double totalValue = m_study->evaluateSingleGoal(computation);

    // update total value
    bool updateTotalValue = (totalValue < m_totalValue);
    if (updateTotalValue)
    {
        m_totalValue = totalValue;
        // set optimal step
        optimalStepNode->setText(1, QString("%1").arg(m_step));
    }

    // parameters
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        currentParametersNode->child(i)->setText(1, QString::number(computation->config()->parameters()->number(m_study->parameters()[i].name())));

        if (updateTotalValue)
            optimalParametersNode->child(i)->setText(1, QString::number(computation->config()->parameters()->number(m_study->parameters()[i].name())));
    }

    // functionals
    for (int i = 0; i < m_study->functionals().count(); i++)
    {
        double value = computation->results()->value(m_study->functionals()[i].name());

        currentFunctionalsNode->child(i)->setText(1, QString::number(value));

        if (updateTotalValue)
            optimalFunctionalsNode->child(i)->setText(1, QString::number(value));
    }

    // total objective function
    totalObjectiveSeries->append(m_step, totalValue);
    totalObjectiveUncertaintySeries->append(m_step, solutionUncertainty.uncertainty);
    totalObjectiveUncertaintyLowerSeries->append(m_step, solutionUncertainty.lowerBound);
    totalObjectiveUncertaintyUpperSeries->append(m_step, solutionUncertainty.upperBound);
    // fit x
    axisX->setRange(1, m_step);

    QPen pen;
    pen.setWidth(2.0);

    QVector<double> lower;
    foreach (QPointF point, totalObjectiveUncertaintyLowerSeries->points())
        lower.append(point.y());
    QVector<double> upper;
    foreach (QPointF point, totalObjectiveUncertaintyUpperSeries->points())
        upper.append(point.y());

    double minBounds = *std::min_element(lower.constBegin(), lower.constEnd());
    double maxBounds = *std::max_element(upper.constBegin(), upper.constEnd());
    axisUncertainty->setRange(minBounds, maxBounds);

    totalChart->removeSeries(totalObjectiveUncertaintyArea);
    totalObjectiveUncertaintyArea = new QAreaSeries(totalObjectiveUncertaintyLowerSeries, totalObjectiveUncertaintyUpperSeries);
    totalObjectiveUncertaintyArea->setOpacity(0.5);
    totalObjectiveUncertaintyArea->setPen(pen);
    totalChart->addSeries(totalObjectiveUncertaintyArea);

    // set objective range
    QRectF axesRange = findMinMax(totalObjectiveSeries->points());
    axisObjective->setRange(axesRange.top(), axesRange.bottom());

    m_computationSetsCount = m_study->computationSets(m_study->value(Study::View_Filter).toString()).count();
    
    currentStepNode->setText(1, QString("%1 / %2").arg(m_step).arg(progressBar->maximum()));
    progressBar->setValue(m_step);
    QApplication::processEvents();

    m_step++;
}

void LogOptimizationDialog::aborted()
{
    m_study->abortSolving();

    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);
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
    int h = 1.5/5.0 * QGuiApplication::primaryScreen()->availableGeometry().height();
    
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

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(chkClearSolution, 0, 0);
    layoutGeneral->addWidget(chkSolveProblem, 1, 0);
    layoutGeneral->setRowStretch(10, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);
    
    chkDoE = new QCheckBox(tr("Enable DoE"));
    txtDoEDeviation = new LineEditDouble();
    txtDoEDeviation->setBottom(0.0);
    txtDoESweepSamples = new QSpinBox();
    txtDoESweepSamples->setMinimum(1);
    txtDoESweepSamples->setMaximum(1000);

    QGridLayout *layoutDoE = new QGridLayout();
    layoutDoE->addWidget(chkDoE, 0, 1, 1, 2);
    layoutDoE->addWidget(new QLabel(tr("Deviation (%):")), 1, 1);
    layoutDoE->addWidget(txtDoEDeviation, 1, 2);
    layoutDoE->addWidget(new QLabel(tr("Number of Samples:")), 2, 1);
    layoutDoE->addWidget(txtDoESweepSamples, 2, 2);
    layoutDoE->setRowStretch(10, 1);

    QGroupBox *grpDoE = new QGroupBox(tr("Design of Experiments"));
    grpDoE->setLayout(layoutDoE);

    QHBoxLayout *layoutOrdinary = new QHBoxLayout();
    layoutOrdinary->addWidget(grpGeneral);
    layoutOrdinary->addWidget(grpDoE);

    QVBoxLayout *layoutStudy = new QVBoxLayout();
    layoutStudy->addLayout(layoutOrdinary);
    layoutStudy->addLayout(createStudyControls());
    layoutStudy->addStretch();
    
    QWidget *widgetStudy = new QWidget(this);
    widgetStudy->setLayout(layoutStudy);
    
    tabStudy = new QTabWidget(this);
    tabStudy->addTab(widgetStudy, tr("Study"));
    tabStudy->addTab(createParametersAndFunctionals(), tr("Parameters and functionals"));
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabStudy);
    layout->addLayout(layoutButtonBox);
    
    setLayout(layout);
    
    readParameters();
    readFunctionals();
}

QWidget *StudyDialog::createParametersAndFunctionals()
{
    auto *layoutParameters = new QVBoxLayout();
    layoutParameters->addWidget(createParameters());

    auto *grpParameters = new QGroupBox(tr("Parameters"));
    grpParameters->setLayout(layoutParameters);

    auto *layoutFunctionals = new QVBoxLayout();
    layoutFunctionals->addWidget(createFunctionals());

    auto *grpFunctionals = new QGroupBox(tr("Goal Functions"));
    grpFunctionals->setLayout(layoutFunctionals);

    auto *layoutPF = new QVBoxLayout();
    layoutPF->addWidget(grpParameters);
    layoutPF->addWidget(grpFunctionals);
    
    auto *widget = new QWidget(this);
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
    
    btnParameterAdd->setEnabled(m_study->parameters().count() < Agros::problem()->config()->parameters()->items().count());
    
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

    chkDoE->setChecked(m_study->value(Study::General_DoE).toBool());
    txtDoEDeviation->setValue(m_study->value(Study::General_DoE_Deviation).toDouble());
    txtDoESweepSamples->setValue(m_study->value(Study::General_DoE_SweepSamples).toInt());
}

void StudyDialog::save()
{
    m_study->setValue(Study::General_ClearSolution, chkClearSolution->isChecked());
    m_study->setValue(Study::General_SolveProblem, chkSolveProblem->isChecked());

    m_study->setValue(Study::General_DoE, chkDoE->isChecked());
    m_study->setValue(Study::General_DoE_Deviation, txtDoEDeviation->value());
    m_study->setValue(Study::General_DoE_SweepSamples, txtDoESweepSamples->value());
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
            
            // clear and solve
            study->setValue(Study::General_ClearSolution, m_study->value(Study::General_ClearSolution).toBool());
            study->setValue(Study::General_SolveProblem, m_study->value(Study::General_SolveProblem).toBool());

            StudyDialog *studyDialog = StudyDialog::factory(study, this);
            if (studyDialog->showDialog() == QDialog::Accepted)
            {
                Agros::problem()->studies()->addStudy(study);
                
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
    setMinimumWidth(400);

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
        Agros::problem()->config()->checkVariableName(str);
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
    checkRange();
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
    foreach (QString name, Agros::problem()->config()->parameters()->items().keys())
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
    int h = 1.0/3.0 * QGuiApplication::primaryScreen()->availableGeometry().height();
    
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

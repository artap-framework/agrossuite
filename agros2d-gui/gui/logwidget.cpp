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

#include "logwidget.h"

#include "util/util.h"
#include "solver/solutionstore.h"

#include "qcustomplot/qcustomplot.h"

LogGui::LogGui()
{
    // qRegisterMetaType<QVector<double> >("QVector<double>");
    // qRegisterMetaType<SolverAgros::Phase>("SolverAgros::Phase");
}



LogConfigWidget::LogConfigWidget(LogWidget *logWidget)
    : QWidget(logWidget), m_logWidget(logWidget)
{

}

LogWidget::LogWidget(QWidget *parent) : QWidget(parent),
    m_printCounter(0)
{    
   (static_cast <LogGui *>(Agros2D::log()))->setWidget(this);
    plainLog = new QTextEdit(this);
    plainLog->setReadOnly(true);
    // plainLog->setMaximumBlockCount(500);
    plainLog->setMinimumSize(160, 80);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(plainLog, 1);

    setLayout(layoutMain);

    createActions();

    // context menu
    mnuInfo = new QMenu(this);
    mnuInfo->addAction(actShowTimestamp);
#ifndef QT_NO_DEBUG_OUTPUT
    mnuInfo->addAction(actShowDebug);
#endif
    mnuInfo->addSeparator();
    mnuInfo->addAction(actClear);
    /*
    connect(Agros2D::log(), SIGNAL(headingMsg(QString)), this, SLOT(printHeading(QString)));
    connect(Agros2D::log(), SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Agros2D::log(), SIGNAL(warningMsg(QString, QString)), this, SLOT(printWarning(QString, QString)));
    connect(Agros2D::log(), SIGNAL(debugMsg(QString, QString)), this, SLOT(printDebug(QString, QString)));

    connect(Agros2D::log(), SIGNAL(appendImg(QString)), this, SLOT(appendImage(QString)));
     connect(Agros2D::log(), SIGNAL(appendHtm(QString)), this, SLOT(appendHtml(QString))); */

    plainLog->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plainLog, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));

    // create log directory
    QDir().mkdir(QString("%1/log").arg(tempProblemDir()));
}

LogWidget::~LogWidget()
{
    (static_cast <LogGui *>(Agros2D::log()))->removeWidget(this);
}

void LogWidget::clear()
{
    plainLog->clear();
}

void LogWidget::contextMenu(const QPoint &pos)
{
    mnuInfo->exec(QCursor::pos());
}

void LogWidget::createActions()
{
    QSettings settings;

    actShowTimestamp = new QAction(tr("Show timestamp"), this);
    actShowTimestamp->setCheckable(true);
    actShowTimestamp->setChecked(settings.value("LogWidget/ShowTimestamp", false).toBool());
    connect(actShowTimestamp, SIGNAL(triggered()), this, SLOT(showTimestamp()));

    actShowDebug = new QAction(tr("Show debug"), this);
    actShowDebug->setCheckable(true);
    actShowDebug->setChecked(settings.value("LogWidget/ShowDebug", false).toBool());
    connect(actShowDebug, SIGNAL(triggered()), this, SLOT(showDebug()));

    actClear = new QAction(tr("Clear"), this);
    connect(actClear, SIGNAL(triggered()), this, SLOT(clear()));
}

void LogWidget::showTimestamp()
{
    QSettings settings;
    settings.setValue("LogWidget/ShowTimestamp", actShowTimestamp->isChecked());
}

void LogWidget::showDebug()
{
    QSettings settings;
    settings.setValue("LogWidget/ShowDebug", actShowDebug->isChecked());
}

void LogWidget::printHeading(const QString &message)
{
    print(tr("Start"), tr("%1").arg(message), "green");
    plainLog->ensureCursorVisible();
}

void LogWidget::printMessage(const QString &module, const QString &message)
{
    print(module, message, "black");
}

void LogWidget::printError(const QString &module, const QString &message)
{
    print(module, message, "red");
}

void LogWidget::printWarning(const QString &module, const QString &message)
{
    print(module, message, "blue");
}

void LogWidget::printDebug(const QString &module, const QString &message)
{
#ifndef QT_NO_DEBUG_OUTPUT
    if (actShowDebug->isChecked())
        print(module, message, "gray");
#endif
}

void LogWidget::print(const QString &module, const QString &message, const QString &color)
{
    QString strTime = "";
    if (actShowTimestamp->isChecked())
    {
        strTime = QString(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": ").toHtmlEscaped();
    }

    // QString strMessage = QString(message).toHtmlEscaped();
    QString strMessage = QString(message).toHtmlEscaped().replace("\n", "<br/>");

    QString html = QString("<div><span style=\"color: gray;\">%1</span><span style=\"color: %2;\"><strong>%3</strong>: %4</span></div>").
            arg(strTime).
            arg(color).
            arg(module).
            arg(strMessage);

    plainLog->append(html);

    ensureCursorVisible();
}

void LogWidget::ensureCursorVisible()
{
    // ensure cursor visible
    QTextCursor cursor = plainLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    plainLog->setTextCursor(cursor);
    plainLog->ensureCursorVisible();
    plainLog->repaint();
}

void LogWidget::welcomeMessage()
{
    print("Agros2D", tr("version: %1").arg(QApplication::applicationVersion()), "green");
}

void LogWidget::appendImage(const QString &fileName)
{
    plainLog->append(QString());

    QTextCursor cursor = plainLog->textCursor();
    cursor.insertImage(fileName);

    plainLog->append(QString());

    ensureCursorVisible();
}

void LogWidget::appendHtml(const QString &html)
{
    plainLog->append(QString());
    plainLog->append(html);
    plainLog->append(QString());
    ensureCursorVisible();
}

// *******************************************************************************************************

LogView::LogView(QWidget *parent) : QWidget(parent)
{
    setObjectName("LogView");

    actLog = new QAction(icon("log"), tr("Log"), this);
    actLog->setShortcut(Qt::Key_F10);
    actLog->setCheckable(true);

    m_logWidget = new LogWidget(this);
    m_logWidget->welcomeMessage();

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 2);
    layoutMain->addWidget(m_logWidget);

    setLayout(layoutMain);

    m_logConfigWidget = new LogConfigWidget(m_logWidget);
}

LogView::~LogView()
{    
    delete m_logConfigWidget;
}

// *******************************************************************************************************

LogDialog::LogDialog(Computation *computation, const QString &title) : QDialog(QApplication::activeWindow()),
    m_computation(computation),
    m_nonlinearChart(nullptr), m_nonlinearErrorGraph(nullptr), m_nonlinearProgress(nullptr),
    m_adaptivityChart(nullptr), m_adaptivityErrorGraph(nullptr), m_adaptivityDOFsGraph(nullptr), m_adaptivityProgress(nullptr),
    m_timeChart(nullptr), m_timeTimeStepGraph(nullptr), m_timeProgress(nullptr),
    m_progress(nullptr)
{
    setModal(true);

    setWindowTitle(title);
    setAttribute(Qt::WA_DeleteOnClose);

    createControls();

    connect(btnAbort, SIGNAL(clicked()), m_computation, SLOT(doAbortSolve()));
    connect(Agros2D::problem(), SIGNAL(meshed()), this, SLOT(tryClose()));
    connect(m_computation, SIGNAL(solved()), this, SLOT(tryClose()));

    int w = 2.0/3.0 * QApplication::desktop()->screenGeometry().width();
    int h = 2.0/3.0 * QApplication::desktop()->screenGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

void LogDialog::closeEvent(QCloseEvent *e)
{
    if (m_computation->isMeshing() || m_computation->isSolving())
        e->ignore();
}

void LogDialog::reject()
{
    if (m_computation->isMeshing() || m_computation->isSolving())
        m_computation->doAbortSolve();
    else
        close();
}

LogDialog::~LogDialog()
{
}

void LogDialog::createControls()
{
    /*
    connect(Agros2D::log(), SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(Agros2D::log(), SIGNAL(updateNonlinearChart(SolverAgros::Phase, const QVector<double>, const QVector<double>)),
           this, SLOT(updateNonlinearChartInfo(SolverAgros::Phase, const QVector<double>, const QVector<double>)));
    connect(Agros2D::log(), SIGNAL(updateAdaptivityChart(const FieldInfo *, int, int)), this, SLOT(updateAdaptivityChartInfo(const FieldInfo *, int, int)));
    connect(Agros2D::log(), SIGNAL(updateTransientChart(double)), this, SLOT(updateTransientChartInfo(double)));
    connect(Agros2D::log(), SIGNAL(addIconImg(QIcon, QString)), this, SLOT(addIcon(QIcon, QString))); */

    m_logWidget = new LogWidget(this);
    // (static_cast <LogGui *>(Agros2D::log()))->setWidget(m_logWidget);


#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif

    QFont fontProgress = font();
    fontProgress.setPointSize(fontSize);

    m_progress = new QListWidget(this);
    m_progress->setCurrentRow(0);
    m_progress->setViewMode(QListView::IconMode);
    m_progress->setResizeMode(QListView::Adjust);
    m_progress->setMovement(QListView::Static);
    m_progress->setResizeMode(QListView::Adjust);
    m_progress->setFlow(QListView::LeftToRight);
    m_progress->setIconSize(QSize(32, 32));
    m_progress->setMinimumHeight(85);
    m_progress->setMaximumHeight(85);
    m_progress->setFont(fontProgress);
    m_progress->setStyleSheet(QString("QListView { background-color: %1; border: 0px; padding: 0px; margin: 0px; }").
                              arg(this->palette().color(QPalette::Background).name()));

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

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();

    // transient
    if (m_computation->isTransient())
    {
        m_timeChart = new QCustomPlot(this);
        QCPPlotTitle *timeTitle = new QCPPlotTitle(m_timeChart, tr("Transient problem"));
        timeTitle->setFont(fontTitle);
        m_timeChart->plotLayout()->insertRow(0);
        m_timeChart->plotLayout()->addElement(0, 0, timeTitle);
        m_timeChart->legend->setVisible(true);
        m_timeChart->legend->setFont(fontChart);

        m_timeChart->xAxis->setTickLabelFont(fontChart);
        m_timeChart->xAxis->setLabelFont(fontChart);
        // m_timeChart->xAxis->setTickStep(1.0);
        m_timeChart->xAxis->setAutoTickStep(true);
        m_timeChart->xAxis->setLabel(tr("number of steps"));

        m_timeChart->yAxis->setTickLabelFont(fontChart);
        m_timeChart->yAxis->setLabelFont(fontChart);
        m_timeChart->yAxis->setLabel(tr("step length"));
        m_timeChart->yAxis2->setVisible(true);
        m_timeChart->yAxis2->setTickLabelFont(fontChart);
        m_timeChart->yAxis2->setLabelFont(fontChart);
        m_timeChart->yAxis2->setLabel(tr("total time"));

        m_timeTimeStepGraph = m_timeChart->addGraph(m_timeChart->xAxis, m_timeChart->yAxis);
        m_timeTimeStepGraph->setLineStyle(QCPGraph::lsLine);
        // m_timeTimeStepGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
        m_timeTimeStepGraph->setPen(pen);
        m_timeTimeStepGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
        m_timeTimeStepGraph->setName(tr("step length"));
        m_timeTimeTotalGraph = m_timeChart->addGraph(m_timeChart->xAxis, m_timeChart->yAxis2);
        m_timeTimeTotalGraph->setLineStyle(QCPGraph::lsLine);
        m_timeTimeTotalGraph->setPen(pen);
        m_timeTimeTotalGraph->setBrush(QBrush(QColor(255, 0, 0, 20)));
        m_timeTimeTotalGraph->setName(tr("total time"));

        m_timeProgress = new QProgressBar(this);
        m_timeProgress->setMaximum(10000);

        QVBoxLayout *layoutTime = new QVBoxLayout();
        layoutTime->addWidget(m_timeChart, 1);
        layoutTime->addWidget(m_timeProgress);

        layoutHorizontal->addLayout(layoutTime, 1);
    }

    // nonlinear
    if (m_computation->determineIsNonlinear())
    {
        m_nonlinearChart = new QCustomPlot(this);
        QCPPlotTitle *nonlinearTitle = new QCPPlotTitle(m_nonlinearChart, tr("Nonlinear solver"));
        nonlinearTitle->setFont(fontTitle);
        m_nonlinearChart->plotLayout()->insertRow(0);
        m_nonlinearChart->plotLayout()->addElement(0, 0, nonlinearTitle);
        m_nonlinearChart->setFont(fontChart);

        m_nonlinearChart->xAxis->setTickLabelFont(fontChart);
        m_nonlinearChart->xAxis->setLabelFont(fontChart);
        // m_nonlinearChart->xAxis->setTickStep(1.0);
        m_nonlinearChart->xAxis->setAutoTickStep(true);
        m_nonlinearChart->xAxis->setLabel(tr("number of iterations"));

        m_nonlinearChart->yAxis->setScaleType(QCPAxis::stLogarithmic);
        m_nonlinearChart->yAxis->setTickLabelFont(fontChart);
        m_nonlinearChart->yAxis->setLabelFont(fontChart);
        m_nonlinearChart->yAxis->setLabel(tr("rel. change of sln. (%)"));

        m_nonlinearErrorGraph = m_nonlinearChart->addGraph(m_nonlinearChart->xAxis, m_nonlinearChart->yAxis);
        m_nonlinearErrorGraph->setLineStyle(QCPGraph::lsLine);
        m_nonlinearErrorGraph->setPen(pen);
        m_nonlinearErrorGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));

        m_nonlinearProgress = new QProgressBar(this);
        m_nonlinearProgress->setMaximum(10000);

        QVBoxLayout *layoutNonlinear = new QVBoxLayout();
        layoutNonlinear->addWidget(m_nonlinearChart, 1);
        layoutNonlinear->addWidget(m_nonlinearProgress);

        layoutHorizontal->addLayout(layoutNonlinear, 1);
    }

    // adaptivity
    if (m_computation->numAdaptiveFields() > 0)
    {
        m_adaptivityChart = new QCustomPlot(this);
        QCPPlotTitle *adaptivityTitle = new QCPPlotTitle(m_adaptivityChart, tr("Adaptivity"));
        adaptivityTitle->setFont(fontTitle);
        m_adaptivityChart->plotLayout()->insertRow(0);
        m_adaptivityChart->plotLayout()->addElement(0, 0, adaptivityTitle);
        m_adaptivityChart->legend->setVisible(true);
        m_adaptivityChart->legend->setFont(fontChart);

        m_adaptivityChart->xAxis->setTickLabelFont(fontChart);
        m_adaptivityChart->xAxis->setLabelFont(fontChart);
        // m_adaptivityChart->xAxis->setTickStep(1.0);
        m_adaptivityChart->xAxis->setAutoTickStep(true);
        m_adaptivityChart->xAxis->setLabel(tr("number of iterations"));

        m_adaptivityChart->yAxis->setScaleType(QCPAxis::stLogarithmic);
        m_adaptivityChart->yAxis->setTickLabelFont(fontChart);
        m_adaptivityChart->yAxis->setLabelFont(fontChart);
        m_adaptivityChart->yAxis->setLabel(tr("error"));
        m_adaptivityChart->yAxis2->setVisible(true);
        m_adaptivityChart->yAxis2->setTickLabelFont(fontChart);
        m_adaptivityChart->yAxis2->setLabelFont(fontChart);
        m_adaptivityChart->yAxis2->setLabel(tr("number of DOFs"));

        m_adaptivityErrorGraph = m_adaptivityChart->addGraph(m_adaptivityChart->xAxis, m_adaptivityChart->yAxis);
        m_adaptivityErrorGraph->setLineStyle(QCPGraph::lsLine);
        m_adaptivityErrorGraph->setPen(pen);
        m_adaptivityErrorGraph->setBrush(QBrush(QColor(0, 0, 255, 20)));
        m_adaptivityErrorGraph->setName(tr("error"));
        m_adaptivityDOFsGraph = m_adaptivityChart->addGraph(m_adaptivityChart->xAxis, m_adaptivityChart->yAxis2);
        m_adaptivityDOFsGraph->setLineStyle(QCPGraph::lsLine);
        m_adaptivityDOFsGraph->setPen(pen);
        m_adaptivityDOFsGraph->setBrush(QBrush(QColor(255, 0, 0, 20)));
        m_adaptivityDOFsGraph->setName(tr("DOFs"));

        m_adaptivityProgress = new QProgressBar(this);
        m_adaptivityProgress->setMaximum(10000);

        QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
        layoutAdaptivity->addWidget(m_adaptivityChart, 1);
        layoutAdaptivity->addWidget(m_adaptivityProgress);

        layoutHorizontal->addLayout(layoutAdaptivity, 1);
    }

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_progress, 0);
    if (!layoutHorizontal->isEmpty())
        layout->addLayout(layoutHorizontal, 4);
    else
        delete layoutHorizontal;
    layout->addWidget(m_logWidget, 1);
    layout->addStretch();
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogDialog::printError(const QString &module, const QString &message)
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);

    m_logWidget->setVisible(true);
}

void LogDialog::updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions)
{
    if (!m_nonlinearErrorGraph)
        return;

    m_nonlinearErrorGraph->setData(steps, relativeChangeOfSolutions);
    m_nonlinearChart->rescaleAxes();
    m_nonlinearChart->replot(QCustomPlot::rpImmediate);

    // progress bar
    if (phase == SolverAgros::Phase_Finished)
    {
        m_nonlinearProgress->setValue(10000);
    }
    else
    {
        double valueRelativeChange = pow(10000.0, ((relativeChangeOfSolutions.first() - relativeChangeOfSolutions.last()) / relativeChangeOfSolutions.first()));
        m_nonlinearProgress->setValue(valueRelativeChange);
    }
}

void LogDialog::updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    if (!m_adaptivityErrorGraph)
        return;

    QVector<double> adaptiveSteps;
    QVector<double> adaptiveDOFs;
    QVector<double> adaptiveError;

    for (int i = 0; i < adaptivityStep; i++)
    {
        SolutionStore::SolutionRunTimeDetails runTime = m_computation->solutionStore()->multiSolutionRunTimeDetail(FieldSolutionID(fieldInfo->fieldId(), timeStep, i));

        adaptiveSteps.append(i + 1);
        adaptiveDOFs.append(runTime.value(SolutionStore::SolutionRunTimeDetails::DOFs).toInt());
        adaptiveError.append(runTime.value(SolutionStore::SolutionRunTimeDetails::AdaptivityError).toDouble());
    }

    m_adaptivityErrorGraph->setData(adaptiveSteps, adaptiveError);
    m_adaptivityDOFsGraph->setData(adaptiveSteps, adaptiveDOFs);
    m_adaptivityChart->rescaleAxes();
    m_adaptivityChart->replot(QCustomPlot::rpImmediate);

    // progress bar
    double valueSteps = 10000.0 * (adaptivityStep / fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());
    double valueTol = pow(10000.0, (adaptiveError.first() - adaptiveError.last()) / adaptiveError.first());
    m_adaptivityProgress->setValue(qMax(valueSteps, valueTol));
}

void LogDialog::updateTransientChartInfo(double actualTime)
{
    if (!m_timeTimeStepGraph)
        return;

    QVector<double> timeSteps;
    QVector<double> timeLengths = m_computation->timeStepLengths().toVector();
    QVector<double> timeTotal;
    double maximum = 0.0;
    for (int i = 0; i < timeLengths.size(); i++)
    {
        timeSteps.append(i + 1);
        if (timeLengths[i] > maximum)
            maximum = timeLengths[i];

        timeTotal.append((timeTotal.size() == 0 ? 0.0 : timeTotal.last()) + timeLengths[i]);
    }

    m_timeTimeStepGraph->setData(timeSteps, timeLengths);
    m_timeChart->yAxis->setRangeLower(0.0);
    m_timeChart->yAxis->setRangeUpper(maximum);
    m_timeTimeTotalGraph->setData(timeSteps, timeTotal);
    m_timeChart->yAxis2->setRangeLower(0.0);
    m_timeChart->yAxis2->setRangeUpper(timeTotal.last());
    m_timeTimeStepGraph->rescaleKeyAxis();
    m_timeChart->replot(QCustomPlot::rpImmediate);

    // progress bar
    m_timeProgress->setValue((10000.0 * actualTime / m_computation->config()->value(ProblemConfig::TimeTotal).toDouble()));
}

void LogDialog::addIcon(const QIcon &icn, const QString &label)
{
    static QString previousLabel;

    if (previousLabel != label)
    {
        QListWidgetItem *item = new QListWidgetItem(icn, label, m_progress);
        item->setTextAlignment(Qt::AlignHCenter);

        m_progress->addItem(item);
        // m_progress->setCurrentItem(item);
        m_progress->repaint();
    }

    previousLabel = label;
}

void LogDialog::tryClose()
{
    if (m_computation->isSolving())
    {
        Agros2D::log()->printError(tr("Solver"), tr("Solution is being aborted."));
    }
    else
    {
        close();
    }
}


void LogGui::setWidget(LogWidget *logWidget)
{
    m_logWidgets.push_back(logWidget);
}

void LogGui::removeWidget(LogWidget *logWidget)
{
    m_logWidgets.removeOne(logWidget);
}

void LogGui::printMessage(const QString &module, const QString &message)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->printMessage(module, message);
    }
}

void LogGui::printError(const QString &module, const QString &message)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->printError(module, message);
    }
}

void LogGui::printHeading(const QString &message)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->printHeading(message);
    }
}

void LogGui::printWarning(const QString &module, const QString &message)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->printWarning(module, message);
    }
}

void LogGui::printDebug(const QString &module, const QString &message)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->printDebug(module, message);
    }
}

void LogGui::appendImage(const QString &fileName)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->appendImage(fileName);
    }
}

void LogGui::appendHtml(const QString &html)
{
    foreach (LogWidget* logWidget, m_logWidgets) {
        logWidget->appendHtml(html);
    }
}

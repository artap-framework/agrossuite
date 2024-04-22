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
#include "gui/chart.h"
#include "solver/solutionstore.h"

LogGui::LogGui()
{
     qRegisterMetaType<QVector<double> >("QVector<double>");
     qRegisterMetaType<SolverAgros::Phase>("SolverAgros::Phase");
}

void LogGui::printHeading(const QString &message)
{
    emit m_connectLog->headingMsg(message);
}

void LogGui::printMessage(const QString &module, const QString &message)
{
    emit m_connectLog->messageMsg(module, message);
}

void LogGui::printError(const QString &module, const QString &message)
{
    emit m_connectLog->errorMsg(module, message);
}

void LogGui::printWarning(const QString &module, const QString &message)
{
    emit m_connectLog->warningMsg(module, message);
}

void LogGui::printDebug(const QString &module, const QString &message)
{
    emit m_connectLog->debugMsg(module, message);
}

LogWidget::LogWidget(QWidget *parent, ConnectLog *connectLog) : QWidget(parent),
    printCounter(0), m_connectLog(connectLog)
{       
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

    connect(m_connectLog, SIGNAL(headingMsg(QString)), this, SLOT(printHeading(QString)));
    connect(m_connectLog, SIGNAL(messageMsg(QString, QString)), this, SLOT(printMessage(QString, QString)));
    connect(m_connectLog, SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(m_connectLog, SIGNAL(warningMsg(QString, QString)), this, SLOT(printWarning(QString, QString)));
    connect(m_connectLog, SIGNAL(debugMsg(QString, QString)), this, SLOT(printDebug(QString, QString)));

    connect(m_connectLog, SIGNAL(appendImg(QString)), this, SLOT(appendImage(QString)));
    connect(m_connectLog, SIGNAL(appendHtm(QString)), this, SLOT(appendHtml(QString)));

    plainLog->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plainLog, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenu(const QPoint &)));

    // create log directory

    QDir().mkdir(QString("%1/log").arg(tempProblemDir()));
}

LogWidget::~LogWidget()
{    
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
    print("Agros", tr("version: %1").arg(QApplication::applicationVersion()), "green");
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

LogView::LogView(QWidget *parent, ConnectLog *connectLog) : QWidget(parent),
  m_connectLog(connectLog)
{
    setObjectName("LogView");

    actLog = new QAction(icon("log"), tr("Log"), this);
    actLog->setShortcut(Qt::Key_F10);
    actLog->setCheckable(true);

    logWidget = new LogWidget(this, connectLog);
    logWidget->welcomeMessage();

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 2);
    layoutMain->addWidget(logWidget);

    setLayout(layoutMain);
}

LogView::~LogView()
{        
}

LogDialog::LogDialog(Computation *computation, const QString &title, ConnectLog *connectLog) : QDialog(QApplication::activeWindow()),
    m_computation(computation),
    nonlinearChart(nullptr), nonlinearErrorSeries(nullptr), nonlinearProgress(nullptr),
    adaptivityChart(nullptr), adaptivityErrorSeries(nullptr), adaptivityDOFsSeries(nullptr), adaptivityProgress(nullptr),
    timeChart(nullptr), timeTimeStepSeries(nullptr), timeTimeTotalSeries(nullptr), timeProgress(nullptr),    
    m_connectLog(connectLog)
{
    setModal(true);

    setWindowTitle(title);
    setAttribute(Qt::WA_DeleteOnClose);

    createControls();

    int w = 1.0/2.0 * QGuiApplication::primaryScreen()->availableGeometry().width();
    int h = 1.0/2.0 * QGuiApplication::primaryScreen()->availableGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

void LogDialog::abortSolving()
{
    m_computation->abortSolving();
}

void LogDialog::closeEvent(QCloseEvent *e)
{
    if (m_computation->isMeshing() || m_computation->isSolving())
        e->ignore();
}

void LogDialog::reject()
{
    if (m_computation->isMeshing() || m_computation->isSolving())
        m_computation->abortSolving();
    else
        close();
}

LogDialog::~LogDialog()
{    
}

void LogDialog::createControls()
{

    connect(m_connectLog, SIGNAL(errorMsg(QString, QString)), this, SLOT(printError(QString, QString)));
    connect(m_connectLog, SIGNAL(updateNonlinearChart(SolverAgros::Phase, const QVector<double>, const QVector<double>)),
           this, SLOT(updateNonlinearChartInfo(SolverAgros::Phase, const QVector<double>, const QVector<double>)));
    connect(m_connectLog, SIGNAL(updateAdaptivityChart(const FieldInfo *, int, int)), this, SLOT(updateAdaptivityChartInfo(const FieldInfo *, int, int)));
    connect(m_connectLog, SIGNAL(updateTransientChart(double)), this, SLOT(updateTransientChartInfo(double)));

    logWidget = new LogWidget(this, m_connectLog);

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

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();

    // transient
    if (m_computation->isTransient())
    {
        timeChart = new QChart();
        timeChart->legend()->hide();
        timeChart->setTitle(tr("Transient problem"));

        // axis x
        axisX = new QValueAxis;
        axisX->setLabelFormat("%g");
        axisX->setGridLineVisible(true);
        axisX->setTitleText(tr("number of steps"));
        timeChart->addAxis(axisX, Qt::AlignBottom);

        // axis y
        axisSteps = new QValueAxis;
        axisSteps->setLabelFormat("%g");
        axisSteps->setGridLineVisible(true);
        axisSteps->setTitleText(tr("length of steps"));
        timeChart->addAxis(axisSteps, Qt::AlignRight);

        axisTotal = new QValueAxis;
        axisTotal->setLabelFormat("%g");
        axisTotal->setGridLineVisible(true);
        axisTotal->setTitleText(tr("total time"));
        timeChart->addAxis(axisTotal, Qt::AlignLeft);

        // zeroSeries = new QLineSeries();
        // zeroSeries->attachAxis(axisX);

        // attach axis
        timeTimeStepSeries = new QLineSeries();
        timeChart->addSeries(timeTimeStepSeries);
        timeTimeStepSeries->attachAxis(axisX);
        timeTimeStepSeries->attachAxis(axisSteps);

        timeTimeTotalSeries = new QLineSeries();
        timeChart->addSeries(timeTimeTotalSeries);
        timeTimeTotalSeries->attachAxis(axisX);
        timeTimeTotalSeries->attachAxis(axisTotal);

        //        auto seriesStep = new QAreaSeries(zeroSeries, timeTimeStepSeries);
        //        seriesStep->setOpacity(0.5);
        //        seriesStep->setPen(pen);
        //        timeChart->addSeries(seriesStep);

        //        auto seriesTotal = new QAreaSeries(zeroSeries, timeTimeTotalSeries);
        //        seriesTotal->setOpacity(0.5);
        //        seriesTotal->setPen(pen);
        //        timeChart->addSeries(seriesTotal);

        timeProgress = new QProgressBar(this);
        timeProgress->setMaximum(10000);

        QChartView *chartView = new QChartView();
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setChart(timeChart);

        QVBoxLayout *layoutTime = new QVBoxLayout();
        layoutTime->addWidget(chartView, 1);
        layoutTime->addWidget(timeProgress);

        layoutHorizontal->addLayout(layoutTime, 1);
    }

    // nonlinear
    if (m_computation->determineIsNonlinear())
    {
        nonlinearChart = new QChart();
        nonlinearChart->legend()->hide();
        nonlinearChart->setTitle(tr("Nonlinear solver"));

        // axis x
        axisX = new QValueAxis;
        axisX->setLabelFormat("%g");
        axisX->setGridLineVisible(true);
        axisX->setTitleText(tr("number of iterations"));
        nonlinearChart->addAxis(axisX, Qt::AlignBottom);

        // axis y
        nonlinearAxis = new QLogValueAxis;
        nonlinearAxis->setLabelFormat("%g");
        nonlinearAxis->setGridLineVisible(true);
        nonlinearAxis->setTitleText(tr("rel. change of sln. (%)"));
        nonlinearChart->addAxis(nonlinearAxis, Qt::AlignLeft);

        // zeroSeries = new QLineSeries();
        // zeroSeries->attachAxis(axisX);

        // attach axis
        nonlinearErrorSeries = new QLineSeries();
        nonlinearChart->addSeries(nonlinearErrorSeries);
        nonlinearErrorSeries->attachAxis(axisX);
        nonlinearErrorSeries->attachAxis(nonlinearAxis);       

    //        auto seriesError = new QAreaSeries(zeroSeries, nonlinearErrorSeries);
    //        seriesError->setOpacity(0.5);
    //        seriesError->setPen(pen);
    //        nonlinearChart->addSeries(seriesError);

        nonlinearProgress = new QProgressBar(this);
        nonlinearProgress->setMaximum(10000);

        QChartView *chartView = new QChartView();
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setChart(nonlinearChart);

        QVBoxLayout *layoutNonlinear = new QVBoxLayout();
        layoutNonlinear->addWidget(chartView, 1);
        layoutNonlinear->addWidget(nonlinearProgress);

        layoutHorizontal->addLayout(layoutNonlinear, 1);
    }

    // adaptivity
    if (m_computation->numAdaptiveFields() > 0)
    {
        adaptivityChart = new QChart();
        adaptivityChart->legend()->hide();
        adaptivityChart->setTitle(tr("Adaptivity"));

        // axis x
        axisX = new QValueAxis;
        axisX->setLabelFormat("%g");
        axisX->setGridLineVisible(true);
        axisX->setTitleText(tr("number of iterations"));
        adaptivityChart->addAxis(axisX, Qt::AlignBottom);

        // axis y
        axisError = new QLogValueAxis;
        axisError->setLabelFormat("%g");
        axisError->setGridLineVisible(true);
        axisError->setTitleText(tr("error"));
        adaptivityChart->addAxis(axisError, Qt::AlignRight);

        axisDOFs = new QValueAxis;
        axisDOFs->setLabelFormat("%g");
        axisDOFs->setGridLineVisible(true);
        axisDOFs->setTitleText(tr("number of DOFs"));
        adaptivityChart->addAxis(axisDOFs, Qt::AlignLeft);

        // zeroSeries = new QLineSeries();
        // zeroSeries->attachAxis(axisX);

        // attach axis
        adaptivityErrorSeries = new QLineSeries();
        adaptivityChart->addSeries(adaptivityErrorSeries);
        adaptivityErrorSeries->attachAxis(axisX);
        adaptivityErrorSeries->attachAxis(axisError);

        adaptivityDOFsSeries = new QLineSeries();
        adaptivityChart->addSeries(adaptivityDOFsSeries);
        adaptivityDOFsSeries->attachAxis(axisX);
        adaptivityDOFsSeries->attachAxis(axisDOFs);

        //        auto seriesDOFs = new QAreaSeries(zeroSeries, adaptivityDOFsSeries);
        //        seriesDOFs->setOpacity(0.5);
        //        seriesDOFs->setPen(pen);
        //        seriesDOFs->attachAxis(axisX);
        //        seriesDOFs->attachAxis(axisDOFs);
        //        adaptivityChart->addSeries(seriesDOFs);

        //        auto seriesError = new QAreaSeries(zeroSeries, adaptivityErrorSeries);
        //        seriesError->setOpacity(0.5);
        //        seriesError->setPen(pen);
        //        seriesError->attachAxis(axisX);
        //        seriesError->attachAxis(axisError);
        //        adaptivityChart->addSeries(seriesError);

        adaptivityProgress = new QProgressBar(this);
        adaptivityProgress->setMaximum(10000);

        QChartView *chartView = new QChartView();
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setChart(adaptivityChart);

        QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
        layoutAdaptivity->addWidget(chartView, 3);
        layoutAdaptivity->addWidget(adaptivityProgress);

        layoutHorizontal->addLayout(layoutAdaptivity, 1);
    }

    QVBoxLayout *layout = new QVBoxLayout();
    if (!layoutHorizontal->isEmpty())
        layout->addLayout(layoutHorizontal, 4);
    else
        delete layoutHorizontal;
    layout->addWidget(logWidget, 1);
    layout->addStretch();
    layout->addLayout(layoutStatus);

    setLayout(layout);
}

void LogDialog::printError(const QString &module, const QString &message)
{
    btnAbort->setEnabled(false);
    btnClose->setEnabled(true);

    logWidget->setVisible(true);
}

void LogDialog::updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions)
{
    if (!nonlinearErrorSeries)
        return;

    // block signals
    nonlinearChart->blockSignals(true);

    // zeroSeries->clear();
    nonlinearErrorSeries->clear();
    for (int i = 0; i < steps.size(); i++)
    {
        // zeroSeries->append(steps[i], 0.0);
        nonlinearErrorSeries->append(steps[i], relativeChangeOfSolutions[i]);
    }

    // fit
    fitToDataChart(nonlinearChart);

    // unblock signals
    nonlinearChart->blockSignals(false);

    // progress bar
    if (phase == SolverAgros::Phase_Finished)
    {
        nonlinearProgress->setValue(10000);
    }
    else
    {
        double valueRelativeChange = pow(10000.0, ((relativeChangeOfSolutions.first() - relativeChangeOfSolutions.last()) / relativeChangeOfSolutions.first()));
        nonlinearProgress->setValue(valueRelativeChange);
    }
}

void LogDialog::updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    if (!adaptivityErrorSeries)
        return;

    if (!adaptivityDOFsSeries)
        return;

    // block signals
    adaptivityChart->blockSignals(true);

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

    // zeroSeries->clear();
    adaptivityDOFsSeries->clear();
    adaptivityErrorSeries->clear();
    for (int i = 0; i < adaptiveSteps.size(); i++)
    {
        // zeroSeries->append(adaptiveSteps[i], 0.0);
        adaptivityDOFsSeries->append(adaptiveSteps[i], adaptiveDOFs[i]);
        adaptivityErrorSeries->append(adaptiveSteps[i], adaptiveError[i]);
    }
    axisX->setTickCount(adaptiveSteps.size());

//    QPen pen;
//    pen.setWidth(2);

//    auto seriesDOFs = new QAreaSeries(zeroSeries, adaptivityDOFsSeries);
//    seriesDOFs->setOpacity(0.5);
//    seriesDOFs->setPen(pen);
//    seriesDOFs->attachAxis(axisX);
//    seriesDOFs->attachAxis(axisDOFs);
//    auto seriesError = new QAreaSeries(zeroSeries, adaptivityErrorSeries);
//    seriesError->setOpacity(0.5);
//    seriesError->setPen(pen);
//    seriesError->attachAxis(axisX);
//    seriesError->attachAxis(axisError);

//    if (adaptivityChart->series().count() > 0)
//        adaptivityChart->removeAllSeries();
//    adaptivityChart->addSeries(seriesDOFs);
//    adaptivityChart->addSeries(seriesError);

    // fit
    fitToDataChart(adaptivityChart);

    // unblock signals
    adaptivityChart->blockSignals(false);

    // progress bar
    double valueSteps = 10000.0 * ((double) adaptivityStep / fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());
    adaptivityProgress->setValue(valueSteps);
}

void LogDialog::updateTransientChartInfo(double actualTime)
{
    if (!timeTimeStepSeries)
        return;

    if (!timeTimeTotalSeries)
        return;

    // block signals
    timeChart->blockSignals(true);

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

    // zeroSeries->clear();
    timeTimeStepSeries->clear();
    timeTimeTotalSeries->clear();
    for (int i = 0; i < timeLengths.size(); i++)
    {
        // zeroSeries->append(timeSteps[i], 0.0);
        timeTimeStepSeries->append(timeSteps[i], timeLengths[i]);
        timeTimeTotalSeries->append(timeSteps[i], timeTotal[i]);
    }

    // fit
    fitToDataChart(timeChart);

    // unblock signals
    timeChart->blockSignals(false);

    // progress bar
    timeProgress->setValue((10000.0 * actualTime / m_computation->config()->value(ProblemConfig::TimeTotal).toDouble()));
}

void LogDialog::closeLog()
{
    if (m_computation->isSolving())
    {
        Agros::log()->printError(tr("Solver"), tr("Solution is being aborted."));
    }
    else
    {
        close();
    }
}

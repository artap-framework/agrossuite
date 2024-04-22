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

#ifndef GUI_LOGWIDGET_H
#define GUI_LOGWIDGET_H

#include "util/util.h"
#include "gui/other.h"
#include "logview.h"
#include <QtCharts>

class LogWidget;
class ConnectLog;

#define signals public

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    LogWidget(QWidget *parent = 0, ConnectLog *connectLog = 0);
    ~LogWidget();
    void setConnectLog(ConnectLog * connectLog) { connectLog = connectLog; }

    void welcomeMessage();

public slots:
    void clear();

protected:
    void print(const QString &module, const QString &message,
               const QString &color = "");

    void ensureCursorVisible();

private:
    QMenu *mnuInfo;

    QTextEdit *plainLog;

    QAction *actShowTimestamp;
    QAction *actShowDebug;
    QAction *actClear;

    int printCounter;

    ConnectLog *m_connectLog;


    void createActions();

private slots:
    void contextMenu(const QPoint &pos);

    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);
    void printHeading(const QString &message);

    void appendImage(const QString &fileName);
    void appendHtml(const QString &html);

    void showTimestamp();
    void showDebug();
};

class LogView : public QWidget
{
    Q_OBJECT
public:
    LogView(QWidget *parent = 0, ConnectLog *connectLog = 0);
    ~LogView();

    QAction *actLog;    

private:
    LogWidget *logWidget;

    ConnectLog *m_connectLog;
};

class LogDialog : public QDialog
{
    Q_OBJECT
public:
    LogDialog(Computation *computation, const QString &title = tr("Progress..."), ConnectLog * connectLog = 0);
    ~LogDialog();

public slots:
    void closeLog();
    void abortSolving();

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void reject();

private:
    LogWidget *logWidget;

    QPushButton *btnClose;
    QPushButton *btnAbort;

    QValueAxis *axisX;
    // QLineSeries *zeroSeries;

    QChart *nonlinearChart;
    QProgressBar *nonlinearProgress;
    QLineSeries *nonlinearErrorSeries;
    QLogValueAxis *nonlinearAxis;

    QProgressBar *adaptivityProgress;
    QChart *adaptivityChart;
    QLineSeries *adaptivityErrorSeries;
    QLineSeries *adaptivityDOFsSeries;
    QValueAxis *axisDOFs;
    QLogValueAxis *axisError;

    QProgressBar *timeProgress;
    QChart *timeChart;
    QLineSeries *timeTimeStepSeries;
    QLineSeries *timeTimeTotalSeries;
    QValueAxis *axisSteps;
    QValueAxis *axisTotal;

    // log
    ConnectLog *m_connectLog;

    // computation
    Computation *m_computation;

    void createControls();

private slots:
    void printError(const QString &module, const QString &message);

    void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions);
    void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep);
    void updateTransientChartInfo(double actualTime);  
};

class ConnectLog : public QObject
{
    Q_OBJECT

public:
    ConnectLog() {}

signals:
    void headingMsg(const QString &message);
    void messageMsg(const QString &module, const QString &message);
    void errorMsg(const QString &module, const QString &message);
    void warningMsg(const QString &module, const QString &message);
    void debugMsg(const QString &module, const QString &message);

    void updateNonlinearChart(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions);
    void updateAdaptivityChart(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep);
    void updateTransientChart(double actualTime);

    void appendImg(const QString &fileName);
    void appendHtm(const QString &html);
};

class LogGui : public Log
{
public:
    LogGui();

    void setConnectLog(ConnectLog *connectLog) { m_connectLog = connectLog; }
    void printHeading(const QString &message);
    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);

    inline void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions) {emit m_connectLog->updateNonlinearChart(phase, steps, relativeChangeOfSolutions);}
    inline void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) {emit m_connectLog->updateAdaptivityChart(fieldInfo, timeStep, adaptivityStep);}
    inline void updateTransientChartInfo(double actualTime) { emit m_connectLog->updateTransientChart(actualTime);}

    void appendImage(const QString &fileName) {emit m_connectLog->appendImg(fileName);}
    void appendHtml(const QString &html) {emit m_connectLog->appendHtm(html);}

protected:
    ConnectLog *m_connectLog;
};

#endif // GUI_LOGWIDGET_H

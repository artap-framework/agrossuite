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

class QCustomPlot;
class QCPGraph;
class LogWidget;
class ConnectLog;

#define signals public

class LogConfigWidget : public QWidget
{
    Q_OBJECT

public:
    LogConfigWidget(LogWidget *logWidget);

private:
    LogWidget *m_logWidget;
};

class LogWidget : public QWidget
{
    Q_OBJECT
public:
    LogWidget(QWidget *parent = 0, ConnectLog *connectLog = 0);
    ~LogWidget();
    void setConnectLog(ConnectLog * connectLog) { m_connectLog = connectLog; }

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

    int m_printCounter;

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

    inline LogConfigWidget *logConfigWidget() { return m_logConfigWidget; }

private:
    LogWidget *m_logWidget;
    LogConfigWidget *m_logConfigWidget;

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
    LogWidget *m_logWidget;

    QPushButton *btnClose;
    QPushButton *btnAbort;

    QCustomPlot *m_nonlinearChart;
    QProgressBar *m_nonlinearProgress;
    QCPGraph *m_nonlinearErrorGraph;

    QCustomPlot *m_adaptivityChart;
    QProgressBar *m_adaptivityProgress;
    QCPGraph *m_adaptivityErrorGraph;
    QCPGraph *m_adaptivityDOFsGraph;

    QCustomPlot *m_timeChart;
    QProgressBar *m_timeProgress;
    QCPGraph *m_timeTimeStepGraph;
    QCPGraph *m_timeTimeTotalGraph;

    QListWidget *m_progress;

    Computation *m_computation;

    ConnectLog *m_connectLog;

    void createControls();

private slots:
    void printError(const QString &module, const QString &message);

    void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions);
    void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep);
    void updateTransientChartInfo(double actualTime);

    void addIcon(const QIcon &icn, const QString &label);
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

    void addIcon(const QIcon &icn, const QString &label);
};

class LogGui : public Log
{
public:
    LogGui();

    void setConnectLog(ConnectLog *connectLog) { m_connectLog = connectLog; }
    void printHeading(const QString &message) {emit m_connectLog->headingMsg(message);}
    void printMessage(const QString &module, const QString &message){emit m_connectLog->messageMsg(module, message);}
    void printError(const QString &module, const QString &message){emit m_connectLog->errorMsg(module, message);}
    void printWarning(const QString &module, const QString &message){emit m_connectLog->warningMsg(module, message);}
    void printDebug(const QString &module, const QString &message){emit m_connectLog->debugMsg(module, message);}

    inline void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions) {emit  m_connectLog->updateNonlinearChart(phase, steps, relativeChangeOfSolutions);}
    inline void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) {emit m_connectLog->updateAdaptivityChart(fieldInfo, timeStep, adaptivityStep);}
    inline void updateTransientChartInfo(double actualTime) { emit m_connectLog->updateTransientChart(actualTime);}

    void appendImage(const QString &fileName) {emit m_connectLog->appendImg(fileName);}
    void appendHtml(const QString &html) {emit m_connectLog->appendHtm(html);}

    inline void addIcon(const QIcon &icn, const QString &label) {emit m_connectLog->addIcon(icn, label);}

protected:
    ConnectLog *m_connectLog;
};

#endif // GUI_LOGWIDGET_H

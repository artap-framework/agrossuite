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



class LogConfigWidget : public QWidget
{
    Q_OBJECT

public:
    LogConfigWidget(LogWidget *logWidget);

private:
    LogWidget *m_logWidget;
};

class AGROS_LIBRARY_API LogWidget : public QWidget
{
    Q_OBJECT
public:
    LogWidget(QWidget *parent = 0);
    ~LogWidget();

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

    void createActions();


private slots:
    void contextMenu(const QPoint &pos);
    void showTimestamp();
    void showDebug();

public: // ToDo: Improve
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);
    void printHeading(const QString &message);
    void printMessage(const QString &module, const QString &message);
    void appendImage(const QString &fileName);
    void appendHtml(const QString &html);

};

class AGROS_LIBRARY_API LogView : public QWidget
{
    Q_OBJECT
public:
    LogView(QWidget *parent = 0);
    ~LogView();

    QAction *actLog;

    inline LogConfigWidget *logConfigWidget() { return m_logConfigWidget; }

private:
    LogWidget *m_logWidget;
    LogConfigWidget *m_logConfigWidget;
};

class AGROS_LIBRARY_API LogDialog : public QDialog
{
    Q_OBJECT
public:
    LogDialog(Computation *computation, const QString &title = tr("Progress..."));
    ~LogDialog();

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

    void createControls();

public:
    void printError(const QString &module, const QString &message);

    void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions);
    void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep);
    void updateTransientChartInfo(double actualTime);

    void addIcon(const QIcon &icn, const QString &label);

private slots:
    void tryClose();
};

class AGROS_LIBRARY_API LogGui : public Log
{
      QVector<LogWidget * > m_logWidgets;
      LogDialog *m_logDialog;

public:
    LogGui();
    void setWidget(LogWidget * logWidget);
    void removeWidget(LogWidget * logWidget);
    void setDialog(LogDialog * logDialog) { m_logDialog = logDialog; }
    void printHeading(const QString &message);
    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);

    inline void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions)
    { m_logDialog->updateNonlinearChartInfo(phase, steps, relativeChangeOfSolutions); }
    inline void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) { m_logDialog->updateAdaptivityChartInfo( fieldInfo, timeStep, adaptivityStep);}
    inline void updateTransientChartInfo(double actualTime) { m_logDialog->updateTransientChartInfo( actualTime);}

    void appendImage(const QString &fileName);
    void appendHtml(const QString &html);

    inline void addIcon(const QIcon &icn, const QString &label) { m_logDialog->addIcon(icn, label);}

};
#endif // GUI_LOGWIDGET_H

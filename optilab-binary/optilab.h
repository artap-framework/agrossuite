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

#ifndef OPTILABDIALOG_H
#define OPTILABDIALOG_H

#include "util.h"
#include "optilab_data.h"

#include "qcustomplot/qcustomplot.h"

class LogWidget;
class PythonEditorAgrosDialog;

class OptilabWindow : public QMainWindow
{
    Q_OBJECT
public:
    OptilabWindow(int argc, char *argv[]);
    ~OptilabWindow();

    void showDialog();
    inline void setStartupProblemFilename(const QString& fn) { m_startupProblemFilename = fn; }

public slots:


private slots:
    void openProblemAgros2D();

    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void refreshVariants();
    void loadVariant(const QString &fileName);

    void variantOpenInExternalAgros2D();
    void variantSolveInExternalSolver();

    void variantOpenInAgros2D();
    void variantSolveInAgros2D();

    void processOpenError(QProcess::ProcessError error);
    void processOpenFinished(int exitCode);
    void processSolveError(QProcess::ProcessError error);
    void processSolveFinished(int exitCode);

    void doScriptEditor();

private:
    QString m_startupProblemFilename;

    QTreeWidget *trvVariants;
    QLabel *lblProblems;

    QWebView *webView;
    QString m_cascadeStyleSheet;

    QSplitter *splitter;

    QPushButton *btnOpenInAgros2D;
    QPushButton *btnSolveInAgros2D;
    QPushButton *btnSolveInExternalSolver;
    QPushButton *btnOpenInExternalAgros2D;

    QAction *actRefresh;
    QAction *actScriptEditor;

    LogWidget *logWidget;

    PythonEditorAgrosDialog *scriptEditorDialog;

    void createActions();
    void createToolBars();
    void createMain();
};

#endif // OPTILABDIALOG_H

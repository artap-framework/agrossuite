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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gui/other.h"
#include "util/util.h"
#include "util/enums.h"
#include "gui/logwidget.h"

class SceneViewPost2D;
class SceneViewPost3D;
class SceneViewMesh;
class OptiLab;
class SettingsWidget;
class ExamplesWidget;
class ResultsView;
class VolumeIntegralValueView;
class SurfaceIntegralValueView;
class PreprocessorWidget;
class PostprocessorWidget;
class TooltipView;
class LogView;

class SceneTransformDialog;
class SceneViewWidget;
class LogStdOut;
class Computation;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char *argv[], QWidget *parent = 0);
    ~MainWindow();

    inline void setStartupExecute(bool execute) { m_startupExecute = execute; }
    inline void setStartupProblemFilename(const QString& fn) { m_startupProblemFilename = fn; }

private slots:
    void doDocumentNew();
    void doDocumentOpen(const QString &fileName = "");
    void doDocumentOpenRecent(QAction *action);
    void doDocumentSave();
    void doDocumentSaveAs();
    void doDocumentClose();
    void doDocumentImportDXF();
    void doDocumentExportDXF();
    void doCreatePythonFromModel();

    void doSolveCurrentComputation();
    void doSolveNewComputation();
    void doSolveComputation(Computation *computation);
    void doSolveFinished();
    void doDeleteSolutions();

    void doOptions();
    void doMaterialBrowser();
    void doShowLog();

    void doFullScreen();

    void doCheckVersion();
    void doAbout();
    void doApplyStyle();
    void setEnabledControls(bool state);
    void setControls();
    void clear();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void showEvent(QShowEvent *event);    

private:
    QMenu *mnuRecentFiles;
    QMenu *mnuCustomForms;

    QMenu *mnuFile;
    QMenu *mnuEdit;
    QMenu *mnuTools;
    QMenu *mnuHelp;

    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    QAction *actDocumentSave;    
    QAction *actDocumentSaveAs;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actCreateFromModel;

    QAction *actExit;
    QActionGroup *actDocumentOpenRecentGroup;

    QAction *actCopy;

    QAction *actOptions;
    QAction *actFullScreen;
    QAction *actShowLog;

    QAction *actMaterialBrowser;

    QAction *actSolve;
    QAction *actSolveNewComputation;
    QAction *actDeleteSolutions;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actCheckVersion;
    QAction *actAbout;

    QMenu *mnuMesh;
    QToolButton *btnMesh;

    QComboBox *cmbTimeStep;

    // scene mode
    QActionGroup *actSceneModeGroup;

    OptiLab *optiLab;

    QWidget *viewControls;
    QStackedLayout *tabControlsLayout;
    ExamplesWidget *exampleWidget;
    PreprocessorWidget *problemWidget;
    PostprocessorWidget *postprocessorWidget;

    LogViewDialog *logView;
    ConnectLog *m_connectLog;

    LogStdOut *logStdOut;
    LogDialog *logDialog;

    QString m_startupProblemFilename;
    bool m_startupExecute;
    QTimer *timerApplyStyle;

    void setRecentFiles();

    void createActions();
    void createToolBox();
    void createMenus();
    void createMain();
};

#endif // MAINWINDOW_H

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

#include "util.h"
#include "util/enums.h"

class PostDeal;
class SceneViewPost2D;
class SceneViewPost3D;
class SceneViewPreprocessor;
class SceneViewMesh;
class OptiLab;
class InfoWidget;
class SettingsWidget;
class ProblemWidget;
class ResultsView;
class VolumeIntegralValueView;
class SurfaceIntegralValueView;
class PreprocessorWidget;
class PostprocessorWidget;
class PythonScriptingConsoleView;
class TooltipView;
class LogView;

class PythonEditorDialog;
class SceneTransformDialog;
class SceneViewWidget;
class LogStdOut;
class Computation;

class AGROS_LIBRARY_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char *argv[], QWidget *parent = 0);
    ~MainWindow();

    inline void setStartupExecute(bool execute) { m_startupExecute = execute; }
    inline void setStartupProblemFilename(const QString& fn) { m_startupProblemFilename = fn; }
    inline void setStartupScriptFilename(const QString& fn) { m_startupScriptFilename = fn; }

private slots:
    inline void doSetWindowTitle(const QString &name) { setWindowTitle("Agros2D - " + name); }

    void doDocumentNew();
    void doDocumentOpen(const QString &fileName = "");
    void doDocumentOpenForm(const QString &fileName, const QString &formName);
    void doDocumentOpenRecent(QAction *action);
    void doDocumentSave();
    void doDocumentSaveAs();    
    void doDeleteSolution();
    void doDocumentClose();
    void doDocumentImportDXF();
    void doDocumentExportDXF();
    void doDocumentExportMeshFile();
    void doExportVTKGeometry();
    void doDocumentSaveImage();
    void doDocumentSaveGeometry();    
    void doExamples(const QString &groupName = "");

    void doMouseSceneModeChanged(MouseSceneMode mouseSceneMode);

    void doMesh();
    void doSolve();
    void doSolveNewComputation();
    void doSolveFinished();

    void doOptions();
    void doTransform();
    void doMaterialBrowser();
    void doCreateVideo();
    void doUnitTests();

    void doHideControlPanel();
    void doFullScreen();

    void doCut();
    void doCopy();
    void doPaste();

    void doHelp();
    void doHelpShortCut();
    void doCheckVersion();
    void doAbout();
    void doStartedScript();
    void doExecutedScript();
    void setEnabledControls(bool state);
    void setControls();
    void clear();

    /// fields added or removed, menus need to be modified
    void doFieldsChanged();

    void connectComputation(QSharedPointer<Computation> computation);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QStringList recentFiles;

    QMenu *mnuRecentFiles;
    QMenu *mnuProblemAddBoundaryAndMaterial;
    QMenu *mnuCustomForms;

    QMenu *mnuFile;
    QMenu *mnuEdit;
    QMenu *mnuProblem;
    QMenu *mnuTools;
    QMenu *mnuSettings;
    QMenu *mnuHelp;

    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    QAction *actDocumentSave;    
    QAction *actDocumentSaveAs;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actDocumentExportMeshFile;
    QAction *actDocumentSaveImage;
    QAction *actDocumentSaveGeometry;
    QAction *actExportVTKGeometry;

    QAction *actExamples;
    QAction *actExit;
    QActionGroup *actDocumentOpenRecentGroup;

    QAction *actHideControlPanel;

    QAction *actCopy;

    QAction *actOptions;
    QAction *actFullScreen;

    QAction *actMaterialBrowser;
    QAction *actCreateVideo;
    QAction *actUnitTests;

    QAction *actMesh;
    QAction *actSolve;
    QAction *actSolveNewComputation;
    QAction *actDeleteSolution;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actOnlineHelp;
    // QAction *actCollaborationServer;
    QAction *actCheckVersion;
    QAction *actAbout;
    QAction *actAboutQt;

    QMenu *mnuMesh;
    QToolButton *btnMesh;

    QComboBox *cmbTimeStep;

    // pointers to actions (geometry, post2d and post3d)
    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;
    QAction *actSceneZoomRegion;

    // scene mode
    QActionGroup *actSceneModeGroup;

    SceneViewWidget *sceneViewInfoWidget;
    SceneViewWidget *sceneViewPreprocessorWidget;
    SceneViewWidget *sceneViewMeshWidget;
    SceneViewWidget *sceneViewPost2DWidget;
    SceneViewWidget *sceneViewPost3DWidget;
    SceneViewWidget *sceneViewPostParticleTracingWidget;
    SceneViewWidget *sceneViewChartWidget;
    SceneViewWidget *sceneViewPythonEditorWidget;
    SceneViewWidget *sceneViewOptilabWidget;

    QStackedLayout *tabViewLayout;
    InfoWidget *sceneInfoWidget;
    SceneViewPreprocessor *sceneViewPreprocessor;
    PythonEditorDialog *scriptEditor;
    OptiLab *optiLab;

    QWidget *viewControls;
    QStackedLayout *tabControlsLayout;
    ProblemWidget *problemWidget;
    PreprocessorWidget *preprocessorWidget;
    PostprocessorWidget *postprocessorWidget;

    ResultsView *resultsView;
    PythonScriptingConsoleView *consoleView;
    LogView *logView;

    SceneTransformDialog *sceneTransformDialog;

    QSplitter *splitter;

    LogStdOut *logStdOut;

    QString m_startupProblemFilename;
    QString m_startupScriptFilename;
    bool m_startupExecute;

    // TODO: remove
    QSharedPointer<Computation> m_computation;

    void setRecentFiles();

    void createActions();
    void createToolBox();
    void createMenus();
    void createToolBars();
    void createMain();
    void createViews();   
};

#endif // MAINWINDOW_H

// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "util.h"

class SceneView;
class SceneInfoView;
class LocalPointValueView;
class VolumeIntegralValueView;
class SurfaceIntegralValueView;
class PostprocessorView;
class PythonScriptingConsoleView;
class TooltipView;

class HelpDialog;
class ChartDialog;
class PythonLabAgros;
class ReportDialog;
class VideoDialog;
class LogDialog;
class ServerDownloadDialog;
class SceneTransformDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void processParameters();
    void open(const QString &fileName);

private slots:
    inline void doSetWindowTitle(const QString &name) { setWindowTitle("Agros2D - " + name); }

    void doDocumentNew();
    void doDocumentOpen(const QString &fileName = "");
    void doDocumentDownloadFromServer();
    void doDocumentOpenRecent(QAction *action);
    void doDocumentSave();
    void doDocumentSaveAs();
    void doDocumentUploadToServer();
    void doDocumentClose();
    void doDocumentImportDXF();
    void doDocumentExportDXF();
    void doDocumentExportMeshFile();
    void doDocumentSaveImage();
    void doLoadBackground();
    void doExportVTKScalar();
    void doExportVTKOrder();

    void doViewQuick2DNone();
    void doViewQuick2DOrder();
    void doViewQuick2DScalarView();
    void doViewQuick3DScalarView();
    void doViewQuick3DScalarViewSolid();
    void doViewQuick3DParticleTracing();
    void doViewQuick3DModel();
    void doViewQuick(SceneViewPostprocessorShow show);

    void doSceneMouseMoved(const QPointF &position);
    void doMouseSceneModeChanged(MouseSceneMode mouseSceneMode);

    void doCreateMesh();
    void doSolve();
    void doProgressLog();

    void doChart();
    void doScriptEditor();
    void doScriptEditorRunScript(const QString &fileName = "");
    void doScriptEditorRunCommand();
    void doOptions();
    void doTransform();
    void doReport();
    void doCreateVideo();
    void doFullScreen();
    void doApplicationLog();

    void doCut();
    void doCopy();
    void doPaste();

    void doHelp();
    void doHelpShortCut();
    void doCollaborationServer();
    void doOnlineHelp();
    void doCheckVersion();
    void doAbout();
    void doInvalidated();
    void doSceneModeChanged(SceneMode sceneMode);
    void doPostprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor);

    void doTimeStepChanged(int index);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void closeEvent(QCloseEvent *event);

private:
    QStringList recentFiles;

    QMenu *mnuFile;
    QMenu *mnuFileImportExport;
    QMenu *mnuRecentFiles;
    QMenu *mnuEdit;
    QMenu *mnuView;
    QMenu *mnuProblem;
    QMenu *mnuTools;
    QMenu *mnuHelp;

    QToolBar *tlbFile;
    QToolBar *tlbEdit;
    QToolBar *tlbView;
    QToolBar *tlbProblem;
    QToolBar *tlbTools;
    QToolBar *tlbTransient;

    QAction *actDocumentNew;
    QAction *actDocumentOpen;
    QAction *actDocumentDownloadFromServer;
    QAction *actDocumentSave;
    QAction *actDocumentSaveAs;
    QAction *actDocumentUploadToServer;
    QAction *actDocumentClose;
    QAction *actDocumentImportDXF;
    QAction *actDocumentExportDXF;
    QAction *actDocumentExportMeshFile;
    QAction *actExportVTKScalar;
    QAction *actExportVTKOrder;
    QAction *actDocumentSaveImage;
    QAction *actExit;
    QAction *actLoadBackground;
    QActionGroup *actDocumentOpenRecentGroup;

    QAction *actUndo;
    QAction *actRedo;
    QAction *actCopy;

    QAction *actOptions;
    QAction *actCreateMesh;
    QAction *actSolve;
    QAction *actProgressLog;
    QAction *actChart;
    QAction *actFullScreen;
    QAction *actPostprocessorView;

    QAction *actViewQuick2DNone;
    QAction *actViewQuick2DOrder;
    QAction *actViewQuick2DScalarView;
    QAction *actViewQuick3DScalarView;
    QAction *actViewQuick3DScalarViewSolid;
    QAction *actViewQuick3DParticleTracing;
    QAction *actViewQuick3DModel;

    QAction *actScriptEditor;
    QAction *actScriptEditorRunScript;
    QAction *actScriptEditorRunCommand;
    QAction *actReport;
    QAction *actCreateVideo;
    QAction *actApplicationLog;

    QAction *actHelp;
    QAction *actHelpShortCut;
    QAction *actOnlineHelp;
    QAction *actCollaborationServer;
    QAction *actCheckVersion;
    QAction *actAbout;
    QAction *actAboutQt;

    QLabel *lblProblemType;
    QLabel *lblPhysicField;
    QLabel *lblMessage;
    QLabel *lblPosition;
    QLabel *lblMouseMode;
    QLabel *lblAnalysisType;

    QComboBox *cmbTimeStep;

    SceneView *sceneView;
    SceneInfoView *sceneInfoView;
    LocalPointValueView *localPointValueView;
    VolumeIntegralValueView *volumeIntegralValueView;
    SurfaceIntegralValueView *surfaceIntegralValueView;
    PostprocessorView *postprocessorView;
    PythonScriptingConsoleView *consoleView;
    TooltipView *tooltipView;

    HelpDialog *helpDialog;
    ChartDialog *chartDialog;
    PythonLabAgros *scriptEditorDialog;
    ReportDialog *reportDialog;
    VideoDialog *videoDialog;
    LogDialog *logDialog;
    ServerDownloadDialog *collaborationDownloadDialog;
    SceneTransformDialog *sceneTransformDialog;

    void setRecentFiles();

    void createActions();
    void createToolBox();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createScene();
    void createViews();
};

#endif // MAINWINDOW_H

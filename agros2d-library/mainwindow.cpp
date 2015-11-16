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

#include "mainwindow.h"

#include "gui/about.h"
#include "gui/imageloader.h"
#include "gui/problemdialog.h"
#include "gui/fielddialog.h"
#include "util/global.h"
#include "util/checkversion.h"
#include "util/form_interface.h"

#include "scene.h"
#include "scenebasic.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "sceneview_particle.h"
// #include "sceneview_vtk2d.h"
#include "logview.h"
#include "gui/infowidget.h"
#include "preprocessorview.h"
#include "postprocessorview.h"
#include "chartdialog.h"
#include "confdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "pythonlab/python_unittests.h"
#include "optilab/optilab.h"
#include "videodialog.h"
#include "resultsview.h"
#include "materialbrowserdialog.h"
#include "solver/module.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "scenetransformdialog.h"
#include "chartdialog.h"
#include "examplesdialog.h"
#include "solver/solver.h"

#include "util/form_script.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent)
{
    setWindowIcon(icon("agros2d"));

    m_startupScriptFilename = "";
    m_startupProblemFilename = "";
    m_startupExecute = false;

    // log stdout
    logStdOut = NULL;
    if (Agros2D::configComputer()->value(Config::Config_LogStdOut).toBool())
        logStdOut = new LogStdOut();

    createPythonEngine(argc, argv, new PythonEngineAgros());

    // scene
    sceneViewPreprocessor = new SceneViewPreprocessor(this);
    // sceneViewVTK2D = new SceneViewVTK2D(postDeal, this);

    // scene - info widget
    sceneInfoWidget = new InfoWidget(this);
    sceneInfoWidget->setRecentProblemFiles(&recentFiles);
    connect(sceneInfoWidget, SIGNAL(open(QString)), this, SLOT(doDocumentOpen(QString)));
    connect(sceneInfoWidget, SIGNAL(openForm(QString, QString)), this, SLOT(doDocumentOpenForm(QString, QString)));
    connect(sceneInfoWidget, SIGNAL(examples(QString)), this, SLOT(doExamples(QString)));

    // preprocessor
    preprocessorWidget = new PreprocessorWidget(sceneViewPreprocessor, this);
    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), preprocessorWidget, SLOT(refresh()));
    // postprocessor
    postprocessorWidget = new PostprocessorWidget();

    connect(postprocessorWidget, SIGNAL(apply()), this, SLOT(setControls()));
    connect(postprocessorWidget, SIGNAL(modeChanged()), this, SLOT(setControls()));

    // problem
    mainWidget = new MainWidget(this);

    // view info
    consoleView = new PythonScriptingConsoleView(currentPythonEngine(), this);
    logView = new LogView(this);
    resultsView = new ResultsView(this);

    // PythonLab
    scriptEditor = new PythonEditorDialog(consoleView->console(), this);

    // OptiLab
    optiLab = new OptiLab(this);

    createActions();
    createMenus();
    createToolBars();
    createMain();

    // info
    sceneInfoWidget->setRecentScriptFiles(scriptEditor->recentFiles());
    sceneTransformDialog = new SceneTransformDialog(sceneViewPreprocessor, this);

    // python engine
    connect(currentPythonEngineAgros(), SIGNAL(startedScript()), this, SLOT(doStartedScript()));
    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(doExecutedScript()));

    connect(tabViewLayout, SIGNAL(currentChanged(int)), this, SLOT(setControls()));
    connect(Agros2D::problem()->scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(Agros2D::problem(), SIGNAL(fileNameChanged(QString)), this, SLOT(doSetWindowTitle(QString)));
    connect(Agros2D::problem()->scene()->actTransform, SIGNAL(triggered()), this, SLOT(doTransform()));

    connect(Agros2D::problem()->scene(), SIGNAL(cleared()), this, SLOT(clear()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(setControls()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), sceneViewPreprocessor, SLOT(refresh()));

    // preprocessor
    connect(sceneViewPreprocessor, SIGNAL(sceneGeometryModeChanged(SceneGeometryMode)), preprocessorWidget, SLOT(loadTooltip(SceneGeometryMode)));
    connect(currentPythonEngine(), SIGNAL(executedScript()), Agros2D::problem()->scene(), SLOT(doInvalidated()));
    connect(currentPythonEngine(), SIGNAL(executedScript()), Agros2D::problem()->scene()->loopsInfo(), SLOT(processPolygonTriangles()));
    currentPythonEngineAgros()->setSceneViewPreprocessor(sceneViewPreprocessor);

    // postprocessor 2d
    connect(postprocessorWidget->sceneViewPost2D(), SIGNAL(mousePressed()), resultsView, SLOT(doShowResults()));
    connect(postprocessorWidget->sceneViewPost2D(), SIGNAL(mousePressed(const Point &)), resultsView, SLOT(showPoint(const Point &)));
    connect(postprocessorWidget->sceneViewPost2D(), SIGNAL(postprocessorModeGroupChanged(SceneModePostprocessor)), resultsView, SLOT(doPostprocessorModeGroupChanged(SceneModePostprocessor)));

    // info
    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(doFieldsChanged()));

    // reconnect computation slots
    connect(Agros2D::singleton(), SIGNAL(connectComputation(QSharedPointer<Computation>)), this, SLOT(connectComputation(QSharedPointer<Computation>)));

    sceneViewPreprocessor->clear();

    QSettings settings;
    recentFiles = settings.value("MainWindow/RecentFiles").value<QStringList>();

    Agros2D::problem()->scene()->clear();

    mainWidget->actProperties->trigger();
    sceneViewPreprocessor->doZoomBestFit();

    // set recent files
    setRecentFiles();

    // accept drops
    setAcceptDrops(true);

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    checkForNewVersion(true);

    restoreGeometry(settings.value("MainWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("MainWindow/State", saveState()).toByteArray());
    splitterMain->restoreState(settings.value("MainWindow/SplitterMainState").toByteArray());
    splitterView->restoreState(settings.value("MainWindow/SplitterViewState").toByteArray());
    // show/hide control and view panel
    actHideControlPanel->setChecked(settings.value("MainWindow/ControlPanel", true).toBool());
    actHideViewPanel->setChecked(settings.value("MainWindow/ViewPanel", true).toBool());
    doHideControlPanel();
    doHideViewPanel();

    setControls();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/RecentFiles", recentFiles);
    settings.setValue("MainWindow/SplitterMainState", splitterMain->saveState());
    settings.setValue("MainWindow/SplitterViewState", splitterView->saveState());
    settings.setValue("MainWindow/ControlPanel", actHideControlPanel->isChecked());
    settings.setValue("MainWindow/ViewPanel", actHideViewPanel->isChecked());

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());

    delete logStdOut;
}

void MainWindow::createActions()
{
    actDocumentNew = new QAction(icon("document-new"), tr("&New..."), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(doDocumentNew()));

    actDocumentOpen = new QAction(icon("document-open"), tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(doDocumentOpen()));

    // actDocumentDownloadFromServer = new QAction(icon(""), tr("&Download from server..."), this);
    // actDocumentDownloadFromServer->setShortcut(tr("Ctrl+Shift+O"));
    // connect(actDocumentDownloadFromServer, SIGNAL(triggered()), this, SLOT(doDocumentDownloadFromServer()));

    actDocumentSave = new QAction(icon("document-save"), tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));

    actDeleteSolution = new QAction(icon(""), tr("Delete solution"), this);
    connect(actDeleteSolution, SIGNAL(triggered()), this, SLOT(doDeleteSolution()));

    actDocumentSaveAs = new QAction(icon("document-save-as"), tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));

    // actDocumentUploadToServer = new QAction(icon(""), tr("Upload to server..."), this);
    // connect(actDocumentUploadToServer, SIGNAL(triggered()), this, SLOT(doDocumentUploadToServer()));

    actDocumentClose = new QAction(tr("&Close"), this);
    actDocumentClose->setShortcuts(QKeySequence::Close);
    connect(actDocumentClose, SIGNAL(triggered()), this, SLOT(doDocumentClose()));

    actDocumentImportDXF = new QAction(tr("Import DXF..."), this);
    connect(actDocumentImportDXF, SIGNAL(triggered()), this, SLOT(doDocumentImportDXF()));

    actDocumentExportDXF = new QAction(tr("Export DXF..."), this);
    connect(actDocumentExportDXF, SIGNAL(triggered()), this, SLOT(doDocumentExportDXF()));

    actDocumentExportMeshFile = new QAction(tr("Export mesh file..."), this);
    connect(actDocumentExportMeshFile, SIGNAL(triggered()), this, SLOT(doDocumentExportMeshFile()));

    actExportVTKGeometry = new QAction(tr("Export VTK geometry..."), this);
    connect(actExportVTKGeometry, SIGNAL(triggered()), this, SLOT(doExportVTKGeometry()));

    actDocumentSaveImage = new QAction(tr("Export image..."), this);
    connect(actDocumentSaveImage, SIGNAL(triggered()), this, SLOT(doDocumentSaveImage()));

    actDocumentSaveGeometry = new QAction(tr("Export geometry..."), this);
    connect(actDocumentSaveGeometry, SIGNAL(triggered()), this, SLOT(doDocumentSaveGeometry()));

    actExamples = new QAction(tr("Open example..."), this);
    actExamples->setShortcut(tr("Ctrl+Shift+O"));
    connect(actExamples, SIGNAL(triggered()), this, SLOT(doExamples()));

    actCreateVideo = new QAction(icon("video"), tr("Create &video..."), this);
    connect(actCreateVideo, SIGNAL(triggered()), this, SLOT(doCreateVideo()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actCopy = new QAction(icon("edit-copy"), tr("Copy image to clipboard"), this);
    // actCopy->setShortcuts(QKeySequence::Copy);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopy()));

    actHelp = new QAction(icon("help-contents"), tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    // actHelp->setEnabled(false);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpShortCut = new QAction(icon(""), tr("&Shortcuts"), this);
    actHelpShortCut->setEnabled(false);
    connect(actHelpShortCut, SIGNAL(triggered()), this, SLOT(doHelpShortCut()));

    actCheckVersion = new QAction(icon(""), tr("Check version"), this);
    connect(actCheckVersion, SIGNAL(triggered()), this, SLOT(doCheckVersion()));

    actAbout = new QAction(icon("about"), tr("About &Agros2D"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actOptions = new QAction(icon("options"), tr("&Options"), this);
    actOptions->setMenuRole(QAction::PreferencesRole);
    connect(actOptions, SIGNAL(triggered()), this, SLOT(doOptions()));

    actUnitTests = new QAction(tr("Unit tests..."), this);
    connect(actUnitTests, SIGNAL(triggered()), this, SLOT(doUnitTests()));

    actFullScreen = new QAction(icon("view-fullscreen"), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actMaterialBrowser = new QAction(icon(""), tr("Material browser..."), this);
    connect(actMaterialBrowser, SIGNAL(triggered()), this, SLOT(doMaterialBrowser()));

    actMesh = new QAction(icon("scene-meshgen"), tr("&Mesh area"), this);
    actMesh->setShortcut(QKeySequence(tr("Alt+W")));
    connect(actMesh, SIGNAL(triggered()), this, SLOT(doMesh()));

    actSolve = new QAction(icon("run"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actSolveNewComputation = new QAction(icon("run"), tr("&Solve new"), this);
    actSolveNewComputation->setShortcut(QKeySequence(tr("Alt+Shift+S")));
    connect(actSolveNewComputation, SIGNAL(triggered()), this, SLOT(doSolveNewComputation()));

    // zoom actions (geometry, post2d and post3d)
    // scene - zoom
    actSceneZoomIn = new QAction(icon("zoom-in"), tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);

    actSceneZoomOut = new QAction(icon("zoom-out"), tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);

    actSceneZoomBestFit = new QAction(icon("zoom-original"), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));

    actSceneZoomRegion = new QAction(icon("zoom-fit-best"), tr("Zoom region"), this);
    actSceneZoomRegion->setCheckable(true);

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(mainWidget->actProperties);
    actSceneModeGroup->addAction(sceneViewPreprocessor->actSceneModePreprocessor);
    actSceneModeGroup->addAction(postprocessorWidget->actSceneModePost);
    actSceneModeGroup->addAction(scriptEditor->actSceneModePythonEditor);
    actSceneModeGroup->addAction(optiLab->actSceneModeOptiLab);

    actHideControlPanel = new QAction(icon("showhide"), tr("Show/hide control panel"), this);
    actHideControlPanel->setShortcut(tr("Alt+0"));
    actHideControlPanel->setCheckable(true);
    connect(actHideControlPanel, SIGNAL(triggered()), this, SLOT(doHideControlPanel()));

    actHideViewPanel = new QAction(icon("showhide"), tr("Show/hide view panel"), this);
    actHideViewPanel->setShortcut(tr("Alt+."));
    actHideViewPanel->setCheckable(true);
    connect(actHideViewPanel, SIGNAL(triggered()), this, SLOT(doHideViewPanel()));
}


void MainWindow::doFieldsChanged()
{
    // add boundaries and materials menu
    mnuProblemAddBoundaryAndMaterial->clear();
    Agros2D::problem()->scene()->addBoundaryAndMaterialMenuItems(mnuProblemAddBoundaryAndMaterial, this);
}

void MainWindow::connectComputation(QSharedPointer<Computation> computation)
{
    if (!m_computation.isNull())
    {
        disconnect(m_computation.data(), SIGNAL(meshed()), this, SLOT(setControls()));
        disconnect(m_computation.data(), SIGNAL(solved()), this, SLOT(setControls()));
        disconnect(m_computation.data(), SIGNAL(meshed()), this, SLOT(doSolveFinished()));
        disconnect(m_computation.data(), SIGNAL(solved()), this, SLOT(doSolveFinished()));
    }

    m_computation = computation;

    if (!m_computation.isNull())
    {
        connect(m_computation.data(), SIGNAL(meshed()), this, SLOT(setControls()));
        connect(m_computation.data(), SIGNAL(solved()), this, SLOT(setControls()));
        connect(m_computation.data(), SIGNAL(meshed()), this, SLOT(doSolveFinished()));
        connect(m_computation.data(), SIGNAL(solved()), this, SLOT(doSolveFinished()));
    }
}

void MainWindow::createMenus()
{
    menuBar()->clear();

    mnuRecentFiles = new QMenu(tr("&Recent files"), this);
    QMenu *mnuFileImportExport = new QMenu(tr("Import/Export"), this);
    mnuFileImportExport->addAction(actDocumentImportDXF);
    mnuFileImportExport->addAction(actDocumentExportDXF);
    mnuFileImportExport->addSeparator();
    mnuFileImportExport->addAction(actDocumentExportMeshFile);
    mnuFileImportExport->addAction(actDocumentSaveImage);
    mnuFileImportExport->addAction(actDocumentSaveGeometry);
    mnuFileImportExport->addSeparator();
    mnuFileImportExport->addAction(actExportVTKGeometry);
    mnuFileImportExport->addAction(postprocessorWidget->sceneViewMesh()->actExportVTKMesh);
    mnuFileImportExport->addAction(postprocessorWidget->sceneViewMesh()->actExportVTKOrder);
    // mnuFileImportExport->addAction(sceneViewPost2D->actExportVTKScalar);
    // mnuFileImportExport->addAction(sceneViewPost2D->actExportVTKContours);

    // QMenu *mnuServer = new QMenu(tr("Colaboration"), this);
    // mnuServer->addAction(actDocumentDownloadFromServer);
    // mnuServer->addAction(actDocumentUploadToServer);
    // mnuServer->addAction(actCollaborationServer);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actExamples);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentSave);
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addAction(actDeleteSolution);
    mnuFile->addSeparator();
    mnuFile->addMenu(mnuFileImportExport);
    // mnuFile->addMenu(mnuServer);
    mnuFile->addSeparator();
#ifndef Q_WS_MAC
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentClose);
    mnuFile->addAction(actExit);
#endif

    mnuEdit = menuBar()->addMenu(tr("E&dit"));
    mnuEdit->addAction(preprocessorWidget->actUndo);
    mnuEdit->addAction(preprocessorWidget->actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCopy);
    mnuEdit->addSeparator();
    mnuEdit->addAction(Agros2D::problem()->scene()->actDeleteSelected);
    mnuEdit->addSeparator();
    mnuEdit->addAction(sceneViewPreprocessor->actSceneViewSelectRegion);
    mnuEdit->addAction(Agros2D::problem()->scene()->actTransform);

    QMenu *mnuProblemAddGeometry = new QMenu(tr("&Add geometry"), this);
    mnuProblemAddGeometry->addAction(Agros2D::problem()->scene()->actNewNode);
    mnuProblemAddGeometry->addAction(Agros2D::problem()->scene()->actNewEdge);
    mnuProblemAddGeometry->addAction(Agros2D::problem()->scene()->actNewLabel);

    mnuProblemAddBoundaryAndMaterial = new QMenu(tr("&Add boundaries and materials"), this);

    mnuProblem = menuBar()->addMenu(tr("&Problem"));
    mnuProblem->addMenu(mnuProblemAddGeometry);
    mnuProblem->addMenu(mnuProblemAddBoundaryAndMaterial);
    mnuProblem->addSeparator();
    mnuProblem->addAction(actMesh);
    mnuProblem->addAction(actSolve);
    mnuProblem->addAction(actSolveNewComputation);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actMaterialBrowser);
    mnuTools->addAction(actCreateVideo);
    // read custom forms
    mnuTools->addSeparator();
    mnuCustomForms = new QMenu(tr("Custom forms"), this);
    mnuTools->addMenu(mnuCustomForms);
    readCustomScripts(mnuCustomForms, consoleView->console(), this);

    mnuSettings = menuBar()->addMenu(tr("S&ettings"));
    mnuSettings->addAction(actUnitTests);
    mnuSettings->addSeparator();
    mnuSettings->addAction(actHideControlPanel);
    mnuSettings->addAction(actHideViewPanel);
    mnuSettings->addAction(actFullScreen);
    mnuSettings->addSeparator();
    mnuSettings->addAction(actOptions);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actCheckVersion);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros2D" MacOSX menu
}

void MainWindow::createToolBars()
{
    // main toolbar
    mainWidget->toolBar->addAction(actDocumentNew);
    mainWidget->toolBar->addAction(actDocumentOpen);
    mainWidget->toolBar->addAction(actDocumentSave);

    // zoom toolbar
    QMenu *menu = new QMenu();
    menu->addAction(actSceneZoomBestFit);
    menu->addAction(actSceneZoomRegion);
    menu->addAction(actSceneZoomIn);
    menu->addAction(actSceneZoomOut);

    QToolButton *toolButton = new QToolButton();
    toolButton->setMenu(menu);
    toolButton->setIcon(icon("zoom"));
    toolButton->setPopupMode(QToolButton::InstantPopup);

    preprocessorWidget->toolBar->addSeparator();
    preprocessorWidget->toolBar->addWidget(toolButton);
}

void MainWindow::createMain()
{
    sceneViewInfoWidget = new SceneViewWidget(sceneInfoWidget, this);
    sceneViewPreprocessorWidget = new SceneViewWidget(sceneViewPreprocessor, this);
    sceneViewMeshWidget = new SceneViewWidget(postprocessorWidget->sceneViewMesh(), this);
    sceneViewPost2DWidget = new SceneViewWidget(postprocessorWidget->sceneViewPost2D(), this);
    sceneViewPost3DWidget = new SceneViewWidget(postprocessorWidget->sceneViewPost3D(), this);
    sceneViewPostParticleTracingWidget = new SceneViewWidget(postprocessorWidget->sceneViewParticleTracing(), this);
    sceneViewChartWidget = new SceneViewWidget(postprocessorWidget->sceneViewChart(), this);
    sceneViewPythonEditorWidget = new SceneViewWidget(scriptEditor, this);
    sceneViewOptilabWidget = new SceneViewWidget(optiLab, this);

    tabViewLayout = new QStackedLayout();
    tabViewLayout->setContentsMargins(0, 0, 0, 0);
    tabViewLayout->addWidget(sceneViewInfoWidget);
    tabViewLayout->addWidget(sceneViewPreprocessorWidget);
    tabViewLayout->addWidget(sceneViewMeshWidget);
    tabViewLayout->addWidget(sceneViewPost2DWidget);
    tabViewLayout->addWidget(sceneViewPost3DWidget);
    tabViewLayout->addWidget(sceneViewPostParticleTracingWidget);
    tabViewLayout->addWidget(sceneViewChartWidget);
    tabViewLayout->addWidget(sceneViewPythonEditorWidget);
    tabViewLayout->addWidget(sceneViewOptilabWidget);

    QWidget *viewWidget = new QWidget();
    viewWidget->setLayout(tabViewLayout);

    tabControlsLayout = new QStackedLayout();
    tabControlsLayout->setContentsMargins(0, 0, 0, 0);
    tabControlsLayout->addWidget(mainWidget);
    tabControlsLayout->addWidget(preprocessorWidget);
    tabControlsLayout->addWidget(postprocessorWidget);
    tabControlsLayout->addWidget(optiLab->optiLabWidget());
    tabControlsLayout->addWidget(scriptEditor->pythonEditorWidget());

    viewControls = new QWidget();
    viewControls->setLayout(tabControlsLayout);

    QTabWidget *tabInfo = new QTabWidget(this);
    tabInfo->addTab(consoleView, tr("Python console"));
    tabInfo->addTab(resultsView, tr("Results"));

    splitterView = new QSplitter(Qt::Vertical, this);
    splitterView->addWidget(tabInfo);
    splitterView->addWidget(logView);
    splitterView->setCollapsible(0, false);
    splitterView->setCollapsible(1, false);
    splitterView->setStretchFactor(0, 1);
    splitterView->setStretchFactor(1, 3);

    // spacing
    QLabel *spacing = new QLabel;
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // left toolbar
    QToolBar *tlbLeftBar = new QToolBar();
    tlbLeftBar->setOrientation(Qt::Vertical);
    // fancy layout
#ifdef Q_WS_WIN
    int fontSize = 7;
#endif
#ifdef Q_WS_X11
    int fontSize = 8;
#endif
    tlbLeftBar->setStyleSheet(QString("QToolBar { border: 1px solid rgba(70, 70, 70, 255); }"
                                      "QToolBar { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:1 rgba(120, 120, 120, 255)); }"
                                      "QToolButton { border: 0px; color: rgba(230, 230, 230, 255); font: bold; font-size: %1pt; width: 75px; }"
                                      "QToolButton:hover { border: 0px; background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:0.5 rgba(160, 160, 160, 255), stop:1 rgba(150, 150, 150, 255)); }"
                                      "QToolButton:checked:hover, QToolButton:checked { border: 0px; color: rgba(30, 30, 30, 255); background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(160, 160, 160, 255), stop:0.5 rgba(220, 220, 220, 255), stop:1 rgba(160, 160, 160, 255)); }").arg(fontSize));
    // system layout
    // leftToolBar->setStyleSheet("QToolButton { font: bold; font-size: 8pt; width: 75px; }");

    tlbLeftBar->setIconSize(QSize(32, 32));
    tlbLeftBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    tlbLeftBar->addAction(mainWidget->actProperties);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(sceneViewPreprocessor->actSceneModePreprocessor);
    tlbLeftBar->addAction(postprocessorWidget->actSceneModePost);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(optiLab->actSceneModeOptiLab);
    tlbLeftBar->addAction(scriptEditor->actSceneModePythonEditor);
    tlbLeftBar->addWidget(spacing);
    tlbLeftBar->addAction(actMesh);
    tlbLeftBar->addAction(actSolve);
    tlbLeftBar->addAction(actSolveNewComputation);

    splitterMain = new QSplitter(Qt::Horizontal, this);
    splitterMain->addWidget(viewControls);
    splitterMain->addWidget(viewWidget);
    splitterMain->addWidget(splitterView);
    splitterMain->setCollapsible(0, false);
    splitterMain->setCollapsible(1, false);
    splitterMain->setCollapsible(2, false);
    splitterMain->setStretchFactor(0, 1);
    splitterMain->setStretchFactor(1, 4);
    splitterMain->setStretchFactor(2, 1);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(tlbLeftBar);
    layoutMain->addWidget(splitterMain);

    QWidget *main = new QWidget();
    main->setLayout(layoutMain);

    setCentralWidget(main);
}

void MainWindow::doMouseSceneModeChanged(MouseSceneMode mouseSceneMode)
{
    /*
    lblMouseMode->setText("Mode: -");

    switch (mouseSceneMode)
    {
    case MouseSceneMode_Add:
    {
        switch (sceneViewGeometry->sceneMode())
        {
        case SceneGeometryMode_OperateOnNodes:
            lblMouseMode->setText(tr("Mode: Add node"));
            break;
        case SceneGeometryMode_OperateOnEdges:
            lblMouseMode->setText(tr("Mode: Add edge"));
            break;
        case SceneGeometryMode_OperateOnLabels:
            lblMouseMode->setText(tr("Mode: Add label"));
            break;
        default:
            break;
        }
    }
        break;
    case MouseSceneMode_Pan:
        lblMouseMode->setText(tr("Mode: Pan"));
        break;
    case MouseSceneMode_Rotate:
        lblMouseMode->setText(tr("Mode: Rotate"));
        break;
    case MouseSceneMode_Move:
    {
        switch (sceneViewGeometry->sceneMode())
        {
        case SceneGeometryMode_OperateOnNodes:
            lblMouseMode->setText(tr("Mode: Move node"));
            break;
        case SceneGeometryMode_OperateOnEdges:
            lblMouseMode->setText(tr("Mode: Move edge"));
            break;
        case SceneGeometryMode_OperateOnLabels:
            lblMouseMode->setText(tr("Mode: Move label"));
            break;
        default:
            break;
        }
    }
        break;
    default:
        break;
    }
    */
}

void MainWindow::setRecentFiles()
{
    // recent files
    if (!Agros2D::problem()->config()->fileName().isEmpty())
    {
        QFileInfo fileInfo(Agros2D::problem()->config()->fileName());
        if (recentFiles.indexOf(fileInfo.absoluteFilePath()) == -1)
            recentFiles.insert(0, fileInfo.absoluteFilePath());
        else
            recentFiles.move(recentFiles.indexOf(fileInfo.absoluteFilePath()), 0);

        while (recentFiles.count() > 15) recentFiles.removeLast();
    }

    mnuRecentFiles->clear();
    for (int i = 0; i<recentFiles.count(); i++)
    {
        QAction *actMenuRecentItem = new QAction(recentFiles[i], this);
        actDocumentOpenRecentGroup->addAction(actMenuRecentItem);
        mnuRecentFiles->addAction(actMenuRecentItem);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        QString fileName = QUrl(event->mimeData()->urls().at(0)).toLocalFile().trimmed();
        if (QFile::exists(fileName))
        {
            doDocumentOpen(fileName);

            event->acceptProposedAction();
        }
    }
}

void MainWindow::doDocumentNew()
{
    FieldSelectDialog dialog(QList<QString>(), this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        // clear all computations
        Agros2D::clearComputations();

        // clear preprocessor
        Agros2D::problem()->clearFieldsAndConfig();

        // add field
        try
        {
            FieldInfo *fieldInfo = new FieldInfo(dialog.selectedFieldId());

            Agros2D::problem()->addField(fieldInfo);

            sceneViewPreprocessor->actSceneModePreprocessor->trigger();
            sceneViewPreprocessor->doZoomBestFit();
        }
        catch (AgrosPluginException& e)
        {
            Agros2D::problem()->scene()->clear();

            Agros2D::log()->printError(tr("Problem"), e.toString());
        }
    }
}

void MainWindow::doDocumentOpen(const QString &fileName)
{
    QSettings settings;
    QString fileNameDocument;

    if (fileName.isEmpty())
    {
        QString dir = settings.value("General/LastProblemDir", "data").toString();

        fileNameDocument = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("Agros2D files (*.ags *.a2d *.py);;Agros2D data files (*.ags);;Python script (*.py);;Agros2D data files - deprecated (*.a2d)"));
    }
    else
    {
        fileNameDocument = fileName;
    }

    if (fileNameDocument.isEmpty()) return;

    if (QFile::exists(fileNameDocument))
    {
        QFileInfo fileInfo(fileNameDocument);
        if (fileInfo.suffix() == "ags" || fileInfo.suffix() == "a2d")
        {
            Agros2D::problem()->readProblemFromFile(fileNameDocument);
            setRecentFiles();

            // load solution
            // m_computation->readSolutionFromFile(fileNameDocument);

            sceneViewPreprocessor->actSceneModePreprocessor->trigger();
            sceneViewPreprocessor->doZoomBestFit();

            return;
        }
        else if (fileInfo.suffix() == "py")
        {
            // python script
            scriptEditor->doFileOpen(fileNameDocument);
            scriptEditor->actSceneModePythonEditor->trigger();

            return;
        }

        Agros2D::log()->printError(tr("Problem"), tr("Unknown suffix."));
    }
    else
    {
        Agros2D::log()->printError(tr("Problem"), tr("File '%1' is not found.").arg(fileNameDocument));
    }
}

void MainWindow::doDocumentOpenForm(const QString &fileName, const QString &formName)
{
    // try to open standard form
    bool customForm = false;
    foreach (QAction *action, mnuCustomForms->actions())
    {
        if (FormScript *form = dynamic_cast<FormScript *>(action->parent()))
        {
            if (QFileInfo(form->fileName()).absoluteFilePath() == QFileInfo(fileName).absoluteFilePath())
            {
                customForm = true;
                form->showForm(formName);
            }
        }
    }
    if (!customForm)
    {
        // example form
        FormScript form(fileName, consoleView->console(), this);
        form.showForm(formName);
    }
}

void MainWindow::doDocumentOpenRecent(QAction *action)
{
    QString fileName = action->text();
    doDocumentOpen(fileName);
}

void MainWindow::doDocumentSave()
{
    if (QFile::exists(Agros2D::problem()->config()->fileName()))
    {
        try
        {
            // write to archive
            Agros2D::problem()->writeProblemToArchive(Agros2D::problem()->config()->fileName(), true);
        }
        catch (AgrosException &e)
        {
            Agros2D::log()->printError(tr("Problem"), e.toString());
        }
    }
    else
    {
        doDocumentSaveAs();
    }
}

void MainWindow::doDeleteSolution()
{
    // clear all computations
    Agros2D::clearComputations();

    mainWidget->actProperties->trigger();
    sceneViewPreprocessor->doZoomBestFit();
}

void MainWindow::doDocumentSaveAs()
{
    QSettings settings;
    QString dir = settings.value("General/LastProblemDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Agros2D files (*.ags)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "ags") fileName += ".ags";

        try
        {
            Agros2D::problem()->writeProblemToArchive(fileName, true);
            setRecentFiles();
        }
        catch (AgrosException &e)
        {
            Agros2D::log()->printError(tr("Problem"), e.toString());
        }
    }
    sceneInfoWidget->refresh();
}

void MainWindow::doDocumentClose()
{
    // clear all computations
    Agros2D::clearComputations();

    // clear preprocessor
    Agros2D::problem()->clearFieldsAndConfig();

    mainWidget->actProperties->trigger();
}

void MainWindow::doDocumentImportDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        Agros2D::problem()->scene()->importFromDxf(fileName);
        sceneViewPreprocessor->doZoomBestFit();

        QFileInfo fileInfo(fileName);
        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentExportDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "dxf") fileName += ".dxf";
        Agros2D::problem()->scene()->exportToDxf(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentSaveImage()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export image to file"), dir, tr("PNG files (*.png)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "png") fileName += ".png";

        if (sceneViewPreprocessor->actSceneModePreprocessor->isChecked())
            sceneViewPreprocessor->saveImageToFile(fileName);
        // else if (sceneViewMesh->actSceneModeMesh->isChecked())
        //    sceneViewMesh->saveImageToFile(fileName);
        // else if (sceneViewPost2D->actSceneModePost2D->isChecked())
        //     sceneViewPost2D->saveImageToFile(fileName);
        // else if (sceneViewPost3D->actSceneModePost3D->isChecked())
        //    sceneViewPost3D->saveImageToFile(fileName);
        // else if (sceneViewParticleTracing->actSceneModeParticleTracing->isChecked())
        //    sceneViewParticleTracing->saveImageToFile(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doDocumentSaveGeometry()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();


    QString selected;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export geometry to file"), dir,
                                                    tr("SVG files (*.svg)"),
                                                    &selected);

    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix().toLower() != "svg") fileName += ".svg";

        sceneViewPreprocessor->saveGeometryToSvg(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doExamples(const QString &groupName)
{
    ExamplesDialog examples(this);
    if (examples.showDialog(groupName) == QDialog::Accepted)
    {
        if (QFile::exists(examples.selectedFilename()))
        {
            QFileInfo fileInfo(examples.selectedFilename());

            if (fileInfo.suffix() == "ags" || fileInfo.suffix() == "a2d" || fileInfo.suffix() == "py")
            {
                doDocumentOpen(examples.selectedFilename());
            }
            else if (fileInfo.suffix() == "ui")
            {
                doDocumentOpenForm(examples.selectedFilename(), examples.selectedFormFilename());
            }
        }
    }
}

void MainWindow::doCreateVideo()
{
    VideoDialog *videoDialog = nullptr;

    switch (postprocessorWidget->mode())
    {
    case PostprocessorWidgetMode_Mesh:
        videoDialog = new VideoDialog(postprocessorWidget->sceneViewMesh(), postprocessorWidget->computation().data(), this);
        break;
    case PostprocessorWidgetMode_Post2D:
        videoDialog = new VideoDialog(postprocessorWidget->sceneViewMesh(), postprocessorWidget->computation().data(), this);
        break;
    case PostprocessorWidgetMode_Chart:
        videoDialog = new VideoDialog(postprocessorWidget->sceneViewMesh(), postprocessorWidget->computation().data(), this);
        break;
    default:
        break;
    }

    // if (sceneViewMesh->actSceneModeMesh->isChecked())
    //
    // else if (sceneViewPost2D->actSceneModePost2D->isChecked())
    //     videoDialog = new VideoDialog(sceneViewPost2D, postDeal, this);
    // else if (sceneViewPost3D->actSceneModePost3D->isChecked())
    //    videoDialog = new VideoDialog(sceneViewPost3D, postDeal, this);

    if (videoDialog)
    {
        videoDialog->showDialog();
        delete videoDialog;
    }
}

void MainWindow::doMesh()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(false);
    computation->meshWithGUI();
}

void MainWindow::doSolve()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(false, true);
    computation->solveWithGUI();
}

void MainWindow::doSolveNewComputation()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true, true);
    computation->solveWithGUI();
}

void MainWindow::doSolveFinished()
{
    if (m_computation->isMeshed() && !currentPythonEngine()->isScriptRunning())
    {
        postprocessorWidget->actSceneModePost->trigger();
        postprocessorWidget->sceneViewMesh()->doZoomBestFit();
        postprocessorWidget->sceneViewPost2D()->doZoomBestFit();
        postprocessorWidget->sceneViewPost3D()->doZoomBestFit();
        postprocessorWidget->sceneViewParticleTracing()->doZoomBestFit();
    }

    // show empty results
    resultsView->showEmpty();
}

void MainWindow::doFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::doOptions()
{
    ConfigComputerDialog configDialog(this);
    if (configDialog.exec())
    {
        sceneViewPreprocessor->refresh();
        // postDeal->refresh();
        // setControls();
    }
}

void MainWindow::doUnitTests()
{
    UnitTestsWidget unit(this);
    connect(&unit, SIGNAL(openInPythonLab(QString, QString)), scriptEditor, SLOT(doFileOpenAndFind(QString, QString)));
    connect(&unit, SIGNAL(openInPythonLab(QString, QString)), scriptEditor, SLOT(show()));
    unit.exec();
}

void MainWindow::doTransform()
{
    sceneTransformDialog->showDialog();
}

void MainWindow::doMaterialBrowser()
{
    MaterialBrowserDialog materialBrowserDialog(this);
    materialBrowserDialog.showDialog(false);
}

void MainWindow::doCut()
{

}

void MainWindow::doCopy()
{
    // copy image to clipboard
    QPixmap pixmap;
    if (sceneViewPreprocessor->actSceneModePreprocessor->isChecked())
        pixmap = sceneViewPreprocessor->renderScenePixmap();
    // else if (sceneViewMesh->actSceneModeMesh->isChecked())
    //    pixmap = sceneViewMesh->renderScenePixmap();
    // else if (sceneViewPost2D->actSceneModePost2D->isChecked())
    //     pixmap = sceneViewPost2D->renderScenePixmap();
    // else if (sceneViewPost3D->actSceneModePost3D->isChecked())
    //     pixmap = sceneViewPost3D->renderScenePixmap();
    // else if (sceneViewParticleTracing->actSceneModeParticleTracing->isChecked())
    //    pixmap = sceneViewParticleTracing->renderScenePixmap();

    QApplication::clipboard()->setImage(pixmap.toImage());
}

void MainWindow::doPaste()
{

}

void MainWindow::clear()
{
    mainWidget->actProperties->trigger();

    setControls();
}

void MainWindow::doStartedScript()
{
    // disable controls
    setEnabledControls(false);
}

void MainWindow::doExecutedScript()
{
    // enable controls
    setEnabledControls(true);
    setControls();
}

void MainWindow::setEnabledControls(bool state)
{
    tabViewLayout->setEnabled(state);
    tabControlsLayout->setEnabled(state);

    resultsView->setEnabled(state);
    consoleView->setEnabled(state);
    logView->setEnabled(state);

    menuBar()->setEnabled(state);

    centralWidget()->setEnabled(state);
}

void MainWindow::setControls()
{
    if (currentPythonEngine()->isScriptRunning())
        return;

    setUpdatesEnabled(false);
    setEnabled(true);

    actDeleteSolution->setEnabled(Agros2D::computations().count() > 0);

    // set controls
    Agros2D::problem()->scene()->actTransform->setEnabled(false);

    sceneViewPreprocessor->actSceneZoomRegion = NULL;
    // sceneViewMesh->actSceneZoomRegion = NULL;

    bool showZoom = sceneViewPreprocessor->actSceneModePreprocessor->isChecked() || postprocessorWidget->actSceneModePost->isChecked();

    actSceneZoomIn->setVisible(showZoom);
    actSceneZoomOut->setVisible(showZoom);
    actSceneZoomBestFit->setVisible(showZoom);
    actSceneZoomRegion->setVisible(showZoom);
    actSceneZoomRegion->setEnabled(sceneViewPreprocessor->actSceneModePreprocessor->isChecked() ||
                                   postprocessorWidget->actSceneModePost->isChecked());

    // disconnect signals
    actSceneZoomIn->disconnect();
    actSceneZoomOut->disconnect();
    actSceneZoomBestFit->disconnect();

    if (mainWidget->actProperties->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewInfoWidget);
        tabControlsLayout->setCurrentWidget(mainWidget);
    }
    else if (sceneViewPreprocessor->actSceneModePreprocessor->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewPreprocessorWidget);
        tabControlsLayout->setCurrentWidget(preprocessorWidget);

        Agros2D::problem()->scene()->actTransform->setEnabled(true);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewPreprocessor, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewPreprocessor, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewPreprocessor, SLOT(doZoomBestFit()));
        sceneViewPreprocessor->actSceneZoomRegion = actSceneZoomRegion;
    }
    else if (postprocessorWidget->actSceneModePost->isChecked())
    {
        switch (postprocessorWidget->mode())
        {
        case PostprocessorWidgetMode_Mesh:
        {
            tabViewLayout->setCurrentWidget(sceneViewMeshWidget);

            connect(actSceneZoomIn, SIGNAL(triggered()), postprocessorWidget->sceneViewMesh(), SLOT(doZoomIn()));
            connect(actSceneZoomOut, SIGNAL(triggered()), postprocessorWidget->sceneViewMesh(), SLOT(doZoomOut()));
            connect(actSceneZoomBestFit, SIGNAL(triggered()), postprocessorWidget->sceneViewMesh(), SLOT(doZoomBestFit()));
            postprocessorWidget->sceneViewMesh()->actSceneZoomRegion = actSceneZoomRegion;
        }
            break;
        case PostprocessorWidgetMode_Post2D:
        {
            tabViewLayout->setCurrentWidget(sceneViewPost2DWidget);

            connect(actSceneZoomIn, SIGNAL(triggered()), postprocessorWidget->sceneViewPost2D(), SLOT(doZoomIn()));
            connect(actSceneZoomOut, SIGNAL(triggered()), postprocessorWidget->sceneViewPost2D(), SLOT(doZoomOut()));
            connect(actSceneZoomBestFit, SIGNAL(triggered()), postprocessorWidget->sceneViewPost2D(), SLOT(doZoomBestFit()));
            postprocessorWidget->sceneViewPost2D()->actSceneZoomRegion = actSceneZoomRegion;
        }
            break;

        case PostprocessorWidgetMode_Post3D:
        {
            tabViewLayout->setCurrentWidget(sceneViewPost3DWidget);

            connect(actSceneZoomIn, SIGNAL(triggered()), postprocessorWidget->sceneViewPost3D(), SLOT(doZoomIn()));
            connect(actSceneZoomOut, SIGNAL(triggered()), postprocessorWidget->sceneViewPost3D(), SLOT(doZoomOut()));
            connect(actSceneZoomBestFit, SIGNAL(triggered()), postprocessorWidget->sceneViewPost3D(), SLOT(doZoomBestFit()));
        }
            break;
        case PostprocessorWidgetMode_Chart:
            tabViewLayout->setCurrentWidget(sceneViewChartWidget);
            break;
        case PostprocessorWidgetMode_ParticleTracing:
        {
            tabViewLayout->setCurrentWidget(sceneViewPostParticleTracingWidget);

            connect(actSceneZoomIn, SIGNAL(triggered()), postprocessorWidget->sceneViewParticleTracing(), SLOT(doZoomIn()));
            connect(actSceneZoomOut, SIGNAL(triggered()), postprocessorWidget->sceneViewParticleTracing(), SLOT(doZoomOut()));
            connect(actSceneZoomBestFit, SIGNAL(triggered()), postprocessorWidget->sceneViewParticleTracing(), SLOT(doZoomBestFit()));
            // postprocessorWidget->sceneViewPost2D()->actSceneZoomRegion = actSceneZoomRegion;
        }
            break;
        default:
            break;
        }
        tabControlsLayout->setCurrentWidget(postprocessorWidget);


    }
    else if (optiLab->actSceneModeOptiLab->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewOptilabWidget);
        tabControlsLayout->setCurrentWidget(optiLab->optiLabWidget());
    }
    else if (scriptEditor->actSceneModePythonEditor->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewPythonEditorWidget);
        tabControlsLayout->setCurrentWidget(scriptEditor->pythonEditorWidget());
    }

    // menu bar
    menuBar()->clear();
    if (scriptEditor->actSceneModePythonEditor->isChecked())
    {
        menuBar()->addMenu(scriptEditor->mnuFile);
        menuBar()->addMenu(scriptEditor->mnuEdit);
        menuBar()->addMenu(scriptEditor->mnuTools);
    }
    else
    {
        menuBar()->addMenu(mnuFile);
        menuBar()->addMenu(mnuEdit);
        menuBar()->addMenu(mnuTools);
        menuBar()->addMenu(mnuProblem);
    }
    menuBar()->addMenu(mnuSettings);
    menuBar()->addMenu(mnuHelp);

    if (!m_computation.isNull())
        actDocumentExportMeshFile->setEnabled(m_computation->isMeshed());

    // postprocessorWidget->updateControls();

    setUpdatesEnabled(true);
}

void MainWindow::doHelp()
{
    showPage("index.html");
}

void MainWindow::doHelpShortCut()
{
    showPage("getting_started/shortcut_keys.html");
}

void MainWindow::doCheckVersion()
{
    checkForNewVersion();
}

void MainWindow::doAbout()
{
    AboutDialog about(this);
    about.exec();
}

void MainWindow::doHideControlPanel()
{
    viewControls->setVisible(actHideControlPanel->isChecked());
}

void MainWindow::doHideViewPanel()
{
    splitterView->setVisible(actHideViewPanel->isChecked());
}

void MainWindow::doDocumentExportMeshFile()
{
    if (m_computation->isMeshed())
    {
        QSettings settings;
        QString dir = settings.value("General/LastMeshDir").toString();

        QString fileName = QFileDialog::getSaveFileName(this, tr("Export mesh file"), dir, tr("Mesh files (*.msh)"));
        QFileInfo fileInfo(fileName);

        if (!fileName.isEmpty())
        {
            if (fileInfo.suffix() != "msh") fileName += ".msh";

            // remove existing file
            if (QFile::exists(fileName + ".msh"))
                QFile::remove(fileName + ".msh");

            // copy file
            QFile::copy(cacheProblemDir() + "/initial.msh", fileName);
            if (fileInfo.absoluteDir() != cacheProblemDir())
                settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
        }
    }
    else
    {
        Agros2D::log()->printMessage(tr("Problem"), tr("The problem is not meshed"));
    }
}

void MainWindow::doExportVTKGeometry()
{
    // file dialog
    QSettings settings;
    QString dir = settings.value("General/LastVTKDir").toString();

    QString fn = QFileDialog::getSaveFileName(QApplication::activeWindow(), tr("Export VTK file"), dir, tr("VTK files (*.vtk)"));
    if (fn.isEmpty())
        return;

    if (!fn.endsWith(".vtk"))
        fn.append(".vtk");

    m_computation->scene()->exportVTKGeometry(fn);

    if (!fn.isEmpty())
    {
        QFileInfo fileInfo(fn);
        if (fileInfo.absoluteDir() != tempProblemDir())
        {
            QSettings settings;
            settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
        }
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    // startup
    if (!m_startupProblemFilename.isEmpty())
    {
        // open problem
        doDocumentOpen(m_startupProblemFilename);
        m_startupProblemFilename = "";

        if (m_startupExecute)
            doSolve();
    }
    else if (!m_startupScriptFilename.isEmpty())
    {
        // open script
        scriptEditor->doFileOpen(m_startupScriptFilename);
        scriptEditor->actSceneModePythonEditor->trigger();

        m_startupScriptFilename = "";

        if (m_startupExecute)
            scriptEditor->doRunPython();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (currentPythonEngine()->isScriptRunning())
    {
        QMessageBox::information(QApplication::activeWindow(), tr("Script"),
                                 tr("Cannot close main window. Script is still running."));
        event->ignore();
    }
}

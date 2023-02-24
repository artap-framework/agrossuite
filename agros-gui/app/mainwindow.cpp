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
#include "gui/problemdialog.h"
#include "gui/fielddialog.h"
#include "gui/logwidget.h"
#include "gui/checkversion.h"
#include "util/global.h"
#include "util/conf.h"

#include "app/scenegeometrydialog.h"
#include "app/scenemarkerdialog.h"
#include "app/scriptgeneratordialog.h"

#include "solver/module.h"
#include "solver/problem.h"
#include "solver/solver.h"
#include "solver/problem_config.h"

#include "scene.h"
#include "scenebasic.h"
#include "sceneview_common.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "sceneview_particle.h"
#include "logview.h"
#include "gui/infowidget.h"
#include "preprocessorview.h"
#include "postprocessorview.h"
#include "chartdialog.h"
#include "confdialog.h"
#include "optilab/optilab.h"
#include "materialbrowserdialog.h"
#include "chartdialog.h"
#include "examplesdialog.h"

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent),
    logDialog(nullptr), logStdOut(nullptr)
{
    setWindowIcon(icon("agros"));

    m_startupProblemFilename = "";
    m_startupExecute = false;

    // log stdout
    if (Agros::configComputer()->value(Config::Config_LogStdOut).toBool())
        logStdOut = new LogStdOut();

    // scene
    sceneViewProblem = new SceneViewPreprocessor(this);
    // sceneViewVTK2D = new SceneViewVTK2D(postDeal, this);

    // scene - info widget
    sceneInfoWidget = new InfoWidget(this);

    // preprocessor
    problemWidget = new PreprocessorWidget(sceneViewProblem, this);
    // postprocessor
    postprocessorWidget = new PostprocessorWidget();

    connect(postprocessorWidget, SIGNAL(changed()), this, SLOT(setControls()));
    connect(postprocessorWidget, SIGNAL(modeChanged()), this, SLOT(setControls()));

    // problem
    exampleWidget = new ExamplesWidget(this, sceneInfoWidget);
    connect(exampleWidget, SIGNAL(problemOpen(QString)), this, SLOT(doDocumentOpen(QString)));

    // view info
    m_connectLog = new ConnectLog();
    logView = new LogView(this, m_connectLog);
    (static_cast<LogGui * > (Agros::log()))->setConnectLog(m_connectLog);

    // OptiLab
    optiLab = new OptiLab(this);

    createActions();
    createMenus();
    createToolBars();
    createMain();

    // info
    connect(tabViewLayout, SIGNAL(currentChanged(int)), this, SLOT(setControls()));

    // connect(Agros::problem()->scene(), SIGNAL(cleared()), this, SLOT(clear()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(setControls()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), sceneViewProblem, SLOT(refresh()));

    // preprocessor
    connect(sceneViewProblem, SIGNAL(sceneGeometryModeChanged(SceneGeometryMode)), problemWidget, SLOT(loadTooltip(SceneGeometryMode)));

    sceneViewProblem->clear();

    Agros::problem()->clearFieldsAndConfig();

    exampleWidget->actExamples->trigger();
    sceneViewProblem->doZoomBestFit();

    // set recent files
    setRecentFiles();

    // accept drops
    setAcceptDrops(true);

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    checkForNewVersion(true);

    QSettings settings;
    restoreGeometry(settings.value("MainWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("MainWindow/State", saveState()).toByteArray());
    splitterMain->restoreState(settings.value("MainWindow/SplitterMainState").toByteArray());

    // show/hide control and view panel
    actHideControlPanel->setChecked(settings.value("MainWindow/ControlPanel", true).toBool());
    doHideControlPanel();

    setControls();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());
    settings.setValue("MainWindow/SplitterMainState", splitterMain->saveState());
    settings.setValue("MainWindow/ControlPanel", actHideControlPanel->isChecked());

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());

    delete m_connectLog;
    if (logStdOut != nullptr)
        delete logStdOut;
}

void MainWindow::createActions()
{
    actDocumentNew = new QAction(iconAwesome(fa::fa_file), tr("&New..."), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(doDocumentNew()));

    actDocumentOpen = new QAction(iconAwesome(fa::fa_folder_open), tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(doDocumentOpen()));

    actDocumentSave = new QAction(iconAwesome(fa::fa_floppy_disk), tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));

    actDeleteSolutions = new QAction(tr("Delete solutions"), this);
    connect(actDeleteSolutions, SIGNAL(triggered()), this, SLOT(doDeleteSolutions()));
    actDeleteSolutionsAndResults = new QAction(tr("Delete solutions and results"), this);
    connect(actDeleteSolutionsAndResults, SIGNAL(triggered()), this, SLOT(doDeleteSolutionsAndResults()));

    actDocumentSaveAs = new QAction(tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));

    actDocumentClose = new QAction(iconAwesome(fa::fa_folder_closed), tr("&Close"), this);
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

    actCreateFromModel = new QAction(iconAwesome(fa::fa_file_lines), tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence(tr("Ctrl+M")));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    actExit = new QAction(tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actCopy = new QAction(iconAwesome(fa::fa_copy), tr("Copy image to clipboard"), this);
    // actCopy->setShortcuts(QKeySequence::Copy);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopy()));

    actHelp = new QAction(tr("&Help"), this);
    actHelp->setShortcut(QKeySequence::HelpContents);
    // actHelp->setEnabled(false);
    connect(actHelp, SIGNAL(triggered()), this, SLOT(doHelp()));

    actHelpShortCut = new QAction(tr("&Shortcuts"), this);
    actHelpShortCut->setEnabled(false);
    connect(actHelpShortCut, SIGNAL(triggered()), this, SLOT(doHelpShortCut()));

    actCheckVersion = new QAction(tr("Check version"), this);
    connect(actCheckVersion, SIGNAL(triggered()), this, SLOT(doCheckVersion()));

    actAbout = new QAction(tr("About &Agros Suite"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actOptions = new QAction(tr("&Options"), this);
    actOptions->setMenuRole(QAction::PreferencesRole);
    connect(actOptions, SIGNAL(triggered()), this, SLOT(doOptions()));

    actFullScreen = new QAction(iconAwesome(fa::fa_expand), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actMaterialBrowser = new QAction(tr("Material browser..."), this);
    connect(actMaterialBrowser, SIGNAL(triggered()), this, SLOT(doMaterialBrowser()));

    actSolve = new QAction(icon("run"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actSolveNewComputation = new QAction(icon("run-step"), tr("&Solve new"), this);
    actSolveNewComputation->setShortcut(QKeySequence(tr("Alt+Shift+S")));
    connect(actSolveNewComputation, SIGNAL(triggered()), this, SLOT(doSolveNewComputation()));

    // zoom actions (geometry, post2d and post3d)
    // scene - zoom
    actSceneZoomIn = new QAction(iconAwesome(fa::fa_square_plus), tr("Zoom in"), this);
    actSceneZoomIn->setShortcut(QKeySequence::ZoomIn);

    actSceneZoomOut = new QAction(iconAwesome(fa::fa_square_minus), tr("Zoom out"), this);
    actSceneZoomOut->setShortcut(QKeySequence::ZoomOut);

    actSceneZoomBestFit = new QAction(iconAwesome(fa::fa_square_arrow_up_right), tr("Zoom best fit"), this);
    actSceneZoomBestFit->setShortcut(tr("Ctrl+0"));

    actSceneZoomRegion = new QAction(iconAwesome(fa::fa_square), tr("Zoom region"), this);
    actSceneZoomRegion->setCheckable(true);

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(exampleWidget->actExamples);
    actSceneModeGroup->addAction(sceneViewProblem->actSceneModeProblem);
    actSceneModeGroup->addAction(postprocessorWidget->actSceneModeResults);
    actSceneModeGroup->addAction(optiLab->actSceneModeOptiLab);
    actSceneModeGroup->addAction(logView->actLog);

    actHideControlPanel = new QAction(iconAwesome(fa::fa_gear), tr("Show/hide control panel"), this);
    actHideControlPanel->setShortcut(tr("Alt+0"));
    actHideControlPanel->setCheckable(true);
    connect(actHideControlPanel, SIGNAL(triggered()), this, SLOT(doHideControlPanel()));
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
    // mnuFileImportExport->addAction(postprocessorWidget->sceneViewMesh()->actExportVTKMesh);
    // mnuFileImportExport->addAction(postprocessorWidget->sceneViewMesh()->actExportVTKOrder);
    mnuFileImportExport->addAction(postprocessorWidget->sceneViewPost2D()->actExportVTKScalar);
    mnuFileImportExport->addAction(postprocessorWidget->sceneViewPost2D()->actExportVTKContours);

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentSave);
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addAction(actDeleteSolutions);
    mnuFile->addAction(actDeleteSolutionsAndResults);
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
    mnuEdit->addAction(problemWidget->actUndo);
    mnuEdit->addAction(problemWidget->actRedo);
    mnuEdit->addSeparator();
    mnuEdit->addAction(actCopy);
    mnuEdit->addSeparator();
    mnuEdit->addAction(problemWidget->actDeleteSelected);
    mnuEdit->addSeparator();
    mnuEdit->addAction(sceneViewProblem->actSceneViewSelectRegion);
    mnuEdit->addAction(problemWidget->actTransform);

    QMenu *mnuProblemAddGeometry = new QMenu(tr("&Add geometry"), this);
    mnuProblemAddGeometry->addAction(problemWidget->actNewNode);
    mnuProblemAddGeometry->addAction(problemWidget->actNewEdge);
    mnuProblemAddGeometry->addAction(problemWidget->actNewLabel);

    mnuProblem = menuBar()->addMenu(tr("&Problem"));
    mnuProblem->addMenu(mnuProblemAddGeometry);
    mnuProblem->addMenu(problemWidget->mnuBoundariesAndMaterials);
    mnuProblem->addSeparator();
    mnuProblem->addAction(actSolve);
    mnuProblem->addAction(actSolveNewComputation);
    mnuProblem->addSeparator();
    mnuProblem->addAction(actCreateFromModel);

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actMaterialBrowser);

    mnuSettings = menuBar()->addMenu(tr("S&ettings"));
    mnuSettings->addAction(actHideControlPanel);
    mnuSettings->addAction(actFullScreen);
    mnuSettings->addSeparator();
    mnuSettings->addAction(actOptions);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actHelp);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actCheckVersion);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros" MacOSX menu
}

void MainWindow::createToolBars()
{
    // main toolbar
    problemWidget->toolBar->insertAction(problemWidget->toolBar->actions().first(), actDocumentSave);
    problemWidget->toolBar->insertAction(problemWidget->toolBar->actions().first(), actDocumentOpen);
    problemWidget->toolBar->insertAction(problemWidget->toolBar->actions().first(), actDocumentNew);

    // zoom toolbar
    QMenu *menu = new QMenu(this);
    menu->addAction(actSceneZoomBestFit);
    menu->addAction(actSceneZoomRegion);
    menu->addAction(actSceneZoomIn);
    menu->addAction(actSceneZoomOut);

    QToolButton *toolButton = new QToolButton();
    toolButton->setIconSize(QSize(20, 20));
    toolButton->setMenu(menu);
    toolButton->setIcon(iconAwesome(fa::fa_toggle_on));
    toolButton->setPopupMode(QToolButton::InstantPopup);

    problemWidget->toolBar->addSeparator();
    problemWidget->toolBar->addWidget(toolButton);
}

void MainWindow::createMain()
{
    sceneViewInfoWidget = new SceneViewWidget(sceneInfoWidget, this);
    sceneViewProblemWidget = new SceneViewWidget(sceneViewProblem, this);
    sceneViewMeshWidget = new SceneViewWidget(postprocessorWidget->sceneViewMesh(), this);
    sceneViewPost2DWidget = new SceneViewWidget(postprocessorWidget->sceneViewPost2D(), this);
    sceneViewPost3DWidget = new SceneViewWidget(postprocessorWidget->sceneViewPost3D(), this);
    // sceneViewPostParticleTracingWidget = new SceneViewWidget(postprocessorWidget->sceneViewParticleTracing(), this);
    sceneViewChartWidget = new SceneViewWidget(postprocessorWidget->sceneViewChart(), this);
    sceneViewOptilabWidget = new SceneViewWidget(optiLab, this);
    sceneViewLogWidget = new SceneViewWidget(logView, this);

    tabViewLayout = new QStackedLayout();
    tabViewLayout->setContentsMargins(0, 0, 0, 0);
    tabViewLayout->addWidget(sceneViewInfoWidget);
    tabViewLayout->addWidget(sceneViewProblemWidget);
    tabViewLayout->addWidget(sceneViewMeshWidget);
    tabViewLayout->addWidget(sceneViewPost2DWidget);
    tabViewLayout->addWidget(sceneViewPost3DWidget);
    // tabViewLayout->addWidget(sceneViewPostParticleTracingWidget);
    tabViewLayout->addWidget(sceneViewChartWidget);
    tabViewLayout->addWidget(sceneViewOptilabWidget);
    tabViewLayout->addWidget(sceneViewLogWidget);

    QWidget *viewWidget = new QWidget();
    viewWidget->setLayout(tabViewLayout);

    tabControlsLayout = new QStackedLayout();
    tabControlsLayout->setContentsMargins(0, 0, 0, 0);
    tabControlsLayout->addWidget(exampleWidget);
    tabControlsLayout->addWidget(problemWidget);
    tabControlsLayout->addWidget(postprocessorWidget);
    tabControlsLayout->addWidget(optiLab->optiLabWidget());
    tabControlsLayout->addWidget(logView->logConfigWidget());

    viewControls = new QWidget();
    viewControls->setLayout(tabControlsLayout);

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
                                      "QToolBar { background-color: rgba(64, 66, 68, 255); }"
                                      "QToolButton { border: 0px; color: rgba(230, 230, 230, 255); font: bold; font-size: %1pt; width: 60px; }"
                                      "QToolButton:hover { border: 0px; background: rgba(150, 150, 150, 255); }"
                                      "QToolButton:checked:hover, QToolButton:checked { border: 0px; color: rgba(30, 30, 30, 255); background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(160, 160, 160, 255), stop:0.5 rgba(220, 220, 220, 255), stop:1 rgba(160, 160, 160, 255)); }").arg(fontSize));
    // system layout
    // leftToolBar->setStyleSheet("QToolButton { font: bold; font-size: 8pt; width: 75px; }");

    tlbLeftBar->setIconSize(QSize(32, 32));
    tlbLeftBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    tlbLeftBar->addAction(exampleWidget->actExamples);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(sceneViewProblem->actSceneModeProblem);
    tlbLeftBar->addAction(postprocessorWidget->actSceneModeResults);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(optiLab->actSceneModeOptiLab);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(logView->actLog);
    tlbLeftBar->addWidget(spacing);
    tlbLeftBar->addAction(actSolve);
    tlbLeftBar->addAction(actSolveNewComputation);
    tlbLeftBar->addAction(optiLab->optiLabWidget()->actRunStudy);

    splitterMain = new QSplitter(Qt::Horizontal, this);
    splitterMain->addWidget(viewControls);
    splitterMain->addWidget(viewWidget);
    splitterMain->setCollapsible(0, false);
    splitterMain->setCollapsible(1, false);
    splitterMain->setStretchFactor(0, 1);
    splitterMain->setStretchFactor(1, 4);

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
    mnuRecentFiles->clear();

    // recent files
    QSettings settings;
    QStringList recentFiles = settings.value("RecentProblems").value<QStringList>();

    if (!Agros::problem()->archiveFileName().isEmpty())
    {
        QFileInfo fileInfo(Agros::problem()->archiveFileName());
        if (recentFiles.indexOf(fileInfo.absoluteFilePath()) == -1)
            recentFiles.insert(0, fileInfo.absoluteFilePath());
        else
            recentFiles.move(recentFiles.indexOf(fileInfo.absoluteFilePath()), 0);
    }

    while (recentFiles.count() > 15) recentFiles.removeLast();

    settings.setValue("RecentProblems", recentFiles);

    for (int i = 0; i < recentFiles.count(); i++)
    {
        QFileInfo fileInfo(recentFiles[i]);
        if (fileInfo.isDir())
            continue;
        if (!QFile::exists(fileInfo.absoluteFilePath()))
            continue;

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
    if (dialog.exec() == QDialog::Accepted)
    {
        // clear preprocessor
        Agros::problem()->clearFieldsAndConfig();

        // add field
        try
        {
            FieldInfo *fieldInfo = new FieldInfo(dialog.selectedFieldId());

            Agros::problem()->addField(fieldInfo);

            sceneViewProblem->actSceneModeProblem->trigger();
            sceneViewProblem->doZoomBestFit();
        }
        catch (AgrosPluginException& e)
        {
            Agros::problem()->scene()->clear();

            Agros::log()->printError(tr("Problem"), e.toString());
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

        fileNameDocument = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("Agros files (*.ags *.a2d *.py);;Agros2D data files (*.ags);;Agros2D data files - deprecated (*.a2d)"));
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
            Agros::problem()->readProblemFromFile(fileNameDocument);

            setRecentFiles();

            sceneViewProblem->actSceneModeProblem->trigger();
            sceneViewProblem->doZoomBestFit();

            return;
        }

        Agros::log()->printError(tr("Problem"), tr("Unknown suffix."));
    }
    else
    {
        Agros::log()->printError(tr("Problem"), tr("File '%1' is not found.").arg(fileNameDocument));
    }
}

void MainWindow::doDocumentOpenRecent(QAction *action)
{
    QString fileName = action->text();
    doDocumentOpen(fileName);
}

void MainWindow::doDocumentSave()
{
    if (QFile::exists(Agros::problem()->archiveFileName()))
    {
        try
        {
            // write to archive
            Agros::problem()->writeProblemToArchive(Agros::problem()->archiveFileName(), false);
        }
        catch (AgrosException &e)
        {
            Agros::log()->printError(tr("Problem"), e.toString());
        }
    }
    else
    {
        doDocumentSaveAs();
    }
}

void MainWindow::doDeleteSolutions()
{
    sceneViewProblem->actSceneModeProblem->trigger();

    // clear solutions
    foreach (QSharedPointer<Computation> computation, Agros::singleton()->computations())
        computation->clearSolution();

    optiLab->optiLabWidget()->refresh();
    sceneViewProblem->refresh();
    setControls();
}

void MainWindow::doDeleteSolutionsAndResults()
{
    sceneViewProblem->actSceneModeProblem->trigger();

    // clear all computations
    Agros::clearComputations();

    optiLab->optiLabWidget()->refresh();
    sceneViewProblem->refresh();
    setControls();
}

void MainWindow::doDocumentSaveAs()
{
    QSettings settings;
    QString dir = settings.value("General/LastProblemDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"), dir, tr("Agros files (*.ags)"));
    if (!fileName.isEmpty())
    {
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() != "ags") fileName += ".ags";

        try
        {
            Agros::problem()->writeProblemToArchive(fileName, false);
            setRecentFiles();
        }
        catch (AgrosException &e)
        {
            Agros::log()->printError(tr("Problem"), e.toString());
        }
    }
}

void MainWindow::doDocumentClose()
{
    // clear problem
    Agros::problem()->clearFieldsAndConfig();

    exampleWidget->actExamples->trigger();
}

void MainWindow::doDocumentImportDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Import file"), dir, tr("DXF files (*.dxf)"));
    if (!fileName.isEmpty())
    {
        Agros::problem()->scene()->importFromDxf(fileName);
        sceneViewProblem->doZoomBestFit();

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
        Agros::problem()->scene()->exportToDxf(fileName);

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

        if (sceneViewProblem->actSceneModeProblem->isChecked())
            sceneViewProblem->saveImageToFile(fileName);
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

        sceneViewProblem->saveGeometryToSvg(fileName);

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void MainWindow::doCreatePythonFromModel()
{
    auto scriptDialog = new ScriptGeneratorDialog();
    scriptDialog->show();
}

void MainWindow::doSolve()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(false);

    logDialog = new LogDialog(computation.data(), tr("Solver"), m_connectLog);
    logDialog->show();

    // solve thread
    SolveThread *solveThread = new SolveThread(computation.data());
    connect(solveThread, SIGNAL(finished()), this, SLOT(doSolveFinished()));
    solveThread->startCalculation();
}

void MainWindow::doSolveNewComputation()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

    logDialog = new LogDialog(computation.data(), tr("Solver"), m_connectLog);
    logDialog->show();

    // solve thread
    SolveThread *solveThread = new SolveThread(computation.data());
    connect(solveThread, SIGNAL(finished()), this, SLOT(doSolveFinished()));
    solveThread->startCalculation();
}

void MainWindow::doSolveFinished()
{
    if (Agros::problem()->currentComputation()->isSolved())
    {
        // close log dialog
        logDialog->closeLog();

        // refresh postprocessor
        postprocessorWidget->solveFinished();
    }

    setControls();
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
        sceneViewProblem->refresh();
        setControls();
    }
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
    if (sceneViewProblem->actSceneModeProblem->isChecked())
    {
        pixmap = sceneViewProblem->renderScenePixmap();
    }
    else if (postprocessorWidget->actSceneModeResults->isChecked())
    {
        if (postprocessorWidget->mode() == PostprocessorWidgetMode_Mesh)
            pixmap = postprocessorWidget->sceneViewMesh()->renderScenePixmap();
        else if (postprocessorWidget->mode() == PostprocessorWidgetMode_Post2D)
            pixmap = postprocessorWidget->sceneViewPost2D()->renderScenePixmap();
        else if (postprocessorWidget->mode() == PostprocessorWidgetMode_Post3D)
            pixmap = postprocessorWidget->sceneViewPost3D()->renderScenePixmap();
        else if (postprocessorWidget->mode() == PostprocessorWidgetMode_Chart)
            pixmap = postprocessorWidget->sceneViewChart()->chart()->toPixmap();
    }

    QApplication::clipboard()->setImage(pixmap.toImage());
}

void MainWindow::doPaste()
{

}

void MainWindow::clear()
{
    setControls();
}

void MainWindow::doStartedScript()
{
    // disable controls
    setEnabledControls(false);
}

void MainWindow::setEnabledControls(bool state)
{
    tabViewLayout->setEnabled(state);
    tabControlsLayout->setEnabled(state);

    logView->setEnabled(state);

    menuBar()->setEnabled(state);

    centralWidget()->setEnabled(state);
}

void MainWindow::setControls()
{
    setUpdatesEnabled(false);
    setEnabled(true);

    actDeleteSolutions->setEnabled(!Agros::computations().isEmpty());
    actDeleteSolutionsAndResults->setEnabled(!Agros::computations().isEmpty());

    // set controls
    actSolve->setEnabled(sceneViewProblem->actSceneModeProblem->isChecked());
    actSolve->setVisible(sceneViewProblem->actSceneModeProblem->isChecked());
    actSolveNewComputation->setEnabled(sceneViewProblem->actSceneModeProblem->isChecked());
    actSolveNewComputation->setVisible(sceneViewProblem->actSceneModeProblem->isChecked());
    optiLab->optiLabWidget()->actRunStudy->setEnabled(optiLab->actSceneModeOptiLab->isChecked());
    optiLab->optiLabWidget()->actRunStudy->setVisible(optiLab->actSceneModeOptiLab->isChecked());

    postprocessorWidget->refresh();

    sceneViewProblem->actSceneZoomRegion = NULL;

    bool showZoom = sceneViewProblem->actSceneModeProblem->isChecked() || postprocessorWidget->actSceneModeResults->isChecked();

    actSceneZoomIn->setVisible(showZoom);
    actSceneZoomOut->setVisible(showZoom);
    actSceneZoomBestFit->setVisible(showZoom);
    actSceneZoomRegion->setVisible(showZoom);
    actSceneZoomRegion->setEnabled(sceneViewProblem->actSceneModeProblem->isChecked() ||
                                   postprocessorWidget->actSceneModeResults->isChecked());

    // disconnect signals
    actSceneZoomIn->disconnect();
    actSceneZoomOut->disconnect();
    actSceneZoomBestFit->disconnect();

    if (exampleWidget->actExamples->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewInfoWidget);
        tabControlsLayout->setCurrentWidget(exampleWidget);
    }
    else if (sceneViewProblem->actSceneModeProblem->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewProblemWidget);
        tabControlsLayout->setCurrentWidget(problemWidget);

        connect(actSceneZoomIn, SIGNAL(triggered()), sceneViewProblem, SLOT(doZoomIn()));
        connect(actSceneZoomOut, SIGNAL(triggered()), sceneViewProblem, SLOT(doZoomOut()));
        connect(actSceneZoomBestFit, SIGNAL(triggered()), sceneViewProblem, SLOT(doZoomBestFit()));
        sceneViewProblem->actSceneZoomRegion = actSceneZoomRegion;
    }
    else if (postprocessorWidget->actSceneModeResults->isChecked())
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
        /*
        case PostprocessorWidgetMode_ParticleTracing:
        {
            tabViewLayout->setCurrentWidget(sceneViewPostParticleTracingWidget);

            connect(actSceneZoomIn, SIGNAL(triggered()), postprocessorWidget->sceneViewParticleTracing(), SLOT(doZoomIn()));
            connect(actSceneZoomOut, SIGNAL(triggered()), postprocessorWidget->sceneViewParticleTracing(), SLOT(doZoomOut()));
            connect(actSceneZoomBestFit, SIGNAL(triggered()), postprocessorWidget->sceneViewParticleTracing(), SLOT(doZoomBestFit()));
            // postprocessorWidget->sceneViewPost2D()->actSceneZoomRegion = actSceneZoomRegion;
        }
            break;
        */
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
    else if (logView->actLog->isChecked())
    {
        tabViewLayout->setCurrentWidget(sceneViewLogWidget);
        tabControlsLayout->setCurrentWidget(logView->logConfigWidget());
    }

    // menu bar
    menuBar()->clear();
    menuBar()->addMenu(mnuFile);
    menuBar()->addMenu(mnuEdit);
    menuBar()->addMenu(mnuTools);
    menuBar()->addMenu(mnuProblem);
    menuBar()->addMenu(mnuSettings);
    menuBar()->addMenu(mnuHelp);

    // window title
    setWindowTitle(QString("Agros - %1").arg(Agros::problem()->archiveFileName()));

    // update preprocessor
    problemWidget->refresh();
    // update postprocessor
    postprocessorWidget->refresh();
    // update optilab
    optiLab->refresh();

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

void MainWindow::doDocumentExportMeshFile()
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

        // create mesh
        bool isMeshed = Agros::problem()->isMeshed();
        if (Agros::problem()->mesh())
        {
            std::ofstream ofsMesh(fileName.toStdString());
            boost::archive::binary_oarchive sbMesh(ofsMesh);
            Agros::problem()->initialMesh().save(sbMesh, 0);

            // if (!isMeshed)
            // Agros::problem()->initialMesh().clear();

            // copy file
            QFile::copy(cacheProblemDir() + "/initial.msh", fileName);
            if (fileInfo.absoluteDir() != cacheProblemDir())
                settings.setValue("General/LastMeshDir", fileInfo.absolutePath());
        }
        else
        {
            Agros::log()->printMessage(tr("Problem"), tr("The problem is not meshed"));
        }
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

    Agros::problem()->scene()->exportVTKGeometry(fn);

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
}

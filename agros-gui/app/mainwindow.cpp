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
#include <boost/archive/text_oarchive.hpp>

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent),
    logDialog(nullptr), logStdOut(nullptr)
{
    setWindowIcon(icon("agros"));

    m_startupProblemFilename = "";
    m_startupExecute = false;

    // log stdout
    logStdOut = new LogStdOut();

    // scene
    // sceneViewVTK2D = new SceneViewVTK2D(postDeal, this);

    // preprocessor
    problemWidget = new PreprocessorWidget(this);
    // postprocessor
    postprocessorWidget = new PostprocessorWidget();

    connect(postprocessorWidget, SIGNAL(changed()), this, SLOT(setControls()));
    connect(postprocessorWidget, SIGNAL(modeChanged(PostprocessorWidgetMode)), this, SLOT(setControls()));

    // problem
    exampleWidget = new ExamplesWidget(this);
    connect(exampleWidget, SIGNAL(problemOpen(QString)), this, SLOT(doDocumentOpen(QString)));

    // view info
    m_connectLog = new ConnectLog();
    logView = new LogViewDialog(this, m_connectLog);
    logView->setVisible(false);
    (dynamic_cast<LogGui * > (Agros::log()))->setConnectLog(m_connectLog);

    // OptiLab
    optiLab = new OptiLab(this);

    createActions();
    createMenus();
    createMain();

    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(setControls()));
    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), problemWidget->sceneViewProblem(), SLOT(refresh()));

    // preprocessor
    connect(problemWidget->sceneViewProblem(), SIGNAL(sceneGeometryModeChanged(SceneGeometryMode)), problemWidget, SLOT(loadTooltip(SceneGeometryMode)));

    problemWidget->sceneViewProblem()->clear();

    Agros::problem()->clearFieldsAndConfig();

    exampleWidget->actExamples->trigger();
    problemWidget->sceneViewProblem()->doZoomBestFit();

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

    setControls();

    doApplyStyle();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/State", saveState());    

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());

    delete m_connectLog;
    if (logStdOut != nullptr)
        delete logStdOut;
}

void MainWindow::createActions()
{
    actDocumentNew = new QAction(tr("&New..."), this);
    actDocumentNew->setShortcuts(QKeySequence::New);
    connect(actDocumentNew, SIGNAL(triggered()), this, SLOT(doDocumentNew()));

    actDocumentOpen = new QAction(tr("&Open..."), this);
    actDocumentOpen->setShortcuts(QKeySequence::Open);
    connect(actDocumentOpen, SIGNAL(triggered()), this, SLOT(doDocumentOpen()));

    actDocumentSave = new QAction(tr("&Save"), this);
    actDocumentSave->setShortcuts(QKeySequence::Save);
    connect(actDocumentSave, SIGNAL(triggered()), this, SLOT(doDocumentSave()));

    actDeleteSolutions = new QAction(tr("Delete solutions"), this);
    connect(actDeleteSolutions, SIGNAL(triggered()), this, SLOT(doDeleteSolutions()));
    actDeleteSolutionsAndResults = new QAction(tr("Delete solutions and results"), this);
    connect(actDeleteSolutionsAndResults, SIGNAL(triggered()), this, SLOT(doDeleteSolutionsAndResults()));

    actDocumentSaveAs = new QAction(tr("Save &As..."), this);
    actDocumentSaveAs->setShortcuts(QKeySequence::SaveAs);
    connect(actDocumentSaveAs, SIGNAL(triggered()), this, SLOT(doDocumentSaveAs()));

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

    actCreateFromModel = new QAction(tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence("Ctrl+M"));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    actExit = new QAction(tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actCopy = new QAction(tr("Copy image to clipboard"), this);
    // actCopy->setShortcuts(QKeySequence::Copy);
    connect(actCopy, SIGNAL(triggered()), this, SLOT(doCopy()));

    actCheckVersion = new QAction(tr("Check version"), this);
    connect(actCheckVersion, SIGNAL(triggered()), this, SLOT(doCheckVersion()));

    actAbout = new QAction(tr("About &agros"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actOptions = new QAction(tr("&Options"), this);
    actOptions->setMenuRole(QAction::PreferencesRole);
    connect(actOptions, SIGNAL(triggered()), this, SLOT(doOptions()));

    actFullScreen = new QAction(tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actDocumentOpenRecentGroup = new QActionGroup(this);
    connect(actDocumentOpenRecentGroup, SIGNAL(triggered(QAction *)), this, SLOT(doDocumentOpenRecent(QAction *)));

    actShowLog = new QAction(tr("Log dialog"), this);
    connect(actShowLog, SIGNAL(triggered()), this, SLOT(doShowLog()));

    actMaterialBrowser = new QAction(tr("Material browser..."), this);
    connect(actMaterialBrowser, SIGNAL(triggered()), this, SLOT(doMaterialBrowser()));

    actSolve = new QAction(icon("main_solve"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence("Alt+S"));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolve()));

    actSolveNewComputation = new QAction(icon("main_solvenew"), tr("&Solve new"), this);
    actSolveNewComputation->setShortcut(QKeySequence(tr("Alt+Shift+S")));
    connect(actSolveNewComputation, SIGNAL(triggered()), this, SLOT(doSolveNewComputation()));

    actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(exampleWidget->actExamples);
    actSceneModeGroup->addAction(problemWidget->sceneViewProblem()->actSceneModeProblem);
    actSceneModeGroup->addAction(postprocessorWidget->actSceneModeResults);
    actSceneModeGroup->addAction(optiLab->actSceneModeOptiLab);

    // apply stylesheet
    timerApplyStyle = new QTimer(this);
    connect(timerApplyStyle, &QTimer::timeout, this, &MainWindow::doApplyStyle);
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

    mnuTools = menuBar()->addMenu(tr("&Tools"));
    mnuTools->addAction(actMaterialBrowser);
    mnuTools->addSeparator();
    mnuTools->addAction(actCreateFromModel);
    mnuTools->addSeparator();
    mnuTools->addAction(actFullScreen);
    mnuTools->addAction(actShowLog);
    mnuTools->addAction(actOptions);

    mnuHelp = menuBar()->addMenu(tr("&Help"));
    mnuHelp->addAction(actCheckVersion);
    mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros" MacOSX menu
}

void MainWindow::createMain()
{
    tabControlsLayout = new QStackedLayout();
    tabControlsLayout->addWidget(exampleWidget);
    tabControlsLayout->addWidget(problemWidget);
    tabControlsLayout->addWidget(postprocessorWidget);
    tabControlsLayout->addWidget(optiLab);

    viewControls = new QWidget();
    viewControls->setLayout(tabControlsLayout);

    // spacing
    QLabel *spacing = new QLabel;
    spacing->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // left toolbar
    QToolBar *tlbLeftBar = new QToolBar();
    tlbLeftBar->setOrientation(Qt::Vertical);
    tlbLeftBar->setProperty("leftbar", true);
    tlbLeftBar->setIconSize(QSize(32, 32));
    tlbLeftBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    tlbLeftBar->addAction(exampleWidget->actExamples);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(problemWidget->sceneViewProblem()->actSceneModeProblem);
    tlbLeftBar->addAction(postprocessorWidget->actSceneModeResults);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(optiLab->actSceneModeOptiLab);
    tlbLeftBar->addWidget(spacing);
    tlbLeftBar->addAction(actSolve);
    tlbLeftBar->addAction(actSolveNewComputation);
    tlbLeftBar->addAction(optiLab->optiLabWidget()->actRunStudy);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(tlbLeftBar);
    layoutMain->addWidget(viewControls);

    QWidget *main = new QWidget();
    main->setLayout(layoutMain);

    setCentralWidget(main);
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
    // clear preprocessor
    Agros::problem()->clearFieldsAndConfig();

    problemWidget->sceneViewProblem()->actSceneModeProblem->trigger();
    problemWidget->sceneViewProblem()->doZoomBestFit();
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

            problemWidget->sceneViewProblem()->actSceneModeProblem->trigger();
            problemWidget->sceneViewProblem()->doZoomBestFit();

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
    problemWidget->sceneViewProblem()->actSceneModeProblem->trigger();

    // clear solutions
    foreach (QSharedPointer<Computation> computation, Agros::singleton()->computations())
        computation->clearSolution();

    optiLab->optiLabWidget()->refresh();
    problemWidget->sceneViewProblem()->refresh();
    setControls();
}

void MainWindow::doDeleteSolutionsAndResults()
{
    problemWidget->sceneViewProblem()->actSceneModeProblem->trigger();

    // clear all computations
    Agros::clearComputations();

    optiLab->optiLabWidget()->refresh();
    problemWidget->sceneViewProblem()->refresh();
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
        problemWidget->sceneViewProblem()->doZoomBestFit();

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

        if (problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked())
            problemWidget->sceneViewProblem()->saveImageToFile(fileName);
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

        problemWidget->sceneViewProblem()->saveGeometryToSvg(fileName);

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
        problemWidget->sceneViewProblem()->refresh();
        setControls();
    }
}

void MainWindow::doMaterialBrowser()
{
    MaterialBrowserDialog materialBrowserDialog(this);
    materialBrowserDialog.showDialog(false);
}

void MainWindow::doShowLog()
{
    logView->setVisible(true);
}

void MainWindow::doCut()
{

}

void MainWindow::doCopy()
{
    // copy image to clipboard
    QPixmap pixmap;
    if (problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked())
    {
        pixmap = problemWidget->sceneViewProblem()->renderScenePixmap();
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
            pixmap = postprocessorWidget->sceneViewChart()->chartView()->grab();
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

void MainWindow::doApplyStyle()
{
    QFile f(QString("%1/resources/themes/theme.qss").arg(Agros::dataDir()));

    if (!f.exists())
    {
        qInfo() << "Unable to set stylesheet, file not found";
    }
    else
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
}

void MainWindow::setEnabledControls(bool state)
{
    tabControlsLayout->setEnabled(state);

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
    actSolve->setEnabled(problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked());
    actSolve->setVisible(problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked());
    actSolveNewComputation->setEnabled(problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked());
    actSolveNewComputation->setVisible(problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked());
    optiLab->optiLabWidget()->actRunStudy->setEnabled(optiLab->actSceneModeOptiLab->isChecked());
    optiLab->optiLabWidget()->actRunStudy->setVisible(optiLab->actSceneModeOptiLab->isChecked());

    postprocessorWidget->refresh();

    if (exampleWidget->actExamples->isChecked())
    {
        tabControlsLayout->setCurrentWidget(exampleWidget);
    }
    else if (problemWidget->sceneViewProblem()->actSceneModeProblem->isChecked())
    {
        tabControlsLayout->setCurrentWidget(problemWidget);
        // problemWidget->sceneViewProblem()->actSceneZoomRegion = actSceneZoomRegion;
    }
    else if (postprocessorWidget->actSceneModeResults->isChecked())
    {
        tabControlsLayout->setCurrentWidget(postprocessorWidget);
    }
    else if (optiLab->actSceneModeOptiLab->isChecked())
    {
        tabControlsLayout->setCurrentWidget(optiLab);
    }

    // menu bar
    menuBar()->clear();
    menuBar()->addMenu(mnuFile);
    menuBar()->addMenu(mnuEdit);
    menuBar()->addMenu(mnuTools);
    menuBar()->addMenu(mnuHelp);

    // window title
    setWindowTitle(QString("Agros - %1").arg(Agros::problem()->archiveFileName()));

    // update preprocessor
    problemWidget->refresh();
    // update postprocessor
    postprocessorWidget->refresh();
    // update optilab
    optiLab->refresh();

    if (Agros::configComputer()->value(Config::Config_ReloadStyle).toBool())
        timerApplyStyle->start(1000);
    else
        timerApplyStyle->stop();


    setUpdatesEnabled(true);
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
            boost::archive::text_oarchive sbMesh(ofsMesh);
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

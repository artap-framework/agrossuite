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
#include "optilab/optilab_widget.h"
#include "materialbrowserdialog.h"
#include "chartdialog.h"
#include "examplesdialog.h"

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_oarchive.hpp>

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent),
    logSolverDialog(nullptr)
{
    setWindowIcon(icon("agros"));

    m_startupProblemFilename = "";
    m_startupExecute = false;

    // preprocessor
    problemWidget = new PreprocessorWidget(this);
    // postprocessor
    postprocessorWidget = new PostprocessorWidget();

    connect(postprocessorWidget, SIGNAL(changed()), this, SLOT(setControls()));
    connect(postprocessorWidget, SIGNAL(modeChanged(PostprocessorWidgetMode)), this, SLOT(setControls()));

    // problem
    exampleWidget = new ExamplesWidget(this);
    connect(exampleWidget, SIGNAL(problemOpen(QString)), this, SLOT(doDocumentOpen(QString)));

    // create log view dialog
    m_connectLog = new ConnectLog();
    createLogViewDialog(m_connectLog);
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

    // delete log view dialog
    deleteLogViewDialog();

    delete m_connectLog;
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

    actCreateFromModel = new QAction(tr("&Create script from model"), this);
    actCreateFromModel->setShortcut(QKeySequence("Ctrl+M"));
    connect(actCreateFromModel, SIGNAL(triggered()), this, SLOT(doCreatePythonFromModel()));

    actExit = new QAction(tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actCheckVersion = new QAction(tr("Check version"), this);
    connect(actCheckVersion, SIGNAL(triggered()), this, SLOT(doCheckVersion()));

    actAbout = new QAction(tr("About &agros"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actShortcuts = new QAction(tr("Shortcuts"), this);
    actShortcuts->setMenuRole(QAction::AboutRole);
    connect(actShortcuts, SIGNAL(triggered()), this, SLOT(doShortcuts()));

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
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolveCurrentComputation()));

    actSolveNewComputation = new QAction(icon("main_solvenew"), tr("&Solve new"), this);
    actSolveNewComputation->setShortcut(QKeySequence(tr("Alt+Shift+S")));
    connect(actSolveNewComputation, SIGNAL(triggered()), this, SLOT(doSolveNewComputation()));

    // solve from optilab
    connect(optiLab, SIGNAL(doSolveCurrentComputation(Computation *)), this, SLOT(doSolveComputation(Computation *)));

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

    mnuFile = menuBar()->addMenu(tr("&File"));
    mnuFile->addAction(actDocumentNew);
    mnuFile->addAction(actDocumentOpen);
    mnuFile->addMenu(mnuRecentFiles);
    mnuFile->addSeparator();
    mnuFile->addAction(actDocumentSave);
    mnuFile->addAction(actDocumentSaveAs);
    mnuFile->addSeparator();
    mnuFile->addAction(actDeleteSolutions);
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
    mnuHelp->addAction(actShortcuts);
    mnuHelp->addAction(actAbout);   // will be added to "Agros" MacOSX menu
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
    tlbLeftBar->setProperty("os", operatingSystem());
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
    // check if problem has changed
    if (Agros::problem()->hasChanged())
    {
        if (!checkModifiedQuestion())
            return;
    }

    // clear preprocessor
    Agros::problem()->clearFieldsAndConfig();

    problemWidget->sceneViewProblem()->actSceneModeProblem->trigger();
    problemWidget->sceneViewProblem()->doZoomBestFit();
}

void MainWindow::doDocumentOpen(const QString &fileName)
{
    // check if problem has changed
    if (Agros::problem()->hasChanged())
    {
        if (!checkModifiedQuestion())
            return;
    }

    QSettings settings;
    QString fileNameDocument;

    if (fileName.isEmpty())
    {
        QString dir = settings.value("General/LastProblemDir", "data").toString();

        fileNameDocument = QFileDialog::getOpenFileName(this, tr("Open file"), dir, tr("agros files (*.ags *.a2d);;agros data files (*.ags);;Agros2D data files - deprecated (*.a2d)"));
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
            Agros::problem()->writeProblemToFile(Agros::problem()->archiveFileName(), false);
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
            Agros::problem()->writeProblemToFile(fileName, false);
            setRecentFiles();
        }
        catch (AgrosException &e)
        {
            Agros::log()->printError(tr("Problem"), e.toString());
        }
    }
}

bool MainWindow::doDocumentClose()
{
    // check if problem has changed
    if (Agros::problem()->hasChanged())
    {
        if (!checkModifiedQuestion())
            return false;
    }

    // clear problem
    Agros::problem()->clearFieldsAndConfig();

    exampleWidget->actExamples->trigger();
    return true;
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

void MainWindow::doCreatePythonFromModel()
{
    auto *scriptDialog = new ScriptGeneratorDialog();
    scriptDialog->doGenerate();
    scriptDialog->show();
}

int MainWindow::checkModifiedQuestion()
{
    QMessageBox msgBox;
    msgBox.setStyleSheet("QLabel{min-width: 300px;}");
    msgBox.setText(tr("The model has been modified."));
    if (Agros::problem()->archiveFileName() == "")
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
    else
        msgBox.setInformativeText(tr("Do you want to save your changes to file '%1'?").arg(QFileInfo(Agros::problem()->archiveFileName()).baseName()));
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);

    const int result = msgBox.exec();
    switch (result) {
    case QMessageBox::Save:
        doDocumentSave();
        return true;
        break;
    case QMessageBox::Discard:
        return true;
        break;
    case QMessageBox::Cancel:
        return false;
        break;
    default:
        // should never be reached
        break;
    }
}

void MainWindow::doSolveCurrentComputation()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(false);
    doSolveComputation(computation.data());
}

void MainWindow::doSolveNewComputation()
{
    // create computation from preprocessor
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);
    doSolveComputation(computation.data());
}

void MainWindow::doSolveComputation(Computation *computation)
{
    logSolverDialog = new LogSolverDialog(computation, tr("Solver"), m_connectLog);
    logSolverDialog->show();

    // solve thread
    auto *solveThread = new SolveThread(computation);
    connect(solveThread, SIGNAL(finished()), this, SLOT(doSolveFinished()));
    solveThread->startCalculation();
}

void MainWindow::doSolveFinished()
{
    if (Agros::problem()->currentComputation()->isSolved())
    {
        // close log dialog
        logSolverDialog->closeLog();

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
    showLogViewDialog();
}

void MainWindow::clear()
{
    setControls();
}

void MainWindow::doApplyStyle()
{
    const QString theme = readFileContent(QString("%1/resources/themes/theme.qss").arg(Agros::dataDir()));
    qApp->setStyleSheet(theme);
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

void MainWindow::doShortcuts()
{
    ShortcutDialog shortcut(this);
    shortcut.exec();
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
            doSolveCurrentComputation();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (doDocumentClose())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

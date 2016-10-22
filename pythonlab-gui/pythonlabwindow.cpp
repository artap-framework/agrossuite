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

#include "pythonlabwindow.h"
#include "pythonengine.h"
#include "pythoneditor.h"

#include "other/util.h"

MainWindow::MainWindow(int argc, char *argv[], QWidget *parent) : QMainWindow(parent)
{
    setWindowIcon(icon("agros2d"));

    m_startupScriptFilename = "";

    // PythonLab
    createPythonEngine(argc, argv, new PythonEngine());
    scriptEditor = new PythonEditorView(this);

    splitterMain = new QSplitter(Qt::Horizontal, this);
    splitterMain->addWidget(scriptEditor->pythonEditorWidget());
    splitterMain->addWidget(scriptEditor);
    splitterMain->setCollapsible(0, false);
    splitterMain->setCollapsible(1, false);
    splitterMain->setStretchFactor(0, 1);
    splitterMain->setStretchFactor(1, 4);

    createActions();

    setCentralWidget(splitterMain);

    // accept drops
    setAcceptDrops(true);

    // macx
    setUnifiedTitleAndToolBarOnMac(true);

    QSettings settings;
    restoreGeometry(settings.value("PythonLab/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("PythonLab/State", saveState()).toByteArray());

    // show/hide control and view panel
    actHideControlPanel->setChecked(settings.value("PythonLab/ControlPanel", true).toBool());

    setControls();
}

MainWindow::~MainWindow()
{
    QSettings settings;
    settings.setValue("PythonLab/Geometry", saveGeometry());
    settings.setValue("PythonLab/State", saveState());
    settings.setValue("PythonLab/ControlPanel", actHideControlPanel->isChecked());

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());
}

void MainWindow::createActions()
{
    actExit = new QAction(tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actFullScreen = new QAction(icon("view-fullscreen"), tr("Fullscreen mode"), this);
    actFullScreen->setShortcut(QKeySequence(tr("F11")));
    connect(actFullScreen, SIGNAL(triggered()), this, SLOT(doFullScreen()));

    actHideControlPanel = new QAction(iconAwesome(fa::columns), tr("Show/hide control panel"), this);
    actHideControlPanel->setShortcut(tr("Alt+0"));
    actHideControlPanel->setCheckable(true);
    connect(actHideControlPanel, SIGNAL(triggered()), this, SLOT(doHideControlPanel()));
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
            // doDocumentOpen(fileName);

            event->acceptProposedAction();
        }
    }
}

void MainWindow::doFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::doHideControlPanel()
{
    viewControls->setVisible(actHideControlPanel->isChecked());
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
    menuBar()->setEnabled(state);

    centralWidget()->setEnabled(state);
}

void MainWindow::setControls()
{
    setUpdatesEnabled(false);
    setEnabled(true);

    // menu bar
    menuBar()->clear();
    menuBar()->addMenu(scriptEditor->mnuFile);
    menuBar()->addMenu(scriptEditor->mnuEdit);
    menuBar()->addMenu(scriptEditor->mnuTools);
    menuBar()->addMenu(scriptEditor->mnuHelp);

    setUpdatesEnabled(true);
}

void MainWindow::showEvent(QShowEvent *event)
{
    // startup
    if (!m_startupScriptFilename.isEmpty())
    {
        // open script
        scriptEditor->doFileOpen(m_startupScriptFilename);

        m_startupScriptFilename = "";
    }
}

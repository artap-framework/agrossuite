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

#include "optilab.h"

#include "util/constants.h"
#include "gui/lineeditdouble.h"
#include "gui/common.h"
#include "gui/systemoutput.h"
#include "gui/about.h"

#include "logview.h"

#include <fstream>
#include <string>

#include "pythonlab/pythonengine_agros.h"

OptilabWindow::OptilabWindow(int argc, char *argv[]) : QMainWindow()
{
    setWindowIcon(icon("optilab"));
    setWindowTitle(tr("Optilab"));

    m_startupProblemFilename = "";

    // python

    // silent mode
    setSilentMode(true);

    createPythonEngine(argc, argv, new PythonEngineAgros());
    scriptEditorDialog = new PythonEditorAgrosDialog(currentPythonEngine(), QStringList(), NULL);

    variantsWidget = new VariantsWidget(this);
    analysesWidget = new AnalysesWidget(this);

    createActions();
    createToolBars();
    createMain();

    variantsWidget->actVariants->trigger();

    QSettings settings;
    restoreGeometry(settings.value("OptilabWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("OptilabWindow/State", saveState()).toByteArray());
}

OptilabWindow::~OptilabWindow()
{
    QSettings settings;
    settings.setValue("OptilabWindow/Geometry", saveGeometry());
    settings.setValue("OptilabWindow/State", saveState());

    removeDirectory(tempProblemDir());
}

void OptilabWindow::doScriptEditor()
{
    scriptEditorDialog->showDialog();
}

void OptilabWindow::createActions()
{
    actScriptEditor = new QAction(icon("script-python"), tr("PythonLab"), this);
    actScriptEditor->setShortcut(Qt::Key_F9);
    connect(actScriptEditor, SIGNAL(triggered()), this, SLOT(doScriptEditor()));

    QActionGroup *actSceneModeGroup = new QActionGroup(this);
    actSceneModeGroup->addAction(variantsWidget->actVariants);
    actSceneModeGroup->addAction(analysesWidget->actAnalyses);

    connect(actSceneModeGroup, SIGNAL(triggered(QAction *)), this, SLOT(setControls()));
}

void OptilabWindow::createToolBars()
{
    // top toolbar
#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    QToolBar *tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->setOrientation(Qt::Horizontal);
    tlbTools->setAllowedAreas(Qt::TopToolBarArea);
    tlbTools->setMovable(false);
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->addSeparator();
}

void OptilabWindow::createMain()
{
    consoleView = new PythonScriptingConsoleView(currentPythonEngine(), this);
    consoleView->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    consoleView->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, consoleView);

    logView = new LogView(this);
    logView->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, logView);

    tabViewLayout = new QStackedLayout();
    tabViewLayout->setContentsMargins(0, 0, 0, 0);
    tabViewLayout->addWidget(variantsWidget);
    tabViewLayout->addWidget(analysesWidget);
    tabViewLayout->setCurrentWidget(variantsWidget);

    QWidget *viewWidget = new QWidget();
    viewWidget->setLayout(tabViewLayout);

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
    tlbLeftBar->setStyleSheet(QString("QToolBar { border: 1px solid rgba(200, 200, 200, 255); }"
                                      "QToolBar { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:1 rgba(120, 120, 120, 255)); }"
                                      "QToolButton { border: 0px; color: rgba(230, 230, 230, 255); font: bold; font-size: %1pt; width: 65px; }"
                                      "QToolButton:hover { border: 0px; background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(70, 70, 70, 255), stop:0.5 rgba(160, 160, 160, 255), stop:1 rgba(150, 150, 150, 255)); }"
                                      "QToolButton:checked:hover, QToolButton:checked { border: 0px; color: rgba(30, 30, 30, 255); background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(160, 160, 160, 255), stop:0.5 rgba(220, 220, 220, 255), stop:1 rgba(160, 160, 160, 255)); }").arg(fontSize));
    // system layout
    // leftToolBar->setStyleSheet("QToolButton { font: bold; font-size: 8pt; width: 65px; }");

    tlbLeftBar->setIconSize(QSize(32, 32));
    tlbLeftBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    tlbLeftBar->addAction(variantsWidget->actVariants);
    tlbLeftBar->addAction(analysesWidget->actAnalyses);
    tlbLeftBar->addWidget(spacing);
    tlbLeftBar->addSeparator();
    tlbLeftBar->addAction(actScriptEditor);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(tlbLeftBar);
    layoutMain->addWidget(viewWidget);

    QWidget *main = new QWidget();
    main->setLayout(layoutMain);

    setCentralWidget(main);
}

void OptilabWindow::showDialog()
{
    show();
    activateWindow();
}

void OptilabWindow::setControls()
{
    if (variantsWidget->actVariants->isChecked())
    {
        tabViewLayout->setCurrentWidget(variantsWidget);
    }
    else if (analysesWidget->actAnalyses->isChecked())
    {
        tabViewLayout->setCurrentWidget(analysesWidget);
    }
}

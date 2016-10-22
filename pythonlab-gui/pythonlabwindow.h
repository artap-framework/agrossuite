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

#ifndef PYTHONLABWINDOW_H
#define PYTHONLABWINDOW_H

#include "pythonlabutil.h"
#include "pythoneditor.h"

class AGROS_LIBRARY_API MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int argc, char *argv[], QWidget *parent = 0);
    ~MainWindow();

    inline void setStartupScriptFilename(const QString& fn) { m_startupScriptFilename = fn; }

private slots:
    void doFullScreen();
    void doHideControlPanel();

    void doStartedScript();
    void setEnabledControls(bool state);
    void setControls();
    void clear();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void showEvent(QShowEvent *event);    

private:
    QAction *actExit;
    QAction *actHideControlPanel;
    QAction *actFullScreen;

    QString m_startupScriptFilename;
    PythonEditorView *scriptEditor;

    QSplitter *splitterMain;
    QWidget *viewControls;

    void createActions();
    void createToolBox();
};

#endif // PYTHONLABWINDOW_H

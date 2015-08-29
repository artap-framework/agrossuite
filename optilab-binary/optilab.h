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

#ifndef OPTILABDIALOG_H
#define OPTILABDIALOG_H

#include "util.h"
#include "optilab_data.h"
#include "optilab_analyses.h"
#include "optilab_variants.h"

#include "qcustomplot/qcustomplot.h"

class InfoWidget;
class PythonEditorAgrosDialog;
class PythonScriptingConsoleView;
class LogView;

class OptilabWindow : public QMainWindow
{
    Q_OBJECT
public:
    OptilabWindow(int argc, char *argv[]);
    ~OptilabWindow();

    void showDialog();
    inline void setStartupProblemFilename(const QString& fn) { m_startupProblemFilename = fn; }

public slots:
    void doScriptEditor();
    void setControls();

private:
    QString m_startupProblemFilename;

    QStackedLayout *tabViewLayout;
    InfoWidget *infoWidget;
    VariantsWidget *variantsWidget;
    AnalysesWidget *analysesWidget;

    PythonScriptingConsoleView *consoleView;
    LogView *logView;

    QAction *actScriptEditor;

    PythonEditorAgrosDialog *scriptEditorDialog;

    void createActions();
    void createToolBars();
    void createMain();
};

#endif // OPTILABDIALOG_H

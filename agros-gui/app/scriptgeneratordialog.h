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

#ifndef SCRIPTGENERATORDIALOG_H
#define SCRIPTGENERATORDIALOG_H

#include "util/util.h"
#include "gui/other.h"
#include "app/scriptgenerator_utils.h"

#include "util/script_generator.h"

#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtGui/QSyntaxHighlighter>

#include <QSyntaxHighlighter>

#include <QRegExp>
#include <QHash>
#include <QTextCharFormat>

class ScriptGeneratorDialog : public QDialog
{
    Q_OBJECT
public:
    ScriptGeneratorDialog(QWidget *parent = 0);
    ~ScriptGeneratorDialog();

public slots:
    void doGenerate();
    void doGenerateWithStudies();
    void doGenerateModelForServer();

private slots:
    void doSaveAs();
    void doClose();

protected:
    void createControls();

private:
    ScriptEditor *pythonScript;

    QWidget *widCreate;
    QCheckBox *chkCopy;

    QCheckBox *chkParametersAsVariables;

    ScriptGenerator m_scriptGenerator;
};

#endif // SCRIPTGENERATORDIALOG_H

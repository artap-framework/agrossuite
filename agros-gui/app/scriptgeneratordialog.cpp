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

#include "scriptgeneratordialog.h"

#include "util/global.h"
#include "solver/problem.h"
#include "optilab/study.h"

ScriptGeneratorDialog::ScriptGeneratorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Script Generator"));
    setModal(true);
    setWindowFlags(Qt::WindowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    createControls();

    setMinimumSize(QApplication::activeWindow()->width()/2, QApplication::activeWindow()->height()/2);

    // QSettings settings;
    // restoreGeometry(settings.value("ScriptGeneratorDialog/Geometry", saveGeometry()).toByteArray());
}

ScriptGeneratorDialog::~ScriptGeneratorDialog()
{
    // QSettings settings;
    // settings.setValue("ScriptGeneratorDialog/Geometry", saveGeometry());
}

void ScriptGeneratorDialog::createControls()
{
    chkParametersAsVariables = new QCheckBox(tr("Interpret parameters as Python variables"));
    chkParametersAsVariables->setChecked((Agros::problem()->studies()->items().count() == 0));

    QVBoxLayout *layoutConfig = new QVBoxLayout();
    // layoutConfig->setContentsMargins(0, 0, 0, 0);
    layoutConfig->addWidget(chkParametersAsVariables);

    pythonScript = new ScriptEditor(this);

    // highlighter
    new PythonHighlighter(pythonScript->document());

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(pythonScript, 1);
    layoutMain->addLayout(layoutConfig);

    // dialog buttons
    QPushButton *btnGenerate = new QPushButton(tr("Generate"));
    connect(btnGenerate, SIGNAL(clicked()), this, SLOT(doGenerate()));
    btnGenerate->setDefault(true);

    QPushButton *btnGenerateWithStudies = new QPushButton(tr("Generate with studies"));
    connect(btnGenerateWithStudies, SIGNAL(clicked()), this, SLOT(doGenerateWithStudies()));
    btnGenerateWithStudies->setDefault(true);

    QPushButton *btnGenerateModelForServer = new QPushButton(tr("Generate model for server"));
    connect(btnGenerateModelForServer, SIGNAL(clicked()), this, SLOT(doGenerateModelForServer()));
    btnGenerateModelForServer->setDefault(true);

    QPushButton *btnOpen = new QPushButton(tr("Save to file ..."));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(doSaveAs()));

    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonBox = new QHBoxLayout();
    layoutButtonBox->addStretch();
    layoutButtonBox->addWidget(btnGenerate);
    layoutButtonBox->addWidget(btnGenerateWithStudies);
    layoutButtonBox->addWidget(btnGenerateModelForServer);
    layoutButtonBox->addSpacing(20);
    layoutButtonBox->addWidget(btnOpen);
    layoutButtonBox->addWidget(btnClose);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutMain);
    // layout->addStretch();
    layout->addLayout(layoutButtonBox);

    setLayout(layout);
}

void ScriptGeneratorDialog::doSaveAs()
{
    QSettings settings;
    QString dir = settings.value("General/LastPythonDir", "data").toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to Python file"), dir, tr("Python (*.py)"));
    if (!fileName.isEmpty())
    {
        QString script = pythonScript->toPlainText();

        writeStringContent(fileName, script);
    }

    // open in external editor
    // QDesktopServices desk;
    // desk.openUrl(QUrl::fromLocalFile(fn));
}

void ScriptGeneratorDialog::doGenerate()
{
    m_scriptGenerator.setAddComputation(true);
    m_scriptGenerator.setAddStudies(false);
    m_scriptGenerator.setParametersAsVariables(chkParametersAsVariables->isChecked());

    QString script = m_scriptGenerator.createPythonFromModel();
    pythonScript->setPlainText(script);
}

void ScriptGeneratorDialog::doGenerateWithStudies()
{
    m_scriptGenerator.setAddComputation(true);
    m_scriptGenerator.setAddStudies(true);
    m_scriptGenerator.setParametersAsVariables(chkParametersAsVariables->isChecked());

    QString script = m_scriptGenerator.createPythonFromModel();
    pythonScript->setPlainText(script);
}

void ScriptGeneratorDialog::doGenerateModelForServer()
{
    QString script = m_scriptGenerator.createPythonFromModelForServer();
    pythonScript->setPlainText(script);
}

void ScriptGeneratorDialog::doClose()
{
    close();
}

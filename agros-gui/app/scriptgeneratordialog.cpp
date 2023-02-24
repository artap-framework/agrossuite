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

    doGenerate();

    // QSettings settings;
    // restoreGeometry(settings.value("ScriptGeneratorDialog/Geometry", saveGeometry()).toByteArray());
}

ScriptGeneratorDialog::~ScriptGeneratorDialog()
{
    // QSettings settings;
    // settings.setValue("ScriptGeneratorDialog/Geometry", saveGeometry());
}


void ScriptGeneratorDialog::showDialog()
{
//    lstTranslateX->setText(Agros::problem()->config()->labelX() + ":");
//    lstTranslateY->setText(Agros::problem()->config()->labelY() + ":");
//    lstRotateBasePointX->setText(Agros::problem()->config()->labelX() + ":");
//    lstRotateBasePointY->setText(Agros::problem()->config()->labelY() + ":");
//    lstScaleBasePointX->setText(Agros::problem()->config()->labelX() + ":");
//    lstScaleBasePointY->setText(Agros::problem()->config()->labelY() + ":");

    show();
}

void ScriptGeneratorDialog::createControls()
{
//    // translate
//    txtTranslateX = new ValueLineEdit();
//    txtTranslateY = new ValueLineEdit();

//    lstTranslateX = new QLabel();
//    lstTranslateY = new QLabel();

//    QGridLayout *layoutTranslate = new QGridLayout();
//    layoutTranslate->addWidget(lstTranslateX, 0, 0);
//    layoutTranslate->addWidget(txtTranslateX, 0, 1);
//    layoutTranslate->addWidget(lstTranslateY, 1, 0);
//    layoutTranslate->addWidget(txtTranslateY, 1, 1);
//    layoutTranslate->addWidget(new QLabel(""), 2, 0);

//    widTranslate = new QWidget();
//    widTranslate->setLayout(layoutTranslate);

//    // rotate
//    txtRotateBasePointX = new ValueLineEdit();
//    txtRotateBasePointY = new ValueLineEdit();
//    txtRotateAngle = new ValueLineEdit();

//    lstRotateBasePointX = new QLabel();
//    lstRotateBasePointY = new QLabel();

//    QGridLayout *layoutRotate = new QGridLayout();
//    layoutRotate->addWidget(lstRotateBasePointX, 0, 0);
//    layoutRotate->addWidget(txtRotateBasePointX, 0, 1);
//    layoutRotate->addWidget(lstRotateBasePointY, 1, 0);
//    layoutRotate->addWidget(txtRotateBasePointY, 1, 1);
//    layoutRotate->addWidget(new QLabel(tr("Angle:")), 2, 0);
//    layoutRotate->addWidget(txtRotateAngle, 2, 1);

//    widRotate = new QWidget();
//    widRotate->setLayout(layoutRotate);

//    // scale
//    txtScaleBasePointX = new ValueLineEdit();
//    txtScaleBasePointY = new ValueLineEdit();
//    txtScaleFactor = new ValueLineEdit();

//    lstScaleBasePointX = new QLabel();
//    lstScaleBasePointY = new QLabel();

//    QGridLayout *layoutScale = new QGridLayout();
//    layoutScale->addWidget(lstScaleBasePointX, 0, 0);
//    layoutScale->addWidget(txtScaleBasePointX, 0, 1);
//    layoutScale->addWidget(lstScaleBasePointY, 1, 0);
//    layoutScale->addWidget(txtScaleBasePointY, 1, 1);
//    layoutScale->addWidget(new QLabel(tr("Scaling factor:")), 2, 0);
//    layoutScale->addWidget(txtScaleFactor, 2, 1);

//    widScale = new QWidget();
//    widScale->setLayout(layoutScale);

//    // copy
//    chkCopy = new QCheckBox(tr("Copy objects"));
//    connect(chkCopy, SIGNAL(toggled(bool)), this, SLOT(doCopyChecked(bool)));
//    chkWithMarkers->setEnabled(false);


    chkScriptAddComputation = new QCheckBox(tr("Add Computation block (with solve())"));
    chkScriptAddComputation->setChecked(true);

    chkScriptAddSolution = new QCheckBox(tr("Add Solution block"));
    chkScriptAddSolution->setChecked(true);

    chkParametersAsVariables = new QCheckBox(tr("Interpret parameters as Python variables"));
    chkParametersAsVariables->setChecked((Agros::problem()->studies()->items().count() == 0));

    QVBoxLayout *layoutConfig = new QVBoxLayout();
    // layoutConfig->setContentsMargins(0, 0, 0, 0);
    layoutConfig->addWidget(chkScriptAddComputation);
    layoutConfig->addWidget(chkScriptAddSolution);
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

    QPushButton *btnOpen = new QPushButton(tr("Save to file ..."));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(doSaveAs()));

    QPushButton *btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QHBoxLayout *layoutButtonBox = new QHBoxLayout();
    layoutButtonBox->addStretch();
    layoutButtonBox->addWidget(btnOpen);
    layoutButtonBox->addWidget(btnGenerate);
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
    m_scriptGenerator.setAddComputation(chkScriptAddComputation->isChecked());
    m_scriptGenerator.setAddSolution(chkScriptAddSolution->isChecked());
    m_scriptGenerator.setParametersAsVariables(chkParametersAsVariables->isChecked());

    QString script = m_scriptGenerator.createPythonFromModel();
    pythonScript->setPlainText(script);
}

void ScriptGeneratorDialog::doClose()
{
    close();
}

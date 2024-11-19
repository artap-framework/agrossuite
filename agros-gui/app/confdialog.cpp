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

#include "confdialog.h"

#include "util/global.h"
#include "util/conf.h"

#include "solver/module.h"

ConfigComputerDialog::ConfigComputerDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Options"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void ConfigComputerDialog::load()
{
    // language
    cmbLanguage->setCurrentIndex(cmbLanguage->findText(Agros::configComputer()->value(Config::Config_Locale).toString()));
    if (cmbLanguage->currentIndex() == -1 && cmbLanguage->count() > 0)
        cmbLanguage->setCurrentIndex(0);

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(Agros::configComputer()->value(Config::Config_ShowResults).toBool());

    // new version
    chkCheckNewVersion->setChecked(Agros::configComputer()->value(Config::Config_CheckNewVersion).toBool());

    // development
    chkDiscreteSaveMatrixRHS->setChecked(Agros::configComputer()->value(Config::Config_LinearSystemSave).toBool());
    cmbDumpFormat->setCurrentIndex(cmbDumpFormat->findData((MatrixExportFormat) Agros::configComputer()->value(Config::Config_LinearSystemFormat).toInt(), Qt::UserRole));

    // cache size
    txtCacheSize->setValue(Agros::configComputer()->value(Config::Config_CacheSize).toInt());

    // std log
    chkLogStdOut->setChecked(Agros::configComputer()->value(Config::Config_LogStdOut).toBool());

    // style
    chkApplyStyle->setChecked(Agros::configComputer()->value(Config::Config_ReloadStyle).toBool());

    // workspace
    chkShowRulersAndAxes->setChecked(Agros::configComputer()->value(Config::Config_ShowRulers).toBool());

    txtRulersFontSizes->setValue(Agros::configComputer()->value(Config::Config_RulersFontPointSize).toInt());
    txtPostFontSizes->setValue(Agros::configComputer()->value(Config::Config_PostFontPointSize).toInt());
}

void ConfigComputerDialog::save()
{
    // language
    if (Agros::configComputer()->value(Config::Config_Locale).toString() != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                             tr("Language change"),
                             tr("Interface language has been changed. You must restart the application."));
    Agros::configComputer()->setValue(Config::Config_Locale, cmbLanguage->currentText());

    // show result in line edit value widget
    Agros::configComputer()->setValue(Config::Config_ShowResults, chkLineEditValueShowResult->isChecked());

    // check version
    Agros::configComputer()->setValue(Config::Config_CheckNewVersion, chkCheckNewVersion->isChecked());

    // development
    Agros::configComputer()->setValue(Config::Config_LinearSystemSave, chkDiscreteSaveMatrixRHS->isChecked());
    Agros::configComputer()->setValue(Config::Config_LinearSystemFormat, (MatrixExportFormat) cmbDumpFormat->itemData(cmbDumpFormat->currentIndex(), Qt::UserRole).toInt());

    // cache size
    Agros::configComputer()->setValue(Config::Config_CacheSize, txtCacheSize->value());

    // std log
    Agros::configComputer()->setValue(Config::Config_LogStdOut, chkLogStdOut->isChecked());

    // style
    Agros::configComputer()->setValue(Config::Config_ReloadStyle, chkApplyStyle->isChecked());

    // workspace
    Agros::configComputer()->setValue(Config::Config_ShowRulers, chkShowRulersAndAxes->isChecked());

    Agros::configComputer()->setValue(Config::Config_RulersFontPointSize, txtRulersFontSizes->value());
    Agros::configComputer()->setValue(Config::Config_PostFontPointSize, txtPostFontSizes->value());

    // save
    Agros::configComputer()->save();
}

void ConfigComputerDialog::createControls()
{
    QTabWidget *tabConfig = new QTabWidget(this);
    tabConfig->addTab(createMainWidget(), tr("Main"));
    tabConfig->addTab(createSolverWidget(), tr("Solver"));
    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tabConfig);
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ConfigComputerDialog::createMainWidget()
{
    QWidget *mainWidget = new QWidget(this);

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 0, 0);
    layoutGeneral->addWidget(cmbLanguage, 0, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // other
    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));
    chkCheckNewVersion = new QCheckBox(tr("Check a new version at launch"));

    chkLogStdOut = new QCheckBox(tr("Print application log to standard output"));
    chkApplyStyle = new QCheckBox(tr("Reload stylesheet continuously"));

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkCheckNewVersion);
    layoutOther->addWidget(chkLogStdOut);
    layoutOther->addWidget(chkApplyStyle);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // workspace
    chkShowRulersAndAxes = new QCheckBox(tr("Show rulers"));

    QGridLayout *layoutGrid = new QGridLayout();
    layoutGrid->addWidget(chkShowRulersAndAxes, 1, 0);

    QGroupBox *grpGrid = new QGroupBox(tr("Grid"));
    grpGrid->setLayout(layoutGrid);

    txtRulersFontSizes = new QSpinBox();
    txtRulersFontSizes->setMinimum(6);
    txtRulersFontSizes->setMaximum(40);

    txtPostFontSizes = new QSpinBox();
    txtPostFontSizes->setMinimum(6);
    txtPostFontSizes->setMaximum(40);

    QGridLayout *layoutFont = new QGridLayout();
    layoutFont->setColumnStretch(1, 1);
    layoutFont->addWidget(new QLabel(tr("Rulers:")), 0, 0);
    layoutFont->addWidget(txtRulersFontSizes, 0, 1);
    layoutFont->addWidget(new QLabel(tr("Postprocessor:")), 2, 0);
    layoutFont->addWidget(txtPostFontSizes, 2, 1);

    QGroupBox *grpFont = new QGroupBox(tr("Fonts"));
    grpFont->setLayout(layoutFont);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpOther);
    layout->addWidget(grpGrid);
    layout->addWidget(grpFont);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ConfigComputerDialog::createSolverWidget()
{
    // general
    txtCacheSize = new QSpinBox(this);
    txtCacheSize->setMinimum(2);
    txtCacheSize->setMaximum(50);

    QGridLayout *layoutSolver = new QGridLayout();
    layoutSolver->addWidget(new QLabel(tr("Number of cache slots:")), 1, 0);
    layoutSolver->addWidget(txtCacheSize, 1, 1);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    chkDiscreteSaveMatrixRHS = new QCheckBox(tr("Save matrix and RHS"));
    cmbDumpFormat = new QComboBox(this);
    // cmbDumpFormat->addItem(dumpFormatString(EXPORT_FORMAT_PLAIN_ASCII), EXPORT_FORMAT_PLAIN_ASCII);
    cmbDumpFormat->addItem(dumpFormatString(EXPORT_FORMAT_MATLAB_MATIO), EXPORT_FORMAT_MATLAB_MATIO);
    // cmbDumpFormat->addItem(dumpFormatString(DF_MATRIX_MARKET), DF_MATRIX_MARKET);

    QGridLayout *layoutDevelopment = new QGridLayout();
    layoutDevelopment->addWidget(chkDiscreteSaveMatrixRHS, 0, 0, 1, 2);
    layoutDevelopment->addWidget(new QLabel(tr("Matrix format")), 1, 0);
    layoutDevelopment->addWidget(cmbDumpFormat, 1, 1);

    QGroupBox *grpDevelopment = new QGroupBox(tr("Development"));
    grpDevelopment->setLayout(layoutDevelopment);

    QVBoxLayout *layoutGeneral = new QVBoxLayout();
    layoutGeneral->addWidget(grpSolver);
    layoutGeneral->addWidget(grpDevelopment);
    layoutGeneral->addStretch();

    QWidget *solverGeneralWidget = new QWidget(this);
    solverGeneralWidget->setLayout(layoutGeneral);

    return solverGeneralWidget;
}

void ConfigComputerDialog::fillComboBoxPhysicField(QComboBox *cmbPhysicField)
{
    // block signals
    cmbPhysicField->blockSignals(true);

    cmbPhysicField->clear();
    QMapIterator<QString, QString> it(Module::availableModules());
    while (it.hasNext())
    {
        it.next();
        cmbPhysicField->addItem(it.value(), it.key());
    }

    // unblock signals
    cmbPhysicField->blockSignals(false);

    // physic field
    if (cmbPhysicField->currentIndex() == -1)
        cmbPhysicField->setCurrentIndex(0);
}

void ConfigComputerDialog::doAccept()
{
    save();
    accept();
}

void ConfigComputerDialog::doReject()
{
    reject();
}


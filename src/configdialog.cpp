// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "configdialog.h"

#include "gui.h"

#include "scene.h"
#include "sceneview.h"
#include "pythonlabagros.h"

ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent)
{
    logMessage("ConfigDialog::ConfigDialog()");

    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void ConfigDialog::load()
{
    logMessage("ConfigDialog::load()");

    // gui style
    cmbGUIStyle->setCurrentIndex(cmbGUIStyle->findText(Util::config()->guiStyle));
    if (cmbGUIStyle->currentIndex() == -1 && cmbGUIStyle->count() > 0) cmbGUIStyle->setCurrentIndex(0);

    // language
    cmbLanguage->setCurrentIndex(cmbLanguage->findText(Util::config()->language));
    if (cmbLanguage->currentIndex() == -1 && cmbLanguage->count() > 0) cmbLanguage->setCurrentIndex(0);

    // default physic field
    cmbDefaultPhysicField->setCurrentIndex(cmbDefaultPhysicField->findData(Util::config()->defaultPhysicField));

    // collaboration server
    txtCollaborationServerURL->setText(Util::config()->collaborationServerURL);

    // check version
    chkCheckVersion->setChecked(Util::config()->checkVersion);

    // show convergence chart
    chkShowConvergenceChart->setChecked(Util::config()->showConvergenceChart);

    // logs
    chkEnabledApplicationLog->setChecked(Util::config()->enabledApplicationLog);
    chkEnabledProgressLog->setChecked(Util::config()->enabledProgressLog);

    // show result in line edit value widget
    chkLineEditValueShowResult->setChecked(Util::config()->lineEditValueShowResult);

    // geometry
    txtMeshAngleSegmentsCount->setValue(Util::config()->angleSegmentsCount);
    chkMeshCurvilinearElements->setChecked(Util::config()->curvilinearElements);

    // delete files
    chkDeleteTriangleMeshFiles->setChecked(Util::config()->deleteTriangleMeshFiles);
    chkDeleteHermes2DMeshFile->setChecked(Util::config()->deleteHermes2DMeshFile);

    // colors
    colorBackground->setColor(Util::config()->colorBackground);
    colorGrid->setColor(Util::config()->colorGrid);
    colorCross->setColor(Util::config()->colorCross);
    colorNodes->setColor(Util::config()->colorNodes);
    colorEdges->setColor(Util::config()->colorEdges);
    colorLabels->setColor(Util::config()->colorLabels);
    colorContours->setColor(Util::config()->colorContours);
    colorVectors->setColor(Util::config()->colorVectors);
    colorInitialMesh->setColor(Util::config()->colorInitialMesh);
    colorSolutionMesh->setColor(Util::config()->colorSolutionMesh);
    colorHighlighted->setColor(Util::config()->colorHighlighted);
    colorSelected->setColor(Util::config()->colorSelected);

    // adaptivity
    txtMaxDOFs->setValue(Util::config()->maxDofs);
    //chkIsoOnly->setChecked(Util::config()->isoOnly);
    txtConvExp->setValue(Util::config()->convExp);
    txtThreshold->setValue(Util::config()->threshold);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(Util::config()->strategy));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(Util::config()->meshRegularity));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(Util::config()->projNormType));

    // command argument
    txtArgumentTriangle->setText(Util::config()->commandTriangle);
    txtArgumentFFmpeg->setText(Util::config()->commandFFmpeg);

    // global script
    txtGlobalScript->setPlainText(Util::config()->globalScript);
}

void ConfigDialog::save()
{
    logMessage("ConfigDialog::save()");

    // gui style
    Util::config()->guiStyle = cmbGUIStyle->currentText();
    setGUIStyle(cmbGUIStyle->currentText());

    // language
    if (Util::config()->language != cmbLanguage->currentText())
        QMessageBox::warning(QApplication::activeWindow(),
                                 tr("Language change"),
                                 tr("Interface language has been changed. You must restart the application."));
    Util::config()->language = cmbLanguage->currentText();

    // default physic field
    Util::config()->defaultPhysicField = (PhysicField) cmbDefaultPhysicField->itemData(cmbDefaultPhysicField->currentIndex()).toInt();

    // collaboration server
    QString collaborationServerUrl = txtCollaborationServerURL->text();
    if (!collaborationServerUrl.startsWith("http://"))
        collaborationServerUrl = QString("http://%1").arg(collaborationServerUrl);

    if (!collaborationServerUrl.endsWith("/"))
        collaborationServerUrl = QString("%1/").arg(collaborationServerUrl);

    Util::config()->collaborationServerURL = collaborationServerUrl;

    // check version
    Util::config()->checkVersion = chkCheckVersion->isChecked();

    // show convergence chart
    Util::config()->showConvergenceChart = chkShowConvergenceChart->isChecked();

    // logs
    Util::config()->enabledApplicationLog = chkEnabledApplicationLog->isChecked();
    Util::config()->enabledProgressLog = chkEnabledProgressLog->isChecked();

    // show result in line edit value widget
    Util::config()->lineEditValueShowResult = chkLineEditValueShowResult->isChecked();

    // mesh
    Util::config()->angleSegmentsCount = txtMeshAngleSegmentsCount->value();
    Util::config()->curvilinearElements = chkMeshCurvilinearElements->isChecked();

    // delete files
    Util::config()->deleteTriangleMeshFiles = chkDeleteTriangleMeshFiles->isChecked();
    Util::config()->deleteHermes2DMeshFile = chkDeleteHermes2DMeshFile->isChecked();

    // color
    Util::config()->colorBackground = colorBackground->color();
    Util::config()->colorGrid = colorGrid->color();
    Util::config()->colorCross = colorCross->color();
    Util::config()->colorNodes = colorNodes->color();
    Util::config()->colorEdges = colorEdges->color();
    Util::config()->colorLabels = colorLabels->color();
    Util::config()->colorContours = colorContours->color();
    Util::config()->colorVectors = colorVectors->color();
    Util::config()->colorInitialMesh = colorInitialMesh->color();
    Util::config()->colorSolutionMesh = colorSolutionMesh->color();
    Util::config()->colorHighlighted = colorHighlighted->color();
    Util::config()->colorSelected = colorSelected->color();

    // adaptivity
    Util::config()->maxDofs = txtMaxDOFs->value();
    //Util::config()->isoOnly = chkIsoOnly->isChecked();
    Util::config()->convExp = txtConvExp->value();
    Util::config()->threshold = txtThreshold->value();
    Util::config()->strategy = cmbStrategy->itemData(cmbStrategy->currentIndex()).toInt();
    Util::config()->meshRegularity = cmbMeshRegularity->itemData(cmbMeshRegularity->currentIndex()).toInt();
    Util::config()->projNormType = (ProjNormType) cmbProjNormType->itemData(cmbProjNormType->currentIndex()).toInt();

    // command argument
    Util::config()->commandTriangle = txtArgumentTriangle->text();
    Util::config()->commandFFmpeg = txtArgumentFFmpeg->text();

    // global script
    Util::config()->globalScript = txtGlobalScript->toPlainText();

    // save
    Util::config()->save();
}

void ConfigDialog::createControls()
{
    logMessage("ConfigDialog::createControls()");

    lstView = new QListWidget(this);
    pages = new QStackedWidget(this);

    panMain = createMainWidget();
    panSolver = createSolverWidget();
    panColors = createColorsWidget();
    panGlobalScriptWidget = createGlobalScriptWidget();

    // List View
    lstView->setCurrentRow(0);
    lstView->setViewMode(QListView::IconMode);
    lstView->setResizeMode(QListView::Adjust);
    lstView->setMovement(QListView::Static);
    lstView->setFlow(QListView::TopToBottom);
    lstView->setIconSize(QSize(60, 60));
    lstView->setMinimumWidth(135);
    lstView->setMaximumWidth(135);
    lstView->setMinimumHeight((45+fontMetrics().height()*4)*5);
    connect(lstView, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
               this, SLOT(doCurrentItemChanged(QListWidgetItem *, QListWidgetItem *)));

    QSize sizeItem(131, 85);

    // listView items
    QListWidgetItem *itemMain = new QListWidgetItem(icon("options-main"), tr("Main"), lstView);
    itemMain->setTextAlignment(Qt::AlignHCenter);
    itemMain->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemMain->setSizeHint(sizeItem);

    QListWidgetItem *itemSolver = new QListWidgetItem(icon("options-solver"), tr("Solver"), lstView);
    itemSolver->setTextAlignment(Qt::AlignHCenter);
    itemSolver->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemSolver->setSizeHint(sizeItem);

    QListWidgetItem *itemColors = new QListWidgetItem(icon("options-colors"), tr("Colors"), lstView);
    itemColors->setTextAlignment(Qt::AlignHCenter);
    itemColors->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemColors->setSizeHint(sizeItem);

    QListWidgetItem *itemGlobalScript = new QListWidgetItem(icon("options-python"), tr("Python"), lstView);
    itemGlobalScript->setTextAlignment(Qt::AlignHCenter);
    itemGlobalScript->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    itemGlobalScript->setSizeHint(sizeItem);

    pages->addWidget(panMain);
    pages->addWidget(panSolver);
    pages->addWidget(panColors);
    pages->addWidget(panGlobalScriptWidget);

    QHBoxLayout *layoutHorizontal = new QHBoxLayout();
    layoutHorizontal->addWidget(lstView);
    layoutHorizontal->addWidget(pages);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutHorizontal);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

QWidget *ConfigDialog::createMainWidget()
{
    logMessage("ConfigDialog::createMainWidget()");

    QWidget *mainWidget = new QWidget(this);

    // general
    cmbGUIStyle = new QComboBox(mainWidget);
    cmbGUIStyle->addItems(QStyleFactory::keys());
    cmbGUIStyle->addItem("Manhattan");

    cmbLanguage = new QComboBox(mainWidget);
    cmbLanguage->addItems(availableLanguages());

    cmbDefaultPhysicField = new QComboBox();
    fillComboBoxPhysicField(cmbDefaultPhysicField);

    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->addWidget(new QLabel(tr("UI:")), 0, 0);
    layoutGeneral->addWidget(cmbGUIStyle, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Language:")), 1, 0);
    layoutGeneral->addWidget(cmbLanguage, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Default physic field:")), 2, 0);
    layoutGeneral->addWidget(cmbDefaultPhysicField, 2, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // collaboration
    txtCollaborationServerURL = new SLineEditDouble();

    QVBoxLayout *layoutCollaboration = new QVBoxLayout();
    layoutCollaboration->addWidget(new QLabel(tr("Collaboration server URL:")));
    layoutCollaboration->addWidget(txtCollaborationServerURL);

    QGroupBox *grpCollaboration = new QGroupBox(tr("Collaboration"));
    grpCollaboration->setLayout(layoutCollaboration);

    // logs
    chkEnabledApplicationLog = new QCheckBox(tr("Enabled application log"));
    chkEnabledProgressLog = new QCheckBox(tr("Enabled progress log"));

    cmdClearApplicationLog = new QPushButton(mainWidget);
    cmdClearApplicationLog->setText(tr("Clear application log"));
    connect(cmdClearApplicationLog, SIGNAL(clicked()), this, SLOT(doClearApplicationLog()));

    QGridLayout *layoutClearButtons = new QGridLayout();
    layoutClearButtons->addWidget(cmdClearApplicationLog, 0, 0);

    QVBoxLayout *layoutLogs = new QVBoxLayout();
    layoutLogs->addWidget(chkEnabledProgressLog);
    layoutLogs->addWidget(chkEnabledApplicationLog);
    layoutLogs->addLayout(layoutClearButtons);

    QGroupBox *grpLogs = new QGroupBox(tr("Logs"));
    grpLogs->setLayout(layoutLogs);

    // other
    chkLineEditValueShowResult = new QCheckBox(tr("Show value result in line edit input"));
    chkCheckVersion = new QCheckBox(tr("Check new version during startup."));
    chkExperimentalFeatures = new QCheckBox(tr("Enable experimental features"));
    chkExperimentalFeatures->setToolTip(tr("Warning: Agros2D should be unstable!"));

    QVBoxLayout *layoutOther = new QVBoxLayout();
    layoutOther->addWidget(chkLineEditValueShowResult);
    layoutOther->addWidget(chkCheckVersion);
    layoutOther->addWidget(chkExperimentalFeatures);

    QGroupBox *grpOther = new QGroupBox(tr("Other"));
    grpOther->setLayout(layoutOther);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpGeneral);
    layout->addWidget(grpCollaboration);
    layout->addWidget(grpLogs);
    layout->addWidget(grpOther);
    layout->addStretch();

    mainWidget->setLayout(layout);

    return mainWidget;
}

QWidget *ConfigDialog::createSolverWidget()
{
    logMessage("ConfigDialog::createSolverWidget()");

    // general
    chkDeleteTriangleMeshFiles = new QCheckBox(tr("Delete files with initial mesh (Triangle)"));
    chkDeleteHermes2DMeshFile = new QCheckBox(tr("Delete files with solution mesh (Hermes2D)"));
    chkShowConvergenceChart = new QCheckBox(tr("Show convergence chart after solving"));

    QVBoxLayout *layoutSolver = new QVBoxLayout();
    layoutSolver->addWidget(chkDeleteTriangleMeshFiles);
    layoutSolver->addWidget(chkDeleteHermes2DMeshFile);
    layoutSolver->addWidget(chkShowConvergenceChart);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(layoutSolver);

    txtMeshAngleSegmentsCount = new QSpinBox(this);
    txtMeshAngleSegmentsCount->setMinimum(2);
    txtMeshAngleSegmentsCount->setMaximum(20);
    chkMeshCurvilinearElements = new QCheckBox(tr("Curvilinear elements"));

    QGridLayout *layoutMesh = new QGridLayout();
    layoutMesh->addWidget(new QLabel(tr("Angle segments count:")), 0, 0);
    layoutMesh->addWidget(txtMeshAngleSegmentsCount, 0, 1);
    layoutMesh->addWidget(chkMeshCurvilinearElements, 1, 0, 1, 2);

    QGroupBox *grpMesh = new QGroupBox(tr("Mesh"));
    grpMesh->setLayout(layoutMesh);

    QVBoxLayout *layoutGeneral = new QVBoxLayout();
    layoutGeneral->addWidget(grpSolver);
    layoutGeneral->addWidget(grpMesh);
    layoutGeneral->addStretch();

    QWidget *solverGeneralWidget = new QWidget(this);
    solverGeneralWidget->setLayout(layoutGeneral);

    // adaptivity
    lblMaxDofs = new QLabel(tr("Maximum number of DOFs:"));
    txtMaxDOFs = new QSpinBox(this);
    txtMaxDOFs->setMinimum(1e2);
    txtMaxDOFs->setMaximum(1e9);
    txtMaxDOFs->setSingleStep(1e2);
    /*
    chkIsoOnly = new QCheckBox(tr("Isotropic refinement"));
    lblIsoOnly = new QLabel(tr("<table>"
                               "<tr><td><b>true</b></td><td>isotropic refinement</td></tr>"
                               "<tr><td><b>false</b></td><td>anisotropic refinement</td></tr>"
                               "</table>"));
    */
    txtConvExp = new SLineEditDouble();
    lblConvExp = new QLabel(tr("<b></b>default value is 1.0, this parameter influences<br/>the selection of candidates in hp-adaptivity"));
    txtThreshold = new SLineEditDouble();
    lblThreshold = new QLabel(tr("<b></b>quantitative parameter of the adapt(...) function<br/>with different meanings for various adaptive strategies"));
    cmbStrategy = new QComboBox();
    cmbStrategy->addItem(tr("0"), 0);
    cmbStrategy->addItem(tr("1"), 1);
    cmbStrategy->addItem(tr("2"), 2);
    lblStrategy = new QLabel(tr("<table>"
                                 "<tr><td><b>0</b></td><td>refine elements until sqrt(<b>threshold</b>)<br/>times total error is processed.<br/>If more elements have similar errors,<br/>refine all to keep the mesh symmetric</td></tr>"
                                 "<tr><td><b>1</b></td><td>refine all elements<br/>whose error is larger than <b>threshold</b><br/>times maximum element error</td></tr>"
                                 "<tr><td><b>2</b></td><td>refine all elements<br/>whose error is larger than <b>threshold</b></td></tr>"
                                 "</table>"));
    cmbMeshRegularity = new QComboBox();
    cmbMeshRegularity->addItem(tr("arbitrary level hang. nodes"), -1);
    cmbMeshRegularity->addItem(tr("at most one-level hang. nodes"), 1);
    cmbMeshRegularity->addItem(tr("at most two-level hang. nodes"), 2);
    cmbMeshRegularity->addItem(tr("at most three-level hang. nodes"), 3);
    cmbMeshRegularity->addItem(tr("at most four-level hang. nodes"), 4);
    cmbMeshRegularity->addItem(tr("at most five-level hang. nodes"), 5);

    cmbProjNormType = new QComboBox();
    cmbProjNormType->addItem(errorNormString(HERMES_H1_NORM), HERMES_H1_NORM);
    cmbProjNormType->addItem(errorNormString(HERMES_L2_NORM), HERMES_L2_NORM);
    cmbProjNormType->addItem(errorNormString(HERMES_H1_SEMINORM), HERMES_H1_SEMINORM);

    // default
    QPushButton *btnAdaptivityDefault = new QPushButton(tr("Default"));
    connect(btnAdaptivityDefault, SIGNAL(clicked()), this, SLOT(doAdaptivityDefault()));

    QGridLayout *layoutAdaptivitySettings = new QGridLayout();
    layoutAdaptivitySettings->addWidget(lblMaxDofs, 0, 0);
    layoutAdaptivitySettings->addWidget(txtMaxDOFs, 0, 1, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Conv. exp.:")), 2, 0);
    layoutAdaptivitySettings->addWidget(txtConvExp, 2, 1);
    layoutAdaptivitySettings->addWidget(lblConvExp, 3, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Strategy:")), 4, 0);
    layoutAdaptivitySettings->addWidget(cmbStrategy, 4, 1);
    layoutAdaptivitySettings->addWidget(lblStrategy, 5, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Threshold:")), 6, 0);
    layoutAdaptivitySettings->addWidget(txtThreshold, 6, 1);
    layoutAdaptivitySettings->addWidget(lblThreshold, 7, 0, 1, 2);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Mesh regularity:")), 8, 0);
    layoutAdaptivitySettings->addWidget(cmbMeshRegularity, 8, 1);
    layoutAdaptivitySettings->addWidget(new QLabel(tr("Norm:")), 9, 0);
    layoutAdaptivitySettings->addWidget(cmbProjNormType, 9, 1);

    QVBoxLayout *layoutAdaptivity = new QVBoxLayout();
    layoutAdaptivity->addLayout(layoutAdaptivitySettings);
    layoutAdaptivity->addStretch();
    layoutAdaptivity->addWidget(btnAdaptivityDefault, 0, Qt::AlignLeft);

    QWidget *solverAdaptivityWidget = new QWidget(this);
    solverAdaptivityWidget->setLayout(layoutAdaptivity);

    // commands
    txtArgumentTriangle = new QLineEdit("");
    txtArgumentFFmpeg = new QLineEdit("");

    // default
    QPushButton *btnCommandsDefault = new QPushButton(tr("Default"));
    connect(btnCommandsDefault, SIGNAL(clicked()), this, SLOT(doCommandsDefault()));

    QVBoxLayout *layoutCommands = new QVBoxLayout();
    layoutCommands->addWidget(new QLabel(tr("Triangle")));
    layoutCommands->addWidget(txtArgumentTriangle);
    layoutCommands->addWidget(new QLabel(tr("FFmpeg")));
    layoutCommands->addWidget(txtArgumentFFmpeg);
    layoutCommands->addStretch();
    layoutCommands->addWidget(btnCommandsDefault, 0, Qt::AlignLeft);

    QWidget *solverCommandsWidget = new QWidget(this);
    solverCommandsWidget->setLayout(layoutCommands);

    QTabWidget *solverWidget = new QTabWidget(this);
    solverWidget->addTab(solverGeneralWidget, icon(""), tr("General"));
    solverWidget->addTab(solverAdaptivityWidget, icon(""), tr("Adaptivity"));
    solverWidget->addTab(solverCommandsWidget, icon(""), tr("Commands"));

    return solverWidget;
}

QWidget *ConfigDialog::createColorsWidget()
{
    logMessage("ConfigDialog::createColorsWidget()");

    QWidget *colorsWidget = new QWidget(this);

    // colors
    colorBackground = new ColorButton(this);
    colorGrid = new ColorButton(this);
    colorCross = new ColorButton(this);

    colorNodes = new ColorButton(this);
    colorEdges = new ColorButton(this);
    colorLabels = new ColorButton(this);
    colorContours = new ColorButton(this);
    colorVectors = new ColorButton(this);
    colorInitialMesh = new ColorButton(this);
    colorSolutionMesh = new ColorButton(this);

    colorHighlighted = new ColorButton(this);
    colorSelected = new ColorButton(this);

    QGridLayout *layoutColors = new QGridLayout();
    layoutColors->addWidget(new QLabel(tr("Background:")), 0, 0);
    layoutColors->addWidget(new QLabel(tr("Grid:")), 1, 0);
    layoutColors->addWidget(new QLabel(tr("Cross:")), 2, 0);
    layoutColors->addWidget(new QLabel(tr("Nodes:")), 3, 0);
    layoutColors->addWidget(new QLabel(tr("Edges:")), 4, 0);
    layoutColors->addWidget(new QLabel(tr("Labels:")), 5, 0);
    layoutColors->addWidget(new QLabel(tr("Contours:")), 6, 0);
    layoutColors->addWidget(new QLabel(tr("Vectors:")), 7, 0);
    layoutColors->addWidget(new QLabel(tr("Initial mesh:")), 8, 0);
    layoutColors->addWidget(new QLabel(tr("Solution mesh:")), 9, 0);
    layoutColors->addWidget(new QLabel(tr("Highlighted elements:")), 10, 0);
    layoutColors->addWidget(new QLabel(tr("Selected elements:")), 11, 0);

    layoutColors->addWidget(colorBackground, 0, 1);
    layoutColors->addWidget(colorGrid, 1, 1);
    layoutColors->addWidget(colorCross, 2, 1);
    layoutColors->addWidget(colorNodes, 3, 1);
    layoutColors->addWidget(colorEdges, 4, 1);
    layoutColors->addWidget(colorLabels, 5, 1);
    layoutColors->addWidget(colorContours, 6, 1);
    layoutColors->addWidget(colorVectors, 7, 1);
    layoutColors->addWidget(colorInitialMesh, 8, 1);
    layoutColors->addWidget(colorSolutionMesh, 9, 1);
    layoutColors->addWidget(colorHighlighted, 10, 1);
    layoutColors->addWidget(colorSelected, 11, 1);

    // default
    QPushButton *btnDefault = new QPushButton(tr("Default"));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(doColorsDefault()));

    QGroupBox *grpColor = new QGroupBox(tr("Colors"));
    grpColor->setLayout(layoutColors);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpColor);
    layout->addStretch();
    layout->addWidget(btnDefault, 0, Qt::AlignLeft);

    colorsWidget->setLayout(layout);

    return colorsWidget;
}

QWidget *ConfigDialog::createGlobalScriptWidget()
{
    logMessage("ConfigDialog::createGlobalScriptWidget()");

    QWidget *viewWidget = new QWidget(this);

    txtGlobalScript = new ScriptEditor(currentPythonEngine(), this);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(txtGlobalScript);

    viewWidget->setLayout(layout);

    return viewWidget;
}

void ConfigDialog::doCurrentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    logMessage("ConfigDialog::doCurrentItemChanged()");

    pages->setCurrentIndex(lstView->row(current));
}

void ConfigDialog::doAccept()
{
    logMessage("ConfigDialog::doAccept()");

    save();
    accept();
}

void ConfigDialog::doReject()
{
    logMessage("ConfigDialog::ConfigDialog()");

    reject();
}

void ConfigDialog::doClearApplicationLog()
{
    logMessage("ConfigDialog::doClearApplicationLog()");

    if (QMessageBox::question(this, tr("Delete"), tr("Are you sure that you want to permanently delete the application logfile?"), tr("&Yes"), tr("&No")) == 0)
    {
        QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
        QFile::remove(location + "/app.log");

        QMessageBox::information(QApplication::activeWindow(), tr("Information"), tr("Application log was cleared successfully."));
    }
}

void ConfigDialog::doAdaptivityDefault()
{
    logMessage("ConfigDialog::doAdaptivityDefault()");

    txtMaxDOFs->setValue(MAX_DOFS);
    //chkIsoOnly->setChecked(ADAPTIVITY_ISOONLY);
    txtConvExp->setValue(ADAPTIVITY_CONVEXP);
    txtThreshold->setValue(ADAPTIVITY_THRESHOLD);
    cmbStrategy->setCurrentIndex(cmbStrategy->findData(ADAPTIVITY_STRATEGY));
    cmbMeshRegularity->setCurrentIndex(cmbMeshRegularity->findData(ADAPTIVITY_MESHREGULARITY));
    cmbProjNormType->setCurrentIndex(cmbProjNormType->findData(ADAPTIVITY_PROJNORMTYPE));
}

void ConfigDialog::doCommandsDefault()
{
    logMessage("ConfigDialog::doCommandsDefault()");

    txtArgumentTriangle->setText(COMMANDS_TRIANGLE);
    txtArgumentFFmpeg->setText(COMMANDS_FFMPEG);
}

void ConfigDialog::doColorsDefault()
{
    logMessage("ConfigDialog::doColorsDefault()");

    colorBackground->setColor(COLORBACKGROUND);
    colorGrid->setColor(COLORGRID);
    colorCross->setColor(COLORCROSS);
    colorNodes->setColor(COLORNODES);
    colorEdges->setColor(COLOREDGES);
    colorLabels->setColor(COLORLABELS);
    colorContours->setColor(COLORCONTOURS);
    colorVectors->setColor(COLORVECTORS);
    colorInitialMesh->setColor(COLORINITIALMESH);
    colorSolutionMesh->setColor(COLORSOLUTIONMESH);
    colorHighlighted->setColor(COLORHIGHLIGHTED);
    colorSelected->setColor(COLORSELECTED);
}

// *******************************************************************************************************

ColorButton::ColorButton(QWidget *parent) : QPushButton(parent)
{
    logMessage("ColorButton::ColorButton()");

    setAutoFillBackground(false);
    setCursor(Qt::PointingHandCursor);
    connect(this, SIGNAL(clicked()), this, SLOT(doClicked()));
}

ColorButton::~ColorButton()
{
    logMessage("ColorButton::~ColorButton()");
}

void ColorButton::setColor(const QColor &color)
{
    logMessage("ColorButton::setColor()");

    m_color = color;
    repaint();
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    logMessage("ColorButton::paintEvent()");

    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setPen(m_color);
    painter.setBrush(m_color);
    painter.drawRect(rect());
}

void ColorButton::doClicked()
{
    logMessage("ColorButton::doClicked()");

    QColor color = QColorDialog::getColor(m_color);

    if (color.isValid())
    {
        setColor(color);
    }
}

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

#include "preprocessorview.h"

#include "util/constants.h"
#include "util/global.h"
#include "gui/parameterdialog.h"
#include "solver/problem_config.h"
#include "optilab/study.h"

#include "scene.h"
#include "logview.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "problemdialog.h"
#include "pythonlab/pythonengine_agros.h"
#include "solver/module.h"

#include "solver/problem.h"
#include "ctemplate/template.h"

PreprocessorWidget::PreprocessorWidget(SceneViewPreprocessor *sceneView, QWidget *parent): QWidget(parent)
{
    this->m_sceneViewPreprocessor = sceneView;

    setMinimumWidth(160);
    setObjectName("PreprocessorView");

    createActions();

    // context menu
    mnuPreprocessor = new QMenu(this);

    // boundary conditions, materials and geometry information
    createControls();

    connect(Agros2D::problem()->scene(), SIGNAL(cleared()), this, SLOT(refresh()));

    connect(Agros2D::problem()->scene(), SIGNAL(invalidated()), this, SLOT(refresh()));
    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(refresh()));

    connect(Agros2D::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));

    connect(trvWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));
    connect(trvWidget, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));

    doItemChanged(NULL, NULL);
}

PreprocessorWidget::~PreprocessorWidget()
{
    QSettings settings;
    settings.setValue("PreprocessorWidget/SplitterState", splitter->saveState());
    settings.setValue("PreprocessorWidget/SplitterGeometry", splitter->saveGeometry());
    settings.setValue("PreprocessorWidget/TreeColumnWidth", trvWidget->columnWidth(0));
}

void PreprocessorWidget::createActions()
{
    actProperties = new QAction(icon("scene-properties"), tr("&Properties"), this);
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(icon("scene-delete"), tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));

    actNewParameter = new QAction(icon(""), tr("New parameter"), this);
    connect(actNewParameter, SIGNAL(triggered()), this, SLOT(doNewParameter()));
}

void PreprocessorWidget::createMenu()
{
    mnuPreprocessor->clear();

    mnuPreprocessor->addAction(Agros2D::problem()->scene()->actNewNode);
    mnuPreprocessor->addAction(Agros2D::problem()->scene()->actNewEdge);
    mnuPreprocessor->addAction(Agros2D::problem()->scene()->actNewLabel);
    mnuPreprocessor->addSeparator();
    Agros2D::problem()->scene()->addBoundaryAndMaterialMenuItems(mnuPreprocessor, this);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actNewParameter);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actDelete);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actProperties);
}

void PreprocessorWidget::createControls()
{
    QSettings settings;

    // undo framework
    actUndo = Agros2D::problem()->scene()->undoStack()->createUndoAction(this);
    actUndo->setIcon(icon("edit-undo"));
    actUndo->setIconText(tr("&Undo"));
    actUndo->setShortcuts(QKeySequence::Undo);

    actRedo = Agros2D::problem()->scene()->undoStack()->createRedoAction(this);
    actRedo->setIcon(icon("edit-redo"));
    actRedo->setIconText(tr("&Redo"));
    actRedo->setShortcuts(QKeySequence::Redo);

    // main toolbar
    toolBar = new QToolBar();
    toolBar->addAction(actUndo);
    toolBar->addAction(actRedo);
    toolBar->addSeparator();
    toolBar->addAction(m_sceneViewPreprocessor->actOperateOnNodes);
    toolBar->addAction(m_sceneViewPreprocessor->actOperateOnEdges);
    toolBar->addAction(m_sceneViewPreprocessor->actOperateOnLabels);
    toolBar->addSeparator();
    toolBar->addAction(m_sceneViewPreprocessor->actSceneViewSelectRegion);
    toolBar->addAction(Agros2D::problem()->scene()->actTransform);
    toolBar->addSeparator();
    toolBar->addAction(Agros2D::problem()->scene()->actDeleteSelected);

    txtViewNodes = new QTextEdit(this);
    txtViewNodes->setReadOnly(true);
    txtViewNodes->setVisible(false);
    txtViewNodes->setText(tr("Tooltip_OperateOnNodes"));

    txtViewEdges = new QTextEdit(this);
    txtViewEdges->setReadOnly(true);
    txtViewEdges->setVisible(false);
    txtViewEdges->setText(tr("Tooltip_OperateOnEdges"));

    txtViewLabels = new QTextEdit(this);
    txtViewLabels->setReadOnly(true);
    txtViewLabels->setVisible(false);
    txtViewLabels->setText(tr("Tooltip_OperateOnLabels"));

    loadTooltip(SceneGeometryMode_OperateOnNodes);

    trvWidget = new QTreeWidget(this);
    trvWidget->setHeaderHidden(false);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setColumnCount(2);
    trvWidget->setColumnWidth(0, settings.value("PreprocessorWidget/TreeColumnWidth", 200).toInt());
    // trvWidget->setIndentation(15);

    QStringList headers;
    headers << tr("Name") << tr("Value");
    trvWidget->setHeaderLabels(headers);

    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));

    QPushButton *btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    QGridLayout *layoutTreeView = new QGridLayout();
    layoutTreeView->addWidget(trvWidget, 1, 0, 1, 4);
    layoutTreeView->addWidget(new QLabel(tr("Grid step:")), 2, 0);
    layoutTreeView->addWidget(txtGridStep, 2, 1);
    layoutTreeView->addWidget(chkSnapToGrid, 2, 2);
    layoutTreeView->addWidget(btnOK, 2, 3);

    QWidget *widgetTreeView = new QWidget();
    widgetTreeView->setLayout(layoutTreeView);

    QHBoxLayout *layoutView = new QHBoxLayout();
    layoutView->addWidget(txtViewNodes);
    layoutView->addWidget(txtViewEdges);
    layoutView->addWidget(txtViewLabels);

    QWidget *view = new QWidget();
    view->setLayout(layoutView);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(widgetTreeView);
    splitter->addWidget(view);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(toolBar);
    layoutMain->addWidget(splitter);

    setLayout(layoutMain);

    splitter->restoreState(settings.value("PreprocessorWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PreprocessorWidget/SplitterGeometry").toByteArray());
}

void PreprocessorWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        doDelete();
        return;
    }

    QWidget::keyPressEvent(event);
}

void PreprocessorWidget::refresh()
{
    txtGridStep->setText(QString::number(Agros2D::problem()->setting()->value(ProblemSetting::View_GridStep).toDouble()));
    chkSnapToGrid->setChecked(Agros2D::problem()->setting()->value(ProblemSetting::View_SnapToGrid).toBool());

    // script speed improvement
    if (currentPythonEngine()->isScriptRunning()) return;

    blockSignals(true);
    setUpdatesEnabled(false);

    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    QTreeWidgetItem *parametersNode = new QTreeWidgetItem(trvWidget);
    parametersNode->setText(0, tr("Parameters"));
    // labelsNode->setIcon(0, icon("scenelabel"));
    parametersNode->setFont(0, fnt);
    parametersNode->setExpanded(true);

    QList<QTreeWidgetItem *> listParameters;
    ParametersType parameters = Agros2D::problem()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
    foreach (QString key, parameters.keys())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, key);
        item->setText(1, QString::number(parameters[key]));
        item->setData(0, Qt::UserRole, key);
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryParameter);

        listParameters.append(item);
    }
    parametersNode->addChildren(listParameters);

    // markers
    foreach (FieldInfo *fieldInfo, Agros2D::problem()->fieldInfos())
    {
        // field
        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(trvWidget);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        fieldNode->setExpanded(true);

        // materials
        QTreeWidgetItem *materialsNode = new QTreeWidgetItem(fieldNode);
        materialsNode->setIcon(0, icon("scenelabelmarker"));
        materialsNode->setText(0, tr("Materials"));
        // materialsNode->setForeground(0, QBrush(Qt::darkBlue));
        materialsNode->setFont(0, fnt);
        materialsNode->setExpanded(true);

        foreach (SceneMaterial *material, Agros2D::problem()->scene()->materials->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(materialsNode);

            item->setText(0, material->name());
            item->setIcon(0, (Agros2D::problem()->scene()->labels->haveMarker(material).count() > 0) ? icon("scene-labelmarker") : icon("scene-labelmarker-notused"));
            if (Agros2D::problem()->scene()->labels->haveMarker(material).isEmpty())
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, material->variant());
            item->setData(1, Qt::UserRole, PreprocessorWidget::Material);
        }

        // boundary conditions
        QTreeWidgetItem *boundaryConditionsNode = new QTreeWidgetItem(fieldNode);
        boundaryConditionsNode->setIcon(0, icon("sceneedgemarker"));
        boundaryConditionsNode->setText(0, tr("Boundary conditions"));
        // boundaryConditionsNode->setForeground(0, QBrush(Qt::darkBlue));
        boundaryConditionsNode->setFont(0, fnt);
        boundaryConditionsNode->setExpanded(true);

        foreach (SceneBoundary *boundary, Agros2D::problem()->scene()->boundaries->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

            item->setText(0, boundary->name());
            item->setText(1, boundaryType.name());
            item->setIcon(0, (Agros2D::problem()->scene()->edges->haveMarker(boundary).count() > 0) ? icon("scene-edgemarker") : icon("scene-edgemarker-notused"));
            if (Agros2D::problem()->scene()->edges->haveMarker(boundary).isEmpty())
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, boundary->variant());
            item->setData(1, Qt::UserRole, PreprocessorWidget::Boundary);
        }
    }

    // geometry
    QTreeWidgetItem *geometryNode = new QTreeWidgetItem(trvWidget);
    // geometryNode->setIcon(0, icon("geometry"));
    geometryNode->setText(0, tr("Geometry"));
    geometryNode->setFont(0, fnt);
    geometryNode->setExpanded(false);

    // nodes
    QTreeWidgetItem *nodesNode = new QTreeWidgetItem(geometryNode);
    nodesNode->setText(0, tr("Nodes"));
    nodesNode->setIcon(0, icon("scenenode"));
    // nodesNode->setForeground(0, QBrush(Qt::darkBlue));
    nodesNode->setFont(0, fnt);

    int inode = 0;
    foreach (SceneNode *node, Agros2D::problem()->scene()->nodes->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(nodesNode);

        item->setText(0, QString("%1").
                      arg(inode));
        item->setText(1, QString("[%1; %2]").
                      arg(node->point().x, 0, 'e', 2).
                      arg(node->point().y, 0, 'e', 2));
        item->setIcon(0, icon("scene-node"));
        item->setData(0, Qt::UserRole, node->variant());
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryNode);

        inode++;
    }

    // edges
    QTreeWidgetItem *edgesNode = new QTreeWidgetItem(geometryNode);
    edgesNode->setText(0, tr("Edges"));
    edgesNode->setIcon(0, icon("sceneedge"));
    // edgesNode->setForeground(0, QBrush(Qt::darkBlue));
    edgesNode->setFont(0, fnt);

    int iedge = 0;
    foreach (SceneEdge *edge, Agros2D::problem()->scene()->edges->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(edgesNode);

        item->setText(0, QString("%1").
                      arg(iedge));
        item->setText(1, QString("%1 m").
                      arg((edge->angle() < EPS_ZERO) ?
                              (edge->nodeEnd()->point() - edge->nodeStart()->point()).magnitude() :
                              edge->radius() * edge->angle() / 180.0 * M_PI, 0, 'e', 2));
        item->setIcon(0, icon("scene-edge"));
        item->setData(0, Qt::UserRole, edge->variant());
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryEdge);

        iedge++;
    }

    // labels
    QTreeWidgetItem *labelsNode = new QTreeWidgetItem(geometryNode);
    labelsNode->setText(0, tr("Labels"));
    labelsNode->setIcon(0, icon("scenelabel"));
    // labelsNode->setForeground(0, QBrush(Qt::darkBlue));
    labelsNode->setFont(0, fnt);

    int ilabel = 0;
    foreach (SceneLabel *label, Agros2D::problem()->scene()->labels->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(labelsNode);

        item->setText(0, QString("%1").
                      arg(ilabel));
        item->setText(1, QString("[%1; %2]").
                      arg(label->point().x, 0, 'e', 2).
                      arg(label->point().y, 0, 'e', 2));
        item->setIcon(0, icon("scene-label"));
        item->setData(0, Qt::UserRole, label->variant());
        item->setData(1, Qt::UserRole, GeometryLabel);

        ilabel++;
    }

    // parameters
    QTreeWidgetItem *optilabNode = new QTreeWidgetItem(trvWidget);
    optilabNode->setText(0, tr("OptiLab"));
    optilabNode->setFont(0, fnt);
    optilabNode->setExpanded(true);

    foreach (Study *study, Agros2D::problem()->studies()->items())
    {
        // study
        QTreeWidgetItem *studyNode = new QTreeWidgetItem(optilabNode);
        studyNode->setText(0, studyTypeString(study->type()));
        studyNode->setFont(0, fnt);
        studyNode->setData(0, Qt::UserRole, study->variant());
        studyNode->setData(1, Qt::UserRole, PreprocessorWidget::OptilabStudy);
        studyNode->setExpanded(true);

        // parameters
        QTreeWidgetItem *parametersNode = new QTreeWidgetItem(studyNode);
        parametersNode->setText(0, tr("Parameters"));
        parametersNode->setFont(0, fnt);
        parametersNode->setExpanded(true);

        foreach (Parameter parameter, study->parameters())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(parametersNode);

            item->setText(0, QString("%1").arg(parameter.name()));
            item->setData(0, Qt::UserRole, parameter.name());
            item->setData(1, Qt::UserRole, PreprocessorWidget::OptilabParameter);
        }

        // functionals
        QTreeWidgetItem *functionalsNode = new QTreeWidgetItem(studyNode);
        functionalsNode->setText(0, tr("Functionals"));
        functionalsNode->setFont(0, fnt);
        functionalsNode->setExpanded(true);

        foreach (Functional functional, study->functionals())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(functionalsNode);

            item->setText(0, QString("%1").arg(functional.name()));
            item->setData(0, Qt::UserRole, functional.name());
            item->setData(1, Qt::UserRole, PreprocessorWidget::OptilabFunctional);
        }
    }

    setUpdatesEnabled(true);
    blockSignals(false);
}

void PreprocessorWidget::loadTooltip(SceneGeometryMode sceneMode)
{
    txtViewNodes->setVisible(sceneMode == SceneGeometryMode_OperateOnNodes);
    txtViewEdges->setVisible(sceneMode == SceneGeometryMode_OperateOnEdges);
    txtViewLabels->setVisible(sceneMode == SceneGeometryMode_OperateOnLabels);
}

void PreprocessorWidget::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *current = trvWidget->itemAt(pos);
    doItemChanged(current, NULL);

    if (current)
        trvWidget->setCurrentItem(current);

    mnuPreprocessor->exec(QCursor::pos());
}

void PreprocessorWidget::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    createMenu();

    actProperties->setEnabled(false);
    actDelete->setEnabled(false);

    Agros2D::problem()->scene()->selectNone();
    Agros2D::problem()->scene()->highlightNone();

    if (current)
    {
        PreprocessorWidget::Type type = (PreprocessorWidget::Type) trvWidget->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == PreprocessorWidget::GeometryNode || type == PreprocessorWidget::GeometryEdge || type == PreprocessorWidget::GeometryLabel)
        {
            // geometry
            SceneBasic *objectBasic = current->data(0, Qt::UserRole).value<SceneBasic *>();
            m_sceneViewPreprocessor->actSceneModePreprocessor->trigger();

            if (dynamic_cast<SceneNode *>(objectBasic))
                m_sceneViewPreprocessor->actOperateOnNodes->trigger();
            if (dynamic_cast<SceneEdge *>(objectBasic))
                m_sceneViewPreprocessor->actOperateOnEdges->trigger();
            if (dynamic_cast<SceneLabel *>(objectBasic))
                m_sceneViewPreprocessor->actOperateOnLabels->trigger();

            objectBasic->setSelected(true);

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::Boundary)
        {
            // edge marker
            // select all edges
            SceneBoundary *objectBoundary = current->data(0, Qt::UserRole).value<SceneBoundary *>();
            m_sceneViewPreprocessor->actOperateOnEdges->trigger();

            Agros2D::problem()->scene()->edges->haveMarker(objectBoundary).setSelected();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::Material)
        {
            // label marker
            // select all labels
            SceneMaterial *objectMaterial = current->data(0, Qt::UserRole).value<SceneMaterial *>();
            m_sceneViewPreprocessor->actOperateOnLabels->trigger();

            Agros2D::problem()->scene()->labels->haveMarker(objectMaterial).setSelected();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::GeometryParameter)
        {
            // parameter
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        m_sceneViewPreprocessor->refresh();
    }
}

void PreprocessorWidget::doItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    doProperties();
}

void PreprocessorWidget::doProperties()
{
    if (trvWidget->currentItem())
    {
        PreprocessorWidget::Type type = (PreprocessorWidget::Type) trvWidget->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == PreprocessorWidget::GeometryNode || type == PreprocessorWidget::GeometryEdge || type == PreprocessorWidget::GeometryLabel)
        {
            // geometry
            SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic *>();
            if (objectBasic->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewPreprocessor->refresh();
                refresh();
            }
        }
        else if (type == PreprocessorWidget::Boundary)
        {
            // edge marker
            SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>();

            if (objectBoundary->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewPreprocessor->refresh();
                refresh();
            }
        }
        else if (type == PreprocessorWidget::Material)
        {
            // label marker
            SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>();
            if (objectMaterial->showDialog(this) == QDialog::Accepted)
            {
                m_sceneViewPreprocessor->refresh();
                refresh();
            }
        }
        else if (type == PreprocessorWidget::GeometryParameter)
        {
            // parameter
            QString key = trvWidget->currentItem()->data(0, Qt::UserRole).toString();

            ParameterDialog dialog(key, this);
            if (dialog.exec() == QDialog::Accepted)
            {
                m_sceneViewPreprocessor->refresh();
                refresh();
            }
        }
    }
}

void PreprocessorWidget::doDelete()
{
    if (trvWidget->currentItem())
    {
        PreprocessorWidget::Type type = (PreprocessorWidget::Type) trvWidget->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == PreprocessorWidget::GeometryNode || type == PreprocessorWidget::GeometryEdge || type == PreprocessorWidget::GeometryLabel)
        {
            // scene objects
            SceneBasic *objectBasic = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBasic*>();

            if (SceneNode *node = dynamic_cast<SceneNode *>(objectBasic))
            {
                Agros2D::problem()->scene()->nodes->remove(node);
            }

            else if (SceneEdge *edge = dynamic_cast<SceneEdge *>(objectBasic))
            {
                Agros2D::problem()->scene()->edges->remove(edge);
            }

            else if (SceneLabel *label = dynamic_cast<SceneLabel *>(objectBasic))
            {
                Agros2D::problem()->scene()->labels->remove(label);
            }
        }
        else if (type == PreprocessorWidget::Material)
        {
            // label marker
            SceneMaterial *objectMaterial = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneMaterial *>();
            Agros2D::problem()->scene()->removeMaterial(objectMaterial);
        }
        else if (type == PreprocessorWidget::Boundary)
        {
            // edge marker
            SceneBoundary *objectBoundary = trvWidget->currentItem()->data(0, Qt::UserRole).value<SceneBoundary *>();
            Agros2D::problem()->scene()->removeBoundary(objectBoundary);
        }
        else if (type == PreprocessorWidget::GeometryParameter)
        {
            // parameter
            QString key = trvWidget->currentItem()->data(0, Qt::UserRole).toString();
            ParametersType parameters = Agros2D::problem()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
            parameters.remove(key);

            Agros2D::problem()->checkAndApplyParameters(parameters);
        }

        refresh();
        m_sceneViewPreprocessor->refresh();
    }
}

void PreprocessorWidget::doNewParameter()
{
    ParameterDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        m_sceneViewPreprocessor->refresh();
        refresh();
    }
}

void PreprocessorWidget::doApply()
{
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_GridStep, txtGridStep->text().toDouble());
    Agros2D::problem()->setting()->setValue(ProblemSetting::View_SnapToGrid, chkSnapToGrid->isChecked());
}

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
#include "util/point.h"

#include "gui/parameterdialog.h"
#include "gui/functiondialog.h"
#include "gui/problemdialog.h"
#include "gui/fielddialog.h"
#include "gui/other.h"
#include "app/scenegeometrydialog.h"

#include "solver/problem_config.h"
#include "solver/problem_parameter.h"
#include "solver/problem_function.h"

#include "scene.h"
#include "logview.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "sceneview_geometry.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "solver/module.h"

#include "solver/problem.h"

PreprocessorWidget::PreprocessorWidget(QWidget *parent): QWidget(parent)
{
    m_sceneViewProblem = new SceneViewProblem(this);

    setObjectName("PreprocessorView");

    createActions();

    // context menu
    mnuPreprocessor = new QMenu(this);
    mnuFields = new QMenu(tr("New field"), this);
    mnuMaterials = new QMenu(tr("New materials"), this);
    mnuBoundaries = new QMenu(tr("New boundaries"), this);
    connect(this, SIGNAL(changed()), m_sceneViewProblem, SLOT(refresh()));

    // boundary conditions, materials and geometry information
    createControls();

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

    // clear fields
    foreach (QAction *action, actNewFields.values())
        delete action;
    actNewFields.clear();

    // clear actions
    foreach (QAction *action, actNewBoundaries.values())
        delete action;
    actNewBoundaries.clear();

    foreach (QAction *action, actNewMaterials.values())
        delete action;
    actNewMaterials.clear();
}

void PreprocessorWidget::createActions()
{
    actProperties = new QAction(tr("&Properties"), this);
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doProperties()));

    actDelete = new QAction(tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doDelete()));

    actNewParameter = new QAction(iconAlphabet('P', AlphabetColor_Green), tr("New parameter"), this);
    connect(actNewParameter, SIGNAL(triggered()), this, SLOT(doNewParameter()));

    actNewFunctionAnalytic = new QAction(tr("New analytic function..."), this);
    connect(actNewFunctionAnalytic, SIGNAL(triggered()), this, SLOT(doNewFunctionAnalytic()));
    // actNewFunctionInterpolation = new QAction(tr("New interpolation function..."), this);
    // connect(actNewFunctionInterpolation, SIGNAL(triggered()), this, SLOT(doNewFunctionInterpolation()));

    // scene - add items
    actNewRectangle = new QAction(icon("geometry_rectangle"), tr("New &rectangle..."), this);
    actNewRectangle->setIconVisibleInMenu(true);
    connect(actNewRectangle, SIGNAL(triggered()), this, SLOT(doNewRectangle()));

    actNewCircle = new QAction(icon("geometry_circle"), tr("New &circle..."), this);
    actNewCircle->setIconVisibleInMenu(true);
    connect(actNewCircle, SIGNAL(triggered()), this, SLOT(doNewCircle()));

    actNewNode = new QAction(icon("geometry_node"), tr("New &node..."), this);
    actNewNode->setShortcut(QKeySequence("Alt+N"));
    actNewNode->setIconVisibleInMenu(true);
    connect(actNewNode, SIGNAL(triggered()), this, SLOT(doNewNode()));

    actNewEdge = new QAction(icon("geometry_edge"), tr("New &edge..."), this);
    actNewEdge->setShortcut(QKeySequence("Alt+E"));
    actNewEdge->setIconVisibleInMenu(true);
    connect(actNewEdge, SIGNAL(triggered()), this, SLOT(doNewEdge()));

    actNewLabel = new QAction(icon("geometry_label"), tr("New &label..."), this);
    actNewLabel->setShortcut(QKeySequence("Alt+L"));
    actNewLabel->setIconVisibleInMenu(true);
    connect(actNewLabel, SIGNAL(triggered()), this, SLOT(doNewLabel()));

    actExportGeometryToClipboard = new QAction(tr("Copy geometry to clipboard"), this);
    connect(actExportGeometryToClipboard, SIGNAL(triggered()), this, SLOT(exportGeometryToClipboard()));

    actExportGeometryToBitmap = new QAction(tr("Export geometry to PNG..."), this);
    connect(actExportGeometryToBitmap, SIGNAL(triggered()), this, SLOT(exportGeometryToBitmap()));

    actExportGeometryToSvg = new QAction(tr("Export geometry to SVG..."), this);
    connect(actExportGeometryToSvg, SIGNAL(triggered()), this, SLOT(exportGeometryToSvg()));

    actExportGeometryToVTK = new QAction(tr("Export geometry to VTK..."), this);
    connect(actExportGeometryToVTK, SIGNAL(triggered()), this, SLOT(exportGeometryToVTK()));

    actImportGeometryFromDXF = new QAction(tr("Import geometry from DXF..."), this);
    connect(actImportGeometryFromDXF, SIGNAL(triggered()), this, SLOT(importGeometryFromDXF()));

    actExportGeometryToDXF = new QAction(tr("Export geometry to DXF..."), this);
    connect(actExportGeometryToDXF, SIGNAL(triggered()), this, SLOT(exportGeometryToDXF()));


    // add to menu
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewRectangle);
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewCircle);
    m_sceneViewProblem->menuScene()->addSeparator();
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewNode);
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewEdge);
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewLabel);

    connect(m_sceneViewProblem, SIGNAL(sceneGeometryChanged()), this, SLOT(refresh()));

    // actNewBoundary->setShortcut(QKeySequence("Alt+B"));
    // actNewMaterial->setShortcut(QKeySequence("Alt+M"));

    // clear fields
    foreach (StringAction *action, actNewFields.values())
        delete action;
    actNewFields.clear();

    // clear boundaries
    foreach (StringAction *action, actNewBoundaries.values())
        delete action;
    actNewBoundaries.clear();

    // clear materials
    foreach (StringAction *action, actNewMaterials.values())
        delete action;
    actNewMaterials.clear();

    QMapIterator<QString, QString> it(Module::availableModules());
    while (it.hasNext())
    {
        it.next();

        auto *actionField = new StringAction(this, it.key(), it.value());
        actionField->setIcon(icon("fields/" + it.key()));
        actionField->setIconVisibleInMenu(true);
        connect(actionField, SIGNAL(triggered(QString)), this, SLOT(doNewField(QString)));
        actNewFields[it.key()] = actionField;

        auto *actionBoundary = new StringAction(this, it.key(), it.value());
        actionBoundary->setIcon(icon("fields/" + it.key()));
        actionBoundary->setIconVisibleInMenu(true);
        connect(actionBoundary, SIGNAL(triggered(QString)), this, SLOT(doNewBoundary(QString)));
        actNewBoundaries[it.key()] = actionBoundary;

        auto *actionMaterial = new StringAction(this, it.key(), it.value());
        actionMaterial->setIcon(icon("fields/" + it.key()));
        actionMaterial->setIconVisibleInMenu(true);
        connect(actionMaterial, SIGNAL(triggered(QString)), this, SLOT(doNewMaterial(QString)));
        actNewMaterials[it.key()] = actionMaterial;
    }
}

void PreprocessorWidget::createMenu()
{
    mnuPreprocessor->clear();

    mnuPreprocessor->addMenu(mnuFields);
    mnuPreprocessor->addAction(actNewParameter);
    mnuPreprocessor->addSeparator();       
    mnuPreprocessor->addMenu(mnuMaterials);
    mnuPreprocessor->addMenu(mnuBoundaries);
    mnuPreprocessor->addSeparator();

    auto *mnuFunctions = new QMenu(tr("New function"));
    mnuFunctions->addAction(actNewFunctionAnalytic);
    // mnuFunctions->addAction(actNewFunctionInterpolation);
    mnuPreprocessor->addMenu(mnuFunctions);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actDelete);
    mnuPreprocessor->addAction(actProperties);

    // enable
    bool enabled = !Agros::problem()->fieldInfos().isEmpty();
    // bool onlyOneField = (Agros::problem()->fieldInfos().size() == 1);

    mnuMaterials->setEnabled(enabled);
    mnuBoundaries->setEnabled(enabled);

    // toolbar
    toolButtonMaterials->setEnabled(enabled);
    toolButtonBoundaries->setEnabled(enabled);
}

void PreprocessorWidget::createControls()
{
    QSettings settings;

    // undo framework
    actUndo = undoStack()->createUndoAction(this);
    actUndo->setIcon(icon("undo"));
    actUndo->setIconText(tr("&Undo"));
    actUndo->setShortcuts(QKeySequence::Undo);

    actRedo = undoStack()->createRedoAction(this);
    actRedo->setIcon(icon("redo"));
    actRedo->setIconText(tr("&Redo"));
    actRedo->setShortcuts(QKeySequence::Redo);

    // add geometry
    auto *addButton = new QToolButton();
    addButton->setText(tr("Add geometry"));
    addButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addButton->setIconSize(QSize(24, 24));
    addButton->addAction(actNewNode);
    addButton->addAction(actNewEdge);
    addButton->addAction(actNewLabel);
    addButton->addAction(actNewRectangle);
    addButton->addAction(actNewCircle);
    addButton->setAutoRaise(true);
    addButton->setIcon(icon("geometry_add"));
    addButton->setPopupMode(QToolButton::InstantPopup);

    // zoom
    auto *mnuZoom = new QMenu(this);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomBestFit);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomIn);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomOut);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomRegion);

    // export
    auto *mnuExport = new QMenu(this);
    mnuExport->addAction(actExportGeometryToClipboard);
    mnuExport->addSeparator();
    mnuExport->addAction(actExportGeometryToBitmap);
    mnuExport->addAction(actExportGeometryToSvg);
    mnuExport->addAction(actExportGeometryToVTK);
    mnuExport->addSeparator();
    mnuExport->addAction(actImportGeometryFromDXF);
    mnuExport->addAction(actExportGeometryToDXF);

    auto *exportButton = new QToolButton();
    exportButton->setText(tr("Import/Export"));
    exportButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    exportButton->setIconSize(QSize(24, 24));
    exportButton->setMenu(mnuExport);
    exportButton->setAutoRaise(true);
    exportButton->setIcon(icon("export"));
    exportButton->setPopupMode(QToolButton::InstantPopup);

    auto *zoomButton = new QToolButton();
    zoomButton->setText(tr("Zoom"));
    zoomButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    zoomButton->setIconSize(QSize(24, 24));
    zoomButton->setMenu(mnuZoom);
    zoomButton->setAutoRaise(true);
    zoomButton->setIcon(icon("geometry_zoom"));
    zoomButton->setPopupMode(QToolButton::InstantPopup);

    // right toolbar
    toolBarRight = new QToolBar();
    toolBarRight->setProperty("modulebar", true);
    toolBarRight->setProperty("os", operatingSystem());
    toolBarRight->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBarRight->addWidget(addButton);
    toolBarRight->addSeparator();
    toolBarRight->addAction(m_sceneViewProblem->actOperateOnNodes);
    toolBarRight->addAction(m_sceneViewProblem->actOperateOnEdges);
    toolBarRight->addAction(m_sceneViewProblem->actOperateOnLabels);
    toolBarRight->addSeparator();
    toolBarRight->addAction(m_sceneViewProblem->actSceneObjectProperties);
    toolBarRight->addAction(m_sceneViewProblem->actSceneObjectDeleteSelected);
    toolBarRight->addSeparator();
    toolBarRight->addAction(m_sceneViewProblem->actSceneViewSelectRegion);
    toolBarRight->addAction(m_sceneViewProblem->actSceneObjectClearSelected);
    toolBarRight->addWidget(zoomButton);
    toolBarRight->addSeparator();
    toolBarRight->addWidget(exportButton);

    auto *toolButtonFields = new QToolButton();
    toolButtonFields->setText(tr("Field"));
    toolButtonFields->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonFields->setToolTip(tr("New field"));
    toolButtonFields->setMenu(mnuFields);
    toolButtonFields->setAutoRaise(true);
    toolButtonFields->setIcon(icon("menu_field"));
    toolButtonFields->setPopupMode(QToolButton::InstantPopup);

    auto *actButtonParameter = new QAction(icon("menu_parameter"), tr("Parameter"));
    connect(actButtonParameter, SIGNAL(triggered()), this, SLOT(doNewParameter()));

    toolButtonMaterials = new QToolButton();
    toolButtonMaterials->setText(tr("Material"));
    toolButtonMaterials->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonMaterials->setToolTip(tr("New material"));
    toolButtonMaterials->setMenu(mnuMaterials);
    toolButtonMaterials->setAutoRaise(true);
    toolButtonMaterials->setIcon(icon("menu_material"));
    toolButtonMaterials->setPopupMode(QToolButton::InstantPopup);

    toolButtonBoundaries = new QToolButton();
    toolButtonBoundaries->setText(tr("Boundary"));
    toolButtonBoundaries->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonBoundaries->setToolTip(tr("New boundary"));
    toolButtonBoundaries->setMenu(mnuBoundaries);
    toolButtonBoundaries->setAutoRaise(true);
    toolButtonBoundaries->setIcon(icon("menu_boundary"));
    toolButtonBoundaries->setPopupMode(QToolButton::InstantPopup);

    auto *toolButtonFunctions = new QToolButton();
    toolButtonFunctions->setText(tr("Function"));
    toolButtonFunctions->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonFunctions->setToolTip(tr("New function"));
    toolButtonFunctions->addAction(actNewFunctionAnalytic);
    // toolButtonFunctions->addAction(actNewFunctionInterpolation);
    toolButtonFunctions->setAutoRaise(true);
    toolButtonFunctions->setIcon(icon("menu_function"));
    toolButtonFunctions->setPopupMode(QToolButton::InstantPopup);

    // left toolbar
    toolBarLeft = new QToolBar();
    toolBarLeft->setProperty("topbar", true);
    toolBarLeft->setProperty("os", operatingSystem());
    toolBarLeft->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBarLeft->addWidget(toolButtonFields);
    toolBarLeft->addAction(actButtonParameter);
    toolBarLeft->addSeparator();
    toolBarLeft->addWidget(toolButtonMaterials);
    toolBarLeft->addWidget(toolButtonBoundaries);
    toolBarLeft->addSeparator();
    toolBarLeft->addWidget(toolButtonFunctions);

    trvWidget = new QTreeWidget(this);
    trvWidget->setExpandsOnDoubleClick(false);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setUniformRowHeights(true);
    trvWidget->setColumnCount(2);
    trvWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trvWidget->header()->resizeSection(0, 220);
    trvWidget->header()->setStretchLastSection(true);
    trvWidget->setIndentation(trvWidget->indentation() - 2);

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    // problem
    problemNode = new QTreeWidgetItem(trvWidget);
    problemNode->setText(0, tr("Problem"));
    problemNode->setFont(0, fnt);
    problemNode->setIcon(0, icon("problem"));
    problemNode->setToolTip(0, problemPropertiesToString());
    problemNode->setData(1, Qt::UserRole, PreprocessorWidget::ProblemProperties);
    problemNode->setExpanded(true);

    auto problemPropertiesNode = new QTreeWidgetItem(problemNode);
    problemPropertiesNode->setText(0, tr("Properties"));
    problemPropertiesNode->setIcon(0, icon("problem_properties"));
    problemPropertiesNode->setData(1, Qt::UserRole, PreprocessorWidget::ProblemProperties);

    parametersNode = new QTreeWidgetItem(problemNode);
    parametersNode->setText(0, tr("Parameters"));
    parametersNode->setIcon(0, icon("menu_parameter"));
    parametersNode->setFont(0, fnt);
    parametersNode->setExpanded(true);

    functionNode = new QTreeWidgetItem(problemNode);
    functionNode->setText(0, tr("Functions"));
    functionNode->setIcon(0, icon("menu_function"));
    functionNode->setFont(0, fnt);
    functionNode->setExpanded(true);

    // fields
    fieldsNode = new QTreeWidgetItem(trvWidget);
    fieldsNode->setText(0, tr("Fields"));
    fieldsNode->setIcon(0, icon("menu_field"));
    fieldsNode->setFont(0, fnt);
    fieldsNode->setExpanded(true);

    // geometry
    geometryNode = new QTreeWidgetItem(trvWidget);
    geometryNode->setText(0, tr("Geometry"));
    geometryNode->setIcon(0, icon("geometry"));
    geometryNode->setFont(0, fnt);
    geometryNode->setExpanded(false);

    // nodes
    nodesNode = new QTreeWidgetItem(geometryNode);
    nodesNode->setText(0, tr("Nodes"));
    nodesNode->setIcon(0, icon("geometry_node"));
    nodesNode->setFont(0, fnt);

    // edges
    edgesNode = new QTreeWidgetItem(geometryNode);
    edgesNode->setText(0, tr("Edges"));
    edgesNode->setIcon(0, icon("geometry_edge"));
    edgesNode->setFont(0, fnt);

    // labels
    labelsNode = new QTreeWidgetItem(geometryNode);
    labelsNode->setText(0, tr("Labels"));
    labelsNode->setIcon(0, icon("geometry_label"));
    labelsNode->setFont(0, fnt);

    txtViewNodes = new QTextEdit(this);
    txtViewNodes->setReadOnly(true);
    txtViewNodes->setVisible(false);
    txtViewNodes->setText(createTooltipOperateOnNodes());

    txtViewEdges = new QTextEdit(this);
    txtViewEdges->setReadOnly(true);
    txtViewEdges->setVisible(false);
    txtViewEdges->setText(createTooltipOperateOnEdges());

    txtViewLabels = new QTextEdit(this);
    txtViewLabels->setReadOnly(true);
    txtViewLabels->setVisible(false);
    txtViewLabels->setText(createTooltipOperateOnLabels());

    loadTooltip(SceneGeometryMode_OperateOnNodes);

    auto *layoutView = new QHBoxLayout();
    layoutView->setContentsMargins(0, 0, 0, 0);
    layoutView->addWidget(txtViewNodes);
    layoutView->addWidget(txtViewEdges);
    layoutView->addWidget(txtViewLabels);

    auto viewWidget = new QWidget();
    viewWidget->setLayout(layoutView);

    txtGridStep = new QLineEdit("0.1");
    txtGridStep->setValidator(new QDoubleValidator(txtGridStep));
    chkSnapToGrid = new QCheckBox(tr("Snap to grid"));

    auto *btnOK = new QPushButton(tr("Apply"));
    connect(btnOK, SIGNAL(clicked()), SLOT(doApply()));

    auto *layoutTreeView = new QGridLayout();
    layoutTreeView->setContentsMargins(2, 2, 2, 3);
    layoutTreeView->addWidget(trvWidget, 1, 0, 1, 4);
    layoutTreeView->addWidget(new QLabel(tr("Grid step:")), 3, 0);
    layoutTreeView->addWidget(txtGridStep, 3, 1);
    layoutTreeView->addWidget(chkSnapToGrid, 3, 2);
    layoutTreeView->addWidget(btnOK, 3, 3);

    auto *widgetTreeView = new QWidget();
    widgetTreeView->setLayout(layoutTreeView);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(widgetTreeView);
    splitter->addWidget(viewWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    splitter->restoreState(settings.value("PreprocessorWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("PreprocessorWidget/SplitterGeometry").toByteArray());

    auto *layoutLeft = new QVBoxLayout();
    layoutLeft->setContentsMargins(2, 2, 2, 3);
    layoutLeft->addWidget(toolBarLeft);
    layoutLeft->addWidget(splitter);

    auto *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(toolBarRight);
    layoutRight->addWidget(m_sceneViewProblem);

    auto *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addLayout(layoutLeft);
    layoutMain->addLayout(layoutRight);
    layoutMain->setStretch(1, 1);

    setLayout(layoutMain);
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
    txtGridStep->setText(QString::number(Agros::problem()->config()->value(ProblemConfig::GridStep).toDouble()));
    chkSnapToGrid->setChecked(Agros::problem()->config()->value(ProblemConfig::SnapToGrid).toBool());

    trvWidget->blockSignals(true);
    trvWidget->setUpdatesEnabled(false);
    // trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    // parameters
    foreach(auto i, parametersNode->takeChildren()) delete i;
    QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();
    if (parameters.count() > 0)
    {
        foreach (QString key, parameters.keys())
        {
            auto *item = new QTreeWidgetItem(parametersNode);
            item->setText(0, QString("%1").arg(key));
            item->setText(1, QString("%1").arg(parameters[key].value()));
            item->setData(0, Qt::UserRole, key);
            item->setData(1, Qt::UserRole, PreprocessorWidget::ModelParameter);
        }
    }

    // functions
    ProblemFunctions *functions = Agros::problem()->config()->functions();
    foreach (auto i, functionNode->takeChildren()) delete i;
    if (functions->items().count() > 0)
    {
        foreach (ProblemFunction *function, functions->items())
        {
            auto *item = new QTreeWidgetItem(functionNode);
            item->setText(0, function->name());
            // item->setText(1, QString::number(parameters[key]));
            item->setData(0, Qt::UserRole, function->name());
            item->setData(1, Qt::UserRole, PreprocessorWidget::ModelFunction);
        }
    }

    // field and markers
    foreach (auto i, fieldsNode->takeChildren()) delete i;
    foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
    {
        // field
        auto *fieldNode = new QTreeWidgetItem(fieldsNode);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        fieldNode->setIcon(0, icon("fields/" + fieldInfo->fieldId()));
        fieldNode->setToolTip(0, fieldPropertiesToString(fieldInfo));
        fieldNode->setData(0, Qt::UserRole, fieldInfo->fieldId());
        fieldNode->setData(1, Qt::UserRole, PreprocessorWidget::FieldProperties);
        fieldNode->setExpanded(true);

        auto fieldPropertiesNode = new QTreeWidgetItem(fieldNode);
        fieldPropertiesNode->setText(0, tr("Properties"));
        fieldPropertiesNode->setIcon(0, icon("field_properties"));
        fieldPropertiesNode->setData(0, Qt::UserRole, fieldInfo->fieldId());
        fieldPropertiesNode->setData(1, Qt::UserRole, PreprocessorWidget::FieldProperties);

        // materials
        auto *materialsNode = new QTreeWidgetItem(fieldNode);
        materialsNode->setIcon(0, icon("menu_material"));
        materialsNode->setText(0, tr("Materials"));
        materialsNode->setFont(0, fnt);
        materialsNode->setExpanded(true);

        foreach (SceneMaterial *material, Agros::problem()->scene()->materials->filter(fieldInfo).items())
        {
            auto *item = new QTreeWidgetItem(materialsNode);

            item->setText(0, material->name());
            if (Agros::problem()->scene()->labels->haveMarker(material).isEmpty())
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, Agros::problem()->scene()->materials->items().indexOf(material));
            item->setData(1, Qt::UserRole, PreprocessorWidget::Material);
        }

        // boundary conditions
        auto *boundaryConditionsNode = new QTreeWidgetItem(fieldNode);
        boundaryConditionsNode->setIcon(0, icon("menu_boundary"));
        boundaryConditionsNode->setText(0, tr("Boundary conditions"));
        boundaryConditionsNode->setFont(0, fnt);
        boundaryConditionsNode->setExpanded(true);

        foreach (SceneBoundary *boundary, Agros::problem()->scene()->boundaries->filter(fieldInfo).items())
        {
            auto *item = new QTreeWidgetItem(boundaryConditionsNode);

            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

            item->setText(0, QString("%1").arg(boundary->name()));
            item->setText(1, QString("%1").arg(boundaryType.name()));
            if (Agros::problem()->scene()->faces->haveMarker(boundary).isEmpty())
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, Agros::problem()->scene()->boundaries->items().indexOf(boundary));
            item->setData(1, Qt::UserRole, PreprocessorWidget::Boundary);
        }
    }

    foreach (auto i, nodesNode->takeChildren()) delete i;
    int inode = 0;
    foreach (SceneNode *node, Agros::problem()->scene()->nodes->items())
    {
        auto *item = new QTreeWidgetItem(nodesNode);
        item->setText(0, QString("%1").arg(inode));
        item->setText(1, QString("[%1; %2] ").arg(node->point().x, 0, 'e', 2).arg(node->point().y, 0, 'e', 2));
        item->setData(0, Qt::UserRole, Agros::problem()->scene()->nodes->items().indexOf(node));
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryNode);

        inode++;
    }

    foreach(auto i, edgesNode->takeChildren()) delete i;
    int iface = 0;
    foreach (SceneFace *face, Agros::problem()->scene()->faces->items())
    {
        auto *item = new QTreeWidgetItem(edgesNode);
        item->setText(0, QString("%1. %2 m").
                      arg(iface).arg((face->angle() < EPS_ZERO) ? (face->nodeEnd()->point() - face->nodeStart()->point()).magnitude() : face->radius() * face->angle() / 180.0 * M_PI, 0, 'e', 2));
        item->setData(0, Qt::UserRole, Agros::problem()->scene()->faces->items().indexOf(face));
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryEdge);

        iface++;
    }

    int ilabel = 0;
    foreach (auto i, labelsNode->takeChildren()) delete i;
    foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
    {
        auto *item = new QTreeWidgetItem(labelsNode);
        item->setText(0, QString("%1").arg(ilabel));
        item->setText(1, QString("[%1; %2]").arg(label->point().x, 0, 'e', 2).arg(label->point().y, 0, 'e', 2));
        item->setData(0, Qt::UserRole, Agros::problem()->scene()->labels->items().indexOf(label));
        item->setData(1, Qt::UserRole, GeometryLabel);

        ilabel++;
    }

    trvWidget->setUpdatesEnabled(true);
    trvWidget->blockSignals(false);

    // boundaries
    mnuMaterials->clear();
    foreach(FieldInfo* fieldInfo, Agros::problem()->fieldInfos())
        mnuMaterials->addAction(actNewMaterials[fieldInfo->fieldId()]);

    // materials
    mnuBoundaries->clear();
    foreach(FieldInfo* fieldInfo, Agros::problem()->fieldInfos())
        mnuBoundaries->addAction(actNewBoundaries[fieldInfo->fieldId()]);

    // fields
    mnuFields->clear();
    foreach(StringAction *fieldAction, actNewFields.values())
        if (!Agros::problem()->fieldInfos().keys().contains(fieldAction->key()))
            mnuFields->addAction(fieldAction);

    // create menu
    createMenu();

    // refresh view
    m_sceneViewProblem->refresh();
}

void PreprocessorWidget::loadTooltip(SceneGeometryMode sceneMode)
{
    txtViewNodes->setVisible(sceneMode == SceneGeometryMode_OperateOnNodes);
    txtViewEdges->setVisible(sceneMode == SceneGeometryMode_OperateOnEdges);
    txtViewLabels->setVisible(sceneMode == SceneGeometryMode_OperateOnLabels);
}

QString PreprocessorWidget::problemPropertiesToString()
{
    QMap<QString, QString> items;
    items[tr("Coordinate type")] = coordinateTypeString(Agros::problem()->config()->coordinateType());
    items[tr("Mesh type")] = meshTypeString(Agros::problem()->config()->meshType());

    if (Agros::problem()->isHarmonic())
    {
        items[tr("Frequency")] = Agros::problem()->config()->value(ProblemConfig::Frequency).value<Value>().toString() + " Hz";
    }
    if (Agros::problem()->isTransient())
    {
        items[tr("Method")] = timeStepMethodString((TimeStepMethod) Agros::problem()->config()->value(ProblemConfig::TimeMethod).toInt());
        items[tr("Order")] = QString::number(Agros::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
        items[tr("Tolerance")] = QString("%1 %").arg(Agros::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
        items[tr("Initial step size")] = QString("%1").arg(Agros::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
        items[tr("Constant time step")] = QString("%1 s").arg(QString::number(Agros::problem()->config()->constantTimeStepLength()));
        items[tr("Number of const. time steps")] = QString("%1 s").arg(Agros::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
        items[tr("Total time")] = QString("%1 s").arg(Agros::problem()->config()->value(ProblemConfig::TimeTotal).toDouble());
    }

    return createToolTip(tr("Problem properties"), items);
}

QString PreprocessorWidget::fieldPropertiesToString(FieldInfo *fieldInfo)
{
    QMap<QString, QString> items;
    items[tr("Analysis")] = QString("%1").arg(analysisTypeString(fieldInfo->analysisType()));
    items[tr("Solver")] = QString("%1").arg(linearityTypeString(fieldInfo->linearityType()));
    items[tr("Number of refinements")] = QString("%1").arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    items[tr("Polynomial order")] = QString("%1").arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    items[tr("Adaptivity")] = QString("%1").arg(adaptivityTypeString(fieldInfo->adaptivityType()));
    items[tr("Matrix solver")] = QString("%1").arg(matrixSolverTypeString(fieldInfo->matrixSolver()));

    return createToolTip(fieldInfo->name(), items);
}

void PreprocessorWidget::doContextMenu(const QPoint &pos)
{
    auto *current = trvWidget->itemAt(pos);
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

    Agros::problem()->scene()->selectNone();
    Agros::problem()->scene()->highlightNone();

    if (current)
    {
        PreprocessorWidget::Type type = (PreprocessorWidget::Type) trvWidget->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == PreprocessorWidget::GeometryNode || type == PreprocessorWidget::GeometryEdge || type == PreprocessorWidget::GeometryLabel)
        {
            // geometry
            SceneBasic *objectBasic = nullptr;

            if (type == PreprocessorWidget::GeometryNode)
            {
                m_sceneViewProblem->actOperateOnNodes->trigger();
                objectBasic = Agros::problem()->scene()->nodes->at(current->data(0, Qt::UserRole).toInt());
            }
            if (type == PreprocessorWidget::GeometryEdge)
            {
                m_sceneViewProblem->actOperateOnEdges->trigger();
                objectBasic = Agros::problem()->scene()->faces->at(current->data(0, Qt::UserRole).toInt());
            }
            if (type == PreprocessorWidget::GeometryLabel)
            {
                m_sceneViewProblem->actOperateOnLabels->trigger();
                objectBasic = Agros::problem()->scene()->labels->at(current->data(0, Qt::UserRole).toInt());
            }

            objectBasic->setSelected(true);

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);

            m_sceneViewProblem->refresh();
        }
        else if (type == PreprocessorWidget::Boundary)
        {
            // edge marker
            // select all edges
            SceneBoundary *objectBoundary = Agros::problem()->scene()->boundaries->at(current->data(0, Qt::UserRole).toInt());
            m_sceneViewProblem->actOperateOnEdges->trigger();

            Agros::problem()->scene()->faces->haveMarker(objectBoundary).setSelected();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);

            m_sceneViewProblem->refresh();
        }
        else if (type == PreprocessorWidget::Material)
        {
            // label marker
            // select all labels
            SceneMaterial *objectMaterial = Agros::problem()->scene()->materials->at(current->data(0, Qt::UserRole).toInt());
            m_sceneViewProblem->actOperateOnLabels->trigger();

            Agros::problem()->scene()->labels->haveMarker(objectMaterial).setSelected();

            actProperties->setEnabled(true);
            actDelete->setEnabled(true);

            m_sceneViewProblem->refresh();
        }
        else if (type == PreprocessorWidget::ModelParameter)
        {
            // geometry
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::ModelFunction)
        {
            // function
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::FieldProperties)
        {
            // field
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::ProblemProperties)
        {
            // problem
            actProperties->setEnabled(true);
        }

        emit changed();
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
            // geometry - node
            if (type == PreprocessorWidget::GeometryNode)
            {
                auto *dialog = new SceneNodeDialog(Agros::problem()->scene()->nodes->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()), this, true);
                if (dialog->exec() == QDialog::Accepted)
                {
                    m_sceneViewProblem->refresh();
                    refresh();
                }
            }
            // geometry - face
            if (type == PreprocessorWidget::GeometryEdge)
            {
                auto *dialog = new SceneFaceDialog(Agros::problem()->scene()->faces->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()), this, true);
                if (dialog->exec() == QDialog::Accepted)
                {
                    m_sceneViewProblem->refresh();
                    refresh();
                }
            }
            // geometry - label
            if (type == PreprocessorWidget::GeometryLabel)
            {
                auto *dialog = new SceneLabelDialog(Agros::problem()->scene()->labels->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()), this, true);
                if (dialog->exec() == QDialog::Accepted)
                {
                    m_sceneViewProblem->refresh();
                    refresh();
                }
            }
        }
        else if (type == PreprocessorWidget::Boundary)
        {
            // edge marker
            SceneBoundary *objectBoundary = Agros::problem()->scene()->boundaries->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            auto *dialog = new SceneBoundaryDialog(objectBoundary, this);

            if (dialog->exec() == QDialog::Accepted)
            {
                m_sceneViewProblem->refresh();
                refresh();
            }
        }
        else if (type == PreprocessorWidget::Material)
        {
            // label marker
            SceneMaterial *objectMaterial = Agros::problem()->scene()->materials->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            auto *dialog = new SceneMaterialDialog(objectMaterial, this);

            if (dialog->exec() == QDialog::Accepted)
            {
                m_sceneViewProblem->refresh();
                refresh();
            }
        }
        else if (type == PreprocessorWidget::ModelParameter)
        {
            // parameter
            QString key = trvWidget->currentItem()->data(0, Qt::UserRole).toString();

            ParameterDialog dialog(key, this);
            if (dialog.exec() == QDialog::Accepted)
            {
                m_sceneViewProblem->refresh();
                refresh();
            }
        }
        else if (type == PreprocessorWidget::ModelFunction)
        {
            // function
            QString key = trvWidget->currentItem()->data(0, Qt::UserRole).toString();
            ProblemFunction *function = Agros::problem()->config()->functions()->function(key);

            ProblemFunctionDialog *dialog = nullptr;
            if (function->type() == ProblemFunctionType_Analytic)
                dialog = new ProblemFunctionAnalyticDialog(dynamic_cast<ProblemFunctionAnalytic *>(function), this);
            else if (function->type() == ProblemFunctionType_Interpolation)
                assert(0); // dialog = new ProblemFunctionAnalytic(function, this);
            else
                assert(0);

            if (dialog->showDialog() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == PreprocessorWidget::FieldProperties)
        {
            // field properties
            FieldInfo *fieldInfo = Agros::problem()->fieldInfo(trvWidget->currentItem()->data(0, Qt::UserRole).toString());

            FieldDialog fieldDialog(fieldInfo, this);
            if (fieldDialog.exec() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == PreprocessorWidget::ProblemProperties)
        {
            // problem properties
            ProblemDialog problemDialog(this);
            if (problemDialog.exec() == QDialog::Accepted)
            {
                refresh();
            }
        }

        emit changed();
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
            if (type == PreprocessorWidget::GeometryNode)
            {
                undoStack()->beginMacro(tr("Delete selected"));
                SceneNode *node = Agros::problem()->scene()->nodes->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
                QList<PointValue> selectedNodePoints;
                selectedNodePoints.append(node->pointValue());

                undoStack()->push(new SceneNodeCommandRemoveMulti(selectedNodePoints));
                undoStack()->endMacro();

                Agros::problem()->scene()->invalidate();
                m_sceneViewProblem->update();
            }
            else if (type == PreprocessorWidget::GeometryEdge)
            {
                undoStack()->beginMacro(tr("Delete selected"));
                SceneFace *edge = Agros::problem()->scene()->faces->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());

                QList<PointValue> selectedEdgePointsStart;
                QList<PointValue> selectedEdgePointsEnd;
                QList<Value> selectedEdgeAngles;
                QList<int> selectedEdgeSegments;
                QList<QMap<QString, QString> > selectedEdgeMarkers;

                selectedEdgePointsStart.append(edge->nodeStart()->pointValue());
                selectedEdgePointsEnd.append(edge->nodeEnd()->pointValue());
                selectedEdgeAngles.append(edge->angleValue());
                selectedEdgeSegments.append(edge->segments());
                selectedEdgeMarkers.append(edge->markersKeys());

                undoStack()->push(new SceneEdgeCommandRemoveMulti(selectedEdgePointsStart, selectedEdgePointsEnd,
                                                          selectedEdgeMarkers, selectedEdgeAngles, selectedEdgeSegments));
                undoStack()->endMacro();


                Agros::problem()->scene()->invalidate();
                m_sceneViewProblem->update();
            }
            else if (type == PreprocessorWidget::GeometryLabel)
            {
                undoStack()->beginMacro(tr("Delete selected"));
                SceneLabel *label = Agros::problem()->scene()->labels->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());

                QList<PointValue> selectedLabelPointsStart;
                QList<double> selectedLabelAreas;
                QList<QMap<QString, QString> > selectedLabelMarkers;

                selectedLabelPointsStart.append(label->pointValue());
                selectedLabelAreas.append(label->area());
                selectedLabelMarkers.append(label->markersKeys());

                undoStack()->push(new SceneLabelCommandRemoveMulti(selectedLabelPointsStart, selectedLabelMarkers, selectedLabelAreas));

                undoStack()->endMacro();

                Agros::problem()->scene()->invalidate();
                m_sceneViewProblem->update();
            }
        }
        else if (type == PreprocessorWidget::Material)
        {
            // label marker
            SceneMaterial *objectMaterial = Agros::problem()->scene()->materials->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            Agros::problem()->scene()->removeMaterial(objectMaterial);
            Agros::problem()->scene()->invalidate();
        }
        else if (type == PreprocessorWidget::Boundary)
        {
            // edge marker
            SceneBoundary *objectBoundary = Agros::problem()->scene()->boundaries->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            Agros::problem()->scene()->removeBoundary(objectBoundary);
            Agros::problem()->scene()->invalidate();
        }
        else if (type == PreprocessorWidget::ModelParameter)
        {
            // parameter
            QString key = trvWidget->currentItem()->data(0, Qt::UserRole).toString();
            QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();
            parameters.remove(key);

            QStringList err = Agros::problem()->checkAndApplyParameters(parameters);
            if (!err.isEmpty())
            {
                QString msg;
                foreach (QString str, err)
                    msg += str + "\n";
                QMessageBox::critical(this, tr("Parameters error"), msg);
            }
        }
        else if (type == PreprocessorWidget::ModelFunction)
        {
            // function
            QString key = trvWidget->currentItem()->data(0, Qt::UserRole).toString();
            Agros::problem()->config()->functions()->remove(key);
        }
        else if (type == PreprocessorWidget::FieldProperties)
        {
            // parameter
            FieldInfo *fieldInfo = Agros::problem()->fieldInfo(trvWidget->currentItem()->data(0, Qt::UserRole).toString());

            if (QMessageBox::question(this, tr("Delete"), tr("Physical field '%1' will be pernamently deleted. Are you sure?").
                                      arg(fieldInfo->name()), tr("&Yes"), tr("&No")) == 0)
            {
                Agros::problem()->removeField(fieldInfo);
            }
        }

        refresh();
        emit changed();
    }
}

void PreprocessorWidget::doNewParameter()
{
    ParameterDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        refresh();
        emit changed();
    }
}

void PreprocessorWidget::doNewFunctionAnalytic()
{
    ProblemFunctionAnalytic *function = new ProblemFunctionAnalytic("", "value");

    ProblemFunctionAnalyticDialog dialog(function, this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        Agros::problem()->config()->functions()->add(function);

        refresh();
        emit changed();
    }
    else
    {
        delete function;
    }
}

void PreprocessorWidget::doNewFunctionInterpolation()
{
    /*
            FunctionAnalyticDialog dialog(this);
            if (dialog.exec() == QDialog::Accepted)
            {
                m_sceneViewProblem->refresh();
                refresh();
            }
            */
}

void PreprocessorWidget::doNewField(const QString &field)
{
    // add field
    FieldInfo *fieldInfo = new FieldInfo(field);

    FieldDialog fieldDialog(fieldInfo, this);
    if (fieldDialog.exec() == QDialog::Accepted)
    {
        Agros::problem()->addField(fieldInfo);

        refresh();
        emit changed();
    }
    else
    {
        delete fieldInfo;
    }
}

void PreprocessorWidget::doApply()
{
    Agros::problem()->config()->setValue(ProblemConfig::GridStep, txtGridStep->text().toDouble());
    Agros::problem()->config()->setValue(ProblemConfig::SnapToGrid, chkSnapToGrid->isChecked());
}

void PreprocessorWidget::doNewNode(const Point &point)
{
    auto *node = new SceneNode(Agros::problem()->scene(), PointValue(Agros::problem(), point));
    auto *dialog = new SceneNodeDialog(node, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneNode *nodeAdded = Agros::problem()->scene()->addNode(node);
        Agros::problem()->scene()->invalidate();

        if (nodeAdded == node)
            undoStack()->push(new SceneNodeCommandAdd(node->pointValue()));

        refresh();
        emit changed();
    }
    else
    {
        delete node;
    }
}

void PreprocessorWidget::doNewEdge()
{
    auto *edge = new SceneFace(Agros::problem()->scene(), Agros::problem()->scene()->nodes->at(0), Agros::problem()->scene()->nodes->at(1), Value(Agros::problem(), 0.0));
    auto *dialog = new SceneFaceDialog(edge, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneFace *edgeAdded = Agros::problem()->scene()->addFace(edge);
        Agros::problem()->scene()->invalidate();

        refresh();
        emit changed();

        if (edgeAdded == edge)
            undoStack()->push(getAddCommand(edge));
    }
    else
    {
        delete edge;
    }
}

void PreprocessorWidget::doNewLabel(const Point &point)
{
    auto *label = new SceneLabel(Agros::problem()->scene(), PointValue(Agros::problem(), point), 0.0);
    auto *dialog = new SceneLabelDialog(label, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneLabel *labelAdded = Agros::problem()->scene()->addLabel(label);
        Agros::problem()->scene()->invalidate();

        if (labelAdded == label)
            undoStack()->push(getAddCommand(label));

        refresh();
        emit changed();
    }
    else
    {
        delete label;
    }
}

void PreprocessorWidget::doNewRectangle()
{
    auto *nodeLB = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));
    auto *nodeRB = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));
    auto *nodeLT = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));
    auto *nodeRT = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));

    auto *dialog = new SceneRectangleDialog(nodeLB, nodeRB, nodeLT, nodeRT, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneNode *nodeAddedLB = Agros::problem()->scene()->addNode(nodeLB);
        SceneNode *nodeAddedRB = Agros::problem()->scene()->addNode(nodeRB);
        SceneNode *nodeAddedLT = Agros::problem()->scene()->addNode(nodeLT);
        SceneNode *nodeAddedRT = Agros::problem()->scene()->addNode(nodeRT);

        auto *edgeB = new SceneFace(Agros::problem()->scene(), nodeAddedLB, nodeAddedRB, Value(Agros::problem(), 0.0));
        auto *edgeR = new SceneFace(Agros::problem()->scene(), nodeAddedRB, nodeAddedRT, Value(Agros::problem(), 0.0));
        auto *edgeT = new SceneFace(Agros::problem()->scene(), nodeAddedRT, nodeAddedLT, Value(Agros::problem(), 0.0));
        auto *edgeL = new SceneFace(Agros::problem()->scene(), nodeAddedLT, nodeAddedLB, Value(Agros::problem(), 0.0));

        SceneFace *edgeAddedB = Agros::problem()->scene()->addFace(edgeB);
        SceneFace *edgeAddedR = Agros::problem()->scene()->addFace(edgeR);
        SceneFace *edgeAddedT = Agros::problem()->scene()->addFace(edgeT);
        SceneFace *edgeAddedL = Agros::problem()->scene()->addFace(edgeL);

        Agros::problem()->scene()->invalidate();

        if (nodeAddedLB == nodeLB)
            undoStack()->push(new SceneNodeCommandAdd(nodeLB->pointValue()));
        if (nodeAddedRB == nodeRB)
            undoStack()->push(new SceneNodeCommandAdd(nodeRB->pointValue()));
        if (nodeAddedLT == nodeLT)
            undoStack()->push(new SceneNodeCommandAdd(nodeLT->pointValue()));
        if (nodeAddedRT == nodeRT)
            undoStack()->push(new SceneNodeCommandAdd(nodeRT->pointValue()));

        if (edgeAddedB == edgeB)
            undoStack()->push(getAddCommand(edgeB));
        if (edgeAddedR == edgeR)
            undoStack()->push(getAddCommand(edgeR));
        if (edgeAddedT == edgeT)
            undoStack()->push(getAddCommand(edgeT));
        if (edgeAddedL == edgeL)
            undoStack()->push(getAddCommand(edgeL));

        refresh();
        emit changed();
    }
    else
    {
        delete nodeLB;
        delete nodeRB;
        delete nodeLT;
        delete nodeRT;
    }
}

void PreprocessorWidget::doNewCircle()
{
    auto *nodeL = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));
    auto *nodeR = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));
    auto *nodeB = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));
    auto *nodeT = new SceneNode(Agros::problem()->scene(), PointValue(Value(Agros::problem(), "0.0"), Value(Agros::problem(), "0.0")));

    auto *dialog = new SceneCircleDialog(nodeL, nodeR, nodeB, nodeT, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneNode *nodeAddedL = Agros::problem()->scene()->addNode(nodeL);
        SceneNode *nodeAddedR = Agros::problem()->scene()->addNode(nodeR);
        SceneNode *nodeAddedB = Agros::problem()->scene()->addNode(nodeB);
        SceneNode *nodeAddedT = Agros::problem()->scene()->addNode(nodeT);

        auto *edgeB = new SceneFace(Agros::problem()->scene(), nodeAddedB, nodeAddedR, Value(Agros::problem(), 90.0));
        auto *edgeR = new SceneFace(Agros::problem()->scene(), nodeAddedR, nodeAddedT, Value(Agros::problem(), 90.0));
        auto *edgeT = new SceneFace(Agros::problem()->scene(), nodeAddedT, nodeAddedL, Value(Agros::problem(), 90.0));
        auto *edgeL = new SceneFace(Agros::problem()->scene(), nodeAddedL, nodeAddedB, Value(Agros::problem(), 90.0));

        SceneFace *edgeAddedB = Agros::problem()->scene()->addFace(edgeB);
        SceneFace *edgeAddedR = Agros::problem()->scene()->addFace(edgeR);
        SceneFace *edgeAddedT = Agros::problem()->scene()->addFace(edgeT);
        SceneFace *edgeAddedL = Agros::problem()->scene()->addFace(edgeL);

        Agros::problem()->scene()->invalidate();

        if (nodeAddedL == nodeL)
            undoStack()->push(new SceneNodeCommandAdd(nodeL->pointValue()));
        if (nodeAddedR == nodeR)
            undoStack()->push(new SceneNodeCommandAdd(nodeR->pointValue()));
        if (nodeAddedB == nodeB)
            undoStack()->push(new SceneNodeCommandAdd(nodeB->pointValue()));
        if (nodeAddedT == nodeT)
            undoStack()->push(new SceneNodeCommandAdd(nodeT->pointValue()));

        if (edgeAddedB == edgeB)
            undoStack()->push(getAddCommand(edgeB));
        if (edgeAddedR == edgeR)
            undoStack()->push(getAddCommand(edgeR));
        if (edgeAddedT == edgeT)
            undoStack()->push(getAddCommand(edgeT));
        if (edgeAddedL == edgeL)
            undoStack()->push(getAddCommand(edgeL));

        refresh();
        emit changed();
    }
    else
    {
        delete nodeL;
        delete nodeR;
        delete nodeB;
        delete nodeT;
    }
}

void PreprocessorWidget::doNewBoundary(const QString &field)
{
    // first boundary as default
    QList<Module::BoundaryType> boundaryTypes = Agros::problem()->fieldInfo(field)->boundaryTypes();
    assert(boundaryTypes.count() > 1);

    auto *boundary = new SceneBoundary(Agros::problem()->scene(),
                                                Agros::problem()->fieldInfo(field),
                                                tr("new boundary"),
                                                boundaryTypes.first().id());
    auto *dialog = new SceneBoundaryDialog(boundary, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        Agros::problem()->scene()->addBoundary(boundary);
        Agros::problem()->scene()->invalidate();

        refresh();
        emit changed();
    }
    else
    {
        delete boundary;
    }
}

void PreprocessorWidget::doNewMaterial(const QString &field)
{
    auto *material = new SceneMaterial(Agros::problem()->scene(),
                                                Agros::problem()->fieldInfo(field),
                                                tr("new material"));

    auto *dialog = new SceneMaterialDialog(material, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        Agros::problem()->scene()->addMaterial(material);
        Agros::problem()->scene()->invalidate();

        refresh();
        emit changed();
    }
    else
    {
        delete material;
    }
}

void PreprocessorWidget::exportGeometryToClipboard()
{
    // copy image to clipboard
    QPixmap pixmap = m_sceneViewProblem->renderScenePixmap();
    QImage image = pixmap.toImage().convertToFormat(QImage::Format_RGB32);

    QApplication::clipboard()->setImage(image);
}

void PreprocessorWidget::exportGeometryToSvg()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString fn = QFileDialog::getSaveFileName(QApplication::activeWindow(), tr("Export geometry to file"), dir, tr("SVG files (*.svg)"));
    if (fn.isEmpty())
        return;

    if (!fn.endsWith(".svg"))
        fn.append(".svg");

    m_sceneViewProblem->saveGeometryToSvg(fn);

    QFileInfo fileInfo(fn);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void PreprocessorWidget::exportGeometryToBitmap()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString selectedFilter;
    QString fn = QFileDialog::getSaveFileName(QApplication::activeWindow(), tr("Export geometry to file"), dir, "PNG files (*.png);;JPG files (*.jpg);;BMP files (*.bmp)", &selectedFilter);
    if (fn.isEmpty())
        return;

    QString ext = selectedFilter.last(5).first(4);
    if (!fn.endsWith(ext))
        fn.append(ext);

    m_sceneViewProblem->saveImageToFile(fn);

    QFileInfo fileInfo(fn);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void PreprocessorWidget::exportGeometryToVTK()
{
    // file dialog
    QSettings settings;
    QString dir = settings.value("General/LastVTKDir").toString();

    QString fn = QFileDialog::getSaveFileName(QApplication::activeWindow(), tr("Export VTK file"), dir, tr("VTK files (*.vtk)"));
    if (fn.isEmpty())
        return;

    if (!fn.endsWith(".vtk"))
        fn.append(".vtk");

    Agros::problem()->scene()->exportVTKGeometry(fn);

    QFileInfo fileInfo(fn);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastVTKDir", fileInfo.absolutePath());
    }
}

void PreprocessorWidget::importGeometryFromDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fn = QFileDialog::getOpenFileName(this, tr("Import DXF file"), dir, tr("DXF files (*.dxf)"));
    if (fn.isEmpty())
        return;

    Agros::problem()->scene()->importFromDxf(fn);
    m_sceneViewProblem->doZoomBestFit();

    QFileInfo fileInfo(fn);
    if (fileInfo.absoluteDir() != tempProblemDir())
        settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
}

void PreprocessorWidget::exportGeometryToDXF()
{
    QSettings settings;
    QString dir = settings.value("General/LastDXFDir").toString();

    QString fn = QFileDialog::getSaveFileName(this, tr("Export DXF file"), dir, tr("DXF files (*.dxf)"));
    if (fn.isEmpty())
        return;

    QFileInfo fileInfo(fn);
    if (!fn.endsWith(".dxf"))
        fn.append(".dxf");

    Agros::problem()->scene()->exportToDxf(fn);

    if (fileInfo.absoluteDir() != tempProblemDir())
        settings.setValue("General/LastDXFDir", fileInfo.absolutePath());
}
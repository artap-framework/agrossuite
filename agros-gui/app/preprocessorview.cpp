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
#include "gui/recipedialog.h"
#include "gui/other.h"
#include "app/scenegeometrydialog.h"

#include "solver/problem_config.h"
#include "solver/problem_parameter.h"
#include "solver/problem_function.h"
#include "optilab/study.h"
#include "optilab/study_dialog.h"

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

StringAction::StringAction(QObject* parent, const QString &k, const QString &v) :
    QAction(v, parent),
    m_key(k), m_value(v)
{
    connect(this, SIGNAL(triggered()), this, SLOT(doTriggered()));
}

void StringAction::doTriggered()
{
    emit triggered(m_key);
}

PreprocessorWidget::PreprocessorWidget(QWidget *parent): QWidget(parent)
{
    m_sceneViewProblem = new SceneViewProblem(this);

    setMinimumWidth(200);
    setObjectName("PreprocessorView");

    createActions();

    // context menu
    mnuPreprocessor = new QMenu(this);
    mnuFields = new QMenu(tr("New fields"), this);
    mnuStudies = new QMenu(tr("New studies"), this);
    mnuMaterials = new QMenu(tr("New materials"), this);
    mnuBoundaries = new QMenu(tr("New boundaries"), this);
    connect(this, SIGNAL(refreshGUI()), m_sceneViewProblem, SLOT(refresh()));

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
    settings.setValue("PreprocessorWidget/TreeColumnWidth0", trvWidget->columnWidth(0));
    settings.setValue("PreprocessorWidget/TreeColumnWidth1", trvWidget->columnWidth(1));
    settings.setValue("PreprocessorWidget/SplitterState", splitter->saveState());
    settings.setValue("PreprocessorWidget/SplitterGeometry", splitter->saveGeometry());

    // clear fields
    foreach (QAction *action, actNewFields.values())
        delete action;
    actNewFields.clear();

    // clear studies
    foreach (QAction *action, actNewStudies.values())
        delete action;
    actNewStudies.clear();

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

    actNewRecipeLocalValue = new QAction(tr("Local value recipe..."), this);
    connect(actNewRecipeLocalValue, SIGNAL(triggered()), this, SLOT(doNewRecipeLocalValue()));
    actNewRecipeSurfaceIntegral = new QAction(tr("Surface integral recipe..."), this);
    connect(actNewRecipeSurfaceIntegral, SIGNAL(triggered()), this, SLOT(doNewRecipeSurfaceIntegral()));
    actNewRecipeVolumeIntegral = new QAction(tr("Volume integral recipe..."), this);
    connect(actNewRecipeVolumeIntegral, SIGNAL(triggered()), this, SLOT(doNewRecipeVolumeIntegral()));

    // scene - add items
    actNewNode = new QAction(icon("geometry_node"), tr("New &node..."), this);
    actNewNode->setShortcut(QKeySequence("Alt+N"));
    connect(actNewNode, SIGNAL(triggered()), this, SLOT(doNewNode()));
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewNode);

    actNewEdge = new QAction(icon("geometry_edge"), tr("New &edge..."), this);
    actNewEdge->setShortcut(QKeySequence("Alt+E"));
    connect(actNewEdge, SIGNAL(triggered()), this, SLOT(doNewEdge()));
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewEdge);

    actNewLabel = new QAction(icon("geometry_label"), tr("New &label..."), this);
    actNewLabel->setShortcut(QKeySequence("Alt+L"));
    connect(actNewLabel, SIGNAL(triggered()), this, SLOT(doNewLabel()));
    m_sceneViewProblem->menuScene()->insertAction(m_sceneViewProblem->menuScene()->actions().first(), actNewLabel);

    // actNewBoundary->setShortcut(QKeySequence("Alt+B"));
    // actNewMaterial->setShortcut(QKeySequence("Alt+M"));

    // clear fields
    foreach (StringAction *action, actNewFields.values())
        delete action;
    actNewFields.clear();

    // clear studies
    foreach (StringAction *action, actNewStudies.values())
        delete action;
    actNewStudies.clear();

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

        StringAction* actionField = new StringAction(this, it.key(), it.value());
        actionField->setIcon(icon("fields/" + it.key()));
        connect(actionField, SIGNAL(triggered(QString)), this, SLOT(doNewField(QString)));
        actNewFields[it.key()] = actionField;

        StringAction* actionBoundary = new StringAction(this, it.key(), it.value());
        connect(actionBoundary, SIGNAL(triggered(QString)), this, SLOT(doNewBoundary(QString)));
        actNewBoundaries[it.key()] = actionBoundary;

        StringAction* actionMaterial = new StringAction(this, it.key(), it.value());
        connect(actionMaterial, SIGNAL(triggered(QString)), this, SLOT(doNewMaterial(QString)));
        actNewMaterials[it.key()] = actionMaterial;
    }

    foreach (QString name, studyTypeStringKeys())
    {
        QString label = studyTypeString(studyTypeFromStringKey(name));

        StringAction* actionStudy = new StringAction(this, name, label);
        connect(actionStudy, SIGNAL(triggered(QString)), this, SLOT(doNewStudy(QString)));        
        actNewStudies[name] = actionStudy;
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

    QMenu *mnuFunctions = new QMenu(tr("New function"));
    mnuFunctions->addAction(actNewFunctionAnalytic);
    // mnuFunctions->addAction(actNewFunctionInterpolation);
    mnuPreprocessor->addMenu(mnuFunctions);

    QMenu *mnuRecipe = new QMenu(tr("New recipe"));
    mnuRecipe->addAction(actNewRecipeLocalValue);
    mnuRecipe->addAction(actNewRecipeSurfaceIntegral);
    mnuRecipe->addAction(actNewRecipeVolumeIntegral);

    mnuPreprocessor->addMenu(mnuRecipe);
    mnuPreprocessor->addMenu(mnuStudies);
    mnuPreprocessor->addSeparator();
    mnuPreprocessor->addAction(actDelete);
    mnuPreprocessor->addAction(actProperties);

    // enable
    bool enabled = !Agros::problem()->fieldInfos().isEmpty();
    // bool onlyOneField = (Agros::problem()->fieldInfos().size() == 1);

    mnuMaterials->setEnabled(enabled);
    mnuBoundaries->setEnabled(enabled);
    mnuRecipe->setEnabled(enabled);
    mnuStudies->setEnabled(enabled);

    // toolbar
    toolButtonMaterials->setEnabled(enabled);
    toolButtonBoundaries->setEnabled(enabled);
    toolButtonRecipes->setEnabled(enabled);
    toolButtonStudies->setEnabled(enabled);
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

    // zoom
    auto *mnuZoom = new QMenu(this);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomBestFit);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomIn);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomOut);
    mnuZoom->addAction(m_sceneViewProblem->actSceneZoomRegion);

    auto *addButton = new QToolButton();
    addButton->setText(tr("Add geometry"));
    addButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    addButton->setIconSize(QSize(24, 24));
    addButton->addAction(actNewNode);
    addButton->addAction(actNewEdge);
    addButton->addAction(actNewLabel);
    addButton->setAutoRaise(true);
    addButton->setIcon(icon("geometry_add"));
    addButton->setPopupMode(QToolButton::InstantPopup);

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
    toolBarRight->addWidget(zoomButton);

    auto *toolButtonFields = new QToolButton();
    toolButtonFields->setText(tr("Field"));
    toolButtonFields->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonFields->setToolTip(tr("New fields"));
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

    toolButtonRecipes = new QToolButton();
    toolButtonRecipes->setText(tr("Recipe"));
    toolButtonRecipes->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonRecipes->setToolTip(tr("New recipe"));
    toolButtonRecipes->addAction(actNewRecipeLocalValue);
    toolButtonRecipes->addAction(actNewRecipeSurfaceIntegral);
    toolButtonRecipes->addAction(actNewRecipeVolumeIntegral);
    toolButtonRecipes->setAutoRaise(true);
    toolButtonRecipes->setIcon(icon("menu_recipe"));
    toolButtonRecipes->setPopupMode(QToolButton::InstantPopup);

    toolButtonStudies = new QToolButton();
    toolButtonStudies->setText(tr("Study"));
    toolButtonStudies->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonStudies->setToolTip(tr("New studies"));
    toolButtonStudies->setMenu(mnuStudies);
    toolButtonStudies->setAutoRaise(true);
    toolButtonStudies->setIcon(icon("menu_study"));
    toolButtonStudies->setPopupMode(QToolButton::InstantPopup);

    // left toolbar
    toolBarLeft = new QToolBar();
    toolBarLeft->setProperty("topbar", true);
    toolBarLeft->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBarLeft->addWidget(toolButtonFields);
    toolBarLeft->addAction(actButtonParameter);
    toolBarLeft->addSeparator();
    toolBarLeft->addWidget(toolButtonMaterials);
    toolBarLeft->addWidget(toolButtonBoundaries);
    toolBarLeft->addSeparator();
    toolBarLeft->addWidget(toolButtonFunctions);
    toolBarLeft->addWidget(toolButtonRecipes);
    toolBarLeft->addWidget(toolButtonStudies);

    trvWidget = new QTreeWidget(this);
    trvWidget->setExpandsOnDoubleClick(false);
    trvWidget->setHeaderHidden(true);
    trvWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    trvWidget->setMouseTracking(true);
    trvWidget->setUniformRowHeights(true);
    trvWidget->setColumnCount(1);
    trvWidget->setIndentation(trvWidget->indentation() - 2);

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
    trvWidget->clear();

    QFont fnt = trvWidget->font();
    fnt.setBold(true);

    // problem
    QTreeWidgetItem *problemNode = new QTreeWidgetItem(trvWidget);
    problemNode->setText(0, tr("Problem"));
    problemNode->setFont(0, fnt);
    problemNode->setIcon(0, iconAlphabet('P', AlphabetColor_Brown));
    problemNode->setToolTip(0, problemPropertiesToString());
    problemNode->setData(1, Qt::UserRole, PreprocessorWidget::ProblemProperties);
    problemNode->setExpanded(true);

    // problem properties
    QTreeWidgetItem *problemPropertiesNode = new QTreeWidgetItem(problemNode);
    problemPropertiesNode->setText(0, tr("Properties"));
    problemPropertiesNode->setIcon(0, iconAlphabet('P', AlphabetColor_Green));
    problemPropertiesNode->setForeground(0, QBrush(Qt::darkGray));
    problemPropertiesNode->setData(1, Qt::UserRole, PreprocessorWidget::ProblemProperties);
    problemProperties(problemPropertiesNode);

    // parameters
    QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();
    if (parameters.count() > 0)
    {
        QTreeWidgetItem *parametersNode = new QTreeWidgetItem(problemNode);
        parametersNode->setText(0, tr("Parameters"));
        parametersNode->setIcon(0, iconAlphabet('P', AlphabetColor_Green));
        parametersNode->setFont(0, fnt);
        parametersNode->setExpanded(true);

        foreach (QString key, parameters.keys())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(parametersNode);
            item->setText(0, QString("%1 (%2)").arg(key).arg(parameters[key].value()));
            item->setData(0, Qt::UserRole, key);
            item->setData(1, Qt::UserRole, PreprocessorWidget::ModelParameter);
        }
    }

    // functions
    ProblemFunctions *functions = Agros::problem()->config()->functions();
    if (functions->items().count() > 0)
    {
        QTreeWidgetItem *functionNode = new QTreeWidgetItem(problemNode);
        functionNode->setText(0, tr("Functions"));
        functionNode->setIcon(0, iconAlphabet('F', AlphabetColor_Blue));
        functionNode->setFont(0, fnt);
        functionNode->setExpanded(true);

        foreach (ProblemFunction *function, functions->items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(functionNode);
            item->setText(0, function->name());
            // item->setText(1, QString::number(parameters[key]));
            item->setData(0, Qt::UserRole, function->name());
            item->setData(1, Qt::UserRole, PreprocessorWidget::ModelFunction);
        }
    }

    // fields
    QTreeWidgetItem *fieldsNode = new QTreeWidgetItem(trvWidget);
    fieldsNode->setText(0, tr("Fields"));
    fieldsNode->setIcon(0, iconAlphabet('F', AlphabetColor_Brown));
    fieldsNode->setFont(0, fnt);
    fieldsNode->setExpanded(true);

    // field and markers
    foreach (FieldInfo *fieldInfo, Agros::problem()->fieldInfos())
    {
        // field
        QTreeWidgetItem *fieldNode = new QTreeWidgetItem(fieldsNode);
        fieldNode->setText(0, fieldInfo->name());
        fieldNode->setFont(0, fnt);
        // fieldNode->setIcon(0, iconAlphabet(fieldInfo->fieldId().at(0), AlphabetColor_Green));
        fieldNode->setIcon(0, icon("fields/" + fieldInfo->fieldId()));
        fieldNode->setData(0, Qt::UserRole, fieldInfo->fieldId());
        fieldNode->setToolTip(0, fieldPropertiesToString(fieldInfo));        
        fieldNode->setData(1, Qt::UserRole, PreprocessorWidget::FieldProperties);
        fieldNode->setExpanded(true);

        // field properties
        QTreeWidgetItem *fieldPropertiesNode = propertiesItem(fieldNode, tr("Properties"), "", PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
        fieldPropertiesNode->setIcon(0, iconAlphabet('P', AlphabetColor_Green));
        fieldProperties(fieldInfo, fieldPropertiesNode);

        // materials
        QTreeWidgetItem *materialsNode = new QTreeWidgetItem(fieldNode);
        materialsNode->setIcon(0, iconAlphabet('M', AlphabetColor_Red));
        materialsNode->setText(0, tr("Materials"));
        materialsNode->setFont(0, fnt);
        materialsNode->setExpanded(true);

        foreach (SceneMaterial *material, Agros::problem()->scene()->materials->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(materialsNode);

            item->setText(0, material->name());
            if (Agros::problem()->scene()->labels->haveMarker(material).isEmpty())
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, Agros::problem()->scene()->materials->items().indexOf(material));
            item->setData(1, Qt::UserRole, PreprocessorWidget::Material);
        }

        // boundary conditions
        QTreeWidgetItem *boundaryConditionsNode = new QTreeWidgetItem(fieldNode);
        boundaryConditionsNode->setIcon(0, iconAlphabet('B', AlphabetColor_Purple));
        boundaryConditionsNode->setText(0, tr("Boundary conditions"));
        boundaryConditionsNode->setFont(0, fnt);
        boundaryConditionsNode->setExpanded(true);

        foreach (SceneBoundary *boundary, Agros::problem()->scene()->boundaries->filter(fieldInfo).items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(boundaryConditionsNode);

            Module::BoundaryType boundaryType = fieldInfo->boundaryType(boundary->type());

            item->setText(0, QString("%1 (%2)").arg(boundary->name()).arg(boundaryType.name()));
            if (Agros::problem()->scene()->faces->haveMarker(boundary).isEmpty())
                item->setForeground(0, QBrush(Qt::gray));
            item->setData(0, Qt::UserRole, Agros::problem()->scene()->boundaries->items().indexOf(boundary));
            item->setData(1, Qt::UserRole, PreprocessorWidget::Boundary);
        }
    }

    // geometry
    QTreeWidgetItem *geometryNode = new QTreeWidgetItem(trvWidget);
    geometryNode->setText(0, tr("Geometry"));
    geometryNode->setIcon(0, icon("geometry"));
    geometryNode->setFont(0, fnt);
    geometryNode->setExpanded(false);

    // nodes
    QTreeWidgetItem *nodesNode = new QTreeWidgetItem(geometryNode);
    nodesNode->setText(0, tr("Nodes"));
    nodesNode->setIcon(0, iconAlphabet('N', AlphabetColor_Green));
    nodesNode->setFont(0, fnt);

    int inode = 0;
    foreach (SceneNode *node, Agros::problem()->scene()->nodes->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(nodesNode);

        item->setText(0, QString("%1. [%2; %3] ").arg(inode).arg(node->point().x, 0, 'e', 2).arg(node->point().y, 0, 'e', 2));
        item->setData(0, Qt::UserRole, Agros::problem()->scene()->nodes->items().indexOf(node));
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryNode);

        inode++;
    }

    // edges
    QTreeWidgetItem *edgesNode = new QTreeWidgetItem(geometryNode);
    edgesNode->setText(0, tr("Edges"));
    edgesNode->setIcon(0, iconAlphabet('E', AlphabetColor_Green));
    edgesNode->setFont(0, fnt);

    int iface = 0;
    foreach (SceneFace *face, Agros::problem()->scene()->faces->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(edgesNode);

        item->setText(0, QString("%1. %2 m").
                      arg(iface).arg((face->angle() < EPS_ZERO) ? (face->nodeEnd()->point() - face->nodeStart()->point()).magnitude() : face->radius() * face->angle() / 180.0 * M_PI, 0, 'e', 2));
        item->setData(0, Qt::UserRole, Agros::problem()->scene()->faces->items().indexOf(face));
        item->setData(1, Qt::UserRole, PreprocessorWidget::GeometryEdge);

        iface++;
    }

    // labels
    QTreeWidgetItem *labelsNode = new QTreeWidgetItem(geometryNode);
    labelsNode->setText(0, tr("Labels"));
    labelsNode->setIcon(0, iconAlphabet('L', AlphabetColor_Green));
    labelsNode->setFont(0, fnt);

    int ilabel = 0;
    foreach (SceneLabel *label, Agros::problem()->scene()->labels->items())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(labelsNode);

        item->setText(0, QString("%1. [%2; %3]").arg(ilabel).arg(label->point().x, 0, 'e', 2).arg(label->point().y, 0, 'e', 2));
        item->setData(0, Qt::UserRole, Agros::problem()->scene()->labels->items().indexOf(label));
        item->setData(1, Qt::UserRole, GeometryLabel);

        ilabel++;
    }

    // optilab
    QTreeWidgetItem *optilabNode = new QTreeWidgetItem(trvWidget);
    optilabNode->setText(0, tr("OptiLab"));
    optilabNode->setIcon(0, iconAlphabet('O', AlphabetColor_Brown));
    optilabNode->setFont(0, fnt);
    optilabNode->setExpanded(true);

    // recipes
    if (Agros::problem()->recipes()->items().count() > 0)
    {
        QTreeWidgetItem *recipesNode = new QTreeWidgetItem(optilabNode);
        recipesNode->setText(0, tr("Recipes"));
        recipesNode->setFont(0, fnt);
        // recipesNode->setExpanded(true);

        foreach (ResultRecipe *recipe, Agros::problem()->recipes()->items())
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(recipesNode);

            item->setText(0, QString("%1 (%2)").arg(recipe->name()).arg(resultRecipeTypeString(recipe->type())));
            item->setData(0, Qt::UserRole, recipe->name());
            item->setData(1, Qt::UserRole, PreprocessorWidget::OptilabRecipe);
        }
    }

    if (Agros::problem()->studies()->items().count() > 0)
    {
        for (int k = 0; k < Agros::problem()->studies()->items().count(); k++)
        {
            Study *study = Agros::problem()->studies()->items().at(k);

            // study
            QTreeWidgetItem *studyNode = new QTreeWidgetItem(optilabNode);
            studyNode->setText(0, tr("Study - %1").arg(studyTypeString(study->type())));
            studyNode->setIcon(0, iconAlphabet(studyTypeString(study->type()).at(0), AlphabetColor_Green));
            studyNode->setFont(0, fnt);
            studyNode->setData(0, Qt::UserRole, k);
            studyNode->setData(1, Qt::UserRole, PreprocessorWidget::OptilabStudy);
            // studyNode->setExpanded(true);

            // parameters
            QTreeWidgetItem *parametersNode = new QTreeWidgetItem(studyNode);
            parametersNode->setText(0, tr("Parameters"));
            parametersNode->setFont(0, fnt);
            parametersNode->setData(0, Qt::UserRole, k);
            parametersNode->setData(1, Qt::UserRole, PreprocessorWidget::OptilabStudy);
            parametersNode->setExpanded(true);

            foreach (Parameter parameter, study->parameters())
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(parametersNode);

                item->setText(0, QString("%1 (%2 - %3)").arg(parameter.name()).arg(parameter.lowerBound()).arg(parameter.upperBound()));
                item->setData(0, Qt::UserRole, parameter.name());
                item->setData(1, Qt::UserRole, PreprocessorWidget::OptilabParameter);                
                item->setData(2, Qt::UserRole, k);
            }

            // functionals
            QTreeWidgetItem *functionalsNode = new QTreeWidgetItem(studyNode);
            functionalsNode->setText(0, tr("Functionals"));
            functionalsNode->setFont(0, fnt);
            functionalsNode->setData(0, Qt::UserRole, study->variant());
            functionalsNode->setData(1, Qt::UserRole, PreprocessorWidget::OptilabStudy);
            functionalsNode->setExpanded(true);

            foreach (Functional functional, study->functionals())
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(functionalsNode);

                item->setText(0, QString("%1 (%2 %) %3").arg(functional.name()).arg(functional.weight()).
                                 arg((functional.expression().count() < 20) ? functional.expression() : functional.expression().left(20) + "..."));
                item->setData(0, Qt::UserRole, functional.name());
                item->setData(1, Qt::UserRole, PreprocessorWidget::OptilabFunctional);                
                item->setData(2, Qt::UserRole, k);
            }
        }
    }

    trvWidget->resizeColumnToContents(1);
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
        {
            fieldAction->setIcon(icon("fields/" + fieldAction->key()));
            mnuFields->addAction(fieldAction);
        }

    // studies
    mnuStudies->clear();
    foreach(StringAction *studyAction, actNewStudies.values())
        mnuStudies->addAction(studyAction);

    // create menu
    createMenu();
}

void PreprocessorWidget::loadTooltip(SceneGeometryMode sceneMode)
{
    txtViewNodes->setVisible(sceneMode == SceneGeometryMode_OperateOnNodes);
    txtViewEdges->setVisible(sceneMode == SceneGeometryMode_OperateOnEdges);
    txtViewLabels->setVisible(sceneMode == SceneGeometryMode_OperateOnLabels);
}

QString PreprocessorWidget::problemPropertiesToString()
{
    // TODO: rework
    QString html = "<body style=\"font-size: 11px;\">";
    html += QString("<h4>%1</h4>").arg(tr("Problem properties"));
    html += "<table width=\"100%\">";
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Coordinate type:")).arg(coordinateTypeString(Agros::problem()->config()->coordinateType()));
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Mesh type:")).arg(meshTypeString(Agros::problem()->config()->meshType()));
    if (Agros::problem()->isHarmonic())
    {
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Frequency:")).arg(Agros::problem()->config()->value(ProblemConfig::Frequency).value<Value>().toString() + " Hz");
    }
    if (Agros::problem()->isTransient())
    {
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Method:")).arg(timeStepMethodString((TimeStepMethod) Agros::problem()->config()->value(ProblemConfig::TimeMethod).toInt()));
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Order:")).arg(QString::number(Agros::problem()->config()->value(ProblemConfig::TimeOrder).toInt()));
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Tolerance (%):")).arg(Agros::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Initial step size:")).arg(Agros::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Constant time step:")).arg(QString::number(Agros::problem()->config()->constantTimeStepLength()) + " s");
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Number of const. time steps:")).arg(Agros::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
        html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Total time:")).arg(QString::number(Agros::problem()->config()->value(ProblemConfig::TimeTotal).toDouble()) + " s");
    }

    html += "</table>";
    html += "</body>";

    return html;
}

QString PreprocessorWidget::fieldPropertiesToString(FieldInfo *fieldInfo)
{
    // TODO: rework
    QString html = "<body style=\"font-size: 11px;\">";
    html += QString("<h4>%1</h4>").arg(fieldInfo->name());
    html += "<table width=\"100%\">";
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Analysis:")).arg(analysisTypeString(fieldInfo->analysisType()));
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Solver:")).arg(linearityTypeString(fieldInfo->linearityType()));
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Number of refinements:")).arg(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Polynomial order:")).arg(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Adaptivity:")).arg(adaptivityTypeString(fieldInfo->adaptivityType()));
    html += QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(tr("Matrix solver:")).arg(matrixSolverTypeString(fieldInfo->matrixSolver()));
    html += "</table>";
    html += "</body>";

    return html;
}

void PreprocessorWidget::problemProperties(QTreeWidgetItem *item)
{
    propertiesItem(item, tr("Coordinate type"), coordinateTypeString(Agros::problem()->config()->coordinateType()), PreprocessorWidget::ProblemProperties);
    propertiesItem(item, tr("Mesh type"), meshTypeString(Agros::problem()->config()->meshType()), PreprocessorWidget::ProblemProperties);
    if (Agros::problem()->isHarmonic())
    {
        propertiesItem(item, tr("Frequency"), Agros::problem()->config()->value(ProblemConfig::Frequency).value<Value>().toString() + " Hz", PreprocessorWidget::ProblemProperties);
    }
    if (Agros::problem()->isTransient())
    {
        propertiesItem(item, tr("Method"), timeStepMethodString((TimeStepMethod) Agros::problem()->config()->value(ProblemConfig::TimeMethod).toInt()), PreprocessorWidget::ProblemProperties);
        propertiesItem(item, tr("Order"), QString::number(Agros::problem()->config()->value(ProblemConfig::TimeOrder).toInt()), PreprocessorWidget::ProblemProperties);
        propertiesItem(item, tr("Tolerance"), QString::number(Agros::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble()), PreprocessorWidget::ProblemProperties);
        propertiesItem(item, tr("Initial step size"), QString::number(Agros::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble()) + " %", PreprocessorWidget::ProblemProperties);
        propertiesItem(item, tr("Constant time step"), QString::number(Agros::problem()->config()->constantTimeStepLength()) + " s", PreprocessorWidget::ProblemProperties);
        propertiesItem(item, tr("Number of const. time steps"), QString::number(Agros::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt()), PreprocessorWidget::ProblemProperties);
        propertiesItem(item, tr("Total time"), QString::number(Agros::problem()->config()->value(ProblemConfig::TimeTotal).toDouble()) + " s", PreprocessorWidget::ProblemProperties);
    }
}

void PreprocessorWidget::fieldProperties(FieldInfo *fieldInfo, QTreeWidgetItem *item)
{
    propertiesItem(item, tr("Analysis"), analysisTypeString(fieldInfo->analysisType()), PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
    propertiesItem(item, tr("Solver"), linearityTypeString(fieldInfo->linearityType()), PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
    propertiesItem(item, tr("Number of refinements"), QString::number(fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt()), PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
    propertiesItem(item, tr("Polynomial order"), QString::number(fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt()), PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
    propertiesItem(item, tr("Adaptivity"), adaptivityTypeString(fieldInfo->adaptivityType()), PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
    propertiesItem(item, tr("Matrix solver"), matrixSolverTypeString(fieldInfo->matrixSolver()), PreprocessorWidget::FieldProperties, fieldInfo->fieldId());
}

QTreeWidgetItem *PreprocessorWidget::propertiesItem(QTreeWidgetItem *item, const QString &key, const QString &value, PreprocessorWidget::Type type, const QString &data)
{
    QTreeWidgetItem *result = new QTreeWidgetItem(item);
    if (value.isEmpty())
        result->setText(0, QString("%1").arg(key));
    else
        result->setText(0, QString("%1 - %2").arg(key).arg(value));
    result->setForeground(0, QBrush(Qt::darkGray));
    result->setForeground(1, QBrush(Qt::darkGray));
    result->setData(0, Qt::UserRole, data);
    result->setData(1, Qt::UserRole, type);

    return result;
}

void PreprocessorWidget::doContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *current = trvWidget->itemAt(pos);
    doItemChanged(current, NULL);

    if (current)
        trvWidget->setCurrentItem(current);

    actNewRecipeLocalValue->setEnabled(Agros::problem()->fieldInfos().count() > 0);
    actNewRecipeSurfaceIntegral->setEnabled(Agros::problem()->fieldInfos().count() > 0);
    actNewRecipeVolumeIntegral->setEnabled(Agros::problem()->fieldInfos().count() > 0);

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
        else if (type == PreprocessorWidget::OptilabStudy)
        {
            // optilab - study
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }
        else if (type == PreprocessorWidget::OptilabParameter)
        {
            // optilab - parameter
            actProperties->setEnabled(true);
        }
        else if (type == PreprocessorWidget::OptilabFunctional)
        {
            // optilab - parameter
            actProperties->setEnabled(true);
        }
        else if (type == PreprocessorWidget::OptilabRecipe)
        {
            // problem - recipe
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        emit refreshGUI();
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
                SceneNodeDialog *dialog = new SceneNodeDialog(Agros::problem()->scene()->nodes->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()), this, true);
                if (dialog->exec() == QDialog::Accepted)
                {
                    m_sceneViewProblem->refresh();
                    refresh();
                }
            }
            // geometry - face
            if (type == PreprocessorWidget::GeometryEdge)
            {
                SceneFaceDialog *dialog = new SceneFaceDialog(Agros::problem()->scene()->faces->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()), this, true);
                if (dialog->exec() == QDialog::Accepted)
                {
                    m_sceneViewProblem->refresh();
                    refresh();
                }
            }
            // geometry - label
            if (type == PreprocessorWidget::GeometryLabel)
            {
                SceneLabelDialog *dialog = new SceneLabelDialog(Agros::problem()->scene()->labels->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()), this, true);
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
            SceneBoundaryDialog *dialog = new SceneBoundaryDialog(objectBoundary, this);

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
            SceneMaterialDialog *dialog = new SceneMaterialDialog(objectMaterial, this);

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
        else if (type == PreprocessorWidget::OptilabStudy)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            StudyDialog *studyDialog = StudyDialog::factory(study, this);
            if (studyDialog->showDialog() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == PreprocessorWidget::OptilabParameter)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            QString parameter = trvWidget->currentItem()->data(0, Qt::UserRole).toString();

            StudyParameterDialog dialog(study, &study->parameter(parameter));
            if (dialog.exec() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == PreprocessorWidget::OptilabFunctional)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            QString functional = trvWidget->currentItem()->data(0, Qt::UserRole).toString();

            StudyFunctionalDialog dialog(study, &study->functional(functional));
            if (dialog.exec() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == PreprocessorWidget::OptilabRecipe)
        {
            ResultRecipe *recipe = Agros::problem()->recipes()->recipe(trvWidget->currentItem()->data(0, Qt::UserRole).toString());

            RecipeDialog *dialog = nullptr;
            if (LocalValueRecipe *localRecipe = dynamic_cast<LocalValueRecipe *>(recipe))
            {
                dialog = new LocalValueRecipeDialog(localRecipe, this);
            }
            else if (SurfaceIntegralRecipe *surfaceRecipe = dynamic_cast<SurfaceIntegralRecipe *>(recipe))
            {
                dialog = new SurfaceIntegralRecipeDialog(surfaceRecipe, this);
            }
            else if (VolumeIntegralRecipe *volumeRecipe = dynamic_cast<VolumeIntegralRecipe *>(recipe))
            {
                dialog = new VolumeIntegralRecipeDialog(volumeRecipe, this);
            }
            else
                assert(0);

            if (dialog->showDialog() == QDialog::Accepted)
            {
                refresh();
            }
        }

        emit refreshGUI();
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
                Agros::problem()->scene()->nodes->remove(Agros::problem()->scene()->nodes->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()));
                Agros::problem()->scene()->invalidate();
            }
            else if (type == PreprocessorWidget::GeometryEdge)
            {
                Agros::problem()->scene()->faces->remove(Agros::problem()->scene()->faces->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()));
                Agros::problem()->scene()->invalidate();
            }
            else if (type == PreprocessorWidget::GeometryLabel)
            {
                Agros::problem()->scene()->labels->remove(Agros::problem()->scene()->labels->at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt()));
                Agros::problem()->scene()->invalidate();
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

            Agros::problem()->checkAndApplyParameters(parameters);
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
        else if (type == PreprocessorWidget::OptilabStudy)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvWidget->currentItem()->data(0, Qt::UserRole).toInt());
            if (QMessageBox::question(this, tr("Delete"), tr("Study '%1' will be pernamently deleted. Are you sure?").
                                      arg(studyTypeString(study->type())), tr("&Yes"), tr("&No")) == 0)
            {
                Agros::problem()->studies()->removeStudy(study);
            }
        }
        else if (type == PreprocessorWidget::OptilabRecipe)
        {
            // recipe
            ResultRecipe *recipe = Agros::problem()->recipes()->recipe(trvWidget->currentItem()->data(0, Qt::UserRole).toString());

            if (QMessageBox::question(this, tr("Delete"), tr("Recipe '%1' will be pernamently deleted. Are you sure?").
                                      arg(recipe->name()), tr("&Yes"), tr("&No")) == 0)
            {
                Agros::problem()->recipes()->removeRecipe(recipe);
            }
        }

        refresh();

        emit refreshGUI();
    }
}

void PreprocessorWidget::doNewParameter()
{
    ParameterDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        refresh();
        emit refreshGUI();
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
        emit refreshGUI();
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
        emit refreshGUI();
    }
    else
    {
        delete fieldInfo;
    }
}

void PreprocessorWidget::doNewStudy(const QString &name)
{
    // add study
    Study *study = Study::factory(studyTypeFromStringKey(name));

    StudyDialog *studyDialog = StudyDialog::factory(study, this);
    if (studyDialog->showDialog() == QDialog::Accepted)
    {
        Agros::problem()->studies()->addStudy(study);

        refresh();
        emit refreshGUI();
    }
    else
    {
        delete study;
    }
}

void PreprocessorWidget::doNewRecipeLocalValue()
{
    doNewRecipe(ResultRecipeType_LocalValue);
}

void PreprocessorWidget::doNewRecipeSurfaceIntegral()
{
    doNewRecipe(ResultRecipeType_SurfaceIntegral);
}

void PreprocessorWidget::doNewRecipeVolumeIntegral()
{
    doNewRecipe(ResultRecipeType_VolumeIntegral);
}

void PreprocessorWidget::doNewRecipe(ResultRecipeType type)
{
    if (Agros::problem()->fieldInfos().count() > 0)
    {
        ResultRecipe *recipe = ResultRecipe::factory(type);
        recipe->setFieldId(Agros::problem()->fieldInfos().first()->fieldId());

        RecipeDialog *dialog = nullptr;
        if (type == ResultRecipeType_LocalValue)
        {
            dialog = new LocalValueRecipeDialog((LocalValueRecipe *) recipe, this);
        }
        else if (type == ResultRecipeType_SurfaceIntegral)
        {
            dialog = new SurfaceIntegralRecipeDialog((SurfaceIntegralRecipe *) recipe, this);
        }
        else if (type == ResultRecipeType_VolumeIntegral)
        {
            dialog = new VolumeIntegralRecipeDialog((VolumeIntegralRecipe *) recipe, this);
        }
        else
            assert(0);

        if (dialog->showDialog() == QDialog::Accepted)
        {
            Agros::problem()->recipes()->addRecipe(recipe);

            refresh();
            emit refreshGUI();
        }
        else
        {
            delete recipe;
        }
    }
}

void PreprocessorWidget::doApply()
{
    Agros::problem()->config()->setValue(ProblemConfig::GridStep, txtGridStep->text().toDouble());
    Agros::problem()->config()->setValue(ProblemConfig::SnapToGrid, chkSnapToGrid->isChecked());
}

void PreprocessorWidget::doNewNode(const Point &point)
{
    SceneNode *node = new SceneNode(Agros::problem()->scene(),
                                    PointValue(Agros::problem(), point));
    SceneNodeDialog *dialog = new SceneNodeDialog(node, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneNode *nodeAdded = Agros::problem()->scene()->addNode(node);
        Agros::problem()->scene()->invalidate();

        if (nodeAdded == node)
            undoStack()->push(new SceneNodeCommandAdd(node->pointValue()));

        refresh();
        emit refreshGUI();
    }
    else
    {
        delete node;
    }
}

void PreprocessorWidget::doNewEdge()
{
    SceneFace *edge = new SceneFace(Agros::problem()->scene(),
                                    Agros::problem()->scene()->nodes->at(0),
                                    Agros::problem()->scene()->nodes->at(1),
                                    Value(Agros::problem(), 0.0));
    SceneFaceDialog *dialog = new SceneFaceDialog(edge, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneFace *edgeAdded = Agros::problem()->scene()->addFace(edge);
        Agros::problem()->scene()->invalidate();

        refresh();
        emit refreshGUI();

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
    SceneLabel *label = new SceneLabel(Agros::problem()->scene(),
                                       PointValue(Agros::problem(), point),
                                       0.0);
    SceneLabelDialog *dialog = new SceneLabelDialog(label, this, true);

    if (dialog->exec() == QDialog::Accepted)
    {
        SceneLabel *labelAdded = Agros::problem()->scene()->addLabel(label);
        Agros::problem()->scene()->invalidate();

        if (labelAdded == label)
            undoStack()->push(getAddCommand(label));

        refresh();
        emit refreshGUI();
    }
    else
    {
        delete label;
    }
}

void PreprocessorWidget::doNewBoundary(const QString &field)
{
    // first boundary as default
    QList<Module::BoundaryType> boundaryTypes = Agros::problem()->fieldInfo(field)->boundaryTypes();
    assert(boundaryTypes.count() > 1);

    SceneBoundary *boundary = new SceneBoundary(Agros::problem()->scene(),
                                                Agros::problem()->fieldInfo(field),
                                                tr("new boundary"),
                                                boundaryTypes.first().id());
    SceneBoundaryDialog *dialog = new SceneBoundaryDialog(boundary, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        Agros::problem()->scene()->addBoundary(boundary);
        Agros::problem()->scene()->invalidate();

        refresh();
        emit refreshGUI();
    }
    else
    {
        delete boundary;
    }
}

void PreprocessorWidget::doNewMaterial(const QString &field)
{
    SceneMaterial *material = new SceneMaterial(Agros::problem()->scene(),
                                                Agros::problem()->fieldInfo(field),
                                                tr("new material"));

    SceneMaterialDialog *dialog = new SceneMaterialDialog(material, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        Agros::problem()->scene()->addMaterial(material);
        Agros::problem()->scene()->invalidate();

        refresh();
        emit refreshGUI();
    }
    else
    {
        delete material;
    }
}

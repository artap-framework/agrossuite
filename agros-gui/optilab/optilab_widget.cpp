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

#include "optilab_widget.h"

#include "optilab.h"
#include "optilab/parameter.h"
#include "optilab/study.h"
#include "optilab/study_dialog.h"

#include "gui/recipedialog.h"

#include "util/global.h"

QString treeItemToFullPath(QTreeWidgetItem* treeItem)
{
    QString fullPath= treeItem->text(0);

    while (treeItem->parent() != NULL)
    {
        fullPath= treeItem->parent()->text(0) + "/" + fullPath;
        treeItem = treeItem->parent();
    }
    return fullPath;
}

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent), m_optilab(parent)
{
    foreach (QString name, studyTypeStringKeys())
    {
        QString label = studyTypeString(studyTypeFromStringKey(name));

        StringAction* actionStudy = new StringAction(this, name, label);
        connect(actionStudy, SIGNAL(triggered(QString)), this, SLOT(doNewStudy(QString)));
        actNewStudies[name] = actionStudy;
    }

    createControls();

    connect(trvOptilab, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doItemContextMenu(const QPoint &)));
    connect(trvOptilab, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    connect(trvOptilab, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
}

OptiLabWidget::~OptiLabWidget()
{
    // clear studies
    foreach (QAction *action, actNewStudies.values())
        delete action;
    actNewStudies.clear();
}

void OptiLabWidget::createControls()
{
    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(createControlsOptilab());
    // layout->addWidget(createControlsComputation());

    setLayout(layout);
}

QWidget *OptiLabWidget::createControlsOptilab()
{
    QSettings settings;

    actProperties = new QAction(tr("&Properties"), this);
    connect(actProperties, SIGNAL(triggered()), this, SLOT(doItemProperties()));

    actDelete = new QAction(tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(doItemDelete()));

    actDuplicate = new QAction(tr("Duplicate"), this);
    connect(actDuplicate, SIGNAL(triggered()), this, SLOT(doItemDuplicate()));

    actExport = new QAction(tr("Export"), this);
    actExport->setIcon(icon("menu_function"));
    connect(actExport, SIGNAL(triggered(bool)), this, SLOT(exportData()));

    actRunStudy = new QAction(icon("main_solveopt"), tr("Run study"), this);
    actRunStudy->setShortcut(QKeySequence("Alt+S"));
    actRunStudy->setEnabled(false);
    connect(actRunStudy, SIGNAL(triggered()), this, SLOT(solveStudy()));

    // studies
    auto *mnuStudies = new QMenu(tr("New studies"), this);
    foreach(StringAction *studyAction, actNewStudies.values())
        mnuStudies->addAction(studyAction);

    // recipes
    actNewRecipeLocalPointValue = new QAction(icon("results_point"), tr("Point value recipe..."), this);
    actNewRecipeLocalPointValue->setIconVisibleInMenu(true);
    connect(actNewRecipeLocalPointValue, SIGNAL(triggered()), this, SLOT(doNewRecipeLocalValue()));
    actNewRecipeSurfaceIntegral = new QAction(icon("results_surface"), tr("Surface integral recipe..."), this);
    actNewRecipeSurfaceIntegral->setIconVisibleInMenu(true);
    connect(actNewRecipeSurfaceIntegral, SIGNAL(triggered()), this, SLOT(doNewRecipeSurfaceIntegral()));
    actNewRecipeVolumeIntegral = new QAction(icon("results_volume"), tr("Volume integral recipe..."), this);
    actNewRecipeVolumeIntegral->setIconVisibleInMenu(true);
    connect(actNewRecipeVolumeIntegral, SIGNAL(triggered()), this, SLOT(doNewRecipeVolumeIntegral()));

    auto *mnuRecipe = new QMenu(tr("New recipe"), this);
    mnuRecipe->addAction(actNewRecipeLocalPointValue);
    mnuRecipe->addAction(actNewRecipeSurfaceIntegral);
    mnuRecipe->addAction(actNewRecipeVolumeIntegral);

    actNewParameter = new QAction(icon("menu_parameter"), tr("Parameter"), this);
    connect(actNewParameter, SIGNAL(triggered()), this, SLOT(doNewParameter()));
    actNewGoalFunction = new QAction(icon("menu_function"), tr("Goal"), this);
    connect(actNewGoalFunction, SIGNAL(triggered()), this, SLOT(doNewGoalFunction()));

    auto *mnuParametersAndGoal = new QMenu(tr("New parameter or goal function"), this);
    mnuParametersAndGoal->addAction(actNewParameter);
    mnuParametersAndGoal->addAction(actNewGoalFunction);

    mnuOptilab =  new QMenu(tr("Optilab"));
    mnuOptilab->addMenu(mnuStudies);
    mnuOptilab->addMenu(mnuRecipe);
    mnuOptilab->addMenu(mnuParametersAndGoal);
    mnuOptilab->addSeparator();
    mnuOptilab->addAction(actDuplicate);
    mnuOptilab->addAction(actExport);
    mnuOptilab->addSeparator();
    mnuOptilab->addAction(actDelete);
    mnuOptilab->addAction(actProperties);

    toolButtonRecipes = new QToolButton();
    toolButtonRecipes->setText(tr("Recipe"));
    toolButtonRecipes->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolButtonRecipes->setToolTip(tr("New recipe"));
    toolButtonRecipes->addAction(actNewRecipeLocalPointValue);
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
    toolBarLeft->setProperty("os", operatingSystem());
    toolBarLeft->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    toolBarLeft->addWidget(toolButtonStudies);
    toolBarLeft->addWidget(toolButtonRecipes);
    toolBarLeft->addSeparator();
    toolBarLeft->addAction(actNewParameter);
    toolBarLeft->addAction(actNewGoalFunction);

    QStringList headersOptilab;
    trvOptilab = new QTreeWidget(this);
    // trvOptilab->setMinimumWidth(400);
    trvOptilab->setExpandsOnDoubleClick(false);
    trvOptilab->setHeaderHidden(true);
    trvOptilab->setContextMenuPolicy(Qt::CustomContextMenu);
    trvOptilab->setMouseTracking(true);
    trvOptilab->setUniformRowHeights(true);
    trvOptilab->setColumnCount(2);
    trvOptilab->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    trvOptilab->header()->resizeSection(0, 220);
    trvOptilab->header()->setStretchLastSection(true);
    trvOptilab->setIndentation(trvOptilab->indentation() - 2);


    auto *layoutStudies = new QVBoxLayout();
    layoutStudies->setContentsMargins(2, 2, 2, 2);
    layoutStudies->addWidget(toolBarLeft, 0);
    layoutStudies->addWidget(trvOptilab, 3);

    auto *widgetStudies = new QWidget(this);
    widgetStudies->setLayout(layoutStudies);

    return widgetStudies;
}

void OptiLabWidget::refresh()
{
    trvOptilab->blockSignals(true);
    trvOptilab->setUpdatesEnabled(false);

    // selection
    QString selectedItem = "";
    if (trvOptilab->currentItem())
        selectedItem = treeItemToFullPath(trvOptilab->currentItem());

    // clear tree
    trvOptilab->clear();

    QFont fnt = trvOptilab->font();
    fnt.setBold(true);

    // recipes
    auto *recipesNode = new QTreeWidgetItem(trvOptilab);
    recipesNode->setText(0, tr("Recipes"));
    recipesNode->setIcon(0, icon("menu_recipe"));
    recipesNode->setFont(0, fnt);
    recipesNode->setExpanded(true);

    foreach (ResultRecipe *recipe, Agros::problem()->recipes()->items())
    {
        auto *item = new QTreeWidgetItem(recipesNode);

        item->setText(0, QString("%1").arg(recipe->name()));
        item->setText(1, QString("%1").arg(resultRecipeTypeString(recipe->type())));
        if (recipe->type() == ResultRecipeType_LocalPointValue)
            item->setIcon(0, icon("results_point"));
        else if (recipe->type() == ResultRecipeType_SurfaceIntegral)
            item->setIcon(0, icon("results_surface"));
        else if (recipe->type() == ResultRecipeType_VolumeIntegral)
            item->setIcon(0, icon("results_volume"));
        else
            assert(0);
        item->setData(0, Qt::UserRole, recipe->name());
        item->setData(1, Qt::UserRole, OptiLabWidget::OptilabRecipe);
    }

    // optilab
    auto *studiesNode = new QTreeWidgetItem(trvOptilab);
    studiesNode->setText(0, tr("Studies"));
    studiesNode->setIcon(0, icon("menu_study"));
    studiesNode->setFont(0, fnt);
    studiesNode->setExpanded(true);

    for (int k = 0; k < Agros::problem()->studies()->items().count(); k++)
    {
        Study *study = Agros::problem()->studies()->items().at(k);

        // study
        auto *studyNode = new QTreeWidgetItem(studiesNode);
        studyNode->setText(0, QString("%1").arg(studyTypeString(study->type())));
        studyNode->setText(1, study->computationSets().count() > 0 ? tr("Solved") : tr("Not solved"));
        studyNode->setIcon(0, icon("optilab_algorithm"));
        studyNode->setFont(0, fnt);
        studyNode->setData(1, Qt::UserRole, OptiLabWidget::OptilabStudy);
        studyNode->setData(2, Qt::UserRole, k);
        studyNode->setExpanded(true);
        // select first study if empty
        if (selectedItem.isEmpty())
            selectedItem = treeItemToFullPath(studyNode);

        auto studyPropertiesNode = new QTreeWidgetItem(studyNode);
        studyPropertiesNode->setText(0, tr("Properties"));
        studyPropertiesNode->setIcon(0, icon("problem_properties"));
        studyPropertiesNode->setData(1, Qt::UserRole, OptiLabWidget::OptilabStudy);
        studyPropertiesNode->setData(2, Qt::UserRole, k);

        // parameters
        auto *parametersNode = new QTreeWidgetItem(studyNode);
        parametersNode->setText(0, tr("Parameters"));
        parametersNode->setIcon(0, icon("menu_parameter"));
        parametersNode->setFont(0, fnt);
        // parametersNode->setData(1, Qt::UserRole, OptiLabWidget::OptilabStudy);
        parametersNode->setData(2, Qt::UserRole, k);
        parametersNode->setExpanded(true);

        foreach (Parameter parameter, study->parameters())
        {
            auto *item = new QTreeWidgetItem(parametersNode);

            item->setText(0, QString("%1").arg(parameter.name()));
            item->setText(1, QString("%1 - %2").arg(parameter.lowerBound()).arg(parameter.upperBound()));
            item->setData(0, Qt::UserRole, parameter.name());
            item->setData(1, Qt::UserRole, OptiLabWidget::OptilabParameter);
            item->setData(2, Qt::UserRole, k);
        }

        // functionals
        auto *goalsNode = new QTreeWidgetItem(studyNode);
        goalsNode->setText(0, tr("Goal Functions"));
        goalsNode->setIcon(0, icon("menu_function"));
        goalsNode->setFont(0, fnt);
        goalsNode->setData(0, Qt::UserRole, study->variant());
        // functionalsNode->setData(1, Qt::UserRole, OptiLabWidget::OptilabStudy);
        goalsNode->setData(2, Qt::UserRole, k);
        goalsNode->setExpanded(true);

        foreach (GoalFunction goal, study->goalFunctions())
        {
            auto *item = new QTreeWidgetItem(goalsNode);

            item->setText(0, QString("%1").arg(goal.name()));
            item->setText(1, QString("%2 %").arg(goal.weight()));
            item->setData(0, Qt::UserRole, goal.name());
            item->setData(1, Qt::UserRole, OptiLabWidget::OptilabGoalFunction);
            item->setData(2, Qt::UserRole, k);
        }
    }

    trvOptilab->resizeColumnToContents(1);
    trvOptilab->setUpdatesEnabled(true);
    trvOptilab->blockSignals(false);

    // change selection
    if (!selectedItem.isEmpty())
    {
        QTreeWidgetItemIterator it(trvOptilab);
        while (*it)
        {
            QTreeWidgetItem *item = (*it);
            QString itemSelectedTree = treeItemToFullPath(item);
            if (itemSelectedTree == selectedItem)
            {
                item->setExpanded(true);
                item->setSelected(true);

                selectedItem = itemSelectedTree;
                trvOptilab->setCurrentItem(item);
                doItemChanged(item, NULL);

                break;
            }
            ++it;
        }
    }
}

void OptiLabWidget::solveStudy()
{
    // study
    if (trvOptilab->currentItem())
    {
        OptiLabWidget::Type type = (OptiLabWidget::Type) trvOptilab->currentItem()->data(1, Qt::UserRole).toInt();
        if (type == OptiLabWidget::OptilabStudy || type == OptiLabWidget::OptilabGoalFunction || type == OptiLabWidget::OptilabParameter)
        {
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(2, Qt::UserRole).toInt());

            auto *log = new LogOptimizationDialog(study);
            log->show();

            // solve
            study->solve();

            // close dialog
            log->closeLog();

            refresh();
        }
    }
}

void OptiLabWidget::exportData()
{
    // study
    if (trvOptilab->currentItem())
    {
        OptiLabWidget::Type type = (OptiLabWidget::Type) trvOptilab->currentItem()->data(1, Qt::UserRole).toInt();
        if (type == OptiLabWidget::OptilabStudy || type == OptiLabWidget::OptilabGoalFunction || type == OptiLabWidget::OptilabParameter)
        {
            QSettings settings;
            QString dir = settings.value("General/LastDataDir").toString();

            QString selectedFilter;
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), dir, tr("CSV files (*.csv)"), &selectedFilter);
            if (fileName.isEmpty())
            {
                cerr << "Incorrect file name." << endl;
                return;
            }

            QFileInfo fileInfo(fileName);

            // open file for write
            if (fileInfo.suffix().isEmpty())
                fileName = fileName + ".csv";

            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                cerr << "Could not create " + fileName.toStdString() + " file." << endl;
                return;
            }

            settings.setValue("General/LastDataDir", fileInfo.absolutePath());

            QTextStream out(&file);

            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(2, Qt::UserRole).toInt());
            QList<ComputationSet> computationSets = study->computationSets();

            // headers
            for (int i = 0; i < computationSets.size(); i++)
            {
                foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
                {
                    QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
                    foreach (Parameter parameter, study->parameters())
                    {
                        out << parameter.name() + ";";
                    }

                    StringToDoubleMap results = computation->results()->items();
                    foreach (QString key, results.keys())
                    {
                        out << key + ";";
                    }

                    out << "\n";

                    break;
                }
                break;
            }

            // values
            for (int i = 0; i < computationSets.size(); i++)
            {
                foreach (QSharedPointer<Computation> computation, computationSets[i].computations())
                {
                    QMap<QString, ProblemParameter> parameters = computation->config()->parameters()->items();
                    foreach (Parameter parameter, study->parameters())
                    {
                        out << QString::number(parameters[parameter.name()].value()) + ";";
                    }

                    StringToDoubleMap results = computation->results()->items();
                    foreach (QString key, results.keys())
                    {
                        out << QString::number(results[key]) + ";";
                    }

                    out << "\n";
                }
            }
        }
    }
}

void OptiLabWidget::doItemDoubleClicked(QTreeWidgetItem *item, int role)
{
    doItemProperties();
}

void OptiLabWidget::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    actProperties->setEnabled(false);
    actDelete->setEnabled(false);
    actNewParameter->setEnabled(false);
    actNewGoalFunction->setEnabled(false);
    actRunStudy->setEnabled(false);
    actDuplicate->setEnabled(false);
    actExport->setEnabled(false);

    Study *selected = nullptr;
    if (trvOptilab->currentItem())
    {
        OptiLabWidget::Type type = (OptiLabWidget::Type) trvOptilab->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == OptiLabWidget::OptilabStudy)
        {
            // optilab - study
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
            actNewParameter->setEnabled(true);
            actNewGoalFunction->setEnabled(true);

            actDuplicate->setEnabled(true);
            actExport->setEnabled(true);
        }
        else if (type == OptiLabWidget::OptilabParameter)
        {
            // optilab - parameter
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
            actNewParameter->setEnabled(true);
            actNewGoalFunction->setEnabled(true);
        }
        else if (type == OptiLabWidget::OptilabGoalFunction)
        {
            // optilab - functional
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
            actNewParameter->setEnabled(true);
            actNewGoalFunction->setEnabled(true);
        }
        else if (type == OptiLabWidget::OptilabRecipe)
        {
            // optilab - recipe
            actProperties->setEnabled(true);
            actDelete->setEnabled(true);
        }

        // change study
        if (type == OptiLabWidget::OptilabStudy || type == OptiLabWidget::OptilabGoalFunction || type == OptiLabWidget::OptilabParameter)
        {
            selected = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(2, Qt::UserRole).toInt());
        }
    }

    if (selected)
        actRunStudy->setEnabled(true);
    studySelected(selected);
}

void OptiLabWidget::doItemProperties()
{
    if (trvOptilab->currentItem())
    {
        OptiLabWidget::Type type = (OptiLabWidget::Type) trvOptilab->currentItem()->data(1, Qt::UserRole).toInt();
        if (type == OptiLabWidget::OptilabStudy)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
            StudyDialog *studyDialog = StudyDialog::factory(study, this);
            if (studyDialog->showDialog() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == OptiLabWidget::OptilabParameter)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
            QString parameter = trvOptilab->currentItem()->data(0, Qt::UserRole).toString();

            StudyParameterDialog dialog(study, &study->parameter(parameter));
            if (dialog.exec() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == OptiLabWidget::OptilabGoalFunction)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
            QString goal = trvOptilab->currentItem()->data(0, Qt::UserRole).toString();

            StudyGoalFunctionDialog dialog(study, &study->goal(goal));
            if (dialog.exec() == QDialog::Accepted)
            {
                refresh();
            }
        }
        else if (type == OptiLabWidget::OptilabRecipe)
        {
            ResultRecipe *recipe = Agros::problem()->recipes()->recipe(trvOptilab->currentItem()->data(0, Qt::UserRole).toString());

            RecipeDialog *dialog = nullptr;
            if (auto *localRecipe = dynamic_cast<LocalValueRecipe *>(recipe))
            {
                dialog = new LocalValueRecipeDialog(localRecipe, this);
            }
            else if (auto *surfaceRecipe = dynamic_cast<SurfaceIntegralRecipe *>(recipe))
            {
                dialog = new SurfaceIntegralRecipeDialog(surfaceRecipe, this);
            }
            else if (auto *volumeRecipe = dynamic_cast<VolumeIntegralRecipe *>(recipe))
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
    }
}

void OptiLabWidget::doItemDelete()
{
    if (trvOptilab->currentItem())
    {
        OptiLabWidget::Type type = (OptiLabWidget::Type) trvOptilab->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == OptiLabWidget::OptilabStudy)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(2, Qt::UserRole).toInt());
            if (QMessageBox::question(this, tr("Delete"), tr("Study '%1' will be pernamently deleted. Are you sure?").
                                      arg(studyTypeString(study->type())), tr("&Yes"), tr("&No")) == 0)
            {
                Agros::problem()->studies()->removeStudy(study);
            }
        }
        else if (type == OptiLabWidget::OptilabParameter)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
            QString parameter = trvOptilab->currentItem()->data(0, Qt::UserRole).toString();

            study->removeParameter(parameter);
            refresh();
        }
        else if (type == OptiLabWidget::OptilabGoalFunction)
        {
            // study
            Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
            QString goal = trvOptilab->currentItem()->data(0, Qt::UserRole).toString();

            study->removeGoalFunction(goal);
            refresh();
        }
        else if (type == OptiLabWidget::OptilabRecipe)
        {
            // recipe
            ResultRecipe *recipe = Agros::problem()->recipes()->recipe(trvOptilab->currentItem()->data(0, Qt::UserRole).toString());

            if (QMessageBox::question(this, tr("Delete"), tr("Recipe '%1' will be pernamently deleted. Are you sure?").
                                      arg(recipe->name()), tr("&Yes"), tr("&No")) == 0)
            {
                Agros::problem()->recipes()->removeRecipe(recipe);
            }
        }

        refresh();
    }
}

void OptiLabWidget::doItemDuplicate()
{
    if (trvOptilab->currentItem())
    {
        OptiLabWidget::Type type = (OptiLabWidget::Type) trvOptilab->currentItem()->data(1, Qt::UserRole).toInt();

        if (type == OptiLabWidget::OptilabStudy)
        {
            // study
            Study *studyOriginal = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(2, Qt::UserRole).toInt());

            // select study dialog
            StudySelectDialog dialog(this);
            if (dialog.exec() == QDialog::Accepted)
            {
                // add study
                if (dialog.selectedStudyType() != StudyType_Undefined)
                {
                    Study *study = Study::factory(dialog.selectedStudyType());

                    // copy parameters
                    foreach (Parameter parameter, studyOriginal->parameters())
                        study->addParameter(Parameter(parameter.name(), parameter.lowerBound(), parameter.upperBound()));

                    // copy goals
                    foreach (GoalFunction goal, studyOriginal->goalFunctions())
                        study->addGoalFunction(GoalFunction(goal.name(), goal.expression(), goal.weight()));

                    // clear and solve
                    study->setValue(Study::General_ClearSolution, studyOriginal->value(Study::General_ClearSolution).toBool());
                    study->setValue(Study::General_SolveProblem, studyOriginal->value(Study::General_SolveProblem).toBool());

                    StudyDialog *studyDialog = StudyDialog::factory(study, this);
                    if (studyDialog->showDialog() == QDialog::Accepted)
                    {
                        Agros::problem()->studies()->addStudy(study);
                        refresh();
                    }
                    else
                    {
                        delete study;
                    }
                }
            }
        }
    }
}

void OptiLabWidget::doItemContextMenu(const QPoint &pos)
{
    actNewRecipeLocalPointValue->setEnabled(Agros::problem()->fieldInfos().count() > 0);
    actNewRecipeSurfaceIntegral->setEnabled(Agros::problem()->fieldInfos().count() > 0);
    actNewRecipeVolumeIntegral->setEnabled(Agros::problem()->fieldInfos().count() > 0);

    auto *current = trvOptilab->itemAt(pos);
    doItemChanged(current, NULL);

    mnuOptilab->exec(QCursor::pos());
}

void OptiLabWidget::doNewParameter()
{
    Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
    if (study)
    {
        // select parameter dialog
        ParameterSelectDialog dialog(study, this);
        if (dialog.exec() == QDialog::Accepted)
        {
            // add parameter
            QString name = dialog.selectedParameterName();
            if (!name.isEmpty())
            {
                Parameter parameter(name);
                StudyParameterDialog dialog(study, &parameter);
                if (dialog.exec() == QDialog::Accepted)
                {
                    study->addParameter(parameter);
                    refresh();
                }
            }
        }
    }
}

void OptiLabWidget::doNewGoalFunction()
{
    Study *study = Agros::problem()->studies()->items().at(trvOptilab->currentItem()->data(0, Qt::UserRole).toInt());
    if (study)
    {
        GoalFunction goal;

        StudyGoalFunctionDialog dialog(study, &goal);
        if (dialog.exec() == QDialog::Accepted)
        {
            study->addGoalFunction(goal);
            refresh();
        }
    }
}

void OptiLabWidget::doNewStudy(const QString &name)
{
    // add study
    Study *study = Study::factory(studyTypeFromStringKey(name));

    StudyDialog *studyDialog = StudyDialog::factory(study, this);
    if (studyDialog->showDialog() == QDialog::Accepted)
    {
        Agros::problem()->studies()->addStudy(study);

        refresh();
    }
    else
    {
        delete study;
    }
}

void OptiLabWidget::doNewRecipeLocalValue()
{
    doNewRecipe(ResultRecipeType_LocalPointValue);
}

void OptiLabWidget::doNewRecipeSurfaceIntegral()
{
    doNewRecipe(ResultRecipeType_SurfaceIntegral);
}

void OptiLabWidget::doNewRecipeVolumeIntegral()
{
    doNewRecipe(ResultRecipeType_VolumeIntegral);
}

void OptiLabWidget::doNewRecipe(ResultRecipeType type)
{
    if (Agros::problem()->fieldInfos().count() > 0)
    {
        ResultRecipe *recipe = ResultRecipe::factory(type);
        recipe->setFieldId(Agros::problem()->fieldInfos().first()->fieldId());

        RecipeDialog *dialog = nullptr;
        if (type == ResultRecipeType_LocalPointValue)
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
        }
        else
        {
            delete recipe;
        }
    }
}
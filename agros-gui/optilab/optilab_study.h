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

#ifndef OPTILAB_WIDGET_H
#define OPTILAB_WIDGET_H

#include "util/util.h"
#include "util/enums.h"
#include "util/point.h"
#include "gui/other.h"
#include "gui/chart.h"

class OptiLab;
class Study;
class Computation;
class SceneViewSimpleGeometry;

class OptiLabStudy : public QWidget
{
    Q_OBJECT
public:
    OptiLabStudy(QWidget *parent = 0);
    ~OptiLabStudy();

    QAction *actSceneModeOptiLabStudy;

    QAction *actRunStudy;

public slots:
    void refresh();

private:
    enum Type
    {
        Undefined = 0,
        OptilabStudy,
        OptilabParameter,
        OptilabGoalFunction,
        OptilabRecipe,
    };

    OptiLab *m_optilab;

    QToolBar *toolBarLeft;
    QToolButton *toolButtonStudies;
    QToolButton *toolButtonRecipes;
    QTreeWidget *trvOptilab;

    // ser
    QMenu *mnuOptilab;
    QMap<QString, StringAction *> actNewStudies;
    QAction *actNewRecipeLocalPointValue;
    QAction *actNewRecipeSurfaceIntegral;
    QAction *actNewRecipeVolumeIntegral;
    QAction *actNewParameter;
    QAction *actNewGoalFunction;

    QAction *actProperties;
    QAction *actDelete;
    QAction *actDuplicate;

    void createControls();
    QWidget *createControlsOptilab();

signals:
    void studySelected(Study *study);
    void studySolved(Study *study);

private slots:
    void doItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemProperties();
    void doItemDelete();
    void doItemDuplicate();
    void doItemContextMenu(const QPoint &pos);
    void exportData();

    void doNewRecipeLocalValue();
    void doNewRecipeSurfaceIntegral();
    void doNewRecipeVolumeIntegral();
    void doNewRecipe(ResultRecipeType type);
    void doNewStudy(const QString &key);
    void doNewParameter();
    void doNewGoalFunction();

    void solveStudy();
};

#endif //OPTILAB_WIDGET_H

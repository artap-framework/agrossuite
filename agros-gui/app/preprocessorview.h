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

#ifndef PREPROCESSORVIEW_H
#define PREPROCESSORVIEW_H

#include "util/util.h"
#include "util/point.h"
#include "util/enums.h"
#include "gui/other.h"

class SceneViewProblem;
class FieldInfo;

class PreprocessorWidget : public QWidget
{
    Q_OBJECT

signals:
    void changed();

public slots:    
    void refresh();
    void loadTooltip(SceneGeometryMode sceneMode);

    void doProperties();
    void doDelete();

    void doNewParameter();
    void doNewFunctionAnalytic();
    void doNewFunctionInterpolation();

public:
    PreprocessorWidget(QWidget *parent = 0);
    ~PreprocessorWidget();

    inline SceneViewProblem *sceneViewProblem() { return m_sceneViewProblem; }

    QAction *actUndo;
    QAction *actRedo;

    QToolBar *toolBarRight;
    QToolBar *toolBarLeft;

    QAction *actNewNode;
    QAction *actNewEdge;
    QAction *actNewLabel;
    QAction *actNewRectangle;
    QAction *actNewCircle;

    QMap<QString, StringAction *> actNewFields;
    QMap<QString, StringAction *> actNewBoundaries;
    QMap<QString, StringAction *> actNewMaterials;

    QMenu *mnuFields;
    QMenu *mnuBoundaries;
    QMenu *mnuMaterials;

protected:
    void keyPressEvent(QKeyEvent *event);

private:    
    enum Type
    {
        Undefined = 0,
        ModelParameter,
        ModelFunction,
        GeometryNode,
        GeometryEdge,
        GeometryLabel,
        Material,
        Boundary,
        ProblemProperties,
        FieldProperties
    };

    SceneViewProblem *m_sceneViewProblem;

    QTextEdit *txtViewNodes;
    QTextEdit *txtViewEdges;
    QTextEdit *txtViewLabels;
    QSplitter *splitter;

    QTreeWidget *trvWidget;
    QTreeWidgetItem *problemNode;
    QTreeWidgetItem *parametersNode;
    QTreeWidgetItem *functionNode;
    QTreeWidgetItem *fieldsNode;
    QTreeWidgetItem *geometryNode;
    QTreeWidgetItem *nodesNode;
    QTreeWidgetItem *edgesNode;
    QTreeWidgetItem *labelsNode;

    QAction *actProperties;
    QAction *actDelete;
    QAction *actClear;
    QAction *actNewParameter;
    QAction *actNewFunctionAnalytic;
    // QAction *actNewFunctionInterpolation;

    QAction *actExportGeometryToClipboard;
    QAction *actExportGeometryToSvg;
    QAction *actExportGeometryToBitmap;
    QAction *actExportGeometryToVTK;
    QAction *actImportGeometryFromDXF;
    QAction *actExportGeometryToDXF;

    QToolButton *toolButtonMaterials;
    QToolButton *toolButtonBoundaries;

    QMenu *mnuPreprocessor;

    QLineEdit *txtGridStep;
    QCheckBox *chkSnapToGrid;

    void createActions();
    void createControls();
    void createMenu();

    QString problemPropertiesToString();
    QString fieldPropertiesToString(FieldInfo *fieldInfo);

private slots:
    void doContextMenu(const QPoint &pos);
    void doItemDoubleClicked(QTreeWidgetItem *item, int role);
    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doApply();

    void doNewField(const QString &field);

    void doNewNode(const Point &point = Point());
    void doNewEdge();
    void doNewLabel(const Point &point = Point());
    void doNewRectangle();
    void doNewCircle();

    void doNewBoundary(const QString &field);
    void doNewMaterial(const QString &field);

    void exportGeometryToClipboard();
    void exportGeometryToSvg();
    void exportGeometryToBitmap();
    void exportGeometryToVTK();
    void importGeometryFromDXF();
    void exportGeometryToDXF();
};

#endif // PREPROCESSORVIEW_H

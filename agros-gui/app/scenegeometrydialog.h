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

#ifndef SCENEGEOMETRYDIALOG_H
#define SCENEGEOMETRYDIALOG_H

#include "util/util.h"
#include "gui/other.h"
#include "solver/marker.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

class LineEditDouble;
class ValueLineEdit;

// basic *************************************************************************************************************************************

class SceneBasicDialog : public QDialog
{
    Q_OBJECT

public:
    SceneBasicDialog(QWidget *parent, bool isNew = false);

protected:
    bool m_isNew;
    SceneBasic *m_object;
    QDialogButtonBox *buttonBox;

    virtual QLayout *createContent() = 0;
    void createControls();

    virtual bool load() = 0;
    virtual bool save() = 0;

protected slots:
    void evaluated(bool isError);

private:

    QVBoxLayout *layout;

private slots:
    void doAccept();
    void doReject();
};

// *************************************************************************************************************************************

//TODO general undo framework should reduce code repetition.... TODO

template <typename BasicType>
class SceneBasicTrace
{
public:
    virtual void save(const BasicType& original) = 0;
    virtual void load(BasicType& destination) const = 0;
    virtual void remove() const = 0;
};

template <typename MarkedBasicType>
class MarkedSceneBasicTrace
{
    void saveMarkers(const MarkedBasicType& original);
    void loadMarkers(MarkedBasicType& destination) const;
};

template <typename BasicType, typename BasicHistoryType>
class SceneUndoCommand : public QUndoCommand
{

};

template <typename BasicType, typename BasicHistoryType>
class SceneCommandAdd : public SceneUndoCommand<BasicType, BasicHistoryType>
{
public:
    SceneCommandAdd();
    void undo();
    void redo();
};

// node *********************************************************************************************************************

class SceneNodeDialog : public SceneBasicDialog
{
    Q_OBJECT

public:
    SceneNodeDialog(SceneNode *node, QWidget *parent, bool isNew = false);
    ~SceneNodeDialog();

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    QLabel *lblDistance;
    QLabel *lblAngle;

private slots:
    void doEditingFinished();
};

// undo framework *******************************************************************************************************************

class SceneNodeCommandAdd : public QUndoCommand
{
public:
    SceneNodeCommandAdd(const PointValue &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
};

class SceneNodeCommandRemove : public QUndoCommand
{
public:
    SceneNodeCommandRemove(const PointValue &point, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
};

class SceneNodeCommandEdit : public QUndoCommand
{
public:
    SceneNodeCommandEdit(const PointValue &point, const PointValue &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
    PointValue m_pointNew;
};

class SceneNodeCommandMoveMulti : public QUndoCommand
{
public:
    SceneNodeCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    static void moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo);

    QList<PointValue> m_points;
    QList<PointValue> m_pointsNew;
};

class SceneNodeCommandAddMulti : public QUndoCommand
{
public:
    SceneNodeCommandAddMulti(QList<PointValue> points,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    QList<PointValue> m_points;
};

class SceneNodeCommandRemoveMulti : public QUndoCommand
{
public:
    SceneNodeCommandRemoveMulti(QList<PointValue> points,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    // nodes
    QList<PointValue> m_nodePoints;
    // edges
    QList<Point> m_edgePointStart;
    QList<Point> m_edgePointEnd;
    QList<QMap<QString, QString> > m_edgeMarkers;
    QList<Value> m_edgeAngle;
};

// face *********************************************************************************************************************

class SceneFaceMarker : public QGroupBox
{
    Q_OBJECT

public:
    SceneFaceMarker(SceneFace *edge, FieldInfo *fieldInfo, QWidget *parent);

    void load();
    bool save();
    void fillComboBox();

private:
    FieldInfo *m_fieldInfo;
    SceneFace *m_face;

    QComboBox *cmbBoundary;
    QPushButton *btnBoundary;

private slots:
    void doBoundaryChanged(int index);
    void doBoundaryClicked();
};

class SceneFaceDialog : public SceneBasicDialog
{
    Q_OBJECT

public:
    SceneFaceDialog(SceneFace *edge, QWidget *parent, bool isNew = false);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QLabel *lblEquation;
    ValueLineEdit *txtAngle;
    QSpinBox *txtSegments;
    QLabel *lblLength;

    QComboBox *cmbNodeStart;
    QComboBox *cmbNodeEnd;

    QList<SceneFaceMarker *> m_faceMarkers;

    void fillComboBox();

private slots:
    void nodeChanged();
    void swap();
    void angleChanged();
};

class SceneEdgeSelectDialog : public QDialog
{
    Q_OBJECT

public:
    SceneEdgeSelectDialog(MarkedSceneBasicContainer<SceneBoundary, SceneFace> edges, QWidget *parent);

protected:
    void load();
    bool save();

private:
    MarkedSceneBasicContainer<SceneBoundary, SceneFace> m_edges;
    QMap<FieldInfo*, QComboBox *> cmbBoundaries;

    void fillComboBox();

private slots:
    void doAccept();
    void doReject();
};

// undo framework *******************************************************************************************************************

class SceneFaceCommandAdd : public QUndoCommand
{
public:
    SceneFaceCommandAdd(const PointValue &pointStart, const PointValue &pointEnd, const QMap<QString, QString> &markers,
                        const Value &angle, int segments, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_pointStart;
    PointValue m_pointEnd;
    QMap<QString, QString> m_markers;
    Value m_angle;
    int m_segments;
};

class SceneEdgeCommandAddOrRemoveMulti : public QUndoCommand
{
public:
    SceneEdgeCommandAddOrRemoveMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                        QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QUndoCommand *parent = 0);
    void add();
    void remove();

private:
    QList<PointValue> m_pointStarts;
    QList<PointValue> m_pointEnds;
    QList<Value> m_angles;
    QList<int> m_segments;    
    QList<QMap<QString, QString> > m_markers;
};

class SceneEdgeCommandRemoveMulti : public SceneEdgeCommandAddOrRemoveMulti
{
public:
    SceneEdgeCommandRemoveMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                                QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QUndoCommand *parent = 0)
        : SceneEdgeCommandAddOrRemoveMulti(pointStarts, pointEnds, markers, angles, segments, parent) {}

    void undo() { add(); }
    void redo() { remove(); }
};

class SceneEdgeCommandAddMulti : public SceneEdgeCommandAddOrRemoveMulti
{
public:
    SceneEdgeCommandAddMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                                QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QUndoCommand *parent = 0)
        : SceneEdgeCommandAddOrRemoveMulti(pointStarts, pointEnds, markers, angles, segments, parent) {}

    void undo() { remove(); }
    void redo() { add(); }
};

class SceneFaceCommandRemove : public QUndoCommand
{
public:
    SceneFaceCommandRemove(const PointValue &pointStart, const PointValue &pointEnd, const QMap<QString, QString> &markers,
                           const Value &angle, int segments, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_pointStart;
    PointValue m_pointEnd;
    QMap<QString, QString> m_markers;
    Value m_angle;
    int m_segments;
};

class SceneEdgeCommandEdit : public QUndoCommand
{
public:
    SceneEdgeCommandEdit(const PointValue &pointStart, const PointValue &pointEnd, const PointValue &pointStartNew, const PointValue &pointEndNew,
                         const Value &angle, const Value &angleNew, int segments, int segmentsNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_pointStart;
    PointValue m_pointEnd;
    PointValue m_pointStartNew;
    PointValue m_pointEndNew;
    Value m_angle;
    Value m_angleNew;
    int m_segments;
    int m_segmentsNew;
};

// label *********************************************************************************************************************

class SceneLabelMarker : public QGroupBox
{
    Q_OBJECT

public:
    SceneLabelMarker(SceneLabel *label, FieldInfo *fieldInfo, QWidget *parent);

    void load();
    bool save();
    void fillComboBox();

private:
    FieldInfo *m_fieldInfo;
    SceneLabel *m_label;

    QComboBox *cmbMaterial;
    QPushButton *btnMaterial;

    QSpinBox *txtAreaRefinement;
    QCheckBox *chkAreaRefinement;

    QSpinBox *txtPolynomialOrder;
    QCheckBox *chkPolynomialOrder;

private slots:
    void doMaterialChanged(int index);
    void doMaterialClicked();
    void doAreaRefinement(int);
    void doPolynomialOrder(int);
};

class SceneLabelDialog : public SceneBasicDialog
{
    Q_OBJECT

public:
    SceneLabelDialog(SceneLabel *label, QWidget *parent, bool isNew = false);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    ValueLineEdit *txtPointX;
    ValueLineEdit *txtPointY;
    ValueLineEdit *txtArea;
    QCheckBox *chkArea;

    QList<SceneLabelMarker *> m_labelMarkers;

    void fillComboBox();

private slots:
    void doArea(int);
};

class SceneLabelSelectDialog : public QDialog
{
    Q_OBJECT

public:
    SceneLabelSelectDialog(MarkedSceneBasicContainer<SceneMaterial, SceneLabel> labels, QWidget *parent);

protected:
    void load();
    bool save();

private:
    MarkedSceneBasicContainer<SceneMaterial, SceneLabel> m_labels;
    QMap<const FieldInfo*, QComboBox *> cmbMaterials;

    void fillComboBox();

private slots:
    void doAccept();
    void doReject();
};

// undo framework *******************************************************************************************************************

class SceneLabelCommandAdd : public QUndoCommand
{
public:
    SceneLabelCommandAdd(const PointValue &pointValue, const QMap<QString, QString> &markers, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;

    QMap<QString, QString> m_markers;
    double m_area;
};

class SceneLabelCommandRemove : public QUndoCommand
{
public:
    SceneLabelCommandRemove(const PointValue &point, const QMap<QString, QString> &markers, double area, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;

    QMap<QString, QString> m_markers;
    double m_area;
};

class SceneLabelCommandEdit : public QUndoCommand
{
public:
    SceneLabelCommandEdit(const PointValue &point, const PointValue &pointNew,  QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_point;
    PointValue m_pointNew;
};

class SceneLabelCommandAddOrRemoveMulti : public QUndoCommand
{
public:
    SceneLabelCommandAddOrRemoveMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers, QList<double> areas, QUndoCommand *parent = 0);
    void add();
    void remove();

private:
    // nodes
    QList<PointValue> m_points;
    QList<QMap<QString, QString> > m_markers;
    QList<double> m_areas;
};

class SceneLabelCommandAddMulti : public SceneLabelCommandAddOrRemoveMulti
{
public:
    SceneLabelCommandAddMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers,  QList<double> areas, QUndoCommand *parent = 0) :
        SceneLabelCommandAddOrRemoveMulti(points, markers, areas, parent) {}

    void undo() { remove(); }
    void redo() { add(); }
};

class SceneLabelCommandRemoveMulti : public SceneLabelCommandAddOrRemoveMulti
{
public:
    SceneLabelCommandRemoveMulti(QList<PointValue> points, QList<QMap<QString, QString> > markers,  QList<double> areas, QUndoCommand *parent = 0) :
        SceneLabelCommandAddOrRemoveMulti(points, markers, areas, parent) {}

    void undo() { add(); }
    void redo() { remove(); }
};

class SceneLabelCommandMoveMulti : public QUndoCommand
{
public:
    SceneLabelCommandMoveMulti(QList<PointValue> points, QList<PointValue> pointsNew, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    static void moveAll(QList<PointValue> moveFrom, QList<PointValue> moveTo);

    QList<PointValue> m_points;
    QList<PointValue> m_pointsNew;
};

#endif // SCENEGEOMETRYDIALOG_H

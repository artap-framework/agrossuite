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

#ifndef SCENEEDGE_H
#define SCENEEDGE_H

#include "util.h"
#include "scenebasic.h"
#include "scenemarker.h"

class Scene;
class SceneFaceCommandAdd;
class SceneFaceCommandRemove;

class AGROS_LIBRARY_API SceneFace : public MarkedSceneBasic<SceneBoundary>
{
public:
    SceneFace(Scene *scene, SceneNode *nodeStart, SceneNode *nodeEnd, const Value &angle, int segments = 3, bool isCurvilinear = true);

    inline SceneNode *nodeStart() const { return m_nodeStart; }
    inline void setNodeStart(SceneNode *nodeStart) { m_nodeStart = nodeStart; computeCenterAndRadius(); }
    inline SceneNode *nodeEnd() const { return m_nodeEnd; }
    inline void setNodeEnd(SceneNode *nodeEnd) { m_nodeEnd = nodeEnd; computeCenterAndRadius(); }
    inline double angle() const { return m_angle.number(); }
    inline Value angleValue() const { return m_angle; }    
    inline void setAngleValue(const Value &angle) { m_angle = angle; computeCenterAndRadius(); }
    void swapDirection();

    QList<SceneNode *> lyingNodes() const;

    bool isLyingOnNode(const SceneNode *node) const;
    bool isLyingOnPoint(const Point &point) const;
    bool hasLyingNode() const;
    bool isOutsideArea() const;
    bool isError() const;
    bool isCrossed() const; // very slow, use carefully

    inline Point center() const { return m_centerCache; }
    inline double radius() const { return m_radiusCache; }
    inline Point vector() const { return m_vectorCache; }
    double distance(const Point &point) const;    
    double length() const;
    bool isStraight() const { return (fabs(angle()) < EPS_ZERO); }

    // needed by mesh generator
    inline int segments() const { return m_segments; }
    void setSegments(int segments);
    inline bool isCurvilinear() const { return m_isCurvilinear; }
    void setCurvilinear(bool isCurvilinear);

    // SceneFaceCommandAdd* getAddCommand();
    // SceneFaceCommandRemove* getRemoveCommand();

    int showDialog(QWidget *parent, bool isNew = false);

    static SceneFace *findClosestFace(Scene *scene, const Point &point);

    void addMarkersFromStrings(QMap<QString, QString> markers);

    // for boundary edges returns idx of the only adjacent label
    // for inner edges returns, by convention, the left hand side label
    // used for sufrace integration and drawing of outer normal
    // for convex label regions, this is the other halfplain than the one, where the label coordinates are
    // for non-convex regions it is more complicated!
    int innerLabelIdx(const FieldInfo* fieldInfo) const;

    // used for drawing outer normal, since we want to draw one outer normal for all fields
    // if previous function has for all fieldInfos value X or MARKER_IDX_NOT_EXISTING, returns X
    // otherwise returns MARKER_IDX_NOT_EXISTING and outer normal may not be drawn reasonably.
    // this happends only if this edge divides label which has material A and not material B with
    // label which has material B and not material A. In such a case, universal outer normal does not exist
    int innerLabelIdx() const;

    //sets right and left label idx to MARKER_IDX_NOT_EXISTING
    void unsetRightLeftLabelIdx();

    // first right and left label marker should be set to MARKER_IDX_NOT_EXISTING by previous function
    // than first call to this function will set thel leftLabelIdx. The second call will set rightLabelIdx
    // third call will raise exception
    void addNeighbouringLabel(int idx);

    int leftLabelIdx() const { return m_leftLabelIdx; }
    int rightLabelIdx() const { return m_rightLabelIdx; }
    void setLeftLabelIdx(int idx) { m_leftLabelIdx = idx; }
    void setRightLabelIdx(int idx) { m_rightLabelIdx = idx; }
    SceneLabel *leftLabel() const;
    SceneLabel *rightLabel() const;

private:
    SceneNode *m_nodeStart;
    SceneNode *m_nodeEnd;
    Value m_angle;
    int m_segments;
    bool m_isCurvilinear;

    // cache
    Point m_centerCache;
    double m_radiusCache;
    Point m_vectorCache;
    void computeCenterAndRadius();

    friend class SceneNode;
    friend class Scene;

    int m_leftLabelIdx;
    int m_rightLabelIdx;
};

Q_DECLARE_METATYPE(SceneFace *)

// *************************************************************************************************************************************

class SceneFaceContainer : public MarkedSceneBasicContainer<SceneBoundary, SceneFace>
{
public:
    void removeConnectedToNode(SceneNode* node);

    /// if container contains the same edge, returns it. Otherwise returns NULL
    SceneFace* get(SceneFace* edge) const;

    /// returns corresponding edge or NULL
    SceneFace* get(const Point &pointStart, const Point &pointEnd, double angle, int segments, bool isCurvilinear) const;

    /// returns corresponding edge or NULL
    SceneFace* get(const Point &pointStart, const Point &pointEnd) const;

    /// returns bounding box, assumes container not empty
    RectPoint boundingBox() const;
    static RectPoint boundingBox(QList<SceneFace *> edges);
};

// *************************************************************************************************************************************

/*
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
    SceneFaceDialog(SceneFace *edge, QWidget *parent, bool m_isNew);

protected:
    QLayout *createContent();

    bool load();
    bool save();

private:
    QLabel *lblEquation;
    ValueLineEdit *txtAngle;
    QSpinBox *txtSegments;
    QCheckBox *chkIsCurvilinear;
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
                        const Value &angle, int segments, bool isCurvilinear, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_pointStart;
    PointValue m_pointEnd;
    QMap<QString, QString> m_markers;
    Value m_angle;
    int m_segments;
    bool m_isCurvilinear;
};

class SceneEdgeCommandAddOrRemoveMulti : public QUndoCommand
{
public:
    SceneEdgeCommandAddOrRemoveMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                        QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QList<bool> isCurvilinear, QUndoCommand *parent = 0);
    void add();
    void remove();

private:
    QList<PointValue> m_pointStarts;
    QList<PointValue> m_pointEnds;
    QList<Value> m_angles;
    QList<int> m_segments;
    QList<bool> m_isCurvilinear;
    QList<QMap<QString, QString> > m_markers;
};

class SceneEdgeCommandRemoveMulti : public SceneEdgeCommandAddOrRemoveMulti
{
public:
    SceneEdgeCommandRemoveMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                                QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QList<bool> isCurvilinear, QUndoCommand *parent = 0)
        : SceneEdgeCommandAddOrRemoveMulti(pointStarts, pointEnds, markers, angles, segments, isCurvilinear, parent) {}

    void undo() { add(); }
    void redo() { remove(); }
};

class SceneEdgeCommandAddMulti : public SceneEdgeCommandAddOrRemoveMulti
{
public:
    SceneEdgeCommandAddMulti(QList<PointValue> pointStarts, QList<PointValue> pointEnds,
                                QList<QMap<QString, QString> > markers, QList<Value> angles, QList<int> segments, QList<bool> isCurvilinear, QUndoCommand *parent = 0)
        : SceneEdgeCommandAddOrRemoveMulti(pointStarts, pointEnds, markers, angles, segments, isCurvilinear, parent) {}

    void undo() { remove(); }
    void redo() { add(); }
};

class SceneFaceCommandRemove : public QUndoCommand
{
public:
    SceneFaceCommandRemove(const PointValue &pointStart, const PointValue &pointEnd, const QMap<QString, QString> &markers,
                           const Value &angle, int segments, bool isCurvilinear, QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    PointValue m_pointStart;
    PointValue m_pointEnd;
    QMap<QString, QString> m_markers;
    Value m_angle;
    int m_segments;
    bool m_isCurvilinear;
};

class SceneEdgeCommandEdit : public QUndoCommand
{
public:
    SceneEdgeCommandEdit(const PointValue &pointStart, const PointValue &pointEnd, const PointValue &pointStartNew, const PointValue &pointEndNew,
                         const Value &angle, const Value &angleNew, int segments, int segmentsNew, bool isCurvilinear, bool isCurvilinearNew, QUndoCommand *parent = 0);
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
    bool m_isCurvilinear;
    bool m_isCurvilinearNew;
};

*/
#endif // SCENEEDGE_H

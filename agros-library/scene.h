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

#ifndef SCENE_H
#define SCENE_H

#include "util/util.h"
#include "util/enums.h"
#include "util/point.h"

class Scene;
class SceneNode;
class SceneFace;
class SceneLabel;
class SceneBoundary;
class SceneMaterial;
struct SceneViewSettings;
class LoopsInfo;

class SceneNodeContainer;
class SceneFaceContainer;
class SceneLabelContainer;

class SceneBoundaryContainer;
class SceneMaterialContainer;

class SceneTransformDialog;
class CouplingInfo;

class FieldInfo;
class ProblemBase;
class SolutionStore;
class Log;

class PluginInterface;

QString AGROS_LIBRARY_API generateSvgGeometry(QList<SceneFace *> edges);

class AGROS_LIBRARY_API Scene : public QObject
{
public:
    Scene(ProblemBase *parentProblem);
    ~Scene();

    void copy(const Scene *origin);

    void invalidate();
    void fieldsChange();

    // parent problem
    ProblemBase *parentProblem() { return m_problem; }

    // geometry
    SceneNodeContainer *nodes;
    SceneFaceContainer *faces;
    SceneLabelContainer *labels;

    // boundaries and materials
    SceneBoundaryContainer *boundaries;
    SceneMaterialContainer *materials;

    SceneNode *addNode(SceneNode *node);
    SceneNode *getNode(const Point &point);

    SceneFace *addFace(SceneFace *face);
    SceneFace *getFace(const Point &pointStart, const Point &pointEnd, double angle, int segments);
    SceneFace *getFace(const Point &pointStart, const Point &pointEnd);

    SceneLabel *addLabel(SceneLabel *label);
    SceneLabel *getLabel(const Point &point);

    void addBoundary(SceneBoundary *boundary);
    void removeBoundary(SceneBoundary *boundary);
    void setBoundary(SceneBoundary *boundary); // set edge marker to selected edges
    SceneBoundary *getBoundary(FieldInfo *field, const QString &name);

    void addMaterial(SceneMaterial *material);
    void removeMaterial(SceneMaterial *material);
    void setMaterial(SceneMaterial *material); // set label marker to selected labels
    SceneMaterial *getMaterial(FieldInfo *field, const QString &name);

    void clear();

    RectPoint boundingBox() const;

    void selectNone();
    void selectAll(SceneGeometryMode sceneMode);
    int selectedCount();

    void highlightNone();
    int highlightedCount();

    LoopsInfo *loopsInfo() const { return m_loopsInfo.data(); }
    QMultiMap<SceneFace *, SceneNode *> lyingEdgeNodes() const { return m_lyingEdgeNodes; }
    QMap<SceneNode *, int> numberOfConnectedNodeEdges() const { return m_numberOfConnectedNodeEdges; }
    QList<SceneFace *> crossings() const { return m_crossings; }

    void importFromDxf(const QString &fileName);
    void exportToDxf(const QString &fileName);
    void exportVTKGeometry(const QString &fileName);

    void checkNodeConnect(SceneNode *node);
    void checkTwoNodesSameCoordinates();
    void checkGeometryResult();
    void checkGeometryAssignement();

private:
    ProblemBase *m_problem;

    QSharedPointer<LoopsInfo> m_loopsInfo;
    QMultiMap<SceneFace *, SceneNode *> m_lyingEdgeNodes;
    QMap<SceneNode *, int> m_numberOfConnectedNodeEdges;
    QList<SceneFace *> m_crossings;

    // find lying nodes on edges, number of connected edges and crossings
    void findLyingEdgeNodes();
    void findNumberOfConnectedNodeEdges();
    void findCrossings();
};

#endif /* SCENE_H */

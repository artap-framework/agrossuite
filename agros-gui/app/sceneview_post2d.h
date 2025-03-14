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

#ifndef SCENEVIEWPOST2D_H
#define SCENEVIEWPOST2D_H

#include "util/util.h"
#include "sceneview_mesh.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;

class FieldInfo;
class SceneMarkerSelectDialog;

class SceneViewPost2D : public SceneViewCommon2D, public SceneViewPostInterface
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();

    void selectByMarker();
    void selectPoint();
    void doPostprocessorModeGroup(QAction *action);
    void exportVTKScalarView(const QString &fileName = QString());

public:
    SceneViewPost2D(PostprocessorWidget *postprocessorWidget);
    ~SceneViewPost2D();

    QAction *actSelectPoint;
    QAction *actSelectByMarker;

    QActionGroup *actPostprocessorModeGroup;
    QAction *actPostprocessorModeNothing;
    QAction *actPostprocessorModeLocalPointValue;
    QAction *actPostprocessorModeSurfaceIntegral;
    QAction *actPostprocessorModeVolumeIntegral;

    QAction *actExportVTKScalar;

    void setControls();

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual ProblemBase *problem() const;

    void paintGeometry(); // paint edges

    void paintScalarField(); // paint scalar field surface
    void paintContours(); // paint scalar field contours
    void paintContoursTri(const PostTriangle &triangle, double step);
    void paintVectors(); // paint vector field vectors

    void paintPostprocessorSelectedVolume() const; // paint selected volume for integration
    void paintPostprocessorSelectedSurface() const; // paint selected surface for integration
    void paintPostprocessorSelectedPoint() const; // paint point for local values

private:
    PostprocessorWidget *m_postprocessorWidget;

    // selected point
    Point m_selectedPoint;

    // gl lists
    int m_listContours;
    int m_listVectors;
    int m_listScalarField;

    void createActionsPost2D();

    void exportVTK(const QString &fileName, const QString &variable, PhysicFieldVariableComp physicFieldVariableComp);

    friend class SceneMarkerSelectDialog;
    friend class ResultsView;

private slots:
    void selectedPoint(const Point &p);
    virtual void clearGLLists();    
};

#endif // SCENEVIEWPOST2D_H

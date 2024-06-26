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

#ifndef SCENEVIEWPOST3D_H
#define SCENEVIEWPOST3D_H

#include "util/util.h"
#include "sceneview_common3d.h"

template <typename Scalar> class SceneSolution;
template <typename Scalar> class ViewScalarFilter;
class PostprocessorWidget;

class SceneViewPost3D : public SceneViewCommon3D, public SceneViewPostInterface
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();

public:
    SceneViewPost3D(PostprocessorWidget *postprocessorWidget);
    ~SceneViewPost3D();

    QAction *actSceneModePost3D;

    void setControls();

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();
    virtual void resizeGL(int w, int h);

    virtual ProblemBase *problem() const;

    void paintScalarField3D(); // paint scalar field 3d surface
    void paintScalarField3DSolid(); // paint scalar field 3d solid

    void initLighting();

private:
    PostprocessorWidget *m_postprocessorWidget;

    // gl lists
    int m_listScalarField3D;
    int m_listScalarField3DSolid;
    int m_listModel;

    void createActionsPost3D();

private slots:
    virtual void clearGLLists();    
};

#endif // SCENEVIEWPOST3D_H

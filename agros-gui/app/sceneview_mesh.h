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

#ifndef SCENEVIEWMESH_H
#define SCENEVIEWMESH_H

#include "util/util.h"
#include "sceneview_common2d.h"

#include <deal.II/lac/vector.h>

class PostprocessorWidget;

class SceneViewMesh : public SceneViewCommon2D, public SceneViewPostInterface
{
    Q_OBJECT

public:
    SceneViewMesh(PostprocessorWidget *postprocessorWidget);
    ~SceneViewMesh();

    QAction *actExportVTKOrder;
    QAction *actExportVTKMesh;

    virtual QString labelView() { return tr("Mesh and polynomial order view"); }

    void setControls();

public slots:    
    virtual void clear();
    virtual void refresh();
    void exportVTK(const QString &fileName = QString(), bool exportMeshOnly = false);
    void exportVTKMesh(const QString &fileName = QString());
    void exportVTKOrderView(const QString &fileName = QString());

protected:
    virtual void mousePressEvent(QMouseEvent *event);

    virtual void paintGL();

    virtual ProblemBase *problem() const;

    void paintGeometry();

    void paintInitialMesh();
    void paintSolutionMesh();
    void paintOrder();
    void paintError();
    void paintOrderColorBar();    
    void paintErrorColorBar();

private:
    PostprocessorWidget *m_postprocessorWidget;

    QVector<QVector2D> m_arrayInitialMesh;
    QVector<QVector2D> m_arraySolutionMesh;
    QVector<QVector2D> m_arrayOrderMesh;
    QVector<QVector3D> m_arrayOrderMeshColor;

    dealii::Vector<float>  m_estimated_error_per_cell;

    void createActionsMesh();

private slots:
    virtual void clearGLLists();
};

#endif // SCENEVIEWMESH_H

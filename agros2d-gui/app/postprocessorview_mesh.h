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

#ifndef POSTPROCESSORVIEW_MESH_H
#define POSTPROCESSORVIEW_MESH_H

#include "util/util.h"
#include "postprocessorview.h"

class PostDeal;
class SceneViewMesh;

class PostprocessorSceneMeshWidget : public PostprocessorSceneWidget
{
    Q_OBJECT

public:
    PostprocessorSceneMeshWidget(PhysicalFieldWidget *fieldWidget, SceneViewMesh *sceneMesh);

private:    
    SceneViewMesh *m_sceneMesh;

    QWidget *basic;

    // show
    QCheckBox *chkShowInitialMeshView;
    QCheckBox *chkShowSolutionMeshView;
    QGroupBox *groupBox;
    QRadioButton *rbShowOrderView;
    QRadioButton *rbShowErrorView;

    // polynomial order
    QCheckBox *chkShowOrderColorbar;
    QComboBox *cmbOrderPaletteOrder;
    QCheckBox *chkOrderLabel;
    QSpinBox *txtOrderComponent;

    // mesh and polynomial info
    QLabel *lblMeshInitial;
    QLabel *lblMeshSolution;
    QLabel *lblDOFs;

    void createControls();

public slots:
    virtual void refresh();
    virtual void load();
    virtual void save();
};

#endif // POSTPROCESSORVIEW_MESH_H

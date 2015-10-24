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

#ifndef POSTPROCESSORVIEW_POST3D_H
#define POSTPROCESSORVIEW_POST3D_H

#include "util.h"
#include "postprocessorview.h"

class PostDeal;
class SceneViewPost3D;
class LineEditDouble;
class CollapsableGroupBoxButton;

class PostprocessorScenePost3DWidget : public PostprocessorSceneWidget
{
    Q_OBJECT

public:
    PostprocessorScenePost3DWidget(PostprocessorWidget *postprocessorWidget, SceneViewPost3D *scenePost3D);

private:    
    SceneViewPost3D *m_scenePost3D;

    QButtonGroup *butPost3DGroup;
    QRadioButton *radPost3DNone;
    QRadioButton *radPost3DScalarField3D;
    QRadioButton *radPost3DScalarField3DSolid;
    QRadioButton *radPost3DModel;
    QCheckBox *chkView3DLighting;
    QDoubleSpinBox *txtView3DAngle;
    QCheckBox *chkView3DBackground;
    QDoubleSpinBox *txtView3DHeight;
    QCheckBox *chkView3DBoundingBox;
    QCheckBox *chkView3DSolidGeometry;

    // solid view - materials
    QListWidget *lstSolidMaterials;

    void createControls();

public slots:
    virtual void refresh();
    virtual void load();
    virtual void save();

private slots:

};

#endif // POSTPROCESSORVIEW_POST3D_H

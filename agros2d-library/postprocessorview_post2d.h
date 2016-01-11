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

#ifndef POSTPROCESSORVIEW_POST2D_H
#define POSTPROCESSORVIEW_POST2D_H

#include "util.h"
#include "postprocessorview.h"

class PostDeal;
class SceneViewPost2D;
class LineEditDouble;
class ResultsView;

class PostprocessorScenePost2DWidget : public PostprocessorSceneWidget
{
    Q_OBJECT

public:
    PostprocessorScenePost2DWidget(PostprocessorWidget *postprocessorWidget, SceneViewPost2D *scenePost2D);

private:    
    SceneViewPost2D *m_scenePost2D;

    // toolbar
    QToolBar *toolBar;

    // show
    QCheckBox *chkShowPost2DContourView;
    QCheckBox *chkShowPost2DScalarView;
    QCheckBox *chkShowPost2DVectorView;

    // scalar field
    QComboBox *cmbPostScalarFieldVariable;
    QComboBox *cmbPostScalarFieldVariableComp;
    QCheckBox *chkScalarFieldRangeAuto;
    LineEditDouble *txtScalarFieldRangeMin;
    LineEditDouble *txtScalarFieldRangeMax;
    QCheckBox *chkScalarDeform;

    // vector field
    QComboBox *cmbPost2DVectorFieldVariable;

    // scalar field
    QCheckBox *chkShowScalarColorBar;
    QComboBox *cmbPalette;
    QCheckBox *chkPaletteFilter;
    QSpinBox *txtPaletteSteps;
    QCheckBox *chkScalarFieldRangeLog;
    LineEditDouble *txtScalarFieldRangeBase;
    QSpinBox *txtScalarDecimalPlace;

    // contours
    QComboBox *cmbPost2DContourVariable;
    QSpinBox *txtContoursCount;
    QDoubleSpinBox *txtContourWidth;
    QCheckBox *chkContourDeform;

    // vector field
    QCheckBox *chkVectorProportional;
    QCheckBox *chkVectorColor;
    QSpinBox *txtVectorCount;
    QDoubleSpinBox *txtVectorScale;
    QComboBox *cmbVectorType;
    QComboBox *cmbVectorCenter;
    QCheckBox *chkVectorDeform;

    void createControls();

    QWidget *postScalarAdvancedWidget();
    QWidget *postContourAdvancedWidget();
    QWidget *postVectorAdvancedWidget();

    ResultsView *resultsView;

public slots:
    virtual void refresh();
    virtual void load();
    virtual void save();

private slots:
    void doScalarFieldVariable(int index);
    void doScalarFieldRangeAuto(int state);
    void doPaletteFilter(int state);
    // void doScalarFieldDefault();
    // void doContoursVectorsDefault();
    void doScalarFieldLog(int state);
};

#endif // POSTPROCESSORVIEW_POST2D_H

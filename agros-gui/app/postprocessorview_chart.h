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

#ifndef POSTPROCESSORVIEW_CHART_H
#define POSTPROCESSORVIEW_CHART_H

#include "util/util.h"
#include "postprocessorview.h"
#include "solver/plugin_interface.h"

class SceneViewChart;
class SceneViewChartSimpleGeometry;
class LocalValue;

// definition of chart line
struct ChartLine
{
    Point start;
    Point end;
    int numberOfPoints;
    bool reverse;

    ChartLine() : start(Point()), end(Point()), numberOfPoints(0), reverse(false) {}

    ChartLine(Point start, Point end, int numberOfPoints = 200, bool reverse = false)
    {
        this->start = start;
        this->end = end;
        this->numberOfPoints = numberOfPoints;
        this->reverse = reverse;
    }

    QList<Point> getPoints();
};

class PostprocessorSceneChartWidget : public PostprocessorSceneWidget
{
    Q_OBJECT

public:
    PostprocessorSceneChartWidget(PhysicalFieldWidget *fieldWidget, SceneViewChart *sceneChart);

private:    
    SceneViewChart *sceneChart;

    // variable widget
    SceneViewChartSimpleGeometry *geometryViewer;

    QTabWidget* tbxAnalysisType;

    // buttons
    QPushButton *btnOK;
    QPushButton *btnSaveImage;
    QPushButton *btnExportData;

    // geometry
    QLabel *lblStartX;
    QLabel *lblStartY;
    QLabel *lblEndX;
    QLabel *lblEndY;

    LineEditDouble *txtStartX;
    LineEditDouble *txtStartY;
    LineEditDouble *txtEndX;
    LineEditDouble *txtEndY;

    QRadioButton *radHorizontalAxisLength;
    QRadioButton *radHorizontalAxisX;
    QRadioButton *radHorizontalAxisY;

    QSpinBox *txtHorizontalAxisPoints;
    QCheckBox *chkHorizontalAxisReverse;

    // time
    QLabel *lblPointX;
    QLabel *lblPointY;
    LineEditDouble *txtTimeX;
    LineEditDouble *txtTimeY;

    QComboBox *cmbFieldVariable;
    QComboBox *cmbFieldVariableComp;

    QWidget *widGeometry;
    QWidget *widTime;

    void createControls();

public slots:
    virtual void refresh();
    virtual void load();
    virtual void save();

private slots:
    void doFieldVariable(int index);
    void createChartLine();
};

#endif // POSTPROCESSORVIEW_CHART_H

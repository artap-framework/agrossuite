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

#ifndef OPTILAB_H
#define OPTILAB_H

#include "util/util.h"
#include "util/point.h"
#include "gui/other.h"
#include "gui/chart.h"

class Study;
class Computation;
class SceneViewSimpleGeometry;

class OptiLab : public QWidget
{
    Q_OBJECT
public:
    OptiLab(QWidget *parent = 0);
    ~OptiLab();

    QAction *actSceneModeOptiLab;

signals:
    void solveCurrentComputation(const QSharedPointer<Computation> computation);

public slots:
    void refresh();

    void doStudySolved(Study *study);
    void doStudySelected(Study *study);
    void doStudyChanged(int index);
    void doComputationSelected(const QSharedPointer<Computation> computation);
    void doSolveCurrentComputation();
    void doChartRefreshed();

    void exportData();

private:
    QList<QSharedPointer<Computation> > m_studyComputations;
    QSharedPointer<Computation> m_selectedComputation;

    SceneViewSimpleGeometry *geometryViewer;

    QComboBox *cmbStudies;

    QComboBox *cmbAxisX;
    QComboBox *cmbAxisY;

    // computation
    QAction *actComputationSolve;

    // results
    QTreeWidget *trvResults;

    QToolBar *toolBarRight;

    ChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QLineSeries *trendLineSeries;
    QScatterSeries *valueSeries;
    QScatterSeries *valueSelectedSeries;
    QScatterSeries *valueHoverSeries;
    QLineSeries *averageValueSeries;
    QLineSeries *averageValueLowerSeries;
    QLineSeries *averageValueUpperSeries;
    QAreaSeries *averageValueAreaSeries;
    QScatterSeries *paretoFrontSeries;

    QMenu *mnuChart;
    QMap<int, QMap<QPair<double, double>, QSharedPointer<Computation> > > m_computationMap;


    QAction *actResultsFindMinimum;
    QAction *actResultsFindMaximum;
    QAction *actChartRescale;
    QAction *actChartShowTrend;
    QAction *actChartShowAverageValue;
    QAction *actChartShowParetoFront;

    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;

    QAction *actExportToCsv;

    void createControls();
    QWidget *createControlsChart();
    QWidget *createControlsResults();

    void resultsFindExtrem(bool minimum);
    int findPointIndex(const QPointF &point);

private slots:
    void chartContextMenu(const QPoint &pos);
    void chartRescale(bool checked);
    void chartShowTrend(int state);
    void chartShowAverageValue(int state);
    void chartShowParetoFront(int state);

    void chartClicked(const QPointF &point);
    void chartHovered(const QPointF &point, bool state);

    void doResultChanged();

    void resultsFindMinimum(bool checked);
    void resultsFindMaximum(bool checked);

    void axisXChanged(int index);
    void axisYChanged(int index);

    void doZoomIn();
    void doZoomOut();
    void doZoomBestFit();
};

#endif // OPTILAB_H

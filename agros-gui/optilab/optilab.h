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

class OptiLab;
class Study;
class Computation;
class SceneViewSimpleGeometry;
class OptiLabWidget;

class OptiLab : public QWidget
{
    Q_OBJECT
public:
    OptiLab(QWidget *parent = 0);
    ~OptiLab();

    QAction *actSceneModeOptiLab;
    inline OptiLabWidget *optiLabWidget() { return m_optiLabWidget; }
    inline Study *selectedStudy() { return m_selectedStudy; }

public slots:
    void refresh();

    void doStudySelected(Study *study);
    void doComputationChanged(int index);
    void doComputationSelected(const QString &problemDir);
    void doComputationSolve(bool ok);
    void doChartRefreshed(const QString &problemDir = "");

private:
    Study *m_selectedStudy;

    OptiLabWidget *m_optiLabWidget;
    SceneViewSimpleGeometry *geometryViewer;

    QLabel *lblResultMin;
    QLabel *lblResultMax;
    QLabel *lblResultMean;
    QLabel *lblResultMedian;
    QLabel *lblResultVariance;
    QLabel *lblResultStdDev;
    // QLabel *lblResultNormalCovariance; // normal distribution
    // QLabel *lblResultNormalCorrelation; // normal distribution

    QChart *resultsStatChart;
    QScatterSeries *resultsStatMinMaxSeries;
    QScatterSeries *resultsStatMeanSeries;
    QScatterSeries *resultsStatMedianSeries;
    QValueAxis *axisStatX;
    QValueAxis *axisStatY;

    // computation
    QComboBox *cmbComputations;
    QMenu *mnuComputations;
    QAction *actComputationDelete;
    QAction *actComputationSolve;

    // results
    QTreeWidget *trvResults;
    QMenu *mnuResults;

    QAction *actResultsDependenceOnSteps;
    QAction *actResultsSetHorizontal;
    QAction *actResultsSetVertical;
    QAction *actResultsFindMinimum;
    QAction *actResultsFindMaximum;

    ChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QLineSeries *trendLineSeries;
    QScatterSeries *valueSeries;
    QLineSeries *averageValueSeries;
    QLineSeries *averageValueLowerSeries;
    QLineSeries *averageValueUpperSeries;
    QAreaSeries *averageValueAreaSeries;
    QLineSeries *paretoFrontSeries;

    QMenu *mnuChart;
    QMap<int, QMap<QPair<double, double>, QSharedPointer<Computation> > > m_computationMap;

    QAction *actChartRescale;
    QAction *actChartLogHorizontal;
    QAction *actChartLogVertical;
    QAction *actChartShowTrend;
    QAction *actChartShowAverageValue;
    QAction *actChartParetoFront;

    void createControls();
    QWidget *createControlsGeometryAndStats();
    QWidget *createControlsChart();
    QWidget *createControlsResults();
    // QPair<double, double> findClosestData(QCPGraph *graph, const Point &pos);

    void resultsFindExtrem(bool minimum);

    QScatterSeries *createSeries(const QString &name);

private slots:
    void chartContextMenu(const QPoint &pos);
    void chartRescale(bool checked);
    void chartLogHorizontal(bool checked);
    void chartLogVertical(bool checked);
    void chartShowTrend(bool checked);
    void chartShowAverageValue(bool checked);
    void chartShowParetoFront(bool checked);

    void resultsContextMenu(const QPoint &pos);
    void resultsDependenceOnSteps(bool checked);
    void resultsSetHorizontal(bool checked);
    void resultsSetVertical(bool checked);

    void chartClicked(const QPointF &point);

    void doResultChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest);

    void resultsFindMinimum(bool checked);
    void resultsFindMaximum(bool checked);
};

#endif // OPTILAB_H

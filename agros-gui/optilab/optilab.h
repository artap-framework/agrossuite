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

#include <QWidget>

#include "util/util.h"
#include "util/enums.h"
#include "util/point.h"
#include "gui/other.h"

#include "qcustomplot.h"

class OptiLab;
class Study;
class Computation;
class SceneViewSimpleGeometry;

class OptiLabWidget : public QWidget
{
    Q_OBJECT
public:
    OptiLabWidget(OptiLab *parent);
    ~OptiLabWidget();

    QAction *actRunStudy;

public slots:
    void refresh();
    void exportData();

private:
    OptiLab *m_optilab;

    QTreeWidget *trvComputations;
    QLabel *lblNumberOfComputations;
    QComboBox *cmbStudies;
    QLineEdit *txtFilter;

    QMenu *mnuComputations;
    QAction *actComputationDelete;
    QAction *actComputationSolve;

    void createControls();

signals:
    void studySelected(Study *study);
    void computationSelected(const QString &key);
    void chartRefreshed(const QString &key);

private slots:
    void doComputationChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest);
    void doComputationSelected(const QString &key);
    void doComputationContextMenu(const QPoint &pos);
    void doComputationDelete(bool);
    void doComputationSolve(bool);

    void studyChanged(int index);    

    void solveStudy();
};

class OptiLab : public QWidget
{
    Q_OBJECT
public:
    OptiLab(QWidget *parent = 0);
    ~OptiLab();

    QAction *actSceneModeOptiLab;
    inline OptiLabWidget *optiLabWidget() { return m_optiLabWidget; }

    void setStudy(Study *study);

signals:
    void computationSelected(const QString &key);

public slots:
    void refresh();

    void doComputationSelected(const QString &key);
    void doChartRefreshed(const QString &key = "");

private:
    Study *m_study;

    QSplitter *splitter;

    OptiLabWidget *m_optiLabWidget;
    SceneViewSimpleGeometry *geometryViewer;

    QLabel *lblResultMin;
    QLabel *lblResultMax;
    QLabel *lblResultSum;
    QLabel *lblResultMean;
    QLabel *lblResultMedian;
    QLabel *lblResultVariance;
    QLabel *lblResultStdDev;
    QLabel *lblResultNormalCovariance; // normal distribution
    QLabel *lblResultNormalCorrelation; // normal distribution

    QTreeWidget *trvResults;
    QMenu *mnuResults;

    QAction *actResultsDependenceOnSteps;
    QAction *actResultsSetHorizontal;
    QAction *actResultsSetVertical;
    QAction *actResultsFindMinimum;
    QAction *actResultsFindMaximum;

    QCustomPlot *chart;
    QCPItemLine *chartTrendLine;
    QList<QCPGraph *> chartGraphCharts;
    QCPGraph *chartGraphAverageValue;
    QCPGraph *chartGraphAverageValueChannelLower;
    QCPGraph *chartGraphAverageValueChannelUpper;
    QCPGraph *chartGraphSelectedComputation;
    QCPGraph *chartGraphParetoFront;
    QMenu *mnuChart;
    QMap<int, QMap<QPair<double, double>, QSharedPointer<Computation> > > m_computationMap;

    QAction *actChartRescale;
    QAction *actChartLogHorizontal;
    QAction *actChartLogVertical;
    QAction *actChartShowTrend;
    QAction *actChartShowAverageValue;
    QAction *actChartParetoFront;

    void createControls();
    QWidget *createControlsDistChart();
    QWidget *createControlsChart();
    QWidget *createControlsResults();
    QPair<double, double> findClosestData(QCPGraph *graph, const Point &pos);

    void resultsFindExtrem(bool minimum);

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

    void graphClicked(QCPAbstractPlottable *plottable, int code, QMouseEvent *event);
    void graphMouseDoubleClick(QMouseEvent *event);

    void doResultChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest);

    void resultsFindMinimum(bool checked);
    void resultsFindMaximum(bool checked);
};

#endif // OPTILAB_H

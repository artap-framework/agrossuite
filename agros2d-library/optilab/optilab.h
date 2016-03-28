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

#include "util.h"
#include "util/enums.h"
#include "gui/textedit.h"

#include "qcustomplot.h"

class OptiLab;
class Study;
class Computation;
class SceneViewSimpleGeometry;

class AGROS_LIBRARY_API OptiLabWidget : public QWidget
{
    Q_OBJECT
public:
    OptiLabWidget(OptiLab *parent);
    ~OptiLabWidget();

    QAction *actRunStudy;

private:
    OptiLab *m_optilab;

    QTreeWidget *trvComputations;
    QLabel *lblNumberOfSets;
    QLabel *lblNumberOfComputations;
    QComboBox *cmbStudies;

    QMenu *mnuComputations;
    QAction *actComputationDelete;

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

    void studyChanged(int index);
    void refresh();

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
    void doComputationSelected(const QString &key);
    void doChartRefreshed(const QString &key = "");

private:
    enum ResultType
    {
        ResultType_Parameter,
        ResultType_Functional,
        ResultType_Recipe
    };

    inline QString resultTypeToStringKey(ResultType type)
    {
        if (type == ResultType::ResultType_Parameter) return "parameter";
        else if (type == ResultType::ResultType_Functional) return "functional";
        else if (type == ResultType::ResultType_Recipe) return "recipe";
        else assert(0);
    }

    inline ResultType resultTypeFromStringKey(const QString &type)
    {
        if (type == "parameter") return ResultType::ResultType_Parameter;
        else if (type == "functional") return ResultType::ResultType_Functional;
        else if (type == "recipe") return ResultType::ResultType_Recipe;
        else assert(0);
    }

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
    QMenu *mnuChart;
    QMap<int, QMap<QPair<double, double>, QSharedPointer<Computation> > > m_computationMap;

    QCustomPlot *pdfChart;
    QCustomPlot *cdfChart;

    QAction *actChartRescale;
    QAction *actChartLogHorizontal;
    QAction *actChartLogVertical;
    QAction *actChartShowTrend;
    QAction *actChartShowAverageValue;

    void createControls();
    QWidget *createControlsDistChart();
    QWidget *createControlsChart();
    QWidget *createControlsResults();
    QCPData findClosestData(QCPGraph *graph, const Point &pos);

private slots:
    void refresh();

    void chartContextMenu(const QPoint &pos);
    void chartRescale(bool checked);
    void chartLogHorizontal(bool checked);
    void chartLogVertical(bool checked);
    void chartShowTrend(bool checked);
    void chartShowAverageValue(bool checked);

    void resultsContextMenu(const QPoint &pos);
    void resultsDependenceOnSteps(bool checked);
    void resultsSetHorizontal(bool checked);
    void resultsSetVertical(bool checked);

    void graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event);
    void graphMouseDoubleClick(QMouseEvent *event);

    void doResultChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest);
};

#endif // OPTILAB_H

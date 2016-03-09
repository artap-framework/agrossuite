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
class InfoWidgetGeneral;

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
    QComboBox *cmbStudies;

    QCheckBox *chkChartLogX;
    QCheckBox *chkChartLogY;
    QComboBox *cmbChartX;
    QComboBox *cmbChartY;

    QPushButton *btnPlotChart;

    void createControls();

signals:
    void computationSelected(const QString &key);
    void chartRefreshed(Study *study, QSharedPointer<Computation> computation);

private slots:
    void doComputationChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest);
    void doComputationSelected(const QString &key);

    void studyChanged(int index);
    void refresh();

    void solveStudy();
    void plotChart();
};

class OptiLab : public QWidget
{
    Q_OBJECT
public:
    OptiLab(QWidget *parent = 0);
    ~OptiLab();

    QAction *actSceneModeOptiLab;
    inline OptiLabWidget *optiLabWidget() { return m_optiLabWidget; }

signals:
    void computationSelected(const QString &key);

public slots:
    void doComputationSelected(const QString &key);
    void doChartRefreshed(Study *study, QSharedPointer<Computation> selectedComputation);

private:
    Study *m_study;

    QSplitter *splitter;

    OptiLabWidget *m_optiLabWidget;
    InfoWidgetGeneral *m_infoWidget;

    QCustomPlot *chart;
    QMenu *mnuChart;
    QMap<int, QMap<QPair<double, double>, QSharedPointer<Computation> > > m_computationMap;

    QAction *actRescale;

    void createControls();
    QCPData findClosestData(QCPGraph *graph, const Point &pos);

private slots:
    void refresh();

    void chartContextMenu(const QPoint &pos);
    void chartRescale(bool checked);

    void graphClicked(QCPAbstractPlottable *plottable, QMouseEvent *event);
};

#endif // OPTILAB_H

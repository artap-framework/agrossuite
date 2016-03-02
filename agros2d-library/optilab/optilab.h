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

class OptiLab;
class Study;
class Computation;
class InfoWidgetGeneral;
class QCustomPlot;

class AGROS_LIBRARY_API OptiLabWidget : public QWidget
{
    Q_OBJECT
public:
    OptiLabWidget(OptiLab *parent);
    ~OptiLabWidget();

private:
    OptiLab *m_optilab;

    QTreeWidget *trvComputations;
    QComboBox *cmbStudies;

    QCheckBox *chkShowAllSets;
    QCheckBox *chkChartLogX;
    QCheckBox *chkChartLogY;
    QComboBox *cmbChartX;
    QComboBox *cmbChartY;

    QPushButton *btnSolveStudy;
    QPushButton *btnPlotChart;

    void createControls();

signals:
    void computationSelected(const QString &key);
    void chartRefreshed(Study *study, QSharedPointer<Computation> computation);

private slots:
    void computationChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest);
    void computationSelect(const QString &key);

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

public slots:
    void computationSelected(const QString &key);
    void chartRefreshed(Study *study, QSharedPointer<Computation> selectedComputation);

private:
    OptiLabWidget *m_optiLabWidget;
    InfoWidgetGeneral *m_infoWidget;
    QCustomPlot *m_chart;

    void createControls();
};

#endif // OPTILAB_H

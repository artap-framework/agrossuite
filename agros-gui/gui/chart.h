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

#ifndef GUI_CHART_H
#define GUI_CHART_H

#include "util/util.h"
#include "util/enums.h"

#include <QtCharts>

void fitToDataChart(QChart *chart);

class Crosshairs
{
public:
    Crosshairs(QChart *chart);
    void updatePosition(QPointF position);

private:
    QGraphicsLineItem *m_xLine, *m_yLine;
    QGraphicsTextItem *m_xText, *m_yText;
    QChart *m_chart;
};

class ChartView : public QChartView
{
public:
    ChartView(QChart *chart = nullptr, QWidget *parent = nullptr);

    void fitToData();

protected:
    bool viewportEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);

protected:
    QChart *m_chart;

    bool m_isTouching = false;
    QPointF m_lastMousePos;

    Crosshairs *m_crosshairs;
};

class ChartViewAxis : public ChartView
{
public:
    ChartViewAxis(QChart *chart = nullptr, QWidget *parent = nullptr);

    inline QValueAxis *axisX() { return m_axisX; }
    inline QValueAxis *axisY() { return m_axisY; }
    inline QLineSeries *series() { return m_series; }

protected:
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QLineSeries *m_series;
};

#endif // GUI_COMMON_H

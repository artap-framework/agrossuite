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

#include "util.h"
#include "util/enums.h"

#include "qcustomplot/qcustomplot.h"

class ChartImage : public QCustomPlot
{
public:
    ChartImage(QWidget *parent = NULL);

    void setLabel(const QString &labelx, const QString &labely);    
    void setData(const QVector<double> &x, const QVector<double> &y);

    QString save(const QString &fileName = "");

protected:
    QCPGraph *m_line;

    QFont fontChart;
    QPen pen;
};

class ChartNonlinearImage : public ChartImage
{
public:
    ChartNonlinearImage(QWidget *parent = NULL);

    void setError(const QVector<double> &x, const QVector<double> &y) { setData(x, y); }
};

class ChartAdaptivityImage : public ChartImage
{
public:
    ChartAdaptivityImage(QWidget *parent = NULL);

    void setError(const QVector<double> &x, const QVector<double> &y) { setData(x, y); }
    void setDOFs(const QVector<double> &x, const QVector<double> &y);

protected:
    QCPGraph *m_dofs;
};

class ChartTransientImage : public ChartImage
{
public:
    ChartTransientImage(QWidget *parent = NULL);

    void setStepLength(const QVector<double> &x, const QVector<double> &y) { setData(x, y); }
    void setTotalTime(const QVector<double> &x, const QVector<double> &y);

protected:
    QCPGraph *m_totalTime;
};

#endif // GUI_COMMON_H

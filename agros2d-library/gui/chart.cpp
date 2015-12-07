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

#include "chart.h"

#include "util.h"
#include "util/global.h"

ChartImage::ChartImage(QWidget *parent) : QCustomPlot(parent)
{
    fontChart.setPointSize(fontChart.pointSize() - 3);

    pen.setColor(Qt::darkGray);
    pen.setWidth(2);

    xAxis->setTickLabelFont(fontChart);
    xAxis->setLabelFont(fontChart);
    xAxis->setAutoTickStep(true);

    yAxis->setTickLabelFont(fontChart);
    yAxis->setLabelFont(fontChart);
    yAxis->setAutoTickStep(true);

    m_line = addGraph(xAxis, yAxis);
    m_line->setLineStyle(QCPGraph::lsLine);
    m_line->setPen(pen);
    m_line->setBrush(QBrush(QColor(0, 0, 255, 20)));
}

void ChartImage::setLabel(const QString &labelx, const QString &labely)
{
    xAxis->setLabel(labelx);
    yAxis->setLabel(labely);
}

void ChartImage::setData(const QVector<double> &x, const QVector<double> &y)
{
    m_line->setData(x, y);
}

QString ChartImage::save(const QString &fileName)
{
    rescaleAxes();
    replot(QCustomPlot::rpImmediate);

    QString fn = fileName;
    if (fn.isEmpty())
    {
        QDateTime currentTime(QDateTime::currentDateTime());
        fn = QString("%1/log/%2.png").arg(tempProblemDir()).arg(currentTime.toString("yyyy-MM-dd-hh-mm-ss-zzz"));
    }

    const int width = 650;
    const int height = 280;

    savePng(fn, width, height);

    return fn;
}

ChartNonlinearImage::ChartNonlinearImage(QWidget *parent) : ChartImage(parent)
{
    xAxis->setLabel(QObject::tr("number of iterations (-)"));

    yAxis->setLabel(QObject::tr("rel. change of sol. (%)"));
    yAxis->setScaleType(QCPAxis::stLogarithmic);

    m_line->setName(tr("Error"));
}

ChartAdaptivityImage::ChartAdaptivityImage(QWidget *parent) : ChartImage(parent)
{
    legend->setVisible(true);
    legend->setFont(fontChart);

    xAxis->setLabel(QObject::tr("number of iterations (-)"));

    yAxis->setLabel(QObject::tr("error (%)"));
    yAxis->setScaleType(QCPAxis::stLogarithmic);

    yAxis2->setLabel(QObject::tr("number of DOFs (-)"));
    yAxis2->setTickLabelFont(fontChart);
    yAxis2->setLabelFont(fontChart);
    yAxis2->setVisible(true);

    m_line->setName(tr("Error"));

    m_dofs = addGraph(xAxis, yAxis2);
    m_dofs->setLineStyle(QCPGraph::lsLine);
    m_dofs->setPen(pen);
    m_dofs->setBrush(QBrush(QColor(255, 0, 0, 20)));
    m_dofs->setName(tr("DOFs"));
}

void ChartAdaptivityImage::setDOFs(const QVector<double> &x, const QVector<double> &y)
{
    m_dofs->setData(x, y);
}

ChartTransientImage::ChartTransientImage(QWidget *parent) : ChartImage(parent)
{
    legend->setVisible(true);
    legend->setFont(fontChart);

    xAxis->setLabel(QObject::tr("number of steps (-)"));
    yAxis->setLabel(QObject::tr("step length (s)"));

    yAxis2->setLabel(QObject::tr("total time (s)"));
    yAxis2->setTickLabelFont(fontChart);
    yAxis2->setLabelFont(fontChart);
    yAxis2->setVisible(true);

    m_line->setName(tr("Step length"));

    m_totalTime = addGraph(xAxis, yAxis2);
    m_totalTime->setLineStyle(QCPGraph::lsLine);
    m_totalTime->setPen(pen);
    m_totalTime->setBrush(QBrush(QColor(255, 0, 0, 20)));
    m_totalTime->setName(tr("Total time"));
}

void ChartTransientImage::setTotalTime(const QVector<double> &x, const QVector<double> &y)
{
    m_totalTime->setData(x, y);
}

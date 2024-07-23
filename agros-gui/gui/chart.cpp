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

#include "util/util.h"
#include "util/global.h"

#include <QMouseEvent>

QRectF findMinMax(const QList<QPointF>& points)
{
    qreal minX = std::numeric_limits<qreal>::max();
    qreal maxX = -std::numeric_limits<qreal>::max();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = -std::numeric_limits<qreal>::max();

    for (int i = 0; i < points.size(); i++)
    {
        // x
        if (points.at(i).x() < minX)
            minX = points.at(i).x();
        if (points.at(i).x() > maxX)
            maxX = points.at(i).x();

        // y
        if (points.at(i).y() < minY)
            minY = points.at(i).y();
        if (points.at(i).y() > maxY)
            maxY = points.at(i).y();
    }

    return {QPointF(minX, minY), QPointF(maxX, maxY)};
}

void fitToDataChart(QChart *chart)
{
    // only works for line and scatter chart
    QList<QPointF> points;
    foreach(QAbstractSeries *series, chart->series())
    {
        if (series->type() == QAbstractSeries::SeriesTypeLine)
        {
            auto s = dynamic_cast<QLineSeries *>(series);
            foreach(QPointF point, s->points())
                points.append(point);
        }
        else if (series->type() == QAbstractSeries::SeriesTypeScatter)
        {
            auto s = dynamic_cast<QScatterSeries *>(series);
            foreach(QPointF point, s->points())
                points.append(point);
        }
        else if (series->type() == QAbstractSeries::SeriesTypeArea)
        {
            auto *area = dynamic_cast<QAreaSeries *>(series);
            // add upper
            foreach(QPointF point, area->upperSeries()->points())
                points.append(point);
            // add lower
            foreach(QPointF point, area->lowerSeries()->points())
                points.append(point);
        }
    }

    // find min and max
    QRectF axesRange = findMinMax(points);

    // apply to axes
    foreach(QAbstractSeries *series, chart->series())
    {
        // qInfo() << minX << maxX << minY << maxY;
        foreach(QAbstractAxis *axis, series->attachedAxes())
        {
            if (axis->orientation() == Qt::Horizontal)
            {
                axis->setRange(axesRange.left(), axesRange.right());
            }
            if (axis->orientation() == Qt::Vertical)
            {
                axis->setRange(axesRange.top(), axesRange.bottom());
            }
//            if (axis->orientation() == Qt::Horizontal)
//            {
//                if (axis->type() == QAbstractAxis::AxisTypeValue)
//                {
//                    axis->setMin(std::min(minX, static_cast<QValueAxis *>(axis)->min()));
//                    axis->setMax(std::max(maxX, static_cast<QValueAxis *>(axis)->max()));
//                }
//                else if (axis->type() == QAbstractAxis::AxisTypeLogValue)
//                {
//                    axis->setMin(std::min(minX, static_cast<QLogValueAxis *>(axis)->min()));
//                    axis->setMax(std::max(maxX, static_cast<QLogValueAxis *>(axis)->max()));
//                }
//            }
//            if (axis->orientation() == Qt::Vertical)
//            {
//                if (axis->type() == QAbstractAxis::AxisTypeValue)
//                {
//                    axis->setMin(std::min(minY, static_cast<QValueAxis *>(axis)->min()));
//                    axis->setMax(std::max(maxY, static_cast<QValueAxis *>(axis)->max()));
//                }
//                else if (axis->type() == QAbstractAxis::AxisTypeLogValue)
//                {
//                    axis->setMin(std::min(minY, static_cast<QLogValueAxis *>(axis)->min()));
//                    axis->setMax(std::max(maxY, static_cast<QLogValueAxis *>(axis)->max()));
//                }
//            }
        }
    }
}

Crosshairs::Crosshairs(QChart *chart) :
    m_xLine(new QGraphicsLineItem(chart)),
    m_yLine(new QGraphicsLineItem(chart)),
    m_xText(new QGraphicsTextItem(chart)),
    m_yText(new QGraphicsTextItem(chart)),
    m_chart(chart)
{
    QPen pen;
    pen.setColor(Qt::darkGray);
    pen.setStyle(Qt::DashLine);

    m_xLine->setPen(pen);
    m_yLine->setPen(pen);
    m_xText->setZValue(11);
    m_yText->setZValue(11);
    m_xText->document()->setDocumentMargin(0);
    m_yText->document()->setDocumentMargin(0);
    m_xText->setDefaultTextColor(Qt::white);
    m_yText->setDefaultTextColor(Qt::white);
}

void Crosshairs::updatePosition(QPointF position)
{
    QLineF xLine(position.x(), m_chart->plotArea().top(),
                 position.x(), m_chart->plotArea().bottom());
    QLineF yLine(m_chart->plotArea().left(), position.y(),
                 m_chart->plotArea().right(), position.y());
    m_xLine->setLine(xLine);
    m_yLine->setLine(yLine);

    QString xText = QString("%1").arg(m_chart->mapToValue(position).x());
    QString yText = QString("%1").arg(m_chart->mapToValue(position).y());
    m_xText->setHtml(QString("<div style='background-color: #4b4b4b;'>") + xText + "</div>");
    m_xText->setPos(position.x() + 20, position.y() + 30);

    m_yText->setHtml(QString("<div style='background-color: #4b4b4b;'>") + yText + "</div>");
    m_yText->setPos(position.x() + 20, position.y() + 50);

    // m_xText->setPos(position.x() - m_xText->boundingRect().width() / 2.0, m_chart->plotArea().bottom());
    // m_yText->setPos(m_chart->plotArea().right(), position.y() - m_yText->boundingRect().height() / 2.0);

    if (!m_chart->plotArea().contains(position))
    {
        m_xLine->hide();
        m_xText->hide();
        m_yLine->hide();
        m_yText->hide();
    }
    else
    {
        m_xLine->show();
        m_xText->show();
        m_yLine->show();
        m_yText->show();
    }
}

// *******************************************************************************************************************************************

ChartView::ChartView(QChart *chart, bool showCrosshair, QWidget *parent) : QChartView(parent), m_crosshairs(nullptr)
{
    if (chart == nullptr)
    {
        m_chart = new QChart();
        m_chart->setMinimumSize(320, 240);
        m_chart->legend()->hide();
    }
    else
    {
        m_chart = chart;
    }

    m_chart->setAcceptHoverEvents(true);

    // set chart
    setChart(m_chart);

    // crosshairs
    if (showCrosshair)
        m_crosshairs = new Crosshairs(m_chart);

    setRenderHint(QPainter::Antialiasing);
    // setRubberBand(QChartView::RectangleRubberBand);
    setMouseTracking(true);
}

void ChartView::fitToData()
{
    fitToDataChart(m_chart);
    repaint();
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);
}


bool ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin)
    {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        m_isTouching = true;
    }

    return QChartView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
        m_lastMousePos = event->pos();
        event->accept();
    }

    if (m_isTouching)
        return;

    QChartView::mousePressEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    // pan the chart with a middle mouse drag
    if (event->buttons() & Qt::MiddleButton)
    {
        auto dPos = event->pos() - m_lastMousePos;
        chart()->scroll(-dPos.x(), dPos.y());

        m_lastMousePos = event->pos();
        event->accept();
    }

    // crosshairs
    if (m_crosshairs)
        m_crosshairs->updatePosition(event->pos());

    if (m_isTouching)
        return;

    QChartView::mouseMoveEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
         QApplication::restoreOverrideCursor();
    }

    if (m_isTouching)
        m_isTouching = false;

    QChartView::mouseReleaseEvent(event);
}

void ChartView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // fit data
    if (event->button() == Qt::MiddleButton)
    {
        fitToData();
    }

    QChartView::mouseDoubleClickEvent(event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::wheelEvent(QWheelEvent *event)
{
    qreal factor;
    if (event->angleDelta().y() > 0)
        factor = 2.0;
    else
        factor = 0.5;

    QRectF r = QRectF(chart()->plotArea().left(),chart()->plotArea().top(),
                      chart()->plotArea().width()/factor,chart()->plotArea().height()/factor);

    QPointF mousePos = mapFromGlobal(QCursor::pos());
    r.moveCenter(mousePos);
    chart()->zoomIn(r);

    QPointF delta = chart()->plotArea().center() - mousePos;
    chart()->scroll(delta.x(), -delta.y());
}

// ******************************************************************************************************************************

ChartViewAxis::ChartViewAxis(QChart *chart, QWidget *parent) : ChartView(chart, parent)
{
    // axis x
    m_axisX = new QValueAxis;
    m_axisX->setLabelFormat("%g");
    m_axisX->setGridLineVisible(true);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    // axis y
    m_axisY = new QValueAxis;
    m_axisY->setLabelFormat("%g");
    m_axisY->setGridLineVisible(true);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // attach axis
    m_series = new QLineSeries();
    m_chart->addSeries(m_series);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);
}

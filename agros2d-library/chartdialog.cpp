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

#include "chartdialog.h"

#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "sceneview_geometry_chart.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/solutionstore.h"
#include "solver/problem_config.h"
#include "pythonlab/pythonengine_agros.h"

#include "gui/common.h"
#include "gui/lineeditdouble.h"
#include "gui/physicalfield.h"

#include <QSvgRenderer>
#include "qcustomplot/qcustomplot.h"

SceneViewChart::SceneViewChart(PostprocessorWidget *postprocessorWidget) : QWidget(postprocessorWidget)
{
    QPen chartPen;
    chartPen.setColor(QColor(129, 17, 19));
    chartPen.setWidthF(2);

    m_chart = new QCustomPlot(this);
    m_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_chart->addGraph();
    m_chart->setCursor(QCursor(Qt::CrossCursor));

    m_chart->graph(0)->setLineStyle(QCPGraph::lsLine);
    m_chart->graph(0)->setPen(chartPen);

    QPen tracerPen;
    tracerPen.setWidth(0);
    // tracerPen.setBrush(Qt::NoBrush);
    tracerPen.setColor(Qt::darkGreen);

    connect(m_chart, SIGNAL(mouseMove(QMouseEvent *)), this, SLOT(chartMouseMoved(QMouseEvent*)));

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(m_chart);

    setLayout(layoutMain);

    refresh();

    // reconnect computation slots
    connect(postprocessorWidget, SIGNAL(connectComputation(QSharedPointer<Computation>)), this, SLOT(connectComputation(QSharedPointer<Computation>)));
}

void SceneViewChart::chartMouseMoved(QMouseEvent *event)
{
    // coord of mouse
    double x = m_chart->xAxis->pixelToCoord(event->pos().x());
    double y = m_chart->yAxis->pixelToCoord(event->pos().y());

    emit labelRight(tr("Position: [%1; %2]").arg(x, 8, 'e', 5).arg(y, 8, 'e', 5));
}

void SceneViewChart::refresh()
{
    if (!(m_computation && m_computation->postDeal()->activeViewField()))
        return;

    if (m_computation->setting()->value(PostprocessorSetting::View_ChartVariable).toString().isEmpty())
        return;

    Module::LocalVariable physicFieldVariable = m_computation->postDeal()->activeViewField()->localVariable(m_computation->config()->coordinateType(),
                                                                                                            m_computation->setting()->value(PostprocessorSetting::View_ChartVariable).toString());

    emit labelCenter(physicFieldVariable.name());

    if ((ChartMode) m_computation->setting()->value(PostprocessorSetting::View_ChartMode).toInt() == ChartMode_Geometry)
        plotGeometry();
    else if ((ChartMode) m_computation->setting()->value(PostprocessorSetting::View_ChartMode).toInt() == ChartMode_Time)
        plotTime();

    // rescale axis
    if (!m_chart->graph(0)->data()->isEmpty())
    {
        double min =   numeric_limits<double>::max();
        double max = - numeric_limits<double>::max();
        for (int i = 0; i < m_chart->graph(0)->data()->values().count(); i++)
        {
            double value = m_chart->graph(0)->data()->values().at(i).value;
            if (value < min) min = value;
            if (value > max) max = value;
        }

        if ((max - min) < EPS_ZERO)
        {
            m_chart->graph(0)->valueAxis()->setRange(min - 1, min + 1);
            m_chart->graph(0)->keyAxis()->setRange(m_chart->graph(0)->data()->keys().first(),
                                                   m_chart->graph(0)->data()->keys().last());
        }
        else
        {
            m_chart->rescaleAxes();
        }

        m_chart->replot(QCustomPlot::rpQueued);
    }
}

void SceneViewChart::connectComputation(QSharedPointer<Computation> computation)
{
    if (!m_computation.isNull())
    {
        disconnect(m_computation.data()->postDeal(), SIGNAL(processed()), this, SLOT(refresh()));
    }

    m_computation = computation;

    if (!m_computation.isNull())
    {
        connect(m_computation.data()->postDeal(), SIGNAL(processed()), this, SLOT(refresh()));
    }
}

QVector<double> SceneViewChart::horizontalAxisValues(ChartLine *chartLine)
{
    QList<Point> points = chartLine->getPoints();
    QVector<double> xval;

    switch ((ChartAxisType) m_computation->setting()->value(PostprocessorSetting::View_ChartHorizontalAxis).toInt())
    {
    case ChartAxis_Length:
    {
        for (int i = 0; i < points.length(); i++)
        {
            if (i == 0)
                xval.append(0.0);
            else
                xval.append(xval.at(i-1) + (points.at(i) - points.at(i-1)).magnitude());
        }
    }
        break;
    case ChartAxis_X:
    {
        foreach (Point point, points)
            xval.append(point.x);
    }
        break;
    case ChartAxis_Y:
    {
        foreach (Point point, points)
            xval.append(point.y);
    }
        break;
    default:
        assert(0);
    }

    return xval;
}

void SceneViewChart::plotGeometry()
{
    if (m_computation->setting()->value(PostprocessorSetting::View_ChartVariable).toString().isEmpty())
        return;

    // variable
    Module::LocalVariable physicFieldVariable = m_computation->postDeal()->activeViewField()->localVariable(m_computation->config()->coordinateType(),
                                                                                                            m_computation->setting()->value(PostprocessorSetting::View_ChartVariable).toString());

    // variable component
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) m_computation->setting()->value(PostprocessorSetting::View_ChartVariableComp).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // chart
    QString text;
    switch ((ChartAxisType) m_computation->setting()->value(PostprocessorSetting::View_ChartHorizontalAxis).toInt())
    {
    case ChartAxis_Length:
        text = tr("Length (m)");
        break;
    case ChartAxis_X:
        text = m_computation->config()->labelX() + " (m)";
        break;
    case ChartAxis_Y:
        text = m_computation->config()->labelY() + " (m)";
        break;
    default:
        assert(0);
    }

    m_chart->xAxis->setLabel(text);
    m_chart->yAxis->setLabel(QString("%1 (%2)").
                             arg(physicFieldVariable.name()).
                             arg(physicFieldVariable.unit()));

    // values
    ChartLine chartLine(Point(m_computation->setting()->value(PostprocessorSetting::View_ChartStartX).toDouble(),
                              m_computation->setting()->value(PostprocessorSetting::View_ChartStartY).toDouble()),
                        Point(m_computation->setting()->value(PostprocessorSetting::View_ChartEndX).toDouble(),
                              m_computation->setting()->value(PostprocessorSetting::View_ChartEndY).toDouble()),
                        m_computation->setting()->value(PostprocessorSetting::View_ChartHorizontalAxisPoints).toInt());

    QList<Point> points = chartLine.getPoints();
    QVector<double> xval = horizontalAxisValues(&chartLine);
    QVector<double> yval;

    foreach (Module::LocalVariable variable, m_computation->postDeal()->activeViewField()->localPointVariables(m_computation->config()->coordinateType()))
    {
        if (physicFieldVariable.id() != variable.id()) continue;

        foreach (Point point, points)
        {
            std::shared_ptr<LocalValue> localValue = m_computation->postDeal()->activeViewField()->plugin()->localValue(m_computation.data(),
                                                                                                                        m_computation->postDeal()->activeViewField(),
                                                                                                                        m_computation->postDeal()->activeTimeStep(),
                                                                                                                        m_computation->postDeal()->activeAdaptivityStep(),
                                                                                                                        point);
            QMap<QString, LocalPointValue> values = localValue->values();

            if (variable.isScalar())
            {
                yval.append(values[variable.id()].scalar);
            }
            else
            {
                if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                    yval.append(values[variable.id()].vector.x);
                else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                    yval.append(values[variable.id()].vector.y);
                else
                    yval.append(values[variable.id()].vector.magnitude());
            }
        }
    }

    assert(xval.count() == yval.count());

    // reverse x axis
    if (m_computation->setting()->value(PostprocessorSetting::View_ChartHorizontalAxisReverse).toBool())
    {
        for (int i = 0; i < points.length() / 2; i++)
        {
            double tmp = yval[i];
            yval[i] = yval[points.length() - i - 1];
            yval[points.length() - i - 1] = tmp;
        }
    }


    m_chart->graph(0)->setData(xval, yval);
}

void SceneViewChart::plotTime()
{
    if (m_computation->setting()->value(PostprocessorSetting::View_ChartVariable).toString().isEmpty())
        return;

    // variable
    Module::LocalVariable physicFieldVariable = m_computation->postDeal()->activeViewField()->localVariable(m_computation->config()->coordinateType(),
                                                                                                            m_computation->setting()->value(PostprocessorSetting::View_ChartVariable).toString());

    // variable comp
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) m_computation->setting()->value(PostprocessorSetting::View_ChartVariableComp).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // time levels
    QList<double> times = m_computation->timeStepTimes();

    // chart
    m_chart->xAxis->setLabel(tr("time (s)"));
    m_chart->yAxis->setLabel(QString("%1 (%2)").
                             arg(physicFieldVariable.name()).
                             arg(physicFieldVariable.unit()));

    // table
    QVector<double> xval;
    QVector<double> yval;

    for (int step = 0; step < times.count(); step++)
    {
        foreach (Module::LocalVariable variable, m_computation->postDeal()->activeViewField()->localPointVariables(m_computation->config()->coordinateType()))
        {
            if (physicFieldVariable.id() != variable.id()) continue;

            int adaptiveStep = m_computation->solutionStore()->lastAdaptiveStep(m_computation->postDeal()->activeViewField(), step);
            FieldSolutionID fsid(m_computation->postDeal()->activeViewField()->fieldId(), step, adaptiveStep);
            bool stepIsAvailable = m_computation->solutionStore()->contains(fsid);

            if (stepIsAvailable)
            {
                // change time level
                xval.append(times.at(step));

                Point point(m_computation->setting()->value(PostprocessorSetting::View_ChartTimeX).toDouble(),
                            m_computation->setting()->value(PostprocessorSetting::View_ChartTimeY).toDouble());
                std::shared_ptr<LocalValue> localValue = m_computation->postDeal()->activeViewField()->plugin()->localValue(m_computation.data(),
                                                                                                                            m_computation->postDeal()->activeViewField(),
                                                                                                                            step,
                                                                                                                            adaptiveStep,
                                                                                                                            point);
                QMap<QString, LocalPointValue> values = localValue->values();

                if (variable.isScalar())
                    yval.append(values[variable.id()].scalar);
                else
                {
                    if (physicFieldVariableComp == PhysicFieldVariableComp_X)
                        yval.append(values[variable.id()].vector.x);
                    else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
                        yval.append(values[variable.id()].vector.y);
                    else
                        yval.append(values[variable.id()].vector.magnitude());
                }
            }
        }
    }

    m_chart->graph(0)->setData(xval, yval);
}

void SceneViewChart::doSaveImage()
{
    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save image"), dir, tr("PNG files (*.png)"), &selectedFilter);
    if (fileName.isEmpty())
    {
        cerr << "Incorrect file name." << endl;
        return;
    }

    QFileInfo fileInfo(fileName);

    // open file for write
    if (fileInfo.suffix().isEmpty())
        fileName = fileName + ".png";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cerr << "Could not create " + fileName.toStdString() + " file." << endl;
        return;
    }

    m_chart->savePng(fileName, 1024, 768);
}

void SceneViewChart::doExportData()
{
    QSettings settings;
    QString dir = settings.value("General/LastDataDir").toString();

    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export data to file"), dir, tr("CSV files (*.csv)"), &selectedFilter);
    if (fileName.isEmpty())
    {
        cerr << "Incorrect file name." << endl;
        return;
    }

    QFileInfo fileInfo(fileName);

    // open file for write
    if (fileInfo.suffix().isEmpty())
        fileName = fileName + ".csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        cerr << "Could not create " + fileName.toStdString() + " file." << endl;
        return;
    }
    QTextStream out(&file);

    QMap<QString, QList<double> > table;
    if ((ChartMode) m_computation->setting()->value(PostprocessorSetting::View_ChartMode).toInt() == ChartMode_Geometry)
    {
        ChartLine chartLine(Point(m_computation->setting()->value(PostprocessorSetting::View_ChartStartX).toDouble(),
                                  m_computation->setting()->value(PostprocessorSetting::View_ChartStartY).toDouble()),
                            Point(m_computation->setting()->value(PostprocessorSetting::View_ChartEndX).toDouble(),
                                  m_computation->setting()->value(PostprocessorSetting::View_ChartEndY).toDouble()),
                            m_computation->setting()->value(PostprocessorSetting::View_ChartHorizontalAxisPoints).toInt());

        foreach (Point point, chartLine.getPoints())
        {
            QMap<QString, double> data = getData(point,
                                                 m_computation->postDeal()->activeTimeStep(),
                                                 m_computation->postDeal()->activeAdaptivityStep());
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }
    }
    else if ((ChartMode) m_computation->setting()->value(PostprocessorSetting::View_ChartMode).toInt() == ChartMode_Time)
    {
        Point point(m_computation->setting()->value(PostprocessorSetting::View_ChartTimeX).toDouble(),
                    m_computation->setting()->value(PostprocessorSetting::View_ChartTimeY).toDouble());
        QList<double> times = m_computation->timeStepTimes();
        foreach (int timeStep, m_computation->timeStepLengths())
        {
            QMap<QString, double> data = getData(point,
                                                 timeStep,
                                                 m_computation->solutionStore()->lastAdaptiveStep(m_computation->postDeal()->activeViewField(), timeStep));
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }
    }

    if (table.values().size() > 0)
    {
        // csv
        // headers
        foreach(QString key, table.keys())
            out << key << ";";
        out << "\n";

        // values
        for (int i = 0; i < table.values().first().size(); i++)
        {
            foreach(QString key, table.keys())
                out << QString::number(table.value(key).at(i)) << ";";
            out << endl;
        }

        if (fileInfo.absoluteDir() != tempProblemDir())
            settings.setValue("General/LastDataDir", fileInfo.absolutePath());

        file.close();
    }
}

QStringList SceneViewChart::headers()
{
    QStringList head;
    head << "x" << "y" << "t";

    foreach (Module::LocalVariable variable, m_computation->postDeal()->activeViewField()->localPointVariables(m_computation->config()->coordinateType()))
    {
        if (variable.isScalar())
        {
            // scalar variable
            head.append(variable.shortname());
        }
        else
        {
            // vector variable
            head.append(variable.shortname() + m_computation->config()->labelX().toLower());
            head.append(variable.shortname() + m_computation->config()->labelY().toLower());
            head.append(variable.shortname());
        }
    }

    return head;
}

QMap<QString, double> SceneViewChart::getData(Point point, int timeStep, int adaptivityStep)
{
    QMap<QString, double> table;
    table.insert(m_computation->config()->labelX(), point.x);
    table.insert(m_computation->config()->labelY(), point.y);
    table.insert("t", m_computation->timeStepToTotalTime(m_computation->postDeal()->activeTimeStep()));

    foreach (Module::LocalVariable variable, m_computation->postDeal()->activeViewField()->localPointVariables(m_computation->config()->coordinateType()))
    {
        std::shared_ptr<LocalValue> localValue = m_computation->postDeal()->activeViewField()->plugin()->localValue(m_computation.data(),
                                                                                                                    m_computation->postDeal()->activeViewField(),
                                                                                                                    timeStep,
                                                                                                                    adaptivityStep,
                                                                                                                    point);
        QMap<QString, LocalPointValue> values = localValue->values();

        if (variable.isScalar())
        {
            table.insert(variable.shortname(), values[variable.id()].scalar);
        }
        else
        {
            table.insert(QString(variable.shortname()), values[variable.id()].vector.magnitude());
            table.insert(QString(variable.shortname() + "x"), values[variable.id()].vector.x);
            table.insert(QString(variable.shortname() + "y"), values[variable.id()].vector.y);
        }
    }

    return table;
}

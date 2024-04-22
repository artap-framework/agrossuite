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
#include "sceneview_geometry_simple.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/solutionstore.h"
#include "solver/problem_config.h"

#include "gui/common.h"
#include "gui/lineeditdouble.h"
#include "gui/physicalfield.h"

#include <QtSvg/QSvgRenderer>

SceneViewChart::SceneViewChart(PostprocessorWidget *postprocessorWidget) : QWidget(postprocessorWidget), m_postprocessorWidget(postprocessorWidget)
{
    m_chartView = new ChartViewAxis(nullptr, this);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(m_chartView, 1);

    setLayout(layoutMain);

    refresh();
}

void SceneViewChart::refresh()
{
    if (!(m_postprocessorWidget->currentComputation() && m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()))
        return;

    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString().isEmpty())
        return;

    Module::LocalVariable physicFieldVariable = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localVariable(m_postprocessorWidget->currentComputation()->config()->coordinateType(),
                                                                                                                                   m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString());

    emit labelCenter(physicFieldVariable.name());

    if ((ChartMode) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartMode).toInt() == ChartMode_Geometry)
        plotGeometry();
    else if ((ChartMode) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartMode).toInt() == ChartMode_Time)
        plotTime();
}

QVector<double> SceneViewChart::horizontalAxisValues(ChartLine *chartLine)
{
    QList<Point> points = chartLine->getPoints();
    QVector<double> xval;

    switch ((ChartAxisType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxis).toInt())
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
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString().isEmpty())
        return;

    // variable
    Module::LocalVariable physicFieldVariable = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localVariable(m_postprocessorWidget->currentComputation()->config()->coordinateType(),
                                                                                                                                   m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString());

    // variable component
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariableComp).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // chart
    QString text;
    switch ((ChartAxisType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxis).toInt())
    {
    case ChartAxis_Length:
        text = tr("Length (m)");
        break;
    case ChartAxis_X:
        text = m_postprocessorWidget->currentComputation()->config()->labelX() + " (m)";
        break;
    case ChartAxis_Y:
        text = m_postprocessorWidget->currentComputation()->config()->labelY() + " (m)";
        break;
    default:
        assert(0);
    }

    // values
    ChartLine chartLine(Point(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartStartX).toDouble(),
                              m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartStartY).toDouble()),
                        Point(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartEndX).toDouble(),
                              m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartEndY).toDouble()),
                        m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxisPoints).toInt());

    QList<Point> points = chartLine.getPoints();
    QVector<double> xval = horizontalAxisValues(&chartLine);
    QVector<double> yval;

    foreach (Module::LocalVariable variable, m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localPointVariables(m_postprocessorWidget->currentComputation()->config()->coordinateType()))
    {
        if (physicFieldVariable.id() != variable.id()) continue;

        foreach (Point point, points)
        {
            std::shared_ptr<LocalValue> localValue = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->plugin()->localValue(m_postprocessorWidget->currentComputation().data(),
                                                                                                                                               m_postprocessorWidget->currentComputation()->postDeal()->activeViewField(),
                                                                                                                                               m_postprocessorWidget->currentComputation()->postDeal()->activeTimeStep(),
                                                                                                                                               m_postprocessorWidget->currentComputation()->postDeal()->activeAdaptivityStep(),
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
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxisReverse).toBool())
    {
        for (int i = 0; i < points.length() / 2; i++)
        {
            double tmp = yval[i];
            yval[i] = yval[points.length() - i - 1];
            yval[points.length() - i - 1] = tmp;
        }
    }

    // set labels
    m_chartView->chart()->axisX()->setTitleText(text);
    m_chartView->chart()->axisY()->setTitleText(QString("%1 (%2)").
                                                arg(physicFieldVariable.name()).
                                                arg(physicFieldVariable.unit()));

    // add to chart
    m_chartView->series()->clear();
    for (int i = 0; i < xval.size(); i++)
        m_chartView->series()->append(xval[i], yval[i]);

    // fit
    double minX = *std::min_element(xval.constBegin(), xval.constEnd());
    double maxX = *std::max_element(xval.constBegin(), xval.constEnd());
    m_chartView->axisX()->setRange(minX, maxX);
    double minY = *std::min_element(yval.constBegin(), yval.constEnd());
    double maxY = *std::max_element(yval.constBegin(), yval.constEnd());
    m_chartView->axisY()->setRange(minY, maxY);
}

void SceneViewChart::plotTime()
{
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString().isEmpty())
        return;

    // variable
    Module::LocalVariable physicFieldVariable = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localVariable(m_postprocessorWidget->currentComputation()->config()->coordinateType(),
                                                                                                                                   m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariable).toString());

    // variable comp
    PhysicFieldVariableComp physicFieldVariableComp = (PhysicFieldVariableComp) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartVariableComp).toInt();
    if (physicFieldVariableComp == PhysicFieldVariableComp_Undefined) return;

    // time levels
    QList<double> times = m_postprocessorWidget->currentComputation()->timeStepTimes();

    // table
    QVector<double> xval;
    QVector<double> yval;

    // set labels
    m_chartView->axisX()->setTitleText(tr("Time (s)"));
    m_chartView->axisY()->setTitleText(QString("%1 (%2)").
                          arg(physicFieldVariable.name()).
                          arg(physicFieldVariable.unit()));

    for (int step = 0; step < times.count(); step++)
    {
        foreach (Module::LocalVariable variable, m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localPointVariables(m_postprocessorWidget->currentComputation()->config()->coordinateType()))
        {
            if (physicFieldVariable.id() != variable.id()) continue;

            int adaptiveStep = m_postprocessorWidget->currentComputation()->solutionStore()->lastAdaptiveStep(m_postprocessorWidget->currentComputation()->postDeal()->activeViewField(), step);
            FieldSolutionID fsid(m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->fieldId(), step, adaptiveStep);
            bool stepIsAvailable = m_postprocessorWidget->currentComputation()->solutionStore()->contains(fsid);

            if (stepIsAvailable)
            {
                // change time level
                xval.append(times.at(step));

                Point point(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartTimeX).toDouble(),
                            m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartTimeY).toDouble());
                std::shared_ptr<LocalValue> localValue = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->plugin()->localValue(m_postprocessorWidget->currentComputation().data(),
                                                                                                                                                   m_postprocessorWidget->currentComputation()->postDeal()->activeViewField(),
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

    // add to chart
    m_chartView->series()->clear();
    for (int i = 0; i < xval.size(); i++)
        m_chartView->series()->append(xval[i], yval[i]);

    // fit
    double minX = *std::min_element(xval.constBegin(), xval.constEnd());
    double maxX = *std::max_element(xval.constBegin(), xval.constEnd());
    m_chartView->axisX()->setRange(minX, maxX);
    double minY = *std::min_element(yval.constBegin(), yval.constEnd());
    double maxY = *std::max_element(yval.constBegin(), yval.constEnd());
    m_chartView->axisY()->setRange(minY, maxY);
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

    m_chartView->grab().save(fileName);
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
    if ((ChartMode) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartMode).toInt() == ChartMode_Geometry)
    {
        ChartLine chartLine(Point(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartStartX).toDouble(),
                                  m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartStartY).toDouble()),
                            Point(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartEndX).toDouble(),
                                  m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartEndY).toDouble()),
                            m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartHorizontalAxisPoints).toInt());

        foreach (Point point, chartLine.getPoints())
        {
            QMap<QString, double> data = getData(point,
                                                 m_postprocessorWidget->currentComputation()->postDeal()->activeTimeStep(),
                                                 m_postprocessorWidget->currentComputation()->postDeal()->activeAdaptivityStep());
            foreach (QString key, data.keys())
            {
                QList<double> *values = &table.operator [](key);
                values->append(data.value(key));
            }
        }
    }
    else if ((ChartMode) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartMode).toInt() == ChartMode_Time)
    {
        Point point(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartTimeX).toDouble(),
                    m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ChartTimeY).toDouble());
        // QList<double> times = m_postprocessorWidget->currentComputation()->timeStepTimes();
        for(int timeStep = 0; timeStep < m_postprocessorWidget->currentComputation()->timeStepLengths().size(); timeStep++)
        {
            QMap<QString, double> data = getData(point,
                                                 timeStep,
                                                 m_postprocessorWidget->currentComputation()->solutionStore()->lastAdaptiveStep(m_postprocessorWidget->currentComputation()->postDeal()->activeViewField(), timeStep));
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
            out << Qt::endl;
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

    foreach (Module::LocalVariable variable, m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localPointVariables(m_postprocessorWidget->currentComputation()->config()->coordinateType()))
    {
        if (variable.isScalar())
        {
            // scalar variable
            head.append(variable.shortname());
        }
        else
        {
            // vector variable
            head.append(variable.shortname() + m_postprocessorWidget->currentComputation()->config()->labelX().toLower());
            head.append(variable.shortname() + m_postprocessorWidget->currentComputation()->config()->labelY().toLower());
            head.append(variable.shortname());
        }
    }

    return head;
}

QMap<QString, double> SceneViewChart::getData(Point point, int timeStep, int adaptivityStep)
{
    QMap<QString, double> table;
    table.insert(m_postprocessorWidget->currentComputation()->config()->labelX(), point.x);
    table.insert(m_postprocessorWidget->currentComputation()->config()->labelY(), point.y);
    table.insert("t", m_postprocessorWidget->currentComputation()->timeStepToTotalTime(timeStep));

    foreach (Module::LocalVariable variable, m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localPointVariables(m_postprocessorWidget->currentComputation()->config()->coordinateType()))
    {
        std::shared_ptr<LocalValue> localValue = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->plugin()->localValue(m_postprocessorWidget->currentComputation().data(),
                                                                                                                                           m_postprocessorWidget->currentComputation()->postDeal()->activeViewField(),
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

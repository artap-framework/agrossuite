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

#ifndef SCENEVIEWGEOMETRYCHART_H
#define SCENEVIEWGEOMETRYCHART_H

#include "util.h"

#include "sceneview_common2d.h"
#include "postprocessorview_chart.h"

class PostprocessorWidget;

class SceneViewSimpleGeometry : public SceneViewCommon2D
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();

public:
    SceneViewSimpleGeometry(QWidget *parent);
    virtual ~SceneViewSimpleGeometry();

    void setProblem(QSharedPointer<ProblemBase> problem) { m_problem = problem; updateGL(); }

protected:
    QSharedPointer<ProblemBase> m_problem;

    virtual void doZoomRegion(const Point &start, const Point &end);

    virtual ProblemBase *problem() const { return m_problem.data(); }

    void paintGL();
    void paintGeometry(); // paint nodes, edges and labels
};

class SceneViewSimpleGeometryChart : public SceneViewSimpleGeometry
{
    Q_OBJECT

public:
    SceneViewSimpleGeometryChart(QWidget *parent);
    virtual ~SceneViewSimpleGeometryChart();

    void setChartLine(const ChartLine &chartLine);

protected:
    void paintGL();
    void paintChartLine();

    ChartLine m_chartLine;
};

#endif // SCENEVIEWGEOMETRYCHART_H

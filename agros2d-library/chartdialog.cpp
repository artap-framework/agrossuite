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


SceneViewChart::SceneViewChart(QWidget *parent) : QWidget(parent)
{
    m_chart = new QCustomPlot(this);
    m_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_chart->addGraph();

    m_chart->graph(0)->setLineStyle(QCPGraph::lsLine);

    QHBoxLayout *layoutMain = new QHBoxLayout();
    layoutMain->addWidget(m_chart);

    setLayout(layoutMain);

    setControls();

    // reconnect computation slots
    connect(Agros2D::singleton(), SIGNAL(connectComputation(QSharedPointer<ProblemComputation>)), this, SLOT(connectComputation(QSharedPointer<ProblemComputation>)));
}

void SceneViewChart::setControls()
{    
}

void SceneViewChart::connectComputation(QSharedPointer<ProblemComputation> computation)
{        
    if (!m_computation.isNull())
    {
        disconnect(m_computation.data()->scene(), SIGNAL(cleared()), this, SLOT(setControls()));
        disconnect(m_computation.data()->scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
        disconnect(m_computation.data(), SIGNAL(meshed()), this, SLOT(setControls()));
        disconnect(m_computation.data(), SIGNAL(solved()), this, SLOT(setControls()));
    }

    m_computation = computation;

    connect(m_computation.data()->scene(), SIGNAL(cleared()), this, SLOT(setControls()));
    connect(m_computation.data()->scene(), SIGNAL(invalidated()), this, SLOT(setControls()));
    connect(m_computation.data(), SIGNAL(meshed()), this, SLOT(setControls()));
    connect(m_computation.data(), SIGNAL(solved()), this, SLOT(setControls()));
}

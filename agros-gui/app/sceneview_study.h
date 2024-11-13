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

#ifndef SCENEVIEW_STUDY_H
#define SCENEVIEW_STUDY_H

#include "util/util.h"

#include "sceneview_geometry.h"
#include "postprocessorview_chart.h"
#include "solver/problem.h"

class PostprocessorWidget;

class SceneViewStudy : public SceneViewProblem
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();

public:
    SceneViewStudy(QWidget *parent);
    virtual ~SceneViewStudy() {}

protected:
    void paintGL();
    void paintGeometryStudy(); // paint nodes, edges and labels
};


#endif // SCENEVIEW_STUDY_H

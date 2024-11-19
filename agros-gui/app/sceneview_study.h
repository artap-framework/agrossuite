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

class Study;
class Parameter;
class ResultRecipe;
class PostprocessorWidget;

class SceneViewStudy : public SceneViewCommon2D
{
    Q_OBJECT

public slots:
    virtual void clear();
    virtual void refresh();

public:
    enum InteractionMode
    {
        NoInteraction = -1,
        SelectPoint = 0,
        SelectSurface = 1,
        SelectVolume = 2
    };

    SceneViewStudy(QWidget *parent, bool showRulers = true);
    virtual ~SceneViewStudy() {}

    inline void setInteractionMode(InteractionMode userMode) { m_userMode = userMode; }
    inline void setRecipe(ResultRecipe *recipe) { m_recipe = recipe; }
    inline void setParameter(Parameter *parameter) { m_parameter = parameter; }
    inline void setSelectedPoint(const Point &selectedPoint) { m_selectedPoint = selectedPoint; }

    virtual ProblemBase *problem() const;

private:

protected:
    void paintGL();

    void mousePressEvent(QMouseEvent *event);

    void doZoomRegion(const Point &start, const Point &end) override;

    void paintGeometryStudy();
    void paintGeometryStudyEdges(const double color[]);
    void paintGeometryStudyVolume() const;

    void paintGeometryStudySelectedPoint(const Point &selectedPoint) const;
    void paintGeometryStudySelectedEdges() const;
    void paintGeometryStudySelectedVolume() const;

    QSharedPointer<Computation> m_currentComputation;

    ResultRecipe *m_recipe;
    Parameter *m_parameter;
    Point m_selectedPoint;

    bool m_showRulers;
    InteractionMode m_userMode;
};


#endif // SCENEVIEW_STUDY_H

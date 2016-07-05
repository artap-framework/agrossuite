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

#ifndef SCENEVIEW_POST_H
#define SCENEVIEW_POST_H

#include "util.h"
#include "sceneview_common.h"

template <typename Scalar> class SceneSolution;

class MultiArray;
class PostprocessorWidget;

class ParticleTracing;
class FieldInfo;
class Computation;
class PostprocessorSetting;
class PostTriangle;

class PostDeal : public QObject
{
    Q_OBJECT

public:
    PostDeal(Computation *parentProblem);
    ~PostDeal();

    // contour
    inline QList<PostTriangle> &contourValues() { return m_contourValues; }

    // scalar view
    inline QList<PostTriangle> &scalarValues() { return m_scalarValues; }

    // vector view
    inline QList<PostTriangle> &vectorXValues() { return m_vectorXValues; }
    inline QList<PostTriangle> &vectorYValues() { return m_vectorYValues; }

    std::shared_ptr<PostDataOut> viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                  PhysicFieldVariableComp physicFieldVariableComp);

    // view
    inline FieldInfo* activeViewField() const { return m_activeViewField; }
    void setActiveViewField(FieldInfo* fieldInfo);

    inline int activeTimeStep() const { return m_activeTimeStep; }
    void setActiveTimeStep(int ts);

    inline int activeAdaptivityStep() const { return m_activeAdaptivityStep; }
    void setActiveAdaptivityStep(int as);

    MultiArray activeMultiSolutionArray();

    inline bool isProcessed() const { return m_isProcessed; }

signals:
    void processed();

public slots:
    void refresh();
    void clear();
    void clearView();

private:
    Computation *m_computation;

    bool m_isProcessed;

    // contour
    QList<PostTriangle> m_contourValues;
    // scalar view
    QList<PostTriangle> m_scalarValues;
    // vector view
    QList<PostTriangle> m_vectorXValues;
    QList<PostTriangle> m_vectorYValues;

    // view
    FieldInfo *m_activeViewField;
    int m_activeTimeStep;
    int m_activeAdaptivityStep;

    // stored shared pointers for keeping the instance around
    std::shared_ptr<dealii::DataPostprocessorScalar<2> > m_post;

private slots:
    void processSolved();

    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();

    virtual void clearGLLists() {}
};

class SceneViewPostInterface
{
public:
    SceneViewPostInterface(PostprocessorWidget *postprocessorWidget);

protected:
    double m_texScale;
    double m_texShift;

    GLuint m_textureScalar;

    void paintBackground(); // gradient background
    void paintScalarFieldColorBar(SceneViewCommon *sceneView, double min, double max);

    // palette
    const double *paletteColor(double x) const;
    const double *paletteColorOrder(int n) const;
    void paletteCreate();

private:
    PostprocessorWidget *m_postprocessorWidget;
};

#endif // SCENEVIEW_POST_H

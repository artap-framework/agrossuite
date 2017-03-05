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

#include "util/util.h"
#include "sceneview_common.h"

template <typename Scalar> class SceneSolution;

class MultiArray;
class PostprocessorWidget;

class ParticleTracing;
class FieldInfo;
class Computation;
class PostprocessorSetting;
class PostTriangle;
class PostDataOut;

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

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

#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/data_postprocessor.h>
#include <deal.II/grid/filtered_iterator.h>
#include <deal.II/hp/dof_handler.h>

#include "sceneview_post.h"
#include "postprocessorview.h"

#include "util/global.h"

#include "sceneview_common2d.h"
#include "sceneview_data.h"
#include "scene.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "scenemarkerselectdialog.h"
#include "scenebasicselectdialog.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "logview.h"

#include "solver/plugin_interface.h"
#include "solver/module.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutiontypes.h"
#include "solver/solutionstore.h"

SceneViewPostInterface::SceneViewPostInterface(PostprocessorWidget *postprocessorWidget)
    : m_postprocessorWidget(postprocessorWidget), m_textureScalar(0), m_texScale(0.0), m_texShift(0.0)
{
}

const double* SceneViewPostInterface::paletteColor(double x) const
{
    switch ((PaletteType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::PaletteType).toInt())
    {
    case Palette_Inferno:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataInferno[n];
    }
        break;
    case Palette_Viridis:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataViridis[n];
    }
        break;
    case Palette_Jet:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataJet[n];
    }
        break;
    case Palette_Agros:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataAgros[n];
    }
        break;
    case Palette_HSV:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataHSV[n];
    }
        break;
    case Palette_BWAsc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = x;
        return color;
    }
        break;
    case Palette_BWDesc:
    {
        static double color[3];
        color[0] = color[1] = color[2] = 1.0 - x;
        return color;
    }
        break;
    case Palette_Paruly:
    default:
    {
        if (x < 0.0) x = 0.0;
        else if (x > 1.0) x = 1.0;
        x *= PALETTEENTRIES;
        int n = (int) x;
        return paletteDataParuly[n];
    }
        break;
    }
}

const double* SceneViewPostInterface::paletteColorOrder(int n) const
{
    switch ((PaletteType) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::OrderPaletteOrderType).toInt())
    {
    case Palette_Jet:
        return paletteOrderJet[n];
    case Palette_Inferno:
        return paletteOrderInferno[n];
    case Palette_Viridis:
        return paletteOrderViridis[n];
    case Palette_Agros:
        return paletteOrderAgros[n];
    case Palette_HSV:
        return paletteOrderHSV[n];
    case Palette_BWAsc:
        return paletteOrderBWAsc[n];
    case Palette_BWDesc:
        return paletteOrderBWDesc[n];
    case Palette_Paruly:
    default:
        return paletteOrderParuly[n];
    }
}

void SceneViewPostInterface::paletteCreate()
{
    int paletteSteps = m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::PaletteFilter).toBool()
            ? 100 : m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::PaletteSteps).toInt();

    unsigned char palette[256][3];
    for (int i = 0; i < paletteSteps; i++)
    {
        const double* color = paletteColor((double) i / paletteSteps);
        palette[i][0] = (unsigned char) (color[0] * 255);
        palette[i][1] = (unsigned char) (color[1] * 255);
        palette[i][2] = (unsigned char) (color[2] * 255);
    }
    for (int i = paletteSteps; i < 256; i++)
        memcpy(palette[i], palette[paletteSteps-1], 3);

    // TODO: check it! (should we call makeCurrent()?) makeCurrent();
    if (glIsTexture(m_textureScalar))
        glDeleteTextures(1, &m_textureScalar);
    glGenTextures(1, &m_textureScalar);

    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP, GL_TRUE);
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::PaletteFilter).toBool())
    {
#ifdef Q_WS_WIN
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#endif
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
#ifdef Q_WS_WIN
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
#endif
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexImage1D(GL_TEXTURE_1D, 0, 3, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, palette);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

    // adjust palette
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::PaletteFilter).toBool())
    {
        m_texScale = (double) (paletteSteps-1) / 256.0;
        m_texShift = 0.5 / 256.0;
    }
    else
    {
        m_texScale = (double) paletteSteps / 256.0;
        m_texShift = 0.0;
    }
}

void SceneViewPostInterface::paintScalarFieldColorBar(SceneViewCommon *sceneView, double min, double max)
{
    if (!m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ShowScalarColorBar).toBool()) return;

    sceneView->loadProjectionViewPort();

    glScaled(2.0 / sceneView->width(), 2.0 / sceneView->height(), 1.0);
    glTranslated(-sceneView->width() / 2.0, -sceneView->height() / 2.0, 0.0);

    // dimensions
    int textWidth = (sceneView->m_charDataPost[GLYPH_M].x1 - sceneView->m_charDataPost[GLYPH_M].x0)
            * (QString::number(-1.0, 'e', m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarDecimalPlace).toInt()).length() + 1);
    int textHeight = 2 * (sceneView->m_charDataPost[GLYPH_M].y1 - sceneView->m_charDataPost[GLYPH_M].y0);
    Point scaleSize = Point(45.0 + textWidth, 20*textHeight); // height() - 20.0
    Point scaleBorder = Point(10.0, (Agros::configComputer()->value(Config::Config_ShowRulers).toBool()) ? 1.8 * textHeight : 10.0);
    double scaleLeft = (sceneView->width() - (45.0 + textWidth));
    int numTicks = 11;

    // blended rectangle
    sceneView->drawBlend(Point(scaleLeft, scaleBorder.y), Point(scaleLeft + scaleSize.x - scaleBorder.x, scaleBorder.y + scaleSize.y),
                         0.91, 0.91, 0.91);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // palette border
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 50.0);
    glVertex2d(scaleLeft + 10.0, scaleBorder.y + 10.0);
    glVertex2d(scaleLeft + 30.0, scaleBorder.y + 10.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    // palette
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_textureScalar);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glBegin(GL_QUADS);
    if (fabs(max - min) > EPS_ZERO)
        glTexCoord1d(m_texScale + m_texShift);
    else
        glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + scaleSize.y - 52.0);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + scaleSize.y - 52.0);
    glTexCoord1d(m_texShift);
    glVertex2d(scaleLeft + 12.0, scaleBorder.y + 12.0);
    glVertex2d(scaleLeft + 28.0, scaleBorder.y + 12.0);
    glEnd();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_1D);

    // ticks
    glLineWidth(1.0);
    glBegin(GL_LINES);
    for (int i = 1; i < numTicks; i++)
    {
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        glVertex2d(scaleLeft + 10.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 15.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 25.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
        glVertex2d(scaleLeft + 30.0, scaleBorder.y + scaleSize.y - 49.0 - i*tickY);
    }
    glEnd();

    // line
    glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2d(scaleLeft + 5.0, scaleBorder.y + scaleSize.y - 31.0);
    glVertex2d(scaleLeft + scaleSize.x - 15.0, scaleBorder.y + scaleSize.y - 31.0);
    glEnd();

    // labels
    for (int i = 1; i < numTicks+1; i++)
    {
        double value = 0.0;
        if (!m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeLog).toBool())
            value = min + (double) (i-1) / (numTicks-1) * (max - min);
        else
            value = min + (double) pow((double) m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeBase).toInt(),
                                       ((i-1) / (numTicks-1)))/m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarRangeBase).toInt() * (max - min);

        if (fabs(value) < EPS_ZERO) value = 0.0;
        double tickY = (scaleSize.y - 60.0) / (numTicks - 1.0);

        sceneView->printPostAt(scaleLeft + 33.0 + ((value >= 0.0) ? (sceneView->m_charDataPost[GLYPH_M].x1 - sceneView->m_charDataPost[GLYPH_M].x0) : 0.0),
                               scaleBorder.y + 10.0 + (i-1)*tickY - textHeight / 4.0,
                               QString::number(value, 'e', m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarDecimalPlace).toInt()));
    }

    // variable
    // TODO: add variable
    Module::LocalVariable localVariable = m_postprocessorWidget->currentComputation()->postDeal()->activeViewField()->localVariable(m_postprocessorWidget->currentComputation()->config()->coordinateType(),
                                                                                                                                    m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString());
    QString str = QString("%1 (%2)").
            arg(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString().isEmpty() ? "" : localVariable.shortname()).
            arg(m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarVariable).toString().isEmpty() ? "" : localVariable.unit());

    sceneView->printPostAt(scaleLeft + scaleSize.x / 2.0 - (sceneView->m_charDataPost[GLYPH_M].x1 - sceneView->m_charDataPost[GLYPH_M].x0) * str.count() / 2.0,
                           scaleBorder.y + scaleSize.y - 20.0,
                           str);
}


void SceneViewPostInterface::paintBackground()
{
    // background
    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBegin(GL_QUADS);
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarView3DBackground).toBool())
        glColor3d(0.99, 0.99, 0.99);
    else
        glColor3d(COLORBACKGROUND[0], COLORBACKGROUND[1], COLORBACKGROUND[2]);

    glVertex3d(-1.0, -1.0, 0.0);
    glVertex3d(1.0, -1.0, 0.0);
    if (m_postprocessorWidget->currentComputation()->setting()->value(PostprocessorSetting::ScalarView3DBackground).toBool())
        glColor3d(0.44, 0.56, 0.89);
    glVertex3d(1.0, 1.0, 0.0);
    glVertex3d(-1.0, 1.0, 0.0);
    glEnd();

    glDisable(GL_POLYGON_OFFSET_FILL);

    glPopMatrix();
}

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

#ifndef SCENEVIEWCOMMON_H
#define SCENEVIEWCOMMON_H

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QtOpenGL/QOpenGLDebugMessage>

#include "gui/other.h"

#include "util/util.h"
#include "util/point.h"
#include "util/enums.h"
#include "util/conf.h"

class Scene;
class SceneViewCommon;

class SceneNode;
class SceneFace;
class SceneLabel;

class SceneViewInterface;

class ProblemBase;

namespace Module
{
    class LocalVariable;
}

class SceneViewCommon : public QOpenGLWidget
{
    Q_OBJECT

public slots:
    void doZoomBestFit();
    void doZoomIn();
    void doZoomOut();
    virtual void doZoomRegion(const Point &start, const Point &end) = 0;

    virtual void refresh();
    virtual void clear() = 0;

public:
    SceneViewCommon(QWidget *parent = 0);
    virtual ~SceneViewCommon();

    // actions
    QAction *actSceneZoomIn;
    QAction *actSceneZoomOut;
    QAction *actSceneZoomBestFit;
    QAction *actSceneZoomRegion;

    void saveImageToFile(const QString &fileName, int w = 0, int h = 0);
    QPixmap renderScenePixmap(int w = 0, int h = 0, bool useContext = false);

signals:
    void mouseMoved(const Point &position);
    void mousePressed();
    void mousePressed(const Point &point);
    void postprocessorModeGroupChanged(SceneModePostprocessor sceneModePostprocessor);
    void mouseSceneModeChanged(MouseSceneMode mouseSceneMode);

protected:
    QPoint m_lastPos; // last position of cursor
    SceneNode *m_nodeLast;

    // helper for zoom region
    bool m_zoomRegion;
    QPointF m_zoomRegionPos;

    // problem (preprocessor vs. computation)
    virtual ProblemBase *problem() const = 0;

    void createActions();

    void drawArc(const Point &point, double r, double startAngle, double arcAngle, int segments = -1) const;
    void drawBlend(Point start, Point end, double red = 1.0, double green = 1.0, double blue = 1.0, double alpha = 0.75) const;

    void printAt(int penX, int penY, const QString &text, int fontSize);

    int m_labelRulersSize;
    int m_labelPostSize;

    void printRulersAt(int penX, int penY, const QString &text);
    void printPostAt(int penX, int penY, const QString &text);

    virtual void setZoom(double power) = 0;

    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL() = 0;    
    void setupViewport(int w, int h);
    void loadProjectionViewPort();

    void closeEvent(QCloseEvent *event);

    inline double aspect() const { return (double) width() / (double) height(); }

    friend class SceneViewPostInterface;

private slots:
#if QT_VERSION > 0x050100
    void messageLogged(QOpenGLDebugMessage message);
#endif
};

#endif // SCENEVIEWCOMMON_H

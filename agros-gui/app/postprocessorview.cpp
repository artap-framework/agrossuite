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

#include "postprocessorview.h"
#include "postprocessorview_mesh.h"
#include "postprocessorview_post2d.h"
#include "postprocessorview_post3d.h"
#include "postprocessorview_chart.h"
// #include "postprocessorview_particletracing.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "sceneview_particle.h"
#include "chartdialog.h"
#include "videodialog.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorWidget::PostprocessorWidget()
{
    // scene mode
    actSceneModeResults = new QAction(icon("main_results"), tr("Results"), this);
    actSceneModeResults->setShortcut(tr("Ctrl+3"));
    actSceneModeResults->setCheckable(true);
    actSceneModeResults->setEnabled(false);

    actOperateOnMesh = new QAction(icon("results_mesh"), tr("Mesh"), this);
    actOperateOnMesh->setShortcut(Qt::Key_F2);
    actOperateOnMesh->setCheckable(true);

    actOperateOnPost2D = new QAction(icon("results_post2d"), tr("Post 2D"), this);
    actOperateOnPost2D->setShortcut(Qt::Key_F3);
    actOperateOnPost2D->setCheckable(true);
    actOperateOnPost2D->setChecked(true);

    actOperateOnPost3D = new QAction(icon("results_post3d"), tr("Post 3D"), this);
    actOperateOnPost3D->setShortcut(Qt::Key_F4);
    actOperateOnPost3D->setCheckable(true);

    actOperateOnChart = new QAction(icon("results_chart"), tr("Chart"), this);
    actOperateOnChart->setShortcut(Qt::Key_F5);
    actOperateOnChart->setCheckable(true);

    actOperateGroup = new QActionGroup(this);
    actOperateGroup->setExclusive(true);
    actOperateGroup->addAction(actOperateOnMesh);
    actOperateGroup->addAction(actOperateOnPost2D);
    actOperateGroup->addAction(actOperateOnPost3D);
    actOperateGroup->addAction(actOperateOnChart);
    connect(actOperateGroup, SIGNAL(triggered(QAction *)), this, SLOT(doPostModeSet(QAction *)));

    m_fieldWidget = new PhysicalFieldWidget(this);
    connect(m_fieldWidget, SIGNAL(fieldChanged()), this, SLOT(refresh()));

    m_sceneViewMesh = new SceneViewMesh(this);
    m_meshWidget = new PostprocessorSceneMeshWidget(m_fieldWidget, m_sceneViewMesh);

    m_sceneViewPost2D = new SceneViewPost2D(this);
    m_post2DWidget = new PostprocessorScenePost2DWidget(m_fieldWidget, m_sceneViewPost2D);

    m_sceneViewPost3D = new SceneViewPost3D(this);
    m_post3DWidget = new PostprocessorScenePost3DWidget(m_fieldWidget, m_sceneViewPost3D);

    m_sceneViewChart = new SceneViewChart(this);
    m_chartWidget = new PostprocessorSceneChartWidget(m_fieldWidget, m_sceneViewChart);

    // default view
    m_postMode = PostprocessorWidgetMode_Post2D;

    createControls();
}

void PostprocessorWidget::createControls()
{    
    // dialog buttons
    btnApply = new QPushButton(tr("Apply"));
    connect(btnApply, SIGNAL(clicked()), SLOT(apply()));

    actExportPostprocessorToClipboard = new QAction(tr("Copy view to clipboard"), this);
    connect(actExportPostprocessorToClipboard, SIGNAL(triggered()), this, SLOT(exportPostprocessorToClipboard()));

    actExportPostprocessorToBitmap = new QAction(tr("Export view to bitmap..."), this);
    connect(actExportPostprocessorToBitmap, SIGNAL(triggered()), this, SLOT(exportPostprocessorToBitmap()));

    actExportVideo = new QAction(tr("Create video..."), this);
    connect(actExportVideo, SIGNAL(triggered()), this, SLOT(createVideo()));

    auto *layoutButtons = new QHBoxLayout();
    layoutButtons->setContentsMargins(10, 2, 10, 6);
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnApply);

    tabControlWidgetLayout = new QStackedLayout();
    tabControlWidgetLayout->addWidget(m_meshWidget);
    tabControlWidgetLayout->addWidget(m_post2DWidget);
    tabControlWidgetLayout->addWidget(m_post3DWidget);
    tabControlWidgetLayout->addWidget(m_chartWidget);

    auto *layoutLeft = new QVBoxLayout();
    // layoutLeft->setContentsMargins(2, 2, 2, 3);
    layoutLeft->setContentsMargins(0, 0, 0, 0);
    layoutLeft->addWidget(m_fieldWidget);
    layoutLeft->addLayout(tabControlWidgetLayout);
    layoutLeft->addLayout(layoutButtons);

    tabViewLayout = new QStackedLayout();
    tabViewLayout->addWidget(m_sceneViewMesh);
    tabViewLayout->addWidget(m_sceneViewPost2D);
    tabViewLayout->addWidget(m_sceneViewPost3D);
    tabViewLayout->addWidget(m_sceneViewChart);

    auto viewWidget = new QWidget();
    viewWidget->setContentsMargins(2, 2, 2, 3);
    viewWidget->setLayout(tabViewLayout);

    // export
    auto *mnuExport = new QMenu(this);
    mnuExport->addAction(actExportPostprocessorToClipboard);
    mnuExport->addSeparator();
    mnuExport->addAction(actExportPostprocessorToBitmap);
    mnuExport->addSeparator();
    mnuExport->addAction(m_sceneViewPost2D->actExportVTKScalar);
    mnuExport->addSeparator();
    mnuExport->addAction(actExportVideo);

    auto *exportButton = new QToolButton();
    exportButton->setText(tr("Export"));
    exportButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    exportButton->setIconSize(QSize(24, 24));
    exportButton->setMenu(mnuExport);
    exportButton->setAutoRaise(true);
    exportButton->setIcon(icon("export"));
    exportButton->setPopupMode(QToolButton::InstantPopup);

    // zoom
    mnuZoom = new QMenu(this);

    zoomButton = new QToolButton();
    zoomButton->setText(tr("Zoom"));
    zoomButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    zoomButton->setIconSize(QSize(24, 24));
    zoomButton->setMenu(mnuZoom);
    zoomButton->setAutoRaise(true);
    zoomButton->setIcon(icon("geometry_zoom"));
    zoomButton->setPopupMode(QToolButton::InstantPopup);

    // right toolbar
    toolBarRight = new QToolBar();
    toolBarRight->setProperty("modulebar", true);
    toolBarRight->setProperty("os", operatingSystem());
    toolBarRight->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolBarRight->addAction(actOperateOnMesh);
    toolBarRight->addAction(actOperateOnPost2D);
    toolBarRight->addAction(actOperateOnPost3D);
    toolBarRight->addAction(actOperateOnChart);
    toolBarRight->addSeparator();
    toolBarRight->addWidget(zoomButton);
    toolBarRight->addSeparator();
    toolBarRight->addWidget(exportButton);

    auto layoutRight = new QVBoxLayout();
    layoutRight->setContentsMargins(0, 0, 0, 0);
    layoutRight->addWidget(toolBarRight);
    layoutRight->addWidget(viewWidget);

    auto layoutMain = new QHBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addLayout(layoutLeft);
    layoutMain->addLayout(layoutRight);
    layoutMain->setStretch(1, 1);

    setLayout(layoutMain);
}

void PostprocessorWidget::solveFinished()
{
    // clear commputation
    m_currentComputation.clear();

    refresh();

    actSceneModeResults->trigger();

    m_sceneViewMesh->doZoomBestFit();
    m_sceneViewPost2D->doZoomBestFit();
    m_sceneViewPost3D->doZoomBestFit();
    // m_sceneViewParticleTracing->doZoomBestFit();
}

void PostprocessorWidget::apply()
{
    if (m_currentComputation->isSolved())
    {
        // save settings
        if (m_postMode == PostprocessorWidgetMode_Mesh)
            m_meshWidget->save();
        else if (m_postMode == PostprocessorWidgetMode_Post2D)
            m_post2DWidget->save();
        else if (m_postMode == PostprocessorWidgetMode_Post3D)
            m_post3DWidget->save();
        else if (m_postMode == PostprocessorWidgetMode_Chart)
            m_chartWidget->save();

        // refresh view
        m_currentComputation->postDeal()->setActiveViewField(m_fieldWidget->selectedField());
        m_currentComputation->postDeal()->setActiveTimeStep(m_fieldWidget->selectedTimeStep());
        m_currentComputation->postDeal()->setActiveAdaptivityStep(m_fieldWidget->selectedAdaptivityStep());
        m_currentComputation->postDeal()->refresh();

        // refresh view
        m_sceneViewMesh->refresh();
        m_sceneViewPost2D->refresh();
        m_sceneViewPost3D->refresh();
        m_sceneViewChart->refresh();

        emit changed();
    }
}

void PostprocessorWidget::refresh()
{
    // enable widget
    bool enabled = false;
    foreach (QSharedPointer<Computation> computation, Agros::computations().values())
    {
        if (computation->isSolved())
        {
            enabled = true;
            break;
        }
    }
    actSceneModeResults->setEnabled(enabled);

    // reset computations
    if (!enabled)
        m_currentComputation.clear();

    // update field widget
    m_fieldWidget->updateControls();

    // set current computation
    bool computationChanged = m_currentComputation != m_fieldWidget->selectedComputation();
    if (computationChanged)
    {
        m_currentComputation = m_fieldWidget->selectedComputation();

        if (!m_currentComputation.isNull())
        {
            // refresh widgets
            m_meshWidget->refresh();
            m_post2DWidget->refresh();
            m_post3DWidget->refresh();
            m_chartWidget->refresh();

            // clear post view
            m_currentComputation->postDeal()->clear();

            // refresh views
            m_sceneViewMesh->setControls();
            m_sceneViewPost2D->setControls();
            m_sceneViewPost3D->setControls();
            m_sceneViewChart->refresh();
        }
    }

    if (m_currentComputation.isNull())
        return;

    m_meshWidget->load();
    m_post2DWidget->load();
    m_post3DWidget->load();
    m_chartWidget->load();

    mnuZoom->clear();
    switch (m_postMode)
    {
    case PostprocessorWidgetMode_Mesh:
    {
        tabControlWidgetLayout->setCurrentWidget(m_meshWidget);
        tabViewLayout->setCurrentWidget(m_sceneViewMesh);
        m_sceneViewMesh->refresh();

        zoomButton->setEnabled(true);
        mnuZoom->addAction(m_sceneViewMesh->actSceneZoomBestFit);
        mnuZoom->addAction(m_sceneViewMesh->actSceneZoomIn);
        mnuZoom->addAction(m_sceneViewMesh->actSceneZoomOut);
    }
    break;
    case PostprocessorWidgetMode_Post2D:
    {
        tabControlWidgetLayout->setCurrentWidget(m_post2DWidget);
        tabViewLayout->setCurrentWidget(m_sceneViewPost2D);
        m_sceneViewPost2D->refresh();

        zoomButton->setEnabled(true);
        mnuZoom->addAction(m_sceneViewPost2D->actSceneZoomBestFit);
        mnuZoom->addAction(m_sceneViewPost2D->actSceneZoomIn);
        mnuZoom->addAction(m_sceneViewPost2D->actSceneZoomOut);
    }
    break;

    case PostprocessorWidgetMode_Post3D:
    {
        tabControlWidgetLayout->setCurrentWidget(m_post3DWidget);
        tabViewLayout->setCurrentWidget(m_sceneViewPost3D);
        m_sceneViewPost3D->refresh();

        zoomButton->setEnabled(true);
        mnuZoom->addAction(m_sceneViewPost3D->actSceneZoomBestFit);
        mnuZoom->addAction(m_sceneViewPost3D->actSceneZoomIn);
        mnuZoom->addAction(m_sceneViewPost3D->actSceneZoomOut);
    }
    break;
    case PostprocessorWidgetMode_Chart:
    {
        tabControlWidgetLayout->setCurrentWidget(m_chartWidget);
        tabViewLayout->setCurrentWidget(m_sceneViewChart);

        zoomButton->setEnabled(false);
    }
    break;
    default:
        break;
    }

    if (computationChanged)
        apply();
}

void PostprocessorWidget::clearedComputation()
{
    m_currentComputation.clear();
    refresh();
}

void PostprocessorWidget::processed()
{
    // refresh widgets
    m_meshWidget->refresh();
    m_post2DWidget->refresh();
    m_post3DWidget->refresh();
    m_chartWidget->refresh();
    // m_particleTracingWidget->refresh();

    m_sceneViewMesh->refresh();
    m_sceneViewPost2D->refresh();
    m_sceneViewPost3D->refresh();
    m_sceneViewChart->refresh();
    // m_sceneViewParticleTracing->refresh();
}

void PostprocessorWidget::createVideo()
{
    VideoDialog *videoDialog = nullptr;

    switch (mode())
    {
    case PostprocessorWidgetMode_Mesh:
        videoDialog = new VideoDialog(m_sceneViewMesh, currentComputation().data(), this);
        break;
    case PostprocessorWidgetMode_Post2D:
        videoDialog = new VideoDialog(m_sceneViewPost2D, currentComputation().data(), this);
        break;
    case PostprocessorWidgetMode_Post3D:
        videoDialog = new VideoDialog(m_sceneViewPost3D, currentComputation().data(), this);
        break;
    default:
        break;
    }

    if (videoDialog)
    {
        videoDialog->showDialog();
        delete videoDialog;
    }
}


void PostprocessorWidget::exportPostprocessorToClipboard()
{
    // copy image to clipboard
    QPixmap pixmap;

    switch (mode())
    {
    case PostprocessorWidgetMode_Mesh:
        pixmap = m_sceneViewMesh->renderScenePixmap();
        break;
    case PostprocessorWidgetMode_Post2D:
        pixmap = m_sceneViewPost2D->renderScenePixmap();
        break;
    case PostprocessorWidgetMode_Post3D:
        pixmap = m_sceneViewPost3D->renderScenePixmap();
        break;
    case PostprocessorWidgetMode_Chart:
        pixmap = m_sceneViewChart->grab();
        break;
    default:
        break;
    }

    QApplication::clipboard()->setImage(pixmap.toImage());
}

void PostprocessorWidget::exportPostprocessorToBitmap()
{
    QSettings settings;
    QString dir = settings.value("General/LastImageDir").toString();

    QString selectedFilter;
    QString fn = QFileDialog::getSaveFileName(QApplication::activeWindow(), tr("Export view to file"), dir, "PNG files (*.png);;JPG files (*.jpg);;BMP files (*.bmp)", &selectedFilter);
    if (fn.isEmpty())
        return;

    QString ext = selectedFilter.last(5).first(4);
    if (!fn.endsWith(ext))
        fn.append(ext);

    switch (mode())
    {
    case PostprocessorWidgetMode_Mesh:
        m_sceneViewMesh->saveImageToFile(fn);
        break;
    case PostprocessorWidgetMode_Post2D:
        m_sceneViewPost2D->saveImageToFile(fn);
        break;
    case PostprocessorWidgetMode_Post3D:
        m_sceneViewPost3D->saveImageToFile(fn);
        break;
    case PostprocessorWidgetMode_Chart:
        m_sceneViewChart->grab().save(fn);
        break;
    default:
        break;
    }

    QFileInfo fileInfo(fn);
    if (fileInfo.absoluteDir() != tempProblemDir())
    {
        settings.setValue("General/LastImageDir", fileInfo.absolutePath());
    }
}

void PostprocessorWidget::doPostModeSet(QAction *action)
{
    if (actOperateOnMesh->isChecked())
        m_postMode = PostprocessorWidgetMode_Mesh;
    else if (actOperateOnPost2D->isChecked())
        m_postMode = PostprocessorWidgetMode_Post2D;
    else if (actOperateOnPost3D->isChecked())
        m_postMode = PostprocessorWidgetMode_Post3D;
    else if (actOperateOnChart->isChecked())
        m_postMode = PostprocessorWidgetMode_Chart;
    else
    {
        // set default
        actOperateOnPost2D->setChecked(true);
        m_postMode = PostprocessorWidgetMode_Post2D;
    }

    update();

    emit modeChanged(m_postMode);
}

PostprocessorSceneWidget::PostprocessorSceneWidget(PhysicalFieldWidget *fieldWidget)
    : QWidget(fieldWidget), m_fieldWidget(fieldWidget)
{
    connect(fieldWidget, SIGNAL(fieldChanged()), this, SLOT(refresh()));
}

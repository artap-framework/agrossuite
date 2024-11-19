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

#include "videodialog.h"

#include "util/global.h"
#include "util/conf.h"

#include "solver/problem.h"
#include "solver/solutionstore.h"
#include "solver/field.h"

#include "sceneview_common.h"

VideoDialog::VideoDialog(SceneViewCommon *sceneView, QSharedPointer<Computation> computation, QWidget *parent)
    : QDialog(parent), m_sceneView(sceneView), m_computation(computation)
{
    setModal(true);
    setWindowTitle(tr("Video"));

    // create directory
    QDir(tempProblemDir()).mkdir("video");

    // store timestep
    m_timeStepStore = m_computation->postDeal()->activeTimeStep();
    // store adaptive step
    m_adaptiveStepStore = m_computation->postDeal()->activeAdaptivityStep();

    m_showRulersStore = Agros::configComputer()->value(Config::Config_ShowRulers).toBool();

    // timer create images
    timer = new QTimer(this);

    createControls();
}

VideoDialog::~VideoDialog()
{
    QSettings settings;
    settings.setValue("VideoDialog/ShowRulers", chkFigureShowRulers->isChecked());
    settings.setValue("VideoDialog/ShowAxes", chkFigureShowAxes->isChecked());
    settings.setValue("VideoDialog/SaveImages", chkSaveImages->isChecked());

    // restore previous timestep
    m_computation->postDeal()->setActiveTimeStep(m_timeStepStore);
    m_computation->postDeal()->setActiveAdaptivityStep(m_adaptiveStepStore);

    Agros::configComputer()->setValue(Config::Config_ShowRulers, m_showRulersStore);

    m_computation->postDeal()->refresh();

    delete timer;
}

void VideoDialog::showDialog()
{
    // time steps
    m_timeLevels = m_computation->timeStepLengths();
    m_timeSteps = m_timeLevels.count() - 3;
    lblTransientStep->setText(QString("%1 / %2").arg(0).arg(m_timeSteps));
    lblTransientTime->setText(tr("%1 / %2 s").arg(0.0).arg(m_timeLevels.last()));
    sliderTransientAnimate->blockSignals(true);
    sliderTransientAnimate->setMinimum(0);
    sliderTransientAnimate->setMaximum(m_timeSteps);
    sliderTransientAnimate->blockSignals(false);

    // adaptive steps
    m_adaptiveSteps = m_computation->solutionStore()->lastAdaptiveStep(m_computation->postDeal()->activeViewField()) + 1;
    lblAdaptiveStep->setText(QString("%1 / %2").arg(1).arg(m_adaptiveSteps));
    sliderAdaptiveAnimate->blockSignals(true);
    sliderAdaptiveAnimate->setMinimum(1);
    sliderAdaptiveAnimate->setMaximum(m_adaptiveSteps);
    sliderAdaptiveAnimate->blockSignals(false);

    if (m_computation->postDeal()->activeViewField()->analysisType() == AnalysisType_Transient)
    {
        tabType->setCurrentWidget(tabTransient);
    }
    else if (m_computation->postDeal()->activeViewField()->adaptivityType() != AdaptivityMethod_None)
    {
        tabType->setCurrentWidget(tabAdaptivity);
    }
    else
    {
        QMessageBox::information(this, tr("Video Dialog"), tr("The active field is not time dependent or adaptive."));

        close();
        return;
    }

    if (tabType->currentWidget() == tabTransient)
    {
        connect(btnGenerate, SIGNAL(clicked()), this, SLOT(transientAnimate()));
        connect(timer, SIGNAL(timeout()), this, SLOT(transientAnimateNextStep()));
    }
    else if (tabType->currentWidget() == tabAdaptivity)
    {
        connect(btnGenerate, SIGNAL(clicked()), this, SLOT(adaptiveAnimate()));
        connect(timer, SIGNAL(timeout()), this, SLOT(adaptiveAnimateNextStep()));
    }

    exec();
}

void VideoDialog::createControls()
{
    // tab
    tabTransient = createControlsViewportTimeSteps();
    tabAdaptivity = createControlsViewportAdaptiveSteps();

    tabType = new QStackedWidget();
    tabType->addWidget(tabAdaptivity);
    tabType->addWidget(tabTransient);

    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    QPushButton *btnVideo = new QPushButton(tr("Show video"));
    btnVideo->setDefault(true);
    connect(btnVideo, SIGNAL(clicked()), this, SLOT(doVideo()));

    QPushButton *btnOpenFolder = new QPushButton(tr("Open folder"));
    connect(btnOpenFolder, SIGNAL(clicked()), this, SLOT(doOpenFolder()));

    btnGenerate = new QPushButton(tr("Generate"));

    QHBoxLayout *layoutButton = new QHBoxLayout();
    layoutButton->addStretch();
    layoutButton->addWidget(btnGenerate);
    layoutButton->addWidget(btnVideo);
    layoutButton->addWidget(btnOpenFolder);
    layoutButton->addWidget(btnClose);

    QSettings settings;

    chkSaveImages = new QCheckBox(tr("Save images to disk"));
    chkSaveImages->setChecked(settings.value("VideoDialog/SaveImages", true).toBool());

    chkFigureShowRulers = new QCheckBox(tr("Show rulers"));
    chkFigureShowRulers->setChecked(settings.value("VideoDialog/ShowRulers", Agros::configComputer()->value(Config::Config_ShowRulers).toBool()).toBool());

    QHBoxLayout *layoutButtonViewport = new QHBoxLayout();
    layoutButtonViewport->addStretch();
    layoutButtonViewport->addWidget(btnGenerate);

    QGridLayout *layout = new QGridLayout();
    layout->setColumnStretch(0, 1);
    layout->addWidget(tabType, 0, 0, 1, 2);
    layout->addWidget(chkSaveImages, 1, 0);
    layout->addWidget(chkFigureShowRulers, 2, 1);
    layout->addWidget(chkFigureShowAxes, 3, 1);
    layout->addLayout(layoutButton, 10, 0, 1, 2);

    setLayout(layout);

    setMinimumWidth(300);
}

QWidget *VideoDialog::createControlsViewportTimeSteps()
{
    sliderTransientAnimate = new QSlider(Qt::Horizontal);
    sliderTransientAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sliderTransientAnimate, SIGNAL(valueChanged(int)), this, SLOT(setTransientStep(int)));

    lblTransientStep = new QLabel();
    lblTransientTime = new QLabel();

    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("Time step:")), 0, 0);
    layoutControlsViewport->addWidget(lblTransientStep, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(tr("Time:")), 1, 0);
    layoutControlsViewport->addWidget(lblTransientTime, 1, 1);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sliderTransientAnimate);

    QWidget *widViewport = new QWidget();
    widViewport->setLayout(layoutViewport);

    return widViewport;
}

QWidget *VideoDialog::createControlsViewportAdaptiveSteps()
{
    sliderAdaptiveAnimate = new QSlider(Qt::Horizontal);
    sliderAdaptiveAnimate->setTickPosition(QSlider::TicksBelow);
    connect(sliderAdaptiveAnimate, SIGNAL(valueChanged(int)), this, SLOT(setAdaptiveStep(int)));

    lblAdaptiveStep = new QLabel();

    QGridLayout *layoutControlsViewport = new QGridLayout();
    layoutControlsViewport->addWidget(new QLabel(tr("Adaptive step:")), 0, 0);
    layoutControlsViewport->addWidget(lblAdaptiveStep, 0, 1);
    layoutControlsViewport->addWidget(new QLabel(" "), 1, 0);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addLayout(layoutControlsViewport);
    layoutViewport->addWidget(sliderAdaptiveAnimate);

    QWidget *widViewport = new QWidget();
    widViewport->setLayout(layoutViewport);

    return widViewport;
}

void VideoDialog::transientAnimate()
{
    if (timer->isActive())
    {
        btnClose->setEnabled(true);

        timer->stop();
        btnGenerate->setText(tr("Run"));
    }
    else
    {
        setTransientStep(0);

        btnClose->setEnabled(false);

        btnGenerate->setText(tr("Stop"));
        timer->start(0.0);
    }
}

void VideoDialog::transientAnimateNextStep()
{
    if (m_computation->postDeal()->activeTimeStep() < m_timeSteps)
    {
        setTransientStep(m_computation->postDeal()->activeTimeStep() + 1);
    }
    else
    {
        transientAnimate();
    }
}

void VideoDialog::setTransientStep(int transientStep)
{
    Agros::configComputer()->setValue(Config::Config_ShowRulers, chkFigureShowRulers->isChecked());

    m_computation->postDeal()->setActiveTimeStep(transientStep);
    m_computation->postDeal()->setActiveAdaptivityStep(m_computation->solutionStore()->lastAdaptiveStep(m_computation->postDeal()->activeViewField(), transientStep));
    m_computation->postDeal()->refresh();
    m_sceneView->refresh();

    if (chkSaveImages->isChecked())
        m_sceneView->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000000" + QString::number(transientStep)).right(8)));

    sliderTransientAnimate->setValue(transientStep);

    QString time = QString::number(m_timeLevels[transientStep], 'g');
    lblTransientStep->setText(QString("%1 / %2").arg(transientStep).arg(m_timeSteps));
    lblTransientTime->setText(tr("%1 / %2 s").arg(time).arg(m_timeLevels.last()));

    QApplication::processEvents();
}

void VideoDialog::adaptiveAnimate()
{
    if (timer->isActive())
    {
        btnClose->setEnabled(true);

        timer->stop();
        btnGenerate->setText(tr("Generate"));
    }
    else
    {
        setAdaptiveStep(1);

        btnClose->setEnabled(false);

        btnGenerate->setText(tr("Stop"));
        timer->start(0.0);
    }
}

void VideoDialog::adaptiveAnimateNextStep()
{
    if (m_computation->postDeal()->activeAdaptivityStep() < m_adaptiveSteps - 1)
        setAdaptiveStep(m_computation->postDeal()->activeAdaptivityStep() + 2);
    else
        adaptiveAnimate();
}

void VideoDialog::setAdaptiveStep(int adaptiveStep)
{
    Agros::configComputer()->setValue(Config::Config_ShowRulers, chkFigureShowRulers->isChecked());

    m_computation->postDeal()->setActiveAdaptivityStep(adaptiveStep - 1);
    m_computation->postDeal()->refresh();
    m_sceneView->refresh();

    sliderAdaptiveAnimate->setValue(adaptiveStep);
    lblAdaptiveStep->setText(QString("%1 / %2").arg(adaptiveStep).arg(m_adaptiveSteps));

    if (chkSaveImages->isChecked())
        m_sceneView->saveImageToFile(tempProblemDir() + QString("/video/video_%1.png").arg(QString("0000000" + QString::number(adaptiveStep)).right(8)));

    QApplication::processEvents();
}

void VideoDialog::doVideo()
{
    ImageSequenceDialog video;
    video.exec();
}

void VideoDialog::doOpenFolder()
{
    QDesktopServices::openUrl(QUrl(QString("file:///%1/video").arg(tempProblemDir()), QUrl::TolerantMode));
}

void VideoDialog::doClose()
{
    hide();
}

// *********************************************************************************

ImageSequenceDialog::ImageSequenceDialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("Video"));

    // read images
    QStringList filters;
    filters << "*.png";

    QDir dir(tempProblemDir() + QString("/video"));
    dir.setNameFilters(filters);
    m_images = dir.entryList();

    sliderAnimateSequence = new QSlider(Qt::Horizontal);
    sliderAnimateSequence->setTickPosition(QSlider::TicksBelow);
    sliderAnimateSequence->setMaximum(m_images.count() - 1);
    connect(sliderAnimateSequence, SIGNAL(valueChanged(int)), this, SLOT(animateSequence(int)));

    int speed = 300;
    cmbSpeed = new QComboBox();
    cmbSpeed->addItem(" 10 %", speed / 0.10);
    cmbSpeed->addItem(" 25 %", speed / 0.25);
    cmbSpeed->addItem(" 50 %", speed / 0.50);
    cmbSpeed->addItem(" 75 %", speed / 0.75);
    cmbSpeed->addItem("100 %", speed);
    cmbSpeed->addItem("150 %", speed / 1.50);
    cmbSpeed->addItem("200 %", speed / 2.00);
    cmbSpeed->addItem("300 %", speed / 3.00);
    cmbSpeed->addItem("400 %", speed / 4.00);
    cmbSpeed->addItem("500 %", speed / 5.00);

    QSettings settings;
    cmbSpeed->setCurrentIndex(cmbSpeed->findData(settings.value("ImageSequenceDialog/Speed").toDouble()));
    if (cmbSpeed->currentIndex() == -1)
        cmbSpeed->setCurrentIndex(cmbSpeed->findData(speed));

    btnAnimate = new QPushButton(tr("Run"));

    btnClose = new QPushButton(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doClose()));

    lblStep = new QLabel();

    QHBoxLayout *layoutControls = new QHBoxLayout();
    layoutControls->addWidget(sliderAnimateSequence);
    layoutControls->addWidget(lblStep);
    layoutControls->addWidget(new QLabel(tr("Speed:")));
    layoutControls->addWidget(cmbSpeed);
    layoutControls->addWidget(btnAnimate);
    layoutControls->addWidget(btnClose);

    lblImage = new QLabel();
    lblImage->setAlignment(Qt::AlignCenter);

    QVBoxLayout *layoutViewport = new QVBoxLayout();
    layoutViewport->addWidget(lblImage, Qt::AlignHCenter);
    layoutViewport->addLayout(layoutControls);

    setLayout(layoutViewport);

    // timer create images
    timer = new QTimer(this);
    connect(btnAnimate, SIGNAL(clicked()), this, SLOT(animate()));
    connect(timer, SIGNAL(timeout()), this, SLOT(animateNextStep()));

    if (m_images.count() > 0)
        animateSequence(0);
}

ImageSequenceDialog::~ImageSequenceDialog()
{
    QSettings settings;
    settings.setValue("ImageSequenceDialog/Speed", cmbSpeed->itemData(cmbSpeed->currentIndex()));
}

bool ImageSequenceDialog::showDialog()
{
    return exec();
}

void ImageSequenceDialog::resizeEvent(QResizeEvent *event)
{
    updateImage();
}

void ImageSequenceDialog::animate()
{
    if (timer->isActive())
    {
        btnClose->setEnabled(true);

        timer->stop();
        btnAnimate->setText(tr("Run"));
    }
    else
    {
        sliderAnimateSequence->setValue(0);

        btnClose->setEnabled(false);

        btnAnimate->setText(tr("Stop"));
        timer->start(cmbSpeed->itemData(cmbSpeed->currentIndex()).toDouble());
    }
}

void ImageSequenceDialog::animateNextStep()
{
    if (sliderAnimateSequence->value() < m_images.count() - 1)
    {
        sliderAnimateSequence->setValue(sliderAnimateSequence->value() + 1);
        animateSequence(sliderAnimateSequence->value());
    }
    else
    {
        animate();
    }
}

void ImageSequenceDialog::animateSequence(int index)
{
    if (m_images.count() == 0)
        return;

    QString fileName = tempProblemDir() + QString("/video/") + m_images.at(index);
    if (QFile::exists(fileName))
    {
        m_currentImage.load(fileName);
        updateImage();

        lblStep->setText(QString("%1 / %2").arg(index + 1).arg(m_images.count()));
    }
}

void ImageSequenceDialog::updateImage()
{
    lblImage->setPixmap(m_currentImage.scaled(lblImage->width(),
                                              lblImage->height(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));
}

void ImageSequenceDialog::doClose()
{
    hide();
}

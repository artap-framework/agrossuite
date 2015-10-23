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

#include "postprocessorview_particletracing.h"
#include "postprocessorview.h"

#include "util/global.h"

#include "gui/lineeditdouble.h"
#include "gui/groupbox.h"
#include "gui/common.h"
#include "gui/physicalfield.h"

#include "scene.h"
#include "scenemarker.h"
#include "sceneview_geometry.h"
#include "sceneview_mesh.h"
#include "sceneview_post2d.h"
#include "sceneview_post3d.h"
#include "pythonlab/pythonengine_agros.h"

#include "particle/particle_tracing.h"

#include "solver/module.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorSceneParticleTracingWidget::PostprocessorSceneParticleTracingWidget(PostprocessorWidget *postprocessorWidget, SceneViewParticleTracing *sceneParticleTracing)
    : PostprocessorSceneWidget(postprocessorWidget), m_sceneViewParticleTracing(sceneParticleTracing)
{
    setMinimumWidth(160);
    setObjectName("ParticleTracingView");

    createControls();

    connect(postprocessorWidget->fieldWidget(), SIGNAL(fieldChanged()), this, SLOT(refresh()));
}

void PostprocessorSceneParticleTracingWidget::load()
{
    // particle tracing
    cmbParticleButcherTableType->setCurrentIndex(cmbParticleButcherTableType->findData(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleButcherTableType).toInt()));
    chkParticleIncludeRelativisticCorrection->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleIncludeRelativisticCorrection).toBool());
    txtParticleNumberOfParticles->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleNumberOfParticles).toInt());
    txtParticleStartingRadius->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleStartingRadius).toDouble());
    txtParticleMass->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleMass).toDouble());
    txtParticleConstant->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleConstant).toDouble());
    txtParticlePointX->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleStartX).toDouble());
    txtParticlePointY->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleStartY).toDouble());
    txtParticleVelocityX->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleStartVelocityX).toDouble());
    txtParticleVelocityY->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleStartVelocityY).toDouble());
    chkParticleReflectOnDifferentMaterial->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleReflectOnDifferentMaterial).toBool());
    chkParticleReflectOnBoundary->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleReflectOnBoundary).toBool());
    txtParticleCoefficientOfRestitution->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleCoefficientOfRestitution).toDouble());
    txtParticleCustomForceX->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble());
    txtParticleCustomForceY->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble());
    txtParticleCustomForceZ->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble());
    txtParticleMaximumRelativeError->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleMaximumRelativeError).toDouble());
    txtParticleMaximumSteps->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleMaximumStep).toDouble());
    txtParticleMaximumNumberOfSteps->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleMaximumNumberOfSteps).toInt());
    chkParticleColorByVelocity->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleColorByVelocity).toBool());
    chkParticleShowPoints->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleShowPoints).toBool());
    chkParticleShowBlendedFaces->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleShowBlendedFaces).toBool());
    txtParticleNumShowParticleAxi->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleNumShowParticlesAxi).toInt());
    txtParticleNumShowParticleAxi->setEnabled(m_postprocessorWidget->computation()->config()->coordinateType() == CoordinateType_Axisymmetric);
    txtParticleDragDensity->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble());
    txtParticleDragReferenceArea->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble());
    txtParticleDragCoefficient->setValue(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble());
    chkParticleP2PElectricForce->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool());
    chkParticleP2PMagneticForce->setChecked(m_postprocessorWidget->computation()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool());

    lblParticlePointX->setText(QString("%1 (m):").arg(m_postprocessorWidget->computation()->config()->labelX()));
    lblParticlePointY->setText(QString("%1 (m):").arg(m_postprocessorWidget->computation()->config()->labelY()));
    lblParticleVelocityX->setText(QString("%1 (m/s):").arg(m_postprocessorWidget->computation()->config()->labelX()));
    lblParticleVelocityY->setText(QString("%1 (m/s):").arg(m_postprocessorWidget->computation()->config()->labelY()));
    lblParticleCustomForceX->setText(QString("%1 (N):").arg(m_postprocessorWidget->computation()->config()->labelX()));
    lblParticleCustomForceY->setText(QString("%1 (N):").arg(m_postprocessorWidget->computation()->config()->labelY()));
    lblParticleCustomForceZ->setText(QString("%1 (N):").arg(m_postprocessorWidget->computation()->config()->labelZ()));

    if (m_postprocessorWidget->computation()->config()->coordinateType() == CoordinateType_Planar)
        lblParticleMotionEquations->setText(QString("<i>x</i>\" = <i>F</i><sub>x</sub> / <i>m</i>, &nbsp; <i>y</i>\" = <i>F</i><sub>y</sub> / <i>m</i>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>"));
    else
        lblParticleMotionEquations->setText(QString("<i>r</i>\" = <i>F</i><sub>r</sub> / <i>m</i> + <i>r</i> (<i>&phi;</i>')<sup>2</sup>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i><br /><i>&phi;</i>\" = <i>F</i><sub>&phi;</sub> / <i>m</i> - 2<i>r</i> <i>r</i>' <i>&phi;</i>' / <i>r</i>"));
}

void PostprocessorSceneParticleTracingWidget::save()
{
    // particle tracing
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleButcherTableType, (ButcherTableType) cmbParticleButcherTableType->itemData(cmbParticleButcherTableType->currentIndex()).toInt());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleIncludeRelativisticCorrection, chkParticleIncludeRelativisticCorrection->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleNumberOfParticles, txtParticleNumberOfParticles->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleStartingRadius, txtParticleStartingRadius->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleMass, txtParticleMass->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleConstant, txtParticleConstant->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleStartX, txtParticlePointX->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleStartY, txtParticlePointY->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityX, txtParticleVelocityX->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleStartVelocityY, txtParticleVelocityY->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleReflectOnDifferentMaterial, chkParticleReflectOnDifferentMaterial->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleReflectOnBoundary, chkParticleReflectOnBoundary->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleCoefficientOfRestitution, txtParticleCoefficientOfRestitution->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleCustomForceX, txtParticleCustomForceX->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleCustomForceY, txtParticleCustomForceY->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleCustomForceZ, txtParticleCustomForceZ->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleMaximumRelativeError, txtParticleMaximumRelativeError->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleMaximumStep, txtParticleMaximumSteps->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleMaximumNumberOfSteps, txtParticleMaximumNumberOfSteps->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleColorByVelocity, chkParticleColorByVelocity->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleShowPoints, chkParticleShowPoints->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleShowBlendedFaces, chkParticleShowBlendedFaces->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleNumShowParticlesAxi, txtParticleNumShowParticleAxi->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleDragDensity, txtParticleDragDensity->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleDragCoefficient, txtParticleDragCoefficient->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleDragReferenceArea, txtParticleDragReferenceArea->value());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleP2PElectricForce, chkParticleP2PElectricForce->isChecked());
    m_postprocessorWidget->computation()->setting()->setValue(ProblemSetting::View_ParticleP2PMagneticForce, chkParticleP2PMagneticForce->isChecked());

    m_sceneViewParticleTracing->processParticleTracing();
}

void PostprocessorSceneParticleTracingWidget::createControls()
{
    // particle tracing
    cmbParticleButcherTableType = new QComboBox(this);
    foreach (QString key, butcherTableTypeStringKeys())
        cmbParticleButcherTableType->addItem(butcherTableTypeString(butcherTableTypeFromStringKey(key)), butcherTableTypeFromStringKey(key));

    chkParticleIncludeRelativisticCorrection = new QCheckBox(tr("Relativistic correction"));
    txtParticleNumberOfParticles = new QSpinBox(this);
    txtParticleNumberOfParticles->setMinimum(1);
    txtParticleNumberOfParticles->setMaximum(200);
    txtParticleStartingRadius = new LineEditDouble(0);
    txtParticleStartingRadius->setBottom(0.0);
    txtParticleMass = new LineEditDouble(0);
    txtParticleMass->setBottom(0.0);
    txtParticleConstant = new LineEditDouble(0);
    lblParticlePointX = new QLabel();
    lblParticlePointY = new QLabel();
    txtParticlePointX = new LineEditDouble(0);
    txtParticlePointY = new LineEditDouble(0);
    lblParticleVelocityX = new QLabel();
    lblParticleVelocityY = new QLabel();
    txtParticleVelocityX = new LineEditDouble(0);
    txtParticleVelocityY = new LineEditDouble(0);
    lblParticleCustomForceX = new QLabel();
    lblParticleCustomForceY = new QLabel();
    lblParticleCustomForceZ = new QLabel();
    txtParticleCustomForceX = new LineEditDouble(0);
    txtParticleCustomForceY = new LineEditDouble(0);
    txtParticleCustomForceZ = new LineEditDouble(0);
    txtParticleMaximumRelativeError = new LineEditDouble(0);
    txtParticleMaximumRelativeError->setBottom(0.0);
    txtParticleMaximumSteps = new LineEditDouble(0);
    txtParticleMaximumSteps->setBottom(0.0);
    chkParticleReflectOnDifferentMaterial = new QCheckBox(tr("Reflection on different material"));
    chkParticleReflectOnBoundary = new QCheckBox(tr("Reflection on boundary"));
    txtParticleCoefficientOfRestitution = new LineEditDouble(0);
    txtParticleCoefficientOfRestitution->setBottom(0.0);
    txtParticleCoefficientOfRestitution->setTop(1.0);
    chkParticleColorByVelocity = new QCheckBox(tr("Line color is controlled by velocity"));
    chkParticleShowPoints = new QCheckBox(tr("Show points"));
    chkParticleShowBlendedFaces = new QCheckBox(tr("Show blended faces"));
    txtParticleNumShowParticleAxi = new QSpinBox();
    txtParticleNumShowParticleAxi->setMinimum(1);
    txtParticleNumShowParticleAxi->setMaximum(500);
    txtParticleMaximumNumberOfSteps = new QSpinBox();
    txtParticleMaximumNumberOfSteps->setMinimum(10);
    txtParticleMaximumNumberOfSteps->setMaximum(100000);
    txtParticleMaximumNumberOfSteps->setSingleStep(10);
    txtParticleDragDensity = new LineEditDouble(0);
    txtParticleDragDensity->setBottom(0.0);
    txtParticleDragCoefficient = new LineEditDouble(0);
    txtParticleDragCoefficient->setBottom(0.0);
    txtParticleDragReferenceArea = new LineEditDouble(0);
    txtParticleDragReferenceArea->setBottom(0.0);
    lblParticleMotionEquations = new QLabel();
    chkParticleP2PElectricForce = new QCheckBox(tr("Electrostatic interaction"));
    chkParticleP2PMagneticForce = new QCheckBox(tr("Magnetic interaction"));

    // initial particle position
    QGridLayout *gridLayoutGeneral = new QGridLayout();
    gridLayoutGeneral->addWidget(new QLabel(tr("Equations:")), 0, 0);
    gridLayoutGeneral->addWidget(lblParticleMotionEquations, 1, 0, 1, 2);
    gridLayoutGeneral->addWidget(new QLabel(tr("Number of particles:")), 2, 0);
    gridLayoutGeneral->addWidget(txtParticleNumberOfParticles, 2, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Particles dispersion (m):")), 3, 0);
    gridLayoutGeneral->addWidget(txtParticleStartingRadius, 3, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Mass (kg):")), 4, 0);
    gridLayoutGeneral->addWidget(txtParticleMass, 4, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(gridLayoutGeneral);

    // QPushButton *btnParticleDefault = new QPushButton(tr("Default"));
    // connect(btnParticleDefault, SIGNAL(clicked()), this, SLOT(doParticleDefault()));

    // initial particle position
    QGridLayout *gridLayoutInitialPosition = new QGridLayout();
    gridLayoutInitialPosition->addWidget(lblParticlePointX, 0, 0);
    gridLayoutInitialPosition->addWidget(txtParticlePointX, 0, 1);
    gridLayoutInitialPosition->addWidget(lblParticlePointY, 1, 0);
    gridLayoutInitialPosition->addWidget(txtParticlePointY, 1, 1);

    QGroupBox *grpInitialPosition = new QGroupBox(tr("Initial particle position"));
    grpInitialPosition->setLayout(gridLayoutInitialPosition);

    // initial particle velocity
    QGridLayout *gridLayoutInitialVelocity = new QGridLayout();
    gridLayoutInitialVelocity->addWidget(lblParticleVelocityX, 0, 0);
    gridLayoutInitialVelocity->addWidget(txtParticleVelocityX, 0, 1);
    gridLayoutInitialVelocity->addWidget(lblParticleVelocityY, 1, 0);
    gridLayoutInitialVelocity->addWidget(txtParticleVelocityY, 1, 1);

    QGroupBox *grpInitialVelocity = new QGroupBox(tr("Initial particle velocity"));
    grpInitialVelocity->setLayout(gridLayoutInitialVelocity);

    // reflection
    QGridLayout *gridLayoutReflection = new QGridLayout();
    gridLayoutReflection->setContentsMargins(5, 5, 0, 0);
    gridLayoutReflection->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutReflection->setColumnStretch(1, 1);
    gridLayoutReflection->addWidget(chkParticleReflectOnDifferentMaterial, 0, 0, 1, 2);
    gridLayoutReflection->addWidget(chkParticleReflectOnBoundary, 1, 0, 1, 2);
    gridLayoutReflection->addWidget(new QLabel(tr("Coefficient of restitution (-):")), 2, 0);
    gridLayoutReflection->addWidget(txtParticleCoefficientOfRestitution, 2, 1);
    gridLayoutReflection->addWidget(new QLabel(""), 10, 0);
    gridLayoutReflection->setRowStretch(10, 1);

    QGroupBox *grpReflection = new QGroupBox(tr("Reflection"));
    grpReflection->setLayout(gridLayoutReflection);

    // Lorentz force
    QGridLayout *gridLayoutLorentzForce = new QGridLayout();
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Equation:")), 0, 0);
    gridLayoutLorentzForce->addWidget(new QLabel(QString("<i><b>F</b></i><sub>L</sub> = <i>Q</i> (<i><b>E</b></i> + <i><b>v</b></i> x <i><b>B</b></i>)")), 0, 1);
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Charge (C):")), 1, 0);
    gridLayoutLorentzForce->addWidget(txtParticleConstant, 1, 1);

    QGroupBox *grpLorentzForce = new QGroupBox(tr("Lorentz force"));
    grpLorentzForce->setLayout(gridLayoutLorentzForce);

    // drag force
    QGridLayout *gridLayoutDragForce = new QGridLayout();
    gridLayoutDragForce->setColumnMinimumWidth(0, columnMinimumWidth());
    gridLayoutDragForce->setColumnStretch(1, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Equation:")), 0, 0);
    gridLayoutDragForce->addWidget(new QLabel(QString("<i><b>F</b></i><sub>D</sub> = - &frac12; <i>&rho;</i> <i>v</i><sup>2</sup> <i>C</i><sub>D</sub> <i>S</i> &sdot; <i><b>v</b></i><sub>0</sub>")), 0, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Density (kg/m<sup>3</sup>):")), 1, 0);
    gridLayoutDragForce->addWidget(txtParticleDragDensity, 1, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Reference area (m<sup>2</sup>):")), 2, 0);
    gridLayoutDragForce->addWidget(txtParticleDragReferenceArea, 2, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Coefficient (-):")), 3, 0);
    gridLayoutDragForce->addWidget(txtParticleDragCoefficient, 3, 1);

    QGroupBox *grpDragForce = new QGroupBox(tr("Drag force"));
    grpDragForce->setLayout(gridLayoutDragForce);

    // custom force
    QGridLayout *gridCustomForce = new QGridLayout();
    gridCustomForce->addWidget(lblParticleCustomForceX, 0, 0);
    gridCustomForce->addWidget(txtParticleCustomForceX, 0, 1);
    gridCustomForce->addWidget(lblParticleCustomForceY, 1, 0);
    gridCustomForce->addWidget(txtParticleCustomForceY, 1, 1);
    gridCustomForce->addWidget(lblParticleCustomForceZ, 2, 0);
    gridCustomForce->addWidget(txtParticleCustomForceZ, 2, 1);

    QGroupBox *grpCustomForce = new QGroupBox(tr("Custom force"));
    grpCustomForce->setLayout(gridCustomForce);

    // particle to particle
    QGridLayout *gridP2PForce = new QGridLayout();
    gridP2PForce->addWidget(chkParticleP2PElectricForce, 0, 0);
    gridP2PForce->addWidget(chkParticleP2PMagneticForce, 1, 0);

    QGroupBox *grpP2PForce = new QGroupBox(tr("Particle to particle"));
    grpP2PForce->setLayout(gridP2PForce);

    // forces
    QVBoxLayout *layoutForces = new QVBoxLayout();
    layoutForces->setContentsMargins(5, 5, 0, 0);
    layoutForces->addWidget(grpLorentzForce);
    layoutForces->addWidget(grpDragForce);
    layoutForces->addWidget(grpCustomForce);
    layoutForces->addWidget(grpP2PForce);
    layoutForces->addStretch(1);

    QWidget *widgetForces = new QWidget(this);
    widgetForces->setLayout(layoutForces);

    // solver
    QGridLayout *gridLayoutSolver = new QGridLayout();
    gridLayoutSolver->addWidget(new QLabel(tr("Butcher tableau:")), 0, 0);
    gridLayoutSolver->addWidget(cmbParticleButcherTableType, 0, 1);
    gridLayoutSolver->addWidget(chkParticleIncludeRelativisticCorrection, 1, 0);
    gridLayoutSolver->addWidget(new QLabel(QString("<i>m</i><sub>p</sub> = m / (1 - v<sup>2</sup>/c<sup>2</sup>)<sup>1/2</sup>")), 1, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Max. relative error (%):")), 2, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumRelativeError, 2, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Max. step (m):")), 3, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumSteps, 3, 1);
    gridLayoutSolver->addWidget(new QLabel(tr("Max. number of steps:")), 4, 0);
    gridLayoutSolver->addWidget(txtParticleMaximumNumberOfSteps, 4, 1);
    gridLayoutSolver->addWidget(new QLabel(""), 10, 0);
    gridLayoutSolver->setRowStretch(10, 1);

    QGroupBox *grpSolver = new QGroupBox(tr("Solver"));
    grpSolver->setLayout(gridLayoutSolver);

    // settings
    QGridLayout *gridLayoutSettings = new QGridLayout();
    gridLayoutSettings->setColumnStretch(1, 1);
    gridLayoutSettings->setContentsMargins(5, 5, 0, 0);
    gridLayoutSettings->addWidget(chkParticleColorByVelocity, 2, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowPoints, 3, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowBlendedFaces, 4, 0, 1, 2);
    gridLayoutSettings->addWidget(new QLabel(tr("Show particle multiple times:")), 5, 0);
    gridLayoutSettings->addWidget(txtParticleNumShowParticleAxi, 5, 1);
    //gridLayoutSettings->addWidget(new QLabel(""), 10, 0);
    gridLayoutSettings->setRowStretch(10, 1);

    QGroupBox *grpSettings = new QGroupBox(tr("Settings"));
    grpSettings->setLayout(gridLayoutSettings);

    // tab widget
    QToolBox *tbxWorkspace = new QToolBox();
    tbxWorkspace->addItem(widgetForces, icon(""), tr("Forces"));
    tbxWorkspace->addItem(grpReflection, icon(""), tr("Reflection"));
    tbxWorkspace->addItem(grpSolver, icon(""), tr("Solver"));
    tbxWorkspace->addItem(grpSettings, icon(""), tr("Settings"));

    QVBoxLayout *layoutParticle = new QVBoxLayout();
    layoutParticle->setContentsMargins(2, 2, 2, 3);
    layoutParticle->addWidget(grpGeneral);
    layoutParticle->addWidget(grpInitialPosition);
    layoutParticle->addWidget(grpInitialVelocity);
    layoutParticle->addWidget(tbxWorkspace);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutParticle);

    QScrollArea *widgetArea = new QScrollArea();
    widgetArea->setContentsMargins(0, 0, 0, 0);
    widgetArea->setFrameShape(QFrame::NoFrame);
    widgetArea->setWidgetResizable(true);
    widgetArea->setWidget(widget);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(2, 2, 2, 3);
    layoutMain->addWidget(widgetArea, 1);

    setLayout(layoutMain);
}

void PostprocessorSceneParticleTracingWidget::refresh()
{
    if (!(m_postprocessorWidget->computation() && m_postprocessorWidget->fieldWidget() && m_postprocessorWidget->fieldWidget()->selectedField()))
        return;
}


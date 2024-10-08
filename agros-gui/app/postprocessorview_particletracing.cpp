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
#include "sceneview_particle.h"

#include "particle/particle_tracing.h"

#include "solver/module.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/solutionstore.h"

#include "util/constants.h"

PostprocessorSceneParticleTracingWidget::PostprocessorSceneParticleTracingWidget(PhysicalFieldWidget *fieldWidget, SceneViewParticleTracing *sceneParticleTracing)
    : PostprocessorSceneWidget(fieldWidget), m_sceneViewParticleTracing(sceneParticleTracing)
{
    setMinimumWidth(160);
    setObjectName("ParticleTracingView");

    createControls();
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
    gridLayoutGeneral->addWidget(lblParticleMotionEquations, 0, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Number of particles:")), 1, 0);
    gridLayoutGeneral->addWidget(txtParticleNumberOfParticles, 1, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Particles dispersion (m):")), 2, 0);
    gridLayoutGeneral->addWidget(txtParticleStartingRadius, 2, 1);
    gridLayoutGeneral->addWidget(new QLabel(tr("Mass (kg):")), 3, 0);
    gridLayoutGeneral->addWidget(txtParticleMass, 3, 1);

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

    QVBoxLayout *layoutReflection = new QVBoxLayout();
    layoutReflection->addWidget(grpReflection);
    layoutReflection->addStretch(1);

    QWidget *widgetReflection = new QWidget(this);
    widgetReflection->setLayout(layoutReflection);

    // Lorentz force
    QGridLayout *gridLayoutLorentzForce = new QGridLayout();
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Equation:")), 0, 0);
    gridLayoutLorentzForce->addWidget(new QLabel(QString("<i><b>F</b></i><sub>L</sub> = <i>Q</i> (<i><b>E</b></i> + <i><b>v</b></i> x <i><b>B</b></i>)")), 0, 1);
    gridLayoutLorentzForce->addWidget(new QLabel(tr("Charge (C):")), 1, 0);
    gridLayoutLorentzForce->addWidget(txtParticleConstant, 1, 1);
    gridLayoutLorentzForce->setRowStretch(50, 1);

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
    gridLayoutDragForce->addWidget(new QLabel(tr("Ref. area (m<sup>2</sup>):")), 2, 0);
    gridLayoutDragForce->addWidget(txtParticleDragReferenceArea, 2, 1);
    gridLayoutDragForce->addWidget(new QLabel(tr("Coefficient (-):")), 3, 0);
    gridLayoutDragForce->addWidget(txtParticleDragCoefficient, 3, 1);
    gridLayoutDragForce->setRowStretch(50, 1);

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
    gridCustomForce->setRowStretch(50, 1);

    QGroupBox *grpCustomForce = new QGroupBox(tr("Custom force"));
    grpCustomForce->setLayout(gridCustomForce);

    // forces
    QVBoxLayout *layoutForces = new QVBoxLayout();
    layoutForces->addWidget(grpLorentzForce);
    layoutForces->addWidget(grpDragForce);
    layoutForces->addStretch(1);

    QHBoxLayout *layoutForcesHorizontal = new QHBoxLayout();
    layoutForcesHorizontal->addLayout(layoutForces);
    layoutForcesHorizontal->addWidget(grpCustomForce);

    QWidget *widgetForces = new QWidget(this);
    widgetForces->setLayout(layoutForcesHorizontal);

    // particle to particle
    QGridLayout *gridP2PForce = new QGridLayout();
    gridP2PForce->addWidget(chkParticleP2PElectricForce, 0, 0);
    gridP2PForce->addWidget(chkParticleP2PMagneticForce, 1, 0);

    QGroupBox *grpP2PForce = new QGroupBox(tr("Particle to particle"));
    grpP2PForce->setLayout(gridP2PForce);

    QVBoxLayout *layoutP2P = new QVBoxLayout();
    layoutP2P->addWidget(grpP2PForce);
    layoutP2P->addStretch(1);

    QWidget *widgetP2P = new QWidget(this);
    widgetP2P->setLayout(layoutP2P);

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

    QVBoxLayout *layoutSolver = new QVBoxLayout();
    layoutSolver->addWidget(grpSolver);
    layoutSolver->addStretch(1);

    QWidget *widgetSolver = new QWidget(this);
    widgetSolver->setLayout(layoutSolver);

    // settings
    QGridLayout *gridLayoutSettings = new QGridLayout();
    gridLayoutSettings->setColumnStretch(1, 1);
    gridLayoutSettings->addWidget(chkParticleColorByVelocity, 2, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowPoints, 3, 0, 1, 2);
    gridLayoutSettings->addWidget(chkParticleShowBlendedFaces, 4, 0, 1, 2);
    gridLayoutSettings->addWidget(new QLabel(tr("Show particle multiple times:")), 5, 0);
    gridLayoutSettings->addWidget(txtParticleNumShowParticleAxi, 5, 1);
    //gridLayoutSettings->addWidget(new QLabel(""), 10, 0);
    gridLayoutSettings->setRowStretch(10, 1);

    QGroupBox *grpSettings = new QGroupBox(tr("Settings"));
    grpSettings->setLayout(gridLayoutSettings);

    QVBoxLayout *layoutSettings = new QVBoxLayout();
    layoutSettings->addWidget(grpSettings);
    layoutSettings->addStretch(1);

    QWidget *widgetSettings = new QWidget(this);
    widgetSettings->setLayout(layoutSettings);

    // tab widget
    QTabWidget *tabWorkspace = new QTabWidget();
    tabWorkspace->addTab(widgetForces, tr("Forces"));
    tabWorkspace->addTab(widgetP2P, tr("Particle interaction"));
    tabWorkspace->addTab(widgetReflection, tr("Reflection"));
    tabWorkspace->addTab(widgetSolver, tr("Solver"));
    tabWorkspace->addTab(widgetSettings, tr("Settings"));

    QHBoxLayout *layoutParticleInitial = new QHBoxLayout();
    layoutParticleInitial->addWidget(grpInitialPosition);
    layoutParticleInitial->addWidget(grpInitialVelocity);

    QVBoxLayout *layoutParticle = new QVBoxLayout();
    layoutParticle->addWidget(grpGeneral);
    layoutParticle->addLayout(layoutParticleInitial);
    layoutParticle->addWidget(tabWorkspace);

    setLayout(layoutParticle);
}

void PostprocessorSceneParticleTracingWidget::refresh()
{
    if (!(m_fieldWidget->selectedComputation() && m_fieldWidget->selectedField()))
        return;
}


void PostprocessorSceneParticleTracingWidget::load()
{
    // particle tracing
    cmbParticleButcherTableType->setCurrentIndex(cmbParticleButcherTableType->findData(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleButcherTableType).toInt()));
    chkParticleIncludeRelativisticCorrection->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleIncludeRelativisticCorrection).toBool());
    txtParticleNumberOfParticles->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt());
    txtParticleStartingRadius->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleStartingRadius).toDouble());
    txtParticleMass->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleMass).toDouble());
    txtParticleConstant->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleConstant).toDouble());
    txtParticlePointX->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleStartX).toDouble());
    txtParticlePointY->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleStartY).toDouble());
    txtParticleVelocityX->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleStartVelocityX).toDouble());
    txtParticleVelocityY->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleStartVelocityY).toDouble());
    chkParticleReflectOnDifferentMaterial->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleReflectOnDifferentMaterial).toBool());
    chkParticleReflectOnBoundary->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleReflectOnBoundary).toBool());
    txtParticleCoefficientOfRestitution->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleCoefficientOfRestitution).toDouble());
    txtParticleCustomForceX->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleCustomForceX).toDouble());
    txtParticleCustomForceY->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleCustomForceY).toDouble());
    txtParticleCustomForceZ->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleCustomForceZ).toDouble());
    txtParticleMaximumRelativeError->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleMaximumRelativeError).toDouble());
    txtParticleMaximumSteps->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleMaximumStep).toDouble());
    txtParticleMaximumNumberOfSteps->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleMaximumNumberOfSteps).toInt());
    chkParticleColorByVelocity->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleColorByVelocity).toBool());
    chkParticleShowPoints->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleShowPoints).toBool());
    chkParticleShowBlendedFaces->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleShowBlendedFaces).toBool());
    txtParticleNumShowParticleAxi->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleNumShowParticlesAxi).toInt());
    txtParticleNumShowParticleAxi->setEnabled(m_fieldWidget->selectedComputation()->config()->coordinateType() == CoordinateType_Axisymmetric);
    txtParticleDragDensity->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleDragDensity).toDouble());
    txtParticleDragReferenceArea->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleDragReferenceArea).toDouble());
    txtParticleDragCoefficient->setValue(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleDragCoefficient).toDouble());
    chkParticleP2PElectricForce->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleP2PElectricForce).toBool());
    chkParticleP2PMagneticForce->setChecked(m_fieldWidget->selectedComputation()->setting()->value(PostprocessorSetting::ParticleP2PMagneticForce).toBool());

    lblParticlePointX->setText(QString("%1 (m):").arg(m_fieldWidget->selectedComputation()->config()->labelX()));
    lblParticlePointY->setText(QString("%1 (m):").arg(m_fieldWidget->selectedComputation()->config()->labelY()));
    lblParticleVelocityX->setText(QString("%1 (m/s):").arg(m_fieldWidget->selectedComputation()->config()->labelX()));
    lblParticleVelocityY->setText(QString("%1 (m/s):").arg(m_fieldWidget->selectedComputation()->config()->labelY()));
    lblParticleCustomForceX->setText(QString("%1 (N):").arg(m_fieldWidget->selectedComputation()->config()->labelX()));
    lblParticleCustomForceY->setText(QString("%1 (N):").arg(m_fieldWidget->selectedComputation()->config()->labelY()));
    lblParticleCustomForceZ->setText(QString("%1 (N):").arg(m_fieldWidget->selectedComputation()->config()->labelZ()));

    if (m_fieldWidget->selectedComputation()->config()->coordinateType() == CoordinateType_Planar)
        lblParticleMotionEquations->setText(QString("<i>x</i>\" = <i>F</i><sub>x</sub> / <i>m</i>, &nbsp; <i>y</i>\" = <i>F</i><sub>y</sub> / <i>m</i>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i>"));
    else
        lblParticleMotionEquations->setText(QString("<i>r</i>\" = <i>F</i><sub>r</sub> / <i>m</i> + <i>r</i> (<i>&phi;</i>')<sup>2</sup>, &nbsp; <i>z</i>\" = <i>F</i><sub>z</sub> / <i>m</i><br /><i>&phi;</i>\" = <i>F</i><sub>&phi;</sub> / <i>m</i> - 2<i>r</i> <i>r</i>' <i>&phi;</i>' / <i>r</i>"));
}

void PostprocessorSceneParticleTracingWidget::save()
{
    // particle tracing
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleButcherTableType, (ButcherTableType) cmbParticleButcherTableType->itemData(cmbParticleButcherTableType->currentIndex()).toInt());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleIncludeRelativisticCorrection, chkParticleIncludeRelativisticCorrection->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleNumberOfParticles, txtParticleNumberOfParticles->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleStartingRadius, txtParticleStartingRadius->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleMass, txtParticleMass->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleConstant, txtParticleConstant->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleStartX, txtParticlePointX->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleStartY, txtParticlePointY->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleStartVelocityX, txtParticleVelocityX->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleStartVelocityY, txtParticleVelocityY->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleReflectOnDifferentMaterial, chkParticleReflectOnDifferentMaterial->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleReflectOnBoundary, chkParticleReflectOnBoundary->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleCoefficientOfRestitution, txtParticleCoefficientOfRestitution->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleCustomForceX, txtParticleCustomForceX->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleCustomForceY, txtParticleCustomForceY->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleCustomForceZ, txtParticleCustomForceZ->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleMaximumRelativeError, txtParticleMaximumRelativeError->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleMaximumStep, txtParticleMaximumSteps->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleMaximumNumberOfSteps, txtParticleMaximumNumberOfSteps->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleColorByVelocity, chkParticleColorByVelocity->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleShowPoints, chkParticleShowPoints->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleShowBlendedFaces, chkParticleShowBlendedFaces->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleNumShowParticlesAxi, txtParticleNumShowParticleAxi->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleDragDensity, txtParticleDragDensity->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleDragCoefficient, txtParticleDragCoefficient->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleDragReferenceArea, txtParticleDragReferenceArea->value());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleP2PElectricForce, chkParticleP2PElectricForce->isChecked());
    m_fieldWidget->selectedComputation()->setting()->setValue(PostprocessorSetting::ParticleP2PMagneticForce, chkParticleP2PMagneticForce->isChecked());

    m_sceneViewParticleTracing->processParticleTracing();
}

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

#ifndef POSTPROCESSORVIEW_PARTICLETRACING_H
#define POSTPROCESSORVIEW_PARTICLETRACING_H

#include "util/util.h"
#include "postprocessorview.h"

class SceneViewPost2D;
class LineEditDouble;
class SceneViewParticleTracing;

class PostprocessorSceneParticleTracingWidget : public PostprocessorSceneWidget
{
    Q_OBJECT

public:
    PostprocessorSceneParticleTracingWidget(PhysicalFieldWidget *fieldWidget, SceneViewParticleTracing *sceneParticleTracing);

private:    
    SceneViewPost2D *m_scenePost2D;

    SceneViewParticleTracing *m_sceneViewParticleTracing;

    // particle tracing
    QComboBox *cmbParticleButcherTableType;
    QCheckBox *chkParticleIncludeRelativisticCorrection;
    QSpinBox *txtParticleNumberOfParticles;
    LineEditDouble *txtParticleStartingRadius;
    LineEditDouble *txtParticleMass;
    LineEditDouble *txtParticleConstant;
    LineEditDouble *txtParticlePointX;
    LineEditDouble *txtParticlePointY;
    QLabel *lblParticlePointX;
    QLabel *lblParticlePointY;
    QLabel *lblParticleVelocityX;
    QLabel *lblParticleVelocityY;
    LineEditDouble *txtParticleVelocityX;
    LineEditDouble *txtParticleVelocityY;
    QCheckBox *chkParticleReflectOnDifferentMaterial;
    QCheckBox *chkParticleReflectOnBoundary;
    LineEditDouble *txtParticleCoefficientOfRestitution;
    QLabel *lblParticleMotionEquations;
    QLabel *lblParticleCustomForceX;
    QLabel *lblParticleCustomForceY;
    QLabel *lblParticleCustomForceZ;
    LineEditDouble *txtParticleCustomForceX;
    LineEditDouble *txtParticleCustomForceY;
    LineEditDouble *txtParticleCustomForceZ;
    QCheckBox *chkParticleColorByVelocity;
    QCheckBox *chkParticleShowPoints;
    QCheckBox *chkParticleShowBlendedFaces;
    QSpinBox *txtParticleNumShowParticleAxi;
    LineEditDouble *txtParticleMaximumRelativeError;
    LineEditDouble *txtParticleMaximumSteps;
    QSpinBox *txtParticleMaximumNumberOfSteps;
    LineEditDouble *txtParticleDragDensity;
    LineEditDouble *txtParticleDragCoefficient;
    LineEditDouble *txtParticleDragReferenceArea;
    QCheckBox *chkParticleP2PElectricForce;
    QCheckBox *chkParticleP2PMagneticForce;

    void createControls();

public slots:
    virtual void refresh();
    virtual void load();
    virtual void save();
};

#endif // POSTPROCESSORVIEW_PARTICLETRACING_H

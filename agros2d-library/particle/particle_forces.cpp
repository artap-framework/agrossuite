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

#include "particle_forces.h"

#include "util.h"
#include "util/xml.h"
#include "util/constants.h"

#include "util.h"
#include "value.h"
#include "logview.h"
#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"
#include "solver/problem_config.h"

Point3 ParticleTracingForceCustom::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    return Point3(Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble(),
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble(),
                  Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble());
}

Point3 ParticleTracingForceDrag::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    Point3 velocityReal = (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar) ?
                velocity : Point3(velocity.x, velocity.y, position.x * velocity.z);

    Point3 forceDrag;
    if (velocityReal.magnitude() > 0.0)
        forceDrag = velocityReal.normalizePoint() *
                - 0.5 * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble()
                * velocityReal.magnitude() * velocityReal.magnitude()
                * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble()
                * Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble();

    return forceDrag;
}

ParticleTracingForceField::ParticleTracingForceField(QList<double> particleChargesList)
    : ParticleTracingForce(), m_particleChargesList(particleChargesList)
{
    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if(!fieldInfo->plugin()->hasForce(fieldInfo))
            continue;

        int timeStep = Agros2D::solutionStore()->lastTimeStep(fieldInfo);
        int adaptivityStep = Agros2D::solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

        m_solutionIDs[fieldInfo] = FieldSolutionID(fieldInfo, timeStep, adaptivityStep);
    }
}

Point3 ParticleTracingForceField::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    Point3 totalFieldForce;

    foreach (FieldInfo* fieldInfo, Agros2D::problem()->fieldInfos())
    {
        if (!fieldInfo->plugin()->hasForce(fieldInfo))
            continue;

        Point3 fieldForce;

        // find material
        SceneMaterial *material = nullptr;
        SceneLabel *label = SceneLabel::findLabelAtPoint(Point(position.x, position.y));
        if (label && label->hasMarker(fieldInfo))
        {
            material = label->marker(fieldInfo);
            if (material->isNone())
                return Point3();
        }
        else
        {
            // point not found
            return Point3();
        }

        try
        {
            fieldForce = fieldInfo->plugin()->force(fieldInfo,
                                                    m_solutionIDs[fieldInfo].timeStep,
                                                    m_solutionIDs[fieldInfo].adaptivityStep,
                                                    material, position, velocity)
                    * m_particleChargesList[particleIndex];
        }
        catch (AgrosException e)
        {
            qDebug() << "Particle Tracing warning: " << e.what();
            return Point3();
        }

        totalFieldForce = totalFieldForce + fieldForce;
    }

    return totalFieldForce;
}

ParticleTracingForceFieldP2P::ParticleTracingForceFieldP2P(QList<double> particleChargesList, QList<double> particleMassesList)
    : ParticleTracingForce(), m_particleChargesList(particleChargesList), m_particleMassesList(particleMassesList)
{
}

Point3 ParticleTracingForceFieldP2P::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    // particle to particle force
    Point3 forceP2PElectric;
    Point3 forceP2PMagnetic;
    /*
    if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool() ||
            Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool())
    {
        for (int i = 0; i < m_positionsList.size(); i++)
        {
            if (particleIndex == i)
                continue;

            int timeLevel = timeToLevel(i, m_timesList[particleIndex].last());
            Point3 particlePosition = m_positionsList[i].at(timeLevel);
            Point3 particleVelocity = m_velocitiesList[i].at(timeLevel);

            double distance = 0.0;
            if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                distance = Point3(position.x - particlePosition.x,
                                  position.y - particlePosition.y,
                                  position.z - particlePosition.z).magnitude();
            else
                distance = Point3(position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z),
                                  position.y - particlePosition.y,
                                  position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)).magnitude();

            if (distance > 0)
            {
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool())
                {
                    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                        forceP2PElectric = forceP2PElectric + Point3(
                                    (position.x - particlePosition.x) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.z - particlePosition.z) / distance)
                                * (m_particleChargesList[particleIndex] * m_particleChargesList[i] / (4 * M_PI * EPS0 * distance * distance));
                    else
                        forceP2PElectric = forceP2PElectric + Point3(
                                    (position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z)) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)) / distance)
                                * (m_particleChargesList[particleIndex] * m_particleChargesList[i] / (4 * M_PI * EPS0 * distance * distance));
                }
                if (Agros2D::problem()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool())
                {
                    Point3 r0, v0;
                    if (Agros2D::problem()->config()->coordinateType() == CoordinateType_Planar)
                    {
                        r0 = Point3((position.x - particlePosition.x) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.z - particlePosition.z) / distance);
                        v0 = Point3(velocity.x - particleVelocity.x,
                                    velocity.y - particleVelocity.y,
                                    velocity.z - particleVelocity.z);
                    }
                    else
                    {
                        r0 = Point3((position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z)) / distance,
                                    (position.y - particlePosition.y) / distance,
                                    (position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)) / distance);
                        // TODO: fix velocity
                        assert(0);
                        v0 = Point3(velocity.x * cos(position.z) - particleVelocity.x * cos(particlePosition.z),
                                    velocity.y - particleVelocity.y,
                                    velocity.x * sin(position.z) - particleVelocity.x * sin(particlePosition.z));
                    }

                    forceP2PMagnetic = forceP2PMagnetic + (v0 % v0 % r0)
                            * (m_particleChargesList[particleIndex] * m_particleChargesList[i] * MU0 / (4 * M_PI * distance * distance));

                }
            }
        }
    }
    */

    return (forceP2PElectric + forceP2PMagnetic);
}

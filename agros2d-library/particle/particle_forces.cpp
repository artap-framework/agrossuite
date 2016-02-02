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
#include "particle_tracing.h"

#include "util.h"
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

ParticleTracingForce::ParticleTracingForce(ParticleTracing *particleTracing)
    : m_particleTracing(particleTracing)
{
    particleTracing->addExternalForce(this);
}

Point3 ParticleTracingForceCustom::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    return Point3(m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleCustomForceX).toDouble(),
                  m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleCustomForceY).toDouble(),
                  m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleCustomForceZ).toDouble());
}

Point3 ParticleTracingForceDrag::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    Point3 velocityReal = (m_particleTracing->computation()->config()->coordinateType() == CoordinateType_Planar) ?
                velocity : Point3(velocity.x, velocity.y, position.x * velocity.z);

    Point3 forceDrag;
    if (velocityReal.magnitude() > 0.0)
        forceDrag = velocityReal.normalizePoint() *
                - 0.5 * m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleDragDensity).toDouble()
                * velocityReal.magnitude() * velocityReal.magnitude()
                * m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleDragCoefficient).toDouble()
                * m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleDragReferenceArea).toDouble();

    return forceDrag;
}

ParticleTracingForceField::ParticleTracingForceField(ParticleTracing *particleTracing,
                                                     QList<double> particleChargesList)
    : ParticleTracingForce(particleTracing), m_particleChargesList(particleChargesList)
{
    foreach (FieldInfo* fieldInfo, m_particleTracing->computation()->fieldInfos())
    {
        int timeStep = m_particleTracing->computation()->solutionStore()->lastTimeStep(fieldInfo);
        int adaptivityStep = m_particleTracing->computation()->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

        m_forceValue[fieldInfo] = fieldInfo->plugin()->force(m_particleTracing->computation(), fieldInfo, timeStep, adaptivityStep);
    }
}

Point3 ParticleTracingForceField::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    Point3 totalFieldForce;

    foreach (FieldInfo* fieldInfo, m_particleTracing->computation()->fieldInfos())
    {
        if (!m_forceValue[fieldInfo]->hasForce())
            continue;

        Point3 fieldForce;

        try
        {
            fieldForce = m_forceValue[fieldInfo]->force(position, velocity) * m_particleChargesList[particleIndex];
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

ParticleTracingForceFieldP2P::ParticleTracingForceFieldP2P(ParticleTracing *particleTracing,
                                                           QList<double> particleChargesList, QList<double> particleMassesList)
    : ParticleTracingForce(particleTracing), m_particleChargesList(particleChargesList), m_particleMassesList(particleMassesList)
{
}

Point3 ParticleTracingForceFieldP2P::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    // particle to particle force
    Point3 forceP2PElectric;
    Point3 forceP2PMagnetic;

    if (m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool() ||
            m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool())
    {
        for (int i = 0; i < m_particleTracing->positions().size(); i++)
        {
            if (particleIndex == i)
                continue;

            int timeLevel = m_particleTracing->timeToLevel(i, m_particleTracing->times()[particleIndex].last());
            Point3 particlePosition = m_particleTracing->positions()[i].at(timeLevel);
            Point3 particleVelocity = m_particleTracing->velocities()[i].at(timeLevel);

            double distance = 0.0;
            if (m_particleTracing->computation()->config()->coordinateType() == CoordinateType_Planar)
                distance = Point3(position.x - particlePosition.x,
                                  position.y - particlePosition.y,
                                  position.z - particlePosition.z).magnitude();
            else
                distance = Point3(position.x * cos(position.z) - particlePosition.x * cos(particlePosition.z),
                                  position.y - particlePosition.y,
                                  position.x * sin(position.z) - particlePosition.x * sin(particlePosition.z)).magnitude();

            if (distance > 0)
            {
                if (m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleP2PElectricForce).toBool())
                {
                    if (m_particleTracing->computation()->config()->coordinateType() == CoordinateType_Planar)
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
                if (m_particleTracing->computation()->setting()->value(ProblemSetting::View_ParticleP2PMagneticForce).toBool())
                {
                    Point3 r0, v0;
                    if (m_particleTracing->computation()->config()->coordinateType() == CoordinateType_Planar)
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

    return (forceP2PElectric + forceP2PMagnetic);
}

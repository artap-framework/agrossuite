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

#include "particle_tracing.h"

#include "util/util.h"
#include "util/constants.h"

#include "solver/problem.h"
#include "solver/plugin_interface.h"

#include "util/util.h"
#include "value.h"
#include "logview.h"
#include "scene.h"

#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"

#include "solver/field.h"
#include "solver/solutionstore.h"
#include "solver/problem_config.h"

ParticleTracing::ParticleTracing(Computation *computation, QList<double> particleMassesList, QObject *parent)
    : QObject(parent), m_computation(computation), m_particleMassesList(particleMassesList)
{
}

ParticleTracing::~ParticleTracing()
{
}

void ParticleTracing::clear()
{
    // clear lists
    for (int i = 0; m_positionsList.size(); i++)
        m_positionsList[i].clear();
    m_positionsList.clear();
    for (int i = 0; m_velocitiesList.size(); i++)
        m_velocitiesList[i].clear();
    m_velocitiesList.clear();
    for (int i = 0; m_timesList.size(); i++)
        m_timesList[i].clear();
    m_timesList.clear();

    m_velocityModuleMin =  numeric_limits<double>::max();
    m_velocityModuleMax = -numeric_limits<double>::max();
}

// input position, velocity: planar x, y, z, axi r, z, phi
// output x, y, z
Point3 ParticleTracing::force(int particleIndex, const Point3 &position, const Point3 &velocity)
{
    // Total force
    Point3 totalForce;
    foreach (ParticleTracingForce *force, m_forces)
        totalForce = totalForce + force->force(particleIndex, position, velocity);

    return totalForce;
}

bool ParticleTracing::newtonEquations(int particleIndex,
                                      double step,
                                      Point3 position,
                                      Point3 velocity,
                                      Point3 *newposition,
                                      Point3 *newvelocity)
{
    // relativistic correction
    double mass = m_particleMassesList[particleIndex];
    if (m_computation->setting()->value(PostprocessorSetting::ParticleIncludeRelativisticCorrection).toBool())
    {
        Point3 velocityReal = (m_computation->config()->coordinateType() == CoordinateType_Planar) ?
                    velocity : Point3(velocity.x, velocity.y, position.x * velocity.z);

        mass = mass / (sqrt(1.0 - (velocityReal.magnitude() * velocityReal.magnitude()) / (SPEEDOFLIGHT * SPEEDOFLIGHT)));
    }

    // Total acceleration
    Point3 totalAccel = force(particleIndex, position, velocity) / mass;

    if (m_computation->config()->coordinateType() == CoordinateType_Planar)
    {
        // position
        *newposition = velocity * step;

        // velocity
        *newvelocity = totalAccel * step;
    }
    else
    {
        (*newposition).x = velocity.x * step; // r
        (*newposition).y = velocity.y * step; // z
        (*newposition).z = velocity.z * step; // alpha

        (*newvelocity).x = (totalAccel.x + velocity.z * velocity.z * position.x) * step; // r
        (*newvelocity).y = (totalAccel.y) * step; // z
        (*newvelocity).z = (position.x < EPS_ZERO) ? 0 : (totalAccel.z / position.x - 2 / position.x * velocity.x * velocity.z) * step; // alpha
    }

    return true;
}

int ParticleTracing::timeToLevel(int particleIndex, double time)
{
    if (m_timesList[particleIndex].size() == 1)
        return 0;
    else if (time >= m_timesList[particleIndex].last())
        return m_timesList[particleIndex].size() - 1;
    else
        for (int i = 0; i < m_timesList[particleIndex].size() - 1; i++)
            if ((m_timesList[particleIndex].at(i) <= time) && (time <= m_timesList[particleIndex].at(i+1)))
                return i;

    assert(0);
    return 0;
}

void ParticleTracing::computeTrajectoryParticles(const QList<Point3> initialPositions, const QList<Point3> initialVelocities)
{
    assert(initialPositions.size() == initialVelocities.size());    
    assert(initialPositions.size() == m_computation->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt());

    ButcherTable butcher((ButcherTableType) m_computation->setting()->value(PostprocessorSetting::ParticleButcherTableType).toInt());

    clear();

    int numberOfParticles = m_computation->setting()->value(PostprocessorSetting::ParticleNumberOfParticles).toInt();

    QElapsedTimer timePart;
    timePart.start();

    RectPoint bound = m_computation->scene()->boundingBox();

    double maxStep = (m_computation->setting()->value(PostprocessorSetting::ParticleMaximumStep).toDouble() > 0.0)
            ? m_computation->setting()->value(PostprocessorSetting::ParticleMaximumStep).toDouble() :
              min(bound.width(), bound.height()) / 80.0;
    double relErrorMax = (m_computation->setting()->value(PostprocessorSetting::ParticleMaximumRelativeError).toDouble() > 0.0)
            ? m_computation->setting()->value(PostprocessorSetting::ParticleMaximumRelativeError).toDouble() : 1e-6;
    double relErrorMin = 1e-3;

    // given velocity
    QList<bool> stopComputation;
    QList<int> numberOfSteps;
    QList<double> timeStep;

    // initial positions
    for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
    {
        // position and velocity cache
        m_positionsList.append(QList<Point3>());
        m_velocitiesList.append(QList<Point3>());
        m_timesList.append(QList<double>());

        m_positionsList[particleIndex].append(initialPositions[particleIndex]);
        m_velocitiesList[particleIndex].append(initialVelocities[particleIndex]);
        m_timesList[particleIndex].append(0);

        stopComputation.append(false);
        numberOfSteps.append(0);

        // timeStep.append(initialVelocities[particleIndex].magnitude() > 0
        //                 ? qMax(bound.width(), bound.height()) / initialVelocities[particleIndex].magnitude() / 10 : 1e-11);
        timeStep.append(1e-11);
    }

    bool globalStopComputation = false;
    while (!globalStopComputation)
    {
        double syncTime = 0.0;
        int syncParticle = -1;
        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
            if (m_timesList[particleIndex].last() > syncTime)
            {
                syncTime = m_timesList[particleIndex].last();
                syncParticle = particleIndex;
            }

        double timeStp = 0.0;
        if (syncParticle == -1)
            for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
                if (timeStep[particleIndex] > timeStp)
                {
                    timeStp = timeStep[particleIndex];
                    syncParticle = particleIndex;
                }

        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
        {
            // stop on number of steps
            if (numberOfSteps[particleIndex] > m_computation->setting()->value(PostprocessorSetting::ParticleMaximumNumberOfSteps).toInt() - 1)
                stopComputation[particleIndex] = true;

            // stop on time steps
            if (timeStep[particleIndex] < EPS_ZERO / 100.0)
                stopComputation[particleIndex] = true;

            if (stopComputation[particleIndex])
                continue;

            // sync
            if (!stopComputation[particleIndex] && particleIndex == syncParticle)
            {
                bool otherParticlesIsRunning = false;
                for (int particleIndexOther = 0; particleIndexOther < numberOfParticles; particleIndexOther++)
                    if (particleIndex != particleIndexOther && !stopComputation[particleIndexOther])
                        otherParticlesIsRunning = true;

                if (otherParticlesIsRunning)
                    continue;
            }

            // increase number of steps
            numberOfSteps[particleIndex]++;

            // initial position and velocity
            Point3 position = m_positionsList[particleIndex].last();
            Point3 velocity = m_velocitiesList[particleIndex].last();
            if (m_computation->config()->coordinateType() == CoordinateType_Axisymmetric)
                velocity.z = velocity.z / position.x; // v_phi = omega * r
            double currentTimeStep = timeStep[particleIndex];
            // qDebug() << currentTimeStep;

            // Runge-Kutta steps
            Point3 newPositionH;
            Point3 newVelocityH;

            // Butcher tableu
            QVector<Point3> kp(butcher.get_size());
            QVector<Point3> kv(butcher.get_size());

            int maxStepsRKF = 0;
            while (!stopComputation[particleIndex] && maxStepsRKF < 100)
            {                
                bool butcherOK = true;

                for (int k = 0; k < butcher.get_size(); k++)
                {
                    Point3 pos = position;
                    Point3 vel = velocity;

                    for (int l = 0; l < butcher.get_size(); l++)
                    {
                        if (l < k)
                        {
                            pos = pos + kp[l] * butcher.get_A(k, l);
                            vel = vel + kv[l] * butcher.get_A(k, l);
                        }
                    }

                    if ((m_computation->setting()->value(PostprocessorSetting::ParticleIncludeRelativisticCorrection).toBool())
                            && ((m_computation->config()->coordinateType() == CoordinateType_Planar
                                 ? vel.magnitude() : Point3(vel.x, vel.y, pos.x * vel.z).magnitude()) > SPEEDOFLIGHT))
                    {
                        // decrease time step
                        butcherOK = false;
                        break;
                    }

                    newtonEquations(particleIndex, currentTimeStep, pos, vel, &kp[k], &kv[k]);
                }

                if (butcherOK)
                {
                    // low order
                    Point3 newPositionL = position;
                    Point3 newVelocityL = velocity;
                    for (int k = 0; k < butcher.get_size() - 1; k++)
                    {
                        newPositionL = newPositionL + kp[k] * butcher.get_B2(k);
                        newVelocityL = newVelocityL + kv[k] * butcher.get_B2(k);
                    }

                    // high order
                    newPositionH = position;
                    newVelocityH = velocity;
                    for (int k = 0; k < butcher.get_size(); k++)
                    {
                        newPositionH = newPositionH + kp[k] * butcher.get_B(k);
                        newVelocityH = newVelocityH + kv[k] * butcher.get_B(k);
                    }

                    // optimal step estimation
                    double absErrorPos = fabs(newPositionH.magnitude() - newPositionL.magnitude());
                    double relErrorPos = fabs(absErrorPos / newPositionH.magnitude());
                    double absErrorVel = fabs(newVelocityH.magnitude() - newVelocityL.magnitude());
                    double relErrorVel = fabs(absErrorVel / newVelocityH.magnitude());
                    double currentStepLength = ((m_computation->config()->coordinateType() == CoordinateType_Planar) ?
                                                    (position - newPositionH).magnitude() :
                                                    (Point3(position.x * cos(position.z), position.x * sin(position.z), position.y)
                                                     - Point3(newPositionH.x * cos(newPositionH.z), newPositionH.x * sin(newPositionH.z), newPositionH.y)).magnitude());
                    double currentStepVelocity = ((m_computation->config()->coordinateType() == CoordinateType_Planar) ?
                                                      (velocity - newVelocityH).magnitude() :
                                                      (Point3(velocity.x, velocity.y, position.x * velocity.z) - Point3(newVelocityH.x, newVelocityH.y, newPositionH.x * newVelocityH.z)).magnitude());

                    // zero step
                    if (currentStepLength < EPS_ZERO && currentStepVelocity < EPS_ZERO && newVelocityH.magnitude() < EPS_ZERO)
                    {
                        qDebug() << QString("Particle %1: zero step - stop computation.").arg(particleIndex);
                        stopComputation[particleIndex] = true;
                        break;
                    }

                    // nearly zero step
                    // qDebug() << "currentTimeStep" << currentTimeStep << "currentStepLength" << currentStepLength << "currentStepVelocity" << currentStepVelocity << "absErrorPos" << absErrorPos << "relErrorPos" << relErrorPos << "absErrorVel" << absErrorVel << "relErrorVel" << relErrorVel;
                    if (currentStepLength < EPS_ZERO && currentStepVelocity < EPS_ZERO)
                    {
                        qDebug() << QString("Particle %1: time step is too short - refused.").arg(particleIndex);
                        currentTimeStep *= 3.0;                        
                        continue;
                    }

                    // minimum step
                    if ((currentStepLength > maxStep) || (relErrorVel > relErrorMax && relErrorPos > relErrorMax))
                    {
                        // decrease step
                        qDebug() << QString("Particle %1: time step is too long or relative error was exceeded - refused.").arg(particleIndex);
                        currentTimeStep /= 2.0;
                        continue;
                    }
                    // relative tolerance
                    else if ((relErrorVel < relErrorMin && relErrorPos < relErrorMin))
                    {
                        // increase next step
                        qDebug() << QString("Particle %1: time step increased.").arg(particleIndex);
                        double optStep = 0.8 * currentTimeStep * pow((relErrorMin / relErrorPos), 0.25);
                        if (relErrorPos > 0 && optStep > currentTimeStep)
                            timeStep[particleIndex] = optStep;
                        else
                            timeStep[particleIndex] = 1.2 * currentTimeStep;
                        break;
                    }
                    else
                    {
                        // store current time step
                        timeStep[particleIndex] = currentTimeStep;
                        break;
                    }
                }
                else
                {
                    if (currentTimeStep < EPS_ZERO / 100.0)
                    {
                        // store current time step
                        timeStep[particleIndex] = currentTimeStep;
                        // stop computation
                        break;
                    }
                    else
                    {
                        // decrease step
                        qDebug() << QString("Particle %1: the speed of light was exceeded - refused.").arg(particleIndex);
                        currentTimeStep /= 2.0;
                        continue;
                    }
                }

                maxStepsRKF++;
            }

            // check crossing
            QMap<SceneFace *, Point> intersections;
            foreach (SceneFace *edge, m_computation->scene()->faces->items())
            {
                QList<Point> incts = intersection(Point(position.x, position.y), Point(newPositionH.x, newPositionH.y),
                                                  Point(), 0.0, 0.0,
                                                  edge->nodeStart()->point(), edge->nodeEnd()->point(),
                                                  edge->center(), edge->radius(), edge->angle());

                if (incts.length() > 0)
                {
                    foreach (Point p, incts)
                    {
                        intersections.insert(edge, p);
                    }
                }
            }

            // find the closest intersection
            Point intersect;
            SceneFace *crossingEdge = NULL;
            double distance = numeric_limits<double>::max();
            for (QMap<SceneFace *, Point>::const_iterator it = intersections.begin(); it != intersections.end(); ++it)
                if ((it.value() - Point(position.x, position.y)).magnitude() < distance)
                {
                    distance = (it.value() - Point(position.x, position.y)).magnitude();

                    crossingEdge = it.key();
                    intersect = it.value();
                }

            if (crossingEdge && distance > EPS_ZERO)
            {
                bool impact = false;
                foreach (FieldInfo* fieldInfo, m_computation->fieldInfos())
                {
                    if ((m_computation->setting()->value(PostprocessorSetting::ParticleCoefficientOfRestitution).toDouble() < EPS_ZERO) || // no reflection
                            (crossingEdge->marker(fieldInfo) == m_computation->scene()->boundaries->getNone(fieldInfo)
                             && !m_computation->setting()->value(PostprocessorSetting::ParticleReflectOnDifferentMaterial).toBool()) || // inner edge
                            (crossingEdge->marker(fieldInfo) != m_computation->scene()->boundaries->getNone(fieldInfo)
                             && !m_computation->setting()->value(PostprocessorSetting::ParticleReflectOnBoundary).toBool())) // boundary
                        impact = true;
                }

                // current step ration
                if (impact)
                {
                    newPositionH.x = intersect.x;
                    newPositionH.y = intersect.y;

                    // qDebug() << particleIndex << "impact";
                    stopComputation[particleIndex] = true;
                }
                else
                {
                    // input vector moved to the origin
                    Point vectin = Point(newPositionH.x, newPositionH.y) - intersect;

                    // tangent vector
                    Point tangent;
                    if (crossingEdge->isStraight())
                        tangent = (crossingEdge->nodeStart()->point() - crossingEdge->nodeEnd()->point()).normalizePoint();
                    else
                        tangent = Point((intersect.y - crossingEdge->center().y), -(intersect.x - crossingEdge->center().x)).normalizePoint();

                    Point idealReflectedPosition(intersect.x + (((tangent.x * tangent.x) - (tangent.y * tangent.y)) * vectin.x + 2.0*tangent.x*tangent.y * vectin.y),
                                                 intersect.y + (2.0*tangent.x*tangent.y * vectin.x + ((tangent.y * tangent.y) - (tangent.x * tangent.x)) * vectin.y));

                    double ratio = (Point(position.x, position.y) - intersect).magnitude()
                            / (Point(newPositionH.x, newPositionH.y) - Point(position.x, position.y)).magnitude();

                    // output vector
                    Point vectout = (idealReflectedPosition - intersect).normalizePoint();

                    // stop computation (impact distance is very very small)
                    if ((fabs(distance / 100.0 * vectout.x) < EPS_ZERO) && (fabs(distance / 100.0 * vectout.y) < EPS_ZERO))
                        stopComputation[particleIndex] = true;

                    // output point
                    newPositionH.x = intersect.x + distance / 100.0 * vectout.x;
                    newPositionH.y = intersect.y + distance / 100.0 * vectout.y;

                    // velocity in the direction of output vector
                    Point3 oldv = newVelocityH;
                    newVelocityH.x = vectout.x * Point(oldv.x, oldv.y).magnitude() * m_computation->setting()->value(PostprocessorSetting::ParticleCoefficientOfRestitution).toDouble();
                    newVelocityH.y = vectout.y * Point(oldv.x, oldv.y).magnitude() * m_computation->setting()->value(PostprocessorSetting::ParticleCoefficientOfRestitution).toDouble();

                    // set new timestep
                    currentTimeStep = currentTimeStep * ratio;
                    timeStep[particleIndex] = currentTimeStep;
                }
            }

            // new values
            velocity = newVelocityH;
            position = newPositionH;

            // add to the lists
            m_timesList[particleIndex].append(m_timesList[particleIndex].last() + currentTimeStep);
            m_positionsList[particleIndex].append(position);

            // velocities in planar and axisymmetric arrangement
            if (m_computation->config()->coordinateType() == CoordinateType_Planar)
                m_velocitiesList[particleIndex].append(velocity);
            else
                m_velocitiesList[particleIndex].append(Point3(velocity.x, velocity.y, position.x * velocity.z)); // v_phi = omega * r
        }

        // global stop
        bool stop = true;
        for (int particleIndex = 0; particleIndex < numberOfParticles; particleIndex++)
            stop = stop && stopComputation[particleIndex];
        globalStopComputation = stop;
    }

    // velocity min and max value
    for (int i = 0; i < m_velocitiesList.length(); i++)
    {
        for (int j = 0; j < m_velocitiesList[i].length(); j++)
        {
            double velocity = m_velocitiesList[i][j].magnitude();

            if (velocity < m_velocityModuleMin) m_velocityModuleMin = velocity;
            if (velocity > m_velocityModuleMax) m_velocityModuleMax = velocity;
        }
    }

    //qDebug() << "total particle: " << timePart.elapsed();
}

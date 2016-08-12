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

#ifndef PARTICLETRACING_H
#define PARTICLETRACING_H

#include "util/util.h"
#include "util/enums.h"
#include "util/point.h"
#include "util/global.h"
#include "util/conf.h"

#include "solver/solutiontypes.h"
#include "particle_forces.h"

class FieldInfo;
class SceneMaterial;

class ParticleTracing : public QObject
{
    Q_OBJECT

public:
    ParticleTracing(Computation *computation, QList<double> particleMassesList, QObject *parent = 0);
    ~ParticleTracing();

    void inline addExternalForce(ParticleTracingForce *force) { m_forces.append(force); }

    void clear();

    void computeTrajectoryParticles(const QList<Point3> initialPositions, const QList<Point3> initialVelocities);

    // output
    inline QList<QList<Point3> > positions() const { return m_positionsList; }
    inline QList<QList<Point3> > velocities() const { return m_velocitiesList; }
    inline QList<QList<double> > times() const { return m_timesList; }
    int timeToLevel(int particleIndex, double time);

    inline double velocityModuleMin() const { return m_velocityModuleMin; }
    inline double velocityModuleMax() const { return m_velocityModuleMax; }

    inline Computation *computation() { return m_computation; }

private:
    // computation
    Computation *m_computation;

    QList<ParticleTracingForce *> m_forces;
    // masses
    QList<double> m_particleMassesList;

    // output
    QList<QList<Point3> > m_positionsList;
    QList<QList<Point3> > m_velocitiesList;
    QList<QList<double> > m_timesList;

    double m_velocityModuleMin;
    double m_velocityModuleMax;

    Point3 force(int particleIndex, const Point3 &position, const Point3 &velocity);

    bool newtonEquations(int particleIndex,
                         double step,
                         Point3 position,
                         Point3 velocity,
                         Point3 *newposition,
                         Point3 *newvelocity);    
};

#endif /* PARTICLETRACING_H */

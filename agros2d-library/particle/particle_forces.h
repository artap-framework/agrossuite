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

#ifndef PARTICLETRACING_FORCES_H
#define PARTICLETRACING_FORCES_H

#include "util.h"
#include "util/enums.h"
#include "util/point.h"
#include "util/global.h"
#include "util/conf.h"

#include "solver/solutiontypes.h"
#include "solver/plugin_interface.h"

class FieldInfo;
class SceneMaterial;

class ParticleTracingForce
{
public:
    ParticleTracingForce(ParticleTracing *m_particleTracing);

    virtual ~ParticleTracingForce() {}

    virtual Point3 force(int particleIndex, const Point3 &position, const Point3 &velocity) = 0;

protected:
    ParticleTracing *m_particleTracing;
};

// custom force
class ParticleTracingForceCustom : public ParticleTracingForce
{
public:
    ParticleTracingForceCustom(ParticleTracing *particleTracing) : ParticleTracingForce(particleTracing) {}

    virtual Point3 force(int particleIndex, const Point3 &position, const Point3 &velocity);
};

// drag force
class ParticleTracingForceDrag : public ParticleTracingForce
{
public:
    ParticleTracingForceDrag(ParticleTracing *particleTracing) : ParticleTracingForce(particleTracing) {}

    virtual Point3 force(int particleIndex, const Point3 &position, const Point3 &velocity);
};

// field force
class ParticleTracingForceField : public ParticleTracingForce
{
public:
    ParticleTracingForceField(ParticleTracing *m_particleTracing, QList<double> particleChargesList);

    virtual Point3 force(int particleIndex, const Point3 &position, const Point3 &velocity);

private:
    QMap<FieldInfo *, std::shared_ptr<ForceValue> > m_forceValue;

    QList<double> m_particleChargesList;
};

// field p2p force
class ParticleTracingForceFieldP2P : public ParticleTracingForce
{
public:
    ParticleTracingForceFieldP2P(ParticleTracing *m_particleTracing, QList<double> particleChargesList, QList<double> particleMassesList);

    virtual Point3 force(int particleIndex, const Point3 &position, const Point3 &velocity);

private:
    QMap<FieldInfo *, FieldSolutionID> m_solutionIDs;

    QList<double> m_particleChargesList;
    QList<double> m_particleMassesList;
};

#endif /* PARTICLETRACING_FORCES_H */

cdef extern from "../../agros2d-library/pythonlab/pyparticletracing.h":
    cdef cppclass PyParticleTracing:
        PyParticleTracing()
        void setComputation(PyComputation *computation)

        void getInitialPosition(vector[double] &position)
        void setInitialPosition(vector[double] &position) except +

        void getInitialVelocity(vector[double] &velocity)
        void setInitialVelocity(vector[double] &velocity)

        void setNumberOfParticles(int particles)  except +
        int getNumberOfParticles()

        void setStartingRadius(double radius) except +
        double getStartingRadius()

        double getParticleMass()
        void setParticleMass(double mass) except +

        double getParticleCharge()
        void setParticleCharge(double charge)

        double getDragForceDensity()
        void setDragForceDensity(double rho) except +
        double getDragForceReferenceArea()
        void setDragForceReferenceArea(double area) except +
        double getDragForceCoefficient()
        void setDragForceCoefficient(double coeff) except +

        void getCustomForce(vector[double] &force) except +
        void setCustomForce(vector[double] &force) except +

        bool getElectrostaticInteraction()
        void setElectrostaticInteraction(bool interaction)
        bool getMagneticInteraction()
        void setMagneticInteraction(bool interaction)

        bool getIncludeRelativisticCorrection()
        void setIncludeRelativisticCorrection(bool incl)

        string getButcherTableType()
        void setButcherTableType(string &tableType) except +
        double getMaximumRelativeError()
        void setMaximumRelativeError(double tolerance) except +
        int getMaximumNumberOfSteps()
        void setMaximumNumberOfSteps(int steps) except +
        double getMaximumStep()
        void setMaximumStep(double step) except +

        bool getReflectOnDifferentMaterial()
        void setReflectOnDifferentMaterial(bool reflect)
        bool getReflectOnBoundary()
        void setReflectOnBoundary(bool reflect)
        double getCoefficientOfRestitution()
        void setCoefficientOfRestitution(double coeff)  except +

        bool getColorByVelocity()
        void setColorByVelocity(bool show)
        bool getShowPoints()
        void setShowPoints(bool show)
        bool getShowBlendedFaces()
        void setShowBlendedFaces(bool show)
        int getNumShowParticlesAxi()
        void setNumShowParticlesAxi(int particles)  except +

        void solve(vector[vector[double]] &initialPositions,  vector[vector[double]] &initialVelocities,
                   vector[double] &particleCharges, vector[double] &particleMasses) except +

        int length()
        void positions(vector[vector[double]] &x, vector[vector[double]] &y, vector[vector[double]] &z)
        void velocities(vector[vector[double]] &vx, vector[vector[double]] &vy, vector[vector[double]] &vz)
        void times(vector[vector[double]] &t)

cdef class __ParticleTracing__:
    cdef PyParticleTracing *_tracing

    def __cinit__(self):
        self._tracing = new PyParticleTracing()
    def __dealloc__(self):
        del self._tracing

    def solve(self, initial_positions = [], initial_velocities = [],
                    particle_charges = [], particle_masses = []):

        cdef vector[vector[double]] initial_positions_vector, initial_velocities_vector

        for position in initial_positions:
            initial_positions_vector.push_back(list_to_double_vector(position))

        for velocity in initial_velocities:
            initial_velocities_vector.push_back(list_to_double_vector(velocity))

        self._tracing.solve(initial_positions_vector,
                           initial_velocities_vector,
                           list_to_double_vector(particle_charges),
                           list_to_double_vector(particle_masses))

    """
    def length(self):
        return self._tracing.length()
    """

    def positions(self):
        cdef vector[vector[double]] x, y, z
        self._tracing.positions(x, y, z)

        assert x.size() == y.size() == z.size()

        out = [[], [], []]
        for i in range(x.size()):
            out[0].append(double_vector_to_list(x[i]))
            out[1].append(double_vector_to_list(y[i]))
            out[2].append(double_vector_to_list(z[i]))

        return out

    def velocities(self):
        cdef vector[vector[double]] vx, vy, vz
        self._tracing.velocities(vx, vy, vz)

        assert vx.size() == vy.size() == vz.size()

        out = [[], [], []]
        for i in range(vx.size()):
            out[0].append(double_vector_to_list(vx[i]))
            out[1].append(double_vector_to_list(vy[i]))
            out[2].append(double_vector_to_list(vz[i]))

        return out

    def times(self):
        cdef vector[vector[double]] t
        self._tracing.times(t)

        out = []
        for i in range(t.size()):
            out.append(double_vector_to_list(t[i]))

        return out

    property number_of_particles:
        def __get__(self):
            return self._tracing.getNumberOfParticles()
        def __set__(self, particles):
            self._tracing.setNumberOfParticles(particles)

    property particles_dispersion:
        def __get__(self):
            return self._tracing.getStartingRadius()
        def __set__(self, dispersion):
            self._tracing.setStartingRadius(dispersion)

    property initial_position:
        def __get__(self):
            cdef vector[double] position
            self._tracing.getInitialPosition(position)
            return double_vector_to_list(position)
        def __set__(self, position):
            self._tracing.setInitialPosition(list_to_double_vector(position))

    property initial_velocity:
        def __get__(self):
            cdef vector[double] velocity
            self._tracing.getInitialVelocity(velocity)
            return double_vector_to_list(velocity)
        def __set__(self, velocity):
            self._tracing.setInitialVelocity(list_to_double_vector(velocity))

    property mass:
        def __get__(self):
            return self._tracing.getParticleMass()
        def __set__(self, mass):
            self._tracing.setParticleMass(mass)

    property charge:
        def __get__(self):
            return self._tracing.getParticleCharge()
        def __set__(self, charge):
            self._tracing.setParticleCharge(charge)

    property include_relativistic_correction:
        def __get__(self):
            return self._tracing.getIncludeRelativisticCorrection()
        def __set__(self, correction):
            self._tracing.setIncludeRelativisticCorrection(correction)

    property reflect_on_different_material:
        def __get__(self):
            return self._tracing.getReflectOnDifferentMaterial()
        def __set__(self, reflect):
            self._tracing.setReflectOnDifferentMaterial(reflect)

    property reflect_on_boundary:
        def __get__(self):
            return self._tracing.getReflectOnBoundary()
        def __set__(self, reflect):
            self._tracing.setReflectOnBoundary(reflect)

    property coefficient_of_restitution:
        def __get__(self):
            return self._tracing.getCoefficientOfRestitution()
        def __set__(self, coeff):
            self._tracing.setCoefficientOfRestitution(coeff)

    property drag_force_density:
        def __get__(self):
            return self._tracing.getDragForceDensity()
        def __set__(self, density):
            self._tracing.setDragForceDensity(density)

    property drag_force_reference_area:
        def __get__(self):
            return self._tracing.getDragForceReferenceArea()
        def __set__(self, area):
            self._tracing.setDragForceReferenceArea(area)

    property drag_force_coefficient:
        def __get__(self):
            return self._tracing.getDragForceCoefficient()
        def __set__(self, coeff):
            self._tracing.setDragForceCoefficient(coeff)

    property custom_force:
        def __get__(self):
            cdef vector[double] force
            self._tracing.getCustomForce(force)
            return double_vector_to_list(force)
        def __set__(self, force):
            self._tracing.setCustomForce(list_to_double_vector(force))

    property electrostatic_interaction:
        def __get__(self):
            return self._tracing.getElectrostaticInteraction()
        def __set__(self, interaction):
            self._tracing.setElectrostaticInteraction(interaction)

    property magnetic_interaction:
        def __get__(self):
            return self._tracing.getMagneticInteraction()
        def __set__(self, interaction):
            self._tracing.setMagneticInteraction(interaction)

    property butcher_table_type:
        def __get__(self):
            return self._tracing.getButcherTableType().c_str()
        def __set__(self, table_type):
            self._tracing.setButcherTableType(table_type.encode())

    property maximum_number_of_steps:
        def __get__(self):
            return self._tracing.getMaximumNumberOfSteps()
        def __set__(self, steps):
            self._tracing.setMaximumNumberOfSteps(steps)

    property maximum_relative_error:
        def __get__(self):
            return self._tracing.getMaximumRelativeError()
        def __set__(self, tolerance):
            self._tracing.setMaximumRelativeError(tolerance)

    property maximum_step:
        def __get__(self):
            return self._tracing.getMaximumStep()
        def __set__(self, step):
            self._tracing.setMaximumStep(step)

    property collor_by_velocity:
        def __get__(self):
            return self._tracing.getColorByVelocity()
        def __set__(self, show):
            self._tracing.setColorByVelocity(show)

    property show_points:
        def __get__(self):
            return self._tracing.getShowPoints()
        def __set__(self, show):
            self._tracing.setShowPoints(show)

    property blended_faces:
        def __get__(self):
            return self._tracing.getShowBlendedFaces()
        def __set__(self, show):
            self._tracing.setShowBlendedFaces(show)

    property multiple_show_particles:
        def __get__(self):
            return self._tracing.getNumShowParticlesAxi()
        def __set__(self, particles):
            self._tracing.setNumShowParticlesAxi(particles)
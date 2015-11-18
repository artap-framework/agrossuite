cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyComputation:
        PyComputation(bool newComputation)
        PyComputation(string computation) except +

        void clear() except +
        void mesh() except +
        void solve() except +

        double timeElapsed() except +
        void timeStepsLength(vector[double] &steps) except +
        void timeStepsTimes(vector[double] &times) except +

        void getResults(vector[string] &keys)
        double getResult(string &key) except +
        void setResult(string &key, double value)

        void getParameters(vector[string] &keys)
        double getParameter(string &key) except +

        string getCoordinateType()
        string getMeshType()
        double getFrequency()
        string getTimeStepMethod()
        int getTimeMethodOrder()
        double getTimeMethodTolerance()
        double getTimeTotal()
        int getNumConstantTimeSteps()
        double getInitialTimeStep()
        string getCouplingType(string &sourceField, string &targetField) except +

cdef class __Computation__:
    cdef PyComputation *_computation
    cdef object _solutions
    cdef object parameters
    cdef object results

    def __cinit__(self, new_computation=True, computation = None):
        if not computation:
            self._computation = new PyComputation(<bool> new_computation)
        elif isinstance(computation, str):
            self._computation = new PyComputation(<string> computation.encode())
        else:
            raise TypeError("Parameter type is not supported.")

        self._solutions = dict()
        self.parameters = __Parameters__(self.__get_parameters__,
                                         self.__unauthorized__, False)
        self.results = __Parameters__(self.__get_results__,
                                      self.__set_results__, False)

    def __dealloc__(self):
        del self._computation

    def __unauthorized__(self, value = None):
        raise Exception("Value can not be changed.")

    def clear(self):
        """Clear solution."""
        self._computation.clear()

    def mesh(self):
        """Area discretization."""
        self._computation.mesh()

    def solve(self):
        """Solve problem."""
        self._computation.solve()

    def solution(self, field_id):
        """Add new solution to Computation.

        solution(field_id)

        Keyword arguments:
        field_id -- field keyword 
        """
        if (not field_id in self._solutions):
            solution = __Solution__()
            solution._solution.setComputation(self._computation, field_id.encode())
            self._solutions[field_id] = solution

        return self._solutions[field_id]

    def particle_tracing(self):
        """Create and return new object of ParticleTracing class."""
        tracing = __ParticleTracing__()
        tracing._tracing.setComputation(self._computation)

        return tracing

    def elapsed_time(self):
        """Return elapsed time in seconds."""
        return self._computation.timeElapsed()

    def time_steps_length(self):
        """Return a list of time steps length."""
        cdef vector[double] steps_vector
        self._computation.timeStepsLength(steps_vector)

        steps = list()
        for i in range(steps_vector.size()):
            steps.append(steps_vector[i])
        return steps

    def time_steps_total(self):
        """Return a list of time steps."""
        cdef vector[double] times_vector
        self._computation.timeStepsTimes(times_vector)

        times = list()
        for i in range(times_vector.size()):
            times.append(times_vector[i])
        return times

    # results
    property results:
        def __get__(self):
            return self.results.get_parameters()

    def __get_results__(self):
        cdef vector[string] results_vector
        self._computation.getResults(results_vector)

        results = dict()
        for i in range(results_vector.size()):
            results[(<string>results_vector[i]).decode()] = self._problem.getResult(results_vector[i])
        return results
    def __set_results__(self, results):
        for key in results:
            self._computation.setResult(key.encode(), <double>results[key])

    # parameters
    property parameters:
        def __get__(self):
            return self.parameters.get_parameters()

    def __get_parameters__(self):
        cdef vector[string] parameters_vector
        self._computation.getParameters(parameters_vector)

        parameters = dict()
        for i in range(parameters_vector.size()):
            parameters[(<string>parameters_vector[i]).decode()] = self._computation.getParameter(parameters_vector[i])
        return parameters

    # coordinate type
    coordinate_type = property(_get_coordinate_type, __unauthorized__)
    def _get_coordinate_type(self):
        return self._computation.getCoordinateType().decode()

    # mesh type
    mesh_type = property(_get_mesh_type, __unauthorized__)
    def _get_mesh_type(self):
            return self._computation.getMeshType().decode()

    # frequency
    frequency = property(_get_frequency, __unauthorized__)
    def _get_frequency(self):
            return self._computation.getFrequency()

    # time step method
    time_step_method = property(_get_time_step_method, __unauthorized__)
    def _get_time_step_method(self):
            return self._computation.getTimeStepMethod().decode()

    # time method order
    time_method_order = property(_get_time_method_order, __unauthorized__)
    def _get_time_method_order(self):
            return self._computation.getTimeMethodOrder()

    # time method tolerance
    time_method_tolerance = property(_get_time_method_tolerance, __unauthorized__)            
    def _get_time_method_tolerance(self):
            return self._computation.getTimeMethodTolerance()

    # time total
    time_total = property(_get_time_total, __unauthorized__)
    def _get_time_total(self):
            return self._computation.getTimeTotal()

    # time steps
    time_steps = property(_get_time_steps, __unauthorized__)
    def _get_time_steps(self):
            return self._computation.getNumConstantTimeSteps()

    # initial time step
    initial_time_step = property(_get_initial_time_step, __unauthorized__)
    def _get_initial_time_step(self):
            return self._computation.getInitialTimeStep()

    def get_coupling_type(self, source_field, target_field):
        """Return type of coupling.

        get_coupling_type(source_field, target_field)

        Keyword arguments:
        source_field -- source field id
        target_field -- target field id
        """
        return self._computation.getCouplingType(source_field.encode(), target_field.encode()).decode()

def computation(computation):
    return __Computation__(computation)
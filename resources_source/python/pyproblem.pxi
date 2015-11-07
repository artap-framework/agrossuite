cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyProblemBase:
        PyProblemBase()

        void refresh()

        string getCoordinateType()
        string getMeshType()
        double getFrequency()
        string getTimeStepMethod()
        int getTimeMethodOrder()
        double getTimeMethodTolerance()
        double getTimeTotal()
        int getNumConstantTimeSteps()
        double getTimeInitialTimeStep()
        void getParameters(vector[string] &keys)
        double getParameter(string &key) except +
        string getCouplingType(string &sourceField, string &targetField) except +

    cdef cppclass PyProblem:
        PyProblem(bool clear)

        void clear()

        void setCoordinateType(string &coordinateType) except +
        void setMeshType(string &meshType) except +
        void setFrequency(double frequency) except +
        void setTimeStepMethod(string &timeStepMethod) except +
        void setTimeMethodOrder(int timeMethodOrder) except +
        void setTimeMethodTolerance(double timeMethodTolerance) except +
        void setTimeTotal(double timeTotal) except +
        void setNumConstantTimeSteps(int timeSteps) except +
        void setTimeInitialTimeStep(double timeInitialTimeStep) except +
        void setParameter(string &key, double value)
        void setCouplingType(string &sourceField, string &targetField, string &type) except +

    cdef cppclass PyComputation:
        PyComputation(bool newComputation)

        void clearSolution() except +

        void mesh() except +
        void solve() except +

        double timeElapsed() except +
        void timeStepsLength(vector[double] &steps) except +
        void timeStepsTimes(vector[double] &times) except +

cdef class __ProblemBase__:
    cdef PyProblemBase *base
    cdef object _time_callback
    cdef object parameters

    def __cinit__(self):
        self.base = new PyProblemBase()
        self._time_callback = None

    def __dealloc__(self):
        del self.base

    def refresh(self):
        """Refresh preprocessor and postprocessor."""
        self.base.refresh()

    def _unauthorized(self):
        raise Exception("Parameter can not be changed.")

    def _get_coordinate_type(self):
        return self.base.getCoordinateType().decode()
    coordinate_type = property(_get_coordinate_type, _unauthorized)

    def _get_mesh_type(self):
            return self.base.getMeshType().decode()
    mesh_type = property(_get_mesh_type, _unauthorized)

    def _get_frequency(self):
            return self.base.getFrequency()
    frequency = property(_get_frequency, _unauthorized)

    def _get_time_step_method(self):
            return self.base.getTimeStepMethod().decode()
    time_step_method = property(_get_time_step_method, _unauthorized)

    def _get_time_method_order(self):
            return self.base.getTimeMethodOrder()
    time_method_order = property(_get_time_method_order, _unauthorized)
            
    def _get_time_method_tolerance(self):
            return self.base.getTimeMethodTolerance()
    time_method_tolerance = property(_get_time_method_tolerance, _unauthorized)

    def _get_time_total(self):
            return self.base.getTimeTotal()
    time_total = property(_get_time_total, _unauthorized)

    def _get_time_steps(self):
            return self.base.getNumConstantTimeSteps()
    time_steps = property(_get_time_steps, _unauthorized)

    def _get_time_initial_time_step(self):
            return self.base.getTimeInitialTimeStep()
    time_initial_time_step = property(_get_time_initial_time_step, _unauthorized)

    def _get_time_callback(self):
            return self._time_callback
    time_callback = property(_get_time_callback, _unauthorized)

    def get_coupling_type(self, source_field, target_field):
        """Return type of coupling.

        get_coupling_type(source_field, target_field)

        Keyword arguments:
        source_field -- source field id
        target_field -- target field id
        """
        return self.base.getCouplingType(source_field.encode(), target_field.encode()).decode()

cdef class __Problem__(__ProblemBase__):
    cdef PyProblem *problem

    def __cinit__(self, clear = False):
        self.problem = new PyProblem(clear)
        __ProblemBase__.__init__(self)

    def __dealloc__(self):
        del self.problem

    def clear(self):
        """Clear problem."""
        self._time_callback = None
        self.problem.clear()

    def _set_coordinate_type(self, coordinate_type):
        self.problem.setCoordinateType(coordinate_type.encode())
    coordinate_type = property(__ProblemBase__._get_coordinate_type, _set_coordinate_type)

    def _set_mesh_type(self, mesh_type):
            self.problem.setMeshType(mesh_type.encode())
    mesh_type = property(__ProblemBase__._get_mesh_type, _set_mesh_type)

    def _set_frequency(self, frequency):
            self.problem.setFrequency(frequency)
    frequency = property(__ProblemBase__._get_frequency, _set_frequency)

    def _set_time_step_method(self, time_step_method):
            self.problem.setTimeStepMethod(time_step_method.encode())
    time_step_method = property(__ProblemBase__._get_time_step_method, _set_time_step_method)

    def _set_time_method_order(self, time_method_order):
            self.problem.setTimeMethodOrder(time_method_order)
    time_method_order = property(__ProblemBase__._get_time_method_order, _set_time_method_order)

    def _set_time_method_tolerance(self, time_method_tolerance):
            self.problem.setTimeMethodTolerance(time_method_tolerance)
    time_method_tolerance = property(__ProblemBase__._get_time_method_tolerance, _set_time_method_tolerance)

    def _set_time_total(self, time_total):
            self.problem.setTimeTotal(time_total)
    time_total = property(__ProblemBase__._get_time_total, _set_time_total)

    def _set_time_steps(self, time_steps):
            self.problem.setNumConstantTimeSteps(time_steps)
    time_steps = property(__ProblemBase__._get_time_steps, _set_time_steps)

    def _set_time_initial_time_step(self, time_initial_time_step):
            self.problem.setTimeInitialTimeStep(time_initial_time_step)
    time_initial_time_step = property(__ProblemBase__._get_time_initial_time_step, _set_time_initial_time_step)

    def _set_time_callback(self, callback):
            self._time_callback = callback
    time_callback = property(__ProblemBase__._get_time_callback, _set_time_callback)

    def set_coupling_type(self, source_field, target_field, type):
        """Set type of coupling.

        set_coupling_type(source_field, target_field, type)

        Keyword arguments:
        source_field -- source field id
        target_field -- target field id
        type -- coupling type
        """
        self.problem.setCouplingType(source_field.encode(), target_field.encode(), type.encode())

cdef class __Computation__(__ProblemBase__):
    cdef PyComputation *computation

    def __cinit__(self, new_computation = False):
        self.computation = new PyComputation(new_computation)
        __ProblemBase__.__init__(self)

    def __dealloc__(self):
        del self.computation

    def clear_solution(self):
        """Clear solution."""
        self.computation.clearSolution()

    def mesh(self):
        """Area discretization."""
        self.computation.mesh()

    def solve(self):
        """Solve problem."""
        self.computation.solve()

    def elapsed_time(self):
        """Return elapsed time in seconds."""
        return self.computation.timeElapsed()

    def time_steps_length(self):
        """Return a list of time steps length."""
        cdef vector[double] steps_vector
        self.computation.timeStepsLength(steps_vector)

        steps = list()
        for i in range(steps_vector.size()):
            steps.append(steps_vector[i])

        return steps

    def time_steps_total(self):
        """Return a list of time steps."""
        cdef vector[double] times_vector
        self.computation.timeStepsTimes(times_vector)

        times = list()
        for i in range(times_vector.size()):
            times.append(times_vector[i])

        return times

__problem__ = __Problem__()
def problem(clear = False):
    if (clear):
        __problem__.clear()

    return __problem__

def computation():
    return __Computation__()
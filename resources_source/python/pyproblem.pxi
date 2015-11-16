cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyProblemBase:
        PyProblemBase()

        void refresh()

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
        double getTimeInitialTimeStep()
        string getCouplingType(string &sourceField, string &targetField) except +

    cdef cppclass PyProblem:
        PyProblem(bool clear)

        void clear()

        void setParameter(string &key, double value)
        void setCoordinateType(string &coordinateType) except +
        void setMeshType(string &meshType) except +
        void setFrequency(double frequency) except +
        void setTimeStepMethod(string &timeStepMethod) except +
        void setTimeMethodOrder(int timeMethodOrder) except +
        void setTimeMethodTolerance(double timeMethodTolerance) except +
        void setTimeTotal(double timeTotal) except +
        void setNumConstantTimeSteps(int timeSteps) except +
        void setTimeInitialTimeStep(double timeInitialTimeStep) except +
        void setCouplingType(string &sourceField, string &targetField, string &type) except +

cdef class __ProblemBase__:
    cdef PyProblemBase *_base
    cdef object _time_callback
    cdef object _geometry

    def __cinit__(self):
        self._base = new PyProblemBase()
        self._time_callback = None
        self._geometry = __Geometry__()

    def __dealloc__(self):
        del self._base

    def refresh(self):
        """Refresh preprocessor and postprocessor."""
        self.base.refresh()

    def geometry(self):
        """Get geometry object."""
        return self._geometry

    def _unauthorized(self):
        raise Exception("Value can not be changed.")

    def _get_parameters(self):
        cdef vector[string] params_vector
        self._base.getParameters(params_vector)

        params = dict()
        for i in range(params_vector.size()):
            params[(<string>params_vector[i]).decode()] = self._base.getParameter(params_vector[i])

        return params
    parameters = property(_get_parameters, _unauthorized)

    def _get_coordinate_type(self):
        return self._base.getCoordinateType().decode()
    coordinate_type = property(_get_coordinate_type, _unauthorized)

    def _get_mesh_type(self):
            return self._base.getMeshType().decode()
    mesh_type = property(_get_mesh_type, _unauthorized)

    def _get_frequency(self):
            return self._base.getFrequency()
    frequency = property(_get_frequency, _unauthorized)

    def _get_time_step_method(self):
            return self._base.getTimeStepMethod().decode()
    time_step_method = property(_get_time_step_method, _unauthorized)

    def _get_time_method_order(self):
            return self._base.getTimeMethodOrder()
    time_method_order = property(_get_time_method_order, _unauthorized)
            
    def _get_time_method_tolerance(self):
            return self._base.getTimeMethodTolerance()
    time_method_tolerance = property(_get_time_method_tolerance, _unauthorized)

    def _get_time_total(self):
            return self._base.getTimeTotal()
    time_total = property(_get_time_total, _unauthorized)

    def _get_time_steps(self):
            return self._base.getNumConstantTimeSteps()
    time_steps = property(_get_time_steps, _unauthorized)

    def _get_time_initial_time_step(self):
            return self._base.getTimeInitialTimeStep()
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
        return self._base.getCouplingType(source_field.encode(), target_field.encode()).decode()

cdef class __Problem__(__ProblemBase__):
    cdef PyProblem *_problem
    cdef object _fields

    def __cinit__(self, clear = False):
        __ProblemBase__.__init__(self)
        self._problem = new PyProblem(clear)
        self._fields = dict()

    def __dealloc__(self):
        del self._problem

    def clear(self):
        """Clear problem."""
        self._time_callback = None
        self._problem.clear()

    def field(self, field_id):
        """Add new field to problem and return Field() object.

        field(field_id)

        Keyword arguments:
        field_id -- field keyword 
        """

        if (not field_id in self._fields):
            self._fields[field_id] = __Field__(field_id)
        return self._fields[field_id]

    def __remove_field__(self, field_id):
        if (field_id in self._fields):
            del self._fields[field_id]

    def computation(self):
        """Create and return new Computation() object."""
        return __Computation__()

    def _set_parameters(self, params):
        for key in params:
            self._problem.setParameter(key.encode(), <double>params[key])
    parameters = property(__ProblemBase__._get_parameters, _set_parameters)

    def _set_coordinate_type(self, coordinate_type):
        self._problem.setCoordinateType(coordinate_type.encode())
    coordinate_type = property(__ProblemBase__._get_coordinate_type, _set_coordinate_type)

    def _set_mesh_type(self, mesh_type):
            self._problem.setMeshType(mesh_type.encode())
    mesh_type = property(__ProblemBase__._get_mesh_type, _set_mesh_type)

    def _set_frequency(self, frequency):
            self._problem.setFrequency(frequency)
    frequency = property(__ProblemBase__._get_frequency, _set_frequency)

    def _set_time_step_method(self, time_step_method):
            self._problem.setTimeStepMethod(time_step_method.encode())
    time_step_method = property(__ProblemBase__._get_time_step_method, _set_time_step_method)

    def _set_time_method_order(self, time_method_order):
            self._problem.setTimeMethodOrder(time_method_order)
    time_method_order = property(__ProblemBase__._get_time_method_order, _set_time_method_order)

    def _set_time_method_tolerance(self, time_method_tolerance):
            self._problem.setTimeMethodTolerance(time_method_tolerance)
    time_method_tolerance = property(__ProblemBase__._get_time_method_tolerance, _set_time_method_tolerance)

    def _set_time_total(self, time_total):
            self._problem.setTimeTotal(time_total)
    time_total = property(__ProblemBase__._get_time_total, _set_time_total)

    def _set_time_steps(self, time_steps):
            self._problem.setNumConstantTimeSteps(time_steps)
    time_steps = property(__ProblemBase__._get_time_steps, _set_time_steps)

    def _set_time_initial_time_step(self, time_initial_time_step):
            self._problem.setTimeInitialTimeStep(time_initial_time_step)
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
        self._problem.setCouplingType(source_field.encode(), target_field.encode(), type.encode())

__problem__ = __Problem__()
def problem(clear = False):
    if (clear):
        __problem__.clear()

    return __problem__
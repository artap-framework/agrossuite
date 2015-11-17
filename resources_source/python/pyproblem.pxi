cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyProblem:
        PyProblem(bool clear)

        void clear()
        void refresh()

        void getParameters(vector[string] &keys)
        double getParameter(string &key) except +
        void setParameter(string &key, double value)

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

cdef class __Problem__:
    cdef PyProblem *_problem
    cdef object _time_callback
    cdef object _fields
    cdef object _geometry
    cdef object parameters

    def __cinit__(self, clear = False):
        self._problem = new PyProblem(clear)
        self._time_callback = None
        self._fields = dict()
        self._geometry = __Geometry__()

        self.parameters = __Parameters__(self.__get_parameters__,
                                         self.__set_parameters__,
                                         False)

    def __dealloc__(self):
        del self._problem

    def clear(self):
        """Clear problem."""
        self._time_callback = None
        self._problem.clear()

    def refresh(self):
        """Refresh preprocessor and postprocessor."""
        self._problem.refresh()

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

    def geometry(self):
        """Get geometry object."""
        return self._geometry

    def computation(self):
        """Create and return new Computation() object."""
        return __Computation__()

    # parameters
    property parameters:
        def __get__(self):
            return self.parameters.get_parameters()

    def __get_parameters__(self):
        cdef vector[string] parameters_vector
        self._problem.getParameters(parameters_vector)

        parameters = dict()
        for i in range(parameters_vector.size()):
            parameters[(<string>parameters_vector[i]).decode()] = self._problem.getParameter(parameters_vector[i])
        return parameters
    def __set_parameters__(self, parameters):
        for key in parameters:
            self._problem.setParameter(key.encode(), <double>parameters[key])

    # coordinate type
    coordinate_type = property(_get_coordinate_type, _set_coordinate_type)
    def _get_coordinate_type(self):
        return self._problem.getCoordinateType().decode()
    def _set_coordinate_type(self, coordinate_type):
        self._problem.setCoordinateType(coordinate_type.encode())

    # mesh type
    mesh_type = property(_get_mesh_type, _set_mesh_type)
    def _get_mesh_type(self):
        return self._problem.getMeshType().decode()
    def _set_mesh_type(self, mesh_type):
        self._problem.setMeshType(mesh_type.encode())

    # frequency
    frequency = property(_get_frequency, _set_frequency)
    def _get_frequency(self):
            return self._problem.getFrequency()
    def _set_frequency(self, frequency):
        self._problem.setFrequency(frequency)

    # time step method
    time_step_method = property(_get_time_step_method, _set_time_step_method)
    def _get_time_step_method(self):
        return self._problem.getTimeStepMethod().decode()
    def _set_time_step_method(self, time_step_method):
            self._problem.setTimeStepMethod(time_step_method.encode())

    # time method order
    time_method_order = property(_get_time_method_order, _set_time_method_order)
    def _get_time_method_order(self):
        return self._problem.getTimeMethodOrder()
    def _set_time_method_order(self, time_method_order):
            self._problem.setTimeMethodOrder(time_method_order)

    # time method tolerance
    time_method_tolerance = property(_get_time_method_tolerance, _set_time_method_tolerance)
    def _get_time_method_tolerance(self):
        return self._problem.getTimeMethodTolerance()
    def _set_time_method_tolerance(self, time_method_tolerance):
            self._problem.setTimeMethodTolerance(time_method_tolerance)

    # time total
    time_total = property(_get_time_total, _set_time_total)
    def _get_time_total(self):
        return self._problem.getTimeTotal()
    def _set_time_total(self, time_total):
            self._problem.setTimeTotal(time_total)

    # time steps
    time_steps = property(_get_time_steps, _get_time_steps)
    def _get_time_steps(self):
        return self._problem.getNumConstantTimeSteps()
    def _set_time_steps(self, time_steps):
            self._problem.setNumConstantTimeSteps(time_steps)

    # initial time step
    initial_time_step = property(_get_initial_time_step, _set_initial_time_step)
    def _get_initial_time_step(self):
        return self._problem.getTimeInitialTimeStep()
    def _set_initial_time_step(self, initial_time_step):
            self._problem.setTimeInitialTimeStep(initial_time_step)

    # time callback
    time_callback = property(_get_time_callback, _set_time_callback)
    def _get_time_callback(self):
            return self._time_callback
    def _set_time_callback(self, callback):
            self._time_callback = callback

    def get_coupling_type(self, source_field, target_field):
        """Return type of coupling.

        get_coupling_type(source_field, target_field)

        Keyword arguments:
        source_field -- source field id
        target_field -- target field id
        """
        return self._problem.getCouplingType(source_field.encode(), target_field.encode()).decode()

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
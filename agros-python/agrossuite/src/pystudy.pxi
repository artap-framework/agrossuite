cdef extern from "../../agros-python/pythonlab/pystudy.h":
    cdef cppclass PyStudy:
        PyStudy()

        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +
        void setParameter(string &parameter, string value) except +

        bool getBoolParameter(string &parameter) except +
        int getIntParameter(string &parameter) except +
        double getDoubleParameter(string &parameter) except +
        string getStringParameter(string &parameter) except +

        string type()

        void addParameter(string name, double lowerBound, double upperBound) except +
        void addGoalFunction(string name, string expression, int weight) except +
        void getParameters(vector[string] &name, vector[double] &lowerBound, vector[double] &upperBound) except +
        void getGoalFunctions(vector[string] &name, vector[string] &expression, vector[int] &weight) except +

        void solve() except +

        # postprocessor
        string findExtreme(string type, string key, bool minimum) except +

        void steps(vector[int] &steps)
        void values(string variable, vector[double] &values)
        void results(vector[int] &steps, vector[string] &names, vector[string] &types, vector[double] &values)

        void getComputationProblemDir(int index, string &problemDir) except +

    cdef cppclass PyStudyBayesOpt(PyStudy):
        PyStudyBayesOpt(int index)

        string getInitMethod()
        void setInitMethod(string &initMethod) except +

        string getSurrName()
        void setSurrName(string &surrName) except +

        string getScoreType()
        void setScoreType(string &scoreType) except +

        string getLearningType()
        void setLearningType(string &learningType) except +

    cdef cppclass PyStudyNLopt(PyStudy):
        PyStudyNLopt(int index)

        string getAlgorithm()
        void setAlgorithm(string &algorithm) except +

    cdef cppclass PyStudyPagmo(PyStudy):
        PyStudyPagmo(int index)

        string getAlgorithm()
        void setAlgorithm(string &algorithm) except +

    cdef cppclass PyStudySweep(PyStudy):
        PyStudySweep(int index)

        string getInitMethod()
        void setInitMethod(string &initMethod) except +
        
    cdef cppclass PyStudyModel(PyStudy):
        PyStudyModel(int index)

        # string getInitMethod()
        # void setInitMethod(string &initMethod) except +
        

cdef class __Study__:
    cdef PyStudy *thisptr
    cdef object settings

    def __dealloc__(self):
        del self.thisptr

    def add_parameter(self, name, lower_bound, upper_bound):
        self.thisptr.addParameter(name.encode(), lower_bound, upper_bound)

    def add_goal_function(self, name, expression, weight = 100):
        self.thisptr.addGoalFunction(name.encode(), expression.encode(), weight)

    def parameters(self):	
        cdef vector[string] name
        cdef vector[double] lowerBound
        cdef vector[double] upperBound
        self.thisptr.getParameters(name, lowerBound, upperBound)
        
        out = {}
        for i in range(name.size()):
            out[name[i].decode()] = [lowerBound[i], upperBound[i]]

        return out
        
    def goal_functions(self):	
        cdef vector[string] name
        cdef vector[string] expression
        cdef vector[int] bound
        self.thisptr.getGoalFunctions(name, expression, bound)
        
        out = {}
        for i in range(name.size()):
            out[name[i].decode()] = [expression[i].decode(), bound[i]]

        return out        

    def type(self):
        return self.thisptr.type();

    # settings
    property settings:
        def __get__(self):
            return self.settings.get_parameters()

    property clear_solution:
        def __get__(self):
            return self.thisptr.getBoolParameter(b'General_ClearSolution')
        def __set__(self, val):
            self.thisptr.setParameter(string(b'General_ClearSolution'), <bool> val)

    property solve_problem:
        def __get__(self):
            return self.thisptr.getBoolParameter(b'General_SolveProblem')
        def __set__(self, val):
            self.thisptr.setParameter(string(b'General_SolveProblem'), <bool> val)

    # solve study
    def solve(self):
        self.thisptr.solve();

    # postprocessor
    def find_extreme(self, type, key, minimum = True):
        cdef string problemDir = self.thisptr.findExtreme(type.encode(), key.encode(), minimum)
        if (problemDir.decode() != ""):
            return __Computation__(computation = problemDir.decode())

    def steps(self):
        cdef vector[int] steps
        self.thisptr.steps(steps)

        out = []
        for i in range(steps.size()):
            out.append(steps[i])

        return out

    def values(self, variable):
        cdef vector[double] values
        self.thisptr.values(variable.encode(), values)

        out = []
        for i in range(values.size()):
            out.append(values[i])

        return out

    def results(self):
        cdef vector[int] steps
        cdef vector[string] names
        cdef vector[string] types
        cdef vector[double] values
        self.thisptr.results(steps, names, types, values)

        out = []
        for i in range(steps.size()):
            out.append([steps[i], names[i].decode(), types[i].decode(), values[i]])

        return out

    def computation(self, index):
        cdef string problemDir
        self.thisptr.getComputationProblemDir(index, problemDir)

        if (problemDir.decode() != ""):
            return __Computation__(computation = problemDir.decode())


cdef class __StudyBayesOpt__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyBayesOpt(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'n_init_samples' : self.thisptr.getIntParameter(b'BayesOpt_n_init_samples'),
                'n_iterations' : self.thisptr.getIntParameter(b'BayesOpt_n_iterations'),
                'n_iter_relearn' : self.thisptr.getIntParameter(b'BayesOpt_n_iter_relearn'),
                'init_method' : (<PyStudyBayesOpt*> self.thisptr).getInitMethod().decode(),
                'surr_name' : (<PyStudyBayesOpt*> self.thisptr).getSurrName().decode(),
                'surr_noise' : self.thisptr.getDoubleParameter(b'BayesOpt_surr_noise'),
                'sc_type' : (<PyStudyBayesOpt*> self.thisptr).getScoreType().decode(),
                'l_type' : (<PyStudyBayesOpt*> self.thisptr).getLearningType().decode()}

    def __set_settings__(self, settings):
        positive_value(settings['n_init_samples'], 'n_init_samples')
        self.thisptr.setParameter(string(b'BayesOpt_n_init_samples'), <int> settings['n_init_samples'])

        positive_value(settings['n_iterations'], 'n_iterations')
        self.thisptr.setParameter(string(b'BayesOpt_n_iterations'), <int> settings['n_iterations'])

        positive_value(settings['n_iter_relearn'], 'n_iter_relearn')
        self.thisptr.setParameter(string(b'BayesOpt_n_iter_relearn'), <int> settings['n_iter_relearn'])

        positive_value(settings['surr_noise'], 'surr_noise')
        self.thisptr.setParameter(string(b'BayesOpt_surr_noise'), <double> settings['surr_noise'])

        (<PyStudyBayesOpt*> self.thisptr).setInitMethod(<string> settings['init_method'].encode())
        (<PyStudyBayesOpt*> self.thisptr).setSurrName(<string> settings['surr_name'].encode())
        (<PyStudyBayesOpt*> self.thisptr).setScoreType(<string> settings['sc_type'].encode())
        (<PyStudyBayesOpt*> self.thisptr).setLearningType(<string> settings['l_type'].encode())

cdef class __StudyNLopt__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyNLopt(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'n_iterations' : self.thisptr.getIntParameter(b'NLopt_n_iterations'),
                'xtol_rel' : self.thisptr.getDoubleParameter(b'NLopt_xtol_rel'),
                'xtol_abs' : self.thisptr.getDoubleParameter(b'NLopt_xtol_abs'),
                'ftol_rel' : self.thisptr.getDoubleParameter(b'NLopt_ftol_rel'),
                'ftol_abs' : self.thisptr.getDoubleParameter(b'NLopt_ftol_abs'),
                'algorithm' : (<PyStudyNLopt*> self.thisptr).getAlgorithm().decode()}

    def __set_settings__(self, settings):
        positive_value(settings['n_iterations'], 'n_iterations')
        self.thisptr.setParameter(string(b'NLopt_n_iterations'), <int> settings['n_iterations'])

        positive_value(settings['xtol_rel'], 'xtol_rel')
        self.thisptr.setParameter(string(b'NLopt_xtol_rel'), <double> settings['xtol_rel'])

        positive_value(settings['xtol_abs'], 'xtol_abs')
        self.thisptr.setParameter(string(b'NLopt_xtol_abs'), <double> settings['xtol_abs'])

        positive_value(settings['ftol_rel'], 'ftol_rel')
        self.thisptr.setParameter(string(b'NLopt_ftol_rel'), <double> settings['ftol_rel'])

        positive_value(settings['ftol_abs'], 'ftol_abs')
        self.thisptr.setParameter(string(b'NLopt_ftol_abs'), <double> settings['ftol_abs'])

        (<PyStudyNLopt*> self.thisptr).setAlgorithm(<string> settings['algorithm'].encode())

cdef class __StudyPagmo__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyPagmo(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'algorithm' : (<PyStudyPagmo*> self.thisptr).getAlgorithm().decode(),
                'popsize' : self.thisptr.getIntParameter(b'Pagmo_popsize'),
                'ngen' : self.thisptr.getIntParameter(b'Pagmo_ngen'))}

    def __set_settings__(self, settings):
        positive_value(settings['popsize'], 'popsize')
        self.thisptr.setParameter(string(b'Pagmo_popsize'), <int> settings['popsize'])

        positive_value(settings['ngen'], 'ngen')
        self.thisptr.setParameter(string(b'Pagmo_ngen'), <int> settings['ngen'])

cdef class __StudySweep__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudySweep(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'num_samples' : self.thisptr.getIntParameter(b'Sweep_num_samples'),
                'init_method' : (<PyStudySweep*> self.thisptr).getInitMethod().decode()}

    def __set_settings__(self, settings):
        positive_value(settings['num_samples'], 'num_samples')
        self.thisptr.setParameter(string(b'Sweep_num_samples'), <int> settings['num_samples'])

        (<PyStudySweep*> self.thisptr).setInitMethod(<string> settings['init_method'].encode())
        
cdef class __StudyModel__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyModel(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'name' : self.thisptr.getStringParameter(b'General_Name').decode(),
                'description' : self.thisptr.getStringParameter(b'General_Description').decode()}
        return {}

    def __set_settings__(self, settings):
        self.thisptr.setParameter(string(b'General_Name'), <string> settings['name'].encode())
        self.thisptr.setParameter(string(b'General_Description'), <string> settings['description'].encode())

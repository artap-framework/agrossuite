cdef extern from "../../agros2d-library/pythonlab/pystudy.h":
    cdef cppclass PyStudy:
        PyStudy()

        void setParameter(string &parameter, bool value) except +
        void setParameter(string &parameter, int value) except +
        void setParameter(string &parameter, double value) except +
        void setParameter(string &parameter, string value) except +

        bool getBoolParameter(string &parameter) except +
        int getIntParameter(string &parameter) except +
        double getDoubleParameter(string &parameter) except +

        string type()

        void addParameter(string name, double lowerBound, double upperBound) except +
        void addFunctional(string name, string expression, int weight) except +

        void solve() except +

        # postprocessor
        string findExtreme(string type, string key, bool minimum) except +

    cdef cppclass PyStudyBayesOpt(PyStudy):
        PyStudyBayesOpt()

        string getInitMethod()
        void setInitMethod(string &initMethod) except +

        string getSurrName()
        void setSurrName(string &surrName) except +

        string getScoreType()
        void setScoreType(string &scoreType) except +

        string getLearningType()
        void setLearningType(string &learningType) except +

cdef class __Study__:
    cdef PyStudy *thisptr
    cdef object settings

    def __dealloc__(self):
        del self.thisptr

    def add_parameter(self, name, lowerBound, upperBound):
        self.thisptr.addParameter(name.encode(), lowerBound, upperBound)

    def add_functional(self, name, expression, weight = 100):
        self.thisptr.addFunctional(name.encode(), expression.encode(), weight)

    def type(self):
        return self.thisptr.type();

    # settings
    property settings:
        def __get__(self):
            return self.settings.get_parameters()

    property clear_solution:
        def __get__(self):
            return self.thisptr.getBoolParameter(b'General_ClearSolution')
        def __set__(self, clear):
            self.thisptr.setParameter(string(b'General_ClearSolution'), <bool> clear)

    property solve_problem:
        def __get__(self):
            return self.thisptr.getBoolParameter(b'General_SolveProblem')
        def __set__(self, solve):
            self.thisptr.setParameter(string(b'General_SolveProblem'), <bool> solve)

    # solve study
    def solve(self):
        self.thisptr.solve();

    # postprocessor
    def find_extreme(self, type, key, minimum = True):
        cdef string problemDir = self.thisptr.findExtreme(type.encode(), key.encode(), minimum)
        if (problemDir.decode() != ""):
            return __Computation__(computation = problemDir.decode())

cdef class __StudyBayesOpt__(__Study__):
    def __cinit__(self):
        self.thisptr = new PyStudyBayesOpt()

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

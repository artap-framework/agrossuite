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

        string type()

        void addParameter(string name, double lowerBound, double upperBound, bool penaltyEnabled, double scale, double mu, double sigma) except +
        void addFunctional(string name, string expression, int weight) except +

        void solve() except +

        # postprocessor
        string findExtreme(string type, string key, bool minimum) except +
        void values(string variable, vector[double] &values)
        void steps(vector[int] &steps)

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

    cdef cppclass PyStudyLimbo(PyStudy):
        PyStudyLimbo(int index)

        string getMeanType()
        void setMeanType(string &meanType) except +

        string getGPType()
        void setGPType(string &gpType) except +

        string getAcquiType()
        void setAcquiType(string &acquiType) except +

    cdef cppclass PyStudyNLopt(PyStudy):
        PyStudyNLopt(int index)

        string getAlgorithm()
        void setAlgorithm(string &algorithm) except +

    cdef cppclass PyStudyCMAES(PyStudy):
        PyStudyCMAES(int index)

        string getAlgorithm()
        void setAlgorithm(string &algorithm) except +

        string getSurrogate()
        void setSurrogate(string &surrogate) except +

    cdef cppclass PyStudyNSGA2(PyStudy):
        PyStudyNSGA2(int index)

    cdef cppclass PyStudyNSGA3(PyStudy):
        PyStudyNSGA3(int index)

    cdef cppclass PyStudySweep(PyStudy):
        PyStudySweep(int index)

        string getInitMethod()
        void setInitMethod(string &initMethod) except +

cdef class __Study__:
    cdef PyStudy *thisptr
    cdef object settings

    def __dealloc__(self):
        del self.thisptr

    def add_parameter(self, name, lower_bound, upper_bound, penalty_enabled = False, scale = 0.0, mu = 0.0, sigma = 0.0):
        self.thisptr.addParameter(name.encode(), lower_bound, upper_bound, penalty_enabled, scale, mu, sigma)

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
        def __set__(self, val):
            self.thisptr.setParameter(string(b'General_ClearSolution'), <bool> val)

    property solve_problem:
        def __get__(self):
            return self.thisptr.getBoolParameter(b'General_SolveProblem')
        def __set__(self, val):
            self.thisptr.setParameter(string(b'General_SolveProblem'), <bool> val)

    property doe:
        def __get__(self):
            return self.thisptr.getBoolParameter(b'General_DoE')
        def __set__(self, val):
            self.thisptr.setParameter(string(b'General_DoE'), <bool> val)

    property doe_deviation:
        def __get__(self):
            return self.thisptr.getIntParameter(b'General_DoE_Deviation')
        def __set__(self, val):
            positive_value(val, 'doe_deviation')
            self.thisptr.setParameter(string(b'General_DoE_Deviation'), <double> val)

    property doe_sweep_samples:
        def __get__(self):
            return self.thisptr.getDoubleParameter(b'General_DoE_SweepSamples')
        def __set__(self, val):
            positive_value(val, 'doe_sweep_samples')
            self.thisptr.setParameter(string(b'General_DoE_SweepSamples'), <int> val)

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

cdef class __StudyLimbo__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyLimbo(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'init_randomsampling_samples' : self.thisptr.getIntParameter(b'LIMBO_init_randomsampling_samples'),
                'stop_maxiterations_iterations' : self.thisptr.getIntParameter(b'LIMBO_stop_maxiterations_iterations'),
                'bayes_opt_boptimizer_hp_period' : self.thisptr.getIntParameter(b'LIMBO_bayes_opt_boptimizer_hp_period'),
                'bayes_opt_boptimizer_noise' : self.thisptr.getDoubleParameter(b'LIMBO_bayes_opt_boptimizer_noise'),
                'mean' : (<PyStudyLimbo*> self.thisptr).getMeanType().decode(),
                'gp' : (<PyStudyLimbo*> self.thisptr).getGPType().decode(),
                'acqui' : (<PyStudyLimbo*> self.thisptr).getAcquiType().decode()}

    def __set_settings__(self, settings):
        positive_value(settings['init_randomsampling_samples'], 'init_randomsampling_samples')
        self.thisptr.setParameter(string(b'LIMBO_init_randomsampling_samples'), <int> settings['init_randomsampling_samples'])

        positive_value(settings['stop_maxiterations_iterations'], 'stop_maxiterations_iterations')
        self.thisptr.setParameter(string(b'LIMBO_stop_maxiterations_iterations'), <int> settings['stop_maxiterations_iterations'])

        positive_value(settings['bayes_opt_boptimizer_hp_period'], 'bayes_opt_boptimizer_hp_period')
        self.thisptr.setParameter(string(b'LIMBO_bayes_opt_boptimizer_hp_period'), <int> settings['bayes_opt_boptimizer_hp_period'])

        positive_value(settings['bayes_opt_boptimizer_noise'], 'bayes_opt_boptimizer_noise')
        self.thisptr.setParameter(string(b'LIMBO_bayes_opt_boptimizer_noise'), <double> settings['bayes_opt_boptimizer_noise'])

        (<PyStudyLimbo*> self.thisptr).setMeanType(<string> settings['mean'].encode())
        (<PyStudyLimbo*> self.thisptr).setGPType(<string> settings['gp'].encode())
        (<PyStudyLimbo*> self.thisptr).setAcquiType (<string> settings['acqui'].encode())

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

cdef class __StudyCMAES__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyCMAES(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'maxiter' : self.thisptr.getIntParameter(b'CMAES_maxiter'),
                'maxeval' : self.thisptr.getIntParameter(b'CMAES_maxeval'),
                'ftarget' : self.thisptr.getDoubleParameter(b'CMAES_ftarget'),
                'algorithm' : (<PyStudyCMAES*> self.thisptr).getAlgorithm().decode(),
                'surrogate' : (<PyStudyCMAES*> self.thisptr).getSurrogate().decode()}

    def __set_settings__(self, settings):
        positive_value(settings['maxiter'], 'maxiter')
        self.thisptr.setParameter(string(b'CMAES_maxiter'), <int> settings['maxiter'])

        positive_value(settings['maxeval'], 'maxeval')
        self.thisptr.setParameter(string(b'CMAES_maxeval'), <int> settings['maxeval'])

        positive_value(settings['ftarget'], 'ftarget')
        self.thisptr.setParameter(string(b'CMAES_ftarget'), <double> settings['ftarget'])

        (<PyStudyCMAES*> self.thisptr).setAlgorithm(<string> settings['algorithm'].encode())
        (<PyStudyCMAES*> self.thisptr).setSurrogate(<string> settings['surrogate'].encode())

cdef class __StudyNSGA2__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyNSGA2(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'popsize' : self.thisptr.getIntParameter(b'NSGA2_popsize'),
                'ngen' : self.thisptr.getIntParameter(b'NSGA2_ngen'),
                'pcross' : self.thisptr.getDoubleParameter(b'NSGA2_pcross'),
                'pmut' : self.thisptr.getDoubleParameter(b'NSGA2_pmut'),
                'eta_c' : self.thisptr.getDoubleParameter(b'NSGA2_eta_c'),
                'eta_m' : self.thisptr.getDoubleParameter(b'NSGA2_eta_m'),
                'crowdobj' : self.thisptr.getBoolParameter(b'NSGA2_crowdobj')}

    def __set_settings__(self, settings):
        positive_value(settings['popsize'], 'popsize')
        self.thisptr.setParameter(string(b'NSGA2_popsize'), <int> settings['popsize'])

        positive_value(settings['ngen'], 'ngen')
        self.thisptr.setParameter(string(b'NSGA2_ngen'), <int> settings['ngen'])

        positive_value(settings['pcross'], 'pcross')
        self.thisptr.setParameter(string(b'NSGA2_pcross'), <double> settings['pcross'])

        positive_value(settings['pmut'], 'pmut')
        self.thisptr.setParameter(string(b'NSGA2_pmut'), <double> settings['pmut'])

        positive_value(settings['eta_c'], 'eta_c')
        self.thisptr.setParameter(string(b'NSGA2_eta_c'), <double> settings['eta_c'])

        positive_value(settings['eta_m'], 'eta_m')
        self.thisptr.setParameter(string(b'NSGA2_eta_m'), <double> settings['eta_m'])

        positive_value(settings['crowdobj'], 'crowdobj')
        self.thisptr.setParameter(string(b'NSGA2_crowdobj'), <bool> settings['crowdobj'])

cdef class __StudyNSGA3__(__Study__):
    def __cinit__(self, index = -1):
        self.thisptr = new PyStudyNSGA3(index)

        self.settings = __Parameters__(self.__get_settings__,
                                       self.__set_settings__)

    def __get_settings__(self):
        return {'popsize' : self.thisptr.getIntParameter(b'NSGA3_popsize'),
                'ngen' : self.thisptr.getIntParameter(b'NSGA3_ngen'),
                'pcross' : self.thisptr.getDoubleParameter(b'NSGA3_pcross'),
                # 'pmut' : self.thisptr.getDoubleParameter(b'NSGA3_pmut'),
                'eta_c' : self.thisptr.getDoubleParameter(b'NSGA3_eta_c'),
                'eta_m' : self.thisptr.getDoubleParameter(b'NSGA3_eta_m')}

    def __set_settings__(self, settings):
        positive_value(settings['popsize'], 'popsize')
        self.thisptr.setParameter(string(b'NSGA3_popsize'), <int> settings['popsize'])

        positive_value(settings['ngen'], 'ngen')
        self.thisptr.setParameter(string(b'NSGA3_ngen'), <int> settings['ngen'])

        positive_value(settings['pcross'], 'pcross')
        self.thisptr.setParameter(string(b'NSGA3_pcross'), <double> settings['pcross'])

        # positive_value(settings['pmut'], 'pmut')
        # self.thisptr.setParameter(string(b'NSGA3_pmut'), <double> settings['pmut'])

        positive_value(settings['eta_c'], 'eta_c')
        self.thisptr.setParameter(string(b'NSGA3_eta_c'), <double> settings['eta_c'])

        positive_value(settings['eta_m'], 'eta_m')
        self.thisptr.setParameter(string(b'NSGA3_eta_m'), <double> settings['eta_m'])

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

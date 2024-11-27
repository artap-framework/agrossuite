from agrossuite import agros

from .scenario import AgrosTestCase


class TestBayesOptBooth(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_bayesopt = problem.add_study("bayesopt")
        study_bayesopt.add_parameter("px", -10, 10)
        study_bayesopt.add_parameter("py", -10, 10)
        study_bayesopt.add_goal_function("OF", "(px+2*py-7)**2+(2*px+py-5)**2", 100)
        
        study_bayesopt.clear_solution = True
        study_bayesopt.solve_problem = False
        
        study_bayesopt.settings["n_init_samples"] = 5
        study_bayesopt.settings["n_iterations"] = 20
        study_bayesopt.settings["n_iter_relearn"] = 5
        study_bayesopt.settings["init_method"] = "lhs"
        study_bayesopt.settings["surr_name"] = "sGaussianProcessML"
        study_bayesopt.settings["surr_noise"] = 1e-10
        study_bayesopt.settings["l_type"] = "emperical"
        study_bayesopt.settings["sc_type"] = "map"

        study_bayesopt.solve()
        
        self.computation = study_bayesopt.find_extreme("goal", "OF", True)
        
    def test_values(self):    
        self.value_test("px", self.computation.parameters["px"], 1.0)
        self.value_test("py", self.computation.parameters["py"], 3.0)
        self.lower_then_test("OF", self.computation.results["OF"], 1e-3)

        
class TestNLoptBooth(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_nlopt = problem.add_study("nlopt")
        study_nlopt.add_parameter("px", -10, 10)
        study_nlopt.add_parameter("py", -10, 10)
        study_nlopt.add_goal_function("OF", "(px+2*py-7)**2+(2*px+py-5)**2", 100)
        
        study_nlopt.clear_solution = True
        study_nlopt.solve_problem = False
     
        study_nlopt.settings["n_iterations"] = 50
        study_nlopt.settings["xtol_rel"] = 1e-06
        study_nlopt.settings["xtol_abs"] = 1e-12
        study_nlopt.settings["ftol_rel"] = 1e-06
        study_nlopt.settings["ftol_abs"] = 1e-12
        study_nlopt.settings["algorithm"] = "ln_bobyqa"
     
        study_nlopt.solve()
        
        self.computation = study_nlopt.find_extreme("goal", "OF", True)
        
    def test_values(self):    
        self.value_test("px", self.computation.parameters["px"], 1.0)
        self.value_test("py", self.computation.parameters["py"], 3.0)
        self.lower_then_test("OF", self.computation.results["OF"], 1e-10)


class TestPagmoSphere(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.parameters["px"] = 0
        problem.parameters["py"] = 0
        
        # studies
        study_pagmo = problem.add_study("pagmo")
        study_pagmo.add_parameter("px", -10, 10)
        study_pagmo.add_parameter("py", -10, 10)
        study_pagmo.add_goal_function("OF", "(px-1.0)**2+(py+2.0)**2", 100)

        study_pagmo.clear_solution = True
        study_pagmo.solve_problem = False

        study_pagmo.settings["algorithm"] = "gwo"
        study_pagmo.settings["popsize"] = 10
        study_pagmo.settings["ngen"] = 30

        study_pagmo.solve()
        
        self.computation = study_pagmo.find_extreme("goal", "OF", True)

    def test_values(self):
        self.value_test("px", self.computation.parameters["px"], 1.0)
        self.value_test("py", self.computation.parameters["py"], -2.0)
        self.lower_then_test("OF", self.computation.results["OF"], 1e-3)

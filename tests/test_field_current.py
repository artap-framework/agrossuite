from agrossuite import agros

from .scenario import AgrosTestCase


class TestCurrentPlanar(AgrosTestCase):
    def setUp(self):  
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
               
        # fields
        self.current = problem.field("current")
        self.current.analysis_type = "steadystate"
        self.current.number_of_refinements = 3
        self.current.polynomial_order = 3
        self.current.solver = "linear"
        
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        self.current.add_boundary("Zero", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("Voltage", "current_potential", {"current_potential" : 1})
        
        self.current.add_material("mat 1", {"current_conductivity" : 1e7})
        self.current.add_material("mat 2", {"current_conductivity" : 1e5})
        self.current.add_material("mat 3", {"current_conductivity" : 1e3})
        
        # geometry
        geometry = problem.geometry()
        
        # edges
        geometry.add_edge(0, 0, 0.6, 0, boundaries = {"current" : "Zero"})
        geometry.add_edge(0, 0.8, 0, 0.5, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0.5, 0, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0, 0.35, 0.5,)
        geometry.add_edge(0.35, 0.5, 0.6, 0.5,)
        geometry.add_edge(0.6, 0.8, 0.6, 0.5, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.6, 0.5, 0.6, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0.5, 0.35, 0.5,)
        geometry.add_edge(0, 0.8, 0.6, 0.8, boundaries = {"current" : "Voltage"})
        
        # labels
        geometry.add_label(0.3, 0.670924, materials = {"current" : "mat 1"})
        geometry.add_label(0.105779, 0.364111, materials = {"current" : "mat 2"})
        geometry.add_label(0.394296, 0.203668, materials = {"current" : "mat 3"})        
       
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("current")
        
        # point value
        point = solution.local_values(3.154e-01, 3.523e-01)
        self.value_test("Scalar potential", point["V"], 0.87224)
        self.value_test("Electric field", point["Er"], 1.831040)
        self.value_test("Electric field - x", point["Erx"], 1.0138)
        self.value_test("Electric field - y", point["Ery"], -1.5253)
        self.value_test("Current density", point["Jrc"], 1831.483)
        self.value_test("Current density - x", point["Jrcx"], 1013.8	)
        self.value_test("Current density - y", point["Jrcy"], -1525.3)
        self.value_test("Losses", point["pj"], 3354.2)
        
        # volume integral
        volume = solution.volume_integrals([0, 1, 2])
        self.value_test("Losses", volume["Pj"], 11792)
        
        # surface integral
        surface = solution.surface_integrals([0])
        self.value_test("Current", surface["Ir"], 3629.425713)


class TestCurrentAxisymmetric(AgrosTestCase):
    def setUp(self):  
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # fields
        self.current = problem.field("current")
        self.current.analysis_type = "steadystate"
        self.current.number_of_refinements = 1
        self.current.polynomial_order = 4
        self.current.solver = "linear"
        
        self.current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow" : 0})
        self.current.add_boundary("Ground", "current_potential", {"current_potential" : 0})
        self.current.add_boundary("Voltage", "current_potential", {"current_potential" : 10})
        self.current.add_boundary("Inlet", "current_inward_current_flow", {"current_inward_current_flow" : -3e9})
        
        self.current.add_material("Copper", {"current_conductivity" : 5.7e7})
        
        # geometry
        geometry = problem.geometry()
        
        # edges
        geometry.add_edge(0, 0.45, 0, 0, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0, 0, 0.2, 0, boundaries = {"current" : "Ground"})
        geometry.add_edge(0.2, 0, 0.2, 0.15, boundaries = {"current" : "Inlet"})
        geometry.add_edge(0.2, 0.15, 0.35, 0.45, boundaries = {"current" : "Neumann"})
        geometry.add_edge(0.35, 0.45, 0, 0.45, boundaries = {"current" : "Voltage"})
        
        # labels
        geometry.add_label(0.0933957, 0.350253, materials = {"current" : "Copper"})
                
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("current")
        
        # point value
        point = solution.local_values(0.213175, 0.25045)
        self.value_test("Scalar potential", point["V"], 5.566438)
        self.value_test("Electric field", point["Er"], 32.059116)
        self.value_test("Electric field - r", point["Err"], -11.088553)
        self.value_test("Electric field - z", point["Erz"], -30.080408)
        self.value_test("Current density", point["Jrc"], 1.82737e9)
        self.value_test("Current density - r", point["Jrcr"], -6.320475e8)
        self.value_test("Current density - z", point["Jrcz"], -1.714583e9)
        self.value_test("Losses", point["pj"], 5.858385e10)	
        
        # volume integral
        volume = solution.volume_integrals([0])
        self.value_test("Losses", volume["Pj"], 4.542019e9)
        
        # surface integral
        surface = solution.surface_integrals([1])
        self.value_test("Current", surface["Ir"], -2.166256e8)        
        

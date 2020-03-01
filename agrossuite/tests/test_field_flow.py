from agrossuite import agros

from .scenario import AgrosTestCase


class TestFlowPlanar(AgrosTestCase):
    def setUp(self):         
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        
        # flow
        self.flow = problem.field("flow")
        self.flow.analysis_type = "steadystate"
        self.flow.number_of_refinements = 1
        self.flow.polynomial_order = 2
        
        self.flow.solver = "newton"        
        self.flow.solver_parameters['residual'] = 1e-6
        self.flow.solver_parameters['relative_change_of_solutions'] = 0.01
        self.flow.solver_parameters['damping'] = 'automatic'
        self.flow.solver_parameters['damping_factor'] = 1.0
        self.flow.solver_parameters['jacobian_reuse'] = False
        
        # boundaries        
        self.flow.add_boundary("Inlet", "flow_velocity", {"flow_velocity_x" : { "expression" : "cos((y-0.135)/0.035*pi/2)" }, "flow_velocity_y" : 0})
        self.flow.add_boundary("Wall", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : 0})
        self.flow.add_boundary("Outlet", "flow_outlet", {})
        
        # materials        
        self.flow.add_material("Water", {"flow_density" : 1, "flow_force_x" : 0, "flow_force_y" : 0, "flow_viscosity" : 0.001})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(-0.35, 0.05, 0.4, 0.05, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.4, 0.05, 0.4, 0.25, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.25, 0.1, -0.2, 0.1, boundaries = {"flow" : "Wall"}, angle = 90, segments = 6)
        geometry.add_edge(-0.2, 0.1, -0.2, 0.2, boundaries = {"flow" : "Wall"}, angle = 90, segments = 6)
        geometry.add_edge(-0.2, 0.2, -0.25, 0.2, boundaries = {"flow" : "Wall"}, angle = 90, segments = 6)
        geometry.add_edge(-0.25, 0.2, -0.25, 0.1, boundaries = {"flow" : "Wall"}, angle = 90, segments = 6)
        geometry.add_edge(-0.35, 0.2, -0.05, 0.3, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.05, 0.3, 0, 0.15, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0, 0.15, 0.05, 0.1, boundaries = {"flow" : "Wall"}, angle = 90, segments = 6)
        geometry.add_edge(0.05, 0.1, 0.1, 0.15, boundaries = {"flow" : "Wall"}, angle = 90, segments = 6)
        geometry.add_edge(0.2, 0.3, 0.1, 0.15, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.3, 0.3, 0.3, 0.25, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.3, 0.25, 0.4, 0.25, boundaries = {"flow" : "Wall"})
        geometry.add_edge(0.2, 0.3, 0.3, 0.3, boundaries = {"flow" : "Outlet"})
        geometry.add_edge(-0.4, 0.17, -0.4, 0.1, boundaries = {"flow" : "Inlet"})
        geometry.add_edge(-0.35, 0.2, -0.35, 0.17, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.4, 0.17, -0.35, 0.17, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.4, 0.1, -0.35, 0.1, boundaries = {"flow" : "Wall"})
        geometry.add_edge(-0.35, 0.1, -0.35, 0.05, boundaries = {"flow" : "Wall"})
        
        geometry.add_label(-0.086153, 0.205999, materials = {"flow" : "Water"})
        geometry.add_label(-0.224921, 0.126655, materials = {"flow" : "none"})

        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("flow")
                
        # point value
        point = solution.local_values(-0.023486, 0.117842)
        self.value_test("Pressure", point["p"], 0.65205)
        self.value_test("Velocity", point["v"], 0.406233)
        self.value_test("Velocity - x", point["vx"], 0.337962)
        self.value_test("Velocity - y", point["vy"], -0.225404)
        
        # volume integral
        # volume = solution.volume_integrals([0])
        # testPj = agros.test("Losses", volume["Pj"], 10070.23937)
        
        # surface integral
        surface = solution.surface_integrals([5])
        self.value_test("Pressure force x", surface["Fpx"], -0.13155320861904402 + 0.004544859567663737)
        self.value_test("Pressure force y", surface["Fpy"], -0.002419169736737781 + 5.14286345695936E-4)
        self.value_test("Viscous force x", surface["Fvx"], -0.004544859567663737)
        self.value_test("Viscous force y", surface["Fvy"], -5.14286345695936E-4, 7)
        self.value_test("Total force x", surface["Fx"], -0.13155320861904402)
        self.value_test("Total force y", surface["Fy"], -0.002419169736737781)

class TestFlowAxisymmetric(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        
        # flow
        self.flow = problem.field("flow")
        self.flow.analysis_type = "steadystate"
        self.flow.number_of_refinements = 1
        self.flow.polynomial_order = 3
        self.flow.adaptivity_type = "disabled"
        
        self.flow.solver = "newton"
        self.flow.solver_parameters['residual'] = 0.0001
        self.flow.solver_parameters['damping'] = 'automatic'
        self.flow.solver_parameters['damping_factor'] = 1.0
        self.flow.solver_parameters['jacobian_reuse'] = False
        
        # boundaries
        self.flow.add_boundary("inlet", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : { "expression" : "-cos(r/0.25*pi/2)" }})
        self.flow.add_boundary("outlet", "flow_outlet", {})
        self.flow.add_boundary("wall", "flow_velocity", {"flow_velocity_x" : 0, "flow_velocity_y" : 0})
        self.flow.add_boundary("symmetry", "flow_symmetry", {})
        
        # materials
        self.flow.add_material("fluid", {"flow_viscosity" : 0.001, "flow_density" : 1, "flow_force_x" : 0, "flow_force_y" : 0})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0, 0.5, 0.25, 0.5, boundaries = {"flow" : "inlet"})
        geometry.add_edge(0.25, 0.5, 0.25, -0.05, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.25, -0.05, 0.15, -0.15, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.15, -0.15, 0.15, -0.5, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.05, -0.5, 0.05, -0.05, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.05, -0.05, 0.15, -0.05, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.15, -0.05, 0, 0.1, angle = 90, segments = 7, boundaries = {"flow" : "wall"})
        geometry.add_edge(0, 0.1, 0, 0.5, boundaries = {"flow" : "symmetry"})
        geometry.add_edge(0.05, -0.5, 0, -0.5, boundaries = {"flow" : "wall"})
        geometry.add_edge(0, -0.5, 0, -1.3, boundaries = {"flow" : "symmetry"})
        geometry.add_edge(0, -1.3, 0.15, -1.3, boundaries = {"flow" : "outlet"})
        geometry.add_edge(0.15, -1.3, 0.15, -0.55, boundaries = {"flow" : "wall"})
        geometry.add_edge(0.15, -0.55, 0.15, -0.5, boundaries = {"flow" : "wall"})
        
        geometry.add_label(0.127882, 0.157169, materials = {"flow" : "fluid"})
                
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("flow")
                
        # point value
        point = solution.local_values(2.137e-01, 8.173e-03)
        self.value_test("Pressure", point["p"], 2.880471)
        self.value_test("Velocity", point["v"], 0.652625)
        self.value_test("Velocity - r", point["vr"], 0.02563, 4)
        self.value_test("Velocity - z", point["vz"], -0.652122)
        
        # volume integral
        # volume = solution.volume_integrals([0])
        # testPj = agros.test("Losses", volume["Pj"], 10070.23937)
        
        # surface integral
        surface = solution.surface_integrals([6])
        self.value_test("Pressure force r", surface["Fpx"], 0.282427 + 0.006718)
        self.value_test("Pressure force z", surface["Fpy"], 0.224571 - 0.016407)
        self.value_test("Viscous force r", surface["Fvx"], -0.00737211877987858)
        self.value_test("Viscous force z", surface["Fvy"], 0.015784774172784437)
        self.value_test("Total force r", surface["Fx"], 0.28079171958463456)
        self.value_test("Total force z", surface["Fy"], 0.22369639346087752)

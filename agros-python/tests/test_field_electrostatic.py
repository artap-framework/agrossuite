from agrossuite import agros

from .scenario import AgrosTestCase


class TestElectrostaticPlanar(AgrosTestCase):
    def setUp(self):  
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"

        # fields
        self.electrostatic = problem.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 2
        self.electrostatic.solver = "linear"
        
        self.electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.electrostatic.add_boundary("U = 0 V", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("U = 1000 V", "electrostatic_potential", {"electrostatic_potential" : 1000})
        
        self.electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
        self.electrostatic.add_material("Source", {"electrostatic_charge_density" : 4e-10, "electrostatic_permittivity" : 10})
        self.electrostatic.add_material("Dieletric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 0})
        self.electrostatic.add_material("Dieletric 2", {"electrostatic_permittivity" : 10, "electrostatic_charge_density" : 0})

        # geometry
        geometry = problem.geometry()

        # edges
        geometry.add_edge(1, 2, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(4, 1, 1, 1, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(1, 2, 4, 2, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(4, 2, 4, 1, boundaries = {"electrostatic" : "U = 1000 V"})
        geometry.add_edge(20, 24, 20, 1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(20, 1, 20, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(4, 1, 20, 1, boundaries = {})
        geometry.add_edge(0, 24, 0, 1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, 0, 0, 1, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, 0, 20, 0, boundaries = {"electrostatic" : "U = 0 V"})
        geometry.add_edge(0, 24, 20, 24, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0, 1, 1, 1, boundaries = {})
        geometry.add_edge(7, 13, 14, 13, boundaries = {})
        geometry.add_edge(14, 13, 14, 18, boundaries = {})
        geometry.add_edge(14, 18, 7, 18, boundaries = {})
        geometry.add_edge(7, 18, 7, 13, boundaries = {})
        geometry.add_edge(9.5, 8, 7, 5.5)
        geometry.add_edge(7, 5.5, 14, 4)
        geometry.add_edge(13.5, 7, 14, 4)
        geometry.add_edge(13.5, 7, 9.5, 8)

        # labels
        geometry.add_label(2.78257, 1.37346, materials = {"electrostatic" : "none"})
        geometry.add_label(10.3839, 15.7187, area = 0.2, materials = {"electrostatic" : "Source"})
        geometry.add_label(3.37832, 15.8626, materials = {"electrostatic" : "Air"})
        geometry.add_label(12.3992, 0.556005, materials = {"electrostatic" : "Dieletric 1"})
        geometry.add_label(10.7019, 5.86396, materials = {"electrostatic" : "Dieletric 2"})

        # solve problem
        self.computation = problem.computation()
        self.computation.solve()

    def test_values(self):
        # point value
        solution = self.computation.solution('electrostatic')
        local_values = solution.local_values(5.06, 7.537)
        self.value_test("Scalar potential", local_values["V"], 816.2345607039165)
        self.value_test("Electric field", local_values["E"], 61.152854856080104)
        self.value_test("Electric field - x", local_values["Ex"], 36.5703725807832)
        self.value_test("Electric field - y", local_values["Ey"], -49.01305173634436)
        self.value_test("Displacement", local_values["D"], 5.414588624414737E-10)
        self.value_test("Displacement - x", local_values["Dx"], 3.2380094736792153E-10)
        self.value_test("Displacement - y", local_values["Dy"], -4.3397076555793094E-10)
        self.value_test("Energy density", local_values["we"], 1.6555878322139395E-8)
  
        # volume integral
        volume_integrals = solution.volume_integrals([4])
        self.value_test("Energy", volume_integrals["We"], 2.588874455677146E-7)
        #self.value_test("Volume Maxwell force - x", volume_integrals["Ftx"], -4.422241978223189E-8)
        #self.value_test("Volume Maxwell force - y", volume_integrals["Fty"], -4.422241978223189E-8)
        #self.value_test("Volume Maxwell torque", volume_integrals["Tt"], -1.221282851368224E-6)
        
        # surface integral
        surface_integrals = solution.surface_integrals([0, 1, 2, 3])
        self.value_test("Electric charge", surface_integrals["Q"], 1.0525248919143161E-7)
            
class TestElectrostaticAxisymmetric(AgrosTestCase):
    def setUp(self):       
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # fields
        self.electrostatic = problem.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 0
        self.electrostatic.polynomial_order = 5
        self.electrostatic.solver = "linear"
        
        self.electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : 10})
        self.electrostatic.add_boundary("Ground electrode", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Neumann BC", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        self.electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
        self.electrostatic.add_material("Dielectric n.1", {"electrostatic_charge_density" : 1e-5, "electrostatic_permittivity" : 10})
        self.electrostatic.add_material("Dielectric n.2", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 3})
        
        # geometry
        geometry = problem.geometry()
        
        # edges
        geometry.add_edge(0, 0.2, 0, 0.08, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.01, 0.08, 0.01, 0, boundaries = {"electrostatic" : "Source electrode"})
        geometry.add_edge(0.01, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.03, 0, 0.03, 0.08)
        geometry.add_edge(0.03, 0.08, 0.05, 0.08)
        geometry.add_edge(0.05, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.05, 0.08, 0.05, 0, boundaries = {"electrostatic" : "Ground electrode"})
        geometry.add_edge(0.06, 0, 0.06, 0.08, boundaries = {"electrostatic" : "Ground electrode"})
        geometry.add_edge(0.05, 0.08, 0.06, 0.08, boundaries = {"electrostatic" : "Ground electrode"})
        geometry.add_edge(0.06, 0, 0.2, 0, boundaries = {"electrostatic" : "Neumann BC"})
        geometry.add_edge(0.01, 0.08, 0.03, 0.08)
        geometry.add_edge(0.01, 0.08, 0, 0.08, boundaries = {"electrostatic" : "Source electrode"})
        geometry.add_edge(0.2, 0, 0, 0.2, boundaries = {"electrostatic" : "Neumann BC"}, angle=90)
        
        # labels
        geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric n.1"})
        geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric n.2"})
        geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
        
        # solve
        self.computation = problem.computation()
        self.computation.solve()

    def test_values(self):
        # point value
        solution = self.computation.solution("electrostatic")
        point = solution.local_values(0.0255872, 0.0738211)
        
        self.value_test("Scalar potential", point["V"], 25.89593)
        self.value_test("Electric field", point["E"], 151.108324)
        self.value_test("Electric field - r", point["Er"], 94.939342)
        self.value_test("Electric field - z", point["Ez"], 117.559546)
        self.value_test("Displacement", point["D"], 1.337941e-8)
        self.value_test("Displacement - r", point["Dr"], 8.406108e-9)
        self.value_test("Displacement - z", point["Dz"], 1.040894e-8)
        self.value_test("Energy density", point["we"], 1.01087e-6)
                
        # volume integral
        volume = solution.volume_integrals([0, 1, 2])
        self.value_test("Energy", volume["We"], 1.799349e-8)
    
        # surface integral
        surface = solution.surface_integrals([1, 12])
        self.value_test("Electric charge", surface["Q"], -1.291778e-9)

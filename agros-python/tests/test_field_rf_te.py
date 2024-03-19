from agrossuite import agros

from .scenario import AgrosTestCase


class TestRFTEHarmonicPlanar(AgrosTestCase):
    def setUp(self):          
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.frequency = 1.6e10
        
        # fields
        self.rf = problem.field("rf_te")
        self.rf.analysis_type = "harmonic"
        self.rf.number_of_refinements = 2	
        self.rf.polynomial_order = 3
        self.rf.solver = "linear"
        
        # boundaries
        self.rf.add_boundary("Perfect electric conductor", "rf_te_electric_field")
        self.rf.add_boundary("Matched boundary", "rf_te_impedance", { "rf_te_impedance" : 377 })
        self.rf.add_boundary("Surface current", "rf_te_surface_current", {"rf_te_surface_current_real" : 1, "rf_te_surface_current_imag" : 0.5})
        
        self.rf.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 3e-2})
        
        # geometry
        geometry = problem.geometry()
        
        # edges
        geometry.add_edge(-0.01, 0.02286, -0.01, 0, 0, boundaries = {"rf_te" : "Surface current"})
        geometry.add_edge(0.06907, 0.02286, 0.076, 0.01593, 90, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, 0.01593, 0.081, 0.01593, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, 0.01593, 0.081, 0.02286, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.086, 0.00693, 0.086, 0, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.086, 0.00693, 0.081, 0.00693, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, 0.00693, 0.05907, 0, 20, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(-0.01, 0, 0.05907, 0, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.17, 0.02286, 0.081, 0.02286, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.06907, 0.02286, -0.01, 0.02286, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.19286, 0, 0.17, 0.02286, 90, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.19286, 0, 0.19286, -0.04, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.175, -0.04, 0.175, -0.015, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.175, -0.015, 0.16, 0, 90, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.175, -0.04, 0.19286, -0.04, 0, boundaries = {"rf_te" : "Matched boundary"})
        geometry.add_edge(0.086, 0, 0.16, 0, 0, boundaries = {"rf_te" : "Perfect electric conductor"})
        
        # labels
        geometry.add_label(0.0359418, 0.0109393, materials = {"rf_te" : "Air"})
                
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("rf_te")
                 
        # point value
        point = solution.local_values(0.019107, 0.016725)
        self.value_test("Electric field", point["E"], 456.810483)
        self.value_test("Electric field - real", point["Er"], 146.6280)
        self.value_test("Electric field - imag", point["Ei"], 434.18829)
        self.value_test("Flux density", point["B"], 1.115591e-6)
        self.value_test("Flux density - x - real", point["Brx"], 6.483596e-7)
        self.value_test("Flux density - x - imag", point["Bix"], -2.41665e-7)
        self.value_test("Flux density - y - real", point["Bry"], -4.300969e-7)
        self.value_test("Flux density - y - imag", point["Biy"], -7.656443e-7)
        self.value_test("Magnetic field", point["H"], 0.887759)
        self.value_test("Magnetic field - x - real", point["Hrx"], 0.515948)
        self.value_test("Magnetic field - x - imag", point["Hix"], -0.19231)
        self.value_test("Magnetic field - y - real", point["Hry"], -0.34226)
        self.value_test("Magnetic field - y - imag", point["Hiy"], -0.60928)
        self.value_test("Poynting vector - x", point["Nx"], 156.567066)
        self.value_test("Poynting vector - y", point["Ny"], -3.138616, 1)
        
        # volume integral
        # volume_integrals = solution.volume_integrals([0, 1, 2])
        # testEnergy = agros.test("Energy", volume["We"], 1.799349e-8)
        
        # surface integral
        # surface_integrals = solution.surface_integrals([1, 12])
        # testQ = agros.test("Electric charge", surface["Q"], -1.291778e-9)
        
class TestRFTEHarmonicAxisymmetric(AgrosTestCase):
    def setUp(self):         
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        problem.frequency = 1e+09

        # fields
        # rf
        self.rf = problem.field("rf_te")
        self.rf.analysis_type = "harmonic"
        self.rf.number_of_refinements = 2
        self.rf.polynomial_order = 3
        self.rf.solver = "linear"
        
        # boundaries
        self.rf.add_boundary("PEC", "rf_te_electric_field", {"rf_te_electric_field_real" : 0, "rf_te_electric_field_imag" : 0})
        self.rf.add_boundary("Source", "rf_te_electric_field", {"rf_te_electric_field_real" : 1, "rf_te_electric_field_imag" : 0})
        self.rf.add_boundary("Impedance", "rf_te_impedance", {"rf_te_impedance" : 377})
        self.rf.add_boundary("PMC", "rf_te_magnetic_field", {"rf_te_magnetic_field_real" : 0, "rf_te_magnetic_field_imag" : 0})
        
        # materials
        self.rf.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 0, "rf_te_current_density_external_real" : 0, "rf_te_current_density_external_imag" : 0})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0, 1, 0, -1, boundaries = {"rf_te" : "PEC"})
        geometry.add_edge(0, -1, 1, -1, boundaries = {"rf_te" : "Impedance"})
        geometry.add_edge(1, -1, 1, -0.75, boundaries = {"rf_te" : "Impedance"})
        geometry.add_edge(1, -0.75, 1.7, 0.65, boundaries = {"rf_te" : "Impedance"})
        geometry.add_edge(1.7, 0.65, 0.9, 0.35, boundaries = {"rf_te" : "Impedance"})
        geometry.add_edge(0.9, 0.35, 0, 1, boundaries = {"rf_te" : "Impedance"})
        geometry.add_edge(0.5, -0.25, 0.25, -0.5, angle = 90, boundaries = {"rf_te" : "Source"})
        geometry.add_edge(0.25, -0.5, 0.5, -0.75, angle = 90, boundaries = {"rf_te" : "Source"})
        geometry.add_edge(0.5, -0.75, 0.75, -0.5, angle = 90, boundaries = {"rf_te" : "Source"})
        geometry.add_edge(0.75, -0.5, 0.5, -0.25, angle = 90, boundaries = {"rf_te" : "Source"})
        geometry.add_edge(0.4, 0.25, 0.3, -0.05, boundaries = {"rf_te" : "PMC"})
        geometry.add_edge(0.4, 0.25, 0.75, 0.2, boundaries = {"rf_te" : "PMC"})
        geometry.add_edge(0.75, 0.2, 0.85, -0.1, boundaries = {"rf_te" : "PMC"})
        geometry.add_edge(0.3, -0.05, 0.85, -0.1, boundaries = {"rf_te" : "PMC"})
        
        geometry.add_label(0.399371, 0.440347, materials = {"rf_te" : "Air"})
        geometry.add_label(0.484795, -0.434246, materials = {"rf_te" : "none"})
        geometry.add_label(0.57193, 0.0710058, materials = {"rf_te" : "none"})
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self): 
        # solution
        solution = self.computation.solution("rf_te")
        
        # point value        
        point1 = solution.local_values(1.729e-01, -1.289e-01)
        
        self.value_test("Electric field", point1["E"], 1.6461)
        self.value_test("Electric field - real", point1["Er"], 0.545821056506917)
        self.value_test("Electric field - imag", point1["Ei"], 1.5701)

        self.value_test("Displacement", point1["D"], 1.4575E-11)
        self.value_test("Displacement - real", point1["Dr"], 4.8326E-12)
        self.value_test("Displacement - imag", point1["Di"], 1.3902E-11)

        point2 = solution.local_values(0.05574473738670349, -0.17109256982803345)
        
        self.value_test("Magnetic field", point2["H"], 0.010094494526820486)
        self.value_test("Magnetic field r component - real", point2["Hrr"], -7.315569466E-4)
        self.value_test("Magnetic field r component - imag", point2["Hir"], -0.0036385133316168745)
        self.value_test("Magnetic field z component - real", point2["Hrz"], -0.00929280413705027)
        self.value_test("Magnetic field z component - imag", point2["Hiz"], -0.001177586316666147)
        
        self.value_test("Magnetic flux density", point2["B"], 1.2685115938864646E-8)
        self.value_test("Magnetic flux density r component - real", point2["Brr"], -9.193037214595E-10)
        self.value_test("Magnetic flux density r component - imag", point2["Bir"], -4.572290701038439E-9)
        self.value_test("Magnetic flux density z component - real", point2["Brz"], -1.1677682083282389E-8)
        self.value_test("Magnetic flux density z component - imag", point2["Biz"], -1.47980206571E-9)

        self.value_test("Poynting vector r component", point2["Nr"], -2.0790401117E-4)
        self.value_test("Poynting vector z component", point2["Nz"], 0.00424470406639853)

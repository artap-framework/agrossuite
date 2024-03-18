from agrossuite import agros

from .scenario import AgrosTestCase


class TestAdaptivityElectrostatic(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # fields
        # electrostatic
        self.electrostatic = problem.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 0
        self.electrostatic.polynomial_order = 1
        
        self.electrostatic.adaptivity_type = "hp-adaptivity"
        self.electrostatic.adaptivity_parameters['tolerance'] = 0.5
        self.electrostatic.adaptivity_parameters['strategy_hp'] = "alternate"
        self.electrostatic.solver = "linear"
        
        # boundaries
        self.electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
        self.electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        # materials
        self.electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0.2, 1, 0, 0.5, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0, 0.5, 0, 0.25, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -0.25, 0, -1, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -1, 1.5, 0.5, angle = 90, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(1.5, 0.5, 0, 2, angle = 90, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, 1, 0.2, 1, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0, 2, 0, 1, boundaries = {"electrostatic" : "Border"})
        geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Ground"})
        
        geometry.add_label(0.879551, 0.764057, area = 0.06, materials = {"electrostatic" : "Air"})
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()

    def test_values(self):
        # solution
        solution = self.computation.solution("electrostatic")
                        
        # values from Comsol
        local_values = solution.local_values(5e-2, 5e-1)
        self.value_test("Electrostatic potential", local_values["V"], 648.638)
        self.value_test("Electric field - r", local_values["Er"], 1913.123)
        self.value_test("Electric field - z", local_values["Ez"], -3244.191)
        self.value_test("Energy density", local_values["we"], 6.279E-5)        
			
class TestAdaptivityAcoustic(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 5000
        
        # fields
        # acoustic
        self.acoustic = problem.field("acoustic")
        self.acoustic.analysis_type = "harmonic"
        self.acoustic.number_of_refinements = 0
        self.acoustic.polynomial_order = 1
        self.acoustic.adaptivity_type = "hp-adaptivity"
        self.acoustic.adaptivity_parameters['steps'] = 20
        self.acoustic.adaptivity_parameters['estimator'] = "kelly"
        self.acoustic.adaptivity_parameters['strategy'] = "fixed_fraction_of_total_error"
        self.acoustic.adaptivity_parameters['tolerance'] = 0
        self.acoustic.solver = "linear"
        
        # boundaries
        self.acoustic.add_boundary("Wall", "acoustic_normal_acceleration", {"acoustic_normal_acceleration_real" : 0, "acoustic_normal_acceleration_imag" : 0})
        self.acoustic.add_boundary("Source", "acoustic_pressure", {"acoustic_pressure_real" : 1, "acoustic_pressure_imag" : 0})
        self.acoustic.add_boundary("Matched boundary", "acoustic_impedance", {"acoustic_impedance" : { "expression" : "1.25*343" }})
        
        # materials
        self.acoustic.add_material("Air", {"acoustic_density" : 1.25, "acoustic_speed" : 343})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0.3, 0, 0, 0.3, angle = 90, boundaries = {"acoustic" : "Matched boundary"})
        geometry.add_edge(0.1, 0, 0.3, 0, boundaries = {"acoustic" : "Wall"})
        geometry.add_edge(0.1, 0, 0.025, -0.175, boundaries = {"acoustic" : "Wall"})
        geometry.add_edge(0.025, -0.175, 0.025, -0.2, boundaries = {"acoustic" : "Wall"})
        geometry.add_edge(0.025, -0.2, 0, -0.2, boundaries = {"acoustic" : "Source"})
        geometry.add_edge(0, 0.3, 0, -0.2, boundaries = {"acoustic" : "Wall"})
        
        geometry.add_label(0.109723, 0.176647, materials = {"acoustic" : "Air"})
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("acoustic")
                        
        # values from Comsol
        point1 = solution.local_values(7.544e-3, -0.145)
        self.value_test("Acoustic pressure 1", point1["p"], 0.74307)
        point2 = solution.local_values(6.994e-2, 1.894e-2)
        self.value_test("Acoustic pressure 2", point2["p"], 0.28242)
         	
class TestAdaptivityElasticityBracket(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        # fields
        # elasticity
        self.elasticity = problem.field("elasticity")
        self.elasticity.analysis_type = "steadystate"
        self.elasticity.number_of_refinements = 0
        self.elasticity.polynomial_order = 1
        self.elasticity.adaptivity_type = "p-adaptivity"
        self.elasticity.adaptivity_parameters['steps'] = 2
        self.elasticity.adaptivity_parameters['tolerance'] = 0
        self.elasticity.solver = "linear"
                
        # boundaries
        self.elasticity.add_boundary("Wall", "elasticity_fixed_fixed", {"elasticity_displacement_x" : 0, "elasticity_displacement_y" : 0})
        self.elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : 0})
        self.elasticity.add_boundary("Load", "elasticity_free_free", {"elasticity_force_x" : 0, "elasticity_force_y" : -200})
                
        # materials
        self.elasticity.add_material("Steel", {"elasticity_young_modulus" : 2e11, "elasticity_poisson_ratio" : 0.33, "elasticity_volume_force_x" : 0, "elasticity_volume_force_y" : { "expression" : "-9.81*7800" }, "elasticity_alpha" : 0, "elasticity_temperature_difference" : 0, "elasticity_temperature_reference" : 0})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0.3, 0, 0, 0, boundaries = {"elasticity" : "Load"})
        geometry.add_edge(0, 0, 0, -0.3, boundaries = {"elasticity" : "Wall"})
        geometry.add_edge(0, -0.3, 0.03, -0.27, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.27, -0.03, 0.3, 0, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.03, -0.03, 0.03, -0.15, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.03, -0.03, 0.15, -0.03, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.27, -0.03, 0.03, -0.27, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.03, -0.15, 0.030625, -0.150375, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.030625, -0.150375, 0.031, -0.15, angle = 45, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.150375, -0.030625, 0.15, -0.03, angle = 90, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.15, -0.031, 0.150375, -0.030625, angle = 45, boundaries = {"elasticity" : "Free"})
        geometry.add_edge(0.15, -0.031, 0.031, -0.15, angle = 45, boundaries = {"elasticity" : "Free"})
        
        geometry.add_label(0.19805, -0.0157016, materials = {"elasticity" : "Steel"})
        geometry.add_label(0.0484721, -0.0490752, materials = {"elasticity" : "none"})      
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("elasticity")
                
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point1 = solution.local_values(2.042e-1, -3e-2)
        self.value_test("Displacement", point1["d"], 1.161e-7)
        

class TestAdaptivityMagneticProfileConductor(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        problem.frequency = 50000

        # fields
        # magnetic
        self.magnetic = problem.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        self.magnetic.adaptivity_type = "hp-adaptivity"
        self.magnetic.adaptivity_parameters['steps'] = 5
        self.magnetic.adaptivity_parameters['tolerance'] = 0
        self.magnetic.solver = "linear"
                
        # boundaries
        self.magnetic.add_boundary("Neumann", "magnetic_surface_current", {"magnetic_surface_current_real" : 0, "magnetic_surface_current_imag" : 0})
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})        
        
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 57e6, "magnetic_current_density_external_real" : 1e6/4})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0, 0.002, 0, 0.000768, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0, 0.000768, 0, 0, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0, 0, 0.000768, 0, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0.000768, 0, 0.002, 0, boundaries = {"magnetic" : "Neumann"})
        geometry.add_edge(0.002, 0, 0, 0.002, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.000768, 0, 0.000576, 0.000192, angle = 90)
        geometry.add_edge(0.000576, 0.000192, 0.000384, 0.000192)
        geometry.add_edge(0.000192, 0.000384, 0.000384, 0.000192, angle = 90)
        geometry.add_edge(0.000192, 0.000576, 0.000192, 0.000384)
        geometry.add_edge(0.000192, 0.000576, 0, 0.000768, angle = 90)
        
        geometry.add_label(0.000585418, 0.00126858, materials = {"magnetic" : "Air"})
        geometry.add_label(0.000109549, 8.6116e-05, materials = {"magnetic" : "Copper"})
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()        
        
    def test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
                        
        point = solution.local_values(6.106e-04, 2.378e-04)
        self.value_test("Magnetic potential - real", point["Ar"], 2.7584856899647945E-9)
        self.value_test("Magnetic potential - imag", point["Ai"], -1.0598108775564893E-8)        
        self.value_test("Flux density", point["B"], 1.3839318132148589E-5, 0.1)

class TestAdaptivityRF_TE(AgrosTestCase):
    # TODO: add more adaptivity types
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        problem.frequency = 1.6e+10

        # fields
        # rf_te
        self.rf_te = problem.field("rf_te")
        self.rf_te.analysis_type = "harmonic"
        self.rf_te.number_of_refinements = 1
        self.rf_te.polynomial_order = 1
        self.rf_te.adaptivity_type = "hp-adaptivity"
        self.rf_te.adaptivity_parameters['steps'] = 8
        self.rf_te.adaptivity_parameters['estimator'] = "kelly"
        self.rf_te.adaptivity_parameters['strategy'] = "fixed_fraction_of_total_error"
        self.rf_te.adaptivity_parameters['strategy_hp'] = "alternate"        
        self.rf_te.adaptivity_parameters['tolerance'] = 0
        self.rf_te.solver = "linear"
                
        # boundaries
        self.rf_te.add_boundary("Perfect electric conductor", "rf_te_electric_field", {"rf_te_electric_field_real" : 0, "rf_te_electric_field_imag" : 0})
        self.rf_te.add_boundary("Matched boundary", "rf_te_magnetic_field", {"rf_te_magnetic_field_real" : 0, "rf_te_magnetic_field_imag" : 0})
        self.rf_te.add_boundary("Source", "rf_te_electric_field", {"rf_te_electric_field_real" : { "expression" : "cos(y/0.01143*pi/2)" }, "rf_te_electric_field_imag" : 0})
                
        # materials
        self.rf_te.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 0, "rf_te_current_density_external_real" : 0, "rf_te_current_density_external_imag" : 0})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0.17, -0.01143, 0.17, 0.01143, boundaries = {"rf_te" : "Matched boundary"})
        geometry.add_edge(0, 0.01143, 0, -0.01143, boundaries = {"rf_te" : "Source"})
        geometry.add_edge(0.076, 0.01143, 0.076, 0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, 0.0045, 0.081, 0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, 0.0045, 0.081, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, -0.0045, 0.081, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, -0.0045, 0.076, -0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, -0.0045, 0.076, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0, -0.01143, 0.076, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.081, -0.01143, 0.17, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.17, 0.01143, 0.081, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        geometry.add_edge(0.076, 0.01143, 0, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
        
        geometry.add_label(0.0367388, 0.0025708, materials = {"rf_te" : "Air"})

        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def disabled_test_values(self):
        # solution
        solution = self.computation.solution("rf_te")
                
        point1 = solution.local_values(5.801e-02, 4.192e-03)
        self.value_test("Electric field", point1["E"], 0.1769)
        self.value_test("Flux density", point1["B"], 2.604E-9)
           
class TestAdaptivityHLenses(AgrosTestCase):
    # test for h-adaptivity
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"

        # fields
        # magnetic
        self.magnetic = problem.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        self.magnetic.adaptivity_type = "h-adaptivity"
        self.magnetic.adaptivity_parameters['steps'] = 5
        self.magnetic.adaptivity_parameters['tolerance'] = 0
        self.magnetic.solver = "linear"
                
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})        
        
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0})
        self.magnetic.add_material("Coil 1", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 8e6})
        self.magnetic.add_material("Coil 2", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 1e7})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0, 0.04, 0.05, 0.04, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.05, 0.04, 0.05, -0.08, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.05, -0.08, 0, -0.08, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.08, 0, 0.04, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.0075, 0.00125, 0.01, 0.00125)
        geometry.add_edge(0.01, 0.00125, 0.01, -0.00125)
        geometry.add_edge(0.01, -0.00125, 0.0075, -0.00125)
        geometry.add_edge(0.0075, 0.00125, 0.0075, -0.00125)
        geometry.add_edge(0.0075, 0.00625, 0.01, 0.00625)
        geometry.add_edge(0.01, 0.00375, 0.01, 0.00625)
        geometry.add_edge(0.0075, 0.00625, 0.0075, 0.00375)
        geometry.add_edge(0.01, 0.00375, 0.0075, 0.00375)
        geometry.add_edge(0.0075, -0.01475, 0.01, -0.01475)
        geometry.add_edge(0.01, -0.01475, 0.01, -0.02125)
        geometry.add_edge(0.01, -0.02125, 0.0075, -0.02125)
        geometry.add_edge(0.0075, -0.01475, 0.0075, -0.02125)
        
        geometry.add_label(0.00870469, 0.000204637, materials = {"magnetic" : "Coil 1"})
        geometry.add_label(0.027331, 0.0261643, materials = {"magnetic" : "Air"})
        geometry.add_label(0.00883434, 0.00543334, materials = {"magnetic" : "Coil 1"})
        geometry.add_label(0.00902444, -0.0168831, materials = {"magnetic" : "Coil 2"})
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
                
        # values from Comsol        
        point1 = solution.local_values(5.902e-03, -1.241e-02)
        self.value_test("Magnetic potential", point1["Ar"], 2.2250E-5)
        self.value_test("Flux density - r", point1["Brr"], 0.0032758)
        self.value_test("Flux density - z", point1["Brz"], 0.0068235)
        self.value_test("Energy density", point1["wm"], 22.796)

class TestAdaptivityPAndHCoupled(AgrosTestCase):
    # test for h-adaptivity
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 50

        # fields
        # magnetic
        self.magnetic = problem.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        #self.magnetic.adaptivity_type = "h-adaptivity"
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.adaptivity_parameters['steps'] = 3
        self.magnetic.adaptivity_parameters['tolerance'] = 0
        self.magnetic.solver = "linear"
                
        # boundaries
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})
                
        # materials
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 6e5, "magnetic_current_density_external_imag" : 0})
        self.magnetic.add_material("Insulation", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
        self.magnetic.add_material("Steel", {"magnetic_permeability" : 100, "magnetic_conductivity" : 3e5, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, "magnetic_current_density_external_real" : 0, "magnetic_current_density_external_imag" : 0})
                                
        # heat
        self.heat = problem.field("heat")
        self.heat.analysis_type = "steadystate"
        self.heat.number_of_refinements = 0
        self.heat.polynomial_order = 1
        self.heat.adaptivity_type = "p-adaptivity"
        #self.heat.adaptivity_type = "disabled"
        self.heat.adaptivity_parameters['steps'] = 3
        self.heat.adaptivity_parameters['tolerance'] = 0
        self.heat.solver = "linear"        
        
        # boundaries
        self.heat.add_boundary("Symmetry", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0, "heat_radiation_emissivity" : 0, "heat_radiation_ambient_temperature" : 0})
        self.heat.add_boundary("Convection", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 10, "heat_convection_external_temperature" : 323, "heat_radiation_emissivity" : 0, "heat_radiation_ambient_temperature" : 0})
        self.heat.add_boundary("Radiation", "heat_heat_flux", {"heat_heat_flux" : 0, "heat_convection_heat_transfer_coefficient" : 0, "heat_convection_external_temperature" : 0, "heat_radiation_emissivity" : 0.6, "heat_radiation_ambient_temperature" : 293})       
        
        # materials
        self.heat.add_material("Steel", {"heat_conductivity" : 70, "heat_volume_heat" : 0, "heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_density" : 0, "heat_specific_heat" : 0})
        self.heat.add_material("Insulation", {"heat_conductivity" : 6, "heat_volume_heat" : 0, "heat_velocity_x" : 0, "heat_velocity_y" : 0, "heat_velocity_angular" : 0, "heat_density" : 0, "heat_specific_heat" : 0})
                
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0.3, 0.6, 0, 0.6, boundaries = {"heat" : "Radiation"})
        geometry.add_edge(0, 1.2, 0.9, 1.2, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.9, 1.2, 0.9, -0.5, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.9, -0.5, 0, -0.5, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.5, 0, 0, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0, 0.32, 0, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.3, 0.6, 0.3, 0.1)
        geometry.add_edge(0, 0.1, 0.3, 0.1)
        geometry.add_edge(0, 0.6, 0, 0.1, boundaries = {"heat" : "Symmetry", "magnetic" : "A = 0"})
        geometry.add_edge(0, 0.1, 0, 0, boundaries = {"heat" : "Symmetry", "magnetic" : "A = 0"})
        geometry.add_edge(0.33, 0.7, 0.4, 0.7)
        geometry.add_edge(0.4, 0.7, 0.4, 0.046)
        geometry.add_edge(0.4, 0.046, 0.33, 0.046)
        geometry.add_edge(0.33, 0.046, 0.33, 0.7)
        geometry.add_edge(0.3, 0.6, 0.32, 0.6, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0.32, 0, 0.32, 0.6, boundaries = {"heat" : "Convection"})
        geometry.add_edge(0, 1.2, 0, 0.6, boundaries = {"magnetic" : "A = 0"})
        
        geometry.add_label(0.627519, 0.954318, materials = {"heat" : "none", "magnetic" : "Air"})
        geometry.add_label(0.087409, 0.293345, materials = {"heat" : "Steel", "magnetic" : "Steel"})
        geometry.add_label(0.132733, 0.0478408, materials = {"heat" : "Insulation", "magnetic" : "Insulation"})
        geometry.add_label(0.378237, 0.221582, materials = {"heat" : "none", "magnetic" : "Copper"})

        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
    def disabled_test_values(self):
        # solution
        solution_heat = self.computation.solution("heat")
        solution_magnetic = self.computation.solution("magnetic")
                
        # exact values in this test are taken from Agros -> not a proper test
        # only to see if adaptivity works, should be replaced with comsol values
        point_heat = solution_heat.local_values(3.048e-01, 8.919e-02)
        self.value_test("Temperature", point_heat["T"], 1.277e+03, 0.006)
        point_mag = solution_magnetic.local_values(2.920e-01, 1.333e-01)
        self.value_test("Flux density", point_mag["Br"], 5.893e-01, 0.025)

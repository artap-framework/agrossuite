from agrossuite import agros

from .scenario import AgrosTestCase


class TestMagneticHarmonicPlanar(AgrosTestCase):
    def setUp(self):                                                                                                                         
        # model
        self.problem = agros.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"        
        self.problem.frequency = 50

        # fields
        self.magnetic = self.problem.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        self.magnetic.solver = "linear"
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        self.magnetic.add_material("Cond 2", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 3e7, "magnetic_conductivity" : 5.7e7}) 
        self.magnetic.add_material("Magnet", {"magnetic_permeability" : 1.1, "magnetic_remanence" : 0.1, "magnetic_remanence_angle" : 20})    
        
        # geometry
        self.geometry = self.problem.geometry()
        
        # edges
        self.geometry.add_edge(-0.075, 0.06, 0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.075, 0.06, 0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(0.075, -0.06, -0.075, -0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.075, -0.06, -0.075, 0.06, boundaries = {"magnetic" : "A = 0"})
        self.geometry.add_edge(-0.015, -0.01, -0.015, 0.01)
        self.geometry.add_edge(-0.015, 0.01, -0.005, 0.01)
        self.geometry.add_edge(-0.015, -0.01, -0.005, -0.01)
        self.geometry.add_edge(-0.005, -0.01, -0.005, 0.01)
        self.geometry.add_edge(0.005, 0.02, 0.005, 0)
        self.geometry.add_edge(0.005, 0, 0.015, 0)
        self.geometry.add_edge(0.015, 0, 0.015, 0.02)
        self.geometry.add_edge(0.015, 0.02, 0.005, 0.02)
        self.geometry.add_edge(0.01, -0.01, 0.03, -0.01)
        self.geometry.add_edge(0.03, -0.03, 0.01, -0.03)
        self.geometry.add_edge(0.01, -0.01, 0.01, -0.03)
        self.geometry.add_edge(0.03, -0.01, 0.03, -0.03)
        
        # labels
        self.geometry.add_label(0.035349, 0.036683, materials = {"magnetic" : "Air"}, area=0)
        self.geometry.add_label(-0.0111161, -0.00311249, materials = {"magnetic" : "Cond 2"}, area=1e-05)
        self.geometry.add_label(0.016868, -0.0186625, materials = {"magnetic" : "Magnet"}, area=0)
                    
                     
    # in this test, both Cond1 and Cond2 have conductivity 5.7e7
    # results from comsol test test_magnetic_harmonic_planar
    def general_nonzero_cond_test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
                                                                                                                                
        # point value
        point = solution.local_values(0.012448, 0.016473)
        self.value_test("Magnetic potential", point["A"], 0.001087)
        self.value_test("Magnetic potential - real", point["Ar"], 3.391642e-4)
        self.value_test("Magnetic potential - imag", point["Ai"], -0.001033)
        self.value_test("Flux density", point["B"], 0.038197)
        self.value_test("Flux density - x - real", point["Brx"], -0.004274)
        self.value_test("Flux density - x - imag", point["Bix"], 0.02868)
        self.value_test("Flux density - y - real", point["Bry"], 0.003269)
        self.value_test("Flux density - y - imag", point["Biy"], -0.024707)
        self.value_test("Magnetic field", point["H"], 30351.803874)
        self.value_test("Magnetic field - x - real", point["Hrx"], -3400.886351)
        self.value_test("Magnetic field - x - imag", point["Hix"], 22823.176772)
        self.value_test("Magnetic field - y - real", point["Hry"], 2613.37651)
        self.value_test("Magnetic field - y - imag", point["Hiy"], -19543.255504)
        self.value_test("Energy density", point["wm"], 289.413568)
        self.value_test("Losses density ", point["pj"], 3.435114e5)
        self.value_test("Current density - induced transform - real", point["Jitr"], -1.849337e7)
        self.value_test("Current density - induced transform - imag", point["Jiti"], -6.073744e6)
        self.value_test("Current density - total - real", point["Jr"], 1.50663e6)
        self.value_test("Current density - total - imag", point["Ji"], -6.073744e6)
        self.value_test("Lorentz force - x", point["Flx"], -77163)
        self.value_test("Lorentz force - y", point["Fly"], -89097)
        
        # volume integral
        volume = solution.volume_integrals([3])
        self.value_test("Current - external - real", volume["Ier"], 4000.0)
        self.value_test("Current - external - imag", volume["Iei"], 0.0)
        self.value_test("Current - induced transform - real", volume["Iitr"], -4104.701323)
        self.value_test("Current - induced transform - imag", volume["Iiti"], -1381.947299)
        self.value_test("Current - real", volume["Ir"], -104.701323)
        self.value_test("Current - imag", volume["Ii"], -1381.947299)
        self.value_test("Energy", volume["Wm"], 0.042927)
        self.value_test("Losses", volume["Pj"], 90.542962)
        self.value_test("Lorentz force - x", volume["Flx"], -11.228229)
        self.value_test("Lorentz force - y", volume["Fly"], -4.995809)   

    # in this test, Cond1 has conductivity 0
    # results from comsol test test_magnetic_harmonic_planar_zero_cond
    def general_zero_cond_test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
                                                                                                                                
        point = solution.local_values(-0.0078561, 0.00453680)
        self.value_test("Current density - total - real", point["Jr"], -2.261e6)
        self.value_test("Current density - total - imag", point["Ji"], -2.02266e7)
        self.value_test("Current density - external - real", point["Jer"], 3e7)
        self.value_test("Current density - external - imag", point["Jei"], 0)
        
        point2 = solution.local_values(0.0073347240686417, 0.0036161467432976)
        self.value_test("Current density - total - real", point2["Jr"], 2e7)
        self.value_test("Current density - total - imag", point2["Ji"], 0)
        self.value_test("Current density - external - real", point2["Jer"], 2e7)
        self.value_test("Current density - external - imag", point2["Jei"], 0)
        
        volume = solution.volume_integrals([3])
        self.value_test("Current - real", volume["Ir"], 4000)
        self.value_test("Current - imag", volume["Ii"], 0)
        self.value_test("Current - external - real", volume["Ier"], 4000)
        self.value_test("Current - external - imag", volume["Iei"], 0)

        volume2 = solution.volume_integrals([1])
        self.value_test("Current - real", volume2["Ir"], -55.75)
        self.value_test("Current - imag", volume2["Ii"], -3601)
        self.value_test("Current - external - real", volume2["Ier"], 6000)
        self.value_test("Current - external - imag", volume2["Iei"], 0)

        
    # nonzero conductivity
    def test_values_nonzero_cond(self):
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e7, "magnetic_conductivity" : 5.7e7}) 
        self.geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
        self.computation = self.problem.computation()
        self.computation.solve()  
        self.general_nonzero_cond_test_values()
                          
    # zero conductivity and external current density given                                                        
    def test_values_zero_cond_J_ext_given(self):
        self.magnetic.add_material("Cond 1", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 2e7, "magnetic_conductivity" : 0}) 
        self.geometry.add_label(0.00778124, 0.00444642, materials = {"magnetic" : "Cond 1"}, area=1e-05)
        self.computation = self.problem.computation()
        self.computation.solve()  
        self.general_zero_cond_test_values()

class TestMagneticHarmonicAxisymmetric(AgrosTestCase):
    def setUp(self):                                                                                                                         
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"        
        problem.frequency = 100
        
        # fields
        self.magnetic = problem.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        self.magnetic.solver = "linear"
        
        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Coil", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6}) 
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 50, "magnetic_conductivity" : 5e3}) 
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1}) 
        
        # geometry
        geometry = problem.geometry()
        
        # edges
        geometry.add_edge(0, -0.17, 0.15, -0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.17, 0.15, 0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, 0.17, 0, 0.17, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.035, -0.03, 0.055, -0.03)
        geometry.add_edge(0.055, -0.03, 0.055, 0.11)
        geometry.add_edge(0.055, 0.11, 0.035, 0.11)
        geometry.add_edge(0.035, 0.11, 0.035, -0.03)
        geometry.add_edge(0, -0.05, 0.03, -0.05)
        geometry.add_edge(0.03, -0.05, 0.03, 0.05)
        geometry.add_edge(0.03, 0.05, 0, 0.05)
        geometry.add_edge(0, 0.05, 0, -0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, 0.17, 0, 0.05, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.05, 0, -0.17, boundaries = {"magnetic" : "A = 0"})
        
        # labels
        geometry.add_label(0.109339, 0.112786, materials = {"magnetic" : "Air"}, area=0)
        geometry.add_label(0.0442134, 0.0328588, materials = {"magnetic" : "Coil"}, area=0)
        geometry.add_label(0.0116506, -0.00740064, materials = {"magnetic" : "Iron"}, area=0)
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
                     
    def test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
        
        # point value
        point = solution.local_values(0.027159, 0.039398)
        self.value_test("Magnetic potential", point["A"], 0.001087)
        self.value_test("Magnetic potential - real", point["Ar"], 0.001107)
        self.value_test("Magnetic potential - imag", point["Ai"], -5.24264e-6)
        self.value_test("Flux density", point["B"], 0.099325)
        self.value_test("Flux density - r - real", point["Brr"], 0.027587)
        self.value_test("Flux density - r - imag", point["Bir"], -2.430976e-4)
        self.value_test("Flux density - z - real", point["Brz"],  0.095414)
        self.value_test("Flux density - z - imag", point["Biz"], 7.424088e-4)
        self.value_test("Magnetic field", point["H"], 1580.808517)
        self.value_test("Magnetic field - r - real", point["Hrr"], 439.052884)
        self.value_test("Magnetic field - r - imag", point["Hir"], -3.869019)
        self.value_test("Magnetic field - z - real", point["Hrz"], 1518.562988)
        self.value_test("Magnetic field - z - imag", point["Hiz"], 11.815803)
        self.value_test("Energy density", point["wm"], 39.253502)
        self.value_test("Losses density ", point["pj"], 1210.138583)
        self.value_test("Current density - induced transform - real", point["Jitr"], -16.47024)
        self.value_test("Current density - induced transform - imag", point["Jiti"], -3478.665629)
        self.value_test("Current density - total - real", point["Jr"], -16.47024)
        self.value_test("Current density - total - imag", point["Ji"], -3478.665629)
        self.value_test("Lorentz force - r", point["Flr"], 0.505549)
        self.value_test("Lorentz force - z", point["Flz"], 0.650006)
        
        # volume integral
        volume = solution.volume_integrals([2])
        self.value_test("Current - induced transform - real", volume["Iitr"], -0.067164)
        self.value_test("Current - induced transform - imag", volume["Iiti"], -5.723787)
        self.value_test("Current - external - real", volume["Ier"], 0.0)
        self.value_test("Current - external - imag", volume["Iei"], 0.0)
        self.value_test("Current - real", volume["Ir"], -0.067164)
        self.value_test("Current - imag", volume["Ii"], -5.723787)
        self.value_test("Energy", volume["Wm"], 0.009187)
        self.value_test("Losses", volume["Pj"], 0.228758)
        self.value_test("Lorentz force - r", volume["Flx"], -4.018686e-4)
        self.value_test("Lorentz force - z", volume["Fly"], -1.233904e-5)    
             
class TestMagneticHarmonicNonlinPlanar(AgrosTestCase):
    def setUp(self):  
        # model
        problem = agros.problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"
        
        problem.frequency = 50
        
        # fields
        self.magnetic = problem.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['residual'] = 0.01
        self.magnetic.solver_parameters['damping_factor'] = 0.7
        self.magnetic.solver_parameters['damping'] = 'automatic'
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_ratio'] = 0.9
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 20        

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_imag" : 0, "magnetic_potential_real" : 0})
        
        self.magnetic.add_material("Zelezo", {"magnetic_conductivity" : 5e6, "magnetic_current_density_external_imag" : 0, "magnetic_current_density_external_real" : 3e6, "magnetic_permeability" : { "value" : 5000, 
            "x" : [0,0.227065,0.45413,0.681195,0.90826,1.13533,1.36239,1.58935,1.81236,2.01004,2.13316,2.19999,2.25479,2.29993,2.34251,2.37876,2.41501,2.45126,2.4875,2.52375,2.56,3,5,10,20], 
            "y" : [13001,13001,13001,12786,12168,10967,7494,1409,315,90,41,26,19,15,12,11,9,8,8,7,6,4,3,3,2] 
            }, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_angular" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0})
        
        # geometry
        geometry = problem.geometry()
        
        # edges
        geometry.add_edge(0, 0.0035, -0.0035, 0, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        geometry.add_edge(-0.0035, 0, 0, -0.0035, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        geometry.add_edge(0, -0.0035, 0.0035, 0, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        geometry.add_edge(0.0035, 0, 0, 0.0035, boundaries = {"magnetic" : "A = 0"}, angle = 90)
        
        # labels
        geometry.add_label(0, 0, materials = {"magnetic" : "Zelezo"}, area = 5e-07)
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()

    def disabled_test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
        
        # point value
        pass
        point = solution.local_values(-2e-3, 4e-4)
        self.value_test("Flux density", point["B"], 1.478466609831)
        self.value_test("Permeability", point["mur"], 4381.88257, 5)
        self.value_test("Current density - total - real", point["Jr"], -141396.38032153525)
        self.value_test("Current density - total - imag", point["Ji"], -931848.5966661869)
        
        # volume integral
        volume = solution.volume_integrals()
        self.value_test("Energy", volume["Wm"], 0.012166845506925431)
        self.value_test("Current density - induced - real", volume["Iivr"], -76.31308924012728)
        self.value_test("Current density - induced - imag", volume["Iiti"], -25.458979006398277)
        self.value_test("Energy", volume["Wm"], 0.012166845506925431)


class TestMagneticHarmonicNonlinAxisymmetric(AgrosTestCase):
    def setUp(self):  
        # problem
        problem = agros.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        problem.frequency = 50
        
        # magnetic
        self.magnetic = problem.field("magnetic")
        self.magnetic.analysis_type = "harmonic"
        self.magnetic.number_of_refinements = 1
        self.magnetic.polynomial_order = 3
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "newton"
        self.magnetic.solver_parameters['residual'] = 0.002
        self.magnetic.solver_parameters['damping'] = "automatic"
        self.magnetic.solver_parameters['damping_factor'] = 0.8
        self.magnetic.solver_parameters['jacobian_reuse'] = True
        self.magnetic.solver_parameters['jacobian_reuse_ratio'] = 0.8
        self.magnetic.solver_parameters['damping_factor_decrease_ratio'] = 1.2
        self.magnetic.solver_parameters['jacobian_reuse_steps'] = 20
        self.magnetic.solver_parameters['damping_factor_increase_steps'] = 1
                
        # boundaries
        self.magnetic.add_boundary("A=0", "magnetic_potential", {"magnetic_potential_real" : 0, "magnetic_potential_imag" : 0})
                
        # materials
        self.magnetic.add_material("new material", {"magnetic_permeability" : { "value" : 9300, 
            "x" : [0,0.227065,0.45413,0.681195,0.90826,1.13533,1.36239,1.58935,1.81236,2.01004,2.13316,2.19999,2.25479,2.29993,2.34251,2.37876,2.41501,2.45126,2.4875,2.52375,2.56,3,5,10,20], 
            "y" : [13001,13001,13001,12786,12168,10967,7494,1409,315,90,41,26,19,15,12,11,9,8,8,7,6,4,3,3,2], 
            "interpolation" : "piecewise_linear", "extrapolation" : "constant", "derivative_at_endpoints" : "first" }, 
            "magnetic_conductivity" : 5e6, "magnetic_remanence" : 0, "magnetic_remanence_angle" : 0, "magnetic_velocity_x" : 0, "magnetic_velocity_y" : 0, "magnetic_velocity_angular" : 0, 
            "magnetic_current_density_external_real" : 1e6, "magnetic_current_density_external_imag" : 0})
        
        # geometry
        geometry = problem.geometry()
        geometry.add_edge(0.004, 0, 0.007, -0.003, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.007, -0.003, 0.01, 0, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.01, 0, 0.007, 0.003, angle = 90, boundaries = {"magnetic" : "A=0"})
        geometry.add_edge(0.007, 0.003, 0.004, 0, angle = 90, boundaries = {"magnetic" : "A=0"})
        
        geometry.add_label(0.0069576, -0.000136791, materials = {"magnetic" : "new material"})
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
                                              
    def disabled_test_values(self):
        # solution
        solution = self.computation.solution("magnetic")
        
        # point value
        point = solution.local_values(0.0051, -0.0003)
        self.value_test("Flux density", point["B"], 0.10239, 0.1)
        self.value_test("Permeability", point["mur"], 13001)
        self.value_test("Current density - total - real", point["Jr"], -49436.7, 0.04)
        self.value_test("Current density - total - imag", point["Ji"], 4043.9, 0.04)

        point2 = self.magnetic.local_values(0.0043, -2e-4)
        self.value_test("Flux density", point2["B"], 1.3649)
        self.value_test("Permeability", point2["mur"], 7425.7)
        self.value_test("Current density - total - real", point2["Jr"], 4.846e5)
        self.value_test("Current density - total - imag", point2["Ji"], -3.569e5)
        
        # volume integral
        volume = solution.volume_integrals()        
#    current density 1.5e6 (calculations takes 3 minutes)
        #self.value_test("Energy", volume["Wm"], 0.012166845506925431)
#        self.value_test("Current density - induced - real", volume["Iitr"], -32.95049)
#        self.value_test("Current density - induced - imag", volume["Iiti"], -6.50704)
        self.value_test("Current density - induced - real", volume["Iitr"], -23.807)
        self.value_test("Current density - induced - imag", volume["Iiti"], -3.323)
                     

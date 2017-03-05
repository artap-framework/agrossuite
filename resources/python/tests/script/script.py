import agros
import pythonlab

from tests.scenario import AgrosTestCase
from tests.scenario import AgrosTestResult

def compare(first, second):
    for solution, file in zip(first, second):
        solution_values = list(solution.values())
        file_values = list(file.values())

        for i in range(len(solution_values)):           
            if (round(solution_values[i], 5) != round(file_values[i], 5)):
                return False

    return True

def save_solution_test(problem, field):
    values_from_solution = [field.local_values(0.05, 0),
                            field.surface_integrals([0, 1, 2]),
                            field.volume_integrals()]

    from os import path
    filename = '{0}/temp.a2d'.format(path.dirname(pythonlab.tempname()))
    agros.save_file(filename, True)
    agros.open_file(filename, True)

    field = agros.field('magnetic')
    values_from_file = [field.local_values(0.05, 0),
                        field.surface_integrals([0, 1, 2]),
                        field.volume_integrals()]
                        
    #print(values_from_solution)
    #print(values_from_file)

    return compare(values_from_solution, values_from_file)

class TestSaveAdaptiveSolutionSteady(AgrosTestCase):
    def setUp(self):
        self.problem = agros.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"

        self.magnetic = self.problem.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "umfpack"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        self.magnetic.adaptivity_type = "hp-adaptivity"
        self.magnetic.adaptivity_parameters['steps'] = 4
        self.magnetic.adaptivity_parameters['tolerance'] = 1
        self.magnetic.solver = "linear"

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6})
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 100, "magnetic_conductivity" : 10e6})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

        geometry = self.problem.geometry()
        geometry.add_edge(0, -0.25, 0.1, -0.25)
        geometry.add_edge(0.1, -0.25, 0.1, 0.25)
        geometry.add_edge(0.1, 0.25, 0, 0.25)
        geometry.add_edge(0, 0.25, 0, -0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.15, 0.25, -0.15)
        geometry.add_edge(0.25, -0.15, 0.25, 0.15)
        geometry.add_edge(0.25, 0.15, 0.15, 0.15)
        geometry.add_edge(0.15, 0.15, 0.15, -0.15)
        geometry.add_edge(4.59243e-17, -0.75, 0.75, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.75, 0, 4.59243e-17, 0.75, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(4.59243e-17, 0.75, 0, 0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.25, 4.59243e-17, -0.75, boundaries = {"magnetic" : "A = 0"})

        geometry.add_label(0.5, 0, materials = {"magnetic" : "Air"})
        geometry.add_label(0.2, 0, materials = {"magnetic" : "Copper"})
        geometry.add_label(0.05, 0, materials = {"magnetic" : "Iron"})

        computation = self.problem.computation()
        computation.solve()

    def test_steady_state(self):        
        self.assertTrue(save_solution_test(self.problem, self.magnetic))
    
class TestSaveAdaptiveSolutionTransient(AgrosTestCase):
    def setUp(self):
        self.problem = agros.problem(clear = True)
        self.problem.coordinate_type = "axisymmetric"
        self.problem.mesh_type = "triangle"
        self.problem.time_step_method = "fixed"
        self.problem.time_total = 3
        self.problem.time_steps = 1

        self.magnetic = self.problem.field("magnetic")
        self.magnetic.analysis_type = "transient"
        self.magnetic.matrix_solver = "umfpack"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 1
        #self.magnetic.adaptivity_type = "hp-adaptivity"
        #self.magnetic.adaptivity_parameters['steps'] = 4
        #self.magnetic.adaptivity_parameters['tolerance'] = 1

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        self.magnetic.add_material("Copper", {"magnetic_permeability" : 1, "magnetic_current_density_external_real" : 1e6})
        self.magnetic.add_material("Iron", {"magnetic_permeability" : 100, "magnetic_conductivity" : 10e6})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

        geometry = self.problem.geometry()
        geometry.add_edge(0, -0.25, 0.1, -0.25)
        geometry.add_edge(0.1, -0.25, 0.1, 0.25)
        geometry.add_edge(0.1, 0.25, 0, 0.25)
        geometry.add_edge(0, 0.25, 0, -0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.15, -0.15, 0.25, -0.15)
        geometry.add_edge(0.25, -0.15, 0.25, 0.15)
        geometry.add_edge(0.25, 0.15, 0.15, 0.15)
        geometry.add_edge(0.15, 0.15, 0.15, -0.15)
        geometry.add_edge(4.59243e-17, -0.75, 0.75, 0, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0.75, 0, 4.59243e-17, 0.75, angle = 90, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(4.59243e-17, 0.75, 0, 0.25, boundaries = {"magnetic" : "A = 0"})
        geometry.add_edge(0, -0.25, 4.59243e-17, -0.75, boundaries = {"magnetic" : "A = 0"})

        geometry.add_label(0.5, 0, materials = {"magnetic" : "Air"})
        geometry.add_label(0.2, 0, materials = {"magnetic" : "Copper"})
        geometry.add_label(0.05, 0, materials = {"magnetic" : "Iron"})    

        computation = self.problem.computation()
        computation.solve()                   

    def test_transient(self):
        self.assertTrue(save_solution_test(self.problem, self.magnetic))
    
if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = AgrosTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestSaveAdaptiveSolutionSteady))
    #suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestSaveAdaptiveSolutionTransient))
    suite.run(result)
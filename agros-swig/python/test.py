#!/usr/bin/python3

import agros
import unittest as ut

class AgrosTestCase(ut.TestCase):
    def __init__(self, methodName='runTest'):
        ut.TestCase.__init__(self, methodName)

    def value_test(self, text, value, normal, error = 0.03):
        if (abs(value) < 1e-50):
            if(abs(normal) < 1e-30):
                self.assertTrue(True)
            else:
                str = "{0}: Agros = {1}, correct = {2}".format(text, value, normal)
                self.assertTrue(False, str)
                
            return
            
        test = abs((value - normal)/value) < error
        str = "{0}: Agros = {1}, correct = {2}, error = {3:.4f} %".format(text, value, normal, abs(value - normal)/value*100)
        self.assertTrue(test, str)
        
    # def interval_test(self, text, value, min, max):
    #     test = abs((value - normal)/value) < error
    #     str = "{0}: Agros = {1}, correct = {2}, error = {3:.4f} %".format(text, value, normal, abs(value - normal)/value*100)
    #     self.assertTrue(test, str)
        
    def lower_then_test(self, text, value, bound):
        test = (value < bound)
        str = "{0}: Agros = {1}, correct = {2}".format(text, value, bound)
        self.assertTrue(test, str)
        
class AgrosTestResult(ut.TestResult):
    def __init__(self):
        ut.TestResult.__init__(self)
        self.output = []

    def startTest(self, test):
        from time import time
        
        ut.TestResult.startTest(self, test)
        self.time = time()
        # print("{0}".format(test.id().ljust(60, "."))),

    def addSuccess(self, test):
        from time import time
        
        ut.TestResult.addSuccess(self, test)
        self.time -= time()

        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, -self.time * 1000, "OK", ""])
        
        print(("{0}".format(id.ljust(60, "."))), end=' ')
        print(("{0:08.2f}".format(-self.time * 1000).rjust(15, " ") + " ms " +
              "{0}".format("OK".rjust(10, "."))))

    def addError(self, test, err):
        ut.TestResult.addError(self, test, err)
        id = test.id().split(".")[-2] + "." + test.id().split(".")[-1]
        
        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, 0, "ERROR", err[1]])
        
        print(("{0}".format(id.ljust(60, "."))), end=' ')
        print(("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("ERROR".rjust(10, "."))))        

        import traceback
        print(traceback.print_tb(err[2]))
        print(err[1])

    def addFailure(self, test, err):
        ut.TestResult.addFailure(self, test, err)
        id = test.id().split(".")[-2] + "." + test.id().split(".")[-1]

        modu = ".".join(test.id().split(".")[0:-2])
        tst = test.id().split(".")[-1]
        cls = test.id().split(".")[-2]
        id = cls + "." + tst
        
        self.output.append([modu, cls, tst, 0, "FAILURE", str(err[1])])

        print(("{0}".format(id.ljust(60, "."))), end=' ')
        print(("{0:08.2f}".format(0).rjust(15, " ") + " ms " +
              "{0}".format("FAILURE".rjust(10, "."))))        
        print((err[1]))      
        
    def report(self):
        return self.output


class TestElectrostaticPlanar(AgrosTestCase):
    def setUp(self):  
        # model
        problem = agros.Problem(clear = True)
        problem.coordinate_type = "planar"
        problem.mesh_type = "triangle"

        # fields
        self.electrostatic = problem.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.number_of_refinements = 1
        self.electrostatic.polynomial_order = 2
        self.electrostatic.solver = "linear"
        """
        self.electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.electrostatic.add_boundary("U = 0 V", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("U = 1000 V", "electrostatic_potential", {"electrostatic_potential" : 1000})
        
        self.electrostatic.add_material("Air", {"electrostatic_charge_density" : 0, "electrostatic_permittivity" : 1})
        self.electrostatic.add_material("Source", {"electrostatic_charge_density" : 4e-10, "electrostatic_permittivity" : 10})
        self.electrostatic.add_material("Dieletric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 0})
        self.electrostatic.add_material("Dieletric 2", {"electrostatic_permittivity" : 10, "electrostatic_charge_density" : 0})
        """
        # geometry
        geometry = problem.geometry()

        # edges
        """
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
        """        

        # labels
        """
        geometry.add_label(2.78257, 1.37346, materials = {"electrostatic" : "none"})
        geometry.add_label(10.3839, 15.7187, area = 0.2, materials = {"electrostatic" : "Source"})
        geometry.add_label(3.37832, 15.8626, materials = {"electrostatic" : "Air"})
        geometry.add_label(12.3992, 0.556005, materials = {"electrostatic" : "Dieletric 1"})
        geometry.add_label(10.7019, 5.86396, materials = {"electrostatic" : "Dieletric 2"})
        """
        
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

if __name__ == '__main__':        
    import unittest as ut

    suite = ut.TestSuite()
    result = AgrosTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestElectrostaticPlanar))
    #suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestElectrostaticAxisymmetric))
    suite.run(result)
    

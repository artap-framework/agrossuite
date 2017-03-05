import agros as a2d
from tests.scenario import AgrosTestCase
from tests.scenario import AgrosTestResult

from math import sin, cos

class BenchmarkGeometryTransformation(AgrosTestCase):
    def setUp(self):
        self.problem = a2d.problem(clear = True)
        self.geometry = self.problem.geometry()

        for i in range(100):
            self.geometry.add_node(i*sin(i), i*cos(i))

        self.geometry.select_nodes()

    def test_move(self):
        for i in range(25):
            self.geometry.move_selection(1, 0)

    def test_rotate(self):
        for i in range(25):
            self.geometry.rotate_selection(0, 0, 1)

    def test_scale(self):
        for i in range(25):
            self.geometry.scale_selection(0, 0, 0.5)
            
if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = AgrosTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(BenchmarkGeometryTransformation))
    suite.run(result)
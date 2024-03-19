import unittest


class AgrosTestCase(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        unittest.TestCase.__init__(self, methodName)

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

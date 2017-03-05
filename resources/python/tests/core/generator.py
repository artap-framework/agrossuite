import agros
import pythonlab
import os

from tests.scenario import AgrosTestCase
from tests.scenario import AgrosTestResult

def create_tests(case, dir):
    for (path, dirs, files) in os.walk(dir):
        for file in files:
            name, extension = os.path.splitext(file)

            method = 'test_{0}_{1}'.format(os.path.split(path)[-1].replace(" ", "_"), name.replace(" ", "_")).lower()
            example = '{0}/{1}'.format(path, file)

            if(extension == '.a2d'):
                setattr(case, method, get_test(example))

def get_test(example):
    def test(self):
        agros.open_file(example)
        script = agros.get_script_from_model()
        exec(script in globals(), locals())
        agros.problem(clear=False).computation().solve()

    return test

class TestGenerator(AgrosTestCase): pass
create_tests(TestGenerator, pythonlab.datadir('/resources/examples/Examples'))

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = AgrosTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestGenerator))
    suite.run(result)
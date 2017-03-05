import agros
import os

from test_suite.scenario import AgrosTestCase
from test_suite.scenario import AgrosTestResult

def create_tests(case, dir):
    for (path, dirs, files) in os.walk(dir):
        for file in files:
            name, extension = os.path.splitext(file)

            method = 'test_{0}_{1}'.format(os.path.split(path)[-1].replace(" ", "_"),
                                           name.replace(" ", "_")).lower().replace("~1", "")
            example = '{0}/{1}'.format(path, file)

            if (extension == '.ags'):
                setattr(case, method, get_ags_test(example))
            elif (extension == '.py'):
                setattr(case, method, get_py_test(example))

def get_ags_test(example):
    def test(self):
        agros.open_file(example)
        agros.problem(clear=False).computation().solve()
    return test
        
def get_py_test(example):
    def test(self):        
        with open(example) as f:
            exec(str(f.read() in globals()))
    return test

tests = list()
data_dirs = [agros.datadir('/resources/examples/Examples'),
             agros.datadir('/resources/examples/Other'),
             agros.datadir('/resources/examples/PythonLab'),
             agros.datadir('/resources/examples/Tutorials')]

for dir in data_dirs:
    for (path, dirs, files) in os.walk(dir):
        if not (any("ags" in file for file in files) or
                any("py" in file for file in files)):
            continue

        name = "TestExamples{0}".format(os.path.split(path)[-1].replace(" ", "")).replace("~1", "")
        code = compile('class {0}(AgrosTestCase): pass'.format(name), '<string>', 'exec')
        exec(code)
        create_tests(globals()[name], path)
        tests.append(globals()[name])

if __name__ == '__main__':
    import unittest as ut
    
    suite = ut.TestSuite()
    result = AgrosTestResult()
    for test in tests:
        suite.addTest(ut.TestLoader().loadTestsFromTestCase(test))
    suite.run(result)
from agrossuite import agros

import unittest
import os.path
from glob import glob

import platform

# root_windows = "../../usr/share/agrossuite/resources/examples/"
# root_linux = "/../../build/usr/share/agrossuite/resources/examples/"
#
# if platform.system() == 'Windows':
#     root = root_windows
# else:
#     root = root_linux

root = "a2d/"

class AgrosTutorialsA2D(unittest.TestCase):
    def __init__(self, methodName='runTutorialsA2D'):
        unittest.TestCase.__init__(self, methodName)
        
    def run_tutorial(self, fn):        
        str = "{0}".format(fn)
        
        # open problem
        agros.open_file(fn)
        problem = agros.problem(clear = False)
        
        # solve problem
        self.computation = problem.computation()
        self.computation.solve()
        
        result = True
        self.assertTrue(result, str)


def template(fn):
    base = os.path.basename(fn)
    classname = fn[len(root):-4].replace(" ", "").replace("/", "").replace("-", "").replace("\\", "")
    test_name = ("test_" + base[:-4].replace(" ", "_").
                 replace("-", "_").replace("\\", "").lower())
    file_test_name = ("test_" + fn[len(root):-4].replace(" ", "_").
                      replace("/", "_").replace("-", "_").replace("\\", "").lower() + ".py")
    
    out =  f"from tutorials_a2d import AgrosTutorialsA2D\n"
    out += f"\n"
    out += f"class {classname}(AgrosTutorialsA2D):\n"       
    out += f"    def {test_name}(self):\n"
    out += f"        self.run_tutorial(\"{fn}\")\n"

    f = open(file_test_name, "w")
    f.write(out)
    f.close()

    print(f"{classname} generated.")    


def generator():
    # remove files
    for fn in glob("test_*.py"):
        os.remove(fn)

    print("Generating tutorials tests")

    for fn in glob(root + "**/*.a2d", recursive=True):
        base = os.path.basename(fn)
        template(fn)


if __name__ == "__main__":
    generator()

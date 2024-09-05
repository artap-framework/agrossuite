from agrossuite import agros

import unittest
import os.path
from glob import glob

root = "../../usr/share/agrossuite/resources/examples/"

class AgrosTutorials(unittest.TestCase):
    def __init__(self, methodName='runTutorials'):
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
    classname = fn[len(root):-4].replace(" ", "").replace("/", "").replace("-", "")
    test_name = "test_" + base[:-4].replace(" ", "_").replace("-", "_").lower()
    file_test_name = "test_" + fn[len(root):-4].replace(" ", "_").replace("/", "_").replace("-", "_").lower() + ".py"
    
    out =  f"from tutorials import AgrosTutorials\n"
    out += f"\n"
    out += f"class {classname}(AgrosTutorials):\n"       
    out += f"    def {test_name}(self):\n"
    out += f"        self.run_tutorial(\"{fn}\")\n"

    f = open(file_test_name, "w")
    f.write(out)
    f.close()

    print(f"{fn} generated.")    

def generator():
    print("Generating tutorials tests")
    for fn in glob(root + "**/*.ags", recursive=True):            
        template(fn)   

if __name__ == "__main__":
    generator()

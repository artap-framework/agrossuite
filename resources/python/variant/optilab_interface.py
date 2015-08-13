from variant import ModelDictionary
from variant.model_postprocessor import ModelPostprocessor

if __name__ != '__main__':
    # global optilab model postprocessor
    _optilab_mp =  None
        
def _models_zip(filename):
    md = ModelDictionary()
    md.load(filename)

    global _optilab_mp
    _optilab_mp = ModelPostprocessor(md)

    lst = []
    for k, m in sorted(md.dictionary.items()):
        lst.append({ 'key' : k, 'solved' : m.solved})

    return lst

def _open_in_agros2d(file_name):    
    import os.path    
    import sys; 
    sys.path.insert(0, os.path.abspath(os.path.join(file_name, os.pardir, os.pardir)))

    import problem
    p = problem.Model()
    p.load(file_name)
    p.create()
    
    return p
    
def _solve_in_agros2d(file_name):
    p = _open_in_agros2d(file_name)
    p.solve()
    p.save(file_name)

"""
def parameters(self, only_numbers=False):
    if not only_numbers:
        return sorted(list(self.model_class().parameters.data_types().keys()))
    else:
        return sorted(list(name for name, type in self.model_class().parameters.data_types().items()\
                            if (type == int or type == float)))

def variables(self, only_numbers=False):
    if not only_numbers:
        return sorted(list(self.model_class().variables.data_types().keys()))
    else:
        return sorted(list(name for name, type in self.model_class().variables.data_types().items()\
                            if (type == int or type == float)))
"""
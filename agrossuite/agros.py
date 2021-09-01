import os
import ctypes

# import libs
path = os.path.dirname(os.path.realpath(__file__))

ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_triangle.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_dxflib.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_poly2tri.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_quazip.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_matio.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_nlopt2.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_bayesopt.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_nsga2.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_stb_truetype.so")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_3rdparty_ctemplate.so")
ctypes.cdll.LoadLibrary(path + "/libs/libdeal_II.so.9.2.0")
ctypes.cdll.LoadLibrary(path + "/libs/libagros_library.so")
ctypes.cdll.LoadLibrary(path + "/libs/libsolver_plugin_MUMPS.so")
ctypes.cdll.LoadLibrary(path + "/libs/libsolver_plugin_UMFPACK.so")

from . import _agros

# set datadir
_agros._set_datadir(path)
# read plugins
_agros._read_plugins()

# set properties
cachedir = _agros.cachedir
computation = _agros.computation
datadir = _agros.datadir
get_script_from_model = _agros.get_script_from_model
open_file = _agros.open_file
options = _agros.options
positive_value = _agros.positive_value
problem = _agros.problem
save_file = _agros.save_file
tempdir = _agros.tempdir
value_in_list = _agros.value_in_list
value_in_range = _agros.value_in_range
version = _agros.version

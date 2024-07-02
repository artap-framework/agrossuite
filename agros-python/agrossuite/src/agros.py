import os
import ctypes
import platform

# import libs
path = os.path.dirname(os.path.realpath(__file__))
# os.environ['LD_LIBRARY_PATH'] = path + "/lib" 


if platform.system() == 'Windows':
	ctypes.cdll.LoadLibrary(path + "/agros_library.dll")
	ctypes.cdll.LoadLibrary(path + "/agros_python.dll")
	ctypes.cdll.LoadLibrary(path + "/agros_3rdparty_triangle.dll")
	ctypes.cdll.LoadLibrary(path + "/agros_3rdparty_triangle.dll")

	if os.path.isfile(path + "/lib/solver_plugin_MUMPS.dll"):
		try:
			ctypes.cdll.LoadLibrary(path + "/lib/solver_plugin_MUMPS.dll")
			ctypes.cdll.LoadLibrary(path + "/lib/libifcoremd.dll")
			ctypes.cdll.LoadLibrary(path + "/lib/libmmd.dll")

		except Exception as e:
			print(f'Error: {e}')
else:
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_python.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_triangle.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_dxflib.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_quazip.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_matio.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_nlopt2.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_bayesopt.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_nsga2.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_3rdparty_ctemplate.so")
	ctypes.cdll.LoadLibrary(path + "/lib/libdeal_II.so.9.5.2")
	ctypes.cdll.LoadLibrary(path + "/lib/libagros_library.so")
	if os.path.isfile(path + "/lib/libsolver_plugin_MUMPS.so"): 
		ctypes.cdll.LoadLibrary(path + "/lib/libsolver_plugin_MUMPS.so")

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

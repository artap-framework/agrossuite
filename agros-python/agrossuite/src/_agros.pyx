# distutils: language = c++

# imports
from libcpp cimport bool
from libcpp.pair cimport pair
from libcpp.vector cimport vector
from libcpp.map cimport map

from cython.operator cimport preincrement as incr
from cython.operator cimport dereference as deref

cdef extern from "limits.h":
    int c_INT_MIN "INT_MIN"
    int c_INT_MAX "INT_MAX"
    int c_DOUBLE_MIN "DOUBLE_MIN"
    int c_DOUBLE_MAX "DOUBLE_MAX"

cdef extern from "<string>" namespace "std":
    cdef cppclass string:
        string()
        string(char *)
        char * c_str()

# Parameters class
class __Parameters__(dict):
    def __init__(self, get_method, set_method, check_set = True):
        self.get = get_method
        self.set = set_method
        self.check_set = check_set
        dict.__init__(self, self.get())

    def __update__(self):
        parameters = self.get()
        for parameters_key in parameters:
            dict.__setitem__(self, parameters_key, parameters[parameters_key])

    def __getitem__(self, key):
        self.__update__()
        return dict.__getitem__(self, key)

    def __setitem__(self, key, value):
        if (self.check_set):
            if (not key in self):
                raise KeyError("Invalid key. Valid keys: {0}".format(self.keys()))

        dict.__setitem__(self, key, value)
        self.set(dict(self))

    def get_parameters(self):
        self.__update__()
        return self

# test functions
def value_in_range(value, min, max, key):
    if (value < min or value > max):
        raise IndexError("Value of '{0}' is out of range ({1} - {2})".format(key, min, max))

def value_in_list(value, list, key):
    for item in list:
        if (value == item):
            return

    raise KeyError("Key '{0}' is invalid. Valid keys: {1}".format(list, key))

def positive_value(value, key):
    if (value < 0):
        raise IndexError("Value of {0} must be possitive.".format(key))

# convert functions
cdef vector[int] list_to_int_vector(list):
    cdef vector[int] int_vector
    for item in list:
        int_vector.push_back(item)

    return int_vector

cdef vector[double] list_to_double_vector(list):
    cdef vector[double] double_vector
    for item in list:
        double_vector.push_back(item)

    return double_vector

cdef object double_vector_to_list(vector[double] vector):
    out = list()
    for i in range(vector.size()):
        out.append(vector[i])

    return out

cdef map[string, int] dictionary_to_int_map(dictionary):
    cdef map[string, int] int_map
    cdef pair[string, int] row
    for key in dictionary:
        row.first = key.encode()
        row.second = dictionary[key]
        int_map.insert(row)

    return int_map

cdef map[string, string] dictionary_to_string_map(dictionary):
    cdef map[string, string] string_map
    cdef pair[string, string] row
    for key in dictionary:
        row.first = key.encode()
        row.second = dictionary[key].encode()
        string_map.insert(row)

    return string_map

# wrappers
include "pygeometry.pxi"
include "pyfield.pxi"
include "pyproblem.pxi"
include "pycomputation.pxi"
include "pysolution.pxi"
include "pystudy.pxi"

cdef extern from "../../agros-python/pythonlab/pyagros.h":
    # open and save
    void openFile(string &file) except +
    void saveFile(string &file) except +
    string getScriptFromModel()

    # temp and cache
    string tempDir()
    string cacheDir()
    string dataDir()
    string setDataDir(string &str)
    void readPlugins()

    # version()
    string pyVersion()

    # PyOptions
    cdef cppclass PyOptions:
        int getCacheSize()
        void setCacheSize(int size) except +

        bool getSaveMatrixRHS()
        void setSaveMatrixRHS(bool save)

        bool getSaveSystem()
        void setSaveSystem(bool save)

        string getDumpFormat()
        void setDumpFormat(string format) except +

        bool getStdOutLog()
        void setStdOutLog(bool enabled)

def open_file(file):
    openFile(file.encode())

def save_file(file):
    saveFile(file.encode())

def get_script_from_model():
    return getScriptFromModel().decode()

def tempdir(dir = ""):
    return "{0}/{1}".format(tempDir().decode(), dir)

def cachedir(dir = ""):
    return "{0}/{1}".format(cacheDir().decode(), dir)

# datadir()
def datadir(str = ""):
    return dataDir().decode()
def _set_datadir(str):
    setDataDir(str.encode())
def _read_plugins():
    readPlugins()


def version():
    return pyVersion()

cdef class __Options__:
    cdef PyOptions *thisptr

    def __cinit__(self):
        self.thisptr = new PyOptions()
    def __dealloc__(self):
        del self.thisptr

    property cache_size:
        def __get__(self):
            return self.thisptr.getCacheSize()
        def __set__(self, size):
            self.thisptr.setCacheSize(size)

    property save_matrix_and_rhs:
        def __get__(self):
            return self.thisptr.getSaveMatrixRHS()
        def __set__(self, save):
            self.thisptr.setSaveMatrixRHS(save)

    property save_system:
        def __get__(self):
            return self.thisptr.getSaveSystem()
        def __set__(self, save):
            self.thisptr.setSaveSystem(save)

    property dump_format:
        def __get__(self):
            return self.thisptr.getDumpFormat().decode()
        def __set__(self, format):
            self.thisptr.setDumpFormat(format.encode())

    property log_stdout:
        def __get__(self):
            return self.thisptr.getStdOutLog()
        def __set__(self, enabled):
            self.thisptr.setStdOutLog(enabled)

options = __Options__()

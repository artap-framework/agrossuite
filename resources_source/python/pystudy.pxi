cdef extern from "../../agros2d-library/pythonlab/pyproblem.h":
    cdef cppclass PyStudy:
        PyStudy(string type)

        void addParameter(string name, double lowerBound, double upperBound) except +
        void addFunctional(string name, string expression, int weight) except +
        void solve() except +

cdef class __Study__:
    cdef PyStudy *_study

    def __cinit__(self, type):
        self._study = new PyStudy(type)

    def __dealloc__(self):
        del self._study

    def add_parameter(self, name, lowerBound, upperBound):
        self._study.addParameter(name.encode(), lowerBound, upperBound)

    def add_functional(self, name, expression, weight = 100):
        self._study.addFunctional(name.encode(), expression.encode(), weight)

    def solve(self):
        self._study.solve();

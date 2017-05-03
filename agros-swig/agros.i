/* example.i */
#if defined(SWIGJAVA) || defined(SWIGCSHARP)
%module utils
#else
%module agros
#endif
%include "std_string.i"
%include "exception.i"
/*
%{
#define SWIG_FILE_WITH_INIT
#include "../../agros-swig/src/swig_agros.h"
#include "../../agros-swig/src/swig_problem.h"
#include "../../agros-swig/src/swig_field.h"
#include "../../agros-swig/src/swig_geometry.h"
%}

%rename(version) swigVersion;
extern std::string swigVersion();

class SwigProblemBase
{
public:
    SwigProblemBase();
    virtual ~SwigProblemBase();
};

%rename(Field) SwigField;
class SwigField
{
public:
    SwigField(const std::string fieldId);
    virtual ~SwigField();

    // field id
    std::string fieldId() const;

    // analysis type
    std::string getAnalysisType() const;
    void setAnalysisType(const std::string &analysisType);

    // linearity type
    std::string getLinearityType() const;
    void setLinearityType(const std::string &linearityType);

    // number of refinements
    inline int getNumberOfRefinements();
    void setNumberOfRefinements(int numberOfRefinements);

    // polynomial order
    inline int getPolynomialOrder() const;
    void setPolynomialOrder(int polynomialOrder);

    // matrix solver
    inline std::string getMatrixSolver() const;
    void setMatrixSolver(const std::string &matrixSolver);

    #ifdef SWIGPYTHON
    %pythoncode
    %{
        __swig_getmethods__["field_id"] = fieldId
        if _newclass: field_id = property(fieldId, None)

        __swig_getmethods__["analysis_type"] = getAnalysisType
        __swig_setmethods__["analysis_type"] = setAnalysisType
        if _newclass: analysis_type = property(getAnalysisType, setAnalysisType)

        __swig_getmethods__["number_of_refinements"] = getNumberOfRefinements
        __swig_setmethods__["number_of_refinements"] = setNumberOfRefinements
        if _newclass: number_of_refinements = property(getNumberOfRefinements, setNumberOfRefinements)

        __swig_getmethods__["polynomial_order"] = getPolynomialOrder
        __swig_setmethods__["polynomial_order"] = setPolynomialOrder
        if _newclass: polynomial_order = property(getPolynomialOrder, setPolynomialOrder)

        __swig_getmethods__["solver"] = getLinearityType
        __swig_setmethods__["solver"] = setLinearityType
        if _newclass: solver = property(getLinearityType, setLinearityType)
    %}
    #endif
};

%rename(Geometry) SwigGeometry;
class SwigGeometry
{
public:
    SwigGeometry();
    virtual ~SwigGeometry();
};

%rename(Problem) SwigProblem;
class SwigProblem : public SwigProblemBase
{
public:
    SwigProblem(bool clear = true);
    virtual ~SwigProblem();

    SwigField *field(const std::string fieldId);
    SwigGeometry *geometry();
    SwigComputation *computation(bool newComputation = true);

    void load(const std::string &fn);
    void save(const std::string &fn);

    std::string getCoordinateType();
    void setCoordinateType(const std::string &coordinateType);

    std::string getMeshType();
    void setMeshType(const std::string &meshType);

    #ifdef SWIGPYTHON
    %pythoncode
    %{
        __swig_getmethods__["coordinate_type"] = getCoordinateType
        __swig_setmethods__["coordinate_type"] = setCoordinateType
        if _newclass: coordinate_type = property(getCoordinateType, setCoordinateType)

        __swig_getmethods__["mesh_type"] = getMeshType
        __swig_setmethods__["mesh_type"] = setMeshType
        if _newclass: mesh_type = property(getMeshType, setMeshType)
    %}
    #endif
};

#ifdef SWIGPYTHON
%exception solve {
  try {
    $action
  } catch (std::logic_error &e) {
    PyErr_SetString(PyExc_RuntimeError, const_cast<char*>(e.what())); SWIG_fail;
  }
}
#endif

%rename(Computation) SwigComputation;
class SwigComputation : public SwigProblemBase
{
public:
    SwigComputation(bool newComputation = true);
    SwigComputation(const std::string &computation);
    virtual ~SwigComputation();

    void solve();
    void clear();
};
*/

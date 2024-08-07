// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#ifndef PYTHONLABPROBLEM_H
#define PYTHONLABPROBLEM_H

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/plugin_interface.h"

#include "optilab/study.h"

 class AGROS_LIBRARY_API PyProblemBase
{
public:
    PyProblemBase() {}
    ~PyProblemBase() {}

    // properties
    inline std::string getCoordinateType() const
    {
        return coordinateTypeToStringKey(m_problem->config()->coordinateType()).toStdString();
    }
    inline std::string getMeshType() const
    {
        return meshTypeToStringKey(m_problem->config()->meshType()).toStdString();
    }
    inline double getFrequency() const
    {
        return m_problem->config()->value(ProblemConfig::Frequency).value<Value>().number();
    }
    inline std::string getTimeStepMethod() const
    {
        return timeStepMethodToStringKey((TimeStepMethod) m_problem->config()->value(ProblemConfig::TimeMethod).toInt()).toStdString();
    }
    inline double getTimeMethodTolerance() const
    {
        return m_problem->config()->value(ProblemConfig::TimeMethodTolerance).toDouble();
    }
    inline int getTimeMethodOrder() const
    {
        return m_problem->config()->value(ProblemConfig::TimeOrder).toInt();
    }
    inline double getInitialTimeStep() const
    {
        return m_problem->config()->value(ProblemConfig::TimeInitialStepSize).toDouble();
    }
    inline double getTimeTotal() const
    {
        return m_problem->config()->value(ProblemConfig::TimeTotal).toDouble();
    }
    inline int getNumConstantTimeSteps() const
    {
        return m_problem->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt();
    }

    void getParameters(std::vector<std::string> &keys) const;
    double getParameter(const std::string &key) const;
    void setParameter(const std::string &key, double value);

    std::string getCouplingType(const std::string &sourceField, const std::string &targetField) const;
    void setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type);

protected:
    QSharedPointer<ProblemBase> m_problem;
    void checkExistingFields(const QString &sourceField, const QString &targetField) const;
};

class AGROS_LIBRARY_API PyProblem : public PyProblemBase
{
public:
    PyProblem(bool clearProblem);
    ~PyProblem();

    void clear();

    // properties
    void setCoordinateType(const std::string &coordinateType);
    void setMeshType(const std::string &meshType);
    void setFrequency(double frequency);
    void setTimeStepMethod(const std::string &timeStepMethod);
    void setTimeMethodTolerance(double timeMethodTolerance);
    void setTimeMethodOrder(int timeMethodOrder);
    void setInitialTimeStep(double initialTimeStep);
    void setTimeTotal(double timeTotal);
    void setNumConstantTimeSteps(int timeSteps);

    void load(const std::string &fn);
    void save(const std::string &fn);

    // type of Study at index
    // TODO: do it better!
    std::string typeOfStudyAtIndex(int index);
};

class AGROS_LIBRARY_API PyComputation : public PyProblemBase
{
public:
    PyComputation(bool newComputation);
    PyComputation(const std::string &computation);
    ~PyComputation();

    void clear();
    void solve();

    inline QSharedPointer<Computation> computation() const
    {
        return qSharedPointerDynamicCast<Computation>(m_problem);
    }


    double timeElapsed() const;
    void timeStepsLength(vector<double> &steps) const;
    void timeStepsTimes(vector<double> &times) const;

    // results
    void getResults(std::vector<std::string> &keys) const;
    double getResult(const std::string &key) const;
    void setResult(const std::string &key, double value);
};

class AGROS_LIBRARY_API PySolution
{
public:
    PySolution() {}
    ~PySolution();
    void setComputation(PyComputation *computation, const std::string &fieldId);

    // local values, integrals
    void localValues(double x, double y, int timeStep, int adaptivityStep,
                     map<std::string, double> &results) const;
    void surfaceIntegrals(const vector<int> &edges, int timeStep, int adaptivityStep,
                          map<std::string, double> &results) const;
    void volumeIntegrals(const vector<int> &labels, int timeStep, int adaptivityStep,
                         map<std::string, double> &results) const;

    // mesh info
    void initialMeshInfo(map<std::string, int> &info) const;
    void solutionMeshInfo(int timeStep, int adaptivityStep, map<std::string, int> &info) const;

    // solver info
    void solverInfo(int timeStep, int adaptivityStep,
                    vector<double> &solutionsChange, vector<double> &residual,
                    vector<double> &dampingCoeff, int &jacobianCalculations) const;

    // adaptivity info
    void adaptivityInfo(int timeStep, vector<double> &error, vector<int> &dofs) const;

    // solution (internal)
    void getSolution(int timeStep, int adaptivityStep, vector<double> &sln) const;
    void setSolution(int timeStep, int adaptivityStep, vector<double> &sln);

    // export vtk
    void exportVTK(const std::string &fileName, int timeStep, int adaptivityStep, const std::string &variable, std::string physicFieldVariableComp);

private:
    QSharedPointer<Computation> m_computation;
    FieldInfo *m_fieldInfo;

    int getTimeStep(int timeStep) const;
    int getAdaptivityStep(int adaptivityStep, int timeStep) const;
};

#endif // PYTHONLABPROBLEM_H

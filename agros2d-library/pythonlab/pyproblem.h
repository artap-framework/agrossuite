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

class PyProblem
{
public:
    PyProblem(bool clearProblem);
    ~PyProblem() {}

    void clear();
    void refresh();

    inline std::string getCoordinateType() const { return coordinateTypeToStringKey(m_problem->config()->coordinateType()).toStdString(); }
    inline std::string getMeshType() const { return meshTypeToStringKey(m_problem->config()->meshType()).toStdString(); }
    inline double getFrequency() const { return (Value::parseValueFromString(m_problem->config()->value(ProblemConfig::Frequency).toString())).number(); }
    inline std::string getTimeStepMethod() const { return timeStepMethodToStringKey((TimeStepMethod) m_problem->config()->value(ProblemConfig::TimeMethod).toInt()).toStdString(); }
    inline double getTimeMethodTolerance() const { return m_problem->config()->value(ProblemConfig::TimeMethodTolerance).toDouble(); }
    inline int getTimeMethodOrder() const { return m_problem->config()->value(ProblemConfig::TimeOrder).toInt(); }
    inline double getTimeInitialTimeStep() const { return m_problem->config()->value(ProblemConfig::TimeInitialStepSize).toDouble(); }
    inline double getTimeTotal() const { return m_problem->config()->value(ProblemConfig::TimeTotal).toDouble(); }
    inline int getNumConstantTimeSteps() const { return m_problem->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt(); }
    std::string getCouplingType(const std::string &sourceField, const std::string &targetField) const;

protected:
    void checkExistingFields(const QString &sourceField, const QString &targetField) const;
    QSharedPointer<Problem> m_problem;
};

class PyPreprocessor : public PyProblem
{
public:
    PyPreprocessor(bool clearProblem);
    ~PyPreprocessor() {}

    void setCoordinateType(const std::string &coordinateType);
    void setMeshType(const std::string &meshType);
    void setFrequency(double frequency);
    void setTimeStepMethod(const std::string &timeStepMethod);
    void setTimeMethodTolerance(double timeMethodTolerance);
    void setTimeMethodOrder(int timeMethodOrder);
    void setTimeInitialTimeStep(double timeInitialTimeStep);
    void setTimeTotal(double timeTotal);
    void setNumConstantTimeSteps(int timeSteps);
    void setCouplingType(const std::string &sourceField, const std::string &targetField, const std::string &type);
};

class PyComputation : public QObject, public PyProblem
{
    Q_OBJECT

public:
    PyComputation(bool newComputation = false);
    ~PyComputation() {}

    void clearSolution();

    void mesh();
    void solve();

    double timeElapsed() const;
    void timeStepsLength(vector<double> &steps) const;
    void timeStepsTimes(vector<double> &times) const;

protected:
    QSharedPointer<ProblemComputation> m_computation;
};

#endif // PYTHONLABPROBLEM_H

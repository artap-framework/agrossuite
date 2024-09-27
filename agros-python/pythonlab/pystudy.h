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

#ifndef PYTHONLABSTUDY_H
#define PYTHONLABSTUDY_H

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/plugin_interface.h"

#include "optilab/study.h"
#include "optilab/study_bayesopt.h"
#include "optilab/study_nsga2.h"
#include "optilab/study_nlopt.h"
#include "optilab/study_sweep.h"
#include "optilab/study_model.h"

#include "nlopt.hpp"

class AGROS_LIBRARY_API PyStudy
{
public:
    PyStudy();
    virtual ~PyStudy() {}

    inline std::string type() { return studyTypeToStringKey(m_study->type()).toStdString(); }

    void addParameter(std::string name, double lowerBound, double upperBound);
    void addGoalFunction(std::string name, std::string expression, int weight = 100);
    void getParameters(std::vector<std::string> &name, std::vector<double> &lowerBound, std::vector<double> &upperBound) const;
    void getGoalFunctions(std::vector<std::string> &name, std::vector<std::string> &expression, std::vector<int> &weight) const;

    void solve() const;

    virtual Study *study() = 0;
    virtual Study *study() const = 0;

    template <typename Type>
    void setParameter(const std::string &parameter, Type value)
    {
        assert(study()->stringKeys().contains(QString::fromStdString(parameter)));
        study()->setValue(study()->stringKeyToType(QString::fromStdString(parameter)), value);
    }

    inline int getBoolParameter(const std::string &parameter)
    {
        assert(study()->stringKeys().contains(QString::fromStdString(parameter)));
        return study()->value(study()->stringKeyToType(QString::fromStdString(parameter))).toBool();
    }

    inline int getIntParameter(const std::string &parameter)
    {
        assert(study()->stringKeys().contains(QString::fromStdString(parameter)));
        return study()->value(study()->stringKeyToType(QString::fromStdString(parameter))).toInt();
    }

    inline double getDoubleParameter(const std::string &parameter)
    {
        assert(study()->stringKeys().contains(QString::fromStdString(parameter)));
        return study()->value(study()->stringKeyToType(QString::fromStdString(parameter))).toDouble();
    }

    inline std::string getStringParameter(const std::string &parameter)
    {
        assert(study()->stringKeys().contains(QString::fromStdString(parameter)));
        return study()->value(study()->stringKeyToType(QString::fromStdString(parameter))).toString().toStdString();
    }

    std::string findExtreme(std::string type, std::string key, bool minimum);    

    void steps(vector<int> &steps) const;
    void values(std::string variable, vector<double> &values) const;
    void results(vector<int> &steps, vector<std::string> &names, vector<std::string> &types, vector<double> &values) const;

    void getComputationProblemDir(int index, std::string &problemDir) const;

protected:
    Study *m_study;
};

class AGROS_LIBRARY_API PyStudyBayesOpt : public PyStudy
{
public:
    PyStudyBayesOpt(int index = -1);
    virtual ~PyStudyBayesOpt() {}

    virtual StudyBayesOpt *study() { return static_cast<StudyBayesOpt *>(m_study); }
    virtual StudyBayesOpt *study() const { return static_cast<StudyBayesOpt *>(m_study); }

    // BayesOpt
    inline std::string getInitMethod() const { return m_study->value(Study::BayesOpt_init_method).toString().toStdString(); }
    void setInitMethod(const std::string &initMethod);

    inline std::string getSurrName() const { return m_study->value(Study::BayesOpt_surr_name).toString().toStdString(); }
    void setSurrName(const std::string &surrName);

    inline std::string getScoreType() const { return m_study->value(Study::BayesOpt_sc_type).toString().toStdString(); }
    void setScoreType(const std::string &scoreType);

    inline std::string getLearningType() const { return m_study->value(Study::BayesOpt_l_type).toString().toStdString(); }
    void setLearningType(const std::string &learningType);
};

class AGROS_LIBRARY_API PyStudyNLopt : public PyStudy
{
public:
    PyStudyNLopt(int index = -1);
    virtual ~PyStudyNLopt() {}

    virtual StudyNLopt *study() { return static_cast<StudyNLopt *>(m_study); }
    virtual StudyNLopt *study() const { return static_cast<StudyNLopt *>(m_study); }

    inline std::string getAlgorithm() const { return m_study->value(Study::NLopt_algorithm).toString().toStdString(); }
    void setAlgorithm(const std::string &algorithm);
};

class AGROS_LIBRARY_API PyStudyNSGA2 : public PyStudy
{
public:
    PyStudyNSGA2(int index = -1);
    virtual ~PyStudyNSGA2() {}

    virtual StudyNSGA2 *study() { return static_cast<StudyNSGA2 *>(m_study); }
    virtual StudyNSGA2 *study() const { return static_cast<StudyNSGA2 *>(m_study); }
};

class AGROS_LIBRARY_API PyStudySweep : public PyStudy
{
public:
    PyStudySweep(int index = -1);
    virtual ~PyStudySweep() {}

    virtual StudySweep *study() { return static_cast<StudySweep *>(m_study); }
    virtual StudySweep *study() const { return static_cast<StudySweep *>(m_study); }

    // Sweep
    inline std::string getInitMethod() const { return m_study->value(Study::Sweep_init_method).toString().toStdString(); }
    void setInitMethod(const std::string &initMethod);
};

class AGROS_LIBRARY_API PyStudyModel : public PyStudy
{
public:
    PyStudyModel(int index = -1);
    virtual ~PyStudyModel() {}

    virtual StudyModel *study() { return static_cast<StudyModel *>(m_study); }
    virtual StudyModel *study() const { return static_cast<StudyModel *>(m_study); }

    // Model
    // inline std::string getInitMethod() const { return m_study->value(Study::Sweep_init_method).toString().toStdString(); }
    // void setInitMethod(const std::string &initMethod);
};
#endif // PYTHONLABSTUDY_H


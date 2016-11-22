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
#include "optilab/study_nsga3.h"
#include "optilab/study_nlopt.h"
#include "optilab/study_sweep.h"
#include "optilab/study_limbo.h"
#include "optilab/study_cmaes.h"

#include "nlopt.hpp"

class PyStudy
{
public:
    PyStudy();
    virtual ~PyStudy() {}

    inline std::string type() { return studyTypeToStringKey(m_study->type()).toStdString(); }

    void addParameter(std::string name, double lowerBound, double upperBound,
                      bool penaltyEnabled = false, double scale = 0.0, double mu = 0.0, double sigma = 0.0);
    void addFunctional(std::string name, std::string expression, int weight = 100);

    void solve();

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

    std::string findExtreme(std::string type, std::string key, bool minimum);    

    void steps(vector<int> &steps) const;
    void values(std::string variable, vector<double> &values) const;

protected:
    Study *m_study;
};

class PyStudyBayesOpt : public PyStudy
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

class PyStudyLimbo : public PyStudy
{
public:
    PyStudyLimbo(int index = -1);
    virtual ~PyStudyLimbo() {}

    virtual StudyLimbo *study() { return static_cast<StudyLimbo *>(m_study); }
    virtual StudyLimbo *study() const { return static_cast<StudyLimbo *>(m_study); }

    inline std::string getMeanType() const { return m_study->value(Study::LIMBO_mean).toString().toStdString(); }
    void setMeanType(const std::string &meanType);

    inline std::string getGPType() const { return m_study->value(Study::LIMBO_gp).toString().toStdString(); }
    void setGPType(const std::string &gpType);

    inline std::string getAcquiType() const { return m_study->value(Study::LIMBO_acqui).toString().toStdString(); }
    void setAcquiType(const std::string &acquiType);
};

class PyStudyNLopt : public PyStudy
{
public:
    PyStudyNLopt(int index = -1);
    virtual ~PyStudyNLopt() {}

    virtual StudyNLopt *study() { return static_cast<StudyNLopt *>(m_study); }
    virtual StudyNLopt *study() const { return static_cast<StudyNLopt *>(m_study); }

    inline std::string getAlgorithm() const { return m_study->value(Study::NLopt_algorithm).toString().toStdString(); }
    void setAlgorithm(const std::string &algorithm);
};

class PyStudyCMAES : public PyStudy
{
public:
    PyStudyCMAES(int index = -1);
    virtual ~PyStudyCMAES() {}

    virtual StudyCMAES *study() { return static_cast<StudyCMAES *>(m_study); }
    virtual StudyCMAES *study() const { return static_cast<StudyCMAES *>(m_study); }

    inline std::string getAlgorithm() const { return m_study->value(Study::CMAES_algorithm).toString().toStdString(); }
    void setAlgorithm(const std::string &algorithm);

    inline std::string getSurrogate() const { return m_study->value(Study::CMAES_surrogate).toString().toStdString(); }
    void setSurrogate(const std::string &surrogate);
};

class PyStudyNSGA2 : public PyStudy
{
public:
    PyStudyNSGA2(int index = -1);
    virtual ~PyStudyNSGA2() {}

    virtual StudyNSGA2 *study() { return static_cast<StudyNSGA2 *>(m_study); }
    virtual StudyNSGA2 *study() const { return static_cast<StudyNSGA2 *>(m_study); }
};

class PyStudyNSGA3 : public PyStudy
{
public:
    PyStudyNSGA3(int index = -1);
    virtual ~PyStudyNSGA3() {}

    virtual StudyNSGA3 *study() { return static_cast<StudyNSGA3 *>(m_study); }
    virtual StudyNSGA3 *study() const { return static_cast<StudyNSGA3 *>(m_study); }
};

class PyStudySweep : public PyStudy
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


#endif // PYTHONLABSTUDY_H


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

#include "nlopt.hpp"

class SwigStudy
{
public:
    SwigStudy();
    virtual ~SwigStudy() {}

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

class SwigStudyBayesOpt : public SwigStudy
{
public:
    SwigStudyBayesOpt(int index = -1);
    virtual ~SwigStudyBayesOpt() {}

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

class SwigStudyLimbo : public SwigStudy
{
public:
    SwigStudyLimbo(int index = -1);
    virtual ~SwigStudyLimbo() {}

    virtual StudyLimbo *study() { return static_cast<StudyLimbo *>(m_study); }
    virtual StudyLimbo *study() const { return static_cast<StudyLimbo *>(m_study); }

    inline std::string getMeanType() const { return m_study->value(Study::LIMBO_mean).toString().toStdString(); }
    void setMeanType(const std::string &meanType);

    inline std::string getGPType() const { return m_study->value(Study::LIMBO_gp).toString().toStdString(); }
    void setGPType(const std::string &gpType);

    inline std::string getAcquiType() const { return m_study->value(Study::LIMBO_acqui).toString().toStdString(); }
    void setAcquiType(const std::string &acquiType);
};

class SwigStudyNLopt : public SwigStudy
{
public:
    SwigStudyNLopt(int index = -1);
    virtual ~SwigStudyNLopt() {}

    virtual StudyNLopt *study() { return static_cast<StudyNLopt *>(m_study); }
    virtual StudyNLopt *study() const { return static_cast<StudyNLopt *>(m_study); }

    inline std::string getAlgorithm() const { return m_study->value(Study::NLopt_algorithm).toString().toStdString(); }
    void setAlgorithm(const std::string &algorithm);
};

class SwigStudyNSGA2 : public SwigStudy
{
public:
    SwigStudyNSGA2(int index = -1);
    virtual ~SwigStudyNSGA2() {}

    virtual StudyNSGA2 *study() { return static_cast<StudyNSGA2 *>(m_study); }
    virtual StudyNSGA2 *study() const { return static_cast<StudyNSGA2 *>(m_study); }
};

class SwigStudyNSGA3 : public SwigStudy
{
public:
    SwigStudyNSGA3(int index = -1);
    virtual ~SwigStudyNSGA3() {}

    virtual StudyNSGA3 *study() { return static_cast<StudyNSGA3 *>(m_study); }
    virtual StudyNSGA3 *study() const { return static_cast<StudyNSGA3 *>(m_study); }
};

class SwigStudySweep : public SwigStudy
{
public:
    SwigStudySweep(int index = -1);
    virtual ~SwigStudySweep() {}

    virtual StudySweep *study() { return static_cast<StudySweep *>(m_study); }
    virtual StudySweep *study() const { return static_cast<StudySweep *>(m_study); }

    // Sweep
    inline std::string getInitMethod() const { return m_study->value(Study::Sweep_init_method).toString().toStdString(); }
    void setInitMethod(const std::string &initMethod);
};


#endif // PYTHONLABSTUDY_H


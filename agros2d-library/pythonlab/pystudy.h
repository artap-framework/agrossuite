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

#include "nlopt.hpp"

class PyStudy
{
public:
    PyStudy();
    virtual ~PyStudy() {}

    inline std::string type() { return studyTypeToStringKey(m_study->type()).toStdString(); }

    void addParameter(std::string name, double lowerBound, double upperBound);
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

protected:
    Study *m_study;
};

class PyStudyBayesOpt : public PyStudy
{
public:
    PyStudyBayesOpt();
    virtual ~PyStudyBayesOpt() {}

    virtual StudyBayesOpt *study() { return static_cast<StudyBayesOpt *>(m_study); }
    virtual StudyBayesOpt *study() const { return static_cast<StudyBayesOpt *>(m_study); }

    // BayesOpt
    inline std::string getInitMethod() const { return study()->initMethodToStringKey(m_study->value(Study::BayesOpt_init_method).toInt()).toStdString(); }
    void setInitMethod(const std::string &initMethod);

    inline std::string getSurrName() const { return m_study->value(Study::BayesOpt_surr_name).toString().toStdString(); }
    void setSurrName(const std::string &surrName);

    inline std::string getScoreType() const { return study()->scoreTypeToStringKey((score_type) m_study->value(Study::BayesOpt_sc_type).toInt()).toStdString(); }
    void setScoreType(const std::string &scoreType);

    inline std::string getLearningType() const { return study()->learningTypeToStringKey((learning_type) m_study->value(Study::BayesOpt_l_type).toInt()).toStdString(); }
    void setLearningType(const std::string &learningType);
};

class PyStudyNLopt : public PyStudy
{
public:
    PyStudyNLopt();
    virtual ~PyStudyNLopt() {}

    virtual StudyNLopt *study() { return static_cast<StudyNLopt *>(m_study); }
    virtual StudyNLopt *study() const { return static_cast<StudyNLopt *>(m_study); }

    inline std::string getAlgorithm() const { return study()->algorithmToStringKey((nlopt::algorithm) m_study->value(Study::NLopt_algorithm).toInt()).toStdString(); }
    void setAlgorithm(const std::string &algorithm);
};

class PyStudyNSGA2 : public PyStudy
{
public:
    PyStudyNSGA2();
    virtual ~PyStudyNSGA2() {}

    virtual StudyNSGA2 *study() { return static_cast<StudyNSGA2 *>(m_study); }
    virtual StudyNSGA2 *study() const { return static_cast<StudyNSGA2 *>(m_study); }
};

class PyStudyNSGA3 : public PyStudy
{
public:
    PyStudyNSGA3();
    virtual ~PyStudyNSGA3() {}

    virtual StudyNSGA3 *study() { return static_cast<StudyNSGA3 *>(m_study); }
    virtual StudyNSGA3 *study() const { return static_cast<StudyNSGA3 *>(m_study); }
};

#endif // PYTHONLABSTUDY_H

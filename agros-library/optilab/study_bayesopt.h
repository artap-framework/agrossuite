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

#ifndef STUDY_BAYESOPT_H
#define STUDY_BAYESOPT_H

#include "util/util.h"
#include "util/enums.h"
#include "study.h"

#include "bayesopt/bayesopt.hpp"

class AGROS_LIBRARY_API StudyBayesOpt;

class BayesOptProblem : public bayesopt::ContinuousModel
{
public:
    BayesOptProblem(StudyBayesOpt *study, bayesopt::Parameters par);

    double evaluateSample(const vectord &x) override;
    bool checkReachability(const vectord &x) override;

private:
    StudyBayesOpt *m_study;
    int m_steps;
};

class AGROS_LIBRARY_API StudyBayesOpt : public Study
{
public:
    StudyBayesOpt();

    virtual inline StudyType type() override { return StudyType_BayesOpt; }
    virtual void solve() override;

    virtual int estimatedNumberOfSteps() const override;

    QString scoreTypeString(score_type learningType) const;
    inline QStringList scoreTypeStringKeys() const { QStringList list = scoreTypeList.values(); std::sort(list.begin(), list.end()); return list; }
    inline QString scoreTypeToStringKey(score_type scoreType) const { return scoreTypeList[scoreType]; }
    inline score_type scoreTypeFromStringKey(const QString &scoreType) const { return scoreTypeList.key(scoreType); }

    QString learningTypeString(learning_type learningType) const;
    inline QStringList learningTypeStringKeys() const { QStringList list = learningTypeList.values(); std::sort(list.begin(), list.end()); return list; }
    inline QString learningTypeToStringKey(learning_type learningType) const { return learningTypeList[learningType]; }
    inline learning_type learningTypeFromStringKey(const QString &learningType) const { return learningTypeList.key(learningType); }

    QString surrogateString(const QString &surrogateType) const;
    inline QStringList surrogateStringKeys() const { return surrogateList; }
    inline QString surrogateToStringKey(const QString &surrogate) const { return surrogate; }
    inline QString surrogateFromStringKey(const QString &surrogate) const { return surrogate; }

    QString initMethodString(int method) const;
    inline QStringList initMethodStringKeys() const { QStringList list = initMethodList.values(); std::sort(list.begin(), list.end()); return list; }
    inline QString initMethodToStringKey(int method) const { return initMethodList[method]; }
    inline int initMethodFromStringKey(const QString &method) const { return initMethodList.key(method); }

protected:
    QMap<score_type, QString> scoreTypeList;
    QMap<learning_type, QString> learningTypeList;
    QMap<int, QString> initMethodList;
    QStringList surrogateList;

    virtual void setDefaultValues() override;
    virtual void setStringKeys() override;

    friend class StudyBayesOptDialog;
};

#endif // STUDY_BAYESOPT_H

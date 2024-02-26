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

#ifndef STUDY_SWEEP_H
#define STUDY_SWEEP_H

#include "util/util.h"
#include "util/enums.h"
#include "study.h"

#include "bayesopt/bayesopt.hpp"

class AGROS_LIBRARY_API StudySweep;

class SweepProblem : public bayesopt::ContinuousModel
{
public:
    SweepProblem(StudySweep *study, bayesopt::Parameters par);

    double evaluateSample(const vectord& x);
    bool checkReachability(const vectord &query) { return true; }

private:
    StudySweep *m_study;
    int m_steps;
};

class AGROS_LIBRARY_API StudySweep : public Study
{
public:
    StudySweep();

    virtual inline StudyType type() { return StudyType_Sweep; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

    QString initMethodString(int method) const;
    inline QStringList initMethodStringKeys() const { QStringList list = initMethodList.values(); std::sort(list.begin(), list.end()); return list; }
    inline QString initMethodToStringKey(int method) const { return initMethodList[method]; }
    inline int initMethodFromStringKey(const QString &method) const { return initMethodList.key(method); }

protected:
    QMap<int, QString> initMethodList;

    virtual void setDefaultValues();
    virtual void setStringKeys();

    friend class StudySweepDialog;
};

#endif // STUDY_SWEEP_H

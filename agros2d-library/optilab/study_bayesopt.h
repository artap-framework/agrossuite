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

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"
#include "study_dialog.h"

#include "bayesopt/bayesopt.hpp"

class StudyBayesOptAnalysis;

class BayesOptProblem : public bayesopt::ContinuousModel
{
public:
    BayesOptProblem(StudyBayesOptAnalysis *study, bayesopt::Parameters par);

    double evaluateSample(const vectord& x);
    bool checkReachability(const vectord &query) { return true; }

private:
    StudyBayesOptAnalysis *m_study;
};

class StudyBayesOptAnalysis : public Study
{
public:
    StudyBayesOptAnalysis();

    virtual inline StudyType type() { return StudyType_BayesOptAnalysis; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

protected:
    virtual void setDefaultValues();
    virtual void setStringKeys();

    friend class StudyBayesOptAnalysisDialog;
};

class StudyBayesOptAnalysisDialog : public StudyDialog
{
public:
    StudyBayesOptAnalysisDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyBayesOptAnalysis *study() { return dynamic_cast<StudyBayesOptAnalysis *>(m_study); }

    virtual QWidget *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtNInitSamples;
    QSpinBox *txtNIterations;
    QSpinBox *txtNIterRelearn;
    QComboBox *cmbInitMethod;
};

#endif // STUDY_BAYESOPT_H

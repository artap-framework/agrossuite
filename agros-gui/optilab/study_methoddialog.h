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

#ifndef STUDY_METHODDIALOG_H
#define STUDY_METHODDIALOG_H

#include "util/util.h"
#include "gui/other.h"

#include "optilab/study.h"
#include "optilab/study_dialog.h"

#include "optilab/study_sweep.h"
#include "optilab/study_nsga2.h"
#include "optilab/study_nlopt.h"
#include "optilab/study_bayesopt.h"
#include <optilab/study_pagmo2.h>
#include <optilab/study_model.h>

class StudySweepDialog : public StudyDialog
{
public:
    StudySweepDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudySweep *study() { return dynamic_cast<StudySweep *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtNumSamples;
    QComboBox *cmbInitMethod;
};

class StudyNSGA2Dialog : public StudyDialog
{
public:
    StudyNSGA2Dialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyNSGA2 *study() { return dynamic_cast<StudyNSGA2 *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtPopSize;
    QSpinBox *txtNGen;
    LineEditDouble *txtPCross;
    LineEditDouble *txtPMut;
    LineEditDouble *txtEtaC;
    LineEditDouble *txtEtaM;
    QRadioButton *radCrowdParameters;
    QRadioButton *radCrowdObjective;
};

class StudyPagmoDialog : public StudyDialog
{
public:
    StudyPagmoDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyPagmo *study() { return dynamic_cast<StudyPagmo *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtPopSize;
    QSpinBox *txtNGen;
    QSpinBox *txtEliteCount;
    LineEditDouble *txtCrossoverFraction;
    LineEditDouble *txtMutationRate;
    QComboBox *cmbAlgorithm;
};

class StudyNLoptDialog : public StudyDialog
{
public:
    StudyNLoptDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyNLopt *study() { return dynamic_cast<StudyNLopt *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtNIterations;
    LineEditDouble *txtXRelTol;
    LineEditDouble *txtXAbsTol;
    LineEditDouble *txtFRelTol;
    LineEditDouble *txtFAbsTol;
    QComboBox *cmbAlgorithm;
};

class StudyBayesOptDialog : public StudyDialog
{
public:
    StudyBayesOptDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyBayesOpt *study() { return dynamic_cast<StudyBayesOpt *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtNInitSamples;
    QSpinBox *txtNIterations;
    QSpinBox *txtNIterRelearn;
    QComboBox *cmbInitMethod;
    QComboBox *cmbSurrogateNameMethod;
    LineEditDouble *txtSurrogateNoise;
    QComboBox *cmbHPLearningMethod;
    QComboBox *cmbHPScoreFunction;
};

class StudyModelDialog : public StudyDialog
{
public:
    StudyModelDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyModel *study() { return dynamic_cast<StudyModel *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QLineEdit *txtName;
    QPlainTextEdit *txtDescription;
};

#endif // STUDY_METHODDIALOG_H

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

#ifndef STUDY_JMETAL_H
#define STUDY_JMETAL_H

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"
#include "study_dialog.h"

class LineEditDouble;

class StudyJMetal : public Study
{
public:
    StudyJMetal();

    virtual inline StudyType type() { return StudyType_JMetal; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

protected:
    virtual void setDefaultValues();
    virtual void setStringKeys();

private:
    friend class StudyJMetalDialog;
};

class StudyJMetalDialog : public StudyDialog
{
    Q_OBJECT
public:
    StudyJMetalDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyJMetal *study() { return dynamic_cast<StudyJMetal *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtPopulationSize;
    QSpinBox *txtArchiveSize;
    QSpinBox *txtMaxEvaluations;
    QComboBox *cmbAlgorithm;
    QLabel *lblAlgorithmCrossover;
    QLabel *lblAlgorithmMutation;
    QLabel *lblAlgorithmSelection;

    LineEditDouble *txtCrossoverProbability;
    LineEditDouble *txtCrossoverDistributionIndex;
    LineEditDouble *txtCrossoverWeightParameter;

    LineEditDouble *txtMutationProbability;
    LineEditDouble *txtMutationPerturbation;
    LineEditDouble *txtMutationDistributionIndex;

private slots:
    void currentIndexChanged(int index);
};

#endif // STUDY_JMETAL_H

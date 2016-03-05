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

#ifndef STUDY_GENETIC_H
#define STUDY_GENETIC_H

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"
#include "study_dialog.h"

#include "NSGA2.h"

class LineEditDouble;

class StudyNSGA2Analysis : public Study
{
public:
    StudyNSGA2Analysis();

    virtual inline StudyType type() { return StudyType_NSGA2; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

protected:
    virtual void setDefaultValues();
    virtual void setStringKeys();

private:
    friend class StudyNSGA2AnalysisDialog;
};

class StudyNSGA2AnalysisDialog : public StudyDialog
{
public:
    StudyNSGA2AnalysisDialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyNSGA2Analysis *study() { return dynamic_cast<StudyNSGA2Analysis *>(m_study); }

    virtual QWidget *createStudyControls();

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

#endif // STUDY_GENETIC_H

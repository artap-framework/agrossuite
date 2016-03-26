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

#ifndef STUDY_NSGA3_H
#define STUDY_NSGA3_H

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"
#include "study_dialog.h"

class LineEditDouble;

class StudyNSGA3 : public Study
{
public:
    StudyNSGA3();

    virtual inline StudyType type() { return StudyType_NSGA3; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;
    int currentPopulationSize() const;

protected:
    virtual void setDefaultValues();
    virtual void setStringKeys();

private:
    friend class StudyNSGA3Dialog;
};

class StudyNSGA3Dialog : public StudyDialog
{
public:
    StudyNSGA3Dialog(Study *study, QWidget *parent = 0);

protected:
    virtual inline StudyNSGA3 *study() { return dynamic_cast<StudyNSGA3 *>(m_study); }

    virtual QLayout *createStudyControls();

    virtual void load();
    virtual void save();

private:
    QSpinBox *txtPopSize;
    QSpinBox *txtNGen;
    LineEditDouble *txtPCross;
    // LineEditDouble *txtPMut;
    LineEditDouble *txtEtaC;
    LineEditDouble *txtEtaM;
};

#endif // STUDY_NSGA2_H

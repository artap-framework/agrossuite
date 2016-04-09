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

#ifndef STUDY_NLOPT_H
#define STUDY_NLOPT_H

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"
#include "study_dialog.h"

#include "nlopt.hpp"

class LineEditDouble;

class StudyNLopt : public Study
{
public:
    StudyNLopt();

    virtual inline StudyType type() { return StudyType_NLopt; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

    QString algorithmString(int algorithm) const;
    inline QStringList algorithmStringKeys() const { return algorithmList.values(); }
    inline QString algorithmToStringKey(int algorithm) const { return algorithmList[algorithm]; }
    inline int algorithmFromStringKey(const QString &algorithm) const { return algorithmList.key(algorithm); }

protected:
    QMap<int, QString> algorithmList;

    virtual void setDefaultValues();
    virtual void setStringKeys();

private:
    friend class StudyNLoptDialog;
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

#endif // STUDY_NLOPT_H

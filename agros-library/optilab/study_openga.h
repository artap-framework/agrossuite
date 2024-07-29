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

#ifndef STUDY_OPENGA_H
#define STUDY_OPENGA_H

#include "util/util.h"
#include "util/enums.h"
#include "study.h"

class AGROS_LIBRARY_API StudyOpenGA : public Study
{
public:
    StudyOpenGA();

    virtual inline StudyType type() { return StudyType_OpenGA; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

    static QString algorithmString(const QString &algorithm);
    static QStringList algorithmStringKeys();

protected:
    virtual void setDefaultValues();
    virtual void setStringKeys();

private:
    friend class StudyOpenGADialog;
};

#endif // STUDY_OPENGA_H

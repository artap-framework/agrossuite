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

#ifndef STUDY_LIMBO_H
#define STUDY_LIMBO_H

#include "util/util.h"
#include "util/enums.h"
#include "study.h"

class StudyLimbo : public Study
{
public:
    StudyLimbo();

    virtual inline StudyType type() { return StudyType_Limbo; }
    virtual void solve();

    virtual int estimatedNumberOfSteps() const;

    QString meanString(const QString &meanType) const;
    inline QStringList meanStringKeys() const { return meanList; }
    inline QString meanToStringKey(const QString &mean) const { return mean; }
    inline QString meanFromStringKey(const QString &mean) const { return mean; }

    QString gpString(const QString &gpType) const;
    inline QStringList gpStringKeys() const { return gpList; }
    inline QString gpToStringKey(const QString &gp) const { return gp; }
    inline QString gpFromStringKey(const QString &gp) const { return gp; }

    QString acquiString(const QString &acquiType) const;
    inline QStringList acquiStringKeys() const { return acquiList; }
    inline QString acquiToStringKey(const QString &acqui) const { return acqui; }
    inline QString acquiFromStringKey(const QString &acqui) const { return acqui; }

protected:
    QStringList meanList;
    QStringList gpList;
    QStringList acquiList;

    virtual void setDefaultValues();
    virtual void setStringKeys();

private:
    friend class StudyLimboDialog;
};

#endif // STUDY_LIMBO_H

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

#include <QWidget>

#include "util.h"
#include "util/enums.h"
#include "study.h"


// only one parameter sweep
class StudySweepAnalysis : public Study
{
public:
    StudySweepAnalysis();

    virtual inline StudyType type() { return StudyType_SweepAnalysis; }

    void setParameter(Parameter parameter) { m_parameters.append(parameter); assert(m_parameters.size() == 1); }
    virtual void solve();

    virtual void load(QJsonObject &object);
    virtual void save(QJsonObject &object);

    virtual void fillTreeView(QTreeWidget *trvComputations);

protected:
    QList<QSharedPointer<Computation> > m_computations;
};

#endif // STUDY_SWEEP_H

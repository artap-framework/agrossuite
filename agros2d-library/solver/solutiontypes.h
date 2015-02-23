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

#ifndef SOLUTIONTYPES_H
#define SOLUTIONTYPES_H

#undef signals
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/hp/dof_handler.h>
#include <deal.II/lac/vector.h>
#define signals public

#include "util.h"
#include "util/enums.h"
#include "field.h"

/// this header file should be kept small, since it is included in other header files

class FieldInfo;
class FieldSolutionID;

class AGROS_LIBRARY_API MultiArray
{
public:
    MultiArray();
    MultiArray(dealii::hp::DoFHandler<2> *doFHandler, dealii::Vector<double> &solution)
        : m_doFHandler(doFHandler), m_solution(solution) {}
    ~MultiArray();

    void clear();

    // add next component
    void append(dealii::hp::DoFHandler<2> *doFHandler, dealii::Vector<double> &solution);

    dealii::hp::DoFHandler<2> *doFHandler() { return m_doFHandler; }
    dealii::Vector<double> &solution() { return m_solution; }

    void createEmpty(int numComp);

private:
    dealii::hp::DoFHandler<2> *m_doFHandler;
    dealii::Vector<double> m_solution;
};

class FieldSolutionID
{
public:
    const FieldInfo* fieldInfo;
    int timeStep;
    int adaptivityStep;
    SolutionMode solutionMode;

    FieldSolutionID() : fieldInfo(NULL), timeStep(0), adaptivityStep(0), solutionMode(SolutionMode_Normal) {}
    FieldSolutionID(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, SolutionMode solutionMode) :
        fieldInfo(fieldInfo), timeStep(timeStep), adaptivityStep(adaptivityStep), solutionMode(solutionMode) {}

    QString toString();
};

inline bool operator<(const FieldSolutionID &sid1, const FieldSolutionID &sid2)
{
    if (sid1.fieldInfo != sid2.fieldInfo)
        return sid1.fieldInfo < sid2.fieldInfo;

    if (sid1.timeStep != sid2.timeStep)
        return sid1.timeStep < sid2.timeStep;

    if (sid1.adaptivityStep != sid2.adaptivityStep)
        return sid1.adaptivityStep < sid2.adaptivityStep;

    return sid1.solutionMode < sid2.solutionMode;
}

inline bool operator==(const FieldSolutionID &sid1, const FieldSolutionID &sid2)
{
    return !((sid1 < sid2) || (sid2 < sid1));
}

inline bool operator!=(const FieldSolutionID &sid1, const FieldSolutionID &sid2)
{
    return !(sid1 == sid2);
}

inline ostream& operator<<(ostream& output, const FieldSolutionID& id)
{
    output << "(" << id.fieldInfo->fieldId().toStdString() << ", timeStep " << id.timeStep << ", adaptStep " <<
              id.adaptivityStep << ", type "<< id.solutionMode << ")";
    return output;
}

#endif // SOLUTIONTYPES_H

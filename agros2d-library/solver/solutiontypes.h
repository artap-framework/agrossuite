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
    MultiArray(dealii::hp::DoFHandler<2> *doFHandler,
               dealii::Triangulation<2> *triangulation,
               dealii::Vector<double> &solution)
        : m_doFHandler(doFHandler), m_triangulation(triangulation), m_solution(solution) {}
    ~MultiArray();

    void clear();

    // add next component
    void append(dealii::hp::DoFHandler<2> *doFHandler, dealii::Triangulation<2> *triangulation, dealii::Vector<double> &solution);

    dealii::hp::DoFHandler<2> *doFHandler() { return m_doFHandler; }
    dealii::Triangulation<2> *triangulation() { return m_triangulation; }
    dealii::Vector<double> &solution() { return m_solution; }

private:
    dealii::hp::DoFHandler<2> *m_doFHandler;
    dealii::Triangulation<2> *m_triangulation;
    dealii::Vector<double> m_solution;
};

class FieldSolutionID
{
public:
    const FieldInfo* fieldInfo;
    int timeStep;
    int adaptivityStep;

    FieldSolutionID() : fieldInfo(NULL), timeStep(0), adaptivityStep(0) {}
    FieldSolutionID(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) :
        fieldInfo(fieldInfo), timeStep(timeStep), adaptivityStep(adaptivityStep) {}

    QString toString();
};

inline bool operator<(const FieldSolutionID &sid1, const FieldSolutionID &sid2)
{
    if (sid1.fieldInfo != sid2.fieldInfo)
        return sid1.fieldInfo < sid2.fieldInfo;

    if (sid1.timeStep != sid2.timeStep)
        return sid1.timeStep < sid2.timeStep;

    return sid1.adaptivityStep < sid2.adaptivityStep;
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
              id.adaptivityStep << ")";
    return output;
}

#endif // SOLUTIONTYPES_H

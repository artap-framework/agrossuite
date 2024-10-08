// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifndef {{ID}}_FORCE_H
#define {{ID}}_FORCE_H

#include <QObject>

#include "util/util.h"
#include "solver/field.h"

#include "solver/plugin_interface.h"

class FieldInfo;

class {{CLASS}}ForceValue : public ForceValue
{
public:
    {{CLASS}}ForceValue(Computation *computation,
                        const FieldInfo *fieldInfo,
                        int timeStep,
                        int adaptivityStep);

    virtual Point3 force(const Point3 &point, const Point3 &velocity);
    virtual bool hasForce();

private:
    MultiArray ma;
    // std::shared_ptr<dealii::Functions::FEFieldFunction<2> > localvalues;
    // dealii::DoFHandler<2>::active_cell_iterator currentCell;
};

#endif // {{ID}}_FORCE_H

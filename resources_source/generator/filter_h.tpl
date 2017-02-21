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

#ifndef {{ID}}_FILTER_H
#define {{ID}}_FILTER_H

#include <QObject>

#include "util/util.h"
#include "util/enums.h"
#include "solver/field.h"

#include "{{ID}}_interface.h"

class SceneLabelContainer;

class {{CLASS}}ViewScalarFilter : public dealii::DataPostprocessorScalar<2>
{
public:
    {{CLASS}}ViewScalarFilter(Computation *computation,
                              const FieldInfo *fieldInfo,
                              int timeStep,
                              int adaptivityStep,
                              const QString &variable,
                              PhysicFieldVariableComp physicFieldVariableComp);

    virtual ~{{CLASS}}ViewScalarFilter();    

    virtual void evaluate_scalar_field (const dealii::DataPostprocessorInputs::Scalar<2> &inputs,
                                        std::vector<dealii::Vector<double> > &computed_quantities) const;

    virtual void evaluate_vector_field (const dealii::DataPostprocessorInputs::Vector<2> &inputs,
                                        std::vector<dealii::Vector<double> > &computed_quantities) const;

protected:

private:
    Computation *m_computation;
    const FieldInfo *m_fieldInfo;
    int m_timeStep;
    int m_adaptivityStep;
    MultiArray m_ma;

    SceneLabelContainer *m_labels;
    SceneMaterial *m_noneMarker;

    QString m_variable;
    uint m_variableHash;
    PhysicFieldVariableComp m_physicFieldVariableComp;
    CoordinateType m_coordinateType;
};

#endif // {{ID}}_FILTER_H

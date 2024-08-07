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

#ifndef {{CLASS}}_INTERFACE_H
#define {{CLASS}}_INTERFACE_H

#include <QObject>
#include <QString>

#include "util/util.h"
#include "solver/plugin_interface.h"

class FieldInfo;
class Boundary;

class {{CLASS}}Interface : public QObject, public PluginInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginInterface)
    Q_PLUGIN_METADATA(IID PluginInterface_IID)

public:
{{CLASS}}Interface();
virtual ~{{CLASS}}Interface();

inline virtual QString fieldId() const { return "{{ID}}"; }
virtual QStringList couplings() const;

// solver deal
virtual SolverDeal *solverDeal(Computation *computation, const FieldInfo *fieldInfo);

// postprocessor
// filter
virtual dealii::DataPostprocessorScalar<2> *filter(Computation *computation,
                                                   const FieldInfo *fieldInfo,
                                                   int timeStep,
                                                   int adaptivityStep,
                                                   const QString &variable,
                                                   PhysicFieldVariableComp physicFieldVariableComp);

// error calculators
// virtual ErrorCalculator<double> *errorCalculator(const FieldInfo *fieldInfo,
//                                                                   const QString &calculator, CalculatedErrorType errorType);

// local values
virtual std::shared_ptr<LocalValue> localValue(Computation *computation,
                                               const FieldInfo *fieldInfo,
                                               int timeStep,
                                               int adaptivityStep,
                                               const Point &point);
// surface integrals
virtual std::shared_ptr<IntegralValue> surfaceIntegral(Computation *computation,
                                                       const FieldInfo *fieldInfo,
                                                       int timeStep,
                                                       int adaptivityStep);
// volume integrals
virtual std::shared_ptr<IntegralValue> volumeIntegral(Computation *computation,
                                                      const FieldInfo *fieldInfo,
                                                      int timeStep,
                                                      int adaptivityStep);

// force calculation
virtual std::shared_ptr<ForceValue> force(Computation *computation,
                                          const FieldInfo *fieldInfo,
                                          int timeStep,
                                          int adaptivityStep);

// localization
virtual QString localeName(const QString &name);

// description of module
virtual QString localeDescription();
};

// ***********************************************************************************************************************************

{{CPP}}

#endif // {{ID}}_INTERFACE_H

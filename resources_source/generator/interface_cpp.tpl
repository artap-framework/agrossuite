// This file is part of agros.
//
// agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with agros.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros@googlegroups.com, home page: http://hpfem.org/agros/

#include <QtPlugin>

#include "{{ID}}_interface.h"
#include "{{ID}}_weakform.h"
#include "{{ID}}_filter.h"
#include "{{ID}}_force.h"
#include "{{ID}}_localvalue.h"
#include "{{ID}}_surfaceintegral.h"
#include "{{ID}}_volumeintegral.h"

#include "solver/problem.h"

{{CLASS}}Interface::{{CLASS}}Interface() : PluginInterface()
{  
    QByteArray content = QByteArray::fromBase64({{JSON_CONTENT}});
    m_moduleJson->read(content);

    // coupling sources{{#COUPLING_SOURCE}}
    QByteArray content_{{COUPLING_SOURCE_ID}} = QByteArray::fromBase64("{{JSON_COUPLING_CONTENT}}"); 
    PluginCoupling *plugin_{{COUPLING_SOURCE_ID}} = new PluginCoupling();
    plugin_{{COUPLING_SOURCE_ID}}->read(content_{{COUPLING_SOURCE_ID}});
    m_couplingsJson["{{COUPLING_SOURCE_ID}}"] = plugin_{{COUPLING_SOURCE_ID}};
    {{/COUPLING_SOURCE}}   
}

{{CLASS}}Interface::~{{CLASS}}Interface()
{
}

QStringList {{CLASS}}Interface::couplings() const
{
    QStringList out;
    // coupling sources{{#COUPLING_SOURCE}}
    out.append("{{COUPLING_SOURCE_ID}}");{{/COUPLING_SOURCE}}

    return out;
}

SolverDeal *{{CLASS}}Interface::solverDeal(Computation *computation, const FieldInfo *fieldInfo)
{
    return new SolverDeal{{CLASS}}(computation, fieldInfo);
}

dealii::DataPostprocessorScalar<2> *{{CLASS}}Interface::filter(Computation *computation,
                                                               const FieldInfo *fieldInfo,
                                                               int timeStep,
                                                               int adaptivityStep,
                                                               const QString &variable,
                                                               PhysicFieldVariableComp physicFieldVariableComp)
{
    return new {{CLASS}}ViewScalarFilter(computation, fieldInfo, timeStep, adaptivityStep, variable, physicFieldVariableComp);
}

std::shared_ptr<LocalValue> {{CLASS}}Interface::localValue(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, const Point &point)
{
    return std::shared_ptr<LocalValue>(new {{CLASS}}LocalValue(computation, fieldInfo, timeStep, adaptivityStep, point));
}

std::shared_ptr<IntegralValue> {{CLASS}}Interface::surfaceIntegral(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    return std::shared_ptr<IntegralValue>(new {{CLASS}}SurfaceIntegral(computation, fieldInfo, timeStep, adaptivityStep));
}

std::shared_ptr<IntegralValue> {{CLASS}}Interface::volumeIntegral(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    return std::shared_ptr<IntegralValue>(new {{CLASS}}VolumeIntegral(computation, fieldInfo, timeStep, adaptivityStep));
}

std::shared_ptr<ForceValue> {{CLASS}}Interface::force(Computation *computation, const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    return std::shared_ptr<ForceValue>(new {{CLASS}}ForceValue(computation, fieldInfo, timeStep, adaptivityStep));
}

QString {{CLASS}}Interface::localeName(const QString &name)
{
    {{#NAMES}}
    if (name == "{{NAME}}")
        return QObject::tr("{{NAME}}");
    {{/NAMES}}
    return name;
}

QString {{CLASS}}Interface::localeDescription()
{
    return QObject::tr("{{DESCRIPTION}}");
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros_plugin_{{ID}}, {{CLASS}}Interface)
#endif

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

#include <QtPlugin>

#include "{{ID}}_interface.h"
#include "{{ID}}_weakform.h"
#include "{{ID}}_filter.h"
#include "{{ID}}_force.h"
#include "{{ID}}_localvalue.h"
#include "{{ID}}_surfaceintegral.h"
#include "{{ID}}_volumeintegral.h"
#include "{{ID}}_errorcalculator.h"

#include "util.h"
#include "util/global.h"
#include "util/constants.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

#include "../../resources_source/classes/module_xml.h"

static XMLModule::module *module_module = NULL;

{{CLASS}}Interface::{{CLASS}}Interface() : PluginInterface()
{    
    // xml module description
    if (!module_module)
    {
        try
        {
            std::unique_ptr<XMLModule::module> module_xsd = XMLModule::module_((datadir() + MODULEROOT + QDir::separator() + "{{ID}}.xml").toStdString(),
                                                                             xml_schema::flags::dont_validate & xml_schema::flags::dont_initialize);
            module_module = module_xsd.release();
        }
        catch (const xml_schema::expected_element& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
            qDebug() << str;
        }
        catch (const xml_schema::expected_attribute& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
            qDebug() << str;
        }
        catch (const xml_schema::unexpected_element& e)
        {
            QString str = QString("%1: %2 instead of %3").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.encountered_name())).arg(QString::fromStdString(e.expected_name()));
            qDebug() << str;
        }
        catch (const xml_schema::unexpected_enumerator& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.enumerator()));
            qDebug() << str;
        }
        catch (const xml_schema::expected_text_content& e)
        {
            QString str = QString("%1").arg(QString::fromStdString(e.what()));
            qDebug() << str;
        }
        catch (const xml_schema::parsing& e)
        {
            QString str = QString("%1").arg(QString::fromStdString(e.what()));
            qDebug() << str;
            xml_schema::diagnostics diagnostic = e.diagnostics();
            for(int i = 0; i < diagnostic.size(); i++)
            {
                xml_schema::error err = diagnostic.at(i);
                qDebug() << QString("%1, position %2:%3, %4").arg(QString::fromStdString(err.id())).arg(err.line()).arg(err.column()).arg(QString::fromStdString(err.message()));
            }
        }
        catch (const xml_schema::exception& e)
        {
            qDebug() << QString::fromStdString(e.what());
        }
    }
    m_module = &module_module->field().get();
}

{{CLASS}}Interface::~{{CLASS}}Interface()
{
}

SolverDeal *{{CLASS}}Interface::solverDeal(const FieldInfo *fieldInfo)
{
    return new SolverDeal{{CLASS}}(fieldInfo);
}

/*
ErrorCalculator<double> *{{CLASS}}Interface::errorCalculator(const FieldInfo *fieldInfo,
                                                                               const QString &calculator, CalculatedErrorType errorType)
{
    return new {{CLASS}}ErrorCalculator<double>(fieldInfo, calculator, errorType);
}
*/

std::shared_ptr<dealii::DataPostprocessorScalar<2> > {{CLASS}}Interface::filter(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep,
                                                                                MultiArray *ma,
                                                                                const QString &variable,
                                                                                PhysicFieldVariableComp physicFieldVariableComp)
{
    return std::shared_ptr<dealii::DataPostprocessorScalar<2> >(new {{CLASS}}ViewScalarFilter(fieldInfo, timeStep, adaptivityStep, ma, variable, physicFieldVariableComp));
}

std::shared_ptr<LocalValue>{{CLASS}}Interface::localValue(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep, const Point &point)
{
    return std::shared_ptr<LocalValue>(new {{CLASS}}LocalValue(fieldInfo, timeStep, adaptivityStep, point));
}

std::shared_ptr<IntegralValue> {{CLASS}}Interface::surfaceIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    return std::shared_ptr<IntegralValue>(new {{CLASS}}SurfaceIntegral(fieldInfo, timeStep, adaptivityStep));
}

std::shared_ptr<IntegralValue> {{CLASS}}Interface::volumeIntegral(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    return std::shared_ptr<IntegralValue>(new {{CLASS}}VolumeIntegral(fieldInfo, timeStep, adaptivityStep));
}

std::shared_ptr<ForceValue> {{CLASS}}Interface::force(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep)
{
    return std::shared_ptr<ForceValue>(new {{CLASS}}ForceValue(fieldInfo, timeStep, adaptivityStep));
}

QString {{CLASS}}Interface::localeName(const QString &name)
{
    {{#NAMES}}
    if (name == "{{NAME}}")
        return tr("{{NAME}}");
    {{/NAMES}}
    return name;
}

QString {{CLASS}}Interface::localeDescription()
{
    return tr("{{DESCRIPTION}}");
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(agros2d_plugin_{{ID}}, {{CLASS}}Interface)
#endif

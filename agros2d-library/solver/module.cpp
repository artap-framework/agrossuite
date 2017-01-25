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

#include "module.h"
#include "weak_form.h"
#include "field.h"
#include "problem.h"
#include "logview.h"

#include "util/util.h"
#include "util/global.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenelabel.h"
#include "sceneedge.h"
#include "solver/solver.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"
#include "solver/problem_config.h"

#include "util/constants.h"

#include "../../resources_source/classes/module_xml.h"

QMap<QString, QString> Module::availableModules()
{
    static QMap<QString, QString> modules;

    // read modules
    if (modules.isEmpty())
    {
        QDir dir(datadir() + MODULEROOT);

        QStringList filter;
        filter << "*.xml";
        QStringList list = dir.entryList(filter);

        foreach (QString filename, list)
        {
            try
            {
                // todo: find a way to validate if required. If validated here, sensible error messages will be obtained
                bool validateAtTheBeginning = false;
                ::xml_schema::flags parsing_flags = xml_schema::flags::dont_validate;
                if(validateAtTheBeginning)
                {
                    parsing_flags = 0;
                    qDebug() << "Warning: Validating all XML files. This is time-consuming and should be switched off in module.cpp for release. Set validateAtTheBeginning = false.";
                }
                std::unique_ptr<XMLModule::module> module_xsd(XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + filename).toStdString(), parsing_flags));

                XMLModule::module *mod = module_xsd.get();
                assert(mod->field().present());
                XMLModule::field *field = &mod->field().get();

                // module name
                modules[filename.left(filename.size() - 4)] = QString::fromStdString(field->general_field().name());
            }
            catch (const xml_schema::expected_element& e)
            {
                QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::expected_attribute& e)
            {
                QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::unexpected_element& e)
            {
                QString str = QString("%1: %2 instead of %3").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.encountered_name())).arg(QString::fromStdString(e.expected_name()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::unexpected_enumerator& e)
            {
                QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.enumerator()));
                qDebug() << str;
                throw AgrosException(str);
            }
            catch (const xml_schema::expected_text_content& e)
            {
                QString str = QString("%1").arg(QString::fromStdString(e.what()));
                qDebug() << str;
                throw AgrosException(str);
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
                throw AgrosException(str);
            }
            catch (const xml_schema::exception& e)
            {
                qDebug() << QString("Unknow parser exception: %1").arg(QString::fromStdString(e.what()));
                throw AgrosException(QString::fromStdString(e.what()));
            }
        }
    }

    return modules;
}

template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTemplates(SectionWithTemplates *section)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    // weakform
    for (unsigned int i = 0; i < section->matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = section->matrix_form().at(i);
        assert(form.i().present() && form.j().present() && form.planar().present() && form.axi().present());
        SymFlag symPlanar = SymFlag_NONSYM;
        SymFlag symAxi = SymFlag_NONSYM;
        if(form.symmetric().present())
        {
            symPlanar = (SymFlag) form.symmetric().get();
            symAxi = (SymFlag) form.symmetric().get();
        }
        if(form.symmetric_planar().present())
        {
            symPlanar = (SymFlag) form.symmetric_planar().get();
        }
        if(form.symmetric_axi().present())
        {
            symAxi = (SymFlag) form.symmetric_axi().get();
        }
        FormInfo formInfo(QString::fromStdString(form.id()),
                          form.i().get(),
                          form.j().get(),
                          symPlanar,
                          symAxi);
        formInfo.condition = form.condition().present() ? QString::fromStdString(form.condition().get()) : "";
        formInfo.expr_planar = QString::fromStdString(form.planar().get());
        formInfo.expr_axi = QString::fromStdString(form.axi().get());
        weakForms.append(formInfo);
    }

    return weakForms;
}

template <typename SectionWithTemplates>
QList<FormInfo> wfVectorTemplates(SectionWithTemplates *section)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    for (unsigned int i = 0; i < section->vector_form().size(); i++)
    {
        XMLModule::vector_form form = section->vector_form().at(i);
        assert(form.i().present() && form.j().present() && form.planar().present() && form.axi().present());
        FormInfo formInfo(QString::fromStdString(form.id()),
                          form.i().get(),
                          form.j().get());
        formInfo.condition = form.condition().present() ? QString::fromStdString(form.condition().get()) : "";
        formInfo.expr_planar = QString::fromStdString(form.planar().get());
        formInfo.expr_axi = QString::fromStdString(form.axi().get());
        weakForms.append(formInfo);
    }

    return weakForms;
}

QList<FormInfo> wfEssentialTemplates(XMLModule::surface *surface)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    for (unsigned int i = 0; i < surface->essential_form().size(); i++)
    {
        XMLModule::essential_form form = surface->essential_form().at(i);
        assert(form.i().present() && form.planar().present() && form.axi().present());
        FormInfo formInfo(QString::fromStdString(form.id()),
                          form.i().get());
        formInfo.condition = form.condition().present() ? QString::fromStdString(form.condition().get()) : "";
        formInfo.expr_planar = QString::fromStdString(form.planar().get());
        formInfo.expr_axi = QString::fromStdString(form.axi().get());
        weakForms.append(formInfo);
    }

    return weakForms;
}

XMLModule::linearity_option findLinearityOption(XMLModule::volume *volume, AnalysisType analysisType, LinearityType linearityType)
{
    for (unsigned int i = 0; i < volume->weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = volume->weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            for(unsigned int i = 0; i < wf.linearity_option().size(); i++)
            {
                XMLModule::linearity_option lo = wf.linearity_option().at(i);
                if(lo.type() == linearityTypeToStringKey(linearityType).toStdString())
                {
                    return lo;
                }
            }
        }
    }

    assert(0);
    return XMLModule::linearity_option("");
}

XMLModule::linearity_option findLinearityOption(XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    for(unsigned int i = 0; i < boundary->linearity_option().size(); i++)
    {
        XMLModule::linearity_option lo = boundary->linearity_option().at(i);
        if(lo.type() == linearityTypeToStringKey(linearityType).toStdString())
        {
            return lo;
        }
    }

    assert(0);
    return XMLModule::linearity_option("");
}

template <typename SectionWithElements>
QList<FormInfo> wfMatrixElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = lo.matrix_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

template <typename SectionWithElements>
QList<FormInfo> wfMatrixTransientElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.matrix_transient_form().size(); i++)
    {
        XMLModule::matrix_transient_form form = lo.matrix_transient_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

template <typename SectionWithElements>
QList<FormInfo> wfVectorElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.vector_form().size(); i++)
    {
        XMLModule::vector_form form = lo.vector_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        if(form.variant().present())
            formInfo.variant = weakFormVariantFromStringKey(QString::fromStdString(form.variant().get()));
        if(form.coefficient().present())
            formInfo.coefficient = QString::fromStdString(form.coefficient().get()).toDouble();
        weakForms.append(formInfo);
    }

    return weakForms;
}

QList<FormInfo> wfEssentialElements(XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    // essential
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(boundary, analysisType, linearityType);

    for (unsigned int i = 0; i < lo.essential_form().size(); i++)
    {
        XMLModule::essential_form form = lo.essential_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

QList<FormInfo> Module::wfMatrixVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> matrixElements = wfMatrixElements(&module->volume(), analysisType, linearityType);

    return generateSeparated(matrixElements, matrixTemplates);
}

QList<FormInfo> Module::wfVectorVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> vectorTemplates = wfVectorTemplates(&module->volume());
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> vectorElements = wfVectorElements(&module->volume(), analysisType, linearityType);

    return generateSeparated(vectorElements, vectorTemplates, matrixTemplates);
}

QList<FormInfo> Module::wfMatrixTransientVolumeSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> templates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> matrixElements = wfMatrixTransientElements(&module->volume(), analysisType, linearityType);

    return generateSeparated(matrixElements, templates);
}

QList<FormInfo> Module::wfMatrixSurface(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(surface);
    QList<FormInfo> matrixElements = wfMatrixElements<XMLModule::boundary>(boundary, analysisType, linearityType);
    return generateSeparated(matrixElements, matrixTemplates);
}

QList<FormInfo> Module::wfVectorSurface(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> vectorTemplates = wfVectorTemplates(surface);
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(surface);
    QList<FormInfo> vectorElements = wfVectorElements<XMLModule::boundary>(boundary, analysisType, linearityType);
    return generateSeparated(vectorElements, vectorTemplates, matrixTemplates);
}

QList<FormInfo> Module::essential(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> essentialTemplates = wfEssentialTemplates(surface);
    QList<FormInfo> essentialElements = wfEssentialElements(boundary, analysisType, linearityType);
    return generateSeparated(essentialElements, essentialTemplates);
}

// ***********************************************************************************************

Module::LocalVariable::LocalVariable(const FieldInfo *fieldInfo, XMLModule::localvariable lv,
                                     CoordinateType coordinateType, AnalysisType analysisType)
{
    m_id = QString::fromStdString(lv.id());
    m_name = fieldInfo->plugin()->localeName(QString::fromStdString(lv.name()));
    m_shortname = QString::fromStdString(lv.shortname());
    m_shortnameHtml = (lv.shortname_html().present()) ? QString::fromStdString(lv.shortname_html().get()) : m_shortname;
    m_unit = QString::fromStdString(lv.unit());
    m_unitHtml = (lv.unit_html().present()) ? QString::fromStdString(lv.unit_html().get()) : m_unit;

    m_isScalar = (lv.type() == "scalar");

    for (unsigned int i = 0; i < lv.expression().size(); i++)
    {
        XMLModule::expression exp = lv.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            if (coordinateType == CoordinateType_Planar)
                m_expression = Expression(m_isScalar ? QString::fromStdString(exp.planar().get()) : "",
                                          m_isScalar ? "" : QString::fromStdString(exp.planar_x().get()),
                                          m_isScalar ? "" : QString::fromStdString(exp.planar_y().get()));
            else
                m_expression = Expression(m_isScalar ? QString::fromStdString(exp.axi().get()) : "",
                                          m_isScalar ? "" : QString::fromStdString(exp.axi_r().get()),
                                          m_isScalar ? "" : QString::fromStdString(exp.axi_z().get()));
        }
    }
}

// ***********************************************************************************************

Module::MaterialTypeVariable::MaterialTypeVariable(XMLModule::quantity quant)
    : m_defaultValue(0), m_expressionNonlinear(""), m_isTimeDep(false)
{
    m_id = QString::fromStdString(quant.id());

    if (quant.shortname().present())
        m_shortname = QString::fromStdString(quant.shortname().get());

    if (quant.default_().present())
        m_defaultValue = quant.default_().get();

    if (quant.is_bool().present())
        m_isBool = (quant.is_bool().get() != 0);
    else
        m_isBool = false;

    if (quant.only_if().present())
        m_onlyIf = QString::fromStdString(quant.only_if().get());

    if (quant.only_if_not().present())
        m_onlyIfNot = QString::fromStdString(quant.only_if_not().get());

    if (quant.is_source().present())
        m_isSource = (quant.is_source().get() != 0);
}

// ***********************************************************************************************

Module::BoundaryType::BoundaryType(const FieldInfo *fieldInfo,
                                   QList<BoundaryTypeVariable> boundary_type_variables,
                                   XMLModule::boundary bdy)
{
    m_id = QString::fromStdString(bdy.id());
    m_name = fieldInfo->plugin()->localeName(QString::fromStdString(bdy.name()));
    m_equation = QString::fromStdString(bdy.equation());

    // variables
    for (unsigned int i = 0; i < bdy.quantity().size(); i++)
    {
        XMLModule::quantity qty = bdy.quantity().at(i);

        foreach (Module::BoundaryTypeVariable old, boundary_type_variables)
        {
            if (old.id().toStdString() == qty.id())
            {
                bool isTimeDep = false;
                bool isSpaceDep = false;

                if (qty.dependence().present())
                {
                    if (QString::fromStdString(qty.dependence().get()) == "time")
                    {
                        isTimeDep = true;
                    }
                    else if (QString::fromStdString(qty.dependence().get()) == "space")
                    {
                        isSpaceDep = true;
                    }
                    else if (QString::fromStdString(qty.dependence().get()) == "time-space")
                    {
                        isTimeDep = true;
                        isSpaceDep = true;
                    }
                }

                m_variables.append(Module::BoundaryTypeVariable(old.id(), old.shortname(), isTimeDep, isSpaceDep));
            }
        }
    }

    m_wfMatrix = Module::wfMatrixSurface(&fieldInfo->plugin()->module()->surface(), &bdy, fieldInfo->analysisType(), fieldInfo->linearityType());
    m_wfVector = Module::wfVectorSurface(&fieldInfo->plugin()->module()->surface(), &bdy, fieldInfo->analysisType(), fieldInfo->linearityType());
    m_essential = Module::essential(&fieldInfo->plugin()->module()->surface(), &bdy, fieldInfo->analysisType(), fieldInfo->linearityType());
}

Module::BoundaryTypeVariable::BoundaryTypeVariable(XMLModule::quantity quant)
{
    m_id = QString::fromStdString(quant.id());
    if (quant.shortname().present())
        m_shortname = QString::fromStdString(quant.shortname().get());
    if (quant.default_().present())
        m_defaultValue = quant.default_().get();
    else
        m_defaultValue = 0.0;
}

Module::BoundaryType::~BoundaryType()
{
    m_essential.clear();
    m_variables.clear();
    m_wfMatrix.clear();
    m_wfVector.clear();
    m_essential.clear();
}


// ***********************************************************************************************

// dialog row UI
Module::DialogRow::DialogRow(const FieldInfo *fieldInfo, XMLModule::quantity qty)
{
    m_id = QString::fromStdString(qty.id());
    m_name = (qty.name().present()) ? fieldInfo->plugin()->localeName(QString::fromStdString(qty.name().get())) : "";

    m_shortname = (qty.shortname().present()) ? QString::fromStdString(qty.shortname().get()) : "";
    m_shortnameHtml = (qty.shortname_html().present()) ? QString::fromStdString(qty.shortname_html().get()) : "";
    m_shortnameDependence = (qty.shortname_dependence().present()) ? QString::fromStdString(qty.shortname_dependence().get()) : "";
    m_shortnameDependenceHtml = (qty.shortname_dependence_html().present()) ? QString::fromStdString(qty.shortname_dependence_html().get()) : "";

    m_unit = (qty.unit().present()) ? QString::fromStdString(qty.unit().get()) : "";
    m_unitHtml = (qty.unit_html().present()) ? QString::fromStdString(qty.unit_html().get()) : "";

    m_defaultValue = (qty.default_().present()) ? qty.default_().get() : 0.0;
    m_condition = (qty.condition().present()) ? QString::fromStdString(qty.condition().get()) : "";
}

// dialog UI
Module::DialogUI::DialogUI(const FieldInfo *fieldInfo, XMLModule::gui ui)
{
    for (unsigned int i = 0; i < ui.group().size(); i++)
    {
        XMLModule::group grp = ui.group().at(i);

        // group name
        QString name = (grp.name().present()) ? fieldInfo->plugin()->localeName(QString::fromStdString(grp.name().get())) : "";

        QList<Module::DialogRow> materials;
        for (unsigned int i = 0; i < grp.quantity().size(); i++)
        {
            XMLModule::quantity quant = grp.quantity().at(i);

            // append material
            materials.append(Module::DialogRow(fieldInfo, quant));
        }

        m_groups[name] = materials;
    }
}

Module::DialogRow Module::DialogUI::dialogRow(const QString &id)
{
    foreach (QList<Module::DialogRow> rows, m_groups)
        foreach (Module::DialogRow row, rows)
            if (row.id() == id)
                return row;

    assert(0);
}

void Module::DialogUI::clear()
{
    m_groups.clear();
}

// ***********************************************************************************************

void findVolumeLinearityOption(XMLModule::linearity_option& option, XMLModule::field *module, AnalysisType analysisType, LinearityType linearityType)

{
    for (unsigned int i = 0; i < module->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = module->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            for(unsigned int i = 0; i < wf.linearity_option().size(); i++)
            {
                XMLModule::linearity_option lo = wf.linearity_option().at(i);
                if(lo.type() == linearityTypeToStringKey(linearityType).toStdString())
                {
                    option = lo;
                }
            }
        }
    }
}

void Module::volumeQuantityProperties(XMLModule::field *module, QMap<QString, int> &quantityOrder, QMap<QString, bool> &quantityIsNonlin, QMap<QString, int> &functionOrder)
{
    int nextIndex = 0;
    foreach(XMLModule::quantity quantity, module->volume().quantity())
    {
        QString quantityId = QString::fromStdString(quantity.id());
        quantityOrder[quantityId] = nextIndex;
        nextIndex++;
        quantityIsNonlin[quantityId] = false;
        foreach(XMLModule::weakform_volume weakform, module->volume().weakforms_volume().weakform_volume())
        {
            foreach(XMLModule::quantity quantityInAnalysis, weakform.quantity())
            {
                if(quantity.id() == quantityInAnalysis.id())
                {
                    if(quantityInAnalysis.nonlinearity_axi().present() || quantityInAnalysis.nonlinearity_planar().present())
                    {
                        quantityIsNonlin[quantityId] = true;
                    }
                }
            }
        }

        // if the quantity is nonlinear, we have to reserve space for its derivative as well
        if(quantityIsNonlin[quantityId])
            nextIndex++;
    }

    foreach(XMLModule::function function, module->volume().function())
    {
        QString functionID = QString::fromStdString(function.id());
        functionOrder[functionID] = nextIndex;
        nextIndex++;
    }
}

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

#include "generator.h"
#include "generator_coupling.h"
#include "parser.h"

#include <QDir>

#include "util/constants.h"
#include "solver/weak_form.h"
#include "solver/module.h"
#include "solver/coupling.h"
#include "solver/plugin_interface.h"

#include "parser/lex.h"

#include "util/constants.h"

#include "generator.h"

XMLModule::linearity_option findLinearityOption(XMLModule::volume *volume, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
{
    for (unsigned int i = 0; i < volume->weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = volume->weakforms_volume().weakform_volume().at(i);

        if ((wf.sourceanalysis().get() == analysisTypeToStringKey(analysisTypeSource).toStdString()) &&
                (wf.analysistype() == analysisTypeToStringKey(analysisTypeTarget).toStdString()) &&
                (wf.couplingtype().get() == couplingTypeToStringKey(couplingType).toStdString()))
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

    return XMLModule::linearity_option("");
}

// weak forms
template <typename SectionWithElements>
QList<FormInfo> wfMatrixCouplingElements(SectionWithElements *section, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
{
    // matrix weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisTypeSource, analysisTypeTarget, couplingType, linearityType);

    for (unsigned int i = 0; i < lo.matrix_form().size(); i++)
    {
        XMLModule::matrix_form form = lo.matrix_form().at(i);
        FormInfo formInfo(QString::fromStdString(form.id()));
        weakForms.append(formInfo);
    }

    return weakForms;
}

template <typename SectionWithElements>
QList<FormInfo> wfVectorCouplingElements(SectionWithElements *section, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
{
    // vector weakforms
    QList<FormInfo> weakForms;
    XMLModule::linearity_option lo = findLinearityOption(section, analysisTypeSource, analysisTypeTarget, couplingType, linearityType);

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


template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixCouplingTemplates(SectionWithTemplates *section)
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
QList<FormInfo> wfVectorCouplingTemplates(SectionWithTemplates *section)
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

QList<FormInfo> wfMatrixVolumeCouplingSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType)
{
    QList<FormInfo> templates = wfMatrixCouplingTemplates(volume);
    QList<FormInfo> elements = wfMatrixCouplingElements(volume, sourceAnalysis, targetAnalysis, couplingType, linearityType);

    return wfGenerateSeparated(elements, templates);
}

QList<FormInfo> wfVectorVolumeCouplingSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType)
{
    QList<FormInfo> templatesVector = wfVectorCouplingTemplates(volume);
    QList<FormInfo> templatesMatrix = wfMatrixCouplingTemplates(volume);
    QList<FormInfo> elements = wfVectorCouplingElements(volume, sourceAnalysis, targetAnalysis, couplingType, linearityType);

    return wfGenerateSeparated(elements, templatesVector, templatesMatrix);
}

Agros2DGeneratorCoupling::Agros2DGeneratorCoupling(const QString &couplingId) : m_output(nullptr)
{
    QString iD = couplingId;
    QDir root(QCoreApplication::applicationDirPath());
    root.mkpath(QString("%1/%2").arg(GENERATOR_PLUGINROOT).arg(iD));

    coupling_xsd = XMLModule::module_(compatibleFilename(datadir() + COUPLINGROOT + "/" + couplingId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    XMLModule::module *mod = coupling_xsd.get();
    assert(mod->coupling().present());
    m_coupling = &mod->coupling().get();

    QString sourceModuleId = QString::fromStdString(m_coupling->general_coupling().modules().source().id().c_str());
    QString targetModuleId = QString::fromStdString(m_coupling->general_coupling().modules().target().id().c_str());

    m_source_module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + sourceModuleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    mod = m_source_module_xsd.get();
    assert(mod->field().present());
    m_sourceModule = &mod->field().get();

    m_target_module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + targetModuleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    mod = m_target_module_xsd.get();
    assert(mod->field().present());
    m_targetModule = &mod->field().get();

    QDir().mkdir(GENERATOR_PLUGINROOT + "/" + iD);

    // variables
    foreach (XMLModule::quantity quantity, m_sourceModule->volume().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_sourceVariables.insert(iD, shortName);
    }

    foreach (XMLModule::quantity quantity, m_targetModule->volume().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_targetVariables.insert(iD, shortName);
    }

    volumeQuantityModuleProperties(m_targetModule, quantityOrdering, quantityIsNonlinear, functionOrdering);
    volumeQuantityModuleProperties(m_sourceModule, sourceQuantityOrdering, sourceQuantityIsNonlinear, sourceFunctionOrdering);
}

void Agros2DGeneratorCoupling::generatePluginProjectFile()
{
    QString id = (QString::fromStdString(m_coupling->general_coupling().id().c_str()));

    qDebug() << (QString("generating project file").toLatin1());

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_CMakeLists_txt.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/CMakeLists.txt").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorCoupling::generatePluginInterfaceFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());

    qDebug() << (QString("generating interface file").toLatin1());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_interface_h.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();


    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/coupling_interface_cpp.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);
    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorCoupling::generatePluginWeakFormFiles()
{
    qDebug() << (QString("Coupling: %1.").arg(QString::fromStdString(m_coupling->general_coupling().id())).toLatin1());

    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorCoupling::generatePluginWeakFormHeaderFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());

    qDebug() << (QString("generating weakform header file").toLatin1());

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_h.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}


void Agros2DGeneratorCoupling::generatePluginWeakFormSourceFiles()
{
    QString id = QString::fromStdString(m_coupling->general_coupling().id());
    QStringList modules = QString::fromStdString(m_coupling->general_coupling().id()).split("-");

    qDebug() << (QString("generating weakform source file").toLatin1());

    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_cpp.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}


template <typename WeakForm>
void Agros2DGeneratorCoupling::generateForm(FormInfo formInfo, LinearityType linearityType, ctemplate::TemplateDictionary &output, WeakForm weakform, QString weakFormType)
{
    foreach (CoordinateType coordinateType, PluginFunctions::coordinateTypeList())
    {
        QString expression = (coordinateType == CoordinateType_Planar ? formInfo.expr_planar : formInfo.expr_axi);
        if(expression != "")
        {
            ctemplate::TemplateDictionary *field;
            field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

            QString id = (QString::fromStdString(m_coupling->general_coupling().id().c_str())).replace("-", "_");

            // source files
            QString functionName = QString("%1_%2_%3_%4_%5_%6_%7_%8_%9_%10").
                    arg(weakFormType.toLower()).
                    arg(id).
                    arg(QString::fromStdString(weakform.sourceanalysis().get())).
                    arg(QString::fromStdString(weakform.analysistype())).
                    arg(coordinateTypeToStringKey(coordinateType)).
                    arg(linearityTypeToStringKey(linearityType)).
                    arg(formInfo.id).
                    arg(QString::fromStdString(weakform.couplingtype().get())).
                    arg(QString::number(formInfo.i)).
                    arg(QString::number(formInfo.j));

            CouplingType couplingType = PluginFunctions::couplingTypeFromString(QString::fromStdString(weakform.couplingtype().get()));

            field->SetValue("COLUMN_INDEX", QString::number(formInfo.j).toStdString());
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
            field->SetValue("COORDINATE_TYPE", PluginFunctions::coordinateTypeStringEnum(coordinateType).toStdString());
            field->SetValue("LINEARITY_TYPE", PluginFunctions::linearityTypeStringEnum(linearityType).toStdString());
            field->SetValue("SOURCE_ANALYSIS_TYPE", PluginFunctions::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get()))).toStdString());
            field->SetValue("TARGET_ANALYSIS_TYPE", PluginFunctions::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());
            field->SetValue("ROW_INDEX", QString::number(formInfo.i).toStdString());
            field->SetValue("MODULE_ID", id.toStdString());
            field->SetValue("WEAKFORM_ID", formInfo.id.toStdString());
            field->SetValue("COUPLING_TYPE", PluginFunctions::couplingTypeToString(weakform.couplingtype().get().c_str()).toStdString());

            ParserModuleInfo pmiSource(*m_sourceModule,
                                       analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get())),
                                       coordinateType, linearityType, false);

            ParserModuleInfo pmi(*m_targetModule,
                                 analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())),
                                 coordinateType, linearityType, false);

            QString exprCpp = Parser::parseCouplingWeakFormExpression(pmiSource, pmi, expression);
            field->SetValue("EXPRESSION", exprCpp.toStdString());

            // add weakform
            field = output.AddSectionDictionary("SOURCE");
            field->SetValue("FUNCTION_NAME", functionName.toStdString());
        }
    }
}

void Agros2DGeneratorCoupling::generateWeakForms(ctemplate::TemplateDictionary &output)
{
    //this->m_docString = "";
    foreach(XMLModule::weakform_volume weakform, m_coupling->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType sourceAnalysis = analysisTypeFromStringKey(QString::fromStdString(weakform.sourceanalysis().get().c_str()));
        AnalysisType targetAnalysis = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        CouplingType couplingType = couplingTypeFromStringKey(QString::fromStdString(weakform.couplingtype().get().c_str()));

        qDebug() << "chci" << QString::fromStdString(weakform.sourceanalysis().get().c_str()) << ", " <<  QString::fromStdString(weakform.analysistype().c_str()) << ", " << QString::fromStdString(weakform.couplingtype().get().c_str());

        foreach(XMLModule::linearity_option option, weakform.linearity_option())
        {
            LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

            // generate individual forms
            QList<FormInfo> matrixForms = wfMatrixVolumeCouplingSeparated(&m_coupling->volume(), sourceAnalysis, targetAnalysis, couplingType, linearityType);
            foreach(FormInfo formInfo, matrixForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_MATRIX");
            }

            // generate individual forms
            QList<FormInfo> vectorForms = wfVectorVolumeCouplingSeparated(&m_coupling->volume(), sourceAnalysis, targetAnalysis, couplingType, linearityType);
            foreach(FormInfo formInfo, vectorForms)
            {
                generateForm(formInfo, linearityType, output, weakform, "VOLUME_VECTOR");
            }
        }
    }

}


void Agros2DGeneratorCoupling::prepareWeakFormsOutput()
{
    qDebug() << (QString("parsing weak forms").toLatin1());
    assert(! m_output);
    m_output = new ctemplate::TemplateDictionary("output");

    QString id = QString::fromStdString(m_coupling->general_coupling().id());
    QStringList modules = QString::fromStdString(m_coupling->general_coupling().id()).split("-");
    m_output->SetValue("ID", id.toStdString());
    m_output->SetValue("CLASS", (modules[0].left(1).toUpper() + modules[0].right(modules[0].length() - 1) +
            modules[1].left(1).toUpper() + modules[1].right(modules[1].length() - 1)).toStdString());

    //comment on beginning of weakform.cpp, may be removed
    ctemplate::TemplateDictionary *field;
    foreach(QString quantID, this->quantityOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", quantID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID]).toStdString());
        field->SetValue("OFFSET", "offset.quant");
        if(quantityIsNonlinear[quantID])
        {
            field = m_output->AddSectionDictionary("QUANTITY_INFO");
            field->SetValue("QUANT_ID", QString("derivative %1").arg(quantID).toStdString());
            field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID] + 1).toStdString());
            field->SetValue("OFFSET", "offset.quant");
        }
    }
    foreach(QString funcID, this->functionOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", funcID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(functionOrdering[funcID]).toStdString());
        field->SetValue("OFFSET", "offset.quant");
    }

    foreach(QString quantID, this->sourceQuantityOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", quantID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(sourceQuantityOrdering[quantID]).toStdString());
        field->SetValue("OFFSET", "offset.sourceQuant");
        if(sourceQuantityIsNonlinear[quantID])
        {
            field = m_output->AddSectionDictionary("QUANTITY_INFO");
            field->SetValue("QUANT_ID", QString("derivative %1").arg(quantID).toStdString());
            field->SetValue("INDEX", QString("%1").arg(sourceQuantityOrdering[quantID] + 1).toStdString());
            field->SetValue("OFFSET", "offset.sourceQuant");
        }
    }
    foreach(QString funcID, this->sourceFunctionOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", funcID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(sourceFunctionOrdering[funcID]).toStdString());
        field->SetValue("OFFSET", "offset.sourceQuant");
    }

    generateWeakForms(*m_output);
}

void Agros2DGeneratorCoupling::deleteWeakFormOutput()
{
    delete m_output;
    m_output = nullptr;
}


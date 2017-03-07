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
#include "generator_module.h"
#include "parser.h"
#include "solver/module.h"


#include "util/constants.h"

Agros2DGeneratorModule::Agros2DGeneratorModule(const QString &moduleId) : m_output(nullptr), m_id(moduleId)
{
    QDir root(QCoreApplication::applicationDirPath());
    root.mkpath(QString("%1/%2").arg(GENERATOR_PLUGINROOT).arg(moduleId));

    // read module
    module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + moduleId + ".xml").toStdString(), xml_schema::flags::dont_validate);
    XMLModule::module *mod = module_xsd.get();
    assert(mod->field().present());
    m_module = &mod->field().get();

    QDir().mkdir(GENERATOR_PLUGINROOT + "/" + moduleId);

    // documentation
    QDir doc_root(QCoreApplication::applicationDirPath());
    doc_root.mkpath(QString("%1/%2").arg(GENERATOR_DOCROOT).arg(moduleId));
    QDir().mkdir(GENERATOR_DOCROOT + "/" + moduleId);

    // variables
    foreach (XMLModule::quantity quantity, m_module->volume().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_volumeVariables.insert(iD, shortName);
    }

    // functions
    foreach (XMLModule::function function, m_module->volume().function())
    {
        QString shortName = QString::fromStdString(function.shortname()).replace(" ", "");
        QString iD = QString::fromStdString(function.id().c_str()).replace(" ", "");
        m_volumeVariables.insert(iD, shortName);
    }

    // surface variables
    foreach (XMLModule::quantity quantity, m_module->surface().quantity())
    {
        QString shortName = QString::fromStdString(quantity.shortname().get()).replace(" ", "");
        QString iD = QString::fromStdString(quantity.id().c_str()).replace(" ", "");
        m_surfaceVariables.insert(iD, shortName);
    }

    // localization
    getNames(moduleId);

    Module::volumeQuantityProperties(m_module, quantityOrdering, quantityIsNonlinear, functionOrdering);
}

Agros2DGeneratorModule::~Agros2DGeneratorModule()
{
}

void Agros2DGeneratorModule::generatePluginProjectFile()
{
    qDebug() << (QString("generating project file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());

    ctemplate::TemplateDictionary output("output");
    output.SetValue("ID", id.toStdString());

    // expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/module_CMakeLists_txt.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &text);

    // save to file
    writeStringContent(QString("%1/%2/%3/CMakeLists.txt").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::prepareWeakFormsOutput()
{
    qDebug() << (QString("parsing weak forms").toLatin1());
    assert(! m_output);
    m_output = new ctemplate::TemplateDictionary("output");

    QString id = QString::fromStdString(m_module->general_field().id());

    m_output->SetValue("ID", m_module->general_field().id());
    m_output->SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    //comment on beginning of weakform.cpp, may be removed
    ctemplate::TemplateDictionary *field;
    foreach (QString quantID, this->quantityOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", quantID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID]).toStdString());
        field->SetValue("OFFSET", "offset.quant");
        if (quantityIsNonlinear[quantID])
        {
            field = m_output->AddSectionDictionary("QUANTITY_INFO");
            field->SetValue("QUANT_ID", QString("derivative %1").arg(quantID).toStdString());
            field->SetValue("INDEX", QString("%1").arg(quantityOrdering[quantID] + 1).toStdString());
            field->SetValue("OFFSET", "offset.quant");
        }
    }
    foreach (QString funcID, this->functionOrdering.keys())
    {
        field = m_output->AddSectionDictionary("QUANTITY_INFO");
        field->SetValue("QUANT_ID", funcID.toStdString());
        field->SetValue("INDEX", QString("%1").arg(functionOrdering[funcID]).toStdString());
        field->SetValue("OFFSET", "offset.quant");
    }

    QString description = QString::fromStdString(m_module->general_field().description());
    description = description.replace("\n","");
    m_output->SetValue("DESCRIPTION", description.toStdString());

    // macros
    if (m_module->macros().present())
    {
        foreach (XMLModule::macro macro, m_module->macros().get().macro())
        {
            ctemplate::TemplateDictionary *variable = m_output->AddSectionDictionary("MACRO");
            variable->SetValue("MACRO_ID", macro.id());
            variable->SetValue("MACRO_EXPRESSION", macro.expression());
        }
    }

    generateWeakForms(*m_output);

    foreach(QString name, m_names)
    {
        ctemplate::TemplateDictionary *field = m_output->AddSectionDictionary("NAMES");
        field->SetValue("NAME",name.toStdString());
    }

}

void Agros2DGeneratorModule::deleteWeakFormOutput()
{
    delete m_output;
    m_output = nullptr;
}

void Agros2DGeneratorModule::generatePluginInterfaceFiles()
{
    qDebug() << (QString("generating interface file").toLatin1());
    QString id = QString::fromStdString(m_module->general_field().id());

    std::string text;

    // header - expand template
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/interface_h.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.h").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));

    // source - expand template
    text.clear();

    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/interface_cpp.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_interface.cpp").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}



void Agros2DGeneratorModule::generatePluginEquations()
{
    qDebug() << (QString("generating equations").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    QString outputDir = QDir().absoluteFilePath(QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg("resources/images/equations/"));

    ctemplate::TemplateDictionary output("output");

    output.SetValue("ID", m_module->general_field().id());
    output.SetValue("CLASS", (id.left(1).toUpper() + id.right(id.length() - 1)).toStdString());

    std::string text;

    output.SetValue("LATEX_TEMPLATE", compatibleFilename(QString("%1/%2/equations.tex").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString());

    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        ctemplate::TemplateDictionary *equation = output.AddSectionDictionary("EQUATION_SECTION");

        equation->SetValue("EQUATION", weakform.equation());
        equation->SetValue("OUTPUT_DIRECTORY", outputDir.toStdString());

        equation->SetValue("NAME", QString("%1").arg(QString::fromStdString(weakform.analysistype())).toStdString());
    }

    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            ctemplate::TemplateDictionary *equation = output.AddSectionDictionary("EQUATION_SECTION");

            equation->SetValue("EQUATION", boundary.equation());
            equation->SetValue("OUTPUT_DIRECTORY", outputDir.toStdString());

            equation->SetValue("NAME", QString("%1_%2").arg(QString::fromStdString(weakform.analysistype())).arg(QString::fromStdString(boundary.id())).toStdString());
        }
    }

    ExpandTemplate(compatibleFilename(QString("%1/%2/equations.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, &output, &text);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_equations.py").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(text));
}

void Agros2DGeneratorModule::getNames(const QString &moduleId)
{
    QFile * file = new QFile((datadir().toStdString() + MODULEROOT.toStdString() + "/" + moduleId.toStdString() + ".xml").c_str());
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(file);
    QStringList names;
    while(!xml.atEnd())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        if(token == QXmlStreamReader::EndDocument)
            break;
        if(xml.attributes().hasAttribute("name"))
        {
            if(!m_names.contains(xml.attributes().value("name").toString()))
                m_names.append(xml.attributes().value("name").toString());
        }
        if(xml.attributes().hasAttribute("name"))
        {
            if(!m_names.contains(xml.attributes().value("analysistype").toString()))
                m_names.append(xml.attributes().value("analysistype").toString());
        }
    }
}

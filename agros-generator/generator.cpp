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

#include <QDir>
#include "generator.h"
#include "generator_module.h"
#include "generator_coupling.h"

#include "util/constants.h"
#include "solver/module.h"
#include "solver/coupling.h"

#include "parser/lex.h"

#include "util/constants.h"


int Agros2DGenerator::numberOfSolutions(XMLModule::analyses analyses, AnalysisType analysisType)
{
    foreach (XMLModule::analysis analysis, analyses.analysis())
        if (analysis.id() == analysisTypeToStringKey(analysisType).toStdString())
            return analysis.solutions();

    return -1;
}

// *****************************************************************************************************************

Agros2DGenerator::Agros2DGenerator(int &argc, char **argv) : QCoreApplication(argc, argv)
{
}

void Agros2DGenerator::run()
{
    // generate structure
    createStructure();

    if (!m_module.isEmpty())
    {
        // generate one module or coupling
        QMap<QString, QString> modules = Module::availableModules();
        QList<QString> couplings = couplingList()->availableCouplings();

        try
        {
            if (modules.keys().contains(m_module))
                generateModule(m_module);
            else if (couplings.contains(m_module))
                generateCoupling(m_module);

            exit(0);
        }
        catch(AgrosGeneratorException& err)
        {
            qDebug() << "Generator exception " << err.what();
            exit(1);
        }
    }
    else
    {
        // generate all sources
        try
        {
            generateSources();
            exit(0);
        }
        catch(AgrosGeneratorException& err)
        {
            qDebug() << "Generator exception " << err.what();
            exit(1);
        }
    }
}

void Agros2DGenerator::createStructure()
{
    // create directory
    QDir root(QCoreApplication::applicationDirPath());
    root.mkpath(GENERATOR_PLUGINROOT);

    // documentation
    QDir doc_root(QCoreApplication::applicationDirPath());
    doc_root.mkpath(GENERATOR_DOCROOT);

    ctemplate::TemplateDictionary output("project_output");
    QMap<QString, QString> modules = Module::availableModules();
    foreach (QString moduleId, modules.keys())
    {
        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
        field->SetValue("ID", moduleId.toStdString());
        field->SetValue("CLASS", (moduleId[0].toUpper() + moduleId.right(moduleId.length() - 1)).toStdString());
    }

    // QMap<QString, QString> modules = Module::availableModules();
    //    foreach (QString couplingId, couplings)
    //    {
    //        ctemplate::TemplateDictionary *field = output.AddSectionDictionary("SOURCE");
    //        field->SetValue("ID", couplingId.toStdString());
    //    }

    // expand template
    // generate plugins project file
    std::string textProject;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/plugins_CMakeLists_txt.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &textProject);
    // save to file
    writeStringContent(QString("%1/%2/CMakeLists.txt").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT),
                       QString::fromStdString(textProject));

    // generate static include
    std::string textStatic;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/plugins_plugins_static_h.tpl").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, &output, &textStatic);
    // save to file
    writeStringContent(QString("%1/%2/plugins_static.h").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT),
                       QString::fromStdString(textStatic));

    exit(0);
}

void Agros2DGenerator::generateSources()
{
    QMap<QString, QString> modules = Module::availableModules();
    QList<QString> couplings = couplingList()->availableCouplings();

    qDebug() << couplings;

    foreach (QString moduleId, modules.keys())
    {
        generateModule(moduleId);
        generateDocumentation(moduleId);
    }

    //    foreach (QString couplingId, couplings)
    //    {
    //        generateCoupling(couplingId);
    //    }
}

void Agros2DGenerator::generateModule(const QString &moduleId)
{
    Agros2DGeneratorModule generator(moduleId);

    qDebug() << (QString("Module: %1.").arg(moduleId).toLatin1());

    generator.generatePluginProjectFile();
    generator.prepareWeakFormsOutput();
    generator.generatePluginInterfaceFiles();
    generator.generatePluginWeakFormFiles();
    generator.deleteWeakFormOutput();
    generator.generatePluginFilterFiles();
    generator.generatePluginForceFiles();
    generator.generatePluginLocalPointFiles();
    generator.generatePluginSurfaceIntegralFiles();
    generator.generatePluginVolumeIntegralFiles();

    // generates documentation
    generator.generatePluginDocumentationFiles();

    // generates equations
    generator.generatePluginEquations();
}

void Agros2DGenerator::generateDocumentation(const QString &moduleId)
{
    Agros2DGeneratorModule generator(moduleId);

}

void Agros2DGenerator::generateCoupling(const QString &couplingId)
{
    Agros2DGeneratorCoupling generator(couplingId);

    generator.generatePluginProjectFile();
    generator.prepareWeakFormsOutput();
    generator.generatePluginInterfaceFiles();
    generator.generatePluginWeakFormFiles();
    generator.deleteWeakFormOutput();
}


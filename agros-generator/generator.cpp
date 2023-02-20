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

void checkDuplicities(QList<FormInfo> list)
{
    for(int i = 0; i < list.size(); i++)
    {
        for(int j = 0; j < list.size(); j++)
        {
            if(i != j)
            {
                if(list[i].id == list[j].id)
                    throw AgrosGeneratorException("Duplicities in forms");
            }
        }
    }
}

FormInfo findFormInfo(QList<FormInfo> list, QString id)
{
    foreach(FormInfo form, list)
    {
        if(form.id == id)
            return form;
    }
    throw AgrosGeneratorException(QString("Form %1 not found").arg(id));
}

// todo: implement properly. What if uval is part of some identifier?
void replaceForVariant(QString& str, WeakFormVariant variant, int solutionIndex)
{
    if (variant == WeakFormVariant_Normal)
    {
        // pass
    }
    else if (variant == WeakFormVariant_Residual)
    {
        str.replace("uval", QString("upval%1").arg(solutionIndex));
        str.replace("udx", QString("updx%1").arg(solutionIndex));
        str.replace("udy", QString("updy%1").arg(solutionIndex));
        str.replace("udr", QString("updr%1").arg(solutionIndex));
        str.replace("udz", QString("updz%1").arg(solutionIndex));
    }
    else
    {
        throw AgrosGeneratorException("Unknown form variant");
    }
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
QList<FormInfo> wfVectorModuleTemplates(SectionWithTemplates *section)
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

QList<FormInfo> wfEssentialModuleTemplates(XMLModule::surface *surface)
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

QList<FormInfo> wfGenerateSeparated(QList<FormInfo> elements, QList<FormInfo> templates, QList<FormInfo> templatesForResidual)
{
    checkDuplicities(templates);
    checkDuplicities(elements);
    QList<FormInfo> listResult;
    foreach (FormInfo formElement, elements)
    {
        FormInfo formTemplate;
        try
        {
            formTemplate = findFormInfo(templates, formElement.id);
        }
        catch(AgrosGeneratorException &err)
        {
            if(templatesForResidual.empty())
            {
                throw;
            }
            else
            {
                formTemplate = findFormInfo(templatesForResidual, formElement.id);
                formTemplate.variant = WeakFormVariant_Residual;
            }
        }

        FormInfo formResult(formTemplate.id, formTemplate.i, formTemplate.j, formTemplate.sym_planar, formTemplate.sym_axi);
        formResult.condition = formTemplate.condition;
        formResult.variant = formTemplate.variant;

        if (formElement.coefficient != 1.)
        {
            formResult.expr_axi = QString("%1*(%2)").arg(formElement.coefficient).arg(formTemplate.expr_axi);
            formResult.expr_planar = QString("%1*(%2)").arg(formElement.coefficient).arg(formTemplate.expr_planar);
        }
        else
        {
            formResult.expr_axi = formTemplate.expr_axi;
            formResult.expr_planar = formTemplate.expr_planar;
        }

        // qWarning() << "i" << formResult.i << "j" << formResult.j;

        replaceForVariant(formResult.expr_axi, formElement.variant, formResult.j);
        replaceForVariant(formResult.expr_planar, formElement.variant, formResult.j);

        listResult.push_back(formResult);
    }

    return listResult;
}

template <typename SectionWithElements>
QList<FormInfo> wfMatrixModuleElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
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
QList<FormInfo> wfMatrixTransientModuleElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
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
QList<FormInfo> wfVectorModuleElements(SectionWithElements *section, AnalysisType analysisType, LinearityType linearityType)
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

QList<FormInfo> wfMatrixVolumeModuleSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> matrixElements = wfMatrixModuleElements(&module->volume(), analysisType, linearityType);

    return wfGenerateSeparated(matrixElements, matrixTemplates);
}

QList<FormInfo> wfVectorVolumeModuleSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> vectorTemplates = wfVectorModuleTemplates(&module->volume());
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> vectorElements = wfVectorModuleElements(&module->volume(), analysisType, linearityType);

    return wfGenerateSeparated(vectorElements, vectorTemplates, matrixTemplates);
}

QList<FormInfo> wfMatrixTransientVolumeModuleSeparated(XMLModule::field* module, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> templates = wfMatrixTemplates(&module->volume());
    QList<FormInfo> matrixElements = wfMatrixTransientModuleElements(&module->volume(), analysisType, linearityType);

    return wfGenerateSeparated(matrixElements, templates);
}

QList<FormInfo> wfMatrixSurfaceModule(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(surface);
    QList<FormInfo> matrixElements = wfMatrixModuleElements<XMLModule::boundary>(boundary, analysisType, linearityType);
    return wfGenerateSeparated(matrixElements, matrixTemplates);
}

QList<FormInfo> wfVectorSurfaceModule(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> vectorTemplates = wfVectorModuleTemplates(surface);
    QList<FormInfo> matrixTemplates = wfMatrixTemplates(surface);
    QList<FormInfo> vectorElements = wfVectorModuleElements<XMLModule::boundary>(boundary, analysisType, linearityType);
    return wfGenerateSeparated(vectorElements, vectorTemplates, matrixTemplates);
}

QList<FormInfo> essentialModule(XMLModule::surface *surface, XMLModule::boundary *boundary, AnalysisType analysisType, LinearityType linearityType)
{
    QList<FormInfo> essentialTemplates = wfEssentialModuleTemplates(surface);
    QList<FormInfo> essentialElements = wfEssentialElements(boundary, analysisType, linearityType);
    return wfGenerateSeparated(essentialElements, essentialTemplates);
}

int Agros2DGenerator::numberOfSolutions(XMLModule::analyses analyses, AnalysisType analysisType)
{
    foreach (XMLModule::analysis analysis, analyses.analysis())
        if (analysis.id() == analysisTypeToStringKey(analysisType).toStdString())
            return analysis.solutions();

    return -1;
}


// coupling weak forms
XMLModule::linearity_option findLinearityCouplingOption(XMLModule::volume *volume, AnalysisType analysisTypeSource, AnalysisType analysisTypeTarget, CouplingType couplingType, LinearityType linearityType)
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
    XMLModule::linearity_option lo = findLinearityCouplingOption(section, analysisTypeSource, analysisTypeTarget, couplingType, linearityType);

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

QList<FormInfo> wfVectorVolumeCouplingSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType)
{
    QList<FormInfo> templatesVector = wfVectorCouplingTemplates(volume);
    QList<FormInfo> templatesMatrix = wfMatrixCouplingTemplates(volume);
    QList<FormInfo> elements = wfVectorCouplingElements(volume, sourceAnalysis, targetAnalysis, couplingType, linearityType);

    return wfGenerateSeparated(elements, templatesVector, templatesMatrix);
}

// *****************************************************************************************************************

Agros2DGenerator::Agros2DGenerator(int &argc, char **argv) : QCoreApplication(argc, argv)
{
}

QStringList Agros2DGenerator::availableModules()
{
    QStringList list;

    // read images
    QStringList filters;
    filters << "*.xml";

    QDir dir(QString("resources_source/modules/"));
    dir.setNameFilters(filters);

    foreach (QString id, dir.entryList())
        list.append(id.left(id.count() - 4));

    return list;
}

QStringList Agros2DGenerator::availableCouplings()
{
    QStringList list;

    // read images
    QStringList filters;
    filters << "*.xml";

    QDir dir(QString("resources_source/couplings/"));
    dir.setNameFilters(filters);

    foreach (QString id, dir.entryList())
        list.append(id.left(id.count() - 4));

    return list;
}

void Agros2DGenerator::run()
{
    bool generationIsNeeded = false;
    QString cmakeFile = QString("%1/%2/CMakeLists.txt").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_PLUGINROOT);
    if (QFile::exists(cmakeFile))
    {
        QFileInfo cmakeFileInfo(cmakeFile);
        QDateTime createdCMakeList = cmakeFileInfo.lastModified();
        // qInfo() << "CMakeList = " << createdCMakeList.toString();

        QString dir = QString("%1/%2/").arg(QCoreApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT);
        QDirIterator it(dir, QStringList() << "*.tpl", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QString file = it.next();

            QFileInfo fileInfo(file);
            QDateTime created = fileInfo.lastModified();
            // qInfo() << file << " = " << created.toString() << (createdCMakeList.toMSecsSinceEpoch() < created.toMSecsSinceEpoch());
            if (createdCMakeList.toMSecsSinceEpoch() < created.toMSecsSinceEpoch())
            {
                generationIsNeeded = true;
                break;
            }
        }
    }
    else
    {
        generationIsNeeded = true;
    }

    generationIsNeeded = true;
    if (generationIsNeeded)
    {
        // generate structure
        createStructure();

        if (!m_module.isEmpty())
        {
            // generate one module or coupling
            QStringList modules = Agros2DGenerator::availableModules();
            QStringList couplings = Agros2DGenerator::availableCouplings();

            try
            {
                if (modules.contains(m_module))
                    generateModule(m_module);               

                exit(0);
            }
            catch(AgrosGeneratorException& err)
            {
                qWarning() << "Generator exception " << err.what();
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
                qWarning() << "Generator exception " << err.what();
                exit(1);
            }
        }
    }
    else
    {
        qWarning() << "Generator is not needed. All is up to date.";
        exit(0);
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
    QStringList modules = Agros2DGenerator::availableModules();
    foreach (QString moduleId, modules)
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
    QStringList modules = Agros2DGenerator::availableModules();
    QStringList couplings = Agros2DGenerator::availableCouplings();

    foreach (QString moduleId, modules)
    {
        generateModule(moduleId);
        generateDocumentation(moduleId);
    }
}

void Agros2DGenerator::generateModule(const QString &moduleId)
{
    Agros2DGeneratorModule generator(moduleId);

    qWarning() << (QString("Module: %1.").arg(moduleId).toLatin1());
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

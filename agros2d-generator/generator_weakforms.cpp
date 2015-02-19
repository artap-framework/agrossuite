#include "generator.h"
#include "generator_module.h"
#include "parser.h"
#include "util/constants.h"

#include "solver/weak_form.h"
#include "solver/module.h"
#include "solver/coupling.h"

void Agros2DGeneratorModule::generatePluginWeakFormFiles()
{
    generatePluginWeakFormSourceFiles();
    generatePluginWeakFormHeaderFiles();
}

void Agros2DGeneratorModule::generatePluginWeakFormSourceFiles()
{
    qDebug() << (QString("generating weakform source file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    std::string textWeakform;

    ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_cpp.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                   ctemplate::DO_NOT_STRIP, m_output, &textWeakform);

    // source - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.cpp").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(textWeakform));  
}

void Agros2DGeneratorModule::generatePluginWeakFormHeaderFiles()
{
    qDebug() << (QString("generating weakform header file").toLatin1());

    QString id = QString::fromStdString(m_module->general_field().id());
    std::string textWeakform;

    // header - expand template
    std::string text;
    ctemplate::ExpandTemplate(compatibleFilename(QString("%1/%2/weakform_h.tpl").arg(QApplication::applicationDirPath()).arg(GENERATOR_TEMPLATEROOT)).toStdString(),
                              ctemplate::DO_NOT_STRIP, m_output, &textWeakform);

    // header - save to file
    writeStringContent(QString("%1/%2/%3/%3_weakform.h").
                       arg(QApplication::applicationDirPath()).
                       arg(GENERATOR_PLUGINROOT).
                       arg(id),
                       QString::fromStdString(textWeakform));   
}

void Agros2DGeneratorModule::generateWeakForms(ctemplate::TemplateDictionary &output)
{
    this->m_docString = "";

    QMap<QString, QString> availableModules = Module::availableModules();
    QMap<QString, XMLModule::coupling *> xml_couplings;

    QMap<QString, std::shared_ptr<XMLModule::module> > couplings_xsd;
    std::shared_ptr<XMLModule::module> coupling_xsd;

    QList<AnalysisType> allAnalysisTypes;
    allAnalysisTypes.push_back(AnalysisType_SteadyState);
    allAnalysisTypes.push_back(AnalysisType_Transient);
    allAnalysisTypes.push_back(AnalysisType_Harmonic);

    //qDebug() << couplingList()->availableCouplings();
    foreach(QString sourceField, availableModules.keys())
    {
        if(couplingList()->isCouplingAvailable(sourceField, this->m_id, CouplingType_Weak))
        {
            ctemplate::TemplateDictionary *coupling = output.AddSectionDictionary("COUPLING_SOURCE");
            coupling->SetValue("COUPLING_SOURCE_ID", sourceField.toStdString());

            coupling_xsd = XMLModule::module_(compatibleFilename(datadir() + COUPLINGROOT + "/" + sourceField + "-" + this->m_id + ".xml").toStdString(), xml_schema::flags::dont_validate);
            XMLModule::module *mod = coupling_xsd.get();
            assert(mod->coupling().present());
            xml_couplings[sourceField] = &mod->coupling().get();
            couplings_xsd[sourceField] = coupling_xsd;

            QHash<QString, QString> volumeVariables;
            //QHash<std::string, std::string> volumeVariables;
            volumeVariables.clear();

            std::shared_ptr<XMLModule::module> source_module_xsd = XMLModule::module_(compatibleFilename(datadir() + MODULEROOT + "/" + sourceField + ".xml").toStdString(), xml_schema::flags::dont_validate);;
            XMLModule::field source_module = source_module_xsd->field().get();
            foreach(XMLModule::quantity variable, source_module.volume().quantity())
            {
                ctemplate::TemplateDictionary *sourceVariables = coupling->AddSectionDictionary("COUPLING_VARIABLES");
                sourceVariables->SetValue("VARIABLE", variable.id().c_str());
                sourceVariables->SetValue("VARIABLE_SHORT", variable.shortname().get().c_str());
                volumeVariables.insert(QString::fromStdString(variable.id().c_str()), QString::fromStdString(variable.shortname().get().c_str()));
            }

            foreach(XMLModule::weakform_volume weakform, source_module.volume().weakforms_volume().weakform_volume())
            {
                ctemplate::TemplateDictionary *sectionAnalysisType = coupling->AddSectionDictionary("COUPLING_VARIABLES_ANALYSIS_TYPE");
                sectionAnalysisType->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()))).toStdString());
                foreach(XMLModule::quantity quantity, weakform.quantity())
                {
                    ctemplate::TemplateDictionary *variables = sectionAnalysisType->AddSectionDictionary("COUPLING_VARIABLES");
                    variables->SetValue("VARIABLE", quantity.id().c_str());
                    variables->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
                    variables->SetValue("VARIABLE_SHORT", volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

                }

            }

        }
    }

    // volume
    foreach(XMLModule::weakform_volume weakform, m_module->volume().weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));

        foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
        {
            foreach(XMLModule::linearity_option option, weakform.linearity_option())
            {
                LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

                ctemplate::TemplateDictionary *fieldVolume = generateVolumeVariables(linearityType, coordinateType, output, weakform, "VOLUME");

                QList<FormInfo> matrixForms = Module::wfMatrixVolumeSeparated(m_module, analysisType, linearityType);
                foreach(FormInfo formInfo, matrixForms)
                {
                    generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "MATRIX", weakform);
                }

                QList<FormInfo> matrixTransientForms = Module::wfMatrixTransientVolumeSeparated(m_module, analysisType, linearityType);
                foreach(FormInfo formInfo, matrixTransientForms)
                {
                    generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "TRANSIENT", weakform);
                }

                QList<FormInfo> vectorForms = Module::wfVectorVolumeSeparated(m_module, analysisType, linearityType);
                foreach(FormInfo formInfo, vectorForms)
                {
                    generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "VECTOR", weakform);
                }
                foreach(QString sourceField, xml_couplings.keys())
                {
                    ctemplate::TemplateDictionary *coupling = fieldVolume->AddSectionDictionary("COUPLING_SOURCE");
                    coupling->SetValue("COUPLING_SOURCE_ID", sourceField.toStdString());

                    foreach(AnalysisType sourceAnalysisType, allAnalysisTypes)
                    {
                        ctemplate::TemplateDictionary *sectionAnalysisType = coupling->AddSectionDictionary("COUPLING_FORMS_ANALYSIS_TYPE");
                        sectionAnalysisType->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(sourceAnalysisType).toStdString());

                        QList<FormInfo> vectorForms = CouplingInfo::wfVectorVolumeSeparated(&(xml_couplings[sourceField]->volume()), sourceAnalysisType, analysisType, CouplingType_Weak, linearityType);
                        if (!vectorForms.isEmpty())
                        {
                            foreach(FormInfo formInfo, vectorForms)
                            {
                                generateFormExpression(formInfo, linearityType, coordinateType, *sectionAnalysisType, "COUPLING_VECTOR", weakform);
                            }
                        }
                    }
                }

                // find if there are any possible coupling sources
                foreach(QString mod, availableModules.keys())
                {
                    //qDebug() << mod;
                }
            }
        }
    }

    // surface
    foreach(XMLModule::weakform_surface weakform, m_module->surface().weakforms_surface().weakform_surface())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));
        foreach(XMLModule::boundary boundary, weakform.boundary())
        {
            foreach (CoordinateType coordinateType, Agros2DGenerator::coordinateTypeList())
            {
                foreach(XMLModule::linearity_option option, boundary.linearity_option())
                {
                    LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

                    ctemplate::TemplateDictionary *fieldSurface = generateSurfaceVariables(linearityType, coordinateType, output, weakform, "SURFACE", &boundary);
                    QList<FormInfo> matrixForms = Module::wfMatrixSurface(&m_module->surface(), &boundary, analysisType, linearityType);
                    foreach(FormInfo formInfo, matrixForms)
                    {
                        generateFormExpression(formInfo, linearityType, coordinateType, *fieldSurface, "MATRIX", weakform);
                    }

                    QList<FormInfo> vectorForms = Module::wfVectorSurface(&m_module->surface(), &boundary, analysisType, linearityType);
                    foreach(FormInfo formInfo, vectorForms)
                    {
                        generateFormExpression(formInfo, linearityType, coordinateType, *fieldSurface, "VECTOR", weakform);
                    }

                    QList<FormInfo> essentialForms = Module::essential(&m_module->surface(), &boundary, analysisType, linearityType);
                    if (!essentialForms.isEmpty())
                    {
                        ctemplate::TemplateDictionary *fieldEssential = fieldSurface->AddSectionDictionary("ESSENTIAL");
                        int numSolutions = Agros2DGenerator::numberOfSolutions(m_module->general_field().analyses(), analysisType);
                        fieldEssential->SetValue("NUM_SOLUTIONS", QString::number(numSolutions).toStdString());

                        for (int i = 0; i < numSolutions; i++)
                        {
                            QString maskValue = "false";
                            ctemplate::TemplateDictionary *components = fieldEssential->AddSectionDictionary("COMPONENTS");
                            foreach (FormInfo formInfo, essentialForms)
                            {
                                if (formInfo.i - 1 == i)
                                    maskValue = "true";
                            }
                            components->SetValue("IS_ESSENTIAL", maskValue.toStdString());
                            components->SetValue("COMP_NUM", QString::number(i).toStdString());
                        }

//                        foreach (FormInfo formInfo, essentialForms)
//                        {
//                            generateFormExpression(formInfo, linearityType, coordinateType, *fieldSurface, "ESSENTIAL", weakform);
//                        }
                    }

                    // separate essential condition generation

//                    QList<FormInfo> essentialForms = Module::essential(&m_module->surface(), &boundary, analysisType, linearityType);
                    if (!essentialForms.isEmpty())
                    {
                        ctemplate::TemplateDictionary *fieldExact = generateSurfaceVariables(linearityType, coordinateType, output, weakform, "EXACT", &boundary);

                        // TODO: do it better
                        // form mask (for essential bcs)
                        int numSolutions = Agros2DGenerator::numberOfSolutions(m_module->general_field().analyses(), analysisType);
                        fieldExact->SetValue("NUM_SOLUTIONS", QString::number(numSolutions).toStdString());

                        for (int i = 0; i < numSolutions; i++)
                        {
                            QString maskValue = "false";
                            ctemplate::TemplateDictionary *mask = fieldExact->AddSectionDictionary("FORM_EXPRESSION_MASK");
                            foreach (FormInfo formInfo, essentialForms)
                            {
                                if (formInfo.i - 1 == i)
                                    maskValue = "true";
                            }
                            mask->SetValue("MASK", maskValue.toStdString());
                        }

                        foreach (FormInfo formInfo, essentialForms)
                        {
                            generateFormExpression(formInfo, linearityType, coordinateType, *fieldExact, "ESSENTIAL", weakform);
                        }
                    }
                }
            }
        }
    }
}

template <typename WeakForm>
void Agros2DGeneratorModule::generateFormExpression(FormInfo formInfo,
                                                    LinearityType linearityType,
                                                    CoordinateType coordinateType,
                                                    ctemplate::TemplateDictionary &output,
                                                    QString formType,
                                                    WeakForm weakform)
{
    QString expression = (coordinateType == CoordinateType_Planar ? formInfo.expr_planar : formInfo.expr_axi);

    if (!expression.isEmpty())
    {
        QString symetryAppendix;
        symetryAppendix = "";

        if(formInfo.variant == WeakFormVariant_Normal)
        {
            if((coordinateType == CoordinateType_Planar) && (formInfo.sym_planar))
                symetryAppendix = "_SYM";
            if((coordinateType == CoordinateType_Axisymmetric) && (formInfo.sym_axi))
                symetryAppendix = "_SYM";
        }
        if((formType == "VECTOR") && (symetryAppendix == "_SYM"))
        {
            std::cout << " symmetric vector !!!" << std::endl;
            assert(0);
        }

        ctemplate::TemplateDictionary *expr = output.AddSectionDictionary(("FORM_EXPRESSION_" + formType + symetryAppendix).toStdString());

        expr->SetValue("EXPRESSION_ID", formInfo.id.toStdString());

        if (formInfo.j != 0)
        {
            expr->SetValue("COLUMN_INDEX", QString::number(formInfo.j - 1).toStdString());
        }
        else
        {
            expr->SetValue("COLUMN_INDEX", "0");
        }

//        foreach(XMLModule::function_use functionUse, weakform.function_use())
//        {
//            foreach(XMLModule::function functionDefinition, m_module->volume().function())
//            {
//                if (functionUse.id() == functionDefinition.id())
//                {
//                    generateSpecialFunction(functionDefinition, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), linearityType, coordinateType, *expr);
//                }
//            }
//        }

        expr->SetValue("ROW_INDEX", QString::number(formInfo.i - 1).toStdString());

        ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

        // expression
        QString exprCpp = Parser::parseWeakFormExpression(pmi, expression);
        expr->SetValue("EXPRESSION", exprCpp.toStdString());

        QString exprCppCheck = Parser::parseWeakFormExpressionCheck(pmi, formInfo.condition);
        if (exprCppCheck == "")
            exprCppCheck = "true";
        expr->SetValue("EXPRESSION_CHECK", exprCppCheck.toStdString());
    }
}

template <typename WeakForm>
ctemplate::TemplateDictionary *Agros2DGeneratorModule::generateVolumeVariables(LinearityType linearityType,
                                                                               CoordinateType coordinateType,
                                                                               ctemplate::TemplateDictionary &output,
                                                                               WeakForm weakform,
                                                                               QString weakFormType)
{
    ctemplate::TemplateDictionary *field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());

    ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

    foreach(XMLModule::quantity quantity, weakform.quantity())
    {
        QString nonlinearExpr = pmi.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

        if (linearityType != LinearityType_Linear && quantityIsNonlinear[QString::fromStdString(quantity.id())])
        {
            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldNonlinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // nonlinear value and derivative
            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberFromTable(%1)").
                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_DERIVATIVE", QString("derivativeFromTable(%1)").
                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
        }
        else
        {
            // linear only value

            ctemplate::TemplateDictionary *subFieldLinear = field->AddSectionDictionary("VARIABLE_SOURCE_LINEAR");
            subFieldLinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldLinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldLinear->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            subFieldLinear->SetValue("VARIABLE_VALUE", QString("number()").toStdString());
        }
    }

    // new generation of functions. It is much simpler than whe using Hermes, where external functions had to be created
    foreach(XMLModule::function_use function_use, weakform.function_use())
    {
//        QString nonlinearExpr = pmi.nonlinearExpressionVolume(QString::fromStdString(quantity.id()));

//        if (linearityType != LinearityType_Linear && quantityIsNonlinear[QString::fromStdString(quantity.id())])
//        {
//            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
//            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
//            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_volumeVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

//            // nonlinear value and derivative
//            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberFromTable(%1)").
//                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
//            subFieldNonlinear->SetValue("VARIABLE_DERIVATIVE", QString("derivativeFromTable(%1)").
//                                        arg(Parser::parseErrorExpression(pmi, nonlinearExpr)).toStdString());
//        }
//        else
        {
            // linear only value

            foreach(XMLModule::function function, pmi.volume.function())
            {
                if(function.id() == function_use.id())
                {
                    bool is_constant = (function.type() == "constant");

                    if(linearityType != LinearityType_Linear)
                    {
                        // find out if it does depend on some quantity, which is not constant on element
                        foreach(XMLModule::quantity quantity, function.quantity())
                        {
                            if(quantityIsNonlinear[QString::fromStdString(quantity.id())])
                            {
                                is_constant = false;
                            }
                        }
                    }

                    if((function.type() == "nonlinear") && linearityType == LinearityType_Linear)
                    {

                    }
                    else
                    {
                        ctemplate::TemplateDictionary *subFieldLinear;
                        if(is_constant)
                            subFieldLinear= field->AddSectionDictionary("FUNCTION_SOURCE_CONSTANT");
                        else
                            subFieldLinear= field->AddSectionDictionary("FUNCTION_SOURCE_NONCONSTANT");

                        QString expression;
                        if((coordinateType == CoordinateType_Axisymmetric) && function.function_variant()[0].expr_axi().present())
                            expression = QString::fromStdString(function.function_variant()[0].expr_axi().get());
                        else
                            expression = QString::fromStdString(function.function_variant()[0].expr());

                        subFieldLinear->SetValue("FUNCTION_SHORT", m_volumeVariables.value(QString::fromStdString(function_use.id().c_str())).toStdString());
                        subFieldLinear->SetValue("FUNCTION_EXPRESSION", Parser::parseWeakFormExpression(pmi, expression).toStdString());
                    }
                }
            }

        }
    }

    return field;
}

template <typename WeakForm>
ctemplate::TemplateDictionary *Agros2DGeneratorModule::generateSurfaceVariables(LinearityType linearityType,
                                                                                CoordinateType coordinateType,
                                                                                ctemplate::TemplateDictionary &output,
                                                                                WeakForm weakform,
                                                                                QString weakFormType,
                                                                                XMLModule::boundary *boundary)
{
    ctemplate::TemplateDictionary *field = output.AddSectionDictionary(weakFormType.toStdString() + "_SOURCE");

    field->SetValue("COORDINATE_TYPE", Agros2DGenerator::coordinateTypeStringEnum(coordinateType).toStdString());
    field->SetValue("LINEARITY_TYPE", Agros2DGenerator::linearityTypeStringEnum(linearityType).toStdString());
    field->SetValue("ANALYSIS_TYPE", Agros2DGenerator::analysisTypeStringEnum(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype()))).toStdString());

    field->SetValue("BOUNDARY_ID", boundary->id().c_str());

    ParserModuleInfo pmi(*m_module, analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())), coordinateType, linearityType);

    foreach(XMLModule::quantity quantity, boundary->quantity())
    {
        QString dep = pmi.dependenceSurface(QString::fromStdString(quantity.id()));

        // value
        if (dep.isEmpty())
        {
            // linear only value
            ctemplate::TemplateDictionary *subFieldLinear = field->AddSectionDictionary("VARIABLE_SOURCE_LINEAR");
            subFieldLinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldLinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldLinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            subFieldLinear->SetValue("VARIABLE_VALUE", QString("number()").toStdString());
        }
        else if (dep == "time")
        {
            // linear only value
            ctemplate::TemplateDictionary *subFieldLinear = field->AddSectionDictionary("VARIABLE_SOURCE_LINEAR");
            subFieldLinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldLinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldLinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // linear boundary condition
            subFieldLinear->SetValue("VARIABLE_VALUE", QString("numberAtTime(this->get_time())").toStdString());
        }
        else if (dep == "space")
        {
            // nonlinear case
            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldNonlinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // spacedep boundary condition
            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberAtPoint(Point(p[0], p[1]))").toStdString());
        }
        else if (dep == "time-space")
        {
            // nonlinear case
            ctemplate::TemplateDictionary *subFieldNonlinear = field->AddSectionDictionary("VARIABLE_SOURCE_NONLINEAR");
            subFieldNonlinear->SetValue("VARIABLE", quantity.id().c_str());
            subFieldNonlinear->SetValue("VARIABLE_HASH", QString::number(qHash(QString::fromStdString(quantity.id()))).toStdString());
            subFieldNonlinear->SetValue("VARIABLE_SHORT", m_surfaceVariables.value(QString::fromStdString(quantity.id().c_str())).toStdString());

            // spacedep boundary condition
            subFieldNonlinear->SetValue("VARIABLE_VALUE", QString("numberAtTimeAndPoint(this->get_time(), Point(p[0], p[1]))").toStdString());
        }
    }

    return field;
}

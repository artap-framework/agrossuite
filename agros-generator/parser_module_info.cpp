#include "util/constants.h"
#include "solver/weak_form.h"
#include "solver/module.h"
#include "solver/coupling.h"
#include "generator.h"
#include "parser_module_info.h"

void volumeQuantityModuleProperties(XMLModule::field *module, QMap<QString, int> &quantityOrder, QMap<QString, bool> &quantityIsNonlin, QMap<QString, int> &functionOrder)
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

ParserModuleInfo::ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, bool isSurface) :
    analysisType(analysisType), coordinateType(coordinateType), linearityType(linearityType), isSurface(isSurface),
    constants(field.constants()), macros(field.macros().present() ? field.macros().get() : XMLModule::macros()),volume(field.volume()), surface(field.surface())
{
    numSolutions = Agros2DGenerator::numberOfSolutions(field.general_field().analyses(), analysisType);
    id = QString::fromStdString(field.general_field().id());

    volumeQuantityModuleProperties(&field, quantityOrdering, quantityIsNonlinear, functionOrdering);
}

QString ParserModuleInfo::nonlinearExpressionVolume(const QString &variable) const
{
    foreach (XMLModule::weakform_volume wf, volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisTypeToStringKey(analysisType).toStdString())
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variable.toStdString())
                {
                    if (coordinateType == CoordinateType_Planar)
                    {
                        if (quantityAnalysis.nonlinearity_planar().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                    }
                    else
                    {
                        if (quantityAnalysis.nonlinearity_axi().present())
                            return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                    }
                }
            }
        }
    }

    return "";
}

QString ParserModuleInfo::nonlinearExpressionSurface(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_surface wf, surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variableString)
                    {
                        if (coordinateType == CoordinateType_Planar)
                        {
                            if (quantityAnalysis.nonlinearity_planar().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_planar().get());
                        }
                        else
                        {
                            if (quantityAnalysis.nonlinearity_axi().present())
                                return QString::fromStdString(quantityAnalysis.nonlinearity_axi().get());
                        }
                    }
                }
            }
        }
    }

    return "";
}

//QString ParserModuleInfo::specialFunctionNonlinearExpression(const QString &variable) const
//{
//    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
//    std::string variableString = variable.toStdString();
//    foreach (XMLModule::weakform_volume wf, volume.weakforms_volume().weakform_volume())
//    {
//        if (wf.analysistype() == analysisString)
//        {
//            foreach (XMLModule::function_use functionUse, wf.function_use())
//            {
//                if (functionUse.id() == variableString)
//                {
//                    if (coordinateType == CoordinateType_Planar)
//                    {
//                        if (functionUse.nonlinearity_planar().present())
//                            return QString::fromStdString(functionUse.nonlinearity_planar().get());
//                    }
//                    else
//                    {
//                        if (functionUse.nonlinearity_axi().present())
//                            return QString::fromStdString(functionUse.nonlinearity_axi().get());
//                    }
//                }
//            }
//        }
//    }

//    return "0";
//}

QString ParserModuleInfo::dependenceVolume(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_volume wf, volume.weakforms_volume().weakform_volume())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::quantity quantityAnalysis, wf.quantity())
            {
                if (quantityAnalysis.id() == variableString)
                {
                    if (quantityAnalysis.dependence().present())
                        return QString::fromStdString(quantityAnalysis.dependence().get());
                }
            }
        }
    }

    return "";
}

QString ParserModuleInfo::dependenceSurface(const QString &variable) const
{
    std::string analysisString = analysisTypeToStringKey(analysisType).toStdString();
    std::string variableString = variable.toStdString();
    foreach (XMLModule::weakform_surface wf, surface.weakforms_surface().weakform_surface())
    {
        if (wf.analysistype() == analysisString)
        {
            foreach (XMLModule::boundary boundary, wf.boundary())
            {
                foreach (XMLModule::quantity quantityAnalysis, boundary.quantity())
                {
                    if (quantityAnalysis.id() == variableString)
                    {
                        if (quantityAnalysis.dependence().present())
                            return QString::fromStdString(quantityAnalysis.dependence().get());
                    }
                }
            }
        }
    }

    return "";
}


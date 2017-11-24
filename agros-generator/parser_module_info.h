#ifndef PARSER_MODULE_INFO_H
#define PARSER_MODULE_INFO_H

#include "util/util.h"
#include "util/enums.h"
#include "../../resources_source/classes/module_xml.h"

void volumeQuantityModuleProperties(XMLModule::field *module, QMap<QString, int> &quantityOrder, QMap<QString, bool> &quantityIsNonlin, QMap<QString, int> &functionOrder);

// encapsulates information needed for construction of specific parser instances
struct ParserModuleInfo
{
    ParserModuleInfo(XMLModule::field field, AnalysisType analysisType, CoordinateType coordinateType, LinearityType linearityType, bool isSurface);

    XMLModule::constants constants;
    XMLModule::macros macros;
    XMLModule::volume volume;
    XMLModule::surface surface;

    bool isSurface;

    int numSolutions;
    QString id;

    AnalysisType analysisType;
    CoordinateType coordinateType;
    LinearityType linearityType;

    QString nonlinearExpressionVolume(const QString &variable) const;
    QString nonlinearExpressionSurface(const QString &variable) const;
    QString specialFunctionNonlinearExpression(const QString &variable) const;
    QString dependenceVolume(const QString &variable) const;
    QString dependenceSurface(const QString &variable) const;

    QMap<QString, int> quantityOrdering;
    QMap<QString, bool> quantityIsNonlinear;
    QMap<QString, int> functionOrdering;
};

bool operator<(const ParserModuleInfo &sid1, const ParserModuleInfo &sid2);

#endif // PARSER_MODULE_INFO_H

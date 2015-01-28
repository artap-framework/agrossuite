#ifndef WEAK_FORM_H
#define WEAK_FORM_H

#include "form_info.h"

class BDF2Table;
class Marker;
class Boundary;
class Material;
class SceneMaterial;
class CouplingInfo;
class FieldInfo;

namespace XMLModule
{
class field;
class quantity;
class boundary;
class surface;
class force;
class localvariable;
class gui;
class space;
class calculator;
class linearity_option;
}

struct PositionInfo
{
    PositionInfo();
    int formsOffset;
    int quantAndSpecOffset;
    int numQuantAndSpecFun;
    int previousSolutionsOffset;
    int numPreviousSolutions;
    bool isSource;
};

struct Offset
{
    Offset();
    int forms;
    int quant;
    int prevSol;

    int sourceForms;
    int sourceQuant;
    int sourcePrevSol;

    void print() { qDebug() << "forms: " << forms << ", quant: " << quant << ", prevSol: " << prevSol << ", sourceForms: " << sourceForms << ", sourceQuant: " << sourceQuant << ", sourcePrevSol: " << sourcePrevSol;}
};



struct ProblemID
{
    ProblemID() :
        sourceFieldId(""), targetFieldId(""),
        analysisTypeSource(AnalysisType_Undefined), analysisTypeTarget(AnalysisType_Undefined),
        coordinateType(CoordinateType_Undefined), linearityType(LinearityType_Undefined),
        couplingType(CouplingType_Undefined) {}

    // TODO: set/get methods
    QString sourceFieldId;
    QString targetFieldId;
    AnalysisType analysisTypeSource;
    AnalysisType analysisTypeTarget;
    CoordinateType coordinateType;
    LinearityType linearityType;
    CouplingType couplingType;

    QString toString()
    {
        // TODO: implement toString() method
        return "TODO";
    }
};


const int INVALID_POSITION_INFO_VALUE = -223344;
// maximal number of existing modules
const int MAX_FIELDS = 10;

void findVolumeLinearityOption(XMLModule::linearity_option& option, XMLModule::field *module, AnalysisType analysisType, LinearityType linearityType);

QList<FormInfo> generateSeparated(QList<FormInfo> elements, QList<FormInfo> templates, QList<FormInfo> templatesForResidual = QList<FormInfo>());

template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTemplates(SectionWithTemplates *section);
template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTransientTemplates(SectionWithTemplates *section);

#endif // WEAK_FORM_H

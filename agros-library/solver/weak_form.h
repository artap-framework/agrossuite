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

template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTemplates(SectionWithTemplates *section);
template <typename SectionWithTemplates>
QList<FormInfo> wfMatrixTransientTemplates(SectionWithTemplates *section);

#endif // WEAK_FORM_H

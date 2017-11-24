#include "util/global.h"
#include "problem.h"
#include "field.h"
#include "weak_form.h"


PositionInfo::PositionInfo() :
    formsOffset(INVALID_POSITION_INFO_VALUE),
    quantAndSpecOffset(INVALID_POSITION_INFO_VALUE),
    numQuantAndSpecFun(0),
    previousSolutionsOffset(INVALID_POSITION_INFO_VALUE),
    numPreviousSolutions(0),
    isSource(false)
{

}

Offset::Offset() :
    forms(INVALID_POSITION_INFO_VALUE),
    quant(INVALID_POSITION_INFO_VALUE),
    prevSol(INVALID_POSITION_INFO_VALUE),
    sourceForms(INVALID_POSITION_INFO_VALUE),
    sourceQuant(INVALID_POSITION_INFO_VALUE),
    sourcePrevSol(INVALID_POSITION_INFO_VALUE)
{
}

/*
template <typename Scalar>
Offset WeakFormAgros<Scalar>::offsetInfo(const Marker *sourceMarker, const Marker *targetMarker) const
{
    assert(targetMarker);

    if(sourceMarker)
        return offsetInfo(sourceMarker->fieldInfo(), targetMarker->fieldInfo());
    else
        return offsetInfo(nullptr, targetMarker->fieldInfo());
}

template <typename Scalar>
Offset WeakFormAgros<Scalar>::offsetInfo(const FieldInfo *sourceFieldInfo, const FieldInfo *targetFieldInfo) const
{
    Offset offset;
    assert(targetFieldInfo != nullptr);

    const int fieldID = targetFieldInfo->numberId();

//#ifdef _DEBUG
//    positionInfoBasicCheck(fieldID);
//#endif
    offset.forms = positionInfo(fieldID)->formsOffset;
    offset.prevSol = positionInfo(fieldID)->previousSolutionsOffset;
    offset.quant = positionInfo(fieldID)->quantAndSpecOffset;

    if(sourceFieldInfo)
    {
        const int fieldIDSource = sourceFieldInfo->numberId();
//#ifdef _DEBUG
//        positionInfoSourceFieldInfoCheck(fieldIDSource);
//#endif
        offset.sourceForms = positionInfo(fieldIDSource)->formsOffset;
        offset.sourcePrevSol = positionInfo(fieldIDSource)->previousSolutionsOffset;
        offset.sourceQuant = positionInfo(fieldIDSource)->quantAndSpecOffset;
    }

    return offset;
}

template <typename Scalar>
void WeakFormAgros<Scalar>::outputPositionInfos()
{
    qDebug() << "*************Block************************";
    qDebug() << "weak coupling sources:";
    foreach(FieldInfo* fieldInfo, m_block->sourceFieldInfosCoupling())
    {
        int id = fieldInfo->numberId();
        qDebug() << "fieldInfo " << fieldInfo->fieldId() << ", " << id;
        qDebug() << "field offset " << m_positionInfos[id].formsOffset << ", is source " << m_positionInfos[id].isSource;
        qDebug() << "quantities num " << m_positionInfos[id].numQuantAndSpecFun << ", offset " << m_positionInfos[id].quantAndSpecOffset;
        qDebug() << "previous solutions num " << m_positionInfos[id].numPreviousSolutions << ", offset " << m_positionInfos[id].previousSolutionsOffset;
    }
    qDebug() << "block members:";
    foreach(FieldInfo* fieldInfo, m_block->fieldInfos())
    {
        int id = fieldInfo->numberId();
        qDebug() << "fieldInfo " << fieldInfo->fieldId() << ", " << id;
        qDebug() << "field offset " << m_positionInfos[id].formsOffset << ", is source " << m_positionInfos[id].isSource;
        qDebug() << "quantities num " << m_positionInfos[id].numQuantAndSpecFun << ", offset " << m_positionInfos[id].quantAndSpecOffset;
        qDebug() << "previous solutions num " << m_positionInfos[id].numPreviousSolutions << ", offset " << m_positionInfos[id].previousSolutionsOffset;
    }
    qDebug() << "**************End*************************";
}

template class AGROS_LIBRARY_API WeakFormAgros<double>;
*/

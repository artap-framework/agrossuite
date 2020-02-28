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

#include "enums.h"

#include "util/util.h"
#include "util/global.h"
#include "scene.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

// QMap lookup is faster than in a QMap for less than about 10 elements
static QMap<CoordinateType, QString> coordinateTypeList;
static QMap<PhysicFieldVariableComp, QString> physicFieldVariableCompList;
static QMap<SceneViewPost3DMode, QString> sceneViewPost3DModeList;
static QMap<WeakFormKind, QString> weakFormList;
static QMap<WeakFormVariant, QString> weakFormVariantList;
static QMap<AdaptivityMethod, QString> adaptivityTypeList;
static QMap<AdaptivityEstimator, QString> adaptivityEstimatorList;
static QMap<AdaptivityStrategy, QString> adaptivityStrategyList;
static QMap<AdaptivityStrategyHP, QString> adaptivityStrategyHPList;
static QMap<NormType, QString> adaptivityNormTypeList;
static QMap<TimeStepMethod, QString> timeStepMethodList;
static QMap<AnalysisType, QString> analysisTypeList;
static QMap<CouplingType, QString> couplingTypeList;
static QMap<LinearityType, QString> linearityTypeList;
static QMap<DampingType, QString> dampingTypeList;
static QMap<MeshType, QString> meshTypeList;
static QMap<MatrixSolverType, QString> matrixSolverTypeList;
static QMap<MatrixExportFormat, QString> dumpFormatList;
static QMap<PaletteType, QString> paletteTypeList;
static QMap<VectorType, QString> vectorTypeList;
static QMap<VectorCenter, QString> vectorCenterList;
static QMap<DataTableType, QString> dataTableTypeList;
static QMap<ButcherTableType, QString> butcherTableTypeList;
static QMap<IterSolverDealII, QString> iterLinearSolverDealIIMethodList;
static QMap<PreconditionerDealII, QString> iterLinearSolverDealIIPreconditionerList;
static QMap<StudyType, QString> studyTypeList;
static QMap<ComputationResultType, QString> computationResultTypeList;
static QMap<ResultRecipeType, QString> resultRecipeTypeList;
static QMap<ProblemFunctionType, QString> problemFunctionTypeList;

QStringList coordinateTypeStringKeys() { return coordinateTypeList.values(); }
QString coordinateTypeToStringKey(CoordinateType coordinateType) { return coordinateTypeList[coordinateType]; }
CoordinateType coordinateTypeFromStringKey(const QString &coordinateType) { return coordinateTypeList.key(coordinateType); }

QStringList analysisTypeStringKeys() { return analysisTypeList.values(); }
QString analysisTypeToStringKey(AnalysisType analysisType) { return analysisTypeList[analysisType]; }
AnalysisType analysisTypeFromStringKey(const QString &analysisType) { return analysisTypeList.key(analysisType); }

QStringList couplingTypeStringKeys() { return couplingTypeList.values(); }
QString couplingTypeToStringKey(CouplingType couplingType) { return couplingTypeList[couplingType]; }
CouplingType couplingTypeFromStringKey(const QString &couplingType) { return couplingTypeList.key(couplingType); }

QStringList weakFormStringKeys() { return weakFormList.values(); }
QString weakFormToStringKey(WeakFormKind weakForm) { return weakFormList[weakForm]; }
WeakFormKind weakFormFromStringKey(const QString &weakForm) { return weakFormList.key(weakForm); }

QStringList weakFormVariantStringKeys() { return weakFormVariantList.values(); }
QString weakFormVariantToStringKey(WeakFormVariant weakFormVariant) { return weakFormVariantList[weakFormVariant]; }
WeakFormVariant weakFormVariantFromStringKey(const QString &weakFormVariant) { return weakFormVariantList.key(weakFormVariant); }

QStringList meshTypeStringKeys() { return meshTypeList.values(); }
QString meshTypeToStringKey(MeshType meshType) { return meshTypeList[meshType]; }
MeshType meshTypeFromStringKey(const QString &meshType) { return meshTypeList.key(meshType); }

QStringList physicFieldVariableCompTypeStringKeys() { return physicFieldVariableCompList.values(); }
QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp) { return physicFieldVariableCompList[physicFieldVariableComp]; }
PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp) { return physicFieldVariableCompList.key(physicFieldVariableComp); }

QStringList adaptivityTypeStringKeys() { return adaptivityTypeList.values(); }
QString adaptivityTypeToStringKey(AdaptivityMethod adaptivityType) { return adaptivityTypeList[adaptivityType]; }
AdaptivityMethod adaptivityTypeFromStringKey(const QString &adaptivityType) { return adaptivityTypeList.key(adaptivityType); }

QStringList adaptivityEstimatorStringKeys() { return adaptivityEstimatorList.values(); }
QString adaptivityEstimatorToStringKey(AdaptivityEstimator adaptivityEstimator) { return adaptivityEstimatorList[adaptivityEstimator]; }
AdaptivityEstimator adaptivityEstimatorFromStringKey(const QString &adaptivityEstimator) { return adaptivityEstimatorList.key(adaptivityEstimator); }

QStringList adaptivityStrategyStringKeys() { return adaptivityStrategyList.values(); }
QString adaptivityStrategyToStringKey(AdaptivityStrategy adaptivityStrategy) { return adaptivityStrategyList[adaptivityStrategy]; }
AdaptivityStrategy adaptivityStrategyFromStringKey(const QString &adaptivityStrategy) { return adaptivityStrategyList.key(adaptivityStrategy); }

QStringList adaptivityStrategyHPStringKeys() { return adaptivityStrategyHPList.values(); }
QString adaptivityStrategyHPToStringKey(AdaptivityStrategyHP adaptivityStrategyHP) { return adaptivityStrategyHPList[adaptivityStrategyHP]; }
AdaptivityStrategyHP adaptivityStrategyHPFromStringKey(const QString &adaptivityStrategyHP) { return adaptivityStrategyHPList.key(adaptivityStrategyHP); }

QStringList adaptivityNormTypeStringKeys() { return adaptivityNormTypeList.values(); }
QString adaptivityNormTypeToStringKey(NormType adaptivityNormType) { return adaptivityNormTypeList[adaptivityNormType]; }
NormType adaptivityNormTypeFromStringKey(const QString &adaptivityNormType) { return adaptivityNormTypeList.key(adaptivityNormType); }

QStringList timeStepMethodStringKeys() { return timeStepMethodList.values(); }
QString timeStepMethodToStringKey(TimeStepMethod timeStepMethod) { return timeStepMethodList[timeStepMethod]; }
TimeStepMethod timeStepMethodFromStringKey(const QString &timeStepMethod) { return timeStepMethodList.key(timeStepMethod); }

QStringList linearityTypeStringKeys() { return linearityTypeList.values(); }
QString linearityTypeToStringKey(LinearityType linearityType) { return linearityTypeList[linearityType]; }
LinearityType linearityTypeFromStringKey(const QString &linearityType) { return linearityTypeList.key(linearityType); }

QStringList dampingTypeStringKeys() { return dampingTypeList.values(); }
QString dampingTypeToStringKey(DampingType dampingType) { return dampingTypeList[dampingType]; }
DampingType dampingTypeFromStringKey(const QString &dampingType) { return dampingTypeList.key(dampingType); }

QStringList matrixSolverTypeStringKeys() { return matrixSolverTypeList.values(); }
QString matrixSolverTypeToStringKey(MatrixSolverType matrixSolverType) { return matrixSolverTypeList[matrixSolverType]; }
MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType) { return matrixSolverTypeList.key(matrixSolverType); }

QStringList dumpFormatStringKeys() { return dumpFormatList.values(); }
QString dumpFormatToStringKey(MatrixExportFormat format) { return dumpFormatList[format]; }
MatrixExportFormat dumpFormatFromStringKey(const QString &format) { return dumpFormatList.key(format); }

QStringList sceneViewPost3DModeStringKeys() { return sceneViewPost3DModeList.values(); }
QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode) { return sceneViewPost3DModeList[sceneViewPost3DMode]; }
SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode) { return sceneViewPost3DModeList.key(sceneViewPost3DMode); }

QStringList paletteTypeStringKeys() { return paletteTypeList.values(); }
QString paletteTypeToStringKey(PaletteType paletteType) { return paletteTypeList[paletteType]; }
PaletteType paletteTypeFromStringKey(const QString &paletteType) { return paletteTypeList.key(paletteType); }

QStringList vectorTypeStringKeys() { return vectorTypeList.values(); }
QString vectorTypeToStringKey(VectorType vectorType) { return vectorTypeList[vectorType]; }
VectorType vectorTypeFromStringKey(const QString &vectorType) { return vectorTypeList.key(vectorType); }

QStringList vectorCenterStringKeys() { return vectorCenterList.values(); }
QString vectorCenterToStringKey(VectorCenter vectorCenter) { return vectorCenterList[vectorCenter]; }
VectorCenter vectorCenterFromStringKey(const QString &vectorCenter) { return vectorCenterList.key(vectorCenter); }

QStringList dataTableTypeStringKeys() { return dataTableTypeList.values(); }
QString dataTableTypeToStringKey(DataTableType dataTableType) { return dataTableTypeList[dataTableType]; }
DataTableType dataTableTypeFromStringKey(const QString &dataTableType) { return dataTableTypeList.key(dataTableType); }

QStringList butcherTableTypeStringKeys() { return butcherTableTypeList.values(); }
QString butcherTableTypeToStringKey(ButcherTableType tableType) { return butcherTableTypeList[tableType]; }
ButcherTableType butcherTableTypeFromStringKey(const QString &tableType) { return butcherTableTypeList.key(tableType); }

QStringList iterLinearSolverDealIIMethodStringKeys() { return iterLinearSolverDealIIMethodList.values(); }
QString iterLinearSolverDealIIMethodToStringKey(IterSolverDealII type) { return iterLinearSolverDealIIMethodList[type]; }
IterSolverDealII iterLinearSolverDealIIMethodFromStringKey(const QString &type) { return iterLinearSolverDealIIMethodList.key(type); }

QStringList iterLinearSolverDealIIPreconditionerStringKeys() { return iterLinearSolverDealIIPreconditionerList.values(); }
QString iterLinearSolverDealIIPreconditionerToStringKey(PreconditionerDealII type) { return iterLinearSolverDealIIPreconditionerList[type]; }
PreconditionerDealII iterLinearSolverDealIIPreconditionerFromStringKey(const QString &type) { return iterLinearSolverDealIIPreconditionerList.key(type); }

QStringList studyTypeStringKeys() { return studyTypeList.values(); }
QString studyTypeToStringKey(StudyType type) { return studyTypeList[type]; }
StudyType studyTypeFromStringKey(const QString &type) { return studyTypeList.key(type); }

QStringList computationResultTypeStringKeys() { return computationResultTypeList.values(); }
QString computationResultTypeToStringKey(ComputationResultType type) { return computationResultTypeList[type]; }
ComputationResultType computationResultTypeFromStringKey(const QString &type) { return computationResultTypeList.key(type); }

QStringList resultRecipeTypeStringKeys() { return resultRecipeTypeList.values(); }
QString resultRecipeTypeToStringKey(ResultRecipeType type) { return resultRecipeTypeList[type]; }
ResultRecipeType resultRecipeTypeFromStringKey(const QString &type) { return resultRecipeTypeList.key(type); }

QStringList problemFunctionTypeStringKeys() { return problemFunctionTypeList.values(); }
QString problemFunctionTypeToStringKey(ProblemFunctionType type) { return problemFunctionTypeList[type]; }
ProblemFunctionType problemFunctionTypeFromStringKey(const QString &type) { return problemFunctionTypeList.key(type); }

void initLists()
{
    // coordinate list
    coordinateTypeList.insert(CoordinateType_Planar, "planar");
    coordinateTypeList.insert(CoordinateType_Axisymmetric, "axisymmetric");
    // coordinateTypeList.insert(CoordinateType_Cart, "3D");

    // Analysis Type
    analysisTypeList.insert(AnalysisType_SteadyState, "steadystate");
    analysisTypeList.insert(AnalysisType_Transient, "transient");
    analysisTypeList.insert(AnalysisType_Harmonic, "harmonic");

    // coupling type
    couplingTypeList.insert(CouplingType_Weak, "weak");
    couplingTypeList.insert(CouplingType_None, "none");

    // Weak form type
    weakFormList.insert(WeakForm_MatVol, "matvol");
    weakFormList.insert(WeakForm_MatSurf, "matsur");
    weakFormList.insert(WeakForm_VecVol, "vecvol");
    weakFormList.insert(WeakForm_VecSurf, "vecsur");

    // Weak form variant
    weakFormVariantList.insert(WeakFormVariant_Normal, "normal");
    weakFormVariantList.insert(WeakFormVariant_Residual, "residual");

    // Mesh Type
    meshTypeList.insert(MeshType_Triangle_QuadFineDivision, "triangle");
    meshTypeList.insert(MeshType_GMSH_Quad, "gmsh_quad");

    timeStepMethodList.insert(TimeStepMethod_Fixed, "fixed");
    timeStepMethodList.insert(TimeStepMethod_BDFTolerance, "adaptive");
    timeStepMethodList.insert(TimeStepMethod_BDFNumSteps, "adaptive_numsteps");

    // PHYSICFIELDVARIABLECOMP
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Scalar, "scalar");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Magnitude, "magnitude");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_X, "x");
    physicFieldVariableCompList.insert(PhysicFieldVariableComp_Y, "y");

    // post3d
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_None, "none");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ScalarView3D, "scalar");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_ScalarView3DSolid, "scalarsolid");
    sceneViewPost3DModeList.insert(SceneViewPost3DMode_Model, "model");

    // ADAPTIVITYTYPE
    adaptivityTypeList.insert(AdaptivityMethod_None, "disabled");
    adaptivityTypeList.insert(AdaptivityMethod_H, "h-adaptivity");
    adaptivityTypeList.insert(AdaptivityMethod_P, "p-adaptivity");
    adaptivityTypeList.insert(AdaptivityMethod_HP, "hp-adaptivity");

    // AdaptivityEstimator
    adaptivityEstimatorList.insert(AdaptivityEstimator_Kelly, "kelly");
    adaptivityEstimatorList.insert(AdaptivityEstimator_Uniform, "uniform");
    // adaptivityEstimatorList.insert(AdaptivityEstimator_ReferenceSpatialAndOrder, "reference_spatial_and_order");
    // adaptivityEstimatorList.insert(AdaptivityEstimator_ReferenceSpatial, "reference_spatial");
    // adaptivityEstimatorList.insert(AdaptivityEstimator_ReferenceOrder, "reference_order");

    // AdaptivityStrategy
    adaptivityStrategyList.insert(AdaptivityStrategy_FixedFractionOfCells, "fixed_fraction_of_cells");
    adaptivityStrategyList.insert(AdaptivityStrategy_FixedFractionOfTotalError, "fixed_fraction_of_total_error");
    adaptivityStrategyList.insert(AdaptivityStrategy_BalancedErrorAndCost, "balanced_error_and_cost");

    // AdaptivityStrategyHP
    adaptivityStrategyHPList.insert(AdaptivityStrategyHP_FourierSeries, "fourier_series");
    adaptivityStrategyHPList.insert(AdaptivityStrategyHP_Alternate, "alternate");

    // ADAPTIVITYNORMTYPE
    adaptivityNormTypeList.insert(NormType_H1_NORM, "h1_norm");
    adaptivityNormTypeList.insert(NormType_L2_NORM, "l2_norm");
    adaptivityNormTypeList.insert(NormType_H1_SEMINORM, "h1_seminorm");

    // MatrixSolverType
    // matrixSolverTypeList.insert(SOLVER_EMPTY, "empty");
    matrixSolverTypeList.insert(SOLVER_UMFPACK, "umfpack");
    matrixSolverTypeList.insert(SOLVER_DEALII, "dealii");
    matrixSolverTypeList.insert(SOLVER_EXTERNAL, "external");

    // dump format
    dumpFormatList.insert(EXPORT_FORMAT_PLAIN_ASCII, "plain_ascii");
    dumpFormatList.insert(EXPORT_FORMAT_MATLAB_MATIO, "matlab_mat");
    dumpFormatList.insert(EXPORT_FORMAT_MATRIX_MARKET, "matrix_market");

    // LinearityType
    linearityTypeList.insert(LinearityType_Linear, "linear");
    linearityTypeList.insert(LinearityType_Picard, "picard");
    linearityTypeList.insert(LinearityType_Newton, "newton");

    // DampingType
    dampingTypeList.insert(DampingType_Off, "disabled");
    dampingTypeList.insert(DampingType_Automatic, "automatic");
    dampingTypeList.insert(DampingType_Fixed, "fixed");

    // PaletteType
    paletteTypeList.insert(Palette_Paruly, "paruly");
    paletteTypeList.insert(Palette_Viridis, "viridis");
    paletteTypeList.insert(Palette_Inferno, "inferno");
    paletteTypeList.insert(Palette_Jet, "jet");
    paletteTypeList.insert(Palette_Agros, "agros");
    paletteTypeList.insert(Palette_HSV, "hsv");
    paletteTypeList.insert(Palette_BWAsc, "bw_ascending");
    paletteTypeList.insert(Palette_BWDesc, "bw_descending");

    // VectorType
    vectorTypeList.insert(VectorType_Arrow, "arrow");
    vectorTypeList.insert(VectorType_Cone, "cone");

    // VectorCenter
    vectorCenterList.insert(VectorCenter_Tail, "tail");
    vectorCenterList.insert(VectorCenter_Head, "head");
    vectorCenterList.insert(VectorCenter_Center, "center");

    // DataTableType
    dataTableTypeList.insert(DataTableType_CubicSpline, "cubic_spline");
    dataTableTypeList.insert(DataTableType_PiecewiseLinear, "piecewise_linear");
    dataTableTypeList.insert(DataTableType_Constant, "constant");

    // ButcherTableType
    butcherTableTypeList.insert(Explicit_HEUN_EULER_2_12_embedded, "heun-euler");
    butcherTableTypeList.insert(Explicit_BOGACKI_SHAMPINE_4_23_embedded, "bogacki-shampine");
    butcherTableTypeList.insert(Explicit_FEHLBERG_6_45_embedded, "fehlberg");
    butcherTableTypeList.insert(Explicit_CASH_KARP_6_45_embedded, "cash-karp");
    butcherTableTypeList.insert(Explicit_DORMAND_PRINCE_7_45_embedded, "dormand-prince");

    // Iterative solver - deal.II
    iterLinearSolverDealIIMethodList.insert(IterSolverDealII_CG, "cg");
    iterLinearSolverDealIIMethodList.insert(IterSolverDealII_BiCGStab, "bicgstab");
    iterLinearSolverDealIIMethodList.insert(IterSolverDealII_GMRES, "gmres");
    //iterLinearSolverMethodList.insert(IterSolverType_Richardson, "richardson");
    //iterLinearSolverMethodList.insert(IterSolverType_MinRes, "minres");
    //iterLinearSolverMethodList.insert(IterSolverType_GMRS, "gmrs");
    //iterLinearSolverMethodList.insert(IterSolverType_Relaxation, "relaxation");

    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_Identity, "identity");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_Richardson, "richardson");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_UseMatrix, "usematrix");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_Relaxation, "relaxation");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_Jacobi, "jacobi");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_SOR, "sor");
    iterLinearSolverDealIIPreconditionerList.insert(PreconditionerDealII_SSOR, "ssor");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_PSOR, "psor");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_LACSolver, "lacsolver");
    // iterLinearSolverPreconditionerTypeList.insert(PreconditionerType_Chebyshev, "chebyshev");

    // study type
    studyTypeList.insert(StudyType_Sweep, "sweep");
    studyTypeList.insert(StudyType_NSGA2, "nsga2");
    studyTypeList.insert(StudyType_NSGA3, "nsga3");
    studyTypeList.insert(StudyType_BayesOpt, "bayesopt");
    studyTypeList.insert(StudyType_NLopt, "nlopt");
    studyTypeList.insert(StudyType_Limbo, "limbo");

    // computation result type
    computationResultTypeList.insert(ComputationResultType_Functional, "functional");
    computationResultTypeList.insert(ComputationResultType_Recipe, "recipe");
    computationResultTypeList.insert(ComputationResultType_Other, "other");

    // recipes
    resultRecipeTypeList.insert(ResultRecipeType_LocalValue, "local_value");
    resultRecipeTypeList.insert(ResultRecipeType_SurfaceIntegral, "surface_integral");
    resultRecipeTypeList.insert(ResultRecipeType_VolumeIntegral, "volume_integral");

    // function type
    problemFunctionTypeList.insert(ProblemFunctionType_Analytic, "analytic");
    problemFunctionTypeList.insert(ProblemFunctionType_Interpolation, "interpolation");
}

QString errorNormString(NormType projNormType)
{
    switch (projNormType)
    {
    case NormType_H1_NORM:
        return QObject::tr("H1 norm");
    case NormType_L2_NORM:
        return QObject::tr("L2 norm");
    case NormType_H1_SEMINORM:
        return QObject::tr("H1 seminorm");
    case NormType_HDIV_NORM:
        return QObject::tr("Hdiv norm");
    case NormType_HCURL_NORM:
        return QObject::tr("Hcurl norm");
    case NormType_UNSET_NORM:
        return QObject::tr("Custom norm");
    default:
        std::cerr << "Norm '" + QString::number(projNormType).toStdString() + "' is not implemented. QString errorNormString(ProjNormType projNormType)" << endl;
        throw;
    }
}

QString analysisTypeString(AnalysisType analysisType)
{
    switch (analysisType)
    {
    case AnalysisType_SteadyState:
        return QObject::tr("Steady state");
    case AnalysisType_Transient:
        return QObject::tr("Transient");
    case AnalysisType_Harmonic:
        return QObject::tr("Harmonic");
    default:
        std::cerr << "Analysis type '" + QString::number(analysisType).toStdString() + "' is not implemented. analysisTypeString(AnalysisType analysisType)" << endl;
        throw;
    }
}

QString couplingTypeString(CouplingType couplingType)
{
    switch (couplingType)
    {
    case CouplingType_None:
        return QObject::tr("Not used");
        // case CouplingType_Hard:
        //     return QObject::tr("Hard");
    case CouplingType_Weak:
        return QObject::tr("Weak");
    default:
        return QObject::tr("Not used");
        // std::cerr << "Coupling type '" + QString::number(couplingType).toStdString() + "' is not implemented. couplingTypeString(CouplingType couplingType)" << endl;
        throw;
    }
}

QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp)
{
    switch (physicFieldVariableComp)
    {
    case PhysicFieldVariableComp_Scalar:
        return QObject::tr("Scalar");
    case PhysicFieldVariableComp_Magnitude:
        return QObject::tr("Magnitude");
    case PhysicFieldVariableComp_X:
        return Agros::problem()->config()->labelX();
    case PhysicFieldVariableComp_Y:
        return Agros::problem()->config()->labelY();
    default:
        return QObject::tr("Undefined");
    }
}

QString coordinateTypeString(CoordinateType coordinateType)
{
    return ((coordinateType == CoordinateType_Planar) ? QObject::tr("Planar") : QObject::tr("Axisymmetric"));
}

QString adaptivityTypeString(AdaptivityMethod adaptivityType)
{
    switch (adaptivityType)
    {
    case AdaptivityMethod_None:
        return QObject::tr("Disabled");
    case AdaptivityMethod_H:
        return QObject::tr("h-adaptivity");
    case AdaptivityMethod_P:
        return QObject::tr("p-adaptivity");
    case AdaptivityMethod_HP:
        return QObject::tr("hp-adaptivity");
    default:
        std::cerr << "Adaptivity type '" + QString::number(adaptivityType).toStdString() + "' is not implemented. adaptivityTypeString(AdaptivityType adaptivityType)" << endl;
        throw;
    }
}

QString adaptivityEstimatorString(AdaptivityEstimator adaptivityEstimator)
{
    switch (adaptivityEstimator)
    {
    case AdaptivityEstimator_Kelly:
        return QObject::tr("Kelly error estimator");
    case AdaptivityEstimator_Uniform:
        return QObject::tr("Uniform refinement");
    // case AdaptivityEstimator_ReferenceSpatialAndOrder:
    //     return QObject::tr("Reference solution - spatial and order");
    // case AdaptivityEstimator_ReferenceSpatial:
    //     return QObject::tr("Reference solution - spatial");
    // case AdaptivityEstimator_ReferenceOrder:
    //     return QObject::tr("Reference solution - order increase");
    default:
        std::cerr << "Adaptivity estimator '" + QString::number(adaptivityEstimator).toStdString() + "' is not implemented. adaptivityEstimatorString(AdaptivityEstimator adaptivityEstimator)" << endl;
        throw;
    }
}

QString adaptivityStrategyString(AdaptivityStrategy adaptivityStrategy)
{
    switch (adaptivityStrategy)
    {
    case AdaptivityStrategy_FixedFractionOfCells:
        return QObject::tr("Fixed fraction of number of cells");
    case AdaptivityStrategy_FixedFractionOfTotalError:
        return QObject::tr("Fixed fraction of total error");
    case AdaptivityStrategy_BalancedErrorAndCost:
        return QObject::tr("Balance reducing error and numerical cost");
    default:
        std::cerr << "Adaptivity strategy '" + QString::number(adaptivityStrategy).toStdString() + "' is not implemented. adaptivityStrategyString(AdaptivityStrategy adaptivityStrategy)" << endl;
        throw;
    }
}

QString adaptivityStrategyHPString(AdaptivityStrategyHP adaptivityStrategyHP)
{
    switch (adaptivityStrategyHP)
    {
    case AdaptivityStrategyHP_FourierSeries:
        return QObject::tr("Smoother based on Fourier series");
    case AdaptivityStrategyHP_Alternate:
        return QObject::tr("Alternate h and p");
    default:
        std::cerr << "Adaptivity strategy hp '" + QString::number(adaptivityStrategyHP).toStdString() + "' is not implemented. adaptivityStrategyHPString(AdaptivityStrategyHP adaptivityStrategyHP)" << endl;
        throw;
    }
}

QString timeStepMethodString(TimeStepMethod timeStepMethod)
{
    switch (timeStepMethod)
    {
    case TimeStepMethod_Fixed:
        return QObject::tr("BDF2 Fixed");
    case TimeStepMethod_BDFTolerance:
        return QObject::tr("BDF2 adaptive (tolerance)");
    case TimeStepMethod_BDFNumSteps:
        return QObject::tr("BDF2 adaptive (num. steps)");
    default:
        std::cerr << "Time step method '" + QString::number(timeStepMethod).toStdString() + "' is not implemented. timeStepMethodString(TimeStepMethod timeStepMethod)" << endl;
        throw;
    }
}

QString timeStepMethodString(dealii::TimeStepping::runge_kutta_method timeStepMethod)
{
    switch (timeStepMethod)
    {
    // explicit methods
    case dealii::TimeStepping::FORWARD_EULER:
        return QObject::tr("Forward Euler (expl.)");
    case dealii::TimeStepping::RK_THIRD_ORDER:
        return QObject::tr("Runge-Kutta 3rd order (expl.)");
    case dealii::TimeStepping::RK_CLASSIC_FOURTH_ORDER:
        return QObject::tr("Runge-Kutta 4th order (expl.)");

        // implicit methods
    case dealii::TimeStepping::BACKWARD_EULER:
        return QObject::tr("Backward Euler (impl.)");
    case dealii::TimeStepping::IMPLICIT_MIDPOINT:
        return QObject::tr("Midpoint (impl.)");
    case dealii::TimeStepping::CRANK_NICOLSON:
        return QObject::tr("Crank Nicolson (impl.)");
    case dealii::TimeStepping::SDIRK_TWO_STAGES:
        return QObject::tr("SDIRK two stages (impl.)");

        // embedded explicit methods
    case dealii::TimeStepping::HEUN_EULER:
        return QObject::tr("Heun Euler (embed.)");
    case dealii::TimeStepping::BOGACKI_SHAMPINE:
        return QObject::tr("Bogacki-Shampine (embed.)");
    case dealii::TimeStepping::DOPRI:
        return QObject::tr("Dopri (embed.)");
    case dealii::TimeStepping::FEHLBERG:
        return QObject::tr("Fehlberg (embed.)");
    case dealii::TimeStepping::CASH_KARP:
        return QObject::tr("Cash-Karp (embed.)");
    default:
        std::cerr << "Time step method '" + QString::number(timeStepMethod).toStdString() + "' is not implemented. timeStepMethodString(TimeStepMethod timeStepMethod)" << endl;
        throw;
    }
}

QString weakFormString(WeakFormKind weakForm)
{
    switch (weakForm)
    {
    case WeakForm_MatVol:
        return QObject::tr("Matrix volume");
    case WeakForm_MatSurf:
        return QObject::tr("Matrix surface");
    case WeakForm_VecVol:
        return QObject::tr("Vector volume");
    case WeakForm_VecSurf:
        return QObject::tr("Vector surface");
    default:
        std::cerr << "Weak form '" + QString::number(weakForm).toStdString() + "' is not implemented. weakFormString(WeakForm weakForm)" << endl;
        throw;
    }
}

QString weakFormVariantString(WeakFormVariant weakFormVariant)
{
    switch (weakFormVariant)
    {
    case WeakFormVariant_Normal:
        return QObject::tr("Normal");
    case WeakFormVariant_Residual:
        return QObject::tr("Residual");
    default:
        std::cerr << "Weak form variant '" + QString::number(weakFormVariant).toStdString() + "' is not implemented. weakFormVariantString(WeakFormVariant weakFormVariant)" << endl;
        throw;
    }
}

QString meshTypeString(MeshType meshType)
{
    switch (meshType)
    {
    case MeshType_Triangle_QuadFineDivision:
        return QObject::tr("Triangle (quad)");
    case MeshType_GMSH_Quad:
        return QObject::tr("GMSH (quad)");
    default:
        std::cerr << "Mesh type '" + QString::number(meshType).toStdString() + "' is not implemented. meshTypeString(MeshType meshType)" << endl;
        throw;
    }
}

QString paletteTypeString(PaletteType paletteType)
{
    switch (paletteType)
    {
    case Palette_Viridis:
        return QObject::tr("Viridis");
    case Palette_Inferno:
        return QObject::tr("Inferno");
    case Palette_Paruly:
        return QObject::tr("Paruly");
    case Palette_Jet:
        return QObject::tr("Jet");
    case Palette_Agros:
        return QObject::tr("Agros");
    case Palette_HSV:
        return QObject::tr("HSV");
    case Palette_BWAsc:
        return QObject::tr("B/W ascending");
    case Palette_BWDesc:
        return QObject::tr("B/W descending");
    default:
        std::cerr << "Palette type '" + QString::number(paletteType).toStdString() + "' is not implemented. paletteTypeString(PaletteType paletteType)" << endl;
        throw;
    }
}

QString vectorTypeString(VectorType vectorType)
{
    switch (vectorType)
    {
    case VectorType_Arrow:
        return QObject::tr("Arrow");
    case VectorType_Cone:
        return QObject::tr("Cone");
    default:
        std::cerr << "Vector type '" + QString::number(vectorType).toStdString() + "' is not implemented. vectorTypeString(VectorType vectorType)" << endl;
        throw;
    }
}

QString vectorCenterString(VectorCenter vectorCenter)
{
    switch (vectorCenter)
    {
    case VectorCenter_Tail:
        return QObject::tr("Tail");
    case VectorCenter_Head:
        return QObject::tr("Head");
    case VectorCenter_Center:
        return QObject::tr("Center");
    default:
        std::cerr << "Vector center '" + QString::number(vectorCenter).toStdString() + "' is not implemented. vectorCenterString(VectorCenter vectorCenter)" << endl;
        throw;
    }
}

QString matrixSolverTypeString(MatrixSolverType matrixSolverType)
{
    switch (matrixSolverType)
    {
    case SOLVER_EMPTY:
        return QObject::tr("EMPTY");
    case SOLVER_UMFPACK:
        return QObject::tr("UMFPACK");
    case SOLVER_DEALII:
        return QObject::tr("deal.II (iter.)");
    case SOLVER_EXTERNAL:
        return QObject::tr("External (out of core)");
    default:
        std::cerr << "Matrix solver type '" + QString::number(matrixSolverType).toStdString() + "' is not implemented. matrixSolverTypeString(MatrixSolverType matrixSolverType)" << endl;
        throw;
    }
}

QString dumpFormatString(MatrixExportFormat format)
{
    switch (format)
    {
    case EXPORT_FORMAT_MATLAB_MATIO:
        return QObject::tr("MATLAB MAT");
    case EXPORT_FORMAT_MATRIX_MARKET:
        return QObject::tr("Matrix Market");
    case EXPORT_FORMAT_PLAIN_ASCII:
        return QObject::tr("Plain ASCII");
    default:
        std::cerr << "Matrix dump format '" + QString::number(format).toStdString() + "' is not implemented. dumpFormatString(MatrixExportFormat format)" << endl;
        throw;
    }
}

bool isMatrixSolverIterative(MatrixSolverType type)
{
    return (type == SOLVER_DEALII);
}

QString linearityTypeString(LinearityType linearityType)
{
    switch (linearityType)
    {
    case LinearityType_Linear:
        return QObject::tr("Linear");
    case LinearityType_Picard:
        return QObject::tr("Picard's method");
    case LinearityType_Newton:
        return QObject::tr("Newton's method");
    default:
        std::cerr << "Linearity type '" + QString::number(linearityType).toStdString() + "' is not implemented. linearityTypeString(LinearityType linearityType)" << endl;
        throw;
    }
}

QString dampingTypeString(DampingType dampingType)
{
    switch (dampingType)
    {
    case DampingType_Off:
        return QObject::tr("No damping");
    case DampingType_Fixed:
        return QObject::tr("Fixed");
    case DampingType_Automatic:
        return QObject::tr("Automatic");
    default:
        std::cerr << "Damping type '" + QString::number(dampingType).toStdString() + "' is not implemented. dampingTypeString(DampingType dampingType)" << endl;
        throw;
    }
}

QString dataTableTypeString(DataTableType dataTableType)
{
    switch (dataTableType)
    {
    case DataTableType_CubicSpline:
        return QObject::tr("Cubic spline");
    case DataTableType_PiecewiseLinear:
        return QObject::tr("Piecewise linear");
    case DataTableType_Constant:
        return QObject::tr("Constant");
    default:
        std::cerr << "Data table type '" + QString::number(dataTableType).toStdString() + "' is not implemented. dataTableTypeString(DataTableType dataTableType)" << endl;
        throw;
    }
}

QString butcherTableTypeString(ButcherTableType tableType)
{
    switch (tableType)
    {
    case Explicit_HEUN_EULER_2_12_embedded:
        return QObject::tr("Heun-Euler (2,1)");
    case Explicit_BOGACKI_SHAMPINE_4_23_embedded:
        return QObject::tr("Bogacki-Shampine (2,3)");
    case Explicit_FEHLBERG_6_45_embedded:
        return QObject::tr("Fehlberg (4,5)");
    case Explicit_CASH_KARP_6_45_embedded:
        return QObject::tr("Cash-Karp (4,5)");
    case Explicit_DORMAND_PRINCE_7_45_embedded:
        return QObject::tr("Dormand-Prince (4,5)");
    default:
        std::cerr << "Butcher table type'" + QString::number(tableType).toStdString() + "' is not implemented. butcherTableTypeString(ButcherTableType tableType)" << endl;
        throw;
    }
}

QString iterLinearSolverDealIIMethodString(IterSolverDealII type)
{
    switch (type)
    {
    case IterSolverDealII_CG:
        return QObject::tr("CG");
    case IterSolverDealII_GMRES:
        return QObject::tr("GMRES");
    case IterSolverDealII_BiCGStab:
        return QObject::tr("BiCGStab");
    case IterSolverDealII_Richardson:
        return QObject::tr("Richardson");
    case IterSolverDealII_MinRes:
        return QObject::tr("MinRes");
    case IterSolverDealII_Relaxation:
        return QObject::tr("Relaxation");
    default:
        std::cerr << "Iterative solver Deal.II method '" + QString::number(type).toStdString() + "' is not implemented. iterLinearSolverDealIIMethodString(IterSolverDealII type)" << endl;
        throw;
    }
}

QString iterLinearSolverDealIIPreconditionerString(PreconditionerDealII type)
{
    switch (type)
    {
    case PreconditionerDealII_Identity:
        return QObject::tr("Identity");
    case PreconditionerDealII_Richardson:
        return QObject::tr("Richardson");
    case PreconditionerDealII_UseMatrix:
        return QObject::tr("UseMatrix");
    case PreconditionerDealII_Relaxation:
        return QObject::tr("Relaxation");
    case PreconditionerDealII_Jacobi:
        return QObject::tr("Jacobi");
    case PreconditionerDealII_SOR:
        return QObject::tr("SOR");
    case PreconditionerDealII_SSOR:
        return QObject::tr("SSOR");
    case PreconditionerDealII_PSOR:
        return QObject::tr("PSOR");
    case PreconditionerDealII_LACSolver:
        return QObject::tr("LACSolver");
    case PreconditionerDealII_Chebyshev:
        return QObject::tr("Chebyshev");
    default:
        std::cerr << "Iterative solver Deal.II preconditioner '" + QString::number(type).toStdString() + "' is not implemented. iterLinearSolverPreconditionerTypeString(PreconditionerType type)" << endl;
        throw;
    }
}

QString studyTypeString(StudyType type)
{
    switch (type)
    {
    case StudyType_Sweep:
        return QObject::tr("Sweep");
    case StudyType_NSGA2:
        return QObject::tr("NSGA2 (naturally multiobjective)");
    case StudyType_NSGA3:
        return QObject::tr("NSGA3 (naturally multiobjective)");
    case StudyType_BayesOpt:
        return QObject::tr("BayesOpt (Bayesian optimization)");
    case StudyType_Limbo:
        return QObject::tr("Limbo (Bayesian optimization)");
    case StudyType_NLopt:
        return QObject::tr("NLopt (nonlinear optimization)");
    default:
        std::cerr << "Study type '" + QString::number(type).toStdString() + "' is not implemented. studyTypeString(StudyType type)" << endl;
        throw;
    }
}

QString computationResultTypeString(ComputationResultType type)
{
    switch (type)
    {
    case ComputationResultType_Functional:
        return QObject::tr("Functional");
    case ComputationResultType_Recipe:
        return QObject::tr("Recipe");
    case ComputationResultType_Other:
        return QObject::tr("Other");
    default:
        std::cerr << "Computational result type'" + QString::number(type).toStdString() + "' is not implemented. computationResultTypeString(ComputationResultType type)" << endl;
        throw;
    }
}

QString resultRecipeTypeString(ResultRecipeType type)
{
    switch (type)
    {
    case ResultRecipeType_LocalValue:
        return QObject::tr("Local value");
    case ResultRecipeType_SurfaceIntegral:
        return QObject::tr("Surface integral");
    case ResultRecipeType_VolumeIntegral:
        return QObject::tr("Volume integral");
    default:
        std::cerr << "Result recipe type'" + QString::number(type).toStdString() + "' is not implemented. resultRecipeTypeString(resultRecipeType type)" << endl;
        throw;
    }
}

QString problemFunctionTypeString(ProblemFunctionType type)
{
    switch (type)
    {
    case ProblemFunctionType_Analytic:
        return QObject::tr("Analytic");
    case ProblemFunctionType_Interpolation:
        return QObject::tr("Interpolation");
    default:
        std::cerr << "Result recipe type'" + QString::number(type).toStdString() + "' is not implemented. problemFunctionTypeString(ProblemFunctionType type)" << endl;
        throw;
    }
}

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

#ifndef UTIL_ENUMS_H
#define UTIL_ENUMS_H


#include "util/util.h"
#include "util/table.h"


enum CoordinateType
{
    CoordinateType_Undefined = -1,
    CoordinateType_Planar = 0,
    CoordinateType_Axisymmetric = 1,
    CoordinateType_Cart = 2
};
Q_DECLARE_METATYPE(CoordinateType)

enum AnalysisType
{
    AnalysisType_Undefined = -1,
    AnalysisType_SteadyState = 0,
    AnalysisType_Transient = 2,
    AnalysisType_Harmonic = 3
};
Q_DECLARE_METATYPE(AnalysisType)

enum AdaptivityMethod
{
    AdaptivityMethod_Undefined = 1000,
    AdaptivityMethod_None = 3,
    AdaptivityMethod_H = 1,
    AdaptivityMethod_P = 2,
    AdaptivityMethod_HP = 0
};
Q_DECLARE_METATYPE(AdaptivityMethod)

enum AdaptivityEstimator
{
    AdaptivityEstimator_Undefined = -1,
    AdaptivityEstimator_Kelly = 0,
    // AdaptivityEstimator_ReferenceSpatialAndOrder = 2,
    // AdaptivityEstimator_ReferenceSpatial = 3,
    // AdaptivityEstimator_ReferenceOrder = 4,
    AdaptivityEstimator_Uniform = 5
};

enum AdaptivityStrategy
{
    AdaptivityStrategy_Undefined = -1,
    AdaptivityStrategy_FixedFractionOfCells = 0,
    AdaptivityStrategy_FixedFractionOfTotalError = 1,
    AdaptivityStrategy_BalancedErrorAndCost = 2
};

enum AdaptivityStrategyHP
{
    AdaptivityStrategyHP_Undefined = -1,
    AdaptivityStrategyHP_FourierSeries = 0,
    AdaptivityStrategyHP_Alternate = 1
};

enum LinearityType
{
    LinearityType_Undefined = -1,
    LinearityType_Linear = 0,
    LinearityType_Picard = 1,
    LinearityType_Newton = 2
};
Q_DECLARE_METATYPE(LinearityType)

enum DampingType
{
    DampingType_Undefined = -1,
    DampingType_Automatic = 0,
    DampingType_Fixed = 1,
    DampingType_Off = 2
};

enum CouplingType
{
    CouplingType_Undefined = -1,
    CouplingType_None = 0,
    CouplingType_Weak = 1
};

enum MeshType
{
    MeshType_Undefined = -1,
    MeshType_Triangle_QuadFineDivision = 0,
    MeshType_GMSH_Quad = 5,
};
Q_DECLARE_METATYPE(MeshType)

enum PhysicFieldVariableComp
{
    PhysicFieldVariableComp_Undefined = -1,
    PhysicFieldVariableComp_Scalar = 0,
    PhysicFieldVariableComp_Magnitude = 1,
    PhysicFieldVariableComp_X = 2,
    PhysicFieldVariableComp_Y = 3
};

enum WeakFormKind
{
    WeakForm_MatVol = 0,
    WeakForm_MatSurf = 1,
    WeakForm_VecVol = 2,
    WeakForm_VecSurf = 3,
    WeakForm_ExactSol = 4
};

enum WeakFormVariant
{
    WeakFormVariant_Normal = 0,
    WeakFormVariant_Residual = 1
};

enum SceneGeometryMode
{
    SceneGeometryMode_OperateOnNodes = 0,
    SceneGeometryMode_OperateOnEdges = 1,
    SceneGeometryMode_OperateOnLabels = 2
};

enum MouseSceneMode
{
    MouseSceneMode_Nothing = 0,
    MouseSceneMode_Pan = 1,
    MouseSceneMode_Rotate = 2,
    MouseSceneMode_Move = 3,
    MouseSceneMode_Add = 4
};

enum SceneModePostprocessor
{
    SceneModePostprocessor_Empty = 0,
    SceneModePostprocessor_LocalValue = 1,
    SceneModePostprocessor_SurfaceIntegral = 2,
    SceneModePostprocessor_VolumeIntegral = 3
};

enum PaletteType
{
    Palette_Paruly = 0,
    Palette_Viridis = 1,
    Palette_Inferno = 2,
    Palette_Jet = 3,
    Palette_Agros = 4,
    Palette_HSV = 11,
    Palette_BWAsc = 12,
    Palette_BWDesc = 13
};

enum ChartAxisType
{
    ChartAxis_X = 0,
    ChartAxis_Y = 1,
    ChartAxis_Length = 2
};

enum ChartMode
{
    ChartMode_Geometry = 0,
    ChartMode_Time = 1
};

enum SceneViewPost3DMode
{
    SceneViewPost3DMode_None = -1,
    SceneViewPost3DMode_ScalarView3D = 0,
    SceneViewPost3DMode_ScalarView3DSolid = 1,
    SceneViewPost3DMode_Model = 2
};

enum SceneTransformMode
{
    SceneTransformMode_Translate = 0,
    SceneTransformMode_Rotate = 1,
    SceneTransformMode_Scale = 2
};

enum VectorType
{
    VectorType_Arrow = 0,
    VectorType_Cone = 1
};

enum VectorCenter
{
    VectorCenter_Tail = 0,
    VectorCenter_Head = 1,
    VectorCenter_Center = 2
};

enum DataTableType
{
    DataTableType_Undefined = -1,
    DataTableType_CubicSpline = 0,
    DataTableType_PiecewiseLinear = 1,
    DataTableType_Constant = 2
};

enum TimeStepMethod
{
    TimeStepMethod_Undefined = -1,
    TimeStepMethod_Fixed = 0,
    TimeStepMethod_BDFTolerance = 1,
    TimeStepMethod_BDFNumSteps = 2
};

enum MatrixSolverType
{
    SOLVER_PLUGIN = 0,
    SOLVER_DEALII,
    SOLVER_EMPTY = 100
};
Q_DECLARE_METATYPE(MatrixSolverType)

enum IterSolverDealII
{
    IterSolverDealII_CG = 0,
    IterSolverDealII_BiCGStab = 1,
    IterSolverDealII_GMRES = 2,
    IterSolverDealII_Richardson = 3,
    IterSolverDealII_MinRes = 4,
    IterSolverDealII_Relaxation = 5
};

enum PreconditionerDealII
{
    PreconditionerDealII_Identity = 0,
    PreconditionerDealII_Richardson = 1,
    PreconditionerDealII_UseMatrix = 2,
    PreconditionerDealII_Relaxation = 3,
    PreconditionerDealII_Jacobi = 4,
    PreconditionerDealII_SOR = 5,
    PreconditionerDealII_SSOR = 6,
    PreconditionerDealII_PSOR = 7,
    PreconditionerDealII_LACSolver = 8,
    PreconditionerDealII_Chebyshev = 9
};

enum MatrixExportFormat
{
    /// \brief Plain ascii file
    /// lines contains row column and value
    EXPORT_FORMAT_PLAIN_ASCII = 1,
    /// Binary MATio format
    EXPORT_FORMAT_MATLAB_MATIO = 4,
    /// \brief Matrix Market which can be read by pysparse library
    EXPORT_FORMAT_MATRIX_MARKET = 3
};

enum NormType
{
    NormType_L2_NORM,
    NormType_H1_NORM,
    NormType_H1_SEMINORM,
    NormType_HCURL_NORM,
    NormType_HDIV_NORM,
    NormType_UNSET_NORM
};

enum SymFlag
{
    SymFlag_ANTISYM = -1,
    SymFlag_NONSYM = 0,
    SymFlag_SYM = 1
};

enum StudyType
{
    StudyType_Undefined = -1,
    StudyType_Sweep,
    StudyType_NSGA2,
    StudyType_BayesOpt,
    StudyType_NLopt
};

enum ResultRecipeType
{
    ResultRecipeType_LocalValue,
    ResultRecipeType_SurfaceIntegral,
    ResultRecipeType_VolumeIntegral
};

enum ComputationResultType
{
    ComputationResultType_Functional,
    ComputationResultType_Recipe,
    ComputationResultType_Other
};

enum ProblemFunctionType
{
    ProblemFunctionType_Analytic,
    ProblemFunctionType_Interpolation
};

// keys
AGROS_LIBRARY_API void initLists();

// coordinate type
AGROS_LIBRARY_API QString coordinateTypeString(CoordinateType coordinateType);
AGROS_LIBRARY_API QStringList coordinateTypeStringKeys();
AGROS_LIBRARY_API QString coordinateTypeToStringKey(CoordinateType coordinateType);
AGROS_LIBRARY_API CoordinateType coordinateTypeFromStringKey(const QString &coordinateType);

// analysis type
AGROS_LIBRARY_API QString analysisTypeString(AnalysisType analysisType);
AGROS_LIBRARY_API QStringList analysisTypeStringKeys();
AGROS_LIBRARY_API QString analysisTypeToStringKey(AnalysisType analysisType);
AGROS_LIBRARY_API AnalysisType analysisTypeFromStringKey(const QString &analysisType);

// coupling type
AGROS_LIBRARY_API QString couplingTypeString(CouplingType couplingType);
AGROS_LIBRARY_API QStringList couplingTypeStringKeys();
AGROS_LIBRARY_API QString couplingTypeToStringKey(CouplingType couplingType);
AGROS_LIBRARY_API CouplingType couplingTypeFromStringKey(const QString &couplingType);

// weakform type
AGROS_LIBRARY_API QString weakFormString(WeakFormKind weakForm);
AGROS_LIBRARY_API QStringList weakFormStringKeys();
AGROS_LIBRARY_API QString weakFormToStringKey(WeakFormKind weakForm);
AGROS_LIBRARY_API WeakFormKind weakFormFromStringKey(const QString &weakForm);

// weakform variant
AGROS_LIBRARY_API QString weakFormVariantString(WeakFormVariant weakFormVariant);
AGROS_LIBRARY_API QStringList weakFormVariantStringKeys();
AGROS_LIBRARY_API QString weakFormVariantToStringKey(WeakFormVariant weakFormVariant);
AGROS_LIBRARY_API WeakFormVariant weakFormVariantFromStringKey(const QString &weakFormVariant);

// mesh type
AGROS_LIBRARY_API QString meshTypeString(MeshType meshType);
AGROS_LIBRARY_API QStringList meshTypeStringKeys();
AGROS_LIBRARY_API QString meshTypeToStringKey(MeshType meshType);
AGROS_LIBRARY_API MeshType meshTypeFromStringKey(const QString &meshType);

// physic field variable component
AGROS_LIBRARY_API QString physicFieldVariableCompString(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_LIBRARY_API QStringList physicFieldVariableCompTypeStringKeys();
AGROS_LIBRARY_API QString physicFieldVariableCompToStringKey(PhysicFieldVariableComp physicFieldVariableComp);
AGROS_LIBRARY_API PhysicFieldVariableComp physicFieldVariableCompFromStringKey(const QString &physicFieldVariableComp);

// adaptivity type
AGROS_LIBRARY_API QString adaptivityTypeString(AdaptivityMethod adaptivityType);
AGROS_LIBRARY_API QStringList adaptivityTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityTypeToStringKey(AdaptivityMethod adaptivityType);
AGROS_LIBRARY_API AdaptivityMethod adaptivityTypeFromStringKey(const QString &adaptivityType);

// adaptivity estimator
AGROS_LIBRARY_API QString adaptivityEstimatorString(AdaptivityEstimator adaptivityEstimator);
AGROS_LIBRARY_API QStringList adaptivityEstimatorStringKeys();
AGROS_LIBRARY_API QString adaptivityEstimatorToStringKey(AdaptivityEstimator adaptivityEstimator);
AGROS_LIBRARY_API AdaptivityEstimator adaptivityEstimatorFromStringKey(const QString &adaptivityEstimator);

// adaptivity strategy
AGROS_LIBRARY_API QString adaptivityStrategyString(AdaptivityStrategy adaptivityStrategy);
AGROS_LIBRARY_API QStringList adaptivityStrategyStringKeys();
AGROS_LIBRARY_API QString adaptivityStrategyToStringKey(AdaptivityStrategy adaptivityStrategy);
AGROS_LIBRARY_API AdaptivityStrategy adaptivityStrategyFromStringKey(const QString &adaptivityStrategy);

// adaptivity strategy HP
AGROS_LIBRARY_API QString adaptivityStrategyHPString(AdaptivityStrategyHP adaptivityStrategyHP);
AGROS_LIBRARY_API QStringList adaptivityStrategyHPStringKeys();
AGROS_LIBRARY_API QString adaptivityStrategyHPToStringKey(AdaptivityStrategyHP adaptivityStrategyHP);
AGROS_LIBRARY_API AdaptivityStrategyHP adaptivityStrategyHPFromStringKey(const QString &adaptivityStrategyHP);

// adaptivity norm type
AGROS_LIBRARY_API QString errorNormString(NormType projNormType);
AGROS_LIBRARY_API QStringList adaptivityNormTypeStringKeys();
AGROS_LIBRARY_API QString adaptivityNormTypeToStringKey(NormType adaptivityNormType);
AGROS_LIBRARY_API NormType adaptivityNormTypeFromStringKey(const QString &adaptivityNormType);

// time step method
AGROS_LIBRARY_API QString timeStepMethodString(TimeStepMethod timeStepMethod);
AGROS_LIBRARY_API QStringList timeStepMethodStringKeys();
AGROS_LIBRARY_API QString timeStepMethodToStringKey(TimeStepMethod timeStepMethod);
AGROS_LIBRARY_API TimeStepMethod timeStepMethodFromStringKey(const QString &timeStepMethod);

// matrix solver type
AGROS_LIBRARY_API bool isMatrixSolverIterative(MatrixSolverType type);
AGROS_LIBRARY_API QString matrixSolverTypeString(MatrixSolverType matrixSolverType);
AGROS_LIBRARY_API QStringList matrixSolverTypeStringKeys();
AGROS_LIBRARY_API QString matrixSolverTypeToStringKey(MatrixSolverType matrixSolverType);
AGROS_LIBRARY_API MatrixSolverType matrixSolverTypeFromStringKey(const QString &matrixSolverType);

// matrix dump format
AGROS_LIBRARY_API QString dumpFormatString(MatrixExportFormat format);
AGROS_LIBRARY_API QStringList dumpFormatStringKeys();
AGROS_LIBRARY_API QString dumpFormatToStringKey(MatrixExportFormat format);
AGROS_LIBRARY_API MatrixExportFormat dumpFormatFromStringKey(const QString &format);

// linearity type
AGROS_LIBRARY_API QString linearityTypeString(LinearityType linearityType);
AGROS_LIBRARY_API QStringList linearityTypeStringKeys();
AGROS_LIBRARY_API QString linearityTypeToStringKey(LinearityType linearityType);
AGROS_LIBRARY_API LinearityType linearityTypeFromStringKey(const QString &linearityType);

// damping type
AGROS_LIBRARY_API QString dampingTypeString(DampingType dampingType);
AGROS_LIBRARY_API QStringList dampingTypeStringKeys();
AGROS_LIBRARY_API QString dampingTypeToStringKey(DampingType dampingType);
AGROS_LIBRARY_API DampingType dampingTypeFromStringKey(const QString &dampingType);

// scene view 3d mode
AGROS_LIBRARY_API QStringList sceneViewPost3DModeStringKeys();
AGROS_LIBRARY_API QString sceneViewPost3DModeToStringKey(SceneViewPost3DMode sceneViewPost3DMode);
AGROS_LIBRARY_API SceneViewPost3DMode sceneViewPost3DModeFromStringKey(const QString &sceneViewPost3DMode);

// palette type
AGROS_LIBRARY_API QStringList paletteTypeStringKeys();
AGROS_LIBRARY_API QString paletteTypeToStringKey(PaletteType paletteType);
AGROS_LIBRARY_API PaletteType paletteTypeFromStringKey(const QString &paletteType);
AGROS_LIBRARY_API QString paletteTypeString(PaletteType paletteType);

// vector type
AGROS_LIBRARY_API QString vectorTypeString(VectorType vectorType);
AGROS_LIBRARY_API QStringList vectorTypeStringKeys();
AGROS_LIBRARY_API QString vectorTypeToStringKey(VectorType vectorType);
AGROS_LIBRARY_API VectorType vectorTypeFromStringKey(const QString &vectorType);

// vector center
AGROS_LIBRARY_API QString vectorCenterString(VectorCenter vectorCenter);
AGROS_LIBRARY_API QStringList vectorCenterStringKeys();
AGROS_LIBRARY_API QString vectorCenterToStringKey(VectorCenter vectorCenter);
AGROS_LIBRARY_API VectorCenter vectorCenterFromStringKey(const QString &vectorCenter);

// data table type
AGROS_LIBRARY_API QString dataTableTypeString(DataTableType dataTableType);
AGROS_LIBRARY_API QStringList dataTableTypeStringKeys();
AGROS_LIBRARY_API QString dataTableTypeToStringKey(DataTableType dataTableType);
AGROS_LIBRARY_API DataTableType dataTableTypeFromStringKey(const QString &dataTableType);

// butcher table type
AGROS_LIBRARY_API QString butcherTableTypeString(ButcherTableType tableType);
AGROS_LIBRARY_API QStringList butcherTableTypeStringKeys();
AGROS_LIBRARY_API QString butcherTableTypeToStringKey(ButcherTableType tableType);
AGROS_LIBRARY_API ButcherTableType butcherTableTypeFromStringKey(const QString &tableType);

// iterative solver - method - deal.II
AGROS_LIBRARY_API QString iterLinearSolverDealIIMethodString(IterSolverDealII type);
AGROS_LIBRARY_API QStringList iterLinearSolverDealIIMethodStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverDealIIMethodToStringKey(IterSolverDealII type);
AGROS_LIBRARY_API IterSolverDealII iterLinearSolverDealIIMethodFromStringKey(const QString &type);

// iterative solver - preconditioner - deal.II
AGROS_LIBRARY_API QString iterLinearSolverDealIIPreconditionerString(PreconditionerDealII type);
AGROS_LIBRARY_API QStringList iterLinearSolverDealIIPreconditionerStringKeys();
AGROS_LIBRARY_API QString iterLinearSolverDealIIPreconditionerToStringKey(PreconditionerDealII type);
AGROS_LIBRARY_API PreconditionerDealII iterLinearSolverDealIIPreconditionerFromStringKey(const QString &type);

// study type
AGROS_LIBRARY_API QString studyTypeString(StudyType type);
AGROS_LIBRARY_API QStringList studyTypeStringKeys();
AGROS_LIBRARY_API QString studyTypeToStringKey(StudyType type);
AGROS_LIBRARY_API StudyType studyTypeFromStringKey(const QString &type);

// computation result type
AGROS_LIBRARY_API QString computationResultTypeString(ComputationResultType type);
AGROS_LIBRARY_API QStringList computationResultTypeStringKeys();
AGROS_LIBRARY_API QString computationResultTypeToStringKey(ComputationResultType type);
AGROS_LIBRARY_API ComputationResultType computationResultTypeFromStringKey(const QString &type);

// recipe type
AGROS_LIBRARY_API QString resultRecipeTypeString(ResultRecipeType type);
AGROS_LIBRARY_API QStringList resultRecipeTypeStringKeys();
AGROS_LIBRARY_API QString resultRecipeTypeToStringKey(ResultRecipeType type);
AGROS_LIBRARY_API ResultRecipeType resultRecipeTypeFromStringKey(const QString &type);

// function type
AGROS_LIBRARY_API QString problemFunctionTypeString(ProblemFunctionType type);
AGROS_LIBRARY_API QStringList problemFunctionTypeStringKeys();
AGROS_LIBRARY_API QString problemFunctionTypeToStringKey(ProblemFunctionType type);
AGROS_LIBRARY_API ProblemFunctionType problemFunctionTypeFromStringKey(const QString &type);

#endif // UTIL_ENUMS_H

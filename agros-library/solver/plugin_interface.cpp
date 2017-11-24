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

#include "plugin_interface.h"
#include "field.h"
#include "util/global.h"

// general
const QString GENERAL = "general";
const QString VERSION = "version";
const QString ID = "id";
const QString I = "i";
const QString J = "j";
const QString NAME = "name";
const QString TYPE = "type";
const QString VALUE = "value";

const QString CONSTANTS = "constants";
const QString MACROS = "macros";

const QString MATRIX_FORMS = "matrix_forms";
const QString MATRIX_TRANSIENT_FORMS = "matrix_transient_forms";
const QString VECTOR_FORMS = "vector_forms";
const QString ESSENTIONAL_FORMS = "essentional_forms";

const QString ANALYSES = "analyses";
const QString ANALYSIS = "analysis";
const QString EQUATIONS = "equations";
const QString ORDERINCREASE = "orderincrease";
const QString ANALYSISTYPE = "analysistype";

// processor
const QString PROCESSOR = "processor";
const QString VARIABLES = "variables";
const QString VARIABLE = "variable";
const QString EQUATION = "equation";
const QString VOLUME_ANALYSES = "volume_analyses";
const QString SURFACE_ANALYSES = "surface_analyses";

const QString ITEMS = "items";
const QString ITEM = "item";
const QString DEPENDENCY = "dependency";
const QString NONLINEARITY_PLANAR = "nonlinearity_planar";
const QString NONLINEARITY_AXI = "nonlinearity_axi";
const QString NONLINEARITY_CART = "nonlinearity_cart";
const QString SOLVERS = "solvers";
const QString COEFFICIENT = "coefficient";
const QString VARIANT = "variant";

// preprocessor
const QString PREPROCESSOR = "preprocessor";
const QString GUI = "gui";
const QString VOLUME_RECIPES = "volume_recipes";
const QString SURFACE_RECIPES = "surface_recipes";
const QString QUANTITIES = "quantities";
const QString CONDITION = "condition";
const QString DEFAULT_VALUE = "default_value";
const QString IS_SOURCE = "is_source";
const QString IS_BOOL = "is_bool";
const QString ONLY_IF = "only_if";
const QString ONLY_IF_NOT = "only_if_not";

const QString SHORTNAMEDEPENDENCE = "shortname_dependence";
const QString SHORTNAMEDEPENDENCE_HTML = "shortname_dependence_html";

// postprocessor
const QString POSTPROCESSOR = "postprocessor";
const QString LOCALVARIABLES = "localvariables";
const QString VOLUMEINTEGRALS = "volumeintegrals";
const QString SURFACEINTEGRALS = "surfaceintegrals";
const QString SHORTNAME = "shortname";
const QString SHORTNAME_HTML = "shortname_html";
const QString UNIT = "unit";
const QString UNIT_HTML = "unit_html";
const QString EXPRESSION = "expression";

const QString PLANAR = "planar";
const QString PLANAR_X = "planar_x";
const QString PLANAR_Y = "planar_y";
const QString AXI = "axi";
const QString AXI_R = "axi_r";
const QString AXI_Z = "axi_z";
const QString CART = "cart";
const QString CART_X = "cart_x";
const QString CART_Y = "cart_y";
const QString CART_Z = "cart_z";

QList<WeakFormKind> PluginFunctions::weakFormTypeList()
{
    QList<WeakFormKind> list;
    list << WeakForm_MatVol << WeakForm_MatSurf << WeakForm_VecVol << WeakForm_VecSurf << WeakForm_ExactSol;

    return list;
}

QString PluginFunctions::weakFormTypeStringEnum(WeakFormKind weakformType)
{
    switch (weakformType)
    {
    case WeakForm_MatVol:
        return("WeakForm_MatVol");
        break;
    case WeakForm_MatSurf:
        return("WeakForm_MatSurf");
        break;
    case WeakForm_VecVol:
        return("WeakForm_VecVol");
        break;
    case WeakForm_VecSurf:
        return("WeakForm_VecSurf");
        break;
    case WeakForm_ExactSol:
        return("WeakForm_ExactSol");
        break;
    default:
        assert(0);
    }
}

QList<CouplingType> PluginFunctions::couplingFormTypeList()
{
    QList<CouplingType> list;
    list << CouplingType_Weak << CouplingType_None << CouplingType_Undefined;
    return list;
}

QString PluginFunctions::couplingTypeStringEnum(CouplingType couplingType)
{
    switch (couplingType)
    {
    case CouplingType_Weak:
        return("CouplingType_Weak");
        break;
        // case CouplingType_Hard:
        //     return("CouplingType_Hard");
        //     break;
    case CouplingType_Undefined:
        return("CouplingType_Undefined");
        break;
    default:
        assert(0);
    }
}

QString PluginFunctions::couplingTypeToString(QString couplingType)
{
    if (couplingType == "hard")
        return ("CouplingType_Hard");
    if (couplingType == "weak")
        return ("CouplingType_Weak");
    if (couplingType == "none")
        return ("CouplingType_None");
    if (couplingType == "undefined")
        return ("CouplingType_Undefined");
    else
        assert(0);
}

CouplingType PluginFunctions::couplingTypeFromString(QString couplingType)
{
    // if (couplingType == "hard")
    //     return CouplingType_Hard;
    if (couplingType == "weak")
        return CouplingType_Weak;
    if (couplingType == "none")
        return CouplingType_None;
    if (couplingType == "undefined")
        return CouplingType_Undefined;
    else
        assert(0);
}

QList<LinearityType> PluginFunctions::linearityTypeList()
{
    QList<LinearityType> list;
    list << LinearityType_Linear << LinearityType_Newton << LinearityType_Picard << LinearityType_Undefined;

    return list;
}

QString PluginFunctions::linearityTypeStringEnum(LinearityType linearityType)
{
    switch (linearityType)
    {
    case LinearityType_Linear:
        return ("LinearityType_Linear");
        break;
    case LinearityType_Newton:
        return ("LinearityType_Newton");
        break;
    case LinearityType_Picard:
        return ("LinearityType_Picard");
        break;
    case LinearityType_Undefined:
        return ("LinearityType_Undefined");
        break;
    default:
        assert(0);
    }
}

QString PluginFunctions::physicFieldVariableCompStringEnum(PhysicFieldVariableComp physicFieldVariableComp)
{
    if (physicFieldVariableComp == PhysicFieldVariableComp_Scalar)
        return "PhysicFieldVariableComp_Scalar";
    else if (physicFieldVariableComp == PhysicFieldVariableComp_Magnitude)
        return "PhysicFieldVariableComp_Magnitude";
    else if (physicFieldVariableComp == PhysicFieldVariableComp_X)
        return "PhysicFieldVariableComp_X";
    else if (physicFieldVariableComp == PhysicFieldVariableComp_Y)
        return "PhysicFieldVariableComp_Y";
    else
        assert(0);
}

QList<CoordinateType> PluginFunctions::coordinateTypeList()
{
    QList<CoordinateType> list;
    list << CoordinateType_Planar << CoordinateType_Axisymmetric;

    return list;
}

QString PluginFunctions::coordinateTypeStringEnum(CoordinateType coordinateType)
{
    if (coordinateType == CoordinateType_Planar)
        return "CoordinateType_Planar";
    else if (coordinateType == CoordinateType_Axisymmetric)
        return "CoordinateType_Axisymmetric";
    else
        assert(0);
}

QString PluginFunctions::analysisTypeStringEnum(AnalysisType analysisType)
{
    if (analysisType == AnalysisType_SteadyState)
        return "AnalysisType_SteadyState";
    else if (analysisType == AnalysisType_Transient)
        return "AnalysisType_Transient";
    else if (analysisType == AnalysisType_Harmonic)
        return "AnalysisType_Harmonic";
    else
        assert(0);
}

QString PluginFunctions::boundaryTypeString(const QString boundaryName)
{
    return boundaryName.toLower().replace(" ","_");
}

// *************************************************************************************************

PluginInterface::PluginInterface() : m_module(nullptr), m_moduleJson(new PluginModule())
{
}

PluginInterface::~PluginInterface()
{
    delete m_moduleJson;
}

void PluginModule::load(const QString &fileName)
{
    // clear();

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open problem file.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();

    // general
    QJsonObject generalJson = rootJson[GENERAL].toObject();

    id = generalJson[ID].toString();
    name = generalJson[NAME].toString();

    // constants
    constants.clear();
    QJsonObject constantsJson = generalJson[CONSTANTS].toObject();
    for (QJsonObject::iterator it = constantsJson.begin(); it != constantsJson.end(); it++)
    {
        PluginConstant constant;
        constant.id = it.key();
        // QJsonValue val = it.value();
        // constant.value = val.toDouble();
        constant.value = it.value().toDouble();

        constants.append(constant);
    }
}

void PluginModule::save(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open problem file.");
        return;
    }

    // root object
    QJsonObject rootJson;

    // general
    QJsonObject generalJson;

    generalJson[VERSION] = 1;
    generalJson[ID] = id;
    generalJson[NAME] = name;

    // constants
    QJsonObject constantsJson;
    foreach (PluginConstant constant, constants)
    {
        constantsJson[constant.id] = constant.value;
    }
    generalJson[CONSTANTS] = constantsJson;

    // macros
    QJsonObject macrosJson;
    foreach (PluginMacro macro, macros)
    {
        macrosJson[macro.id] = macro.expression;
    }
    generalJson[MACROS] = macrosJson;

    // analyses
    QJsonObject analysesJson;
    foreach (PluginModuleAnalysis analysis, analyses)
    {
        QJsonObject analysisJson;

        QJsonObject configsJson;
        QMap<int, PluginModuleAnalysis::Equation>::const_iterator config = analysis.configs.constBegin();
        while (config != analysis.configs.constEnd())
        {
            QJsonObject configJson;
            configJson[TYPE] = config.value().type;
            configJson[ORDERINCREASE] = config.value().orderIncrease;

            configsJson[QString::number(config.key())] = configJson;
            ++config;
        }
        analysisJson[EQUATIONS] = configsJson;

        analysesJson[analysis.id] = analysisJson;
    }
    generalJson[ANALYSES] = analysesJson;

    rootJson[GENERAL] = generalJson;

    // processor
    QJsonObject proJson;
    {
        // volume
        {
            QJsonObject proVolumeJson;

            QJsonObject variablesJson;
            foreach (PluginWeakFormRecipe::Variable variable, weakFormRecipeVolume.variables)
            {
                variablesJson[variable.id] = variable.shortName;
            }
            proVolumeJson[VARIABLES] = variablesJson;

            QJsonObject matrixFormsJson;
            foreach (PluginWeakFormRecipe::MatrixForm form, weakFormRecipeVolume.matrixForms)
            {
                QJsonObject matrixJson;
                matrixJson[I] = form.i;
                matrixJson[J] = form.i;
                matrixJson[PLANAR] = form.planar;
                matrixJson[AXI] = form.axi;
                matrixJson[CART] = form.cart;
                matrixJson[CONDITION] = form.condition;

                matrixFormsJson[form.id] = matrixJson;
            }
            proVolumeJson[MATRIX_FORMS] = matrixFormsJson;

            QJsonObject vectorFormsJson;
            foreach (PluginWeakFormRecipe::VectorForm form, weakFormRecipeVolume.vectorForms)
            {
                QJsonObject vectorJson;
                vectorJson[I] = form.i;
                vectorJson[PLANAR] = form.planar;
                vectorJson[AXI] = form.axi;
                vectorJson[CART] = form.cart;
                vectorJson[CONDITION] = form.condition;

                vectorFormsJson[form.id] = vectorJson;
            }
            proVolumeJson[VECTOR_FORMS] = vectorFormsJson;

            proJson[VOLUME_RECIPES] = proVolumeJson;
        }

        // analyses
        QJsonObject proVolumeAnalysesJson;
        foreach (PluginWeakFormAnalysis analysis, weakFormAnalysisVolume)
        {
            // analysis (volume - only one)
            QJsonObject analysisJson;
            analysisJson[ID] = analysis.id;
            analysisJson[NAME] = analysis.name;
            analysisJson[EQUATION] = analysis.equation;

            // variables
            QJsonObject variablesJson;
            foreach (PluginWeakFormAnalysis::Variable variable, analysis.variables)
            {
                QJsonObject variableJson;
                variableJson[DEPENDENCY] = variable.dependency;
                variableJson[NONLINEARITY_PLANAR] = variable.nonlinearity_planar;
                variableJson[NONLINEARITY_AXI] = variable.nonlinearity_axi;
                variableJson[NONLINEARITY_CART] = variable.nonlinearity_cart;

                variablesJson[variable.id] = variableJson;
            }
            analysisJson[VARIABLES] = variablesJson;

            // solver
            QJsonObject solversJson;
            // only one solver
            foreach (PluginWeakFormAnalysis::Solver solver, analysis.solvers)
            {
                QJsonObject solverJson;

                // matrix forms
                QJsonObject matricesJson;
                foreach (PluginWeakFormAnalysis::Solver::Matrix form, solver.matrices)
                {
                    QJsonObject formJson;

                    matricesJson[form.id] = formJson;
                }
                solverJson[MATRIX_FORMS] = matricesJson;

                // matrix transient forms
                QJsonObject matricesTransientJson;
                foreach (PluginWeakFormAnalysis::Solver::MatrixTransient form, solver.matricesTransient)
                {
                    QJsonObject formJson;

                    matricesTransientJson[form.id] = formJson;
                }
                solverJson[MATRIX_TRANSIENT_FORMS] = matricesTransientJson;

                // vector forms
                QJsonObject vectorJson;
                foreach (PluginWeakFormAnalysis::Solver::Vector form, solver.vectors)
                {
                    QJsonObject formJson;
                    formJson[COEFFICIENT] = form.coefficient;
                    formJson[VARIANT] = form.variant;

                    vectorJson[form.id] = formJson;
                }
                solverJson[VECTOR_FORMS] = vectorJson;

                solversJson[linearityTypeToStringKey(solver.linearity)] = solverJson;
            }

            analysisJson[SOLVERS] = solversJson;
            proVolumeAnalysesJson[analysisTypeToStringKey(analysis.analysis)] = analysisJson;
        }

        proJson[VOLUME_ANALYSES] = proVolumeAnalysesJson;
    }

    // surface
    {
        QJsonObject proSurfaceJson;

        QJsonObject variablesJson;
        foreach (PluginWeakFormRecipe::Variable variable, weakFormRecipeSurface.variables)
        {
            QJsonObject variableJson;
            variableJson[SHORTNAME] = variable.shortName;

            variablesJson[variable.id] = variableJson;
        }
        proSurfaceJson[VARIABLES] = variablesJson;

        QJsonObject matrixFormsJson;
        foreach (PluginWeakFormRecipe::MatrixForm form, weakFormRecipeSurface.matrixForms)
        {
            QJsonObject matrixJson;
            matrixJson[I] = form.i;
            matrixJson[J] = form.i;
            matrixJson[PLANAR] = form.planar;
            matrixJson[AXI] = form.axi;
            matrixJson[CART] = form.cart;
            matrixJson[CONDITION] = form.condition;

            matrixFormsJson[form.id] = matrixJson;
        }
        proSurfaceJson[MATRIX_FORMS] = matrixFormsJson;

        QJsonObject vectorFormsJson;
        foreach (PluginWeakFormRecipe::VectorForm form, weakFormRecipeSurface.vectorForms)
        {
            QJsonObject vectorJson;
            vectorJson[I] = form.i;
            vectorJson[PLANAR] = form.planar;
            vectorJson[AXI] = form.axi;
            vectorJson[CART] = form.cart;
            vectorJson[CONDITION] = form.condition;

            vectorFormsJson[form.id] = vectorJson;
        }
        proSurfaceJson[VECTOR_FORMS] = vectorFormsJson;

        QJsonObject essentialFormsJson;
        foreach (PluginWeakFormRecipe::EssentialForm form, weakFormRecipeSurface.essentialForms)
        {
            QJsonObject essentialJson;
            essentialJson[I] = form.i;
            essentialJson[PLANAR] = form.planar;
            essentialJson[AXI] = form.axi;
            essentialJson[CART] = form.cart;
            essentialJson[CONDITION] = form.condition;

            essentialFormsJson[form.id] = essentialJson;
        }
        proSurfaceJson[ESSENTIONAL_FORMS] = essentialFormsJson;

        proJson[SURFACE_RECIPES] = proSurfaceJson;
    }

    QJsonObject proSurfaceAnalysesJson;
    foreach (PluginWeakFormAnalysis analysis, weakFormAnalysisSurface)
    {
        QJsonObject analysisJson;
        analysisJson[ID] = analysis.id;
        analysisJson[NAME] = analysis.name;
        analysisJson[EQUATION] = analysis.equation;

        // variables
        QJsonObject variablesJson;
        foreach (PluginWeakFormAnalysis::Variable variable, analysis.variables)
        {
            QJsonObject variableJson;
            variableJson[DEPENDENCY] = variable.dependency;
            variableJson[NONLINEARITY_PLANAR] = variable.nonlinearity_planar;
            variableJson[NONLINEARITY_AXI] = variable.nonlinearity_axi;
            variableJson[NONLINEARITY_CART] = variable.nonlinearity_cart;

            variablesJson[variable.id] = variableJson;
        }
        analysisJson[VARIABLES] = variablesJson;

        // solver
        QJsonObject solversJson;
        // only one solver
        foreach (PluginWeakFormAnalysis::Solver solver, analysis.solvers)
        {
            QJsonObject solverJson;

            // matrix forms
            QJsonObject matricesJson;
            foreach (PluginWeakFormAnalysis::Solver::Matrix form, solver.matrices)
            {
                QJsonObject formJson;

                matricesJson[form.id] = formJson;
            }
            solverJson[MATRIX_FORMS] = matricesJson;

            // matrix transient forms
            QJsonObject matricesTransientJson;
            foreach (PluginWeakFormAnalysis::Solver::MatrixTransient form, solver.matricesTransient)
            {
                QJsonObject formJson;

                matricesTransientJson[form.id] = formJson;
            }
            solverJson[MATRIX_TRANSIENT_FORMS] = matricesTransientJson;

            // vector forms
            QJsonObject vectorJson;
            foreach (PluginWeakFormAnalysis::Solver::Vector form, solver.vectors)
            {
                QJsonObject formJson;
                formJson[COEFFICIENT] = form.coefficient;
                formJson[VARIANT] = form.variant;

                vectorJson[form.id] = formJson;
            }
            solverJson[VECTOR_FORMS] = vectorJson;

            // essential forms
            QJsonObject essentialJson;
            foreach (PluginWeakFormAnalysis::Solver::Essential form, solver.essentials)
            {
                QJsonObject formJson;

                essentialJson[form.id] = formJson;
            }
            solverJson[ESSENTIONAL_FORMS] = essentialJson;

            solversJson[linearityTypeToStringKey(solver.linearity)] = solverJson;
        }

        analysisJson[SOLVERS] = solversJson;
        proSurfaceAnalysesJson[analysisTypeToStringKey(analysis.analysis)] = analysisJson;
    }
    proJson[SURFACE_ANALYSES] = proSurfaceAnalysesJson;

    rootJson[PROCESSOR] = proJson;

    // preprocessor
    QJsonObject preJson;

    QJsonObject guiJson;

    // volume groups
    QJsonArray volumeGroupsArrayJson;
    foreach (PluginPreGroup group, preVolumeGroups)
    {
        QJsonObject groupJson;
        groupJson[NAME] = group.name;

        QJsonObject quantitiesJson;
        foreach (PluginPreGroup::Quantity quant, group.quantities)
        {
            QJsonObject quantityJson;
            quantityJson[NAME] = quant.name;
            quantityJson[CONDITION] = quant.condition;
            quantityJson[DEFAULT_VALUE] = quant.default_value;
            quantityJson[SHORTNAME] = quant.shortname;
            quantityJson[SHORTNAME_HTML] = quant.shortname_html;
            quantityJson[SHORTNAMEDEPENDENCE] = quant.shortname;
            quantityJson[SHORTNAMEDEPENDENCE_HTML] = quant.shortname_html;
            quantityJson[UNIT] = quant.unit;
            quantityJson[UNIT_HTML] = quant.unit_html;
            quantityJson[IS_SOURCE] = quant.isSource;
            quantityJson[IS_BOOL] = quant.isBool;
            quantityJson[ONLY_IF] = quant.onlyIf;
            quantityJson[ONLY_IF_NOT] = quant.onlyIfNot;

            quantitiesJson[quant.id] = quantityJson;
        }
        groupJson[QUANTITIES] = quantitiesJson;

        volumeGroupsArrayJson.append(groupJson);
    }
    guiJson[VOLUME_RECIPES] = volumeGroupsArrayJson;

    // surface groups
    QJsonArray surfaceGroupsArrayJson;
    foreach (PluginPreGroup group, preSurfaceGroups)
    {
        QJsonObject groupJson;
        groupJson[NAME] = group.name;

        QJsonObject quantitiesJson;
        foreach (PluginPreGroup::Quantity quant, group.quantities)
        {
            QJsonObject quantityJson;
            quantityJson[NAME] = quant.name;
            quantityJson[CONDITION] = quant.condition;
            quantityJson[DEFAULT_VALUE] = quant.default_value;
            quantityJson[SHORTNAME] = quant.shortname;
            quantityJson[SHORTNAME_HTML] = quant.shortname_html;
            quantityJson[UNIT] = quant.unit;
            quantityJson[UNIT_HTML] = quant.unit_html;
            quantityJson[IS_SOURCE] = quant.isSource;
            quantityJson[IS_BOOL] = quant.isBool;
            quantityJson[ONLY_IF] = quant.onlyIf;
            quantityJson[ONLY_IF_NOT] = quant.onlyIfNot;

            quantitiesJson[quant.id] = quantityJson;
        }
        groupJson[QUANTITIES] = quantitiesJson;

        surfaceGroupsArrayJson.append(groupJson);
    }
    guiJson[SURFACE_RECIPES] = surfaceGroupsArrayJson;

    preJson[GUI] = guiJson;
    rootJson[PREPROCESSOR] = preJson;

    // postprocessor
    QJsonObject postJson;

    // local variables
    QJsonArray localVariablesArrayJson;
    foreach (PluginPostVariable variable, postLocalVariables)
    {
        QJsonObject variableJson;
        variableJson[NAME] = variable.name;
        variableJson[TYPE] = variable.type;
        variableJson[SHORTNAME] = variable.shortname;
        variableJson[SHORTNAME_HTML] = variable.shortname_html;
        variableJson[UNIT] = variable.unit;
        variableJson[UNIT_HTML] = variable.unit_html;

        QJsonObject expressionsJson;
        foreach (PluginPostVariable::Expression expr, variable.expresions)
        {
            QJsonObject expressionJson;
            if (variable.type == "scalar")
            {
                expressionJson[PLANAR] = expr.planar;
                expressionJson[AXI] = expr.axi;
                expressionJson[CART] = expr.cart;
            }
            else if (variable.type == "vector")
            {
                expressionJson[PLANAR_X] = expr.planar_x;
                expressionJson[PLANAR_Y] = expr.planar_y;
                expressionJson[AXI_R] = expr.axi_r;
                expressionJson[AXI_Z] = expr.axi_z;
                expressionJson[CART_X] = expr.cart_x;
                expressionJson[CART_Y] = expr.cart_y;
                expressionJson[CART_Z] = expr.cart_z;
            }
            else
                assert(0);

            expressionsJson[analysisTypeToStringKey(expr.analysis)] = expressionJson;
        }
        variableJson[EXPRESSION] = expressionsJson;

        localVariablesArrayJson.append(variableJson);
    }
    postJson[LOCALVARIABLES] = localVariablesArrayJson;

    // volume integrals
    QJsonArray volumeIntegralsArrayJson;
    foreach (PluginPostVariable variable, postVolumeIntegrals)
    {
        QJsonObject variableJson;
        variableJson[NAME] = variable.name;
        variableJson[TYPE] = "scalar";
        variableJson[SHORTNAME] = variable.shortname;
        variableJson[SHORTNAME_HTML] = variable.shortname_html;
        variableJson[UNIT] = variable.unit;
        variableJson[UNIT_HTML] = variable.unit_html;

        QJsonObject expressionsJson;
        foreach (PluginPostVariable::Expression expr, variable.expresions)
        {
            QJsonObject expressionJson;
            expressionJson[PLANAR] = expr.planar;
            expressionJson[AXI] = expr.axi;
            expressionJson[CART] = expr.axi;

            expressionsJson[analysisTypeToStringKey(expr.analysis)] = expressionJson;
        }
        variableJson[EXPRESSION] = expressionsJson;

        volumeIntegralsArrayJson.append(variableJson);
    }
    postJson[VOLUMEINTEGRALS] = volumeIntegralsArrayJson;

    // surface integrals
    QJsonArray surfaceIntegralsArrayJson;
    foreach (PluginPostVariable variable, postSurfaceIntegrals)
    {
        QJsonObject variableJson;
        variableJson[NAME] = variable.name;
        variableJson[TYPE] = "scalar";
        variableJson[SHORTNAME] = variable.shortname;
        variableJson[SHORTNAME_HTML] = variable.shortname_html;
        variableJson[UNIT] = variable.unit;
        variableJson[UNIT_HTML] = variable.unit_html;

        QJsonObject expressionsJson;
        foreach (PluginPostVariable::Expression expr, variable.expresions)
        {
            QJsonObject expressionJson;
            expressionJson[PLANAR] = expr.planar;
            expressionJson[AXI] = expr.axi;
            expressionJson[CART] = expr.axi;

            expressionsJson[analysisTypeToStringKey(expr.analysis)] = expressionJson;
        }
        variableJson[EXPRESSION] = expressionsJson;

        surfaceIntegralsArrayJson.append(variableJson);
    }
    postJson[SURFACEINTEGRALS] = surfaceIntegralsArrayJson;

    // postprocessor
    rootJson[POSTPROCESSOR] = postJson;

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson(QJsonDocument::Indented));
}

void PluginModule::clear()
{
    id.clear();
    name.clear();

    analyses.clear();
    constants.clear();
    macros.clear();

    // processor
    weakFormRecipeVolume.variables.clear();
    weakFormRecipeVolume.matrixForms.clear();
    weakFormRecipeVolume.vectorForms.clear();
    weakFormRecipeVolume.essentialForms.clear();

    weakFormRecipeSurface.variables.clear();
    weakFormRecipeSurface.matrixForms.clear();
    weakFormRecipeSurface.vectorForms.clear();
    weakFormRecipeSurface.essentialForms.clear();

    weakFormAnalysisVolume.clear();
    weakFormAnalysisSurface.clear();

    // preprocessor
    preVolumeGroups.clear();
    preSurfaceGroups.clear();

    // postprocessor
    postLocalVariables.clear();
    postVolumeIntegrals.clear();
    postSurfaceIntegrals.clear();
}

IntegralValue::IntegralScratchData::IntegralScratchData(const dealii::hp::FECollection<2> &feCollection,
                                                        const dealii::hp::QCollection<2> &quadratureFormulas,
                                                        const dealii::hp::QCollection<2-1> &faceQuadratureFormulas)
    :
      hp_fe_values(feCollection,
                   quadratureFormulas,
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(feCollection,
                        faceQuadratureFormulas,
                        dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
{}

IntegralValue::IntegralScratchData::IntegralScratchData(const IntegralScratchData &scratch_data)
    :
      hp_fe_values(scratch_data.hp_fe_values.get_fe_collection(),
                   scratch_data.hp_fe_values.get_quadrature_collection(),
                   dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_JxW_values),
      hp_fe_face_values(scratch_data.hp_fe_face_values.get_fe_collection(),
                        scratch_data.hp_fe_face_values.get_quadrature_collection(),
                        dealii::update_values | dealii::update_gradients | dealii::update_quadrature_points | dealii::update_normal_vectors | dealii::update_JxW_values)
{}


void PluginInterface::convertJson()
{
    // clear current module
    m_moduleJson->clear();

    // save to Json
    m_moduleJson->id = QString::fromStdString(m_module->general_field().id());
    m_moduleJson->name = QString::fromStdString(m_module->general_field().name());
    m_moduleJson->deformedShape = (m_module->general_field().deformed_shape().present()) ? m_module->general_field().deformed_shape().get() : false;

    // constants
    foreach (XMLModule::constant cnst, m_module->constants().constant())
    {
        PluginConstant c;
        c.id = QString::fromStdString(cnst.id());
        c.value = cnst.value();

        m_moduleJson->constants.append(c);
    }

    // macros
    if (m_module->macros().present())
    {
        foreach (XMLModule::macro mcro, m_module->macros().get().macro())
        {
            PluginMacro m;
            m.id = QString::fromStdString(mcro.id());
            m.expression = QString::fromStdString(mcro.expression());

            m_moduleJson->macros.append(m);
        }
    }

    // analyses
    for (unsigned int i = 0; i < m_module->general_field().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = m_module->general_field().analyses().analysis().at(i);

        PluginModuleAnalysis a;
        a.id = QString::fromStdString(an.id());
        a.type = analysisTypeFromStringKey(a.id);
        a.name = QString::fromStdString(an.name());
        a.solutions = an.solutions();

        // spaces
        foreach (XMLModule::space spc, m_module->spaces().space())
        {
            foreach (XMLModule::space_config config, spc.space_config())
            {
                if (a.id == QString::fromStdString(spc.analysistype()))
                {
                    PluginModuleAnalysis::Equation c;
                    c.type = QString::fromStdString(config.type());
                    c.orderIncrease = config.orderadjust();

                    a.configs[config.i()] = c;
                }
            }
        }

        m_moduleJson->analyses.append(a);
    }

    // volume weakform
    XMLModule::volume volume = m_module->volume();
    for (unsigned int i = 0; i < volume.quantity().size(); i++)
    {
        XMLModule::quantity quantity = volume.quantity().at(i);

        PluginWeakFormRecipe::Variable v;
        v.id = QString::fromStdString(quantity.id());
        v.shortName = (quantity.shortname().present()) ? QString::fromStdString(quantity.shortname().get()) : "";

        m_moduleJson->weakFormRecipeVolume.variables.append(v);
    }

    // volume weakform - matrix
    for (unsigned int i = 0; i < volume.matrix_form().size(); i++)
    {
        XMLModule::matrix_form matrix_form = volume.matrix_form().at(i);

        PluginWeakFormRecipe::MatrixForm form;
        form.id = QString::fromStdString(matrix_form.id());
        form.i = (matrix_form.i().present()) ? matrix_form.i().get() : -1;
        form.j = (matrix_form.j().present()) ? matrix_form.j().get() : -1;
        form.planar = (matrix_form.planar().present()) ? QString::fromStdString(matrix_form.planar().get()) : "";
        form.axi = (matrix_form.axi().present()) ? QString::fromStdString(matrix_form.axi().get()) : "";
        form.cart = (matrix_form.planar().present()) ? QString::fromStdString(matrix_form.planar().get()) : "";
        form.condition = (matrix_form.condition().present()) ? QString::fromStdString(matrix_form.condition().get()) : "";

        m_moduleJson->weakFormRecipeVolume.matrixForms.append(form);
    }

    // volume weakform - vector
    for (unsigned int i = 0; i < volume.vector_form().size(); i++)
    {
        XMLModule::vector_form vector_form = volume.vector_form().at(i);

        PluginWeakFormRecipe::VectorForm form;
        form.id = QString::fromStdString(vector_form.id());
        form.i = (vector_form.i().present()) ? vector_form.i().get() : -1;
        form.planar = (vector_form.planar().present()) ? QString::fromStdString(vector_form.planar().get()) : "";
        form.axi = (vector_form.axi().present()) ? QString::fromStdString(vector_form.axi().get()) : "";
        form.cart = (vector_form.planar().present()) ? QString::fromStdString(vector_form.planar().get()) : "";
        form.condition = (vector_form.condition().present()) ? QString::fromStdString(vector_form.condition().get()) : "";

        m_moduleJson->weakFormRecipeVolume.vectorForms.append(form);
    }

    // volume analyses
    for (unsigned int i = 0; i < volume.weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume weakform_volume = volume.weakforms_volume().weakform_volume().at(i);

        PluginWeakFormAnalysis weakForm;
        weakForm.id = "volume";
        weakForm.name = "Volume";
        weakForm.analysis = analysisTypeFromStringKey(QString::fromStdString(weakform_volume.analysistype()));
        weakForm.equation = QString::fromStdString(weakform_volume.equation());

        for (unsigned int j = 0; j < weakform_volume.quantity().size(); j++)
        {
            XMLModule::quantity quantity = weakform_volume.quantity().at(j);

            PluginWeakFormAnalysis::Variable v;
            v.id = QString::fromStdString(quantity.id());
            v.dependency = quantity.dependence().present() ? QString::fromStdString(quantity.dependence().get()) : "";
            v.nonlinearity_planar = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";
            v.nonlinearity_axi = quantity.nonlinearity_axi().present() ? QString::fromStdString(quantity.nonlinearity_axi().get()) : "";
            v.nonlinearity_cart = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";

            weakForm.variables.append(v);
        }

        for (unsigned int j = 0; j < weakform_volume.linearity_option().size(); j++)
        {
            XMLModule::linearity_option linearity = weakform_volume.linearity_option().at(j);

            PluginWeakFormAnalysis::Solver s;
            s.linearity = linearityTypeFromStringKey(QString::fromStdString(linearity.type()));

            for (unsigned int k = 0; k < linearity.matrix_form().size(); k++)
            {
                XMLModule::matrix_form form = linearity.matrix_form().at(k);

                PluginWeakFormAnalysis::Solver::Matrix m;
                m.id = QString::fromStdString(form.id());

                s.matrices.append(m);
            }

            for (unsigned int k = 0; k < linearity.matrix_form().size(); k++)
            {
                XMLModule::matrix_form form = linearity.matrix_form().at(k);

                PluginWeakFormAnalysis::Solver::Matrix m;
                m.id = QString::fromStdString(form.id());

                s.matrices.append(m);
            }

            for (unsigned int k = 0; k < linearity.matrix_transient_form().size(); k++)
            {
                XMLModule::matrix_transient_form form = linearity.matrix_transient_form().at(k);

                PluginWeakFormAnalysis::Solver::MatrixTransient m;
                m.id = QString::fromStdString(form.id());

                s.matricesTransient.append(m);
            }

            for (unsigned int k = 0; k < linearity.vector_form().size(); k++)
            {
                XMLModule::vector_form form = linearity.vector_form().at(k);

                PluginWeakFormAnalysis::Solver::Vector v;
                v.id = QString::fromStdString(form.id());
                v.coefficient = form.coefficient().present() ? QString::fromStdString(form.coefficient().get()).toInt() : 1;
                v.variant = form.variant().present() ? QString::fromStdString(form.variant().get()) : "";

                s.vectors.append(v);
            }

            weakForm.solvers.append(s);
        }

        m_moduleJson->weakFormAnalysisVolume.append(weakForm);
    }

    // surface weakform
    XMLModule::surface surface = m_module->surface();
    for (unsigned int i = 0; i < surface.quantity().size(); i++)
    {
        XMLModule::quantity quantity = surface.quantity().at(i);

        PluginWeakFormRecipe::Variable v;
        v.id = QString::fromStdString(quantity.id());
        v.shortName = (quantity.shortname().present()) ? QString::fromStdString(quantity.shortname().get()) : "";

        m_moduleJson->weakFormRecipeSurface.variables.append(v);
    }

    // surface weakform - matrix
    for (unsigned int i = 0; i < surface.matrix_form().size(); i++)
    {
        XMLModule::matrix_form matrix_form = surface.matrix_form().at(i);

        PluginWeakFormRecipe::MatrixForm form;
        form.id = QString::fromStdString(matrix_form.id());
        form.i = (matrix_form.i().present()) ? matrix_form.i().get() : -1;
        form.j = (matrix_form.j().present()) ? matrix_form.j().get() : -1;
        form.planar = (matrix_form.planar().present()) ? QString::fromStdString(matrix_form.planar().get()) : "";
        form.axi = (matrix_form.axi().present()) ? QString::fromStdString(matrix_form.axi().get()) : "";
        form.cart = (matrix_form.planar().present()) ? QString::fromStdString(matrix_form.planar().get()) : "";
        form.condition = (matrix_form.condition().present()) ? QString::fromStdString(matrix_form.condition().get()) : "";

        m_moduleJson->weakFormRecipeSurface.matrixForms.append(form);
    }

    // surface weakform - vector
    for (unsigned int i = 0; i < surface.vector_form().size(); i++)
    {
        XMLModule::vector_form vector_form = surface.vector_form().at(i);

        PluginWeakFormRecipe::VectorForm form;
        form.id = QString::fromStdString(vector_form.id());
        form.i = (vector_form.i().present()) ? vector_form.i().get() : -1;
        form.planar = (vector_form.planar().present()) ? QString::fromStdString(vector_form.planar().get()) : "";
        form.axi = (vector_form.axi().present()) ? QString::fromStdString(vector_form.axi().get()) : "";
        form.cart = (vector_form.planar().present()) ? QString::fromStdString(vector_form.planar().get()) : "";
        form.condition = (vector_form.condition().present()) ? QString::fromStdString(vector_form.condition().get()) : "";

        m_moduleJson->weakFormRecipeSurface.vectorForms.append(form);
    }

    // surface weakform - essential
    for (unsigned int i = 0; i < surface.essential_form().size(); i++)
    {
        XMLModule::essential_form essential_form = surface.essential_form().at(i);

        PluginWeakFormRecipe::EssentialForm form;
        form.id = QString::fromStdString(essential_form.id());
        form.i = (essential_form.i().present()) ? essential_form.i().get() : -1;
        form.planar = (essential_form.planar().present()) ? QString::fromStdString(essential_form.planar().get()) : "";
        form.axi = (essential_form.axi().present()) ? QString::fromStdString(essential_form.axi().get()) : "";
        form.cart = (essential_form.planar().present()) ? QString::fromStdString(essential_form.planar().get()) : "";
        form.condition = (essential_form.condition().present()) ? QString::fromStdString(essential_form.condition().get()) : "";

        m_moduleJson->weakFormRecipeSurface.essentialForms.append(form);
    }

    // surface analyses
    for (unsigned int i = 0; i < surface.weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface weakform_surface = surface.weakforms_surface().weakform_surface().at(i);

        for (unsigned int k = 0; k < weakform_surface.boundary().size(); k++)
        {
            XMLModule::boundary boundary = weakform_surface.boundary().at(k);

            // surface
            PluginWeakFormAnalysis weakForm;
            weakForm.id = QString::fromStdString(boundary.id());
            weakForm.name = QString::fromStdString(boundary.name());
            weakForm.analysis = analysisTypeFromStringKey(QString::fromStdString(weakform_surface.analysistype()));
            weakForm.equation = QString::fromStdString(boundary.equation());

            for (unsigned int j = 0; j < boundary.quantity().size(); j++)
            {
                XMLModule::quantity quantity = boundary.quantity().at(j);

                PluginWeakFormAnalysis::Variable v;
                v.id = QString::fromStdString(quantity.id());
                v.dependency = quantity.dependence().present() ? QString::fromStdString(quantity.dependence().get()) : "";
                v.nonlinearity_planar = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";
                v.nonlinearity_axi = quantity.nonlinearity_axi().present() ? QString::fromStdString(quantity.nonlinearity_axi().get()) : "";
                v.nonlinearity_cart = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";

                weakForm.variables.append(v);
            }

            for (unsigned int j = 0; j < boundary.linearity_option().size(); j++)
            {
                XMLModule::linearity_option linearity = boundary.linearity_option().at(j);

                PluginWeakFormAnalysis::Solver s;
                s.linearity = linearityTypeFromStringKey(QString::fromStdString(linearity.type()));

                for (unsigned int k = 0; k < linearity.matrix_form().size(); k++)
                {
                    XMLModule::matrix_form form = linearity.matrix_form().at(k);

                    PluginWeakFormAnalysis::Solver::Matrix m;
                    m.id = QString::fromStdString(form.id());

                    s.matrices.append(m);
                }

                for (unsigned int k = 0; k < linearity.matrix_form().size(); k++)
                {
                    XMLModule::matrix_form form = linearity.matrix_form().at(k);

                    PluginWeakFormAnalysis::Solver::Matrix m;
                    m.id = QString::fromStdString(form.id());

                    s.matrices.append(m);
                }

                for (unsigned int k = 0; k < linearity.matrix_transient_form().size(); k++)
                {
                    XMLModule::matrix_transient_form form = linearity.matrix_transient_form().at(k);

                    PluginWeakFormAnalysis::Solver::MatrixTransient m;
                    m.id = QString::fromStdString(form.id());

                    s.matricesTransient.append(m);
                }

                for (unsigned int k = 0; k < linearity.vector_form().size(); k++)
                {
                    XMLModule::vector_form form = linearity.vector_form().at(k);

                    PluginWeakFormAnalysis::Solver::Vector v;
                    v.id = QString::fromStdString(form.id());
                    v.coefficient = form.coefficient().present() ? QString::fromStdString(form.coefficient().get()).toInt() : 1;
                    v.variant = form.variant().present() ? QString::fromStdString(form.variant().get()) : "";

                    s.vectors.append(v);
                }

                for (unsigned int k = 0; k < linearity.essential_form().size(); k++)
                {
                    XMLModule::essential_form form = linearity.essential_form().at(k);

                    PluginWeakFormAnalysis::Solver::Vector v;
                    v.id = QString::fromStdString(form.id());

                    s.vectors.append(v);
                }

                weakForm.solvers.append(s);
            }

            m_moduleJson->weakFormAnalysisSurface.append(weakForm);
        }
    }

    // preprocessor GUI
    for (unsigned int i = 0; i < m_module->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = m_module->preprocessor().gui().at(i);

        for (unsigned int i = 0; i < ui.group().size(); i++)
        {
            XMLModule::group grp = ui.group().at(i);

            PluginPreGroup group;
            group.name = (grp.name().present()) ? this->localeName(QString::fromStdString(grp.name().get())) : "";

            for (unsigned int i = 0; i < grp.quantity().size(); i++)
            {
                XMLModule::quantity quant = grp.quantity().at(i);

                PluginPreGroup::Quantity q;
                q.id = QString::fromStdString(quant.id());
                q.name = (quant.name().present()) ? QString::fromStdString(quant.name().get()) : "";
                q.default_value = (quant.default_().present()) ? quant.default_().get() : 0.0;
                q.condition = (quant.condition().present()) ? QString::fromStdString(quant.condition().get()) : "";
                q.shortname = (quant.shortname().present()) ? QString::fromStdString(quant.shortname().get()) : "";
                q.shortname_html = (quant.shortname_html().present()) ? QString::fromStdString(quant.shortname_html().get()) : "";
                q.shortname_dependence = (quant.shortname_dependence().present()) ? QString::fromStdString(quant.shortname_dependence().get()) : "";
                q.shortname_dependence_html = (quant.shortname_dependence_html().present()) ? QString::fromStdString(quant.shortname_dependence_html().get()) : "";
                q.unit = (quant.unit().present()) ? QString::fromStdString(quant.unit().get()) : "";
                q.unit_html = (quant.unit_html().present()) ? QString::fromStdString(quant.unit_html().get()) : "";
                q.isSource = (quant.is_source().present()) ? quant.is_source().get() : false;
                q.isBool = (quant.is_bool().present()) ? quant.is_bool().get() : false;
                q.onlyIf = (quant.only_if().present()) ? QString::fromStdString(quant.only_if().get()) : "";
                q.onlyIfNot = (quant.only_if_not().present()) ? QString::fromStdString(quant.only_if_not().get()) : "";

                group.quantities.append(q);
            }
            if (ui.type() == "volume")
                m_moduleJson->preVolumeGroups.append(group);
            else if (ui.type() == "surface")
                m_moduleJson->preSurfaceGroups.append(group);
        }
    }

    // local variables
    for (unsigned int i = 0; i < m_module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = m_module->postprocessor().localvariables().localvariable().at(i);

        PluginPostVariable variable;
        variable.id = QString::fromStdString(lv.id());
        variable.name = QString::fromStdString(lv.name());
        variable.type = QString::fromStdString(lv.type());
        variable.shortname = QString::fromStdString(lv.shortname());
        variable.shortname_html = (lv.shortname_html().present()) ? QString::fromStdString(lv.shortname_html().get()) : QString::fromStdString(lv.shortname());
        variable.unit = QString::fromStdString(lv.unit());
        variable.unit_html = (lv.unit_html().present()) ? QString::fromStdString(lv.unit_html().get()) : QString::fromStdString(lv.unit());

        for (unsigned int j = 0; j < lv.expression().size(); j++)
        {
            XMLModule::expression expr = lv.expression().at(j);

            PluginPostVariable::Expression e;
            e.analysis = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));
            e.planar = (expr.planar().present()) ? QString::fromStdString(expr.planar().get()) : "";
            e.planar_x = (expr.planar_x().present()) ? QString::fromStdString(expr.planar_x().get()) : "";
            e.planar_y = (expr.planar_y().present()) ? QString::fromStdString(expr.planar_y().get()) : "";
            e.axi = (expr.axi().present()) ? QString::fromStdString(expr.axi().get()) : "";
            e.axi_r = (expr.axi_r().present()) ? QString::fromStdString(expr.axi_r().get()) : "";
            e.axi_z = (expr.axi_z().present()) ? QString::fromStdString(expr.axi_z().get()) : "";
            e.cart = (expr.planar().present()) ? QString::fromStdString(expr.planar().get()) : "";
            e.cart_x = (expr.planar_x().present()) ? QString::fromStdString(expr.planar_x().get()) : "";
            e.cart_y = (expr.planar_y().present()) ? QString::fromStdString(expr.planar_y().get()) : "";
            e.cart_z = (expr.planar_y().present()) ? QString::fromStdString(expr.planar_y().get()).replace("dy1", "dz1").replace("dy2", "dz2") : "";

            variable.expresions.append(e);
        }
        m_moduleJson->postLocalVariables.append(variable);
    }

    // volume integrals
    for (unsigned int i = 0; i < m_module->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral in = m_module->postprocessor().volumeintegrals().volumeintegral().at(i);

        PluginPostVariable variable;
        variable.id = QString::fromStdString(in.id());
        variable.name = QString::fromStdString(in.name());
        variable.type = "scalar";
        variable.shortname = QString::fromStdString(in.shortname());
        variable.shortname_html = (in.shortname_html().present()) ? QString::fromStdString(in.shortname_html().get()) : QString::fromStdString(in.shortname());
        variable.unit = QString::fromStdString(in.unit());
        variable.unit_html = (in.unit_html().present()) ? QString::fromStdString(in.unit_html().get()) : QString::fromStdString(in.unit());

        for (unsigned int j = 0; j < in.expression().size(); j++)
        {
            XMLModule::expression expr = in.expression().at(j);

            PluginPostVariable::Expression e;
            e.analysis = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));
            e.planar = (expr.planar().present()) ? QString::fromStdString(expr.planar().get()) : "";
            e.axi = (expr.axi().present()) ? QString::fromStdString(expr.axi().get()) : "";
            e.cart = (expr.planar().present()) ? QString::fromStdString(expr.planar().get()) : "";

            variable.expresions.append(e);
        }

        m_moduleJson->postVolumeIntegrals.append(variable);
    }

    // surface integrals
    for (unsigned int i = 0; i < m_module->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral in = m_module->postprocessor().surfaceintegrals().surfaceintegral().at(i);

        PluginPostVariable variable;
        variable.id = QString::fromStdString(in.id());
        variable.name = QString::fromStdString(in.name());
        variable.type = "scalar";
        variable.shortname = QString::fromStdString(in.shortname());
        variable.shortname_html = (in.shortname_html().present()) ? QString::fromStdString(in.shortname_html().get()) : QString::fromStdString(in.shortname());
        variable.unit = QString::fromStdString(in.unit());
        variable.unit_html = (in.unit_html().present()) ? QString::fromStdString(in.unit_html().get()) : QString::fromStdString(in.unit());

        for (unsigned int j = 0; j < in.expression().size(); j++)
        {
            XMLModule::expression expr = in.expression().at(j);

            PluginPostVariable::Expression e;
            e.analysis = analysisTypeFromStringKey(QString::fromStdString(expr.analysistype()));
            e.planar = (expr.planar().present()) ? QString::fromStdString(expr.planar().get()) : "";
            e.axi = (expr.axi().present()) ? QString::fromStdString(expr.axi().get()) : "";
            e.cart = (expr.planar().present()) ? QString::fromStdString(expr.planar().get()) : "";

            variable.expresions.append(e);
        }

        m_moduleJson->postSurfaceIntegrals.append(variable);
    }

    m_moduleJson->save(QString("%1/resources/modules/%2.json").arg(datadir()).arg(this->fieldId()));

    // load
    m_moduleJson->load(QString("%1/resources/modules/%2.json").arg(datadir()).arg(this->fieldId()));
}

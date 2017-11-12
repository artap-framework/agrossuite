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

// preprocessor
const QString PREPROCESSOR = "preprocessor";
const QString GUI = "gui";
const QString VOLUME_RECIPES = "volume_recipes";
const QString SURFACE_RECIPES = "surface_recipes";
const QString QUANTITIES = "quantities";
const QString CONDITION = "condition";
const QString DEFAULT_VALUE = "default_value";
const QString IS_SOURCE = "is_source";
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
    clear();

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open problem file.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();
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
            QJsonObject analysisJson;

            // items (volume - only one)
            QJsonObject itemsJson;
            foreach (PluginWeakFormAnalysis::Item item, analysis.items)
            {
                QJsonObject itemJson;
                itemJson[NAME] = item.name;
                itemJson[EQUATION] = item.equation;

                // variables
                QJsonObject variablesJson;
                foreach (PluginWeakFormAnalysis::Item::Variable variable, item.variables)
                {
                    QJsonObject variableJson;
                    variableJson[DEPENDENCY] = variable.dependency;
                    variableJson[NONLINEARITY_PLANAR] = variable.nonlinearity_planar;
                    variableJson[NONLINEARITY_AXI] = variable.nonlinearity_axi;
                    variableJson[NONLINEARITY_CART] = variable.nonlinearity_cart;

                    variablesJson[variable.id] = variableJson;
                }
                itemJson[VARIABLES] = variablesJson;

                // solver
                QJsonObject solverJson;
                // only one solver
                foreach (PluginWeakFormAnalysis::Item::Solver solver, item.solvers)
                {
                    QJsonObject solverJson;

                    // matrix forms
                    QJsonObject matricesJson;
                    foreach (PluginWeakFormAnalysis::Item::Solver::Matrix form, solver.matrices)
                    {
                        QJsonObject formJson;

                        matricesJson[form.id] = formJson;
                    }
                    solverJson[MATRIX_FORMS] = matricesJson;

                    // matrix transient forms
                    QJsonObject matricesTransientJson;
                    foreach (PluginWeakFormAnalysis::Item::Solver::MatrixTransient form, solver.matricesTransient)
                    {
                        QJsonObject formJson;

                        matricesTransientJson[form.id] = formJson;
                    }
                    solverJson[MATRIX_TRANSIENT_FORMS] = matricesTransientJson;

                    // vector forms
                    QJsonObject vectorJson;
                    foreach (PluginWeakFormAnalysis::Item::Solver::Vector form, solver.vectors)
                    {
                        QJsonObject formJson;

                        vectorJson[form.id] = formJson;
                    }
                    solverJson[VECTOR_FORMS] = vectorJson;

                    solverJson[linearityTypeToStringKey(solver.linearity)] = solverJson;
                }

                itemJson[SOLVERS] = solverJson;
                itemsJson[item.id] = itemJson;
            }
            analysisJson[ITEMS] = itemsJson;

            proVolumeAnalysesJson[analysisTypeToStringKey(analysis.analysis)] = analysisJson;
        }
        proJson[VOLUME_ANALYSES] = proVolumeAnalysesJson;

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
    }

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
            quantityJson[IS_SOURCE] = quant.is_source;
            quantityJson[SHORTNAME] = quant.shortname;
            quantityJson[SHORTNAME_HTML] = quant.shortname_html;
            quantityJson[SHORTNAMEDEPENDENCE] = quant.shortname;
            quantityJson[SHORTNAMEDEPENDENCE_HTML] = quant.shortname_html;
            quantityJson[UNIT] = quant.unit;
            quantityJson[UNIT_HTML] = quant.unit_html;

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
            quantityJson[IS_SOURCE] = quant.is_source;
            quantityJson[SHORTNAME] = quant.shortname;
            quantityJson[SHORTNAME_HTML] = quant.shortname_html;
            quantityJson[UNIT] = quant.unit;
            quantityJson[UNIT_HTML] = quant.unit_html;

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

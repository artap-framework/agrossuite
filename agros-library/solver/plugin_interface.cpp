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

// #include "../../resources_source/classes/module_xml.h"

// general
const QString GENERAL = "general";
const QString VERSION = "version";
const QString DEFORMED_SHAPE = "deformed_shape";
const QString ID = "id";
const QString I = "i";
const QString J = "j";
const QString NAME = "name";
const QString TYPE = "type";
const QString VALUE = "value";

const QString SOURCE = "source";
const QString TARGET = "target";

const QString CONSTANTS = "constants";
const QString MACROS = "macros";

const QString MATRIX_FORMS = "matrix_forms";
const QString MATRIX_TRANSIENT_FORMS = "matrix_transient_forms";
const QString VECTOR_FORMS = "vector_forms";
const QString ESSENTIONAL_FORMS = "essentional_forms";

const QString ANALYSES = "analyses";
const QString ANALYSIS = "analysis";
const QString EQUATIONS = "equations";
const QString SOLUTIONS = "solutions";
const QString ORDERINCREASE = "orderincrease";
const QString ANALYSISTYPE = "analysistype";
const QString ANALYSISSOURCETYPE = "analysissourcetype";
const QString COUPLINGTYPE = "couplingtype";

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

PluginInterface::PluginInterface() : m_moduleJson(new PluginModule())
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
        qWarning() << QString("Couldn't open problem '%1'.").arg(fileName);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();

    // general
    QJsonObject generalJson = rootJson[GENERAL].toObject();

    id = generalJson[ID].toString();
    name = generalJson[NAME].toString();
    deformedShape = generalJson[DEFORMED_SHAPE].toBool();

    // constants
    QJsonObject constantsJson = generalJson[CONSTANTS].toObject();
    for (QJsonObject::iterator it = constantsJson.begin(); it != constantsJson.end(); it++)
    {
        PluginConstant constant;
        constant.id = it.key();
        constant.value = it.value().toDouble();

        constants.append(constant);
    }

    // macros
    QJsonObject macrosJson = generalJson[MACROS].toObject();
    for (QJsonObject::iterator it = macrosJson.begin(); it != macrosJson.end(); it++)
    {
        PluginMacro macro;
        macro.id = it.key();
        macro.expression = it.value().toString();

        macros.append(macro);
    }

    // analyses
    QJsonObject analysesJson = generalJson[ANALYSES].toObject();
    for (QJsonObject::iterator it = analysesJson.begin(); it != analysesJson.end(); it++)
    {
        QJsonObject analysisJson = it.value().toObject();

        PluginModuleAnalysis analysis;
        analysis.id = it.key();
        analysis.type = analysisTypeFromStringKey(analysisJson[TYPE].toString());
        analysis.name = analysisJson[NAME].toString();
        analysis.solutions = analysisJson[SOLUTIONS].toInt();

        QJsonObject configsJson = analysisJson[EQUATIONS].toObject();
        for (QJsonObject::iterator cfg = configsJson.begin(); cfg != configsJson.end(); cfg++)
        {
            QJsonObject configJson = cfg.value().toObject();

            PluginModuleAnalysis::Equation eq;
            eq.orderIncrease = configJson[ORDERINCREASE].toInt();
            eq.type = configJson[TYPE].toString();

            analysis.configs[cfg.key().toInt()] = eq;
        }

        analyses.append(analysis);
    }

    // processor
    QJsonObject proJson = rootJson[PROCESSOR].toObject();

    QJsonObject proVolumeJson = proJson[VOLUME_RECIPES].toObject();

    // volume recipes
    {
        QJsonObject variablesJson = proVolumeJson[VARIABLES].toObject();
        for (QJsonObject::iterator it = variablesJson.begin(); it != variablesJson.end(); it++)
        {
            PluginWeakFormRecipe::Variable variable;
            variable.id = it.key();
            variable.shortName = it.value().toString();

            weakFormRecipeVolume.variables.append(variable);
        }

        QJsonObject matrixFormsJson = proVolumeJson[MATRIX_FORMS].toObject();
        for (QJsonObject::iterator it = matrixFormsJson.begin(); it != matrixFormsJson.end(); it++)
        {
            QJsonObject matrixJson = it.value().toObject();

            PluginWeakFormRecipe::MatrixForm form;
            form.i = matrixJson[I].toInt();
            form.j = matrixJson[J].toInt();
            form.planar = matrixJson[PLANAR].toString();
            form.axi = matrixJson[AXI].toString();
            form.cart = matrixJson[CART].toString();
            form.condition = matrixJson[CONDITION].toString();

            weakFormRecipeVolume.matrixForms.append(form);
        }

        QJsonObject vectorFormsJson = proVolumeJson[VECTOR_FORMS].toObject();
        for (QJsonObject::iterator it = vectorFormsJson.begin(); it != vectorFormsJson.end(); it++)
        {
            QJsonObject vectorJson = it.value().toObject();

            PluginWeakFormRecipe::VectorForm form;
            form.i = vectorJson[I].toInt();
            form.planar = vectorJson[PLANAR].toString();
            form.axi = vectorJson[AXI].toString();
            form.cart = vectorJson[CART].toString();
            form.condition = vectorJson[CONDITION].toString();

            weakFormRecipeVolume.vectorForms.append(form);
        }
    }

    // volume analyses
    QJsonArray proVolumeAnalysesJson = proJson[VOLUME_ANALYSES].toArray();
    {
        for (int ai = 0; ai < proVolumeAnalysesJson.count(); ai++)
        {
            QJsonObject analysisJson = proVolumeAnalysesJson[ai].toObject();

            PluginWeakFormAnalysis analysis;

            for (QJsonObject::iterator it = analysisJson.begin(); it != analysisJson.end(); it++)
            {
                QJsonObject itemJson = it.value().toObject();

                PluginWeakFormAnalysis::Item item;
                item.id = itemJson[ID].toString();
                item.name = itemJson[NAME].toString();
                item.equation = itemJson[EQUATION].toString();
                item.analysis = analysisTypeFromStringKey(itemJson[ANALYSISTYPE].toString());

                // variables
                QJsonObject variablesJson = itemJson[VARIABLES].toObject();
                for (QJsonObject::iterator it = variablesJson.begin(); it != variablesJson.end(); it++)
                {
                    QJsonObject variableJson = it.value().toObject();

                    PluginWeakFormAnalysis::Item::Variable variable;
                    variable.id = it.key();
                    variable.dependency = variableJson[DEPENDENCY].toString();
                    variable.nonlinearity_planar = variableJson[NONLINEARITY_PLANAR].toString();
                    variable.nonlinearity_axi = variableJson[NONLINEARITY_AXI].toString();
                    variable.nonlinearity_cart = variableJson[NONLINEARITY_CART].toString();

                    item.variables.append(variable);
                }

                // solver
                QJsonObject solversJson = itemJson[SOLVERS].toObject();

                for (QJsonObject::iterator its = solversJson.begin(); its != solversJson.end(); its++)
                {
                    QJsonObject solverJson = its.value().toObject();

                    PluginWeakFormAnalysis::Item::Solver solver;
                    solver.linearity = linearityTypeFromStringKey(its.key());

                    // matrix forms
                    QJsonObject matricesJson = solverJson[MATRIX_FORMS].toObject();
                    for (QJsonObject::iterator itm = matricesJson.begin(); itm != matricesJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Matrix form;
                        form.id = itm.key();

                        solver.matrices.append(form);
                    }

                    // matrix transient forms
                    QJsonObject matricesTransientJson = solverJson[MATRIX_TRANSIENT_FORMS].toObject();
                    for (QJsonObject::iterator itm = matricesTransientJson.begin(); itm != matricesTransientJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::MatrixTransient form;
                        form.id = itm.key();

                        solver.matricesTransient.append(form);
                    }

                    // vector forms
                    QJsonObject vectorJson = solverJson[VECTOR_FORMS].toObject();
                    for (QJsonObject::iterator itm = vectorJson.begin(); itm != vectorJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Vector form;
                        form.id = itm.key();
                        form.coefficient = formJson[COEFFICIENT].toInt();
                        form.variant = formJson[VARIANT].toString();

                        solver.vectors.append(form);
                    };

                    item.solvers.append(solver);
                }

                analysis.items.append(item);

                weakFormAnalysisVolume.append(analysis);
            }
        }
    }

    QJsonObject proSurfaceJson = proJson[SURFACE_RECIPES].toObject();

    // surface recipes
    {
        QJsonObject variablesJson = proSurfaceJson[VARIABLES].toObject();
        for (QJsonObject::iterator it = variablesJson.begin(); it != variablesJson.end(); it++)
        {
            PluginWeakFormRecipe::Variable variable;
            variable.id = it.key();
            variable.shortName = it.value().toString();

            weakFormRecipeSurface.variables.append(variable);
        }

        QJsonObject matrixFormsJson = proSurfaceJson[MATRIX_FORMS].toObject();
        for (QJsonObject::iterator it = matrixFormsJson.begin(); it != matrixFormsJson.end(); it++)
        {
            QJsonObject matrixJson = it.value().toObject();

            PluginWeakFormRecipe::MatrixForm form;
            form.i = matrixJson[I].toInt();
            form.j = matrixJson[J].toInt();
            form.planar = matrixJson[PLANAR].toString();
            form.axi = matrixJson[AXI].toString();
            form.cart = matrixJson[CART].toString();
            form.condition = matrixJson[CONDITION].toString();

            weakFormRecipeSurface.matrixForms.append(form);
        }

        QJsonObject vectorFormsJson = proSurfaceJson[VECTOR_FORMS].toObject();
        for (QJsonObject::iterator it = vectorFormsJson.begin(); it != vectorFormsJson.end(); it++)
        {
            QJsonObject vectorJson = it.value().toObject();

            PluginWeakFormRecipe::VectorForm form;
            form.i = vectorJson[I].toInt();
            form.planar = vectorJson[PLANAR].toString();
            form.axi = vectorJson[AXI].toString();
            form.cart = vectorJson[CART].toString();
            form.condition = vectorJson[CONDITION].toString();

            weakFormRecipeSurface.vectorForms.append(form);
        }

        QJsonObject essentialFormsJson = proSurfaceJson[ESSENTIONAL_FORMS].toObject();
        for (QJsonObject::iterator it = essentialFormsJson.begin(); it != essentialFormsJson.end(); it++)
        {
            QJsonObject essentialJson = it.value().toObject();

            PluginWeakFormRecipe::EssentialForm form;
            form.i = essentialJson[I].toInt();
            form.planar = essentialJson[PLANAR].toString();
            form.axi = essentialJson[AXI].toString();
            form.cart = essentialJson[CART].toString();
            form.condition = essentialJson[CONDITION].toString();

            weakFormRecipeSurface.essentialForms.append(form);
        }
    }

    // surface analyses
    QJsonArray proSurfaceAnalysesJson = proJson[SURFACE_ANALYSES].toArray();
    {
        for (int ai = 0; ai < proSurfaceAnalysesJson.count(); ai++)
        {
            QJsonObject analysisJson = proSurfaceAnalysesJson[ai].toObject();

            PluginWeakFormAnalysis analysis;

            for (QJsonObject::iterator it = analysisJson.begin(); it != analysisJson.end(); it++)
            {
                QJsonObject itemJson = it.value().toObject();

                PluginWeakFormAnalysis::Item item;
                item.id = itemJson[ID].toString();
                item.name = itemJson[NAME].toString();
                item.equation = itemJson[EQUATION].toString();
                item.analysis = analysisTypeFromStringKey(itemJson[ANALYSISTYPE].toString());

                // variables
                QJsonObject variablesJson = itemJson[VARIABLES].toObject();
                for (QJsonObject::iterator it = variablesJson.begin(); it != variablesJson.end(); it++)
                {
                    QJsonObject variableJson = it.value().toObject();

                    PluginWeakFormAnalysis::Item::Variable variable;
                    variable.id = it.key();
                    variable.dependency = variableJson[DEPENDENCY].toString();
                    variable.nonlinearity_planar = variableJson[NONLINEARITY_PLANAR].toString();
                    variable.nonlinearity_axi = variableJson[NONLINEARITY_AXI].toString();
                    variable.nonlinearity_cart = variableJson[NONLINEARITY_CART].toString();

                    item.variables.append(variable);
                }

                // solver
                QJsonObject solversJson = itemJson[SOLVERS].toObject();
                // only one solver
                for (QJsonObject::iterator its = solversJson.begin(); its != solversJson.end(); its++)
                {
                    QJsonObject solverJson = its.value().toObject();

                    PluginWeakFormAnalysis::Item::Solver solver;
                    solver.linearity = linearityTypeFromStringKey(its.key());

                    // matrix forms
                    QJsonObject matricesJson = solverJson[MATRIX_FORMS].toObject();
                    for (QJsonObject::iterator itm = matricesJson.begin(); itm != matricesJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Matrix form;
                        form.id = itm.key();

                        solver.matrices.append(form);
                    }

                    // matrix transient forms
                    QJsonObject matricesTransientJson = solverJson[MATRIX_TRANSIENT_FORMS].toObject();
                    for (QJsonObject::iterator itm = matricesTransientJson.begin(); itm != matricesTransientJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::MatrixTransient form;
                        form.id = itm.key();

                        solver.matricesTransient.append(form);
                    }

                    // vector forms
                    QJsonObject vectorJson = solverJson[VECTOR_FORMS].toObject();
                    for (QJsonObject::iterator itm = vectorJson.begin(); itm != vectorJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Vector form;
                        form.id = itm.key();
                        form.coefficient = formJson[COEFFICIENT].toInt();
                        form.variant = formJson[VARIANT].toString();

                        solver.vectors.append(form);
                    };

                    // essential forms
                    QJsonObject essentialJson = solverJson[ESSENTIONAL_FORMS].toObject();
                    for (QJsonObject::iterator itm = essentialJson.begin(); itm != essentialJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Essential form;
                        form.id = itm.key();

                        solver.essentials.append(form);
                    };

                    item.solvers.append(solver);
                }

                analysis.items.append(item);
            }

            weakFormAnalysisSurface.append(analysis);
        }
    }

    // preprocessor
    QJsonObject preJson = rootJson[PREPROCESSOR].toObject();
    QJsonObject guiJson = preJson[GUI].toObject();

    // volume groups
    QJsonArray volumeGroupsArrayJson = guiJson[VOLUME_RECIPES].toArray();
    for (int i = 0; i < volumeGroupsArrayJson.count(); i++)
    {
        QJsonObject groupJson = volumeGroupsArrayJson[i].toObject();

        PluginPreGroup group;
        group.name = groupJson[NAME].toString();

        QJsonObject quantitiesJson = groupJson[QUANTITIES].toObject();
        for (QJsonObject::iterator it = quantitiesJson.begin(); it != quantitiesJson.end(); it++)
        {
            QJsonObject quantityJson = it.value().toObject();

            PluginPreGroup::Quantity quant;
            quant.id = it.key();
            quant.name = quantityJson[NAME].toString();
            quant.condition = quantityJson[CONDITION].toString();
            quant.default_value = quantityJson[DEFAULT_VALUE].toDouble();
            quant.shortname = quantityJson[SHORTNAME].toString();
            quant.shortname_html = quantityJson[SHORTNAME_HTML].toString();
            quant.shortname_dependence = quantityJson[SHORTNAMEDEPENDENCE].toString();
            quant.shortname_dependence_html = quantityJson[SHORTNAMEDEPENDENCE_HTML].toString();
            quant.unit = quantityJson[UNIT].toString();
            quant.unit_html = quantityJson[UNIT_HTML].toString();
            quant.isSource = quantityJson[IS_SOURCE].toBool();
            quant.isBool = quantityJson[IS_BOOL].toBool();
            quant.onlyIf = quantityJson[ONLY_IF].toString();
            quant.onlyIfNot = quantityJson[ONLY_IF_NOT].toString();

            group.quantities.append(quant);
        }

        preVolumeGroups.append(group);
    }

    // volume groups
    QJsonArray surfaceGroupsArrayJson = guiJson[SURFACE_RECIPES].toArray();
    for (int i = 0; i < surfaceGroupsArrayJson.count(); i++)
    {
        QJsonObject groupJson = surfaceGroupsArrayJson[i].toObject();

        PluginPreGroup group;
        group.name = groupJson[NAME].toString();

        QJsonObject quantitiesJson = groupJson[QUANTITIES].toObject();
        for (QJsonObject::iterator it = quantitiesJson.begin(); it != quantitiesJson.end(); it++)
        {
            QJsonObject quantityJson = it.value().toObject();

            PluginPreGroup::Quantity quant;
            quant.id = it.key();
            quant.name = quantityJson[NAME].toString();
            quant.condition = quantityJson[CONDITION].toString();
            quant.default_value = quantityJson[DEFAULT_VALUE].toDouble();
            quant.shortname = quantityJson[SHORTNAME].toString();
            quant.shortname_html = quantityJson[SHORTNAME_HTML].toString();
            quant.unit = quantityJson[UNIT].toString();
            quant.unit_html = quantityJson[UNIT_HTML].toString();
            quant.isSource = quantityJson[IS_SOURCE].toBool();
            quant.isBool = quantityJson[IS_BOOL].toBool();
            quant.onlyIf = quantityJson[ONLY_IF].toString();
            quant.onlyIfNot = quantityJson[ONLY_IF_NOT].toString();

            group.quantities.append(quant);
        }

        preSurfaceGroups.append(group);
    }

    // postprocessor
    QJsonObject postJson = rootJson[POSTPROCESSOR].toObject();

    // local variables
    QJsonArray localVariablesArrayJson = postJson[LOCALVARIABLES].toArray();
    for (int i = 0; i < localVariablesArrayJson.count(); i++)
    {
        QJsonObject variableJson = localVariablesArrayJson[i].toObject();

        PluginPostVariable variable;
        variable.id = variableJson[ID].toString();
        variable.name = variableJson[NAME].toString();
        variable.type = variableJson[TYPE].toString();
        variable.shortname = variableJson[SHORTNAME].toString();
        variable.shortname_html = variableJson[SHORTNAME_HTML].toString();
        variable.unit = variableJson[UNIT].toString();
        variable.unit_html = variableJson[UNIT_HTML].toString();

        QJsonObject expressionsJson = variableJson[EXPRESSION].toObject();
        for (QJsonObject::iterator it = expressionsJson.begin(); it != expressionsJson.end(); it++)
        {
            QJsonObject expressionJson = it.value().toObject();

            PluginPostVariable::Expression expr;
            expr.analysis = analysisTypeFromStringKey(it.key());

            if (variable.type == "scalar")
            {
                expr.planar = expressionJson[PLANAR].toString();
                expr.axi = expressionJson[AXI].toString();
                expr.cart = expressionJson[CART].toString();
            }
            else if (variable.type == "vector")
            {
                expr.planar_x = expressionJson[PLANAR_X].toString();
                expr.planar_y = expressionJson[PLANAR_Y].toString();
                expr.axi_r = expressionJson[AXI_R].toString();
                expr.axi_z = expressionJson[AXI_Z].toString();
                expr.cart_x = expressionJson[CART_X].toString();
                expr.cart_y = expressionJson[CART_Y].toString();
                expr.cart_z = expressionJson[CART_Z].toString();
            }
            else
                assert(0);

            variable.expresions.append(expr);
        }

        postLocalVariables.append(variable);
    }

    // volume integrals
    QJsonArray volumeIntegralsArrayJson = postJson[VOLUMEINTEGRALS].toArray();
    for (int i = 0; i < volumeIntegralsArrayJson.count(); i++)
    {
        QJsonObject variableJson = volumeIntegralsArrayJson[i].toObject();

        PluginPostVariable variable;
        variable.id = variableJson[ID].toString();
        variable.name = variableJson[NAME].toString();
        variable.type = variableJson[TYPE].toString();
        variable.shortname = variableJson[SHORTNAME].toString();
        variable.shortname_html = variableJson[SHORTNAME_HTML].toString();
        variable.unit = variableJson[UNIT].toString();
        variable.unit_html = variableJson[UNIT_HTML].toString();

        QJsonObject expressionsJson = variableJson[EXPRESSION].toObject();
        for (QJsonObject::iterator it = expressionsJson.begin(); it != expressionsJson.end(); it++)
        {
            QJsonObject expressionJson = it.value().toObject();

            PluginPostVariable::Expression expr;
            expr.analysis = analysisTypeFromStringKey(it.key());
            expr.planar = expressionJson[PLANAR].toString();
            expr.axi = expressionJson[AXI].toString();
            expr.cart = expressionJson[CART].toString();

            variable.expresions.append(expr);
        }

        postVolumeIntegrals.append(variable);
    }

    // volume integrals
    QJsonArray surfaceIntegralsArrayJson = postJson[SURFACEINTEGRALS].toArray();
    for (int i = 0; i < surfaceIntegralsArrayJson.count(); i++)
    {
        QJsonObject variableJson = surfaceIntegralsArrayJson[i].toObject();

        PluginPostVariable variable;
        variable.id = variableJson[ID].toString();
        variable.name = variableJson[NAME].toString();
        variable.type = variableJson[TYPE].toString();
        variable.shortname = variableJson[SHORTNAME].toString();
        variable.shortname_html = variableJson[SHORTNAME_HTML].toString();
        variable.unit = variableJson[UNIT].toString();
        variable.unit_html = variableJson[UNIT_HTML].toString();

        QJsonObject expressionsJson = variableJson[EXPRESSION].toObject();
        for (QJsonObject::iterator it = expressionsJson.begin(); it != expressionsJson.end(); it++)
        {
            QJsonObject expressionJson = it.value().toObject();

            PluginPostVariable::Expression expr;
            expr.analysis = analysisTypeFromStringKey(it.key());
            expr.planar = expressionJson[PLANAR].toString();
            expr.axi = expressionJson[AXI].toString();
            expr.cart = expressionJson[CART].toString();

            variable.expresions.append(expr);
        }

        postSurfaceIntegrals.append(variable);
    }
}

void PluginModule::save(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Couldn't open problem file.";
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
        analysisJson[NAME] = analysis.name;
        analysisJson[TYPE] = analysisTypeToStringKey(analysis.type);
        analysisJson[SOLUTIONS] = analysis.solutions;

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
                matrixJson[J] = form.j;
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
        QJsonArray proVolumeAnalysesJson;
        foreach (PluginWeakFormAnalysis analysis, weakFormAnalysisVolume)
        {
            QJsonObject itemsJson;
            foreach (PluginWeakFormAnalysis::Item item, analysis.items)
            {
                // analysis (volume - only one)
                QJsonObject itemJson;
                itemJson[ID] = item.id;
                itemJson[NAME] = item.name;
                itemJson[EQUATION] = item.equation;
                itemJson[ANALYSISTYPE] = analysisTypeToStringKey(item.analysis);

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
                QJsonObject solversJson;
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
                        formJson[COEFFICIENT] = form.coefficient;
                        formJson[VARIANT] = form.variant;

                        vectorJson[form.id] = formJson;
                    }
                    solverJson[VECTOR_FORMS] = vectorJson;

                    solversJson[linearityTypeToStringKey(solver.linearity)] = solverJson;
                }

                itemJson[SOLVERS] = solversJson;

                itemsJson[item.id] = itemJson;
            }

            proVolumeAnalysesJson.append(itemsJson);
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
            matrixJson[J] = form.j;
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

    QJsonArray proSurfaceAnalysesJson;
    foreach (PluginWeakFormAnalysis analysis, weakFormAnalysisSurface)
    {
        QJsonObject itemsJson;
        foreach (PluginWeakFormAnalysis::Item item, analysis.items)
        {
            QJsonObject itemJson;
            itemJson[ID] = item.id;
            itemJson[NAME] = item.name;
            itemJson[EQUATION] = item.equation;
            itemJson[ANALYSISTYPE] = analysisTypeToStringKey(item.analysis);

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
            QJsonObject solversJson;

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
                    formJson[COEFFICIENT] = form.coefficient;
                    formJson[VARIANT] = form.variant;

                    vectorJson[form.id] = formJson;
                }
                solverJson[VECTOR_FORMS] = vectorJson;

                // essential forms
                QJsonObject essentialJson;
                foreach (PluginWeakFormAnalysis::Item::Solver::Essential form, solver.essentials)
                {
                    QJsonObject formJson;

                    essentialJson[form.id] = formJson;
                }
                solverJson[ESSENTIONAL_FORMS] = essentialJson;

                solversJson[linearityTypeToStringKey(solver.linearity)] = solverJson;
            }

            itemJson[SOLVERS] = solversJson;

            itemsJson[item.id] = itemJson;
        }
        proSurfaceAnalysesJson.append(itemsJson);
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
            quantityJson[SHORTNAMEDEPENDENCE] = quant.shortname_dependence;
            quantityJson[SHORTNAMEDEPENDENCE_HTML] = quant.shortname_dependence_html;
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
        variableJson[ID] = variable.id;
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
        variableJson[ID] = variable.id;
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
        variableJson[ID] = variable.id;
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

void PluginCoupling::load(const QString &fileName)
{
    clear();

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << QString("Couldn't open coupling file '%1'.").arg(fileName);
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject rootJson = doc.object();

    // general
    QJsonObject generalJson = rootJson[GENERAL].toObject();

    id = generalJson[ID].toString();
    name = generalJson[NAME].toString();
    source = generalJson[SOURCE].toString();
    target = generalJson[TARGET].toString();

    // constants
    QJsonObject constantsJson = generalJson[CONSTANTS].toObject();
    for (QJsonObject::iterator it = constantsJson.begin(); it != constantsJson.end(); it++)
    {
        PluginConstant constant;
        constant.id = it.key();
        constant.value = it.value().toDouble();

        constants.append(constant);
    }

    // processor
    QJsonObject proJson = rootJson[PROCESSOR].toObject();

    QJsonObject proVolumeJson = proJson[VOLUME_RECIPES].toObject();

    // volume recipes
    {
        QJsonObject variablesJson = proVolumeJson[VARIABLES].toObject();
        for (QJsonObject::iterator it = variablesJson.begin(); it != variablesJson.end(); it++)
        {
            PluginWeakFormRecipe::Variable variable;
            variable.id = it.key();
            variable.shortName = it.value().toString();

            weakFormRecipeVolume.variables.append(variable);
        }

        QJsonObject matrixFormsJson = proVolumeJson[MATRIX_FORMS].toObject();
        for (QJsonObject::iterator it = matrixFormsJson.begin(); it != matrixFormsJson.end(); it++)
        {
            QJsonObject matrixJson = it.value().toObject();

            PluginWeakFormRecipe::MatrixForm form;
            form.i = matrixJson[I].toInt();
            form.j = matrixJson[J].toInt();
            form.planar = matrixJson[PLANAR].toString();
            form.axi = matrixJson[AXI].toString();
            form.cart = matrixJson[CART].toString();
            form.condition = matrixJson[CONDITION].toString();

            weakFormRecipeVolume.matrixForms.append(form);
        }

        QJsonObject vectorFormsJson = proVolumeJson[VECTOR_FORMS].toObject();
        for (QJsonObject::iterator it = vectorFormsJson.begin(); it != vectorFormsJson.end(); it++)
        {
            QJsonObject vectorJson = it.value().toObject();

            PluginWeakFormRecipe::VectorForm form;
            form.i = vectorJson[I].toInt();
            form.planar = vectorJson[PLANAR].toString();
            form.axi = vectorJson[AXI].toString();
            form.cart = vectorJson[CART].toString();
            form.condition = vectorJson[CONDITION].toString();

            weakFormRecipeVolume.vectorForms.append(form);
        }
    }

    // volume analyses
    QJsonArray proVolumeAnalysesJson = proJson[VOLUME_ANALYSES].toArray();
    {
        for (int ai = 0; ai < proVolumeAnalysesJson.count(); ai++)
        {
            QJsonObject analysisJson = proVolumeAnalysesJson[ai].toObject();

            for (QJsonObject::iterator it = analysisJson.begin(); it != analysisJson.end(); it++)
            {
                PluginWeakFormAnalysis analysis;

                QJsonObject itemJson = it.value().toObject();

                PluginWeakFormAnalysis::Item item;
                item.analysis = analysisTypeFromStringKey(itemJson[ANALYSISTYPE].toString());
                item.id = itemJson[ID].toString();
                item.name = itemJson[NAME].toString();
                item.equation = itemJson[EQUATION].toString();
                item.analysisSource = analysisTypeFromStringKey(itemJson[ANALYSISSOURCETYPE].toString());
                item.coupling = couplingTypeFromStringKey(itemJson[COUPLINGTYPE].toString());

                // variables
                QJsonObject variablesJson = itemJson[VARIABLES].toObject();
                for (QJsonObject::iterator it = variablesJson.begin(); it != variablesJson.end(); it++)
                {
                    QJsonObject variableJson = it.value().toObject();

                    PluginWeakFormAnalysis::Item::Variable variable;
                    variable.id = it.key();
                    variable.dependency = variableJson[DEPENDENCY].toString();
                    variable.nonlinearity_planar = variableJson[NONLINEARITY_PLANAR].toString();
                    variable.nonlinearity_axi = variableJson[NONLINEARITY_AXI].toString();
                    variable.nonlinearity_cart = variableJson[NONLINEARITY_CART].toString();

                    item.variables.append(variable);
                }

                // solver
                QJsonObject solversJson = itemJson[SOLVERS].toObject();

                for (QJsonObject::iterator its = solversJson.begin(); its != solversJson.end(); its++)
                {
                    QJsonObject solverJson = its.value().toObject();

                    PluginWeakFormAnalysis::Item::Solver solver;
                    solver.linearity = linearityTypeFromStringKey(its.key());

                    // matrix forms
                    QJsonObject matricesJson = solverJson[MATRIX_FORMS].toObject();
                    for (QJsonObject::iterator itm = matricesJson.begin(); itm != matricesJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Matrix form;
                        form.id = itm.key();

                        solver.matrices.append(form);
                    }

                    // matrix transient forms
                    QJsonObject matricesTransientJson = solverJson[MATRIX_TRANSIENT_FORMS].toObject();
                    for (QJsonObject::iterator itm = matricesTransientJson.begin(); itm != matricesTransientJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::MatrixTransient form;
                        form.id = itm.key();

                        solver.matricesTransient.append(form);
                    }

                    // vector forms
                    QJsonObject vectorJson = solverJson[VECTOR_FORMS].toObject();
                    for (QJsonObject::iterator itm = vectorJson.begin(); itm != vectorJson.end(); itm++)
                    {
                        QJsonObject formJson = itm.value().toObject();

                        PluginWeakFormAnalysis::Item::Solver::Vector form;
                        form.id = itm.key();
                        form.coefficient = formJson[COEFFICIENT].toInt();
                        form.variant = formJson[VARIANT].toString();

                        solver.vectors.append(form);
                    };

                    item.solvers.append(solver);

                    analysis.items.append(item);
                }

                weakFormAnalysisVolume.append(analysis);
            }
        }
    }
}

void PluginCoupling::save(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << QString("Couldn't write coupling file '%1'.").arg(fileName);
        return;
    }

    // root object
    QJsonObject rootJson;

    // general
    QJsonObject generalJson;

    generalJson[ID] = id;
    generalJson[NAME] = name;
    generalJson[SOURCE] = source;
    generalJson[TARGET] = target;

    // constants
    QJsonObject constantsJson;
    foreach (PluginConstant constant, constants)
    {
        constantsJson[constant.id] = constant.value;
    }
    generalJson[CONSTANTS] = constantsJson;

    QJsonObject proJson;
    {
        QJsonObject proVolumeJson;

        QJsonObject matrixFormsJson;
        foreach (PluginWeakFormRecipe::MatrixForm form, weakFormRecipeVolume.matrixForms)
        {
            QJsonObject matrixJson;
            matrixJson[I] = form.i;
            matrixJson[J] = form.j;
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

        // analyses
        QJsonArray proVolumeAnalysesJson;
        foreach (PluginWeakFormAnalysis analysis, weakFormAnalysisVolume)
        {
            QJsonObject itemsJson;
            foreach (PluginWeakFormAnalysis::Item item, analysis.items)
            {
                // analysis (volume - only one)
                QJsonObject itemJson;
                itemJson[ID] = item.id;
                itemJson[NAME] = item.name;
                itemJson[EQUATION] = item.equation;
                itemJson[ANALYSISTYPE] = analysisTypeToStringKey(item.analysis);
                itemJson[ANALYSISSOURCETYPE] = analysisTypeToStringKey(item.analysisSource);
                itemJson[COUPLINGTYPE] = couplingTypeToStringKey(item.coupling);

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
                QJsonObject solversJson;
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
                        formJson[COEFFICIENT] = form.coefficient;
                        formJson[VARIANT] = form.variant;

                        vectorJson[form.id] = formJson;
                    }
                    solverJson[VECTOR_FORMS] = vectorJson;

                    solversJson[linearityTypeToStringKey(solver.linearity)] = solverJson;
                }

                itemJson[SOLVERS] = solversJson;

                itemsJson[item.id] = itemJson;
            }

            proVolumeAnalysesJson.append(itemsJson);
        }

        proJson[VOLUME_ANALYSES] = proVolumeAnalysesJson;
    }

    rootJson[PROCESSOR] = proJson;

    rootJson[GENERAL] = generalJson;

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson(QJsonDocument::Indented));
}

void PluginCoupling::clear()
{
    id.clear();
    name.clear();

    constants.clear();

    // processor
    weakFormRecipeVolume.variables.clear();
    weakFormRecipeVolume.matrixForms.clear();
    weakFormRecipeVolume.vectorForms.clear();
    weakFormRecipeVolume.essentialForms.clear();

    weakFormAnalysisVolume.clear();
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


void PluginInterface::convertJson(XMLModule::field *module)
{
    /*
    // clear current module
    m_moduleJson->clear();

    // save to Json
    m_moduleJson->id = QString::fromStdString(module->general_field().id());
    m_moduleJson->name = QString::fromStdString(module->general_field().name());
    m_moduleJson->deformedShape = (module->general_field().deformed_shape().present()) ? module->general_field().deformed_shape().get() : false;

    // constants
    foreach (XMLModule::constant cnst, module->constants().constant())
    {
        PluginConstant c;
        c.id = QString::fromStdString(cnst.id());
        c.value = cnst.value();

        m_moduleJson->constants.append(c);
    }

    // macros
    if (module->macros().present())
    {
        foreach (XMLModule::macro mcro, module->macros().get().macro())
        {
            PluginMacro m;
            m.id = QString::fromStdString(mcro.id());
            m.expression = QString::fromStdString(mcro.expression());

            m_moduleJson->macros.append(m);
        }
    }

    // analyses
    for (unsigned int i = 0; i < module->general_field().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = module->general_field().analyses().analysis().at(i);

        PluginModuleAnalysis a;
        a.id = QString::fromStdString(an.id());
        a.type = analysisTypeFromStringKey(a.id);
        a.name = QString::fromStdString(an.name());
        a.solutions = an.solutions();

        // spaces
        foreach (XMLModule::space spc, module->spaces().space())
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
    XMLModule::volume volume = module->volume();
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

        PluginWeakFormAnalysis analysis;

        // only one item
        PluginWeakFormAnalysis::Item item;
        item.id = "volume";
        item.name = "Volume";
        item.equation = QString::fromStdString(weakform_volume.equation());
        item.analysis = analysisTypeFromStringKey(QString::fromStdString(weakform_volume.analysistype()));

        for (unsigned int j = 0; j < weakform_volume.quantity().size(); j++)
        {
            XMLModule::quantity quantity = weakform_volume.quantity().at(j);

            PluginWeakFormAnalysis::Item::Variable v;
            v.id = QString::fromStdString(quantity.id());
            v.dependency = quantity.dependence().present() ? QString::fromStdString(quantity.dependence().get()) : "";
            v.nonlinearity_planar = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";
            v.nonlinearity_axi = quantity.nonlinearity_axi().present() ? QString::fromStdString(quantity.nonlinearity_axi().get()) : "";
            v.nonlinearity_cart = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";

            item.variables.append(v);
        }

        for (unsigned int j = 0; j < weakform_volume.linearity_option().size(); j++)
        {
            XMLModule::linearity_option linearity = weakform_volume.linearity_option().at(j);

            PluginWeakFormAnalysis::Item::Solver s;
            s.linearity = linearityTypeFromStringKey(QString::fromStdString(linearity.type()));

            for (unsigned int k = 0; k < linearity.matrix_form().size(); k++)
            {
                XMLModule::matrix_form form = linearity.matrix_form().at(k);

                PluginWeakFormAnalysis::Item::Solver::Matrix m;
                m.id = QString::fromStdString(form.id());

                s.matrices.append(m);
            }

            for (unsigned int k = 0; k < linearity.matrix_transient_form().size(); k++)
            {
                XMLModule::matrix_transient_form form = linearity.matrix_transient_form().at(k);

                PluginWeakFormAnalysis::Item::Solver::MatrixTransient m;
                m.id = QString::fromStdString(form.id());

                s.matricesTransient.append(m);
            }

            for (unsigned int k = 0; k < linearity.vector_form().size(); k++)
            {
                XMLModule::vector_form form = linearity.vector_form().at(k);

                PluginWeakFormAnalysis::Item::Solver::Vector v;
                v.id = QString::fromStdString(form.id());
                v.coefficient = form.coefficient().present() ? QString::fromStdString(form.coefficient().get()).toInt() : 1;
                v.variant = form.variant().present() ? QString::fromStdString(form.variant().get()) : "";

                s.vectors.append(v);
            }

            item.solvers.append(s);
        }
        analysis.items.append(item);

        m_moduleJson->weakFormAnalysisVolume.append(analysis);
    }

    // surface weakform
    XMLModule::surface surface = module->surface();
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

        PluginWeakFormAnalysis analysis;

        for (unsigned int k = 0; k < weakform_surface.boundary().size(); k++)
        {
            XMLModule::boundary boundary = weakform_surface.boundary().at(k);

            // surface
            PluginWeakFormAnalysis::Item item;
            item.id = QString::fromStdString(boundary.id());
            item.name = QString::fromStdString(boundary.name());
            item.equation = QString::fromStdString(boundary.equation());
            item.analysis = analysisTypeFromStringKey(QString::fromStdString(weakform_surface.analysistype()));

            for (unsigned int j = 0; j < boundary.quantity().size(); j++)
            {
                XMLModule::quantity quantity = boundary.quantity().at(j);

                PluginWeakFormAnalysis::Item::Variable v;
                v.id = QString::fromStdString(quantity.id());
                v.dependency = quantity.dependence().present() ? QString::fromStdString(quantity.dependence().get()) : "";
                v.nonlinearity_planar = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";
                v.nonlinearity_axi = quantity.nonlinearity_axi().present() ? QString::fromStdString(quantity.nonlinearity_axi().get()) : "";
                v.nonlinearity_cart = quantity.nonlinearity_planar().present() ? QString::fromStdString(quantity.nonlinearity_planar().get()) : "";

                item.variables.append(v);
            }

            for (unsigned int j = 0; j < boundary.linearity_option().size(); j++)
            {
                XMLModule::linearity_option linearity = boundary.linearity_option().at(j);

                PluginWeakFormAnalysis::Item::Solver s;
                s.linearity = linearityTypeFromStringKey(QString::fromStdString(linearity.type()));

                for (unsigned int k = 0; k < linearity.matrix_form().size(); k++)
                {
                    XMLModule::matrix_form form = linearity.matrix_form().at(k);

                    PluginWeakFormAnalysis::Item::Solver::Matrix m;
                    m.id = QString::fromStdString(form.id());

                    s.matrices.append(m);
                }

                for (unsigned int k = 0; k < linearity.matrix_transient_form().size(); k++)
                {
                    XMLModule::matrix_transient_form form = linearity.matrix_transient_form().at(k);

                    PluginWeakFormAnalysis::Item::Solver::MatrixTransient m;
                    m.id = QString::fromStdString(form.id());

                    s.matricesTransient.append(m);
                }

                for (unsigned int k = 0; k < linearity.vector_form().size(); k++)
                {
                    XMLModule::vector_form form = linearity.vector_form().at(k);

                    PluginWeakFormAnalysis::Item::Solver::Vector v;
                    v.id = QString::fromStdString(form.id());
                    v.coefficient = form.coefficient().present() ? QString::fromStdString(form.coefficient().get()).toInt() : 1;
                    v.variant = form.variant().present() ? QString::fromStdString(form.variant().get()) : "";

                    s.vectors.append(v);
                }

                for (unsigned int k = 0; k < linearity.essential_form().size(); k++)
                {
                    XMLModule::essential_form form = linearity.essential_form().at(k);

                    PluginWeakFormAnalysis::Item::Solver::Essential e;
                    e.id = QString::fromStdString(form.id());

                    s.essentials.append(e);
                }

                item.solvers.append(s);
            }

            analysis.items.append(item);
        }

        m_moduleJson->weakFormAnalysisSurface.append(analysis);
    }

    // preprocessor GUI
    for (unsigned int i = 0; i < module->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = module->preprocessor().gui().at(i);

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
    for (unsigned int i = 0; i < module->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = module->postprocessor().localvariables().localvariable().at(i);

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
    for (unsigned int i = 0; i < module->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral in = module->postprocessor().volumeintegrals().volumeintegral().at(i);

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
    for (unsigned int i = 0; i < module->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral in = module->postprocessor().surfaceintegrals().surfaceintegral().at(i);

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
    */
}

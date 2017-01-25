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
const QString NAME = "name";
const QString TYPE = "type";
const QString VALUE = "value";

const QString CONSTANTS = "constants";

const QString ANALYSES = "analyses";
const QString ANALYSIS = "analysis";
const QString EQUATIONS = "equations";
const QString ORDERINCREASE = "orderincrease";
const QString ANALYSISTYPE = "analysistype";

// processor
const QString PROCESSOR = "processor";
const QString VARIABLES = "variables";
const QString VARIABLE = "variable";

// preprocessor
const QString PREPROCESSOR = "preprocessor";
const QString GUI = "gui";
const QString GROUP = "group";
const QString VOLUMEGROUPS = "volumegroups";
const QString SURFACEGROUPS = "surfacegroups";
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



PluginInterface::PluginInterface() : m_module(nullptr), m_moduleJson(new PluginModule())
{
}

PluginInterface::~PluginInterface()
{
    delete m_moduleJson;
}

void PluginModule::load(const QString &fileName)
{
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
    QJsonArray constantsArrayJson;
    foreach (PluginConstant constant, constants)
    {
        QJsonObject constantJson;
        constantJson[ID] = constant.id;
        constantJson[VALUE] = constant.value;

        constantsArrayJson.append(constantJson);
    }
    generalJson[CONSTANTS] = constantsArrayJson;

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
        QJsonObject proVolumeJson;
        QJsonArray variablesJson;
        foreach (PluginWeakFormRecipe::Variable variable, weakFormRecipeVolume.variables)
        {
            QJsonObject variableJson;
            variableJson[ID] = variable.id;
            variableJson[SHORTNAME] = variable.shortName;

            variablesJson.append(variableJson);
        }

        proVolumeJson[VARIABLES] = variablesJson;
        proJson[VOLUMEGROUPS] = proVolumeJson;
    }

    {
        QJsonObject proSurfaceJson;
        QJsonArray variablesJson;
        foreach (PluginWeakFormRecipe::Variable variable, weakFormRecipeSurface.variables)
        {
            QJsonObject variableJson;
            variableJson[ID] = variable.id;
            variableJson[SHORTNAME] = variable.shortName;

            variablesJson.append(variableJson);
        }

        proSurfaceJson[VARIABLES] = variablesJson;
        proJson[SURFACEGROUPS] = proSurfaceJson;
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

        QJsonArray quantitiesArrayJson;
        foreach (PluginPreGroup::Quantity quant, group.quantities)
        {
            QJsonObject quantityJson;
            quantityJson[NAME] = quant.name;
            quantityJson[ID] = quant.id;
            quantityJson[CONDITION] = quant.condition;
            quantityJson[DEFAULT_VALUE] = quant.default_value;
            quantityJson[IS_SOURCE] = quant.is_source;
            quantityJson[SHORTNAME] = quant.shortname;
            quantityJson[SHORTNAME_HTML] = quant.shortname_html;
            quantityJson[SHORTNAMEDEPENDENCE] = quant.shortname;
            quantityJson[SHORTNAMEDEPENDENCE_HTML] = quant.shortname_html;
            quantityJson[UNIT] = quant.unit;
            quantityJson[UNIT_HTML] = quant.unit_html;

            quantitiesArrayJson.append(quantityJson);
        }
        groupJson[QUANTITIES] = quantitiesArrayJson;

        volumeGroupsArrayJson.append(groupJson);
    }
    guiJson[VOLUMEGROUPS] = volumeGroupsArrayJson;

    // surface groups
    QJsonArray surfaceGroupsArrayJson;
    foreach (PluginPreGroup group, preSurfaceGroups)
    {
        QJsonObject groupJson;
        groupJson[NAME] = group.name;

        QJsonArray quantitiesArrayJson;
        foreach (PluginPreGroup::Quantity quant, group.quantities)
        {
            QJsonObject quantityJson;
            quantityJson[NAME] = quant.name;
            quantityJson[ID] = quant.id;
            quantityJson[CONDITION] = quant.condition;
            quantityJson[DEFAULT_VALUE] = quant.default_value;
            quantityJson[IS_SOURCE] = quant.is_source;
            quantityJson[SHORTNAME] = quant.shortname;
            quantityJson[SHORTNAME_HTML] = quant.shortname_html;
            quantityJson[UNIT] = quant.unit;
            quantityJson[UNIT_HTML] = quant.unit_html;

            quantitiesArrayJson.append(quantityJson);
        }
        groupJson[QUANTITIES] = quantitiesArrayJson;

        surfaceGroupsArrayJson.append(groupJson);
    }
    guiJson[SURFACEGROUPS] = surfaceGroupsArrayJson;

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

        QJsonArray expressionsArrayJson;
        foreach (PluginPostVariable::Expression expr, variable.expresions)
        {
            QJsonObject expressionJson;
            expressionJson[ANALYSIS] = expr.analysis;
            if (variable.type == "scalar")
            {
                expressionJson[PLANAR] = expr.planar;
                expressionJson[AXI] = expr.axi;
                expressionJson[CART] = expr.axi;
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

            expressionsArrayJson.append(expressionJson);
        }
        variableJson[EXPRESSION] = expressionsArrayJson;

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

        QJsonArray expressionsArrayJson;
        foreach (PluginPostVariable::Expression expr, variable.expresions)
        {
            QJsonObject expressionJson;
            expressionJson[ANALYSIS] = expr.analysis;
            expressionJson[PLANAR] = expr.planar;
            expressionJson[AXI] = expr.axi;
            expressionJson[CART] = expr.axi;

            expressionsArrayJson.append(expressionJson);
        }
        variableJson[EXPRESSION] = expressionsArrayJson;

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

        QJsonArray expressionsArrayJson;
        foreach (PluginPostVariable::Expression expr, variable.expresions)
        {
            QJsonObject expressionJson;
            expressionJson[ANALYSIS] = expr.analysis;
            expressionJson[PLANAR] = expr.planar;
            expressionJson[AXI] = expr.axi;
            expressionJson[CART] = expr.axi;

            expressionsArrayJson.append(expressionJson);
        }
        variableJson[EXPRESSION] = expressionsArrayJson;

        surfaceIntegralsArrayJson.append(variableJson);
    }
    postJson[SURFACEINTEGRALS] = surfaceIntegralsArrayJson;

    // postprocessor
    rootJson[POSTPROCESSOR] = postJson;

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson(QJsonDocument::Indented));
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

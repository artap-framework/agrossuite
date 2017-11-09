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

#include "field.h"
#include "util/global.h"

#include "problem.h"
#include "solver/problem_config.h"
#include "scene.h"
#include "scenemarker.h"
#include "module.h"
#include "logview.h"

#include "plugin_interface.h"

#include "../../resources_source/classes/module_xml.h"
#include "../../resources_source/classes/problem_a2d_31_xml.h"

FieldInfo::FieldInfo(QString fieldId)
    : m_plugin(nullptr), m_numberOfSolutions(0), m_hermesMarkerToAgrosLabelConversion(nullptr), m_labelAreas(nullptr)
{    
    assert(!fieldId.isEmpty());
    m_fieldId = fieldId;

    // read plugin
    try
    {
        m_plugin = Agros::loadPlugin(m_fieldId);

    }
    catch (AgrosPluginException &e)
    {
        Agros::log()->printError("Solver", "Cannot load plugin");
        throw;
    }

    assert(m_plugin);

    setStringKeys();
    clear();

    // default analysis
    setAnalysisType(analyses().begin().key());

    convertJson();
}

FieldInfo::~FieldInfo()
{
    // delete m_plugin;
}

void FieldInfo::convertJson()
{
    // clear current module
    m_plugin->moduleJson()->clear();

    // save to Json
    m_plugin->moduleJson()->id = QString::fromStdString(m_plugin->module()->general_field().id());
    m_plugin->moduleJson()->name = QString::fromStdString(m_plugin->module()->general_field().name());
    m_plugin->moduleJson()->deformedShape = (m_plugin->module()->general_field().deformed_shape().present()) ? m_plugin->module()->general_field().deformed_shape().get() : false;

    // constants
    foreach (XMLModule::constant cnst, m_plugin->module()->constants().constant())
    {
        PluginConstant c;
        c.id = QString::fromStdString(cnst.id());
        c.value = cnst.value();

        m_plugin->moduleJson()->constants.append(c);
    }

    // macros
    if (m_plugin->module()->macros().present())
    {
        foreach (XMLModule::macro mcro, m_plugin->module()->macros().get().macro())
        {
            PluginMacro m;
            m.id = QString::fromStdString(mcro.id());
            m.expression = QString::fromStdString(mcro.expression());

            m_plugin->moduleJson()->macros.append(m);
        }
    }

    // analyses
    for (unsigned int i = 0; i < m_plugin->module()->general_field().analyses().analysis().size(); i++)
    {
        XMLModule::analysis an = m_plugin->module()->general_field().analyses().analysis().at(i);

        PluginModuleAnalysis a;
        a.id = QString::fromStdString(an.id());
        a.type = analysisTypeFromStringKey(a.id);
        a.name = QString::fromStdString(an.name());
        a.solutions = an.solutions();

        // spaces
        foreach (XMLModule::space spc, m_plugin->module()->spaces().space())
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

        m_plugin->moduleJson()->analyses.append(a);
    }

    // volume weakform
    XMLModule::volume volume = m_plugin->module()->volume();
    for (unsigned int i = 0; i < volume.quantity().size(); i++)
    {
        XMLModule::quantity quantity = volume.quantity().at(i);

        PluginWeakFormRecipe::Variable v;
        v.id = QString::fromStdString(quantity.id());
        v.shortName = (quantity.shortname().present()) ? QString::fromStdString(quantity.shortname().get()) : "";

        m_plugin->moduleJson()->weakFormRecipeVolume.variables.append(v);
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

        m_plugin->moduleJson()->weakFormRecipeVolume.matrixForms.append(form);
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

        m_plugin->moduleJson()->weakFormRecipeVolume.vectorForms.append(form);
    }

    // surface weakform
    XMLModule::surface surface = m_plugin->module()->surface();
    for (unsigned int i = 0; i < surface.quantity().size(); i++)
    {
        XMLModule::quantity quantity = surface.quantity().at(i);

        PluginWeakFormRecipe::Variable v;
        v.id = QString::fromStdString(quantity.id());
        v.shortName = (quantity.shortname().present()) ? QString::fromStdString(quantity.shortname().get()) : "";

        m_plugin->moduleJson()->weakFormRecipeSurface.variables.append(v);
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

        m_plugin->moduleJson()->weakFormRecipeSurface.matrixForms.append(form);
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

        m_plugin->moduleJson()->weakFormRecipeSurface.vectorForms.append(form);
    }

    // weakform - essential
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

        m_plugin->moduleJson()->weakFormRecipeSurface.essentialForms.append(form);
    }

    // volume
    /*
    foreach(XMLModule::weakform_volume weakform, volume.weakforms_volume().weakform_volume())
    {
        AnalysisType analysisType = analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype().c_str()));


        foreach (CoordinateType coordinateType, PluginFunctions::coordinateTypeList())
        {
            foreach(XMLModule::linearity_option option, weakform.linearity_option())
            {
                LinearityType linearityType = linearityTypeFromStringKey(QString::fromStdString(option.type().c_str()));

                // ctemplate::TemplateDictionary *fieldVolume = generateVolumeVariables(linearityType, coordinateType, output, weakform, "VOLUME");

                QList<FormInfo> matrixForms = Module::wfMatrixVolumeSeparated(m_plugin->module(), analysisType, linearityType);
                foreach(FormInfo formInfo, matrixForms)
                {
                    PluginWeakFormRecipe::MatrixForm form;

                    form.id = formInfo.id;
                    form.i = formInfo.i;
                    form.j = formInfo.j;
                    form.condition = formInfo.condition;
                    form.planar = formInfo.expr_planar;
                    form.axi = formInfo.expr_axi;
                    form.cart = formInfo.expr_planar;

                }

                QList<FormInfo> matrixTransientForms = Module::wfMatrixTransientVolumeSeparated(m_plugin->module(), analysisType, linearityType);
                foreach(FormInfo formInfo, matrixTransientForms)
                {
                    // generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "TRANSIENT", weakform, false);
                }

                QList<FormInfo> vectorForms = Module::wfVectorVolumeSeparated(m_plugin->module(), analysisType, linearityType);
                foreach(FormInfo formInfo, vectorForms)
                {
                    // generateFormExpression(formInfo, linearityType, coordinateType, *fieldVolume, "VECTOR", weakform, false);
                }
            }
        }
    }
    */

    // preprocessor GUI
    for (unsigned int i = 0; i < m_plugin->module()->preprocessor().gui().size(); i++)
    {
        XMLModule::gui ui = m_plugin->module()->preprocessor().gui().at(i);

        for (unsigned int i = 0; i < ui.group().size(); i++)
        {
            XMLModule::group grp = ui.group().at(i);

            PluginPreGroup group;
            group.name = (grp.name().present()) ? m_plugin->localeName(QString::fromStdString(grp.name().get())) : "";

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

                group.quantities.append(q);
            }
            if (ui.type() == "volume")
                m_plugin->moduleJson()->preVolumeGroups.append(group);
            else if (ui.type() == "surface")
                m_plugin->moduleJson()->preSurfaceGroups.append(group);
        }
    }

    // local variables
    for (unsigned int i = 0; i < m_plugin->module()->postprocessor().localvariables().localvariable().size(); i++)
    {
        XMLModule::localvariable lv = m_plugin->module()->postprocessor().localvariables().localvariable().at(i);

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
        m_plugin->moduleJson()->postLocalVariables.append(variable);
    }

    // volume integrals
    for (unsigned int i = 0; i < m_plugin->module()->postprocessor().volumeintegrals().volumeintegral().size(); i++)
    {
        XMLModule::volumeintegral in = m_plugin->module()->postprocessor().volumeintegrals().volumeintegral().at(i);

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

        m_plugin->moduleJson()->postVolumeIntegrals.append(variable);
    }

    // surface integrals
    for (unsigned int i = 0; i < m_plugin->module()->postprocessor().surfaceintegrals().surfaceintegral().size(); i++)
    {
        XMLModule::surfaceintegral in = m_plugin->module()->postprocessor().surfaceintegrals().surfaceintegral().at(i);

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

        m_plugin->moduleJson()->postSurfaceIntegrals.append(variable);
    }

    m_plugin->moduleJson()->save(QString("%1/resources/modules/%2.json").arg(datadir()).arg(m_fieldId));
}

void FieldInfo::copy(const FieldInfo *origin)
{
    clear();

    m_setting = origin->m_setting;

    // analysis
    setAnalysisType(origin->analysisType());
    setAdaptivityType(origin->adaptivityType());
    setLinearityType(origin->linearityType());
    setMatrixSolver(origin->matrixSolver());
}

double FieldInfo::labelArea(int agrosLabel) const
{
    assert(m_labelAreas);
    return m_labelAreas[agrosLabel];
}

void FieldInfo::setAnalysisType(AnalysisType analysisType)
{
    m_setting[Analysis] = QVariant::fromValue(analysisType);
    /*
    foreach (PluginModuleAnalysis an, m_plugin->moduleJson()->analyses)
    {
        if (an.id == analysisTypeToStringKey(analysisType))
        {
            m_numberOfSolutions = an.solutions;

            foreach (int index, an.configs)
            {
                Type key = stringKeyToType(an.configs[index].type);

                        if (m_settingDefault[key].type() == QVariant::Double)
                            m_settingDefault[key] = an.configs[index]. QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toDouble();
                        else if (m_settingDefault[key].type() == QVariant::Int)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toInt();
                        else if (m_settingDefault[key].type() == QVariant::Bool)
                            m_settingDefault[key] = (QString::fromStdString(an.field_config().get().field_item().at(i).field_value()) == "1");
                        else if (m_settingDefault[key].type() == QVariant::String)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                        else if (m_settingDefault[key].type() == QVariant::StringList)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).split("|");
                        else
                            qDebug() << "Key not found" << QString::fromStdString(an.field_config().get().field_item().at(i).field_key()) << QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                    }
            }

            m_availableLinearityTypes = availableLinearityTypes(analysisType);
        }
    }
    */
    foreach (XMLModule::analysis an, m_plugin->module()->general_field().analyses().analysis())
    {
        if (an.type() == analysisTypeToStringKey(analysisType).toStdString())
        {
            m_numberOfSolutions = an.solutions();

            if (an.field_config().present())
            {
                for (int i = 0; i < an.field_config().get().field_item().size(); i ++)
                {
                    Type key = stringKeyToType(QString::fromStdString(an.field_config().get().field_item().at(i).field_key()));

                    if (m_settingDefault.keys().contains(key))
                    {
                        if (m_settingDefault[key].type() == QVariant::Double)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toDouble();
                        else if (m_settingDefault[key].type() == QVariant::Int)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).toInt();
                        else if (m_settingDefault[key].type() == QVariant::Bool)
                            m_settingDefault[key] = (QString::fromStdString(an.field_config().get().field_item().at(i).field_value()) == "1");
                        else if (m_settingDefault[key].type() == QVariant::String)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                        else if (m_settingDefault[key].type() == QVariant::StringList)
                            m_settingDefault[key] = QString::fromStdString(an.field_config().get().field_item().at(i).field_value()).split("|");
                        else
                            qDebug() << "Key not found" << QString::fromStdString(an.field_config().get().field_item().at(i).field_key()) << QString::fromStdString(an.field_config().get().field_item().at(i).field_value());
                    }
                }
            }

            m_availableLinearityTypes = availableLinearityTypes(analysisType);
        }
    }
}

QList<LinearityType> FieldInfo::availableLinearityTypes(AnalysisType at) const
{
    QList<LinearityType> availableLinearityTypes;

    foreach (XMLModule::weakform_volume weakformVolume, m_plugin->module()->volume().weakforms_volume().weakform_volume())
    {
        if (weakformVolume.analysistype() == analysisTypeToStringKey(at).toStdString())
        {
            foreach(XMLModule::linearity_option linearityOption, weakformVolume.linearity_option())
            {
                availableLinearityTypes.push_back(linearityTypeFromStringKey(QString::fromStdString(linearityOption.type())));
            }
        }
    }

    return availableLinearityTypes;
}

int FieldInfo::labelRefinement(SceneLabel *label) const
{
    QMapIterator<SceneLabel *, int> i(m_labelsRefinement);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return 0;
}


int FieldInfo::labelPolynomialOrder(SceneLabel *label)
{
    QMapIterator<SceneLabel *, int> i(m_labelsPolynomialOrder);
    while (i.hasNext()) {
        i.next();
        if (i.key() == label)
            return i.value();
    }

    return value(FieldInfo::SpacePolynomialOrder).toInt();
}

void FieldInfo::clear()
{
    // set default values and types
    setDefaultValues();
    m_setting = m_settingDefault;

    m_labelsRefinement.clear();
    m_labelsPolynomialOrder.clear();
}

// xml module
// name
QString FieldInfo::name() const
{
    return m_plugin->localeName(m_plugin->moduleJson()->name);
}

// deformable shape
bool FieldInfo::hasDeformableShape() const
{
    return m_plugin->moduleJson()->deformedShape;
}

// constants
QMap<QString, double> FieldInfo::constants() const
{    
    QMap<QString, double> constants;

    // constants
    foreach (PluginConstant constant, m_plugin->moduleJson()->constants)
        constants[constant.id] = constant.value;

    return constants;
}

// macros
QMap<QString, QString> FieldInfo::macros() const
{
    QMap<QString, QString> macros;

    foreach (PluginMacro macro, m_plugin->moduleJson()->macros)
        macros[macro.id] = macro.expression;

    return macros;
}

QMap<AnalysisType, QString> FieldInfo::analyses() const
{
    QMap<AnalysisType, QString> analyses;

    foreach (PluginModuleAnalysis analysis, m_plugin->moduleJson()->analyses)
        analyses[analysis.type] = m_plugin->localeName(analysis.name);

    return analyses;
}

QList<QString> FieldInfo::allMaterialQuantities() const
{
    QList<QString> result;

    foreach (PluginWeakFormRecipe::Variable variable, m_plugin->moduleJson()->weakFormRecipeVolume.variables)
        result.append(variable.id);

    return result;

    foreach(XMLModule::quantity quantity, m_plugin->module()->volume().quantity())
    {
        result.push_back(QString::fromStdString(quantity.id()));
    }
}

// material type
QList<Module::MaterialTypeVariable> FieldInfo::materialTypeVariables() const
{
    // all materials variables
    QList<Module::MaterialTypeVariable> materialTypeVariablesAll;
    for (int i = 0; i < m_plugin->module()->volume().quantity().size(); i++)
    {
        XMLModule::quantity quant = m_plugin->module()->volume().quantity().at(i);

        // gui default
        for (unsigned int i = 0; i < m_plugin->module()->preprocessor().gui().size(); i++)
        {
            XMLModule::gui ui = m_plugin->module()->preprocessor().gui().at(i);
            if (ui.type() == "volume")
            {
                for (unsigned int i = 0; i < ui.group().size(); i++)
                {
                    XMLModule::group grp = ui.group().at(i);
                    for (unsigned int i = 0; i < grp.quantity().size(); i++)
                    {
                        XMLModule::quantity quant_ui = grp.quantity().at(i);
                        if (quant_ui.id() == quant.id())
                        {
                            if (quant_ui.default_().present())
                                quant.default_().set(quant_ui.default_().get());

                            if (quant_ui.is_bool().present())
                                quant.is_bool().set(quant_ui.is_bool().get());

                            if (quant_ui.only_if().present())
                                quant.only_if().set(quant_ui.only_if().get());

                            if (quant_ui.only_if_not().present())
                                quant.only_if_not().set(quant_ui.only_if_not().get());

                            if (quant_ui.is_source().present())
                                quant.is_source().set(quant_ui.is_source().get());
                        }
                    }
                }
            }
        }

        // add to the list
        materialTypeVariablesAll.append(Module::MaterialTypeVariable(quant));
    }

    QList<Module::MaterialTypeVariable> materialTypeVariables;
    for (unsigned int i = 0; i < m_plugin->module()->volume().weakforms_volume().weakform_volume().size(); i++)
    {
        XMLModule::weakform_volume wf = m_plugin->module()->volume().weakforms_volume().weakform_volume().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
        {
            for (unsigned int i = 0; i < wf.quantity().size(); i++)
            {
                XMLModule::quantity qty = wf.quantity().at(i);

                foreach (Module::MaterialTypeVariable variable, materialTypeVariablesAll)
                {
                    if (variable.id().toStdString() == qty.id())
                    {
                        QString nonlinearExpression;
                        if (Agros::problem()->config()->coordinateType() == CoordinateType_Planar && qty.nonlinearity_planar().present())
                            nonlinearExpression = QString::fromStdString(qty.nonlinearity_planar().get());
                        else
                            if (qty.nonlinearity_axi().present())
                                nonlinearExpression = QString::fromStdString(qty.nonlinearity_axi().get());

                        bool isTimeDep = false;
                        if (qty.dependence().present())
                            isTimeDep = (QString::fromStdString(qty.dependence().get()) == "time");

                        materialTypeVariables.append(Module::MaterialTypeVariable(variable.id(), variable.shortname(),
                                                                                  nonlinearExpression, isTimeDep, variable.isBool(), variable.onlyIf(), variable.onlyIfNot(), variable.isSource()));
                    }
                }
            }
        }
    }

    materialTypeVariablesAll.clear();

    return materialTypeVariables;
}

// variable by name
bool FieldInfo::materialTypeVariableContains(const QString &id) const
{
    foreach (Module::MaterialTypeVariable var, materialTypeVariables())
        if (var.id() == id)
            return true;

    return false;
}

bool FieldInfo::functionUsedInAnalysis(const QString &id) const
{
    foreach(XMLModule::weakform_volume weakform, this->plugin()->module()->volume().weakforms_volume().weakform_volume())
    {
        if(analysisTypeFromStringKey(QString::fromStdString(weakform.analysistype())) == this->analysisType())
        {
            foreach (XMLModule::function_use functionUse, weakform.function_use())
            {
                if(QString::fromStdString(functionUse.id()) == id)
                    return true;
            }
        }
    }

    return false;
}

Module::MaterialTypeVariable FieldInfo::materialTypeVariable(const QString &id) const
{
    foreach (Module::MaterialTypeVariable var, materialTypeVariables())
        if (var.id() == id)
            return var;

    assert(0);
}

QList<Module::BoundaryType> FieldInfo::boundaryTypes() const
{
    QList<Module::BoundaryTypeVariable> boundaryTypeVariablesAll;
    for (int i = 0; i < m_plugin->module()->surface().quantity().size(); i++)
    {
        XMLModule::quantity quant = m_plugin->module()->surface().quantity().at(i);

        // add to list
        boundaryTypeVariablesAll.append(Module::BoundaryTypeVariable(quant));
    }

    QList<Module::BoundaryType> boundaryTypes;
    for (int i = 0; i < m_plugin->module()->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface wf = m_plugin->module()->surface().weakforms_surface().weakform_surface().at(i);

        if (wf.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
        {
            for (int i = 0; i < wf.boundary().size(); i++)
            {
                XMLModule::boundary bdy = wf.boundary().at(i);
                boundaryTypes.append(Module::BoundaryType(this, boundaryTypeVariablesAll, bdy));
            }
        }
    }

    boundaryTypeVariablesAll.clear();

    return boundaryTypes;
}

// default boundary condition
Module::BoundaryType FieldInfo::boundaryTypeDefault() const
{
    for (int i = 0; i < m_plugin->module()->surface().weakforms_surface().weakform_surface().size(); i++)
    {
        XMLModule::weakform_surface wf = m_plugin->module()->surface().weakforms_surface().weakform_surface().at(i);

        // default
        return boundaryType(QString::fromStdString(wf.default_().get()));
    }

    assert(0);
}

// variable by name
bool FieldInfo::boundaryTypeContains(const QString &id) const
{
    foreach (Module::BoundaryType var, boundaryTypes())
        if (var.id() == id)
            return true;

    return false;
}

Module::BoundaryType FieldInfo::boundaryType(const QString &id) const
{
    foreach (Module::BoundaryType var, boundaryTypes())
        if (var.id() == id)
            return var;

    throw AgrosModuleException(QString("Boundary type %1 not found. Probably using corrupted a2d file or wrong version.").arg(id));
}

// force
Module::Force FieldInfo::force(CoordinateType coordinateType) const
{
    return Module::Force();

    // force
    /*
    XMLModule::force force = m_plugin->module()->postprocessor().force();
    for (unsigned int i = 0; i < force.expression().size(); i++)
    {
        XMLModule::expression exp = force.expression().at(i);
        if (exp.analysistype() == analysisTypeToStringKey(analysisType()).toStdString())
            return Module::Force((coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_x().get()) : QString::fromStdString(exp.axi_r().get()),
                                 (coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_y().get()) : QString::fromStdString(exp.axi_z().get()),
                                 (coordinateType == CoordinateType_Planar) ? QString::fromStdString(exp.planar_z().get()) : QString::fromStdString(exp.axi_phi().get()));
    }

    assert(0);
    return Module::Force();
    */
}

// material and boundary user interface
Module::DialogUI FieldInfo::materialUI() const
{
    QMap<QString, QList<Module::DialogRow> > groups;

    // preprocessor
    foreach (PluginPreGroup group, m_plugin->moduleJson()->preVolumeGroups)
    {

        QList<Module::DialogRow> materials;

        foreach (PluginPreGroup::Quantity quantity, group.quantities)
        {
            Module::DialogRow row(quantity.id, quantity.name, quantity.shortname, quantity.shortname_html,
                                  quantity.shortname_dependence, quantity.shortname_dependence_html,
                                  quantity.unit, quantity.unit_html,
                                  quantity.default_value, quantity.condition);

            materials.append(row);
        }

        groups[group.name] = materials;
    }

    return Module::DialogUI(groups);
}

Module::DialogUI FieldInfo::boundaryUI() const
{
    QMap<QString, QList<Module::DialogRow> > groups;

    // preprocessor
    foreach (PluginPreGroup group, m_plugin->moduleJson()->preSurfaceGroups)
    {

        QList<Module::DialogRow> materials;

        foreach (PluginPreGroup::Quantity quantity, group.quantities)
        {
            Module::DialogRow row(quantity.id, quantity.name, quantity.shortname, quantity.shortname_html,
                                  quantity.shortname_dependence, quantity.shortname_dependence_html,
                                  quantity.unit, quantity.unit_html,
                                  quantity.default_value, quantity.condition);

            materials.append(row);
        }

        groups[group.name] = materials;
    }

    return Module::DialogUI(groups);
}

// local point variables
QList<Module::LocalVariable> FieldInfo::localPointVariables(CoordinateType coordinateType) const
{
    // local variables
    QList<Module::LocalVariable> variables;

    foreach (PluginPostVariable variable, m_plugin->moduleJson()->postLocalVariables)
    {
        foreach (PluginPostVariable::Expression expresion, variable.expresions)
        {
            if (expresion.analysis == analysisType())
            {
                bool isScalar = (variable.type == "scalar");

                Module::LocalVariable::Expression expr;

                if (coordinateType == CoordinateType_Planar)
                    expr = Module::LocalVariable::Expression(isScalar ? expresion.planar : "",
                                                             isScalar ? "" : expresion.planar_x,
                                                             isScalar ? "" : expresion.planar_y);
                else if (coordinateType == CoordinateType_Axisymmetric)
                    expr = Module::LocalVariable::Expression(isScalar ? expresion.axi : "",
                                                             isScalar ? "" : expresion.axi_r,
                                                             isScalar ? "" : expresion.axi_z);
                else
                    assert(0);

                variables.append(Module::LocalVariable(variable.id, m_plugin->localeName(variable.name),
                                                       variable.shortname, variable.shortname_html,
                                                       variable.unit, variable.unit_html,
                                                       isScalar,
                                                       expr));
            }
        }
    }

    return variables;
}

// view scalar variables
QList<Module::LocalVariable> FieldInfo::viewScalarVariables(CoordinateType coordinateType) const
{
    // scalar variables = local variables
    return localPointVariables(coordinateType);
}

// view vector variables
QList<Module::LocalVariable> FieldInfo::viewVectorVariables(CoordinateType coordinateType) const
{
    // vector variables
    QList<Module::LocalVariable> variables;
    foreach (Module::LocalVariable var, localPointVariables(coordinateType))
        if (!var.isScalar())
            variables.append(var);

    return variables;
}

// surface integrals
QList<Module::Integral> FieldInfo::surfaceIntegrals(CoordinateType coordinateType) const
{
    // surface integrals
    QList<Module::Integral> surfaceIntegrals;

    foreach (PluginPostVariable variable, m_plugin->moduleJson()->postSurfaceIntegrals)
    {
        foreach (PluginPostVariable::Expression expresion, variable.expresions)
        {
            if (expresion.analysis == analysisType())
            {
                QString expr;

                if (coordinateType == CoordinateType_Planar)
                    expr = expresion.planar;
                else if (coordinateType == CoordinateType_Axisymmetric)
                    expr = expresion.axi;
                else
                    assert(0);

                surfaceIntegrals.append(Module::Integral(variable.id, m_plugin->localeName(variable.name),
                                                         variable.shortname, variable.shortname_html,
                                                         variable.unit, variable.unit_html,
                                                         expr,
                                                         false));
            }
        }
    }

    return surfaceIntegrals;
}

// volume integrals
QList<Module::Integral> FieldInfo::volumeIntegrals(CoordinateType coordinateType) const
{
    // volume integrals
    QList<Module::Integral> volumeIntegrals;

    foreach (PluginPostVariable variable, m_plugin->moduleJson()->postVolumeIntegrals)
    {
        foreach (PluginPostVariable::Expression expresion, variable.expresions)
        {
            if (expresion.analysis == analysisType())
            {
                QString expr;

                if (coordinateType == CoordinateType_Planar)
                    expr = expresion.planar;
                else if (coordinateType == CoordinateType_Axisymmetric)
                    expr = expresion.axi;
                else
                    assert(0);

                // TODO: eggshell
                volumeIntegrals.append(Module::Integral(variable.id, m_plugin->localeName(variable.name),
                                                        variable.shortname, variable.shortname_html,
                                                        variable.unit, variable.unit_html,
                                                        expr,
                                                        false));
            }
        }
    }

    return volumeIntegrals;
}

// variable by name
Module::LocalVariable FieldInfo::localVariable(CoordinateType coordinateType, const QString &id) const
{
    foreach (Module::LocalVariable var, localPointVariables(coordinateType))
        if (var.id() == id)
            return var;

    qDebug() << "localVariable: " << id;
    assert(0);

    // qDebug() << "Warning: unable to return local variable: " << id;
    // return Module::LocalVariable();
}

Module::Integral FieldInfo::surfaceIntegral(CoordinateType coordinateType, const QString &id) const
{
    foreach (Module::Integral var, surfaceIntegrals(coordinateType))
        if (var.id() == id)
            return var;

    qDebug() << "surfaceIntegral: " << id;
    assert(0);
}

Module::Integral FieldInfo::volumeIntegral(CoordinateType coordinateType, const QString &id) const
{
    foreach (Module::Integral var, volumeIntegrals(coordinateType))
        if (var.id() == id)
            return var;

    qDebug() << "volumeIntegral: " << id;
    assert(0);
}

// default variables
Module::LocalVariable FieldInfo::defaultViewScalarVariable(CoordinateType coordinateType) const
{
    assert(localPointVariables(coordinateType).size() > 0);

    // return first variable
    return localPointVariables(coordinateType).first();
}

Module::LocalVariable FieldInfo::defaultViewVectorVariable(CoordinateType coordinateType) const
{
    // return first vector variable
    foreach (Module::LocalVariable var, localPointVariables(coordinateType))
        if (!var.isScalar())
            return var;

    assert(0);
}

void FieldInfo::load(XMLProblem::field_config *configxsd)
{
    // default
    m_setting = m_settingDefault;

    for (int i = 0; i < configxsd->field_item().size(); i ++)
    {
        Type key = stringKeyToType(QString::fromStdString(configxsd->field_item().at(i).field_key()));

        if (m_settingDefault.keys().contains(key))
        {
            if (m_settingDefault[key].type() == QVariant::Double)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).toDouble();
            else if (m_settingDefault[key].type() == QVariant::Int)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).toInt();
            else if (m_settingDefault[key].type() == QVariant::Bool)
                m_setting[key] = (QString::fromStdString(configxsd->field_item().at(i).field_value()) == "1");
            else if (m_settingDefault[key].type() == QVariant::String)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value());
            else if (m_settingDefault[key].type() == QVariant::StringList)
                m_setting[key] = QString::fromStdString(configxsd->field_item().at(i).field_value()).split("|");
            // else
            //    qDebug() << "Key not found" << QString::fromStdString(configxsd->field_item().at(i).field_key()) << QString::fromStdString(configxsd->field_item().at(i).field_value());
        }
    }
}

void FieldInfo::save(XMLProblem::field_config *configxsd)
{
    foreach (Type key, m_settingDefault.keys())
    {
        if (m_settingDefault[key].type() == QVariant::StringList)
            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), m_setting[key].toStringList().join("|").toStdString()));
        else if (m_settingDefault[key].type() == QVariant::Bool)
            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), QString::number(m_setting[key].toInt()).toStdString()));
        else
            configxsd->field_item().push_back(XMLProblem::field_item(typeToStringKey(key).toStdString(), m_setting[key].toString().toStdString()));

    }
}

void FieldInfo::load(QJsonObject &object)
{
    // default
    m_setting = m_settingDefault;

    foreach (Type key, m_settingDefault.keys())
    {
        if (!object.contains(typeToStringKey(key)))
            continue;

        if (m_settingDefault[key].type() == QVariant::StringList)
            m_setting[key] = object[typeToStringKey(key)].toString().split("|");
        else if (m_settingDefault[key].type() == QVariant::Bool)
            m_setting[key] = object[typeToStringKey(key)].toBool();
        else if (m_settingDefault[key].type() == QVariant::String)
            m_setting[key] = object[typeToStringKey(key)].toString();
        else if (m_settingDefault[key].type() == QVariant::Double)
            m_setting[key] = object[typeToStringKey(key)].toDouble();
        else if (m_settingDefault[key].type() == QVariant::Int)
            m_setting[key] = object[typeToStringKey(key)].toInt();
        else
        {
            if (m_settingDefault[key].userType() == qMetaTypeId<AnalysisType>())
                setAnalysisType(analysisTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_settingDefault[key].userType() == qMetaTypeId<LinearityType>())
                setLinearityType(linearityTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_settingDefault[key].userType() == qMetaTypeId<AdaptivityMethod>())
                setAdaptivityType(adaptivityTypeFromStringKey(object[typeToStringKey(key)].toString()));
            else if (m_settingDefault[key].userType() == qMetaTypeId<MatrixSolverType>())
                setMatrixSolver(matrixSolverTypeFromStringKey( object[typeToStringKey(key)].toString()));
            else
                assert(0);
        }
    }
}

void FieldInfo::save(QJsonObject &object)
{
    foreach (Type key, m_settingDefault.keys())
    {
        if (m_settingDefault[key].type() == QVariant::StringList)
            object[typeToStringKey(key)] = m_setting[key].toStringList().join("|");
        else if (m_settingDefault[key].type() == QVariant::Bool)
            object[typeToStringKey(key)] = m_setting[key].toBool();
        else if (m_settingDefault[key].type() == QVariant::String)
            object[typeToStringKey(key)] = m_setting[key].toString();
        else if (m_settingDefault[key].type() == QVariant::Double)
            object[typeToStringKey(key)] = m_setting[key].toDouble();
        else if (m_settingDefault[key].type() == QVariant::Int)
            object[typeToStringKey(key)] = m_setting[key].toInt();
        else
        {
            if (m_settingDefault[key].userType() == qMetaTypeId<AnalysisType>())
                object[typeToStringKey(key)] = analysisTypeToStringKey(m_setting[key].value<AnalysisType>());
            else if (m_settingDefault[key].userType() == qMetaTypeId<LinearityType>())
                object[typeToStringKey(key)] = linearityTypeToStringKey(m_setting[key].value<LinearityType>());
            else if (m_settingDefault[key].userType() == qMetaTypeId<AdaptivityMethod>())
                object[typeToStringKey(key)] = adaptivityTypeToStringKey(m_setting[key].value<AdaptivityMethod>());
            else if (m_settingDefault[key].userType() == qMetaTypeId<MatrixSolverType>())
                object[typeToStringKey(key)] = matrixSolverTypeToStringKey(m_setting[key].value<MatrixSolverType>());
            else
                assert(0);
        }
    }
}

void FieldInfo::setStringKeys()
{
    m_settingKey[Analysis] = "Analysis";
    m_settingKey[Linearity] = "Linearity";
    m_settingKey[NonlinearResidualNorm] = "NonlinearResidualNorm";
    m_settingKey[NonlinearRelativeChangeOfSolutions] = "NonlinearRelativeChangeOfSolutions";
    m_settingKey[NonlinearDampingType] = "NonlinearDampingType";
    m_settingKey[NonlinearDampingCoeff] = "NonlinearDampingCoeff";
    m_settingKey[NewtonReuseJacobian] = "NewtonReuseJacobian";
    m_settingKey[NewtonJacobianReuseRatio] = "NewtonJacobianReuseRatio";
    m_settingKey[NonlinearDampingFactorDecreaseRatio] = "NonlinearDampingFactorDecreaseRatio";
    m_settingKey[NewtonMaxStepsReuseJacobian] = "NewtonMaxStepsReuseJacobian";
    m_settingKey[NonlinearStepsToIncreaseDampingFactor] = "NonlinearStepsToIncreaseDampingFactor";
    m_settingKey[PicardAndersonAcceleration] = "PicardAndersonAcceleration";
    m_settingKey[PicardAndersonBeta] = "PicardAndersonBeta";
    m_settingKey[PicardAndersonNumberOfLastVectors] = "PicardAndersonNumberOfLastVectors";
    m_settingKey[SpaceNumberOfRefinements] = "SpaceNumberOfRefinements";
    m_settingKey[SpacePolynomialOrder] = "SpacePolynomialOrder";
    m_settingKey[Adaptivity] = "Adaptivity";
    m_settingKey[AdaptivitySteps] = "AdaptivitySteps";
    m_settingKey[AdaptivityTolerance] = "AdaptivityTolerance";
    m_settingKey[AdaptivityTransientBackSteps] = "AdaptivityTransientBackSteps";
    m_settingKey[AdaptivityTransientRedoneEach] = "AdaptivityTransientRedoneEach";
    m_settingKey[AdaptivityFinePercentage] = "AdaptivityFinePercentage";
    m_settingKey[AdaptivityCoarsePercentage] = "AdaptivityCoarsePercentage";
    m_settingKey[AdaptivityEstimator] = "AdaptivityEstimator";
    m_settingKey[AdaptivityStrategy] = "AdaptivityStrategy";
    m_settingKey[AdaptivityStrategyHP] = "AdaptivityStrategyHP";
    m_settingKey[TransientTimeSkip] = "TransientTimeSkip";
    m_settingKey[TransientInitialCondition] = "TransientInitialCondition";
    m_settingKey[LinearSolver] = "LinearSolver";
    m_settingKey[LinearSolverIterDealIIMethod] = "LinearSolverIterDealIIMethod";
    m_settingKey[LinearSolverIterDealIIPreconditioner] = "LinearSolverIterDealIIPreconditioner";
    m_settingKey[LinearSolverIterToleranceAbsolute] = "LinearSolverIterToleranceAbsolute";
    m_settingKey[LinearSolverIterIters] = "LinearSolverIterIters";
    m_settingKey[LinearSolverExternalName] = "LinearSolverExternalName";
    m_settingKey[LinearSolverExternalCommandEnvironment] = "LinearSolverExternalCommandEnvironment";
    m_settingKey[LinearSolverExternalCommandParameters] = "LinearSolverExternalCommandParameters";
    m_settingKey[TimeUnit] = "TimeUnit";
}

void FieldInfo::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[Analysis] = QVariant::fromValue(AnalysisType_Undefined);
    m_settingDefault[Linearity] = QVariant::fromValue(LinearityType_Linear);
    m_settingDefault[NonlinearResidualNorm] = 0.0;
    m_settingDefault[NonlinearRelativeChangeOfSolutions] = 0.1;
    m_settingDefault[NonlinearDampingType] = DampingType_Automatic;
    m_settingDefault[NonlinearDampingCoeff] = 0.8;
    m_settingDefault[NewtonReuseJacobian] = true;
    m_settingDefault[NewtonJacobianReuseRatio] = 0.8;
    m_settingDefault[NonlinearDampingFactorDecreaseRatio] = 1.2;
    m_settingDefault[NewtonMaxStepsReuseJacobian] = 20;
    m_settingDefault[NonlinearStepsToIncreaseDampingFactor] = 1;
    m_settingDefault[PicardAndersonAcceleration] = false;
    m_settingDefault[PicardAndersonBeta] = 0.2;
    m_settingDefault[PicardAndersonNumberOfLastVectors] = 3;
    m_settingDefault[SpaceNumberOfRefinements] = 1;
    m_settingDefault[SpacePolynomialOrder] = 2;
    m_settingDefault[Adaptivity] = QVariant::fromValue(AdaptivityMethod_None);
    m_settingDefault[AdaptivitySteps] = 10;
    m_settingDefault[AdaptivityTolerance] = 1.0;
    m_settingDefault[AdaptivityEstimator] = AdaptivityEstimator_Kelly;
    m_settingDefault[AdaptivityStrategy] = AdaptivityStrategy_FixedFractionOfTotalError;
    m_settingDefault[AdaptivityStrategyHP] = AdaptivityStrategyHP_FourierSeries;
    m_settingDefault[AdaptivityFinePercentage] = 30;
    m_settingDefault[AdaptivityCoarsePercentage] = 3;
    m_settingDefault[AdaptivityTransientBackSteps] = 3;
    m_settingDefault[AdaptivityTransientRedoneEach] = 5;
    m_settingDefault[TransientTimeSkip] = 0.0;
    m_settingDefault[TransientInitialCondition] = 0.0;
    m_settingDefault[LinearSolver] = QVariant::fromValue(SOLVER_UMFPACK);
    m_settingDefault[LinearSolverIterDealIIMethod] = IterSolverDealII_BiCGStab;
    m_settingDefault[LinearSolverIterDealIIPreconditioner] = PreconditionerDealII_SSOR;
    m_settingDefault[LinearSolverIterToleranceAbsolute] = 1e-16;
    m_settingDefault[LinearSolverIterIters] = 1000;
    m_settingDefault[LinearSolverExternalName] = "";
    m_settingDefault[LinearSolverExternalCommandEnvironment] = "";
    m_settingDefault[LinearSolverExternalCommandParameters] = "";
    m_settingDefault[TimeUnit] = "s";
}

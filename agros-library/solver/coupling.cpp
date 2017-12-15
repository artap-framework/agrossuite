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

#include "coupling.h"

#include "util/constants.h"
#include "util/global.h"

#include "logview.h"
#include "scene.h"
#include "util/util.h"
#include "module.h"
#include "module.h"
#include "weak_form.h"
#include "plugin_interface.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

// #include "../../resources_source/classes/module_xml.h"

static CouplingList *m_couplingList = NULL;
CouplingList *couplingList()
{
    if (!m_couplingList)
        m_couplingList = new CouplingList();

    return m_couplingList;
}

CouplingList::CouplingList()
{
    // read couplings
    QDir dir(datadir() + COUPLINGROOT);

    QStringList filter;
    filter << "*.json";
    QStringList list = dir.entryList(filter);

    foreach (QString filename, list)
    {
        PluginCoupling *coupling = new PluginCoupling();
        coupling->load(datadir() + COUPLINGROOT + "/" + filename.left(filename.count() - 4) + "json");

        m_couplingJson.append(coupling);
    }

    /*
    QStringList filter;
    filter << "*.xml";
    QStringList list = dir.entryList(filter);

    foreach (QString filename, list)
    {
        try
        {
            bool validateAtTheBeginning = false;
            ::xml_schema::flags parsing_flags = xml_schema::flags::dont_validate;
            if(validateAtTheBeginning)
            {
                parsing_flags = 0;
                qDebug() << "Warning: Validating all XML coupling files. This is time-consuming and should be switched off in coupling.cpp for release. Set validateAtTheBeginning = false.";
            }

            std::unique_ptr<XMLModule::module> module_xsd = XMLModule::module_(compatibleFilename(datadir() + COUPLINGROOT + "/" + filename).toStdString(),
                                                                               xml_schema::flags::dont_validate & xml_schema::flags::dont_initialize);
            XMLModule::module *mod = module_xsd.get();
            assert(mod->coupling().present());
            XMLModule::coupling *coup = &mod->coupling().get();

            // convert JSON
            PluginCoupling coupling;
            coupling.id = QString::fromStdString(coup->general_coupling().id());
            coupling.name = QString::fromStdString(coup->general_coupling().name());
            coupling.source = QString::fromStdString(coup->general_coupling().modules().source().id());
            coupling.target = QString::fromStdString(coup->general_coupling().modules().target().id());

            qWarning() << coupling.id;

            // constants
            foreach (XMLModule::constant cnst, coup->constants().constant())
            {
                PluginConstant c;
                c.id = QString::fromStdString(cnst.id());
                c.value = cnst.value();

                coupling.constants.append(c);
            }

            // check whether coupling is available for values of source and target fields such as analysis type
            for (int k = 0; k < coup->volume().weakforms_volume().weakform_volume().size(); k++)
            {
                XMLModule::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(k);

                // volume weakform - matrix
                for (unsigned int i = 0; i < coup->volume().matrix_form().size(); i++)
                {
                    XMLModule::matrix_form matrix_form = coup->volume().matrix_form().at(i);

                    PluginWeakFormRecipe::MatrixForm form;
                    form.id = QString::fromStdString(matrix_form.id());
                    form.i = (matrix_form.i().present()) ? matrix_form.i().get() : -1;
                    form.j = (matrix_form.j().present()) ? matrix_form.j().get() : -1;
                    form.planar = (matrix_form.planar().present()) ? QString::fromStdString(matrix_form.planar().get()) : "";
                    form.axi = (matrix_form.axi().present()) ? QString::fromStdString(matrix_form.axi().get()) : "";
                    form.cart = (matrix_form.planar().present()) ? QString::fromStdString(matrix_form.planar().get()) : "";
                    form.condition = (matrix_form.condition().present()) ? QString::fromStdString(matrix_form.condition().get()) : "";

                    coupling.weakFormRecipeVolume.matrixForms.append(form);
                }

                // volume weakform - vector
                for (unsigned int i = 0; i < coup->volume().vector_form().size(); i++)
                {
                    XMLModule::vector_form vector_form = coup->volume().vector_form().at(i);

                    PluginWeakFormRecipe::VectorForm form;
                    form.id = QString::fromStdString(vector_form.id());
                    form.i = (vector_form.i().present()) ? vector_form.i().get() : -1;
                    form.planar = (vector_form.planar().present()) ? QString::fromStdString(vector_form.planar().get()) : "";
                    form.axi = (vector_form.axi().present()) ? QString::fromStdString(vector_form.axi().get()) : "";
                    form.cart = (vector_form.planar().present()) ? QString::fromStdString(vector_form.planar().get()) : "";
                    form.condition = (vector_form.condition().present()) ? QString::fromStdString(vector_form.condition().get()) : "";

                    coupling.weakFormRecipeVolume.vectorForms.append(form);
                }

                // volume analyses
                PluginWeakFormAnalysis analysis;

                for (unsigned int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
                {
                    XMLModule::weakform_volume weakform_volume = coup->volume().weakforms_volume().weakform_volume().at(i);

                    // only one item
                    PluginWeakFormAnalysis::Item item;
                    item.id = "volume";
                    item.name = "Volume";
                    item.analysis = analysisTypeFromStringKey(QString::fromStdString(wf.analysistype()));
                    item.analysisSource = analysisTypeFromStringKey(QString::fromStdString(weakform_volume.sourceanalysis().get()));
                    item.coupling = couplingTypeFromStringKey(QString::fromStdString(weakform_volume.couplingtype().get()));
                    item.equation = QString::fromStdString(weakform_volume.equation());
                    qWarning() << i << QString::fromStdString(weakform_volume.sourceanalysis().get()) <<
                                  QString::fromStdString(weakform_volume.analysistype()) <<
                                  QString::fromStdString(weakform_volume.couplingtype().get());

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

                    coupling.weakFormAnalysisVolume.append(analysis);
                }
            }

            coupling.save(datadir() + COUPLINGROOT + "/" + filename.left(filename.count() - 4) + ".json");
            coupling.load(datadir() + COUPLINGROOT + "/" + filename.left(filename.count() - 4) + ".json");
        }
        catch (const xml_schema::expected_element& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
            qDebug() << str;
            throw AgrosException(str);
        }
        catch (const xml_schema::expected_attribute& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.name()));
            qDebug() << str;
            throw AgrosException(str);
        }
        catch (const xml_schema::unexpected_element& e)
        {
            QString str = QString("%1: %2 instead of %3").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.encountered_name())).arg(QString::fromStdString(e.expected_name()));
            qDebug() << str;
            throw AgrosException(str);
        }
        catch (const xml_schema::unexpected_enumerator& e)
        {
            QString str = QString("%1: %2").arg(QString::fromStdString(e.what())).arg(QString::fromStdString(e.enumerator()));
            qDebug() << str;
            throw AgrosException(str);
        }
        catch (const xml_schema::expected_text_content& e)
        {
            QString str = QString("%1").arg(QString::fromStdString(e.what()));
            qDebug() << str;
            throw AgrosException(str);
        }
        catch (const xml_schema::parsing& e)
        {
            QString str = QString("%1").arg(QString::fromStdString(e.what()));
            qDebug() << str;
            xml_schema::diagnostics diagnostic = e.diagnostics();
            for(int i = 0; i < diagnostic.size(); i++)
            {
                xml_schema::error err = diagnostic.at(i);
                qDebug() << QString("%1, position %2:%3, %4").arg(QString::fromStdString(err.id())).arg(err.line()).arg(err.column()).arg(QString::fromStdString(err.message()));
            }
            throw AgrosException(str);
        }
        catch (const xml_schema::exception& e)
        {
            qDebug() << QString("Unknow parser exception: %1").arg(QString::fromStdString(e.what()));
            throw AgrosException(QString::fromStdString(e.what()));
        }
    }
    */
}

CouplingList::~CouplingList()
{
    foreach (PluginCoupling *plugin, m_couplingJson)
        delete plugin;
}

QString CouplingList::name(FieldInfo *sourceField, FieldInfo *targetField) const
{
    foreach (PluginCoupling *coupling, m_couplingJson)
    {
        if (coupling->source == sourceField->fieldId() && coupling->target == targetField->fieldId())
            return coupling->name;
    }

    assert(0);
    return "";

    //    foreach (Item item, m_couplings)
    //    {
    //        if (item.source == sourceField->fieldId() && item.target == targetField->fieldId())
    //            return item.name;
    //    }
}

bool CouplingList::isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField,
                                       CouplingType couplingType) const
{
    foreach (PluginCoupling *coupling, m_couplingJson)
    {
        if (coupling->source == sourceField->fieldId() && coupling->target == targetField->fieldId())
        {
            foreach (PluginWeakFormAnalysis weakFormAnalysisVolume, coupling->weakFormAnalysisVolume)
            {
                foreach (PluginWeakFormAnalysis::Item item, weakFormAnalysisVolume.items)
                {
                    if (item.analysis == targetField->analysisType() && item.analysisSource == sourceField->analysisType() && item.coupling == couplingType)
                        return true;
                }
            }
        }
    }

    return false;

    //    foreach (Item item, m_couplings)
    //    {
    //        if (item.source == sourceField->fieldId() && item.target == targetField->fieldId())
    //        {
    //            foreach (Item::Analysis analysis, item.analyses)
    //            {
    //                if (analysis.sourceAnalysisType == sourceField->analysisType() && analysis.targetAnalysisType == targetField->analysisType()
    //                        && analysis.couplingType == couplingType)
    //                    return true;
    //            }
    //        }
    //    }
}

bool CouplingList::isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField) const
{
    foreach (PluginCoupling *coupling, m_couplingJson)
    {
        if (coupling->source == sourceField->fieldId() && coupling->target == targetField->fieldId())
        {
            foreach (PluginWeakFormAnalysis weakFormAnalysisVolume, coupling->weakFormAnalysisVolume)
            {
                foreach (PluginWeakFormAnalysis::Item item, weakFormAnalysisVolume.items)
                {
                    if (item.analysis == targetField->analysisType() && item.analysisSource == sourceField->analysisType())
                        return true;
                }
            }
        }
    }

    return false;

    //    foreach (Item item, m_couplings)
    //    {
    //        if (item.source == sourceField->fieldId() && item.target == targetField->fieldId())
    //        {
    //            foreach (Item::Analysis analysis, item.analyses)
    //            {
    //                if (analysis.sourceAnalysisType == sourceField->analysisType() && analysis.targetAnalysisType == targetField->analysisType())
    //                    return true;
    //            }
    //        }
    //    }

    //    return false;
}

bool CouplingList::isCouplingAvailable(QString sourceField, AnalysisType sourceAnalysis,
                                       QString targetField, AnalysisType targetAnalysis,
                                       CouplingType couplingType) const
{
    foreach (PluginCoupling *coupling, m_couplingJson)
    {
        if (coupling->source == sourceField && coupling->target == targetField)
        {
            foreach (PluginWeakFormAnalysis weakFormAnalysisVolume, coupling->weakFormAnalysisVolume)
            {
                    foreach (PluginWeakFormAnalysis::Item item, weakFormAnalysisVolume.items)
                    {
                        if (item.analysis == targetAnalysis && item.analysisSource == sourceAnalysis && item.coupling == couplingType)
                            return true;
                    }
            }
        }
    }

    return false;

    //    foreach (Item item, m_couplings)
    //    {
    //        if (item.source == sourceField && item.target == targetField)
    //        {
    //            foreach (Item::Analysis analysis, item.analyses)
    //            {
    //                if (analysis.sourceAnalysisType == sourceAnalysis && analysis.targetAnalysisType == targetAnalysis
    //                        && analysis.couplingType == couplingType)
    //                    return true;
    //            }
    //        }
    //    }

    //    return false;
}

bool CouplingList::isCouplingAvailable(QString sourceField, QString targetField,
                                       CouplingType couplingType) const
{
    foreach (PluginCoupling *coupling, m_couplingJson)
    {
        if (coupling->source == sourceField && coupling->target == targetField)
        {
            foreach (PluginWeakFormAnalysis weakFormAnalysisVolume, coupling->weakFormAnalysisVolume)
            {
                foreach (PluginWeakFormAnalysis::Item item, weakFormAnalysisVolume.items)
                {
                    if (item.coupling == couplingType)
                        return true;
                }
            }
        }
    }

    return false;

    //    foreach (Item item, m_couplings)
    //    {
    //        if (item.source == sourceField && item.target == targetField)
    //        {
    //            foreach (Item::Analysis analysis, item.analyses)
    //            {
    //                if (analysis.couplingType == couplingType)
    //                    return true;
    //            }
    //        }
    //    }

    //    return false;
}

CouplingInfo::CouplingInfo(FieldInfo *sourceField,
                           FieldInfo *targetField,
                           CouplingType couplingType) :
    m_sourceField(sourceField),
    m_targetField(targetField),
    m_couplingType(couplingType)
{    

}

CouplingInfo::~CouplingInfo()
{
}

void CouplingInfo::setCouplingType(CouplingType couplingType)
{
    m_couplingType = couplingType;
}

CouplingType CouplingInfo::couplingType() const
{
    return m_couplingType;
}

QString CouplingInfo::couplingId() const
{
    assert(m_sourceField);
    assert(m_targetField);

    return m_sourceField->fieldId() + "-" + m_targetField->fieldId();
}

QString CouplingInfo::name() const
{
    return m_name;
}

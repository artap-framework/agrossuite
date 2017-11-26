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
#include "weak_form.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

#include "../../resources_source/classes/module_xml.h"

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

            // check whether coupling is available for values of source and target fields such as analysis type
            for (int i = 0; i < coup->volume().weakforms_volume().weakform_volume().size(); i++)
            {
                XMLModule::weakform_volume wf = coup->volume().weakforms_volume().weakform_volume().at(i);

                CouplingList::Item item;

                item.name = QString::fromStdString(coup->general_coupling().name());
                item.description = QString::fromStdString(coup->general_coupling().description());
                item.sourceField = QString::fromStdString(coup->general_coupling().modules().source().id());
                item.sourceAnalysisType = analysisTypeFromStringKey(QString::fromStdString(wf.sourceanalysis().get()));
                item.targetField = QString::fromStdString(coup->general_coupling().modules().target().id());
                item.targetAnalysisType = analysisTypeFromStringKey(QString::fromStdString(wf.analysistype()));
                item.couplingType = couplingTypeFromStringKey(QString::fromStdString(wf.couplingtype().get()));

                m_couplings.append(item);
            }
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
}

QString CouplingList::name(FieldInfo *sourceField, FieldInfo *targetField) const
{
    foreach (Item item, m_couplings)
    {
        if (item.sourceField == sourceField->fieldId() && item.targetField == targetField->fieldId())
            return item.name;
    }

    assert(0);
    return "";
}

QString CouplingList::description(FieldInfo *sourceField, FieldInfo *targetField) const
{
    foreach (Item item, m_couplings)
    {
        if (item.sourceField == sourceField->fieldId() && item.targetField == targetField->fieldId())
            return item.description;
    }

    assert(0);
    return "";
}

bool CouplingList::isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField, CouplingType couplingType) const
{
    foreach (Item item, m_couplings)
    {
        if (item.sourceAnalysisType == sourceField->analysisType() && item.targetAnalysisType == targetField->analysisType()
                && item.sourceField == sourceField->fieldId() && item.targetField == targetField->fieldId()
                && item.couplingType == couplingType)
            return true;
    }

    return false;
}

bool CouplingList::isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField) const
{
    foreach (Item item, m_couplings)
    {
        if (item.sourceAnalysisType == sourceField->analysisType() && item.targetAnalysisType == targetField->analysisType()
                && item.sourceField == sourceField->fieldId() && item.targetField == targetField->fieldId())
            return true;
    }

    return false;
}

bool CouplingList::isCouplingAvailable(QString sourceField, AnalysisType sourceAnalysis, QString targetField, AnalysisType targetAnalysis, CouplingType couplingType) const
{
    foreach (Item item, m_couplings)
    {
        if (item.sourceAnalysisType == sourceAnalysis && item.targetAnalysisType == targetAnalysis
                && item.sourceField == sourceField && item.targetField == targetField
                && item.couplingType == couplingType)
            return true;
    }

    return false;
}

bool CouplingList::isCouplingAvailable(QString sourceField, QString targetField, CouplingType couplingType) const
{
    foreach (Item item, m_couplings)
    {
        if (item.sourceField == sourceField && item.targetField == targetField
                && item.couplingType == couplingType)
            return true;
    }

    return false;
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
    return couplingList()->name(m_sourceField, m_targetField);
}

QString CouplingInfo::description() const
{
    return couplingList()->description(m_sourceField, m_targetField);
}

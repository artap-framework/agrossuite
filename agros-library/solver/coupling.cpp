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

bool isCouplingAvailable(QString sourceField, AnalysisType sourceAnalysis,
                         QString targetField, AnalysisType targetAnalysis,
                         CouplingType couplingType)
{
    foreach (PluginInterface *plugin, Agros::plugins().values())
    {
        foreach (QString coupling, plugin->couplings())
        {
            if (coupling == sourceField && plugin->fieldId() == targetField)
            {
                PluginCoupling *cpl= plugin->couplingJson(coupling);

                foreach (PluginWeakFormAnalysis weakFormAnalysisVolume, cpl->weakFormAnalysisVolume)
                {
                    foreach (PluginWeakFormAnalysis::Item item, weakFormAnalysisVolume.items)
                    {
                        if (item.analysis == targetAnalysis && item.analysisSource == sourceAnalysis && item.coupling == couplingType)
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

CouplingInfo::CouplingInfo(const QString &sourceId,
                           const QString &targetId,
                           const QString &name,
                           CouplingType couplingType) :
    m_sourceFieldId(sourceId),
    m_targetFieldId(targetId),
    m_name(name),
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
    return m_sourceFieldId + "-" + m_targetFieldId;
}

QString CouplingInfo::name() const
{
    return m_name;
}

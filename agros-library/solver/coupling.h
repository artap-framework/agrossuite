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

#ifndef COUPLING_H
#define COUPLING_H

#include "util/util.h"
#include "util/enums.h"

#include "solver/module.h"

class FieldInfo;
class ProblemConfig;
class PluginCoupling;

bool isCouplingAvailable(QString sourceField, AnalysisType sourceAnalysis,
                         QString targetField, AnalysisType targetAnalysis,
                         CouplingType couplingType);

class AGROS_LIBRARY_API CouplingInfo : public QObject
{
public:
    CouplingInfo(const QString &sourceId,
                 const QString &targetId,
                 const QString &name,
                 CouplingType couplingType = CouplingType_Weak);
    ~CouplingInfo();

    QString couplingId() const;

    CouplingType couplingType() const;
    void setCouplingType(CouplingType couplingType);

    inline QString sourceFieldId() {return m_sourceFieldId; }
    inline QString targetFieldId() {return m_targetFieldId; }

    // name
    QString name() const;

private:
    // pointers to problem infos
    QString m_sourceFieldId;
    QString m_targetFieldId;

    // coupling type
    CouplingType m_couplingType;

    // name
    QString m_name;
};

#endif // COUPLING_H

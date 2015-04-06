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

#include "util.h"
#include "util/enums.h"

#include "solver/module.h"

class FieldInfo;
class ProblemConfig;

class AGROS_LIBRARY_API CouplingList
{
public:
    struct Item
    {
        QString name;
        QString description;

        QString sourceField;
        AnalysisType sourceAnalysisType;
        QString targetField;
        AnalysisType targetAnalysisType;
        CouplingType couplingType;

        inline QString toString()
        {
            return QString("source: %1 (%2), target: %3 (%4), %5").arg(sourceField).arg(analysisTypeString(sourceAnalysisType)).arg(targetField).arg(analysisTypeString(targetAnalysisType)).arg(couplingTypeString(couplingType));
        }
    };

    CouplingList();

    QList<QString> availableCouplings();

    QString name(FieldInfo *sourceField, FieldInfo *targetField) const;
    QString description(FieldInfo *sourceField, FieldInfo *targetField) const;
    bool isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField) const;
    bool isCouplingAvailable(FieldInfo *sourceField, FieldInfo *targetField, CouplingType couplingType) const;
    bool isCouplingAvailable(QString sourceField, AnalysisType sourceAnalysis, QString targetField, AnalysisType targetAnalysis, CouplingType couplingType) const;
    bool isCouplingAvailable(QString sourceField, QString targetField, CouplingType couplingType) const;

private:
    QList<Item> m_couplings;
};

// cached coupling list
AGROS_LIBRARY_API CouplingList *couplingList();

class AGROS_LIBRARY_API CouplingInfo : public QObject
{
    Q_OBJECT
public:
    CouplingInfo(FieldInfo* sourceField, FieldInfo* targetField,
                 CouplingType couplingType = CouplingType_Weak);
    ~CouplingInfo();

    QString couplingId() const;

    CouplingType couplingType() const;
    void setCouplingType(CouplingType couplingType);

    inline FieldInfo *sourceField() {return m_sourceField; }
    inline FieldInfo *targetField() {return m_targetField; }

    // name
    QString name() const;
    // description
    QString description() const;

    // weak forms
    static QList<FormInfo> wfMatrixVolumeSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType);
    static QList<FormInfo> wfVectorVolumeSeparated(XMLModule::volume* volume, AnalysisType sourceAnalysis, AnalysisType targetAnalysis, CouplingType couplingType, LinearityType linearityType);

private:
    // pointers to problem infos
    FieldInfo *m_sourceField;
    FieldInfo *m_targetField;

    // coupling type
    CouplingType m_couplingType;

signals:
    void invalidated();
};

#endif // COUPLING_H

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

#ifndef SOLUTIONSTORE_H
#define SOLUTIONSTORE_H

#include "solutiontypes.h"

#include <deal.II/hp/fe_collection.h>

class Computation;

class AGROS_LIBRARY_API SolutionStore : public QObject
{
public:
    SolutionStore(Computation *parentProblem);
    ~SolutionStore();

    class SolutionRunTimeDetails
    {
    public:
        enum Type
        {
            TimeStepLength,
            AdaptivityError,
            DOFs
        };

        SolutionRunTimeDetails();

        void clear();

        bool load(QJsonObject &results);
        bool save(QJsonObject &results);

        inline QString typeToStringKey(Type type) { return m_runtimeKey[type]; }
        inline Type stringKeyToType(const QString &key) { return m_runtimeKey.key(key); }

        inline QVariant value(Type type) const { return m_runtime[type]; }
        inline void setValue(Type type, int value) {  m_runtime[type] = value; }
        inline void setValue(Type type, double value) {  m_runtime[type] = value; }
        inline void setValue(Type type, bool value) {  m_runtime[type] = value; }
        inline void setValue(Type type, const QString &value) { m_runtime[type] = value; }

        inline QVariant defaultValue(Type type) {  return m_runtimeDefault[type]; }

    private:
        QMap<Type, QVariant> m_runtime;
        QMap<Type, QVariant> m_runtimeDefault;
        QMap<Type, QString> m_runtimeKey;

        void setDefaultValues();
        void setStringKeys();
    };

    bool contains(FieldSolutionID solutionID) const;
    MultiArray &multiArray(FieldSolutionID solutionID);

    void addSolution(FieldSolutionID solutionID, dealii::hp::DoFHandler<2> &doFHandler, dealii::Vector<double> &solution, SolutionRunTimeDetails runTime);
    void replaceSolution(FieldSolutionID solutionID, MultiArray &ma);
    void removeSolution(FieldSolutionID solutionID, bool saveRunTime = true);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(const FieldInfo *fieldInfo, int timeStep = -1) const;
    int lastTimeStep(const FieldInfo *fieldInfo) const;

    bool loadRunTimeDetails();
    bool saveRunTimeDetails();
    SolutionRunTimeDetails multiSolutionRunTimeDetail(FieldSolutionID solutionID) const { assert(m_multiSolutionRunTimeDetails.contains(solutionID)); return m_multiSolutionRunTimeDetails[solutionID]; }

    inline bool isEmpty() const { return m_multiSolutions.isEmpty(); }    
    void clear();

private:
    Computation *m_computation;

    QList<FieldSolutionID> m_multiSolutions;
    QMap<FieldSolutionID, SolutionRunTimeDetails> m_multiSolutionRunTimeDetails;
    QMap<FieldSolutionID, MultiArray> m_multiSolutionDealCache;
    QList<FieldSolutionID> m_multiSolutionCacheIDOrder;

    void insertMultiSolutionToCache(FieldSolutionID solutionID, dealii::hp::DoFHandler<2> &doFHandler, dealii::Vector<double> &solution);

    QString baseStoreFileName(FieldSolutionID solutionID) const;    
};

#endif // SOLUTIONSTORE_H

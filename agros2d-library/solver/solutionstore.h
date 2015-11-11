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

#undef signals
#include <deal.II/hp/fe_collection.h>
#define signals public

class ProblemComputation;

class AGROS_LIBRARY_API SolutionStore : public QObject
{
    Q_OBJECT
public:
    SolutionStore(ProblemComputation *parentProblem);
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
        ~SolutionRunTimeDetails() {}

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
    MultiArray multiArray(FieldSolutionID solutionID);

    void addSolution(FieldSolutionID solutionID, MultiArray multiArray, SolutionRunTimeDetails runTime);
    void removeSolution(FieldSolutionID solutionID, bool saveRunTime = true);

    // last adaptive step for given time step. If time step not given, last time step used implicitly
    int lastAdaptiveStep(const FieldInfo *fieldInfo, int timeStep = -1) const;
    int lastTimeStep(const FieldInfo *fieldInfo) const;

    void loadRunTimeDetails();
    void saveRunTimeDetails();
    SolutionRunTimeDetails multiSolutionRunTimeDetail(FieldSolutionID solutionID) const { assert(m_multiSolutionRunTimeDetails.contains(solutionID)); return m_multiSolutionRunTimeDetails[solutionID]; }

    inline bool isEmpty() const { return m_multiSolutions.isEmpty(); }

    inline QMap<QString, double> results() { return m_results; }
    inline bool hasResults() const { return !m_results.isEmpty(); }
    inline void setResult(QString key, double value) { m_results[key] = value; }
    inline void removeResult(QString key) { m_results.remove(key); }

public slots:
    void clear();

private:
    ProblemComputation *m_computation;

    QList<FieldSolutionID> m_multiSolutions;
    QMap<FieldSolutionID, SolutionRunTimeDetails> m_multiSolutionRunTimeDetails;
    QMap<FieldSolutionID, MultiArray> m_multiSolutionDealCache;
    QList<FieldSolutionID> m_multiSolutionCacheIDOrder;
    QMap<QString, double> m_results;

    void insertMultiSolutionToCache(FieldSolutionID solutionID, MultiArray multiArray);

    QString baseStoreFileName(FieldSolutionID solutionID) const;

    // consts
    const QString RESULTS = "results";
    const QString SOLUTIONS = "solutions";
    const QString FIELDID = "fieldid";
    const QString TIMESTEP = "timestep";
    const QString ADAPTIVITYSTEP = "adaptivitystep";
    const QString RUNTIME = "runtime";
};

#endif // SOLUTIONSTORE_H

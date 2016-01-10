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

#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"
#include "problem_config.h"

#include "mesh/meshgenerator.h"

#include "pythonlab/pythonengine.h"

class ProblemSolver;

class FieldInfo;
class CouplingInfo;

class ProblemBase;
class ProblemConfig;
class ProblemSetting;
class PyProblem;

class Computation;
class PostDeal;
class SolutionStore;
class ProblemResults;
class Studies;

class CalculationThread : public QThread
{
   Q_OBJECT

public:
    enum CalculationType
    {
        CalculationType_Mesh,
        CalculationType_Solve,
        CalculationType_SolveTimeStep
    };

    CalculationThread(Computation *parentProblem);

    void startCalculation(CalculationType type);

protected:
   virtual void run();

signals:
   void signalValueUpdated(QString);

private:
    CalculationType m_calculationType;
    Computation *m_computation;
};

class AGROS_LIBRARY_API ProblemBase : public QObject
{
    Q_OBJECT
signals:

    /// emited when an field is added or removed. Menus need to adjusted
    void fieldsChanged();

    /// emited when an field is added or removed. Menus need to adjusted
    void couplingsChanged();

public slots:
    virtual void clearFieldsAndConfig();

public:
    ProblemBase();
    virtual ~ProblemBase();

    inline ProblemConfig *config() const { return m_config; }
    inline ProblemSetting *setting() const { return m_setting; }
    inline Scene *scene() { return m_scene; }

    bool isTransient() const;
    int numTransientFields() const;
    bool isHarmonic() const;
    inline bool isNonlinear() const { return m_isNonlinear; }
    bool determineIsNonlinear() const; // slow version
    int numAdaptiveFields() const;

    // check and apply parameters
    bool checkAndApplyParameters(StringToDoubleMap parameters, bool apply = true);

    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(const QString &fieldId) { assert(m_fieldInfos.contains(fieldId));
                                                          return m_fieldInfos[fieldId]; }
    inline FieldInfo *fieldInfo(const std::string &name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromLatin1(name)); }
    inline bool hasField(const QString &fieldId) const { return m_fieldInfos.contains(fieldId); }
    void addField(FieldInfo *field);
    void removeField(FieldInfo *field);

    void synchronizeCouplings();
    inline QMap<QPair<FieldInfo *, FieldInfo *>, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo *couplingInfo(FieldInfo *sourceField, FieldInfo *targetField) { assert (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)));
                                                                                        return m_couplingInfos[QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)]; }
    inline CouplingInfo *couplingInfo(const QString &sourceFieldId, const QString &targetFieldId) { return couplingInfo(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline bool hasCoupling(FieldInfo *sourceField, FieldInfo *targetField) { return (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField))); }
    inline bool hasCoupling(const QString &sourceFieldId, const QString &targetFieldId) { return hasCoupling(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }

    virtual QString problemFileName() const = 0;

    void importProblemFromA2D(const QString &fileName);
    void exportProblemToA2D(const QString &fileName);

    void readProblemFromJson(const QString &fileName = "");
    void writeProblemToJson(const QString &fileName = "");

protected:
    Scene *m_scene;

    ProblemConfig *m_config;
    ProblemSetting *m_setting;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo *, FieldInfo *>, CouplingInfo *> m_couplingInfos;

    // determined in create structure to speed up the calculation
    bool m_isNonlinear;

    // transient analysis
    QList<double> m_timeStepLengths;

    // private local dict
    PyObject *m_dictLocal;

    friend class CalculationThread;
    friend class PyProblem;
    friend class AgrosSolver;
    friend class Problem;
    friend class Computation;
    friend class Scene;

private:
    bool applyParametersInternal();
};

class AGROS_LIBRARY_API Problem : public ProblemBase
{
    Q_OBJECT

public:
    Problem();
    ~Problem();

    QSharedPointer<Computation> createComputation(bool newComputation = false, bool setCurrentComputation = true);
    inline QSharedPointer<Computation> currentComputation() { return m_currentComputation; }

    void readProblemFromArchive(const QString &archiveFileName);
    void writeProblemToArchive(const QString &archiveFileName, bool onlyProblemFile = true);
    void readProblemFromFile(const QString &archiveFileName);

    inline Studies *studies() { return m_studies; }

    // filenames
    virtual QString problemFileName() const;
    inline QString archiveFileName() const { return m_fileName; }

signals:
    void fileNameChanged(const QString &archiveFileName);

public slots:
    virtual void clearFieldsAndConfig();

private:
    QString m_fileName;
    QSharedPointer<Computation> m_currentComputation;
    Studies *m_studies;

    friend class PyComputation;
    friend class PyProblem;
};

class AGROS_LIBRARY_API Computation : public ProblemBase
{
    Q_OBJECT

public:
    Computation(const QString &problemDir = "");
    virtual ~Computation();

    // mesh
    void meshThread();
    // solve
    void solveThread();

    inline PostDeal *postDeal() { return m_postDeal; }
    inline ProblemSolver *problemSolver() { return m_problemSolver; }
    inline SolutionStore *solutionStore() { return m_solutionStore; }
    inline ProblemResults *result() const { return m_result; }

    bool isSolved() const;
    bool isSolving() const { return m_isSolving; }
    bool isMeshed() const;
    bool isMeshing() const { return m_isMeshing; }
    bool isAborted() const { return m_abort; }
    bool isPreparedForAction() const { return !isMeshing() && !isSolving() && !m_isPostprocessingRunning; }

    // results
    bool loadResults();
    bool saveResults();

    inline QTime timeElapsed() const { return m_lastTimeElapsed; }

    void setActualTimeStepLength(double timeStep);
    void removeLastTimeStepLength();

    // time step lengths
    QList<double> timeStepLengths() const { return m_timeStepLengths; }
    // cumulative times
    QList<double> timeStepTimes() const;
    double timeStepToTotalTime(int timeStepIndex) const;
    int timeLastStep() const { return m_timeStepLengths.length() - 1; }

    // read initial meshes and solution
    void readInitialMeshFromFile(bool emitMeshed, QSharedPointer<MeshGenerator> meshGenerator = QSharedPointer<MeshGenerator>(nullptr));

    inline const dealii::Triangulation<2> &initialMesh() const { return m_initialMesh; }
    inline dealii::Triangulation<2> &calculationMesh() { return m_calculationMesh; }
    inline void setCalculationMesh(dealii::Triangulation<2> newMesh) { m_calculationMesh.copy_triangulation(newMesh); }

    void propagateBoundaryMarkers();

    void setIsPostprocessingRunning(bool isPostprocessingRunning = true) { m_isPostprocessingRunning = isPostprocessingRunning; }

    bool mesh(bool emitMeshed);
    void solve();

    void meshWithGUI();
    void solveWithGUI();

    virtual QString problemFileName() const;
    inline QString problemDir() { return m_problemDir; }

signals:
    void meshed();
    void solved();

public slots:    
    void clearSolution();
    void clearResults();
    void doAbortSolve();
    virtual void clearFieldsAndConfig();

protected:
    bool m_isSolving;
    bool m_isMeshing;
    bool m_abort;

    bool m_isPostprocessingRunning;

    // initial mesh
    dealii::Triangulation<2> m_initialMesh;
    dealii::Triangulation<2> m_initialUnrefinedMesh;
    // calculation mesh - at the present moment we do not use multimesh
    dealii::Triangulation<2> m_calculationMesh;
    ProblemResults *m_result;

    // solution store
    SolutionStore *m_solutionStore;

    QTime m_lastTimeElapsed;

    // problem dir in cache
    QString m_problemDir;

    CalculationThread *m_calculationThread;

    // post deal
    ProblemSolver *m_problemSolver;
    PostDeal *m_postDeal;

    void solveInit(); // called by solve, can throw SolverException
    void solveAction(); // called by solve, can throw SolverException
    bool meshAction(bool emitMeshed);
};

#endif // PROBLEM_H

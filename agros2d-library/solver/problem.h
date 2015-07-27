#ifndef PROBLEM_H
#define PROBLEM_H

#include "util.h"
#include "value.h"
#include "solutiontypes.h"

#include "mesh/meshgenerator.h"

class ProblemSolver;

class FieldInfo;
class CouplingInfo;

class Problem;
class ProblemConfig;
class ProblemSetting;
class PyProblem;

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

    CalculationThread();

    void startCalculation(CalculationType type);

//public slots:
//    void stopRunning();

protected:
   virtual void run();

signals:
   void signalValueUpdated(QString);

private:
    CalculationType m_calculationType;

};


/// intented as central for solution process
/// shielded from gui and QT
/// holds data describing individual fields, means of coupling and solutions
class AGROS_LIBRARY_API Problem : public QObject
{
    Q_OBJECT

signals:
    void timeStepChanged();
    void meshed();
    void solved();

    /// emited when an field is added or removed. Menus need to adjusted
    void fieldsChanged();

    /// emited when an field is added or removed. Menus need to adjusted
    void couplingsChanged();

    void clearedSolution();

public slots:
    // clear problem
    void clearSolution();
    void clearFieldsAndConfig();
    void doAbortSolve();

public:
    Problem();
    ~Problem();

    inline ProblemConfig *config() const { return m_config; }
    inline ProblemSetting *setting() const { return m_setting; }

    // void createStructure();

    // mesh
    void mesh();
    // solve
    void solve();

    // check geometry
    bool checkGeometry();

    bool isSolved() const;
    bool isSolving() const { return m_isSolving; }
    bool isMeshed() const;
    bool isMeshing() const { return m_isMeshing; }
    bool isAborted() const { return m_abort; }
    bool isPreparedForAction() const { return !isMeshing() && !isSolving() && !m_isPostprocessingRunning; }

    inline QAction *actionMesh() { return actMesh; }
    inline QAction *actionSolve() { return actSolve; }    

    bool isTransient() const;
    int numTransientFields() const;
    bool isHarmonic() const;
    inline bool isNonlinear() const { return m_isNonlinear; }
    bool determineIsNonlinear() const; // slow version
    int numAdaptiveFields() const;

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
    void readSolutionsFromFile();

    inline const dealii::Triangulation<2> &initialMesh() const { return m_initialMesh; }
    inline dealii::Triangulation<2> &calculationMesh() { return m_calculationMesh; }
    inline void setCalculationMesh(dealii::Triangulation<2> newMesh) { m_calculationMesh.copy_triangulation(newMesh); }

    void propagateBoundaryMarkers();

    void setIsPostprocessingRunning(bool pr = true) { m_isPostprocessingRunning = pr; }

private:
    ProblemConfig *m_config;
    ProblemSetting *m_setting;

    ProblemSolver *m_solverDeal;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo *, FieldInfo *>, CouplingInfo *> m_couplingInfos;

    // initial mesh
    dealii::Triangulation<2> m_initialMesh;
    dealii::Triangulation<2> m_initialUnrefinedMesh;
    // calculation mesh - at the present moment we do not use multimesh
    dealii::Triangulation<2> m_calculationMesh;

    QTime m_lastTimeElapsed;

    bool m_isSolving;
    bool m_isMeshing;
    bool m_abort;

    bool m_isPostprocessingRunning;

    CalculationThread *m_calculationThread;

    QAction *actMesh;
    QAction *actSolve;

    // determined in create structure to speed up the calculation
    bool m_isNonlinear;

    // transient analysis
    QList<double> m_timeStepLengths;

    bool mesh(bool emitMeshed);
    bool meshAction(bool emitMeshed);
    void solveInit();
    void solve(bool commandLine);
    void solveAction(); // called by solve, can throw SolverException

    friend class CalculationThread;
    friend class PyProblem;
    friend class AgrosSolver;

private slots:
    void doMeshWithGUI();
    void doSolveWithGUI();
};

#endif // PROBLEM_H

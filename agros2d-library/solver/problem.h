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

#undef signals
#include <deal.II/base/point.h>
#include <deal.II/numerics/data_out.h>
#define signals public

#include "util/util.h"
#include "value.h"
#include "solutiontypes.h"
#include "problem_config.h"
#include "problem_parameter.h"

#include "mesh/meshgenerator.h"

class ProblemSolver;

class FieldInfo;
class CouplingInfo;

class ProblemBase;
class ProblemConfig;
class PostprocessorSetting;
class PyProblem;

class Computation;
class SolutionStore;
class ResultRecipes;
class ComputationResults;
class Studies;

struct PostTriangle
{
    PostTriangle(dealii::Point<2> a, dealii::Point<2> b, dealii::Point<2> c,
                 double va, double vb, double vc)
    {
        this->vertices[0] = a;
        this->vertices[1] = b;
        this->vertices[2] = c;

        this->values[0] = va;
        this->values[1] = vb;
        this->values[2] = vc;
    }

    dealii::Point<2> vertices[3];
    double values[3];
};

class PostDataOut : public dealii::DataOut<2, dealii::hp::DoFHandler<2> >
{
public:
    PostDataOut(FieldInfo *fieldInfo, Computation *parentProblem);

    void compute_nodes(QList<PostTriangle> &values, bool deform = false);

#ifdef _MSC_VER
    virtual dealii::DataOut<2>::cell_iterator first_cell();
    virtual dealii::DataOut<2>::cell_iterator next_cell(const dealii::DataOut<2>::cell_iterator &old_cell);
#else
    virtual typename dealii::DataOut<2>::cell_iterator first_cell();
    virtual typename dealii::DataOut<2>::cell_iterator next_cell(const typename dealii::DataOut<2>::cell_iterator &old_cell);
#endif

    inline double min() const { return m_min; }
    inline double max() const { return m_max; }

private:
    double m_min;
    double m_max;

    Computation *m_computation;
    const FieldInfo *m_fieldInfo;

    void compute_node(dealii::Point<2> &node, const dealii::DataOutBase::Patch<2> *patch,
                      const unsigned int xstep, const unsigned int ystep, const unsigned int zstep,
                      const unsigned int n_subdivisions);
};

class PostDeal : public QObject
{
    Q_OBJECT

public:
    PostDeal(Computation *computation);
    ~PostDeal();

    // contour
    inline QList<PostTriangle> &contourValues() { return m_contourValues; }

    // scalar view
    inline QList<PostTriangle> &scalarValues() { return m_scalarValues; }

    // vector view
    inline QList<PostTriangle> &vectorXValues() { return m_vectorXValues; }
    inline QList<PostTriangle> &vectorYValues() { return m_vectorYValues; }

    std::shared_ptr<PostDataOut> viewScalarFilter(Module::LocalVariable physicFieldVariable,
                                                  PhysicFieldVariableComp physicFieldVariableComp);

    // view
    inline FieldInfo* activeViewField() const { return m_activeViewField; }
    void setActiveViewField(FieldInfo* fieldInfo);

    inline int activeTimeStep() const { return m_activeTimeStep; }
    void setActiveTimeStep(int ts);

    inline int activeAdaptivityStep() const { return m_activeAdaptivityStep; }
    void setActiveAdaptivityStep(int as);

    MultiArray activeMultiSolutionArray();

    inline bool isProcessed() const { return m_isProcessed; }

signals:
    void processed();

public slots:
    void refresh();
    void clear();
    void clearView();

private:
    Computation *m_computation;

    bool m_isProcessed;

    // contour
    QList<PostTriangle> m_contourValues;
    // scalar view
    QList<PostTriangle> m_scalarValues;
    // vector view
    QList<PostTriangle> m_vectorXValues;
    QList<PostTriangle> m_vectorYValues;

    // view
    FieldInfo *m_activeViewField;
    int m_activeTimeStep;
    int m_activeAdaptivityStep;

    // stored shared pointers for keeping the instance around
    std::shared_ptr<dealii::DataPostprocessorScalar<2> > m_post;

private slots:
    void processSolved();

    void processRangeContour();
    void processRangeScalar();
    void processRangeVector();
};

class SolveThread : public QThread
{
    Q_OBJECT

public:
    SolveThread(Computation *computation) : QThread(), m_computation(computation)
    {
        connect(this, SIGNAL(finished()), this, SLOT(finished()));
    }

    inline void startCalculation() { start(QThread::TimeCriticalPriority); }

protected:
    virtual void run();

private slots:
    void finished();

 private:
     Computation *m_computation;
};

class AGROS_LIBRARY_API ProblemBase : public QObject
{
    Q_OBJECT

public:
    ProblemBase();
    virtual ~ProblemBase();

    inline ProblemConfig *config() const { return m_config; }
    inline Scene *scene() { return m_scene; }

    bool isTransient() const;
    int numTransientFields() const;
    bool isHarmonic() const;
    inline bool isNonlinear() const { return m_isNonlinear; }
    bool determineIsNonlinear() const; // slow version
    int numAdaptiveFields() const;

    // check and apply parameters
    bool checkAndApplyParameters(QMap<QString, ProblemParameter> parameters, bool apply = true);

    // field
    inline QMap<QString, FieldInfo *> fieldInfos() const { return m_fieldInfos; }
    inline FieldInfo *fieldInfo(const QString &fieldId) { assert(m_fieldInfos.contains(fieldId));                                                          return m_fieldInfos[fieldId]; }
    inline FieldInfo *fieldInfo(const std::string &name) { return fieldInfo(QString::fromStdString(name)); }
    inline FieldInfo *fieldInfo(const char* name) { return fieldInfo(QString::fromLatin1(name)); }
    inline bool hasField(const QString &fieldId) const { return m_fieldInfos.contains(fieldId); }
    void addField(FieldInfo *field);
    virtual void removeField(FieldInfo *field);

    void synchronizeCouplings();
    inline QMap<QPair<FieldInfo *, FieldInfo *>, CouplingInfo* > couplingInfos() const { return m_couplingInfos; }
    inline CouplingInfo *couplingInfo(FieldInfo *sourceField, FieldInfo *targetField) { assert (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)));
                                                                                        return m_couplingInfos[QPair<FieldInfo*, FieldInfo* >(sourceField, targetField)]; }
    inline CouplingInfo *couplingInfo(const QString &sourceFieldId, const QString &targetFieldId) { return couplingInfo(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }
    inline bool hasCoupling(FieldInfo *sourceField, FieldInfo *targetField) { return (m_couplingInfos.contains(QPair<FieldInfo*, FieldInfo* >(sourceField, targetField))); }
    inline bool hasCoupling(const QString &sourceFieldId, const QString &targetFieldId) { return hasCoupling(fieldInfo(sourceFieldId), fieldInfo(targetFieldId)); }

    inline const dealii::Triangulation<2> &initialMesh() const { return m_initialMesh; }

    bool isMeshed() const;
    bool isMeshing() const { return m_isMeshing; }

    bool mesh();    

    virtual QString problemFileName() const = 0;

    void importProblemFromA2D(const QString &fileName);
    void exportProblemToA2D(const QString &fileName);

    void readProblemFromJson(const QString &fileName = "");
    void writeProblemToJson(const QString &fileName = "");

signals:
    /// emited when an field is added or removed. Menus need to adjusted
    void fieldsChanged();

    /// emited when an field is added or removed. Menus need to adjusted
    void couplingsChanged();

    void meshed();

public slots:
    virtual void clearFields();
    virtual void clearFieldsAndConfig();

protected:
    Scene *m_scene;

    ProblemConfig *m_config;

    QMap<QString, FieldInfo *> m_fieldInfos;
    QMap<QPair<FieldInfo *, FieldInfo *>, CouplingInfo *> m_couplingInfos;

    // determined in create structure to speed up the calculation
    bool m_isNonlinear;

    // transient analysis
    QList<double> m_timeStepLengths;

    // initial mesh
    dealii::Triangulation<2> m_initialMesh;
    dealii::Triangulation<2> m_initialUnrefinedMesh;
    bool m_isMeshing;

    // read initial meshes
    void readInitialMeshFromFile(const QString &problemDir, bool emitMeshed);

    virtual void readProblemFromJsonInternal(QJsonObject &rootJson);
    virtual void writeProblemToJsonInternal(QJsonObject &rootJson);

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

    QSharedPointer<Computation> createComputation(bool newComputation);
    inline QSharedPointer<Computation> currentComputation() { return m_currentComputation; }

    void readProblemFromArchive(const QString &archiveFileName);
    void writeProblemToArchive(const QString &archiveFileName, bool onlyProblemFile = true);
    void readProblemFromFile(const QString &archiveFileName);

    // field
    virtual void removeField(FieldInfo *field);

    // filenames
    virtual QString problemFileName() const;
    inline QString archiveFileName() const { return m_fileName; }

    // recipes
    inline ResultRecipes *recipes() const { return m_recipes; }

    // studies
    inline Studies *studies() { return m_studies; }

signals:
    void fileNameChanged(const QString &archiveFileName);

public slots:
    virtual void clearFieldsAndConfig();

private:
    QString m_fileName;
    QSharedPointer<Computation> m_currentComputation;

    friend class PyComputation;
    friend class PyProblem;

protected:
    // recipes
    ResultRecipes *m_recipes;
    // studies
    Studies *m_studies;

    virtual void readProblemFromJsonInternal(QJsonObject &rootJson);
    virtual void writeProblemToJsonInternal(QJsonObject &rootJson);
};

class AGROS_LIBRARY_API Computation : public ProblemBase
{
    Q_OBJECT

public:
    Computation(const QString &problemDir = "");
    virtual ~Computation();

    inline PostprocessorSetting *setting() const { return m_setting; }

    inline ProblemSolver *problemSolver() { return m_problemSolver; }
    inline SolutionStore *solutionStore() { return m_solutionStore; }

    bool isSolved() const;
    bool isSolving() const { return m_isSolving; }
    bool isAborted() const { return m_abort; }
    bool isPreparedForAction() const { return !isMeshing() && !isSolving() && !m_isPostprocessingRunning; }

    inline QTime timeElapsed() const { return m_lastTimeElapsed; }

    void setActualTimeStepLength(double timeStep);
    void removeLastTimeStepLength();

    // time step lengths
    QList<double> timeStepLengths() const { return m_timeStepLengths; }
    // cumulative times
    QList<double> timeStepTimes() const;
    double timeStepToTotalTime(int timeStepIndex) const;
    int timeLastStep() const { return m_timeStepLengths.length() - 1; }

    inline dealii::Triangulation<2> &calculationMesh() { return m_calculationMesh; }
    inline void setCalculationMesh(const dealii::Triangulation<2> &newMesh) { m_calculationMesh.copy_triangulation(newMesh); }

    void propagateBoundaryMarkers();

    void setIsPostprocessingRunning(bool isPostprocessingRunning = true) { m_isPostprocessingRunning = isPostprocessingRunning; }

    void solve();
    void solveWithThread();

    virtual QString problemFileName() const;
    void readFromProblem();
    inline QString problemDir() { return m_problemDir; }

    // results
    inline ComputationResults *results() const { return m_results; }

    inline PostDeal *postDeal() { return m_postDeal; }

signals:
    void solved();
    void cleared();
    void solvedWithThread();

public slots:    
    virtual void clearFields();
    void clearSolution();
    void clearResults();
    virtual void clearFieldsAndConfig();

    void doAbortSolve();

protected:
    bool m_isSolving;
    bool m_abort;

    bool m_isPostprocessingRunning;

    PostprocessorSetting *m_setting;

    // calculation mesh
    dealii::Triangulation<2> m_calculationMesh;

    // solution store
    SolutionStore *m_solutionStore;

    QTime m_lastTimeElapsed;

    // problem dir in cache
    QString m_problemDir;

    ProblemSolver *m_problemSolver;

    void solveInit(); // called by solve, can throw SolverException

    virtual void readProblemFromJsonInternal(QJsonObject &rootJson);
    virtual void writeProblemToJsonInternal(QJsonObject &rootJson);

    // results
    ComputationResults *m_results;

    // post deal
    PostDeal *m_postDeal;
};

#endif // PROBLEM_H

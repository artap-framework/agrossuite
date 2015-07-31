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

#include <boost/config.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "problem.h"
#include "problem_config.h"

#include "util/global.h"
#include "util/constants.h"

#include "field.h"
#include "solutionstore.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenebasic.h"
#include "scenenode.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "weak_form.h"
//#include "module.h"
#include "coupling.h"
#include "solver.h"
#include "logview.h"

#include "pythonlab/pythonengine.h"

#include "mesh/meshgenerator_triangle.h"
#include "mesh/meshgenerator_cubit.h"
#include "mesh/meshgenerator_gmsh.h"
// #include "mesh/meshgenerator_netgen.h"

CalculationThread::CalculationThread() : QThread()
{
}

void CalculationThread::startCalculation(CalculationType type)
{
    m_calculationType = type;
    start(QThread::TimeCriticalPriority);
}

void CalculationThread::run()
{
    Agros2D::log()->printHeading(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));

    dealii::deal_II_exceptions::disable_abort_on_exception();

    switch (m_calculationType)
    {
    case CalculationType_Mesh:
        Agros2D::problem()->mesh(true);
        break;
    case CalculationType_Solve:
        Agros2D::problem()->solve(false);
        break;
    case CalculationType_SolveTimeStep:
        assert(0);
        break;
    default:
        assert(0);
    }
}

Problem::Problem()
{
    // m_timeStep = 0;
    m_lastTimeElapsed = QTime();
    m_isSolving = false;
    m_isMeshing = false;
    m_abort = false;
    m_isPostprocessingRunning = false;

    m_config = new ProblemConfig();
    m_setting = new ProblemSetting();
    m_calculationThread = new CalculationThread();

    m_isNonlinear = false;

    m_timeStepLengths.append(0.0);

    m_solverDeal = new ProblemSolver();

    actMesh = new QAction(icon("scene-meshgen"), tr("&Mesh area"), this);
    actMesh->setShortcut(QKeySequence(tr("Alt+W")));
    connect(actMesh, SIGNAL(triggered()), this, SLOT(doMeshWithGUI()));

    actSolve = new QAction(icon("run"), tr("&Solve"), this);
    actSolve->setShortcut(QKeySequence(tr("Alt+S")));
    connect(actSolve, SIGNAL(triggered()), this, SLOT(doSolveWithGUI()));

    connect(m_config, SIGNAL(changed()), this, SLOT(clearSolution()));
}

Problem::~Problem()
{
    clearSolution();
    clearFieldsAndConfig();

    delete m_config;
    delete m_setting;
    delete m_calculationThread;

    delete m_solverDeal;
}

bool Problem::isMeshed() const
{
    if (Agros2D::problem()->initialMesh().n_active_cells() == 0)
        return false;

    return (m_fieldInfos.size() > 0);
}

bool Problem::isSolved() const
{
    // return (!Agros2D::solutionStore()->isEmpty() && !m_isSolving && !m_isMeshing);
    return (!Agros2D::solutionStore()->isEmpty());
}

int Problem::numAdaptiveFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->adaptivityType() != AdaptivityMethod_None)
            num++;
    return num;
}

int Problem::numTransientFields() const
{
    int num = 0;
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Transient)
            num++;
    return num;
}

bool Problem::isTransient() const
{
    return numTransientFields() > 0;
}

bool Problem::isHarmonic() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->analysisType() == AnalysisType_Harmonic)
            return true;

    return false;
}

bool Problem::determineIsNonlinear() const
{
    foreach (FieldInfo* fieldInfo, m_fieldInfos)
        if (fieldInfo->linearityType() != LinearityType_Linear)
            return true;

    return false;
}

void Problem::clearSolution()
{
    m_abort = false;

    // m_timeStep = 0;
    m_lastTimeElapsed = QTime();
    m_timeStepLengths.clear();
    m_timeStepLengths.append(0.0);

    m_initialMesh.clear();
    m_initialUnrefinedMesh.clear();
    m_calculationMesh.clear();

    Agros2D::solutionStore()->clearAll();

    // remove cache
    removeDirectory(cacheProblemDir());

    emit clearedSolution();
}

void Problem::clearFieldsAndConfig()
{
    clearSolution();

    // clear fields
    m_solverDeal->clear();

    // clear couplings
    foreach (CouplingInfo* couplingInfo, m_couplingInfos)
        delete couplingInfo;
    m_couplingInfos.clear();

    QMapIterator<QString, FieldInfo *> i(m_fieldInfos);
    while (i.hasNext())
    {
        i.next();
        removeField(i.value());
        delete i.value();
    }
    m_fieldInfos.clear();

    // clear config
    m_config->clear();
    m_setting->clear();
}

void Problem::addField(FieldInfo *field)
{
    clearSolution();

    // remove field
    if (hasField(field->fieldId()))
    {
        removeField(m_fieldInfos[field->fieldId()]);
        delete m_fieldInfos[field->fieldId()];
    }

    // add to the collection
    m_fieldInfos[field->fieldId()] = field;

    // couplings
    synchronizeCouplings();

    emit fieldsChanged();
}

void Problem::removeField(FieldInfo *field)
{
    clearSolution();

    // first remove references to markers of this field from all edges and labels
    Agros2D::scene()->edges->removeFieldMarkers(field);
    Agros2D::scene()->labels->removeFieldMarkers(field);

    // then remove them from lists of markers - here they are really deleted
    Agros2D::scene()->boundaries->removeFieldMarkers(field);
    Agros2D::scene()->materials->removeFieldMarkers(field);

    // remove from the collection
    m_fieldInfos.remove(field->fieldId());

    synchronizeCouplings();

    currentPythonEngine()->runExpression(QString("agros2d.__remove_field__(\"%1\")").arg(field->fieldId()));

    emit fieldsChanged();
}

bool Problem::mesh(bool emitMeshed)
{
    bool result = false;

    // TODO: make global check geometry before mesh() and solve()
    if (Agros2D::problem()->fieldInfos().count() == 0)
    {
        Agros2D::log()->printError(tr("Mesh"), tr("No fields defined"));
        return false;
    }

    m_isMeshing = true;

    try
    {
        result = meshAction(emitMeshed);
    }
    catch (AgrosGeometryException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        m_isMeshing = false;
        Agros2D::log()->printError(tr("Geometry"), QString("%1").arg(e.what()));
        return false;
    }
    catch (AgrosMeshException& e)
    {
        // this assumes that all the code in Hermes and Agros is exception-safe
        // todo:  this is almost certainly not the case, at least for Agros. It should be further investigated
        m_isMeshing = false;
        Agros2D::log()->printError(tr("Mesh"), QString("%1").arg(e.what()));
        return false;
    }
    catch (AgrosException& e)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        m_isMeshing = false;
        Agros2D::log()->printWarning(tr("Mesh"), e.what());
        return false;
    }
    catch (dealii::ExceptionBase &e)
    {
        m_isMeshing = false;
        Agros2D::log()->printWarning(tr("Mesh (deal.II)"), e.what());
        return false;
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not safe at all
        m_isMeshing = false;
        Agros2D::log()->printWarning(tr("Mesh"), tr("An unknown exception occurred and has been ignored"));
        qDebug() << "Mesh: An unknown exception occurred and has been ignored";
        return false;
    }

    m_isMeshing = false;

    return result;
}

bool Problem::meshAction(bool emitMeshed)
{
    clearSolution();

    Agros2D::log()->printMessage(QObject::tr("Mesh Generator"), QObject::tr("Initial mesh generation"));

    // check geometry
    Agros2D::scene()->checkGeometryResult();
    Agros2D::scene()->checkGeometryAssignement();

    QSharedPointer<MeshGenerator> meshGenerator;
    switch (config()->meshType())
    {
    case MeshType_Triangle:
        // case MeshType_Triangle_QuadFineDivision:
        // case MeshType_Triangle_QuadRoughDivision:
        // case MeshType_Triangle_QuadJoin:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorTriangle());
        break;
        // case MeshType_GMSH_Triangle:
    case MeshType_GMSH_Quad:
    case MeshType_GMSH_QuadDelaunay_Experimental:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorGMSH());
        break;
        // case MeshType_NETGEN_Triangle:
        // case MeshType_NETGEN_QuadDominated:
        //     meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorNetgen());
        //     break;
    case MeshType_CUBIT:
        meshGenerator = QSharedPointer<MeshGenerator>(new MeshGeneratorCubitExternal());
        break;
    default:
        QMessageBox::critical(QApplication::activeWindow(), "Mesh generator error", QString("Mesh generator '%1' is not supported.").arg(meshTypeString(config()->meshType())));
        break;
    }

    // add icon to progress
    Agros2D::log()->addIcon(icon("scene-meshgen"),
                            tr("Mesh generator\n%1").arg(meshTypeString(config()->meshType())));

    if (meshGenerator && meshGenerator->mesh())
    {
        // load mesh
        try
        {
            readInitialMeshFromFile(emitMeshed, meshGenerator);
            return true;
        }
        catch (AgrosException& e)
        {
            throw AgrosMeshException(e.what());
        }
    }

    return false;
}

QList<double> Problem::timeStepTimes() const
{
    QList<double> times;

    double time = 0.0;
    for (int ts = 0; ts < m_timeStepLengths.size(); ts++)
    {
        time += m_timeStepLengths[ts];
        times.append(time);
    }

    return times;
}

double Problem::timeStepToTotalTime(int timeStepIndex) const
{
    double time = 0.0;
    for (int ts = 0; ts <= timeStepIndex; ts++)
        time += m_timeStepLengths[ts];

    return time;
}

void Problem::setActualTimeStepLength(double timeStep)
{
    m_timeStepLengths.append(timeStep);
}

void Problem::removeLastTimeStepLength()
{
    m_timeStepLengths.removeLast();
}

void Problem::solveInit()
{
    if (fieldInfos().isEmpty())
    {
        Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("No fields defined"));
        throw AgrosSolverException(tr("No field defined"));
    }

    // check problem settings
    if (Agros2D::problem()->isTransient())
    {
        if (!(Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble() > 0.0))
            throw AgrosSolverException(tr("Total time is zero"));
        if (!(Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble() > 0.0))
            throw AgrosSolverException(tr("Time method tolerance is zero"));
        if (Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() < 0.0)
            throw AgrosSolverException(tr("Initial step size is negative"));
    }

    // open indicator progress
    Indicator::openProgress();

    // control geometry
    Agros2D::scene()->checkGeometryResult();
    Agros2D::scene()->checkGeometryAssignement();

    // nonlinearity
    m_isNonlinear = determineIsNonlinear();

    // save problem
    try
    {
        Agros2D::scene()->writeToFile(tempProblemFileName() + ".a2d", false);
    }
    catch (AgrosException &e)
    {
        Agros2D::log()->printError(tr("Problem"), e.toString());
    }

    // todo: we should not mesh always, but we would need to refine signals to determine when is it neccesary
    // (whether, e.g., parameters of the mesh have been changed)
    if (!mesh(false))
        throw AgrosSolverException(tr("Could not create mesh"));

    // dealii - new concept without Blocks (not necessary)
    m_solverDeal->init();
}

void Problem::doAbortSolve()
{
    m_abort = true;
    Agros2D::log()->printError(QObject::tr("Solver"), QObject::tr("Aborting calculation..."));
}

void Problem::mesh()
{
    if (!isPreparedForAction())
        return;

    m_calculationThread->startCalculation(CalculationThread::CalculationType_Mesh);
}

void Problem::solve()
{    
    if (!isPreparedForAction())
        return;

    m_calculationThread->startCalculation(CalculationThread::CalculationType_Solve);
}

void Problem::solve(bool commandLine)
{
    if (!isPreparedForAction())
        return;

    // clear solution
    clearSolution();

    //    if (numTransientFields() > 1)
    //    {
    //        Agros2D::log()->printError(tr("Solver"), tr("Coupling of more transient fields not possible at the moment."));
    //        return;
    //    }

    if ((m_fieldInfos.size() > 1) && isTransient() && (numAdaptiveFields() >= 1))
    {
        Agros2D::log()->printError(tr("Solver"), tr("Space adaptivity for transient coupled problems not possible at the moment."));
        return;
    }

    if (Agros2D::problem()->fieldInfos().isEmpty())
    {
        Agros2D::log()->printError(tr("Solver"), tr("No fields defined"));
        return;
    }

    if (Agros2D::configComputer()->value(Config::Config_LinearSystemSave).toBool())
        Agros2D::log()->printWarning(tr("Solver"), tr("Matrix and RHS will be saved on the disk and this will slow down the calculation. You may disable it in application settings."));

    try
    {
        m_isSolving = true;

        QTime time;
        time.start();

        solveAction();

        m_lastTimeElapsed = milisecondsToTime(time.elapsed());

        // elapsed time
        Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Elapsed time: %1 s").arg(m_lastTimeElapsed.toString("mm:ss.zzz")));

        // delete temp file
        if (config()->fileName() == tempProblemFileName() + ".a2d")
        {
            QFile::remove(config()->fileName());
            config()->setFileName("");
        }

        m_abort = false;
        m_isSolving = false;

        if (!commandLine)
        {
            emit solved();

            // close indicator progress
            Indicator::closeProgress();
        }
    }
    /*
    catch (Exceptions::NonlinearException &e)
    {
        Solvers::NonlinearConvergenceState convergenceState = e.get_exception_state();
        switch(convergenceState)
        {
        case Solvers::NotConverged:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Newton solver did not converge."));
            break;
        }
        case Solvers::BelowMinDampingCoeff:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Damping coefficient below minimum."));
            break;
        }
        case Solvers::AboveMaxAllowedResidualNorm:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Residual norm exceeded limit."));
            break;
        }
        case Solvers::AboveMaxIterations:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Number of iterations exceeded limit."));
            break;
        }
        case Solvers::Error:
        {
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("An error occurred in Newton solver."));
            break;
        }
        default:
            Agros2D::log()->printError(QObject::tr("Solver (Newton)"), QObject::tr("Newton solver failed from unknown reason."));
        }

        m_isSolving = false;
        return;
    }
    */
    catch (AgrosGeometryException& e)
    {
        Agros2D::log()->printError(QObject::tr("Geometry"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosSolverException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (AgrosException& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (std::exception& e)
    {
        Agros2D::log()->printError(QObject::tr("Solver"), e.what());
        m_isSolving = false;
        return;
    }
    catch (...)
    {
        // todo: dangerous
        // catching all other exceptions. This is not save at all
        m_isSolving = false;
        Agros2D::log()->printError(tr("Solver"), tr("An unknown exception occurred in solver and has been ignored"));
        qDebug() << "Solver: An unknown exception occurred and has been ignored";
        return;
    }
}

void Problem::solveAction()
{
    // clear solution
    clearSolution();

    solveInit();
    assert(isMeshed());

    m_solverDeal->solveProblem();
}

void Problem::readInitialMeshFromFile(bool emitMeshed, QSharedPointer<MeshGenerator> meshGenerator)
{
    if (!meshGenerator)
    {
        // load initial mesh file
        QString fnMesh = QString("%1/%2_initial.msh").arg(cacheProblemDir()).arg("mesh"/*fieldInfo->fieldId()*/);
        std::ifstream ifsMesh(fnMesh.toStdString());
        boost::archive::binary_iarchive sbiMesh(ifsMesh);
        m_initialMesh.load(sbiMesh, 0);

        Agros2D::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from disk"));
    }
    else
    {
        m_initialMesh.copy_triangulation(meshGenerator->triangulation());
        Agros2D::log()->printDebug(tr("Mesh Generator"), tr("Reading initial mesh from memory"));
    }

    int max_num_refinements = 0;
    foreach (FieldInfo *fieldInfo, m_fieldInfos)
    {
        // refine mesh
        // TODO: at the present moment, not possible to refine independently
        max_num_refinements = std::max(max_num_refinements, fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    }

    // this is just a workaround for the problem in deal user data are not preserved on faces after refinement
    m_initialUnrefinedMesh.copy_triangulation(m_initialMesh);

    m_calculationMesh.copy_triangulation(m_initialMesh);
    m_calculationMesh.refine_global(max_num_refinements);
    //propagateBoundaryMarkers();

    // nonlinearity
    m_isNonlinear = determineIsNonlinear();

    if (emitMeshed)
        emit meshed();
}

void Problem::propagateBoundaryMarkers()
{
    dealii::Triangulation<2>::cell_iterator cell_unrefined = m_initialUnrefinedMesh.begin();
    dealii::Triangulation<2>::cell_iterator end_cell_unrefined = m_initialUnrefinedMesh.end();
    dealii::Triangulation<2>::cell_iterator cell_initial = m_initialMesh.begin();
    dealii::Triangulation<2>::cell_iterator cell_calculation = m_calculationMesh.begin();

    for (int idx = 0; cell_unrefined != end_cell_unrefined; ++cell_initial, ++cell_calculation, ++cell_unrefined, ++idx)   // loop over all cells, not just active ones
    {
        for (int f=0; f < dealii::GeometryInfo<2>::faces_per_cell; f++)
        {
            if (cell_unrefined->face(f)->user_index() != 0)
            {
                cell_initial->face(f)->recursively_set_user_index(cell_unrefined->face(f)->user_index());
                cell_calculation->face(f)->recursively_set_user_index(cell_unrefined->face(f)->user_index());
            }
        }
    }
}

void Problem::readSolutionsFromFile()
{
    Agros2D::log()->printMessage(tr("Problem"), tr("Loading DoFHandlers and Solutions from disk"));

    if (QFile::exists(QString("%1/runtime.xml").arg(cacheProblemDir())))
    {
        // load structure
        Agros2D::solutionStore()->loadRunTimeDetails();

        // emit solve
        emit solved();
    }
}

void Problem::synchronizeCouplings()
{
    bool changed = false;

    // add missing
    foreach (FieldInfo* sourceField, m_fieldInfos)
    {
        foreach (FieldInfo* targetField, m_fieldInfos)
        {
            if (sourceField == targetField)
                continue;

            if (couplingList()->isCouplingAvailable(sourceField, targetField))
            {
                QPair<FieldInfo*, FieldInfo*> fieldInfosPair(sourceField, targetField);

                if (!m_couplingInfos.keys().contains(fieldInfosPair))
                {
                    m_couplingInfos[fieldInfosPair] = new CouplingInfo(sourceField, targetField);
                    changed = true;
                }
            }
        }
    }

    // remove extra
    foreach (CouplingInfo *couplingInfo, m_couplingInfos)
    {
        if (!(m_fieldInfos.contains(couplingInfo->sourceField()->fieldId()) &&
              m_fieldInfos.contains(couplingInfo->targetField()->fieldId()) &&
              couplingList()->isCouplingAvailable(couplingInfo->sourceField(), couplingInfo->targetField())))
        {
            QPair<FieldInfo *, FieldInfo *> key = QPair<FieldInfo *, FieldInfo *>(couplingInfo->sourceField(), couplingInfo->targetField());
            m_couplingInfos.remove(key);

            changed = true;
        }
    }

    if (changed)
        emit couplingsChanged();
}

void Problem::doMeshWithGUI()
{
    if (!isPreparedForAction())
        return;

    LogDialog *logDialog = new LogDialog(QApplication::activeWindow(), tr("Mesh"));
    logDialog->show();

    // create mesh
    mesh();
}

void Problem::doSolveWithGUI()
{
    if (!isPreparedForAction())
        return;

    LogDialog *logDialog = new LogDialog(QApplication::activeWindow(), tr("Solver"));
    logDialog->show();

    // solve problem
    solve();
}

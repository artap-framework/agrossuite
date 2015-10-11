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

#include "optilab_solver.h"

#include "util/global.h"
#include "util/checkversion.h"

#include "scenenode.h"
#include "logview.h"
#include "pythonlab/pythonengine_agros.h"

OptilabSolver::OptilabSolver(int &argc, char **argv)
    : AgrosApplication(argc, argv), m_log(NULL), m_enableLog(false)
{    
    createPythonEngine(argc, argv, new PythonEngineAgros());

    checkForNewVersion(true, true);
}

OptilabSolver::~OptilabSolver()
{
    if (m_log)
        delete m_log;
}

// reimplemented from QApplication so we can throw exceptions in slots
bool OptilabSolver::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QCoreApplication::notify(receiver, event);
    }
    catch (std::exception& e)
    {
        qCritical() << "Exception thrown: " << e.what();
        QApplication::exit(1);
    }
    catch (AgrosException e)
    {
        qCritical() << "Exception thrown: " << e.what();
        QApplication::exit(1);
    }
    catch (...)
    {
        qCritical() << "Unknown exception thrown";
        QApplication::exit(1);
    }

    return false;
}

void OptilabSolver::solveProblem()
{
    // log stdout
    if (m_enableLog)
        m_log = new LogStdOut();

    QTime time;
    time.start();

    try
    {
        Agros2D::preprocessor()->readProblemFromFile(m_fileName);

        Agros2D::log()->printMessage(tr("Problem"), tr("Problem '%1' successfuly loaded").arg(m_fileName));

        // solve
        Agros2D::computation()->solve(true);
        // save solution
        // Agros2D::problem()->scene()->writeSolutionToFile(m_fileName);

        Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        // clear all
        // Agros2D::problem()->clearFieldsAndConfig();

        // QApplication::exit(0);
    }
    catch (AgrosException &e)
    {
        Agros2D::log()->printError(tr("Problem"), e.toString());
        // QApplication::exit(-1);
    }
}

void OptilabSolver::runScript()
{
    if (!QFile::exists(m_fileName))
    {
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("Python script '%1' not found").arg(m_fileName));
        QApplication::exit(-1);
    }

    m_command = readFileContent(m_fileName);

    runCommand();
}

void OptilabSolver::runCommand()
{
    // log stdout
    if (m_enableLog)
         m_log = new LogStdOut();

    QTime time;
    time.start();

    // silent mode
    setSilentMode(true);

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    bool successfulRun= currentPythonEngineAgros()->runScript(m_command, m_fileName);

    if (successfulRun)
    {
        Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        Agros2D::preprocessor()->scene()->clear();
        Agros2D::clear();
        QApplication::exit(0);
    }
    else
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("%1\nLine: %2\nStacktrace:\n%3\n").
                                  arg(result.error()).
                                  arg(result.line()).
                                  arg(result.tracebackToString()));

        QApplication::exit(-1);
    }
}

void OptilabSolver::runSuite()
{
    // log stdout
    if (m_enableLog)
         m_log = new LogStdOut();

    // silent mode
    setSilentMode(true);

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    QString testSuite = QString("import test_suite; from test_suite.scenario import run_suite; from test_suite.tests import test; run_suite(test(\"%1\"))").arg(m_suiteName);
    bool successfulRun = currentPythonEngineAgros()->runScript(testSuite);

    if (successfulRun)
    {
        Agros2D::preprocessor()->scene()->clear();
        Agros2D::clear();
        QApplication::exit(0);
    }
    else
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("%1\nLine: %2\nStacktrace:\n%3\n").
                                  arg(result.error()).
                                  arg(result.line()).
                                  arg(result.tracebackToString()));

        QApplication::exit(-1);
    }
}

void OptilabSolver::runTest()
{
    // log stdout
    if (m_enableLog)
         m_log = new LogStdOut();

    // silent mode
    setSilentMode(true);

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    QString testSuite = QString("import test_suite; from test_suite.scenario import run_test; cls = eval(\"%1\"); run_test(cls)").arg(m_testName);
    bool successfulRun = currentPythonEngineAgros()->runScript(testSuite);

    if (successfulRun)
    {
        Agros2D::preprocessor()->scene()->clear();
        Agros2D::clear();
        QApplication::exit(0);
    }
    else
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("%1\nLine: %2\nStacktrace:\n%3\n").
                                  arg(result.error()).
                                  arg(result.line()).
                                  arg(result.tracebackToString()));

        QApplication::exit(-1);
    }
}

void OptilabSolver::printTestSuites()
{
    QStringList list = currentPythonEngineAgros()->testSuiteScenarios();

    foreach (QString test, list)
    {
        std::cout << test.remove("test_").toStdString() << std::endl;
    }

    QApplication::exit(0);
}

void OptilabSolver::stdOut(const QString &str)
{
    std::cout << str.toStdString();
}

void OptilabSolver::stdHtml(const QString &str)
{
    std::cout << str.toStdString();
}
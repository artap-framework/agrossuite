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

#include <Python.h>

#include "agros_solver.h"

#include "util/global.h"
#include "util/checkversion.h"

#include "scenenode.h"
#include "logview.h"
#include "pythonlab/pythonengine_agros.h"

AgrosSolver::AgrosSolver(int &argc, char **argv)
    : AgrosApplication(argc, argv), m_log(NULL), m_enableLog(false), m_status(-1)
{        
}

AgrosSolver::~AgrosSolver()
{
    if (m_log)
    {
        delete m_log;
        m_log = nullptr;
    }
}

// reimplemented from QApplication so we can throw exceptions in slots
bool AgrosSolver::notify(QObject *receiver, QEvent *event)
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

void AgrosSolver::solveProblem()
{
    // log stdout
    if (m_enableLog)
        m_log = new LogStdOut();

    QTime time;
    time.start();

    try
    {
        Agros2D::problem()->readProblemFromArchive(m_fileName);

        Agros2D::log()->printMessage(tr("Problem"), tr("Problem '%1' successfuly loaded").arg(m_fileName));

        // solve
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
        computation->solve();

        // save solution
        Agros2D::problem()->writeProblemToArchive(m_fileName, false);

        Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        // clear all
        Agros2D::problem()->clearFields();

        m_status = 0;
        QApplication::exit(0);
        return;
    }
    catch (AgrosException &e)
    {
        Agros2D::log()->printError(tr("Problem"), e.toString());
        QApplication::exit(-1);
        return;
    }
}

void AgrosSolver::runScript()
{
    if (!QFile::exists(m_fileName))
    {
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("Python script '%1' not found").arg(m_fileName));
        QApplication::exit(-1);
    }

    m_command = readFileContent(m_fileName);

    runCommand();
}

void AgrosSolver::runCommand()
{
    // log stdout
    if (m_enableLog)
         m_log = new LogStdOut();

    QTime time;
    time.start();

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    bool successfulRun= currentPythonEngineAgros()->runScript(m_command, m_fileName);

    if (successfulRun)
    {
        Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

        Agros2D::problem()->scene()->clear();
        Agros2D::clear();

        m_status = 0;
        QApplication::exit(0);
        return;
    }
    else
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("%1\nLine: %2\nStacktrace:\n%3\n").
                                  arg(result.error()).
                                  arg(result.line()).
                                  arg(result.tracebackToString()));

        QApplication::exit(-1);
        return;
    }
}

void AgrosSolver::runTest()
{
    // log stdout
    if (m_enableLog)
         m_log = new LogStdOut();

    connect(currentPythonEngineAgros(), SIGNAL(pythonShowMessage(QString)), this, SLOT(stdOut(QString)));
    connect(currentPythonEngineAgros(), SIGNAL(pythonShowHtml(QString)), this, SLOT(stdHtml(QString)));

    QString testSuite = QString("import test_suite; from test_suite.scenario import run_test; cls = eval(\"%1\"); agros2d_result_report = run_test(cls)").arg(m_testName);
    bool successfulRun = currentPythonEngineAgros()->runScript(testSuite);

    if (successfulRun)
    {
        PyObject *result = PyDict_GetItemString(currentPythonEngine()->globalDict(), "agros2d_result_report");
        if (result)
        {
            Py_INCREF(result);
            for (int i = 0; i < PyList_Size(result); i++)
            {
                PyObject *list = PyList_GetItem(result, i);
                Py_INCREF(list);

                QString tmodule = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(list, 0)));
                QString tcls = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(list, 1)));
                QString ttest = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(list, 2)));
                double telapsedTime = PyFloat_AsDouble(PyList_GetItem(list, 3));
                QString tstatus = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(list, 4)));
                QString terror = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(list, 5)));

                if (tstatus != "OK")
                {
                    QApplication::exit(-1);
                    return;
                }

                Py_XDECREF(list);

            }
            Py_XDECREF(result);
        }

        Agros2D::problem()->clearFieldsAndConfig();
        Agros2D::clear();

        m_status = 0;
        QApplication::exit(0);
        return;
    }
    else
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        Agros2D::log()->printMessage(tr("Scripting Engine"), tr("%1\nLine: %2\nStacktrace:\n%3\n").
                                     arg(result.error()).
                                     arg(result.line()).
                                     arg(result.tracebackToString()));

        QApplication::exit(-1);
        return;
    }
}

void AgrosSolver::stdOut(const QString &str)
{
    std::cout << str.toStdString();
}

void AgrosSolver::stdHtml(const QString &str)
{
    std::cout << str.toStdString();
}

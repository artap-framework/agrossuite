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

#include "../resources_source/python/agros2d.cpp"

#include "pythonengine_agros.h"

#include "logview.h"
#include "solver/plugin_interface.h"
#include "solver/module.h"
#include "solver/problem_result.h"

#ifdef TBB_FOUND
#include <tbb/tbb.h>
tbb::mutex runPythonHeaderMutex;
#endif

void PythonEngineAgros::addCustomExtensions()
{
    PythonEngine::addCustomExtensions();

    // init agros cython extensions
    PyObject *m = PyInit_agros2d();
    PyDict_SetItemString(PyImport_GetModuleDict(), "agros2d", m);
}

void PythonEngineAgros::addCustomFunctions()
{
    addFunctions(readFileContent(datadir() + "/resources/python/functions_agros2d.py"));
}

void PythonEngineAgros::abortScript()
{
    // if (Agros2D::computation()->isMeshing() || Agros2D::computation()->isSolving())
    //     Agros2D::computation()->doAbortSolve();

    PythonEngine::abortScript();
}

void PythonEngineAgros::materialValues(const QString &function, double from, double to,
                                       QVector<double> *keys, QVector<double> *values, int count)
{
    if (function.isEmpty())
        return;

    // function
    bool succesfulRun = runExpression(function);
    if (!succesfulRun)
    {
        ErrorResult result = currentPythonEngineAgros()->parseError();
        qDebug() << "Function: " << result.error();
    }

    // prepare keys
    double step = (to - from) / (count - 1);
    QString keysVector = "[";
    for (int i = 0; i < count; i++)
    {
        double key = from + i * step;
        keys->append(key);

        if (i == 0)
            keysVector += QString("%1").arg(key + EPS_ZERO);
        else if (i == (count - 1))
            keysVector += QString(", %1]").arg(key - EPS_ZERO);
        else
            keysVector += QString(", %1").arg(key);
    }

    // run expression
    runExpression(QString("agros2d_material_values = agros2d_material_eval(%1)").arg(keysVector));

    // extract values
    PyObject *result = PyDict_GetItemString(globalDict(), "agros2d_material_values");
    if (result)
    {
        Py_INCREF(result);
        for (int i = 0; i < count; i++)
            values->append(PyFloat_AsDouble(PyList_GetItem(result, i)));
        Py_XDECREF(result);
    }

    // remove variables
    runExpression("del agros2d_material; del agros2d_material_values");

    // error during execution
    if (keys->size() != values->size())
    {
        keys->clear();
        values->clear();
    }
}

QStringList PythonEngineAgros::testSuiteScenarios()
{
    QStringList list;

    // run expression
    bool successfulRun = currentPythonEngine()->runExpression(QString("from test_suite.scenario import find_all_scenarios; agros2d_scenarios = find_all_scenarios()"));

    if (successfulRun)
    {
        // extract values
        PyObject *result = PyDict_GetItemString(currentPythonEngine()->globalDict(), "agros2d_scenarios");
        if (result)
        {
            Py_INCREF(result);
            for (int i = 0; i < PyList_Size(result); i++)
            {
                QString testName = QString::fromWCharArray(PyUnicode_AsUnicode(PyList_GetItem(result, i)));

                list.append(testName);
            }
            Py_XDECREF(result);
        }
    }
    else
    {
        // parse error
        ErrorResult result = currentPythonEngine()->parseError();
        qDebug() << result.error();
        qDebug() << result.tracebackToString();
    }

    // remove variables
    currentPythonEngine()->runExpression("del agros2d_scenarios");

    return list;
}

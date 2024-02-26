// This plugin is part of Agros2D.
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

#include "global.h"

#include "util/util.h"
#include "util/conf.h"
#include "util/system_utils.h"
#include "logview.h"

#include "solver/problem.h"
#include "solver/plugin_interface.h"
#include "solver/plugin_solver_interface.h"

#include "optilab/study.h"


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#ifdef Q_WS_WIN
#include "Windows.h"
#pragma comment(lib, "psapi.lib")
#endif

bool isPluginDir(const QString &path)
{
    QDir dir(path);

    QStringList filters;
    filters << "libagros_plugin_*.so" << "agros_plugin_*.dll" << "libsolver_plugin_*.so" << "solver_plugin_*.dll";
    QStringList list = dir.entryList(filters);

    return (list.size() > 0);
}

QStringList pluginList(const QString &data)
{
    QString pluginPath = "";

    if (isPluginDir(data + "/libs/"))
        pluginPath = data + "/libs/";
    else if (QCoreApplication::instance() && isPluginDir(QCoreApplication::applicationDirPath() + "/../lib/"))
        pluginPath = QCoreApplication::applicationDirPath() + "/../lib/";

    if (pluginPath.isEmpty())
    {
        throw AgrosPluginException(QObject::tr("Could not find plugins in directory."));
        assert(0);
    }

    QDir dir(pluginPath);

    QStringList filters;
    filters << "libagros_plugin_*.so" << "agros_plugin_*.dll";

    QStringList list;
    foreach (QString entry, dir.entryList(filters))
        list.append(QString("%1/%2").arg(pluginPath).arg(entry));

    return list;
}

QStringList solverList(const QString &data)
{
    QString pluginPath = "";

    if (isPluginDir(data + "/libs/"))
        pluginPath = data + "/libs/";
    else if (QCoreApplication::instance() && isPluginDir(QCoreApplication::applicationDirPath() + "/../lib/"))
        pluginPath = QCoreApplication::applicationDirPath() + "/../lib/";

    if (pluginPath.isEmpty())
    {
        throw AgrosPluginException(QObject::tr("Could not load find plugins in directory."));
        assert(0);
    }

    QDir dir(pluginPath);

    QStringList filters;
    filters << "solver_plugin_*.dll" << "libsolver_plugin_*.so";

    QStringList list;
    foreach (QString entry, dir.entryList(filters))
        list.append(QString("%1/%2").arg(pluginPath).arg(entry));

    return list;
}

void initSingleton()
{
    // For crashes, SIGSEV should be enough.
    setlocale(LC_NUMERIC, "C");

    // char *argv[] = {(char *) QString("%1/agros_python").arg(getenv("PWD")).toStdString().c_str(), NULL};
    // int argc = sizeof(argv) / sizeof(char*) - 1;

    QCoreApplication::setApplicationVersion(versionString());
    QCoreApplication::setOrganizationName("agros");
    QCoreApplication::setOrganizationDomain("agros");
    QCoreApplication::setApplicationName("Agros Suite");
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);

    // force number format
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    // init singleton
    Agros::createSingleton(QSharedPointer<Log>(new LogStdOut()));
}

void clearAgros2DCache()
{    
    QFileInfoList listCache = QFileInfo(cacheProblemDir()).absoluteDir().entryInfoList();
    QFileInfoList listTemp = QFileInfo(tempProblemDir()).absoluteDir().entryInfoList();

    QFileInfoList list;
    list << listCache << listTemp;
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == ".." || fileInfo.fileName() == QString::number(QCoreApplication::applicationPid()))
            continue;

        if (fileInfo.isDir())
        {
            // process doesn't exists
            if (!SystemUtils::isProcessRunning(fileInfo.fileName().toInt()))
                removeDirectory(fileInfo.absoluteFilePath());
        }
    }

}

QString findDataDir()
{
    // data dir
    // windows
#ifdef Q_WS_WIN
    // local installation
    // solver
    if (QCoreApplication::instance() && QFile::exists(QCoreApplication::applicationDirPath() + "/resources/templates/empty.tpl"))
        return(QCoreApplication::applicationDirPath());
#endif

    // linux
#ifdef Q_WS_X11
    // solver DEK
    if (QFile::exists(QString::fromLatin1(getenv("PWD")) + "/agros/resources/templates/empty.tpl"))
        return(QString::fromLatin1(getenv("PWD")) + "/agros");

    // gui and solver
    else if (QCoreApplication::instance() && QFile::exists(QCoreApplication::applicationDirPath() + "/resources/templates/empty.tpl"))
        return(QCoreApplication::applicationDirPath());

    // system installation
    else if (QCoreApplication::instance() && QFile::exists(QCoreApplication::applicationDirPath() + "/../share/agros/resources/templates/empty.tpl"))
        return(QCoreApplication::applicationDirPath() + "/../share/agros");

    // local installation
    // python
    else
    {
        if (QFile::exists(QString::fromLatin1(getenv("PWD")) + "/../../resources/templates/empty.tpl"))
            Agros::setDataDir(QString::fromLatin1(getenv("PWD")) + "/../..");
        else if (QFile::exists(QString::fromLatin1(getenv("PWD")) + "/resources/templates/empty.tpl"))
            Agros::setDataDir(QString::fromLatin1(getenv("PWD")) + "/");
        else
        {
            for (int i = 9; i > 5; i--)
            {
                if (QFile::exists(QDir::homePath() + QString::fromLatin1("/.local/lib/python3.%1/site-packages/agrossuite/resources/templates/empty.tpl").arg(i)))
                    return(QDir::homePath() + QString::fromLatin1("/.local/lib/python3.%1/site-packages/agrossuite/").arg(i));
                else if (QFile::exists(QString::fromLatin1("/usr/local/lib/python3.%1/site-packages/agrossuite/resources/templates/empty.tpl").arg(i)))
                    return(QString::fromLatin1("/usr/local/lib/python3.%1/site-packages/agrossuite/").arg(i));
                else if (QFile::exists(QString::fromLatin1("/usr/lib/python3.%1/site-packages/agrossuite/resources/templates/empty.tpl").arg(i)))
                    return(QString::fromLatin1("/usr/lib/python3.%1/site-packages/agrossuite/").arg(i));
            }
        }
    }
#endif

    return "";
}

static Agros *m_singleton;

Agros::Agros(QSharedPointer<Log> log) : m_log(log)
{
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log)";
    clearAgros2DCache();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - clearAgros2DCache";

    // preprocessor
    m_problem = new Problem();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - m_problem = new Problem();";

    initLists();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - initLists";

    m_configComputer = new Config();
    // qInfo() << "Agros::Agros(QSharedPointer<Log> log) - m_configComputer = new Config();";
}

void Agros::readPlugins()
{
    // set default datadir
    if (m_singleton->dataDir().isEmpty())
        m_singleton->setDataDir(findDataDir());

    // plugins
    // read plugins
#ifdef AGROS_BUILD_PLUGIN_STATIC
    foreach (QObject *obj, QPluginLoader::staticInstances())
    {
        PluginInterface *plugin = qobject_cast<PluginInterface *>(obj);
        m_plugins[plugin->fieldId()] = plugin;
    }
#else
    // plugins
    foreach (QString pluginPath, pluginList(m_singleton->dataDir()))
    {
        // load new plugin
        QPluginLoader *loader = new QPluginLoader(pluginPath);

        if (!loader)
        {
            throw AgrosPluginException(QObject::tr("Could not find 'agros2d_plugin_%1'").arg(pluginPath));
        }

        if (!loader->load())
        {
            QString error = loader->errorString();
            delete loader;
            throw AgrosPluginException(QObject::tr("Could not load 'agros2d_plugin_%1' (%2)").arg(pluginPath).arg(error));
        }

        assert(loader->instance());
        PluginInterface *plugin = qobject_cast<PluginInterface *>(loader->instance());
        m_singleton->m_plugins[plugin->fieldId()] = plugin;

        delete loader;
    }

    // solvers
    foreach (QString pluginPath, solverList(m_singleton->dataDir()))
    {
        // load new plugin
        QPluginLoader *loader = new QPluginLoader(pluginPath);

        if (!loader)
        {
            throw AgrosPluginException(QObject::tr("Could not find 'solver_plugin_%1'").arg(pluginPath));
        }

        if (!loader->load())
        {
            QString error = loader->errorString();
            qInfo() << error;
            delete loader;
            throw AgrosPluginException(QObject::tr("Could not load 'solver_plugin_%1' (%2)").arg(pluginPath).arg(error));
        }

        assert(loader->instance());
        PluginSolverInterface *plugin = qobject_cast<PluginSolverInterface *>(loader->instance());
        m_singleton->m_solvers[plugin->name()] = plugin;

        delete loader;
    }
#endif
}

Agros::~Agros()
{
    delete m_singleton;
}

void Agros::clear()
{    
    delete m_singleton->m_problem;
    m_singleton->m_computations.clear();

    delete m_singleton->m_configComputer;

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());
}

void Agros::addComputation(const QString &problemDir, QSharedPointer<Computation> comp)
{
    Agros::singleton()->m_computations[problemDir] = comp;
}

void Agros::clearComputations()
{
    foreach (QSharedPointer<Computation> computation, Agros::singleton()->m_computations)
    {
        // clear solutions
        computation->clearFieldsAndConfig();
        // remove computation from studies
        Agros::singleton()->problem()->studies()->removeComputation(computation);
        // remove from list
        Agros::singleton()->m_computations.remove(computation->problemDir());
    }

    Agros::singleton()->m_computations.clear();
}

void Agros::createSingleton(QSharedPointer<Log> log)
{
    m_singleton = new Agros(log);
}

Agros* Agros::singleton()
{
    qInfo() << "Agros::singleton()" << m_singleton;
    return m_singleton;
}

PluginInterface *Agros::loadPlugin(const QString &pluginName)
{
    if (Agros::singleton()->m_plugins.contains(pluginName))
        return Agros::singleton()->m_plugins[pluginName];

    assert(0);
    return nullptr;
}

PluginSolverInterface *Agros::loadSolver(const QString &solverName)
{
    if (Agros::singleton()->m_solvers.contains(solverName))
        return Agros::singleton()->m_solvers[solverName];

    assert(0);
    return nullptr;
}

void Agros::setDataDir(const QString &dir)
{
    m_singleton->m_dataDir = dir;
}

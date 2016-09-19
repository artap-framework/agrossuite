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

#include "util/constants.h"

#include "util/util.h"
#include "logview.h"
#include "scene.h"

#include "solver/module.h"

#include "solver/problem.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "optilab/study.h"

#include "util/system_utils.h"

#include "boost/archive/archive_exception.hpp"


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

static QSharedPointer<Agros2D> m_singleton;

Agros2D::Agros2D()
{
    clearAgros2DCache();

    // preprocessor
    m_problem = new Problem();

    initLists();

    m_configComputer = new Config();

    // log
    m_log = new Log();
}

void Agros2D::clear()
{    
    delete m_singleton.data()->m_problem;
    m_singleton.data()->m_computations.clear();

    delete m_singleton.data()->m_configComputer;   
    delete m_singleton.data()->m_log;

    // remove temp and cache plugins
    removeDirectory(cacheProblemDir());
    removeDirectory(tempProblemDir());
}

void Agros2D::addComputation(const QString &problemDir, QSharedPointer<Computation> comp)
{
    Agros2D::singleton()->m_computations[problemDir] = comp;
}

void Agros2D::clearComputations()
{
    foreach (QSharedPointer<Computation> computation, Agros2D::singleton()->m_computations)
    {
        // clear solutions
        computation->clearFieldsAndConfig();
        // remove computation from studies
        Agros2D::singleton()->problem()->studies()->removeComputation(computation);
        // remove from list
        Agros2D::singleton()->m_computations.remove(computation->problemDir());
    }

    Agros2D::singleton()->m_computations.clear();
}

void Agros2D::createSingleton()
{
    m_singleton = QSharedPointer<Agros2D>(new Agros2D());    
}

Agros2D *Agros2D::singleton()
{
    return m_singleton.data();
}

static QMap<QString, PluginInterface *> plugins;
PluginInterface *Agros2D::loadPlugin(const QString &pluginName)
{
    if (plugins.contains(pluginName))
        return plugins[pluginName];

    // load new plugin
    QPluginLoader *loader = NULL;

#ifdef Q_WS_X11
    if (QFile::exists(QString("%1/libs/libagros2d_plugin_%2.so").arg(datadir()).arg(pluginName)))
        loader = new QPluginLoader(QString("%1/libs/libagros2d_plugin_%2.so").arg(datadir()).arg(pluginName));
    else if (QFile::exists(QString(QCoreApplication::applicationDirPath() + "/../lib/libagros2d_plugin_%1.so").arg(pluginName)))
        loader = new QPluginLoader(QString(QCoreApplication::applicationDirPath() + "/../lib/libagros2d_plugin_%1.so").arg(pluginName));
#endif

#ifdef Q_WS_WIN
    if (QFile::exists(QString("%1/libs/agros2d_plugin_%2.dll").arg(datadir()).arg(pluginName)))
        loader = new QPluginLoader(QString("%1/libs/agros2d_plugin_%2.dll").arg(datadir()).arg(pluginName));
#endif

    if (!loader)
    {
        throw AgrosPluginException(QObject::tr("Could not find 'agros2d_plugin_%1'").arg(pluginName));
    }

    if (!loader->load())
    {
        QString error = loader->errorString();
        delete loader;
        throw AgrosPluginException(QObject::tr("Could not load 'agros2d_plugin_%1' (%2)").arg(pluginName).arg(error));
    }

    assert(loader->instance());
    PluginInterface *plugin = qobject_cast<PluginInterface *>(loader->instance());
    plugins[pluginName] = plugin;

    delete loader;

    return plugin;
}


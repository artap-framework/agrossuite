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

#ifndef GLOBAL_H
#define GLOBAL_H

#include "util/util.h"
#include "util/enums.h"

class Problem;
class Computation;
class Config;
class SolutionStore;
class Log;
class PluginInterface;
class PluginSolverInterface;


AGROS_LIBRARY_API void initSingleton();

class AGROS_LIBRARY_API Agros
{
public:
    Agros(const Agros &);
    Agros & operator = (const Agros &);
    Agros(QSharedPointer<Log> log);
    ~Agros();

    static void createSingleton(QSharedPointer<Log> log);
    static Agros* singleton();

    static inline Config *configComputer() { return Agros::singleton()->m_configComputer; }

    static inline Problem *problem() { return Agros::singleton()->m_problem; }

    static inline QMap<QString, QSharedPointer<Computation> > computations() { return Agros::singleton()->m_computations; }
    static void addComputation(const QString &problemDir, QSharedPointer<Computation> comp);
    static void clearComputations();

    static inline Log *log() { return Agros::singleton()->m_log.data(); }

    static inline QMap<QString, PluginInterface *> plugins()  { return Agros::singleton()->m_plugins; }
    static PluginInterface *loadPlugin(const QString &pluginName);

    static inline QMap<QString, PluginSolverInterface *> solvers()  { return Agros::singleton()->m_solvers; }
    static PluginSolverInterface *loadSolver(const QString &solverName);

    static void setDataDir(const QString &dir);
    static inline QString dataDir() { return Agros::singleton()->m_dataDir; }

    static void clear();
    static void readPlugins();

private:    
    // datadir
    QString m_dataDir;
    // computer config
    Config *m_configComputer;
    // problem
    Problem *m_problem;
    // computations
    QMap<QString, QSharedPointer<Computation> > m_computations;
    // log
    QSharedPointer<Log> m_log;
    // plugins
    QMap<QString, PluginInterface *> m_plugins;
    // solvers
    QMap<QString, PluginSolverInterface *> m_solvers;
};

// create script from model
QString createPythonFromModel();

#endif /* GLOBAL_H */

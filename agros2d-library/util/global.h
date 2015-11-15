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

#include "util.h"
#include "util/enums.h"

class Problem;
class Computation;
class Config;
class SolutionStore;
class Log;
class PluginInterface;
class ScriptEngineRemote;
class MemoryMonitor;

class AGROS_LIBRARY_API AgrosApplication : public QApplication
{
public:
    AgrosApplication(int& argc, char ** argv);
    ~AgrosApplication();

    virtual bool notify(QObject *receiver, QEvent *event);

    void runRemoteServer();

private:
    ScriptEngineRemote *m_scriptEngineRemote;
};

class AGROS_LIBRARY_API Agros2D : public QObject
{
    Q_OBJECT

public:
    Agros2D(const Agros2D &);
    Agros2D & operator = (const Agros2D &);
    Agros2D();

    static void createSingleton();
    static Agros2D* singleton();

    static inline Config *configComputer() { return Agros2D::singleton()->m_configComputer; }

    static inline Problem *problem() { return Agros2D::singleton()->m_problem; }

    static void setCurrentComputation(const QString &problemDir);
    static inline QSharedPointer<Computation> computation()
    {
        qWarning() << "Agros2D::computation() deprecated - method will be removed!";
        return Agros2D::singleton()->m_computation;
    }
    static inline QMap<QString, QSharedPointer<Computation> > computations() { return Agros2D::singleton()->m_computations; }
    static void addComputation(const QString &problemDir, QSharedPointer<Computation> comp);
    static void removeComputation(const QString &problemDir);
    static void clearComputations();

    static inline Log *log() { return Agros2D::singleton()->m_log; }
    static inline MemoryMonitor *memoryMonitor() { return Agros2D::singleton()->m_memoryMonitor; }

    static PluginInterface *loadPlugin(const QString &pluginName);

    static void clear();

signals:
    void connectComputation(QSharedPointer<Computation>);

private:    
    // computer config
    Config *m_configComputer;
    // problem
    Problem *m_problem;
    // computation
    QSharedPointer<Computation> m_computation;
    QMap<QString, QSharedPointer<Computation> > m_computations;
    // log and memory monitor
    Log *m_log;
    MemoryMonitor *m_memoryMonitor;
};

#endif /* GLOBAL_H */

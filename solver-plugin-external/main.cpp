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

#include <iostream>

#include "../agros-library/util/sparse_io.h"
#include "../agros-library/solver/solver_io.h"
#include "../agros-library/solver/plugin_solver_interface.h"

QString tempProblemDir()
{
#ifdef Q_WS_WIN
    static QString str = QString("%1/agros/%2").
            arg(QDir::temp().absolutePath()).
            arg(QString::number(QCoreApplication::applicationPid()));
#else
    static QString str = QString("%1/agros-%2/%3").
            arg(QDir::temp().absolutePath()).
            arg(getenv("USER")).
            arg(QString::number(QCoreApplication::applicationPid()));
#endif

    QDir dir(str);
    if (!dir.exists() && !str.isEmpty())
        dir.mkpath(str);

    return str;
}

class ExternalSolverInterface : public QObject, public PluginSolverInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginSolverInterface)
    Q_PLUGIN_METADATA(IID PluginSolverInterface_IID)

public:
    ExternalSolverInterface() {}
    virtual ~ExternalSolverInterface() override {}

    virtual QString name() const override { return QString("External"); }

    virtual void solve(dealii::SparseMatrix<double> &system,
                       dealii::Vector<double> &rhs,
                       dealii::Vector<double> &sln) override
    {
        qInfo() << "Working directory: " << m_workingDirectory;
        qInfo() << "Solver executable: " << m_solverExecutable;
        writeMatioMatrix(system, QString("%1/system.mat").arg(m_workingDirectory).toStdString().c_str(), "system");
        writeMatioVector(rhs, QString("%1/rhs.mat").arg(m_workingDirectory).toStdString().c_str(), "rhs");

        // exec run.sh
        auto command = QString("%1/run.sh").arg(m_workingDirectory);
        qInfo() << "Command: " << command;
        QStringList args;
        args << QString("-wd \"%1\"").arg(m_workingDirectory);
        QString parameters = "{ ";
        foreach (auto parameter, m_parameters.keys())
            parameters += QString("'%1': %2, ").arg(parameter).arg(QString::number(m_parameters[parameter]));
        parameters += " }";
        args << QString("-p \"%1\"").arg(parameters);

        QProcess run;
        run.setWorkingDirectory(m_workingDirectory);
        run.start(command, args);
        if (!run.waitForStarted()) {
            qCritical() << "Could not run command: " << command;
        }
        else
        {
            run.waitForReadyRead();
            qInfo() << run.readAllStandardOutput();
            qCritical() << run.readAllStandardError();
            run.waitForFinished();

            // read solution
            readMatioVector(sln, QString("%1/sln.mat").arg(m_workingDirectory).toStdString().c_str(), "sln");
        }

        // read solution
        // readMatioVector(sln, QString("%1/sln.mat").arg(dir).toStdString().c_str(), "sln");
    }
};

#include "main.moc"

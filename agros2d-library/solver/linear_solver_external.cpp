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

// deal.ii
#include <streambuf>
#include <sstream>

#include "linear_solver_external.h"

#include "util/constants.h"

#include "field.h"
#include "problem.h"
#include "solver/problem_config.h"
#include "solver/solver_utils.h"
#include "scene.h"
#include "sceneedge.h"
#include "scenelabel.h"
#include "scenemarker.h"
#include "scenemarkerdialog.h"
#include "solutionstore.h"
#include "plugin_interface.h"
#include "logview.h"
#include "bdf2.h"
#include "plugin_interface.h"
#include "weak_form.h"

#include "pythonlab/pythonengine.h"

AgrosExternalSolver::AgrosExternalSolver(const dealii::SparseMatrix<double> *system_matrix,
                                         const dealii::Vector<double> *system_rhs)
    : m_system_matrix(system_matrix), m_system_rhs(system_rhs), m_initial_guess(NULL)
{
}

void AgrosExternalSolver::solve(const dealii::Vector<double> *initial_guess)
{
    m_initial_guess = initial_guess;

    QDateTime datetime(QDateTime::currentDateTime());
    QString tm = QString("%1").arg(datetime.toString("yyyy-MM-dd-hh-mm-ss-zzz"));

    QString fileMatrix = QString("%1/solver-%2.matrix").arg(cacheProblemDir()).arg(tm);
    QString fileMatrixPattern = QString("%1/solver-%2.matrix_pattern").arg(cacheProblemDir()).arg(tm);
    QString fileRHS = QString("%1/solver-%2.rhs").arg(cacheProblemDir()).arg(tm);
    QString fileInitial = QString("%1/solver-%2.initial").arg(cacheProblemDir()).arg(tm);
    QString fileSln = QString("%1/solver-%2.sln").arg(cacheProblemDir()).arg(tm);

    // write matrix and rhs to disk
    std::ofstream writeMatrix(fileMatrix.toStdString());
    m_system_matrix->block_write(writeMatrix);
    writeMatrix.close();

    std::ofstream writeMatrixSparsityPattern(fileMatrixPattern.toStdString());
    m_system_matrix->get_sparsity_pattern().block_write(writeMatrixSparsityPattern);
    writeMatrixSparsityPattern.close();

    std::ofstream writeRHS(fileRHS.toStdString());
    m_system_rhs->block_write(writeRHS);
    writeRHS.close();

    // write initial guess to disk
    if (m_initial_guess)
    {
        std::ofstream writeInitial(fileInitial.toStdString());
        m_initial_guess->block_write(writeInitial);
        writeInitial.close();
    }

    // fill command template   
    QString command = QString("%1 \"%2/libs/%3\" -m \"%4\" -p \"%5\" -r \"%6\" -s \"%7\" %8").
            arg(m_commandEnvironment).
            arg(QApplication::applicationDirPath()).
            arg(m_command).
            arg(fileMatrix).
            arg(fileMatrixPattern).
            arg(fileRHS).
            arg(fileSln).
            arg(m_commandParameters);

    // exec
    m_process = new QProcess();
    m_process->setStandardOutputFile(tempProblemDir() + "/solver.out");
    m_process->setStandardErrorFile(tempProblemDir() + "/solver.err");
    connect(m_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
    connect(m_process, SIGNAL(finished(int)), this, SLOT(processFinished(int)));

    m_process->start(command);

    // execute an event loop to process the request (nearly-synchronous)
    QEventLoop eventLoop;
    QObject::connect(m_process, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
    QObject::connect(m_process, SIGNAL(error(QProcess::ProcessError)), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    if (QFile::exists(fileSln))
    {
        std::ifstream readSln(fileSln.toStdString());
        m_solution.block_read(readSln);
        readSln.close();
    }

    if (m_initial_guess)
        QFile::remove(fileInitial);
    QFile::remove(fileMatrix);
    QFile::remove(fileMatrixPattern);
    // QFile::remove(fileRHS);
    QFile::remove(fileSln);

    QFile::remove(tempProblemDir() + "/solver.out");
    QFile::remove(tempProblemDir() + "/solver.err");
}

void AgrosExternalSolver::solve()
{
    solve(nullptr);
}

void AgrosExternalSolver::processError(QProcess::ProcessError error)
{
    Agros2D::log()->printError(tr("Solver"), tr("Could not start external solver"));
    m_process->kill();
}

void AgrosExternalSolver::processFinished(int exitCode)
{
    QString solverOutputMessage = readFileContent(tempProblemDir() + "/solver.out");
    if (!solverOutputMessage.isEmpty())
    {
        solverOutputMessage.insert(0, "\n");
        solverOutputMessage.append("\n");
        Agros2D::log()->printWarning(tr("External solver"), solverOutputMessage);
    }

    if (exitCode == 0)
    {
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        Agros2D::log()->printError(tr("External solver"), errorMessage);
    }
}

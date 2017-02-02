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

#include "logview.h"

#include "util/global.h"
#include "util/constants.h"

#include "scene.h"
#include "solver/problem_config.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/solver.h"
#include "solver/solutionstore.h"

#include "../3rdparty/spdlog/spdlog.h"

// *******************************************************************************************************


/* void Log::printMessage(const QString &module, const QString &message)
{
    m_console->info((QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString());
    m_handler(module, message);
} */



LogStdOut::LogStdOut()
{
    /* m_console = spdlog::stdout_color_mt("console");
    qRegisterMetaType<QVector<double> >("QVector<double>");
    qRegisterMetaType<SolverAgros::Phase>("SolverAgros::Phase");
    size_t q_size = 4096; //queue size must be power of 2
    spdlog::set_async_mode(q_size);
    auto async_file = spdlog::daily_logger_st("async_file_logger", "async_log.txt");
    async_file->info("Pokus"); */
}

void LogStdOut::printHeading(const QString &message)
{
    std::cout << (QString("%1").arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printMessage(const QString &module, const QString &message)
{
    std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printError(const QString &module, const QString &message)
{
    std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printWarning(const QString &module, const QString &message)
{
    std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printDebug(const QString &module, const QString &message)
{
    std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

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
#include "util/conf.h"

#include "scene.h"
#include "solver/problem_config.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/solver.h"
#include "solver/solutionstore.h"


LogStdOut::LogStdOut()
{
}

void LogStdOut::printHeading(const QString &message)
{
    if (Agros::configComputer()->value(Config::Config_LogStdOut).toBool())
        std::cout << (QString("%1").arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printMessage(const QString &module, const QString &message)
{
    if (Agros::configComputer()->value(Config::Config_LogStdOut).toBool())
        std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printError(const QString &module, const QString &message)
{
    std::cerr << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printWarning(const QString &module, const QString &message)
{
    if (Agros::configComputer()->value(Config::Config_LogStdOut).toBool())
        std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

void LogStdOut::printDebug(const QString &module, const QString &message)
{
    if (Agros::configComputer()->value(Config::Config_LogStdOut).toBool())
        std::cout << (QString("%1: %2").arg(module).arg(message).toLatin1()).toStdString() << std::endl;
}

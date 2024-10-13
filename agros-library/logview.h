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

#ifndef TOOLTIPVIEW_H
#define TOOLTIPVIEW_H

#include <functional>

#include "util/util.h"
#include "solver/solver.h"
#include "solver/solver_utils.h"

// #include "../3rdparty/spdlog/spdlog.h"

class FieldInfo;
class SolverAgros;

class AGROS_LIBRARY_API Log
{

public:
    Log() {}
    virtual ~Log() {}

    virtual void printHeading(const QString &message) = 0;
    virtual void printMessage(const QString &module, const QString &message) = 0;
    virtual void printError(const QString &module, const QString &message) = 0;
    virtual void printWarning(const QString &module, const QString &message) = 0;
    virtual void printDebug(const QString &module, const QString &message) = 0;

    virtual inline void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions) = 0;
    virtual inline void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) = 0;
    virtual inline void updateTransientChartInfo(double actualTime) = 0;

    virtual inline void appendImage(const QString &fileName) = 0;
    virtual inline void appendHtml(const QString &html) = 0;
};

class AGROS_LIBRARY_API LogStdOut : public Log
{

public:
    LogStdOut();

    virtual void printHeading(const QString &message) override;
    virtual void printMessage(const QString &module, const QString &message) override;
    virtual void printError(const QString &module, const QString &message) override;
    virtual void printWarning(const QString &module, const QString &message) override;
    virtual void printDebug(const QString &module, const QString &message) override;

    virtual inline void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions) override {}
    virtual inline void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) override {}
    virtual inline void updateTransientChartInfo(double actualTime) override {}

    virtual inline void appendImage(const QString &fileName) override {}
    virtual inline void appendHtml(const QString &html) override {}

private:
      // std::shared_ptr<spdlog::logger>  m_console;
      void (*m_handler) (const QString &module, const QString &message);
};

#endif // TOOLTIPVIEW_H

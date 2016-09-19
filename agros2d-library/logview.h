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

#include "util/util.h"
#include "solver/solver.h"
#include "solver/solver_utils.h"

class FieldInfo;
class SolverAgros;

class AGROS_LIBRARY_API Log: public QObject
{
    Q_OBJECT
public:
    Log();

    inline void printHeading(const QString &message) { emit headingMsg(message); }
    inline void printMessage(const QString &module, const QString &message) { emit messageMsg(module, message); }
    inline void printError(const QString &module, const QString &message) { emit errorMsg(module, message); }
    inline void printWarning(const QString &module, const QString &message) { emit warningMsg(module, message); }
    inline void printDebug(const QString &module, const QString &message) { emit debugMsg(module, message); }

    inline void updateNonlinearChartInfo(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions) { emit updateNonlinearChart(phase, steps, relativeChangeOfSolutions); }
    inline void updateAdaptivityChartInfo(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep) { emit updateAdaptivityChart(fieldInfo, timeStep, adaptivityStep); }
    inline void updateTransientChartInfo(double actualTime) { emit updateTransientChart(actualTime); }

    inline void appendImage(const QString &fileName) { emit appendImg(fileName); }
    inline void appendHtml(const QString &html) { emit appendHtm(html); }

    inline void addIcon(const QIcon &icn, const QString &label) { emit addIconImg(icn, label); }

signals:
    void headingMsg(const QString &message);
    void messageMsg(const QString &module, const QString &message);
    void errorMsg(const QString &module, const QString &message);
    void warningMsg(const QString &module, const QString &message);
    void debugMsg(const QString &module, const QString &message);

    void updateNonlinearChart(SolverAgros::Phase phase, const QVector<double> steps, const QVector<double> relativeChangeOfSolutions);
    void updateAdaptivityChart(const FieldInfo *fieldInfo, int timeStep, int adaptivityStep);
    void updateTransientChart(double actualTime);

    void appendImg(const QString &fileName);
    void appendHtm(const QString &html);

    void addIconImg(const QIcon &icn, const QString &label);
};

class AGROS_LIBRARY_API LogStdOut : public QObject
{
    Q_OBJECT
public:
    LogStdOut();

private slots:
    void printHeading(const QString &message);
    void printMessage(const QString &module, const QString &message);
    void printError(const QString &module, const QString &message);
    void printWarning(const QString &module, const QString &message);
    void printDebug(const QString &module, const QString &message);
};

#endif // TOOLTIPVIEW_H

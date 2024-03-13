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

#ifndef PYAGROS_H
#define PYAGROS_H

// Windows DLL export/import definitions
#ifdef Q_WS_WIN
    // windows
    // DLL build
#ifdef AGROS_LIBRARY_DLL
#define AGROS_LIBRARY_API __declspec(dllexport)
// DLL usage
#else
#define AGROS_LIBRARY_API 
#endif
#else
//// linux
#define AGROS_LIBRARY_API
#endif

#include "util/util.h"
#include "util/global.h"
#include "util/conf.h"

class Solution;

// ************************************************************************************

void openFile(const std::string &file, bool openWithSolution);
void saveFile(const std::string &file, bool saveWithSolution);
std::string getScriptFromModel();

inline std::string tempDir() { return tempProblemDir().toStdString(); }
inline std::string cacheDir() { return cacheProblemDir().toStdString(); }

// functions
std::string pyVersion();

std::string dataDir();
void setDataDir(std::string str);
void readPlugins();

struct PyOptions
{
    // cache size
    inline int getCacheSize() const { return Agros::configComputer()->value(Config::Config_CacheSize).toInt(); }
    void setCacheSize(int size);

    // log
    inline int getStdOutLog() const { return Agros::configComputer()->value(Config::Config_LogStdOut).toBool(); }
    void setStdOutLog(bool enabled) { Agros::configComputer()->setValue(Config::Config_LogStdOut, enabled); }

    // save matrix and rhs
    inline bool getSaveMatrixRHS() const { return Agros::configComputer()->value(Config::Config_LinearSystemSave).toBool(); }
    inline void setSaveMatrixRHS(bool save) { Agros::configComputer()->setValue(Config::Config_LinearSystemSave, save); }

    // save system matrix and rhs (mass, stiffness, ...)
    inline bool getSaveSystem() const { return Agros::configComputer()->value(Config::Config_MatrixSystemSave).toBool(); }
    inline void setSaveSystem(bool save) { Agros::configComputer()->setValue(Config::Config_MatrixSystemSave, save); }

    inline std::string getDumpFormat() const { return dumpFormatToStringKey((MatrixExportFormat) Agros::configComputer()->value(Config::Config_LinearSystemFormat).toInt()).toStdString(); }
    void setDumpFormat(std::string format);
};

#endif // PYTHONENGINEAGROS_H

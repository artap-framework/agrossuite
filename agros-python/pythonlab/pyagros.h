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
#ifdef _MSC_VER
    #define AGROS_LIBRARY_API __declspec(dllexport)
#else
#define AGROS_LIBRARY_API
#endif

#include "util/util.h"
#include "util/global.h"
#include "util/conf.h"

class AGROS_LIBRARY_API Solution;

// ************************************************************************************

void AGROS_LIBRARY_API openFile(const std::string &file);
void AGROS_LIBRARY_API saveFile(const std::string &file);
std::string AGROS_LIBRARY_API getScriptFromModel();

inline std::string AGROS_LIBRARY_API tempDir() { return tempProblemDir().toStdString(); }
inline std::string AGROS_LIBRARY_API cacheDir() { return cacheProblemDir().toStdString(); }

// functions
std::string  AGROS_LIBRARY_API pyVersion();

std::string AGROS_LIBRARY_API dataDir();
void AGROS_LIBRARY_API setDataDir(std::string str);
void AGROS_LIBRARY_API readPlugins();

 struct AGROS_LIBRARY_API PyOptions
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

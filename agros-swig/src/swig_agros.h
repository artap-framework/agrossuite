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

#include "util/util.h"
#include "util/global.h"
#include "scene.h"
#include "solver/field.h"
#include "solver/problem.h"

class Solution;

// ************************************************************************************

void openFile(const std::string &file, bool openWithSolution);
void saveFile(const std::string &file, bool saveWithSolution);
inline std::string getScriptFromModel() { return createPythonFromModel().toStdString(); }

inline std::string tempDir() { return tempProblemDir().toStdString(); }
inline std::string cacheDir() { return cacheProblemDir().toStdString(); }

// functions
void swigInitSingleton();
std::string swigVersion();
std::string swigDatadir(std::string str = "");

struct SwigOptions
{
    // cache size
    inline int getCacheSize() const { return Agros::configComputer()->value(Config::Config_CacheSize).toInt(); }
    void setCacheSize(int size);

    // save matrix and rhs
    inline bool getSaveMatrixRHS() const { return Agros::configComputer()->value(Config::Config_LinearSystemSave).toBool(); }
    inline void setSaveMatrixRHS(bool save) { Agros::configComputer()->setValue(Config::Config_LinearSystemSave, save); }

    inline std::string getDumpFormat() const { return dumpFormatToStringKey((MatrixExportFormat) Agros::configComputer()->value(Config::Config_LinearSystemFormat).toInt()).toStdString(); }
    void setDumpFormat(std::string format);
};

#endif // PYTHONENGINEAGROS_H

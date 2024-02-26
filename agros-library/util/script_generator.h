// This plugin is part of Agros2D.
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


// Windows DLL export/import definitions
#ifdef _MSC_VER
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


#ifndef SCRIPTGENERATOR_H
#define SCRIPTGENERATOR_H

#include "QString"

class AGROS_LIBRARY_API ScriptGenerator
{
public:
    ScriptGenerator();

    inline void setParametersAsVariables(bool v) { m_parametersAsVariables = v; }
    inline void setAddSolution(bool v) { m_addSolution = v; }
    inline void setAddComputation(bool v) { m_addComputation = v; }

    QString createPythonFromModel();

private:
    bool m_addComputation;
    bool m_addSolution;
    bool m_parametersAsVariables;
};

#endif // SCRIPTGENERATOR_H

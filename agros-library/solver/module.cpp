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

#include "module.h"
#include "weak_form.h"
#include "field.h"
#include "problem.h"
#include "logview.h"

#include "util/util.h"
#include "util/global.h"
#include "scene.h"
#include "scenebasic.h"
#include "scenelabel.h"
#include "sceneedge.h"
#include "solver/solver.h"
#include "solver/coupling.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"
#include "solver/problem_config.h"

#include "util/constants.h"

// #include "../../resources_source/classes/module_xml.h"

QMap<QString, QString> Module::availableModules()
{  
    QMap<QString, QString> modules;

    foreach (PluginInterface *plugin, Agros::plugins().values())
        modules[plugin->fieldId()] = plugin->moduleJson()->name;

    return modules;
}

QStringList Module::availableCouplings()
{
    QStringList list;

    // read images
    QStringList filters;
    filters << "*.json";

    QDir dir(QString("resources/couplings/"));
    dir.setNameFilters(filters);

    foreach (QString id, dir.entryList())
        list.append(id.left(id.count() - 4));

    return list;
}

// ***********************************************************************************************

Module::BoundaryType::~BoundaryType()
{
    m_variables.clear();
}


// ***********************************************************************************************

// dialog UI
Module::DialogRow Module::DialogUI::dialogRow(const QString &id)
{
    foreach (QList<Module::DialogRow> rows, m_groups)
        foreach (Module::DialogRow row, rows)
            if (row.id() == id)
                return row;

    assert(0);
}

void Module::DialogUI::clear()
{
    m_groups.clear();
}



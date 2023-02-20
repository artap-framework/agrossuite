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

#include "generator.h"
#include "generator_module.h"

QString Agros2DGeneratorModule::underline(QString text,  char symbol)
{
    QString underlined = text + "\n";
    for(int i = 0; i < text.length(); i++)
    {
        underlined += symbol;
    }
    underlined += "\n";
    return underlined;
}

QString Agros2DGeneratorModule::capitalize(QString text)
{
    text[0] = text[0].toUpper();
    return text;
}

QString Agros2DGeneratorModule::createTable(QList<QStringList> table)
{
    QString text = "";
    int columnsNumber = table.length();
    int rowNumber = table.at(0).length();

    QList<int> columnWidths;
    for(int i = 0; i < columnsNumber; i++)
    {
        columnWidths.append(0);
        for(int j = 0; j < table.at(i).length(); j++)
        {
            if(columnWidths[i] < table.at(i).at(j).length())
                columnWidths[i] = table.at(i).at(j).length();
        }
        columnWidths[i] += 3;
    }

    for(int k = 0; k < rowNumber; k++ )
    {
        text += "+";
        for(int i = 0; i < columnsNumber; i++)
        {
            for(int j =0; j < columnWidths[i] - 1; j++)
            {
                if (k == 1)
                    text += "=";
                else
                    text += "-";
            }
            text += "+";
        }
        text += "\n";

        for(int i = 0; i < columnsNumber; i++)
        {
            QString item =  "| " + table.at(i).at(k) + " ";
            int rest = columnWidths.at(i) - item.length();
            QString fillRest(rest, ' ');
            text += item + fillRest;
        };
        text += "|\n";
    }
    text += "+";
    for(int i = 0; i < columnsNumber; i++)
    {
        for(int j =0; j < columnWidths[i] - 1; j++)
        {
            text += "-";
        }
        text += "+";
    }
    text += "\n\n";
    return text;
}

void Agros2DGeneratorModule::generatePluginDocumentationFiles()
{
    QString id = QString::fromStdString(m_module->general_field().id());
    //    QString name = QString::fromStdString(m_module->general_field().name());
    QString text = "";
    //   text += underline(name,'=');
    //   text += QString::fromStdString(m_module->general_field().description()) + "\n\n";

    /* Creates table of constants */

    text += underline("Constants:",'-');
    text += "\n";

    QList<QStringList> table;
    QStringList names;
    QStringList values;

    names.append("Agros variable");
    values.append("Units");
    foreach(XMLModule::constant con, m_module->constants().constant())
    {
        names.append(QString::fromStdString(con.id()));
        values.append(QString::number(con.value()));
    }

    table.append(names);
    table.append(values);
    text += createTable(table);
    text += "\n\n";
    names.clear();
    values.clear();
    table.clear();

    // Creates volume variables table
    text += underline("Volume variables:",'-');
    text += "\n";

    names.append("Volume (material) variable");
    values.append("Agros variable");
    foreach(XMLModule::quantity quantity, m_module->volume().quantity())
    {
        values.append(QString::fromStdString(quantity.id().c_str()));
        names.append(QString::fromStdString(quantity.shortname().get()));
    }

    table.append(values);
    table.append(names);
    text += createTable(table);
    text += "\n\n";
    names.clear();
    values.clear();
    table.clear();

    // Creates surface variables table
    text += underline("Surface (boundary) variables:",'-');
    text += "\n";

    names.append("Surface variable");
    values.append("Agros variable");
    foreach(XMLModule::quantity quantity, m_module->surface().quantity())
    {
        values.append(QString::fromStdString(quantity.id().c_str()));
        names.append(QString::fromStdString(quantity.shortname().get()));
    }

    table.append(values);
    table.append(names);
    text += createTable(table);
    text += "\n\n";
    names.clear();
    values.clear();
    table.clear();


    /* Creates variable table */

    text += underline("Postprocessor variables:", '-');
    text +=  "\n";


    QStringList latexShortNames;
    QStringList units;
    QStringList descriptions;
    QStringList shortNames;


    latexShortNames.append("Name");
    units.append("Units");
    descriptions.append("Description");
    shortNames.append("Agros variable");

    foreach(XMLModule::localvariable var, m_module->postprocessor().localvariables().localvariable())
    {
        shortNames.append(var.shortname().c_str());
        if(var.shortname_latex().present())
            latexShortNames.append(QString::fromStdString(":math:`" + var.shortname_latex().get() + "`"));
        else latexShortNames.append(" ");
        units.append(var.unit().c_str());
        descriptions.append(QString::fromStdString(var.name()));
    }

    table.append(shortNames);
    table.append(latexShortNames);
    table.append(units);
    table.append(descriptions);
    text += createTable(table);
    text += "\n\n";
    table.clear();

    // Creates table of volume integrals
    text += underline("Volume integrals:", '-');
    text +=  "\n";

    latexShortNames.clear();
    units.clear();
    descriptions.clear();
    shortNames.clear();

    latexShortNames.append("Name");
    units.append("Units");
    descriptions.append("Description");
    shortNames.append("Agros variable");


    foreach(XMLModule::volumeintegral volume_int, m_module->postprocessor().volumeintegrals().volumeintegral())
    {
        shortNames.append(volume_int.shortname().c_str());
        if(volume_int.shortname_latex().present())
            latexShortNames.append(QString::fromStdString(":math:`" + volume_int.shortname_latex().get() + "`"));
        else latexShortNames.append(" ");
        units.append(volume_int.unit().c_str());
        descriptions.append(QString::fromStdString(volume_int.name()));
    }

    table.append(shortNames);
    table.append(latexShortNames);
    table.append(units);
    table.append(descriptions);
    text += createTable(table);
    text += "\n\n";
    table.clear();


    // Creates table of volume integrals
    text += underline("Surface integrals:", '-');
    text +=  "\n";

    latexShortNames.clear();
    units.clear();
    descriptions.clear();
    shortNames.clear();

    latexShortNames.append("Name");
    units.append("Units");
    descriptions.append("Description");
    shortNames.append("Agros variable");


    foreach(XMLModule::surfaceintegral surf_int, m_module->postprocessor().surfaceintegrals().surfaceintegral())
    {
        shortNames.append(surf_int.shortname().c_str());
        if(surf_int.shortname_latex().present())
            latexShortNames.append(QString::fromStdString(":math:`" + surf_int.shortname_latex().get() + "`"));
        else latexShortNames.append(" ");
        units.append(surf_int.unit().c_str());
        descriptions.append(QString::fromStdString(surf_int.name()));
    }

    table.append(shortNames);
    table.append(latexShortNames);
    table.append(units);
    table.append(descriptions);
    text += createTable(table);

    // documentation - save to file
    writeStringContent(QString("%1/%2/%3/%3.gen").
                       arg(QCoreApplication::applicationDirPath()).
                       arg(GENERATOR_DOCROOT).
                       arg(id),
                       text);
}


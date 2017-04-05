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

#include "scene.h"

#include "solver/module.h"
#include "util/constants.h"
#include "util/global.h"
#include "util/system_utils.h"

Config::Config()
{
    setStringKeys();
    setDefaultValues();

    clear();

    load();
}

Config::~Config()
{
    save();
}

void Config::clear()
{
    // set default values and types
    m_setting.clear();
    setDefaultValues();

    m_setting = m_settingDefault;
}

void Config::load()
{
    // default
    m_setting = m_settingDefault;

    QSettings settings;

    foreach (Type key, m_settingKey.keys())
    {
        if (m_settingDefault.keys().contains(key))
        {
            if (m_settingDefault[key].type() == QVariant::Double)
                m_setting[key] = settings.value(m_settingKey[key], m_settingDefault[key]).toDouble();
            else if (m_settingDefault[key].type() == QVariant::Int)
                m_setting[key] = settings.value(m_settingKey[key], m_settingDefault[key]).toInt();
            else if (m_settingDefault[key].type() == QVariant::Bool)
                m_setting[key] = settings.value(m_settingKey[key], m_settingDefault[key]).toBool();
            else if (m_settingDefault[key].type() == QVariant::String)
                m_setting[key] = settings.value(m_settingKey[key], m_settingDefault[key]).toString();
            else
                qDebug() << "Unknown datatype not found" << m_settingKey.value(key);
        }
    }
}

void Config::save()
{
    QSettings settings;

    foreach (Type key, m_setting.keys())
        settings.setValue(m_settingKey[key], m_setting[key]);
}

void Config::setStringKeys()
{
    m_settingKey[Config_LogStdOut] = "Config_LogStdOut";
    m_settingKey[Config_ExternalPythonEditor] = "Config_ExternalPythonEditor";
    m_settingKey[Config_Locale] = "Config_Locale";
    m_settingKey[Config_ShowResults] = "Config_ShowResults";
    m_settingKey[Config_LinearSystemFormat] = "Config_LinearSystemFormat";
    m_settingKey[Config_LinearSystemSave] = "Config_LinearSystemSave";
    m_settingKey[Config_CacheSize] = "Config_CacheSize";
    m_settingKey[Config_ShowGrid] = "Config_ShowGrid";
    m_settingKey[Config_ShowRulers] = "Config_ShowRulers";
    m_settingKey[Config_ShowAxes] = "Config_ShowAxes";
    m_settingKey[Config_RulersFontFamily] = "Config_RulersFontFamily";
    m_settingKey[Config_RulersFontPointSize] = "Config_RulersFontPointSize";
    m_settingKey[Config_PostFontFamily] = "Config_PostFontFamily";
    m_settingKey[Config_PostFontPointSize] = "Config_PostFontPointSize";    
}

void Config::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[Config_LogStdOut] = false;
    m_settingDefault[Config_ExternalPythonEditor] = "pyzo";
    m_settingDefault[Config_Locale] = ""; // defaultLocale();
    m_settingDefault[Config_ShowResults] = false;
    m_settingDefault[Config_LinearSystemFormat] = EXPORT_FORMAT_MATLAB_MATIO;
    m_settingDefault[Config_LinearSystemSave] = false;
    m_settingDefault[Config_CacheSize] = 10;
    m_settingDefault[Config_ShowGrid] = true;
    m_settingDefault[Config_ShowRulers] = true;
    m_settingDefault[Config_ShowAxes] = true;
    m_settingDefault[Config_RulersFontFamily] = QString("Droid");
    m_settingDefault[Config_RulersFontPointSize] = 12;
    m_settingDefault[Config_PostFontFamily] = QString("Droid");
    m_settingDefault[Config_PostFontPointSize] = 16;
}

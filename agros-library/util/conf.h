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

#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QSettings>
#include <QString>

// Windows DLL export/import definitions
#ifdef _MSC_VER
#define AGROS_LIBRARY_API __declspec(dllexport)
#else
//// linux
#define AGROS_LIBRARY_API
#endif

class AGROS_LIBRARY_API Config : public QObject
{
public:
    Config();
    ~Config();

    enum Type
    {
        Unknown,
        Config_LogStdOut,
        Config_ReloadStyle,
        Config_Locale,
        Config_ShowResults,
        Config_LinearSystemFormat,
        Config_LinearSystemSave,
        Config_MatrixSystemSave,
        Config_CacheSize,        
        Config_RulersFontPointSize,
        Config_PostFontPointSize,        
        Config_ShowRulers,
        Config_CheckNewVersion,
    };

    inline QVariant value(Type type) {  return m_setting[type]; }
    inline void setValue(Type type, int value) {  m_setting[type] = value; }
    inline void setValue(Type type, double value) {  m_setting[type] = value; }
    inline void setValue(Type type, bool value) {  m_setting[type] = value; }
    inline void setValue(Type type, const QString &value) { m_setting[type] = value; }
    inline void setValue(Type type, const QStringList &value) { m_setting[type] = value; }

    inline QVariant defaultValue(Type type) const { return m_settingDefault[type]; }

    inline QString keyString(Type type) const { return m_settingKey[type]; }

    void load();
    void save();

    void clear();

private:
    QMap<Type, QVariant> m_setting;
    QMap<Type, QVariant> m_settingDefault;
    QMap<Type, QString> m_settingKey;

    void setDefaultValues();
    void setStringKeys();
};

#endif // CONFIG_H

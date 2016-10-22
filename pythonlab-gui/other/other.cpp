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

#include "other.h"

static QtAwesome *m_awesome = nullptr;
static QMap<QString, QIcon> *m_iconCache = nullptr;

QIcon iconAwesome(int character)
{
    if (!m_awesome)
    {
        m_awesome = new QtAwesome();
        m_awesome->initFontAwesome();
    }

    return m_awesome->icon(character);
}

QIcon icon(const QString &name)
{
    if (!m_iconCache)
       m_iconCache = new QMap<QString, QIcon>();

    if (m_iconCache->contains(name))
    {
        return m_iconCache->value(name);
    }
    else
    {
        if (QFile::exists(":/" + name + ".png"))
            m_iconCache->insert(name, QIcon(":/" + name + ".png"));
        else
            m_iconCache->insert(name, QIcon());

        return m_iconCache->value(name);
    }
}

QString defaultGUIStyle()
{
    QString styleName = "";
    QStringList styles = QStyleFactory::keys();

#ifdef Q_WS_X11
    // kde 4
    if (getenv("KDE_SESSION_VERSION") != NULL)
    {
        if (styles.contains("Oxygen"))
            styleName = "Oxygen";
        else
            styleName = "Plastique";
    }
    // gtk+
    if (styleName.isEmpty())
        styleName = "GTK+";
#endif

#ifdef Q_WS_WIN
    if (styles.contains("WindowsVista"))
        styleName = "WindowsVista";
    else if (styles.contains("WindowsXP"))
        styleName = "WindowsXP";
    else
        styleName = "Windows";
#endif

#ifdef Q_WS_MAC
    styleName = "Aqua";
#endif

    return styleName;
}

void setGUIStyle(const QString &styleName)
{
    // standard style
    QStyle *style = QStyleFactory::create(styleName);

    QApplication::setStyle(style);
    if (QApplication::desktopSettingsAware())
    {
        QApplication::setPalette(QApplication::palette());
    }
}

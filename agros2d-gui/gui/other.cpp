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

static QUndoStack *m_undoStack = nullptr;
static QtAwesome *m_awesome = nullptr;
static QMap<QString, QIcon> *m_iconCache = nullptr;

// undo framework
QUndoStack *undoStack()
{
    if (!m_undoStack)
        m_undoStack = new QUndoStack();
    return  m_undoStack;
}

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

QIcon iconAlphabet(const QChar &letter, AlphabetColor color)
{
    QString directory = "";

    switch (color)
    {
    case AlphabetColor_Blue:
        directory = "blue";
        break;
    case AlphabetColor_Bluegray:
        directory = "bluegray";
        break;
    case AlphabetColor_Brown:
        directory = "brown";
        break;
    case AlphabetColor_Green:
        directory = "green";
        break;
    case AlphabetColor_Lightgray:
        directory = "lightgray";
        break;
    case AlphabetColor_Purple:
        directory = "purple";
        break;
    case AlphabetColor_Red:
        directory = "red";
        break;
    case AlphabetColor_Yellow:
        directory = "yellow";
        break;
    default:
        assert(0);
    }

    static QString alphabet = "abcdefghijklmnopqrstuvwxyz";

    if (alphabet.contains(letter.toLower()))
        return icon(QString("alphabet/%1/%2").arg(directory).arg(letter.toLower()));
    else
        return icon(QString("alphabet/%1/imageback").arg(directory));
}

void showPage(const QString &str)
{
    if (str.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/index.html"));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/" + str));
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

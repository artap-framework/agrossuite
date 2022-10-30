// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#ifdef WITH_UNITY
#include <unity.h>
#endif

#include "util.h"
#include "config.h"

#ifdef Q_WS_WIN
#include "Windows.h"
#pragma comment(lib, "psapi.lib")
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923	/* pi/2 */
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QStandardPaths>
#endif

const QString LANGUAGEROOT = QString("%1/resources%1lang").arg(QDir::separator());

bool almostEqualRelAndAbs(double A, double B, double maxDiff, double maxRelDiff)
{
    // Check if the numbers are really close -- needed when comparing numbers near zero.
    double diff = fabs(A - B);
    if (diff <= maxDiff)
        return true;

    A = fabs(A);
    B = fabs(B);
    double largest = (B > A) ? B : A;

    if (diff <= largest * maxRelDiff)
        return true;

    return false;
}

#define FAST_TRIGONOMETRIC_SIZE 32000

// approximation of atan2(y, x).
// maximum error of 0.0061 radians at 0.35 degrees
double fastatan2(double y, double x)
{
    double abs_y = std::fabs(y) + 1e-8;

    double angle;
    double r;
    if (x >= 0)
    {
        r = (x - abs_y) / (x + abs_y);
        angle = 0.78539816339744830962;
    }
    else
    {
        r = (x + abs_y) / (abs_y - x);
        angle = 3.0f * 0.78539816339744830962;
    }
    angle += (0.1821f * r*r - 0.9675f) * r;

    return (y < 0) ? - angle : angle;
}

static double *sin_table = NULL;
double fastsin(double angle)
{
    while (angle < 0) angle += 2.0*M_PI;
    while (angle >= 2.0*M_PI) angle -= 2.0*M_PI;

    if (!sin_table)
    {
        sin_table = new double[FAST_TRIGONOMETRIC_SIZE];
        for (int i = 0; i < FAST_TRIGONOMETRIC_SIZE; i++)
            sin_table[i] = sin(2.0*M_PI * i / (double) (FAST_TRIGONOMETRIC_SIZE - 1));
    }

    return sin_table[(int) (angle / (2.0*M_PI) * FAST_TRIGONOMETRIC_SIZE)];
}

double fastcos(double angle)
{
    return fastsin(M_PI_2 - angle);
}

QStringList availableLanguages()
{
    QDir dir;
    dir.setPath(datadir() + LANGUAGEROOT);

    // add all translations
    QStringList filters;
    filters << "*.qm";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);

    // remove extension
    QStringList list = dir.entryList();
    list.replaceInStrings(".qm", "");

    // remove system translations
    foreach (QString str, list)
    {
        if (str.startsWith("qt_"))
            list.removeOne(str);
        if (str.startsWith("plugin_"))
            list.removeOne(str);
    }

    return list;
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

QString defaultLocale()
{
    if (QLocale::system().name() == "C")
        return "en_US";
    else
        return QLocale::system().name();
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

void setLocale(const QString &locale)
{
    // non latin-1 chars
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
#endif

    QTranslator *qtTranslator = new QTranslator();
    QTranslator *appTranslator = new QTranslator();
    QTranslator *pluginTranslator = new QTranslator();

    QString country = locale.section('_',0,0);
    if (QFile::exists(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm"))
        qtTranslator->load(QLibraryInfo::location(QLibraryInfo::TranslationsPath) + "/qt_" + country + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/qt_" + country + ".qm"))
        qtTranslator->load(datadir() + LANGUAGEROOT + "/qt_" + country + ".qm");
    else
        qDebug() << "Qt locale file not found.";

    if (QFile::exists(datadir() + LANGUAGEROOT + QDir::separator() + locale + ".qm"))
        appTranslator->load(datadir() + LANGUAGEROOT + QDir::separator() + locale + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/en_US.qm"))
        appTranslator->load(datadir() + LANGUAGEROOT + "/en_US.qm");
    else
        qDebug() << "Locale file not found.";

    if (QFile::exists(datadir() + LANGUAGEROOT + QDir::separator() + "plugin_" + locale + ".qm"))
        pluginTranslator->load(datadir() + LANGUAGEROOT + QDir::separator() + "plugin_" + locale + ".qm");
    else if (QFile::exists(datadir() + LANGUAGEROOT + "/plugin_en_US.qm"))
        pluginTranslator->load(datadir() + LANGUAGEROOT + "/plugin_en_US.qm");
    else
        qDebug() << "Plugin locale file not found.";

    QApplication::installTranslator(qtTranslator);
    QApplication::installTranslator(appTranslator);
    QApplication::installTranslator(pluginTranslator);
}

QIcon icon(const QString &name)
{
    // memory leak but works with Qt5.2 and Ubuntu 14.04
    static QMap<QString, QIcon> *iconCache = new QMap<QString, QIcon>();

    if (iconCache->contains(name))
        return iconCache->value(name);

#ifdef Q_WS_WIN
    if (QFile::exists(":/" + name + "-windows.png"))
        iconCache->insert(name, QIcon(":/" + name + "-windows.png"));
#endif

#ifdef Q_WS_X11
#if (QT_VERSION >= QT_VERSION_CHECK(4, 6, 0))
    iconCache->insert(name, QIcon::fromTheme(name, QIcon(":/" + name + ".png")));
#endif
#endif

    if (!iconCache->contains(name))
    {
        if (QFile::exists(":/" + name + ".svg"))
            iconCache->insert(name, QIcon(":/" + name + ".svg"));
        else if (QFile::exists(":/" + name + ".png"))
            iconCache->insert(name, QIcon(":/" + name + ".png"));
        else
            iconCache->insert(name, QIcon());
    }

    return iconCache->value(name);
}

QString compatibleFilename(const QString &fileName)
{
    QString out = QFileInfo(fileName).absoluteFilePath();

#ifdef Q_WS_WIN
    if (!QFile::exists(fileName))
        out = QString("%1").arg(QFileInfo(fileName).absolutePath());


#ifdef UNICODE
    // For some reason, it seems that unicode does not work on Qt 4.8.4 on Windows (the method from WCharArray gives a linker error).
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    TCHAR szshortpath[4096];
    GetShortPathName((LPCWSTR) out.replace("/", "\\\\").utf16(), szshortpath, 4096);
    out = QString::fromWCharArray(szshortpath);
#else
    char szshortpath[4096];
    GetShortPathNameA((LPCSTR) out.replace("/", "\\").toAscii(), szshortpath, 4096);
    out = QString::fromLocal8Bit(szshortpath);
#endif
#else
    TCHAR szshortpath[4096];
    GetShortPathName((LPCSTR) out.replace("/", "\\").toAscii(), szshortpath, 4096);
    out = QString::fromLocal8Bit(szshortpath);
#endif

    if (!QFile::exists(fileName))
        out = out + "/" + QFileInfo(fileName).fileName();
#endif

    return out;
}

QString datadir()
{
    // windows and local installation
    if (QFile::exists(QCoreApplication::applicationDirPath() + "/resources/python/functions_pythonlab.py"))
        return QCoreApplication::applicationDirPath();

    // linux
    if (QFile::exists(QCoreApplication::applicationDirPath() + "/../share/agros2d/resources/python/functions_pythonlab.py"))
        return QCoreApplication::applicationDirPath() + "/../share/agros2d";

    qCritical() << "Datadir not found.";
    exit(1);
}

QString tempProblemDir()
{
#ifdef Q_WS_WIN
    static QString str = QString("%1/agros2d/%2").
            arg(QDir::temp().absolutePath()).
            arg(QString::number(QCoreApplication::applicationPid()));
#else
    static QString str = QString("%1/agros2d-%2/%3").
            arg(QDir::temp().absolutePath()).
            arg(getenv("USER")).
            arg(QString::number(QCoreApplication::applicationPid()));
#endif

    QDir dir(str);
    if (!dir.exists())
        dir.mkpath(str);

    return str;
}

QString cacheProblemDir()
{
#ifdef Q_WS_X11
    // fast fix for ht condor
    static QString cch = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir dirc(cch);
    if (!dirc.exists() && !cch.isEmpty())
        dirc.mkpath(cch);

    // ro system
    if (!dirc.exists())
        cch = tempProblemDir();

    static QString str = QString("%1/%2").
            arg(cch).
            arg(QString::number(QCoreApplication::applicationPid()));
#endif
#ifdef Q_WS_WIN
    static QString str = QString("%1/agros2d/%2").
            arg(QDir::temp().absolutePath()).
            arg(QString::number(QCoreApplication::applicationPid()));
#endif

    QDir dir(str);
    if (!dir.exists() && !str.isEmpty())
        dir.mkpath(str);

    return str;
}

QString userDataDir()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    static QString str = QString("%1/agros2d/").
            arg(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
#else
#ifdef Q_WS_WIN
    static QString str = QString("%1/agros2d/").
            arg(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#else
    static QString str = QString("%1").
            arg(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
#endif
#endif

    QDir dir(str);
    if (!dir.exists())
        dir.mkpath(str);

    return str;
}

QString tempProblemFileName()
{
    return tempProblemDir() + "/temp";
}

QTime milisecondsToTime(int ms)
{
    // store the current ms remaining
    int tmp_ms = ms;

    // the amount of days left
    int days = floorf(tmp_ms/86400000);
    // adjust tmp_ms to leave remaining hours, minutes, seconds
    tmp_ms = tmp_ms - (days * 86400000);

    // calculate the amount of hours remaining
    int hours = floorf(tmp_ms/3600000);
    // adjust tmp_ms to leave the remaining minutes and seconds
    tmp_ms = tmp_ms - (hours * 3600000);

    // the amount of minutes remaining
    int mins = floorf(tmp_ms/60000);
    //adjust tmp_ms to leave only the remaining seconds
    tmp_ms = tmp_ms - (mins * 60000);

    // seconds remaining
    int secs = floorf(tmp_ms/1000);

    // milliseconds remaining
    tmp_ms = tmp_ms - (secs * 1000);

    return QTime(hours, mins, secs, tmp_ms);
}

bool removeDirectory(const QString &str)
{
    bool error = false;

    if (QDir(str).exists())
    {
        QFileInfoList entries = QDir(str).entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        int count = entries.size();
        for (int idx = 0; idx < count; idx++)
        {
            QFileInfo entryInfo = entries[idx];
            QString path = entryInfo.absoluteFilePath();
            if (entryInfo.isDir())
            {
                error = removeDirectory(path);
            }
            else
            {
                QFile file(path);
                if (!file.remove())
                {
                    error = true;
                    break;
                }
            }
        }
        if (!QDir().rmdir(str))
            error = true;
    }

    return error;
}

void msleep(unsigned long msecs)
{
    QWaitCondition w;
    QMutex sleepMutex;
    sleepMutex.lock();
    w.wait(&sleepMutex, msecs);
    sleepMutex.unlock();
}

void appendToFile(const QString &fileName, const QString &str)
{
    QFile file(fileName);

    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream outFile(&file);
        outFile << str << Qt::endl;

        file.close();
    }
}

QString versionString()
{
    return versionString(VERSION_MAJOR, VERSION_MINOR, VERSION_SUB, VERSION_YEAR, VERSION_MONTH, VERSION_DAY);
}

bool version64bit()
{
#if _WIN32 || _WIN64
#if _WIN64
    return true;
#else
    return false;
#endif
#endif

#if __GNUC__
#if __x86_64__ || __ppc64__
    return true;
#else
    return false;
#endif
#endif
}

QString stringListToString(const QStringList &list)
{
    QString out;
    foreach (QString str, list)
        if (!str.isEmpty())
            out += str + ", ";

    if (out.length() > 0)
        out = out.left(out.length() - 2);

    return out;
}

void showPage(const QString &str)
{
    if (str.isEmpty())
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/index.html"));
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(datadir() + "/resources/help/" + str));
}


QString readFileContent(const QString &fileName)
{
    QString content;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        content = stream.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContent(const QString &fileName, QString content)
{
    writeStringContent(fileName, &content);
}

void writeStringContent(const QString &fileName, QString *content)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&file);
        stream << *content;

        file.waitForBytesWritten(0);
        file.close();
    }
}

QByteArray readFileContentByteArray(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        file.close();
        return content;
    }
    return NULL;
}

void writeStringContentByteArray(const QString &fileName, QByteArray content)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(content);

        file.waitForBytesWritten(0);
        file.close();
    }
}

QString unitToHTML(const QString &str)
{
    // dirty hack
    QString out = str;

    out.replace("-2", "<sup>&#8722;2</sup>");
    out.replace("-3", "<sup>&#8722;3</sup>");
    out.replace("2", "<sup>2</sup>");
    out.replace("3", "<sup>3</sup>");

    return out;
}


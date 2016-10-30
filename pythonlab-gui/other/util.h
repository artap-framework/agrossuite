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

#ifndef UTIL_H
#define UTIL_H

#include <QtCore>
#include <QtNetwork>
#include <QDomDocument>
#include <QtXmlPatterns>
#include <QtPlugin>
#include <QtGlobal>
#include <QtPrintSupport/QPrinter>
#include <QtWidgets>

#include <typeinfo>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <limits>
#include <vector>
#include <memory>
#include <assert.h>
#include <locale.h>
#include <stdlib.h>

// qt5
#ifdef Q_OS_WIN
#define Q_WS_WIN
#endif

#ifdef Q_OS_MAC
#define Q_WS_MAC
#endif

#ifdef Q_OS_LINUX
#define Q_WS_X11
#endif

// Windows DLL export/import definitions
#ifdef Q_WS_WIN
// windows
// DLL build
#ifdef AGROS_LIBRARY_DLL
#define AGROS_LIBRARY_API __declspec(dllexport)
// DLL usage
#else
#define AGROS_LIBRARY_API __declspec(dllimport)
#endif
#else
// linux
#define AGROS_LIBRARY_API
#endif

#include "util/point.h"

// zero
#define EPS_ZERO 1e-10

// typename keyword handling
#ifdef _MSC_VER
#define TYPENAME
#else
#define TYPENAME typename
#endif

using namespace std;

AGROS_LIBRARY_API QString stringListToString(const QStringList &list);

// get available languages
AGROS_LIBRARY_API QStringList availableLanguages();

// set language
AGROS_LIBRARY_API void setLocale(const QString &locale);
AGROS_LIBRARY_API QString defaultLocale();

// windows short name
AGROS_LIBRARY_API QString compatibleFilename(const QString &fileName);

// get datadir
AGROS_LIBRARY_API QString datadir();

// get temp dir
AGROS_LIBRARY_API QString tempProblemDir();
AGROS_LIBRARY_API QString cacheProblemDir();

// get user dir
AGROS_LIBRARY_API QString userDataDir();

// get temp filename
AGROS_LIBRARY_API QString tempProblemFileName();

// convert time in ms to QTime
AGROS_LIBRARY_API QTime milisecondsToTime(int ms);

// remove directory content
AGROS_LIBRARY_API bool removeDirectory(const QString &str);

// sleep function
AGROS_LIBRARY_API void msleep(unsigned long msecs);

// read file content
AGROS_LIBRARY_API QByteArray readFileContentByteArray(const QString &fileName);
AGROS_LIBRARY_API QString readFileContent(const QString &fileName);

// write content into the file
AGROS_LIBRARY_API void writeStringContent(const QString &fileName, QString content);
AGROS_LIBRARY_API void writeStringContent(const QString &fileName, QString *content);
AGROS_LIBRARY_API void writeStringContentByteArray(const QString &fileName, QByteArray content);

// append to the file
AGROS_LIBRARY_API void appendToFile(const QString &fileName, const QString &str);

// join version
AGROS_LIBRARY_API inline QString versionString(int major, int minor, int sub, int year, int month, int day)
{
    return QString("%1.%2.%3.%4%5%6")
            .arg(major)
            .arg(minor)
            .arg(sub)            
            .arg(year)
            .arg(QString("0%1").arg(month).right(2))
            .arg(QString("0%1").arg(day).right(2));
}

AGROS_LIBRARY_API QString versionString();

AGROS_LIBRARY_API bool version64bit();

#endif // UTIL_H
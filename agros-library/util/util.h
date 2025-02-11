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
#include <QtPlugin>
#include <QtGlobal>
#include <QtPrintSupport/QPrinter>

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

#include "../3rdparty/exprtk/exprtk.hpp"

// qt
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
#ifdef _MSC_VER
   #define AGROS_LIBRARY_API __declspec(dllexport)
#else
//// linux
#define AGROS_LIBRARY_API
#endif

// zero
#define EPS_ZERO 1e-10

// typename keyword handling
#ifdef _MSC_VER
#define TYPENAME
#else
#define TYPENAME typename
#endif

// physical constants
#define EPS0 8.854e-12
#define MU0 4*M_PI*1e-7
#define SIGMA0 5.670373e-8
#define PRESSURE_MIN_AIR 20e-6
#define GRAVITATIONAL_ACCELERATION 9.81

#define deg2rad(degrees) (degrees*M_PI/180.0)
#define rad2deg(radians) (radians*180.0/M_PI)

using namespace std;

AGROS_LIBRARY_API bool almostEqualRelAndAbs(double A, double B, double maxDiff, double maxRelDiff);

// approximation of atan2(y, x).
// maximum error of 0.0061 radians at 0.35 degrees
AGROS_LIBRARY_API double fastatan2(double y, double x);
AGROS_LIBRARY_API double fastsin(double angle);
AGROS_LIBRARY_API double fastcos(double angle);

AGROS_LIBRARY_API QString stringListToString(const QStringList &list);

// get available languages
AGROS_LIBRARY_API QStringList availableLanguages();

// set language
AGROS_LIBRARY_API void setLocale(const QString &locale);
AGROS_LIBRARY_API QString defaultLocale();

// windows short name
AGROS_LIBRARY_API QString compatibleFilename(const QString &fileName);

// get temp dir
AGROS_LIBRARY_API QString tempProblemDir();
AGROS_LIBRARY_API QString cacheProblemDir();

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
AGROS_LIBRARY_API void writeStringContent(const QString &fileName, QString content, bool force = false);
AGROS_LIBRARY_API void writeStringContent(const QString &fileName, QString *content, bool force = false);
AGROS_LIBRARY_API void writeStringContentByteArray(const QString &fileName, QByteArray content);

// append to the file
AGROS_LIBRARY_API void appendToFile(const QString &fileName, const QString &str);

// join version
AGROS_LIBRARY_API inline QString versionString(int year, int month, int day)
{
    return QString("%1.%2.%3")
            .arg(year)
            .arg(QString("0%1").arg(month).right(2))
            .arg(QString("0%1").arg(day).right(2));
}

AGROS_LIBRARY_API QString versionString();

AGROS_LIBRARY_API bool version64bit();

// dirty html unit replace
QString unitToHTML(const QString &str);

class AgrosException
{
public:
    AgrosException(const QString &what)
        : m_what(what)
    {
    }

    inline QString what() { return m_what; }
    inline QString toString(){ return m_what; }

protected:
    QString m_what;
};

class AgrosSolverException : public AgrosException
{
public:
    AgrosSolverException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosGeometryException : public AgrosException
{
public:
    AgrosGeometryException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosMeshException : public AgrosException
{
public:
    AgrosMeshException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosPluginException : public AgrosException
{
public:
    AgrosPluginException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosGeneratorException : public AgrosException
{
public:
    AgrosGeneratorException(const QString &what) : AgrosException(what)
    {
    }
};

class AgrosOptilabEvaluationException : public AgrosException
{
public:
    AgrosOptilabEvaluationException(const QString &what) : AgrosException(what)
    {
    }
};

#endif // UTIL_H

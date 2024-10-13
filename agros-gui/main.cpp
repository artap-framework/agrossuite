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

#include <QApplication>

#include "util/util.h"
#include "gui/logwidget.h"
#include "util/conf.h"
#include "util/global.h"
#include "util/system_utils.h"
#include "gui/other.h"
#include "app/mainwindow.h"

class AgrosApplication : public QApplication
{
public:
    AgrosApplication(int& argc, char ** argv) : QApplication(argc, argv)
    {
        setlocale(LC_NUMERIC, "C");

        setWindowIcon(icon("agros"));
        setApplicationVersion(versionString());
        setOrganizationName("agros");
        setOrganizationDomain("agros");
        setApplicationName("agros");

        // force number format
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

        // init singleton
        Agros::createSingleton(QSharedPointer<Log>(new LogGui));
        Agros::readPlugins();
    }

    virtual ~AgrosApplication() {}

    LogGui *log;
};

int main(int argc, char *argv[])
{
    // dealii::MultithreadInfo::set_thread_limit(1);

    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    // QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

    // DPI
#ifdef Q_OS_MAC
    //
#endif

#ifdef Q_OS_LINUX
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);

    const bool hasWaylandDisplay = qEnvironmentVariableIsSet("WAYLAND_DISPLAY");
    const bool isWaylandSessionType = qgetenv("XDG_SESSION_TYPE") == "wayland";
    const QByteArray currentDesktop = qgetenv("XDG_CURRENT_DESKTOP").toLower();
    const QByteArray sessionDesktop = qgetenv("XDG_SESSION_DESKTOP").toLower();
    const bool isGnome = currentDesktop.contains("gnome") || sessionDesktop.contains("gnome");
    const bool isWayland = hasWaylandDisplay || isWaylandSessionType;
    if (isGnome && isWayland)
    {
        // qInfo() << "Warning: Ignoring WAYLAND_DISPLAY on Gnome. Use QT_QPA_PLATFORM=wayland to run on Wayland anyway.";
        // qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

#ifdef Q_OS_WIN
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

    CleanExit cleanExit;
    AgrosApplication app(argc, argv);

    // language
    setLocale(Agros::configComputer()->value(Config::Config_Locale).toString());
    // a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    MainWindow w(argc, argv);

    // command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Agros is an open-source user-friendly finite-element software for teaching, research and engineering.");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption problemOption(QStringList() << "p" << "problem", "The problem file to open.", "problem");
    parser.addOption(problemOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    if (parser.isSet(problemOption))
    {
        QString problem = parser.value(problemOption);

        if (QFile::exists(problem))
        {
            QFileInfo info(problem);
            if (info.suffix() == "ags")
            {
                w.setStartupProblemFilename(problem);
            }
            else
            {
                std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
            }
        }
    }

    w.show();

    return app.exec();
}

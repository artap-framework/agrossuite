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
#include "util/conf.h"
#include "util/global.h"
#include "util/system_utils.h"
#include "gui/other.h"
#include "app/mainwindow.h"

#include "../3rdparty/tclap/CmdLine.h"

#include "boost/archive/archive_exception.hpp"
#include <deal.II/base/exceptions.h>

#ifdef AGROS_BUILD_STATIC
#include "../plugins/plugins_static.h"
#endif

class AGROS_LIBRARY_API AgrosApplication : public QApplication
{
public:
    AgrosApplication(int& argc, char ** argv) : QApplication(argc, argv)
    {
        setlocale(LC_NUMERIC, "C");

        setWindowIcon(icon("agros2d"));
        setApplicationVersion(versionString());
        setOrganizationName("agros");
        setOrganizationDomain("agros");
        setApplicationName("Agros2D");

    #ifdef Q_WS_MAC
        // don't show icons in menu
        setAttribute(Qt::AA_DontShowIconsInMenus, true);
    #endif

    #ifdef Q_WS_X11
        // css fix for QScrollArea in QTabWidget
        // setStyleSheet("QScrollArea { background: transparent; } QScrollArea > QWidget > QWidget { background: transparent; }");
    #endif

        // std::string codec
    #if QT_VERSION < 0x050000
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    #endif
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

        // force number format
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

        // init singleton
        Agros2D::createSingleton();
    }

    virtual ~AgrosApplication() {}

    virtual bool notify(QObject *receiver, QEvent *event)
    {
        try
        {
            // if (!receiver->objectName().isEmpty())
            //     qDebug() << "receiver" << receiver->objectName() << event->type();

            return QApplication::notify(receiver, event);
        }
        catch (dealii::ExceptionBase &e)
        {
            qCritical() << "deal.II exception thrown: " << e.what();
            throw;
        }
        catch (boost::archive::archive_exception &e)
        {
            qCritical() << "boost exception thrown: " << e.what();
            throw;
        }
        catch (std::exception &e)
        {
            qCritical() << "Standard exception thrown: " << e.what();
            throw;
        }
        catch (AgrosException &e)
        {
            qCritical() << "Agros exception thrown: " << e.what();
            throw;
        }
        catch (...)
        {
            qCritical() << "Unknown exception thrown";
            throw;
        }

        return false;
    }
};

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Agros2D", ' ', versionString().toStdString());

        TCLAP::ValueArg<std::string> problemArg("p", "problem", "Open problem", false, "", "string");

        cmd.add(problemArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        CleanExit cleanExit;
        AgrosApplication a(argc, argv);

        // setting gui style
        setGUIStyle(Agros2D::configComputer()->value(Config::Config_GUIStyle).toString());
        // language
        setLocale(Agros2D::configComputer()->value(Config::Config_Locale).toString());
        a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

        MainWindow w(argc, argv);

        if (!problemArg.getValue().empty())
        {
            if (QFile::exists(QString::fromStdString(problemArg.getValue())))
            {
                QFileInfo info(QString::fromStdString(problemArg.getValue()));
                if (info.suffix() == "ags")
                {
                    w.setStartupProblemFilename(QString::fromStdString(problemArg.getValue()));
                }
                else
                {
                    std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
                }
            }
        }        

        w.show();

        return a.exec();
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }    
}

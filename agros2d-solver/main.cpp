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

#include <QCoreApplication>

#include <QTranslator>
#include <QTextCodec>
#include <QDir>
#include <QString>

#include "util/system_utils.h"
#include "logview.h"
#include "optilab/study.h"

#include "../3rdparty/tclap/CmdLine.h"

#ifdef AGROS_BUILD_STATIC
#include "../plugins/plugins_static.h"
#endif

int main(int argc, char *argv[])
{
    try
    {
        // command line info
        TCLAP::CmdLine cmd("Agros2D solver", ' ', versionString().toStdString());

        TCLAP::SwitchArg logArg("l", "enable-log", "Enable log", true);
        TCLAP::ValueArg<std::string> problemArg("p", "problem", "Solve problem", false, "", "string");
        TCLAP::SwitchArg writeArg("w", "write", "Write solution", false);
        TCLAP::ValueArg<int> studyArg("s", "study", "Run study", false, -1, "int");

        cmd.add(logArg);
        cmd.add(problemArg);
        cmd.add(writeArg);
        cmd.add(studyArg);

        // parse the argv array.
        cmd.parse(argc, argv);

        CleanExit cleanExit;
        // AgrosSolver a(argc, argv);

        if (!problemArg.getValue().empty())
        {
            if (QFile::exists(QString::fromStdString(problemArg.getValue())))
            {
                QFileInfo info(QString::fromStdString(problemArg.getValue()));
                if (info.suffix() == "ags")
                {
                    setlocale(LC_NUMERIC, "C");

                    QCoreApplication a(argc, argv);
                    QCoreApplication::setApplicationVersion(versionString());
                    QCoreApplication::setOrganizationName("agros");
                    QCoreApplication::setOrganizationDomain("agros");
                    QCoreApplication::setApplicationName("Agros2D");

                    // std::string codec
                    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

                    // force number format
                    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

                    // init singleton
                    Agros2D::createSingleton();

                    // enable log
                    if (logArg.getValue())
                        LogStdOut *log = new LogStdOut();

                    QTime time;
                    time.start();

                    try
                    {
                        Agros2D::problem()->readProblemFromArchive(QString::fromStdString(problemArg.getValue()));

                        Agros2D::log()->printMessage(QObject::tr("Problem"), QObject::tr("Problem '%1' successfuly loaded").arg(QString::fromStdString(problemArg.getValue())));

                        if (studyArg.getValue() == -1)
                        {
                            // solve problem
                            QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
                            computation->solve();
                        }
                        else
                        {
                            // run study
                            Study *study = Agros2D::problem()->studies()->items().at(studyArg.getValue());

                            // solve
                            if (study)
                                study->solve();
                        }

                        // save solution
                        if (writeArg.getValue())
                            Agros2D::problem()->writeProblemToArchive(QString::fromStdString(problemArg.getValue()), false);

                        Agros2D::log()->printMessage(QObject::tr("Solver"), QObject::tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));

                        // clear all
                        Agros2D::problem()->clearFields();

                        return -1;
                    }
                    catch (AgrosException &e)
                    {
                        Agros2D::log()->printError(QObject::tr("Problem"), e.toString());
                        return -1;
                    }

                    return 0;
                }
                else
                {
                    std::cout << QObject::tr("Unknown suffix.").toStdString() << std::endl;
                    return 1;
                }
            }
        }
        else
        {
            return 1;
        }
    }
    catch (TCLAP::ArgException &e)
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    }
}

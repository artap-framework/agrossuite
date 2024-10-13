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

#include "util/util.h"
#include "generator.h"

int main(int argc, char *argv[])
{
    Agros2DGenerator app(argc, argv);

    // command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("Agros generator");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption moduleOption(QStringList() << "m" << "module", "Generate module.", "module");
    parser.addOption(moduleOption);

    // Process the actual command line arguments given by the user
    parser.process(app);


    // init lists
    initLists();

    QTimer::singleShot(0, &app, SLOT(run()));

    if (parser.isSet(moduleOption))
        app.setModuleName(parser.value(moduleOption));

    return app.exec();
}

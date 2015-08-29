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

#ifndef OPTILABANALYSES_H
#define OPTILABANALYSES_H

#include "util.h"

class AnalysesWidget : public QWidget
{
    Q_OBJECT

public:
    AnalysesWidget(QWidget *parent = 0);
    ~AnalysesWidget();

    QAction *actAnalyses;
};

#endif // OPTILABANALYSES_H

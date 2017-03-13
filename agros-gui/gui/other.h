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

#ifndef OTHER_H
#define OTHER_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtSvg/QtSvg>

#include "util/util.h"
#include "../3rdparty/QtAwesome/QtAwesome.h"

class Computation;

// undo framework
AGROS_LIBRARY_API QUndoStack *undoStack();

// show page
AGROS_LIBRARY_API void showPage(const QString &str = "");

// get icon with respect to actual theme
AGROS_LIBRARY_API QIcon iconAwesome(int character);
AGROS_LIBRARY_API QIcon icon(const QString &name, const QString &defaultName = "");
enum AlphabetColor
{
    AlphabetColor_Blue,
    AlphabetColor_Bluegray,
    AlphabetColor_Brown,
    AlphabetColor_Green,
    AlphabetColor_Lightgray,
    AlphabetColor_Purple,
    AlphabetColor_Red,
    AlphabetColor_Yellow
};
// get color icon with letter
AGROS_LIBRARY_API QIcon iconAlphabet(const QChar &letter, AlphabetColor color);

// set gui style
AGROS_LIBRARY_API QString defaultGUIStyle();
AGROS_LIBRARY_API void setGUIStyle(const QString &styleName);

class SolveThread : public QThread
{
    Q_OBJECT

public:
    SolveThread(Computation *computation) : QThread(), m_computation(computation)
    {
        connect(this, SIGNAL(finished()), this, SLOT(finished()));
    }

    inline void startCalculation() { start(QThread::TimeCriticalPriority); }

protected:
    virtual void run();

private slots:
    void finished();

 private:
     Computation *m_computation;
};

#endif // OTHER_H

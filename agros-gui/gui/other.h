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

class Computation;

// undo framework
QUndoStack *undoStack();

// get icon with respect to actual theme
QIcon icon(const QString &name, const QString &defaultName = "");
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
QIcon iconAlphabet(const QChar &letter, AlphabetColor color);

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


class StringAction : public QAction
{
    Q_OBJECT

public:
    StringAction(QObject* parent, const QString &k, const QString &v) :
        QAction(v, parent),
        m_key(k), m_value(v)
    {
        connect(this, SIGNAL(triggered()), this, SLOT(doTriggered()));
    }

    inline QString key() const { return m_key; }
    inline QString value() const { return m_value; }

    public slots:
        void doTriggered()
    {
        emit triggered(m_key);
    }

    signals:
        void triggered(QString);

private:
    QString m_key;
    QString m_value;
};

#endif // OTHER_H

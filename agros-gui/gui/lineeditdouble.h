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

#ifndef GUI_LINEEDIT_H
#define GUI_LINEEDIT_H

#include "util/util.h"

#include <QtGui>
#include <QtWidgets>

class LineEditDouble : public QLineEdit
{
    Q_OBJECT
public:
    LineEditDouble(double val = 0, QWidget *parent = 0);
    ~LineEditDouble();

    inline void setBottom(double value) { if (m_validator) m_validator->setBottom(value); }
    inline void setTop(double value) { if (m_validator) m_validator->setTop(value); }
    inline double value() { return text().toDouble(); }
    inline void setValue(double value) { setText(QString::number(value)); }

protected:
    void keyPressEvent(QKeyEvent *event);

private:
    QDoubleValidator *m_validator;
};

#endif // GUI_LINEEDIT_H

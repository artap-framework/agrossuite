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

#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include "util/util.h"
#include "util/enums.h"
#include "gui/other.h"

class FieldInfo;

void readPixmap(QLabel *lblEquation, const QString &name);
QLabel *createLabel(const QString &label, const QString &toolTip = "");
void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit);
void fillComboBoxFonts(QComboBox *cmbFonts);
void fillComboBoxScalarVariable(CoordinateType coordinateType, FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);
void fillComboBoxContourVariable(CoordinateType coordinateType, FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);
void fillComboBoxVectorVariable(CoordinateType coordinateType, FieldInfo *fieldInfo, QComboBox *cmbFieldVariable);

inline QString htmlFontFamily() { return QApplication::font().family(); }
#ifdef Q_WS_WIN
    inline int htmlFontSize() { return QApplication::font().pointSize() + 4; }
#endif
#ifdef Q_WS_X11
    inline int htmlFontSize() { return QApplication::font().pointSize() + 1; }
#endif
#ifdef Q_WS_MAC
    inline int htmlFontSize() { return QApplication::font().pointSize() + 1; }
#endif

int columnMinimumWidth();

#endif // GUI_COMMON_H

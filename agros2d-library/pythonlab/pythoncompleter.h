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

#ifndef PYTHONCOMPLETER_H
#define PYTHONCOMPLETER_H

#include <QtGui>
#include <QtWidgets>

class PythonCompleterListView : public QListView
{
    Q_OBJECT
public:
    PythonCompleterListView(QWidget *parent = 0);
};

class PythonCompleterDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PythonCompleterDelegate(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

class PythonCompleter : public QCompleter
{
public:
    PythonCompleter();
    ~PythonCompleter();

private:
    PythonCompleterDelegate *m_delegate;
};

#endif // PYTHONCOMPLETER_H
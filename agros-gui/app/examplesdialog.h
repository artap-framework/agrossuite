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

#ifndef EXAMPLESDIALOG_H
#define EXAMPLESDIALOG_H

#include "util/util.h"
#include "gui/other.h"

class InfoWidget;

class ExamplesWidget : public QWidget
{
    Q_OBJECT

public:
    ExamplesWidget(QWidget *parent);
    ~ExamplesWidget();

    QAction *actExamples;

signals:
    void problemOpen(const QString &fileName);
    void formOpen(const QString &fileName, const QString &formName);

private slots:
    void doExampleItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doExampleItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    InfoWidget *infoWidget;

    QTreeWidget *trvExamples;
    QString m_selectedRecentFilename;
    QString m_selectedRecentFormFilename;
    QString m_selectedExampleFilename;
    QString m_selectedExampleFormFilename;

    QString m_expandedGroup;

    void createActions();

    void readTree();
    int readTutorials(QDir dir, QTreeWidgetItem *parentItem);

    void problemInfo(const QString &fileName);
    QList<QIcon> problemIcons(const QString &fileName);
};

#endif // EXAMPLESDIALOG_H

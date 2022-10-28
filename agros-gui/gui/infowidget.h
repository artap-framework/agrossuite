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

#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include "app/sceneview_common.h"

class SceneViewPreprocessor;

class InfoWidgetGeneral : public QWidget
{
    Q_OBJECT

public:
    InfoWidgetGeneral(QWidget *parent = 0);
    ~InfoWidgetGeneral();

protected:    
    QTextEdit *webEdit;

public slots:
    void clear();
    void showProblemInfo(ProblemBase *problem, const QString &name = "");
};

class InfoWidget : public InfoWidgetGeneral
{
    Q_OBJECT

public:
    InfoWidget(QWidget *parent = 0);
    ~InfoWidget();

signals:
    void open(const QString &fileName);
    void examples(const QString &groupName);

public slots:
    void welcome();

};

#endif // SCENEINFOVIEW_H

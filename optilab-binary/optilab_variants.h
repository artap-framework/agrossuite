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

#ifndef OPTILABVARIANTS_H
#define OPTILABVARIANTS_H

#include "util.h"

class VariantsWidget : public QWidget
{
    Q_OBJECT

public:
    VariantsWidget(QWidget *parent = 0);
    ~VariantsWidget();

    QAction *actRefresh;
    QAction *actVariants;

private slots:
    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void refreshVariants();
    void loadVariant(const QString &fileName);

    void variantOpenInExternal();
    void variantSolve();

    void processOpenError(QProcess::ProcessError error);
    void processOpenFinished(int exitCode);
    void processSolveError(QProcess::ProcessError error);
    void processSolveFinished(int exitCode);

private:
    QPushButton *btnSolve;
    QPushButton *btnOpenInExternal;

    QSplitter *splitter;

    QTreeWidget *trvVariants;
    QLabel *lblProblems;

    QWebView *webView;
    QString m_cascadeStyleSheet;
};

#endif // OPTILABVARIANTS_H

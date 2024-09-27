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

#include "common.h"

#include "util/util.h"
#include "util/global.h"
#include "gui/other.h"

#include "solver/module.h"

#include "solver/solver.h"
#include "solver/field.h"
#include "solver/problem.h"
#include "solver/solutionstore.h"

#include <QNetworkAccessManager>

void readPixmap(QLabel *lblEquation, const QString &name)
{
    QPixmap pixmap;
    pixmap.load(name);
    lblEquation->setPixmap(pixmap);
    lblEquation->setMaximumSize(pixmap.size());
    lblEquation->setMinimumSize(pixmap.size());
}

QLabel *createLabel(const QString &label, const QString &toolTip)
{
    QLabel *lblEquation = new QLabel(label + ":");
    lblEquation->setToolTip(toolTip);
    lblEquation->setMinimumWidth(100);
    return lblEquation;
}

void addTreeWidgetItemValue(QTreeWidgetItem *parent, const QString &name, const QString &text, const QString &unit)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    item->setText(1, text);
    item->setTextAlignment(1, Qt::AlignRight);
    item->setText(2, QString("%1 ").arg(unit));
    item->setTextAlignment(2, Qt::AlignLeft);
}

// column minimum width
int columnMinimumWidth()
{
    if (QGuiApplication::primaryScreen()->availableGeometry().width() == 1024)
        return 70;
    else
        return 110;
}

void fillComboBoxScalarVariable(CoordinateType coordinateType, FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewScalarVariables(coordinateType))
        cmbFieldVariable->addItem(variable.name(),
                                  variable.id());

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxContourVariable(CoordinateType coordinateType, FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    // if (!Agros::problem()->isSolved())
    //     return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewScalarVariables(coordinateType))
        cmbFieldVariable->addItem(variable.name(),
                                  variable.id());


    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

void fillComboBoxVectorVariable(CoordinateType coordinateType, FieldInfo *fieldInfo, QComboBox *cmbFieldVariable)
{
    // if (!Agros::problem()->isSolved())
    //     return;

    // store variable
    QString physicFieldVariable = cmbFieldVariable->itemData(cmbFieldVariable->currentIndex()).toString();

    // clear combo
    cmbFieldVariable->blockSignals(true);
    cmbFieldVariable->clear();
    foreach (Module::LocalVariable variable, fieldInfo->viewVectorVariables(coordinateType))
        cmbFieldVariable->addItem(variable.name(),
                                  variable.id());

    cmbFieldVariable->setCurrentIndex(cmbFieldVariable->findData(physicFieldVariable));
    if (cmbFieldVariable->currentIndex() == -1)
        cmbFieldVariable->setCurrentIndex(0);
    cmbFieldVariable->blockSignals(false);
}

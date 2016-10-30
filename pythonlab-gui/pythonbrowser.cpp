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

#include "pythonengine.h"
#include "pythonconsole.h"
#include "pythonbrowser.h"

#include "other/other.h"

const int namePos = 0;
const int typePos = 1;
const int valuePos = 2;

bool isPythonVariable(const QString& type)
{
    if (type == "int" || type == "float" || type == "string" || type == "bool" ||
            type == "list" || type == "dict" || type == "tuple" ||
            type == "instance" || type == "numpy.ndarray")
        return true;
    else
        return false;
}

PythonBrowser::PythonBrowser(PythonScriptingConsole *console, QWidget *parent)
    : QWidget(parent), console(console)
{
    setObjectName("BrowserView");

    trvBrowser = new QTreeWidget(this);
    trvBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    trvBrowser->setColumnCount(3);
    trvBrowser->setColumnWidth(0, 200);
    trvBrowser->setColumnWidth(0, 200);

    QStringList headers;
    headers << tr("Name") << tr("Type") << tr("Value");
    trvBrowser->setHeaderLabels(headers);

    trvVariables = NULL;
    trvFunctions = NULL;
    trvClasses = NULL;
    trvOther = NULL;

    variableExpanded = true;
    functionExpanded = true;
    classExpanded = true;
    otherExpanded = false;

    // executed();
    trvBrowser->sortItems(0, Qt::AscendingOrder);

    connect(trvBrowser, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(executeCommand(QTreeWidgetItem *, int)));
    connect(trvBrowser, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(doContextMenu(const QPoint &)));

    actCopyName = new QAction(tr("Copy name"), this);
    connect(actCopyName, SIGNAL(triggered()), this, SLOT(copyName()));

    actCopyValue = new QAction(tr("Copy value"), this);
    connect(actCopyValue, SIGNAL(triggered()), this, SLOT(copyValue()));

    actDelete = new QAction(tr("&Delete"), this);
    connect(actDelete, SIGNAL(triggered()), this, SLOT(deleteVariable()));

    mnuContext = new QMenu(trvBrowser);
    mnuContext->addAction(actCopyName);
    mnuContext->addAction(actCopyValue);
    mnuContext->addSeparator();
    mnuContext->addAction(actDelete);

    connect(currentPythonEngine(), SIGNAL(executedScript()), this, SLOT(executed()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(trvBrowser);

    setLayout(layout);
}

void PythonBrowser::doContextMenu(const QPoint &point)
{
    actDelete->setEnabled(false);
    actCopyName->setEnabled(false);
    actCopyValue->setEnabled(false);

    QTreeWidgetItem *item = trvBrowser->itemAt(point);
    if (item && isPythonVariable(item->text(typePos)))
    {
        actDelete->setEnabled(true);
        actCopyName->setEnabled(true);
        actCopyValue->setEnabled(true);
    }
    mnuContext->exec(QCursor::pos());
}

void PythonBrowser::executed()
{
    trvBrowser->setUpdatesEnabled(false);
    trvBrowser->setSortingEnabled(false);

    if (trvVariables) variableExpanded = trvVariables->isExpanded();
    if (trvFunctions) functionExpanded = trvFunctions->isExpanded();
    if (trvClasses) classExpanded = trvClasses->isExpanded();
    if (trvOther) otherExpanded = trvOther->isExpanded();

    trvBrowser->clear();

    trvVariables = new QTreeWidgetItem(trvBrowser);
    trvVariables->setText(0, tr("Variables"));
    trvVariables->setIcon(0, icon("browser-variable"));
    trvVariables->setExpanded(variableExpanded);
    trvOther = new QTreeWidgetItem(trvBrowser);
    trvOther->setText(0, tr("Other"));
    trvOther->setIcon(0, icon("browser-other"));
    trvOther->setExpanded(otherExpanded);
    trvFunctions = new QTreeWidgetItem(trvBrowser);
    trvFunctions->setText(0, tr("Functions"));
    trvFunctions->setIcon(0, icon("browser-function"));
    trvFunctions->setExpanded(functionExpanded);
    trvClasses = new QTreeWidgetItem(trvBrowser);
    trvClasses->setText(0, tr("Classes"));
    trvClasses->setIcon(0, icon("browser-class"));
    trvClasses->setExpanded(classExpanded);

    QList<PythonVariable> list = currentPythonEngine()->variableList();

    foreach (PythonVariable variable, list)
    {
        QTreeWidgetItem *item = NULL;
        if (variable.type == "bool")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-bool"));
        }
        else if (variable.type == "int")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, QString::number(variable.value.toInt()));
            item->setIcon(0, icon("browser-variable-int"));
        }
        else if (variable.type == "float")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, QString::number(variable.value.toDouble()));
            item->setIcon(0, icon("browser-variable-float"));
        }
        else if (variable.type == "string")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-string"));
        }
        else if (variable.type == "list")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-list"));
        }
        else if (variable.type == "dict")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-dict"));
        }
        else if (variable.type == "tuple")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-tuple"));
        }
        else if (variable.type == "instance")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-class"));
        }
        else if (variable.type == "numpy.ndarray")
        {
            item = new QTreeWidgetItem(trvVariables);
            item->setText(2, variable.value.toString());
            item->setIcon(0, icon("browser-variable-array"));
        }
        else if (variable.type == "function")
        {
            item = new QTreeWidgetItem(trvFunctions);
            item->setIcon(0, icon("browser-variable-float"));
        }
        else if (variable.type == "classobj")
        {
            item = new QTreeWidgetItem(trvClasses);
        }
        else if (variable.type == "module")
        {
            item = new QTreeWidgetItem(trvOther);
            item->setText(2, variable.value.toString());
        }
        else
        {
            item = new QTreeWidgetItem(trvOther);
        }

        item->setText(0, variable.name);
        item->setText(1, variable.type);

        // qDebug() << variable.type << ": " << variable.name;
    }

    trvBrowser->setSortingEnabled(true);
    trvBrowser->setUpdatesEnabled(true);
}

void PythonBrowser::executeCommand(QTreeWidgetItem *item, int role)
{
    if (item && !item->text(typePos).isEmpty())
    {
        QString command = item->text(namePos);

        console->clearCommandLine();
        console->insertPlainText(command);
        console->executeLine(command, false);
    }
}

void PythonBrowser::copyName()
{
    if (trvBrowser->currentItem())
        QApplication::clipboard()->setText(trvBrowser->currentItem()->text(namePos));
}

void PythonBrowser::copyValue()
{
    if (trvBrowser->currentItem())
        QApplication::clipboard()->setText(trvBrowser->currentItem()->text(valuePos));
}

void PythonBrowser::deleteVariable()
{
    if (trvBrowser->currentItem() && isPythonVariable(trvBrowser->currentItem()->text(typePos)))
    {
        QString variable = trvBrowser->currentItem()->text(namePos);

        console->clearCommandLine();
        console->insertPlainText("del " + variable);
        console->executeLine("del " + variable, false);
    }
}
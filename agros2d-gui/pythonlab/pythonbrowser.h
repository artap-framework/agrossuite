#ifndef PYTHONVARIABLES_H
#define PYTHONVARIABLES_H

#include "util/util.h"

class PythonScriptingConsole;

bool isPythonVariable(const QString& type);

class PythonBrowser : public QWidget
{
    Q_OBJECT
public:
    PythonBrowser(PythonScriptingConsole *console, QWidget *parent = 0);

private slots:
    void executed();
    void executeCommand(QTreeWidgetItem *item, int role);
    void doContextMenu(const QPoint &point);
    void copyName();
    void copyValue();
    void deleteVariable();

private:
    QTreeWidget *trvBrowser;
    QTreeWidgetItem *trvVariables;
    QTreeWidgetItem *trvFunctions;
    QTreeWidgetItem *trvClasses;
    QTreeWidgetItem *trvOther;

    PythonScriptingConsole *console;

    bool variableExpanded;
    bool functionExpanded;
    bool classExpanded;
    bool otherExpanded;

    QAction *actCopyValue;
    QAction *actCopyName;
    QAction *actDelete;

    QMenu *mnuContext;
};

#endif // PYTHONVARIABLES_H

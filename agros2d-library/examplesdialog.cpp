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

#include "examplesdialog.h"

#include "util/constants.h"
#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
#include "sceneedge.h"

#include "gui/lineeditdouble.h"
#include "gui/common.h"
#include "gui/infowidget.h"

#include "solver/problem.h"
#include "solver/problem_config.h"

#include "solver/module.h"

#include "../3rdparty/quazip/JlCompress.h"
#include "../resources_source/classes/problem_a2d_31_xml.h"

ExamplesWidget::ExamplesWidget(QWidget *parent, InfoWidget *infoWidget)
    : QWidget(parent), m_infoWidget(infoWidget)
{
    m_selectedFilename = "";
    m_selectedFormFilename = "";
    m_expandedGroup = "";

    createActions();

    trvProblems = new QTreeWidget(this);
    trvProblems->setMouseTracking(true);
    trvProblems->setColumnCount(1);
    trvProblems->setIconSize(QSize(24, 24));
    trvProblems->setHeaderHidden(true);

    connect(trvProblems, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvProblems, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    // connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addWidget(trvProblems);

    QGroupBox *widget = new QGroupBox(tr("Examples"));
    widget->setLayout(layoutSurface);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(toolBar);
    layout->addWidget(widget, 1);

    setLayout(layout);

    showWidget("Examples");
}

void ExamplesWidget::createActions()
{
    actExamples = new QAction(icon("agros2d"), tr("Welcome"), this);
    actExamples->setShortcut(tr("Ctrl+1"));
    actExamples->setCheckable(true);

    toolBar = new QToolBar(this);
    // toolBar->addSeparator();

    QAction *actWelcome = new QAction(icon("agros2d"), tr("Welcome screen"), this);
    connect(actWelcome, SIGNAL(triggered(bool)), m_infoWidget, SLOT(showWelcome()));
    toolBar->addAction(actWelcome);
}

void ExamplesWidget::showWidget(const QString &expandedGroup)
{
    m_expandedGroup = expandedGroup;

    readProblems();

    QList<QTreeWidgetItem *> items = trvProblems->findItems(m_expandedGroup, Qt::MatchExactly);
    if (items.count() == 1)
        trvProblems->setCurrentItem(items.at(0));
}

void ExamplesWidget::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    m_infoWidget->clear();

    m_selectedFilename.clear();
    if (current)
    {
        m_selectedFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedFilename.isEmpty())
        {
            problemInfo(m_selectedFilename);
        }
    }
}

void ExamplesWidget::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvProblems->currentItem())
    {        
        if (!m_selectedFilename.isEmpty())
        {
            emit problemOpen(m_selectedFilename);
        }
    }
}

/*
void ExamplesDialog::linkClicked(const QUrl &url)
{
    QString search = "/open?";
    if (url.toString().contains(search))
    {
#if QT_VERSION < 0x050000
        QString fileName = url.queryItemValue("filename");
        QString form = url.queryItemValue("form");
#else
        QString fileName = QUrlQuery(url).queryItemValue("filename");
        QString form = QUrlQuery(url).queryItemValue("form");
#endif

        m_selectedFilename = QUrl(fileName).toLocalFile();
        m_selectedFormFilename = QUrl(form).toLocalFile();

        accept();
    }
}
*/

void ExamplesWidget::readProblems()
{
    // clear listview
    trvProblems->clear();

    QDir dir(QString("%1/resources/examples").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readProblems(dir, trvProblems->invisibleRootItem());
}

int ExamplesWidget::readProblems(QDir dir, QTreeWidgetItem *parentItem)
{
    int count = 0;

    QFileInfoList listExamples = dir.entryInfoList();
    for (int i = 0; i < listExamples.size(); ++i)
    {
        QFileInfo fileInfo = listExamples.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir())
        {
            QFont fnt = trvProblems->font();
            fnt.setBold(true);

            QTreeWidgetItem *dirItem = new QTreeWidgetItem(parentItem);
            dirItem->setText(0, fileInfo.fileName());
            dirItem->setFont(0, fnt);
            QString fn = QString("%1.png").arg(fileInfo.absoluteFilePath());
            if (QFile::exists(fn))
                dirItem->setIcon(0, QIcon(fn));
            // expand group
            if (m_expandedGroup.isEmpty()
                    || (parentItem != trvProblems->invisibleRootItem())
                    || ((m_expandedGroup == fileInfo.fileName()) && parentItem == trvProblems->invisibleRootItem()))
                dirItem->setExpanded(true);

            // recursive read
            int numberOfProblems = readProblems(fileInfo.absoluteFilePath(), dirItem);

            if (numberOfProblems == 0)
            {
                // remove dir from tree
                parentItem->removeChild(dirItem);
            }

            // increase counter
            count += numberOfProblems;
        }
        else if (fileInfo.suffix() == "ags" || fileInfo.suffix() == "a2d")
        {
            QList<QIcon> icons = problemIcons(fileInfo.absoluteFilePath());

            QTreeWidgetItem *exampleProblemItem = new QTreeWidgetItem(parentItem);
            if (icons.count() == 1)
                exampleProblemItem->setIcon(0, icons.at(0));
            else
                exampleProblemItem->setIcon(0, icon("fields/empty"));
            exampleProblemItem->setText(0, fileInfo.baseName());
            if (fileInfo.suffix() == "a2d")
                exampleProblemItem->setTextColor(0, QColor(Qt::blue)); // TODO: only marker - remove
            exampleProblemItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            count++;
        }
        else if (fileInfo.suffix() == "py")
        {
            // skip ui python
            if (QFile::exists(fileInfo.absoluteFilePath().left(fileInfo.absoluteFilePath().length() - 3) + ".ui"))
                continue;

            // skip problem.py
            if (fileInfo.baseName() == "problem")
                continue;

            QTreeWidgetItem *examplePythonItem = new QTreeWidgetItem(parentItem);
            examplePythonItem->setIcon(0, icon("script-python"));
            examplePythonItem->setText(0, fileInfo.baseName());
            examplePythonItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            count++;
        }
        else if (fileInfo.suffix() == "ui")
        {
            QTreeWidgetItem *exampleFormItem = new QTreeWidgetItem(parentItem);
            exampleFormItem->setIcon(0, icon("options-main"));
            exampleFormItem->setText(0, fileInfo.baseName());
            exampleFormItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            count++;
        }
    }

    return count;
}

void ExamplesWidget::problemInfo(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        QFileInfo fileInfo(fileName);

        if (fileInfo.suffix() == "ags")
        {
            // extract problem.a2d file to temp
            QStringList lst;
            lst << "problem.a2d";
            JlCompress::extractFiles(fileInfo.absoluteFilePath(), lst, tempProblemDir());

            QString fn = QString("%1/%2").arg(tempProblemDir()).arg("problem.a2d");
            this->problemInfo(fn);
            QFile::remove(fn);
            return;
        }
        else if (fileInfo.suffix() == "a2d")
        {
            QSharedPointer<Computation> computation = QSharedPointer<Computation>(new Computation(""));
            computation->readProblemFromA2D31(fileName);
            m_infoWidget->showProblemInfo(computation.data());

            // details
            /*
            QString detailsFilename(QString("%1/%2/index.html").arg(fileInfo.absolutePath()).arg(fileInfo.baseName()));
            if (QFile::exists(detailsFilename))
            {
                // replace current path in index.html
                QString detail = readFileContent(detailsFilename);
                detail = detail.replace("{{DIR}}", QString("%1/%2").arg(QUrl::fromLocalFile(fileInfo.absolutePath()).toString()).arg(fileInfo.baseName()));
                detail = detail.replace("{{RESOURCES}}", QUrl::fromLocalFile(QString("%1/resources/").arg(QDir(datadir()).absolutePath())).toString());

                problemInfo.SetValue("PROBLEM_DETAILS", detail.toStdString());
            }
            */
        }
        else if (fileInfo.suffix() == "py")
        {
            m_infoWidget->showPythonInfo(fileName);
        }
        else if (fileInfo.suffix() == "ui")
        {
            // templateName = "example_form.tpl";
        }
    }
}

QList<QIcon> ExamplesWidget::problemIcons(const QString &fileName)
{
    QList<QIcon> icons;

    if (QFile::exists(fileName))
    {
        QString tmpFile = fileName;

        if (QFileInfo(fileName).suffix() == "ags")
        {
            // extract problem.a2d file to temp
            QStringList lst;
            lst << "problem.a2d";
            JlCompress::extractFiles(QFileInfo(fileName).absoluteFilePath(), lst, tempProblemDir());

            tmpFile = QString("%1/%2").arg(tempProblemDir()).arg("problem.a2d");
        }

        // open file
        QFile file(tmpFile);

        QDomDocument doc;
        if (!doc.setContent(&file))
        {
            file.close();
            throw AgrosException(tr("File '%1' is not valid Agros2D file.").arg(fileName));
            return icons;
        }
        file.close();

        // main document
        QDomElement eleDoc = doc.documentElement();

        // problem info
        QDomNode eleProblemInfo = eleDoc.elementsByTagName("problem").at(0);

        QDomNode eleFields = eleProblemInfo.toElement().elementsByTagName("fields").at(0);
        QDomNode nodeField = eleFields.firstChild();
        while (!nodeField.isNull())
        {
            QDomNode eleField = nodeField.toElement();
            icons.append(icon("fields/" + eleField.toElement().attribute("field_id")));

            // next field
            nodeField = nodeField.nextSibling();
        }
    }

    return icons;
}

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

#include "pythonlab/pythonengine_agros.h"

#include "../3rdparty/quazip/JlCompress.h"
#include "../resources_source/classes/problem_a2d_31_xml.h"

ExamplesWidget::ExamplesWidget(QWidget *parent, InfoWidget *infoWidget)
    : QWidget(parent), m_infoWidget(infoWidget)
{
    m_selectedRecentFilename = "";
    m_selectedRecentFormFilename = "";
    m_selectedExampleFilename = "";
    m_selectedExampleFormFilename = "";
    m_expandedGroup = "";

    createActions();

    trvRecentFiles = new QTreeWidget(this);
    trvRecentFiles->setMouseTracking(true);
    trvRecentFiles->setColumnCount(1);
    trvRecentFiles->setIconSize(QSize(24, 24));
    trvRecentFiles->setHeaderHidden(true);

    connect(trvRecentFiles, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doRecentItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvRecentFiles, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doRecentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QHBoxLayout *layoutRecentFiles = new QHBoxLayout();
    layoutRecentFiles->addWidget(trvRecentFiles);

    QGroupBox *widgetRecentFiles = new QGroupBox(tr("Recent files"));
    widgetRecentFiles->setLayout(layoutRecentFiles);

    trvExamples = new QTreeWidget(this);
    trvExamples->setMouseTracking(true);
    trvExamples->setColumnCount(1);
    trvExamples->setIconSize(QSize(24, 24));
    trvExamples->setHeaderHidden(true);

    connect(trvExamples, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doExampleItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvExamples, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doExampleItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QHBoxLayout *layoutExamples = new QHBoxLayout();
    layoutExamples->addWidget(trvExamples);

    QGroupBox *widgetExamples = new QGroupBox(tr("Examples"));
    widgetExamples->setLayout(layoutExamples);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widgetRecentFiles, 1);
    layout->addWidget(widgetExamples, 2);

    setLayout(layout);

    connect(Agros2D::problem()->scene(), SIGNAL(cleared()), this, SLOT(readRecentFiles()));
    connect(currentPythonEngineAgros(), SIGNAL(executedScript()), this, SLOT(readRecentFiles()));

    init("Examples");
}

void ExamplesWidget::createActions()
{
    actExamples = new QAction(icon("agros2d"), tr("Welcome"), this);
    actExamples->setShortcut(tr("Ctrl+1"));
    actExamples->setCheckable(true);
}

void ExamplesWidget::init(const QString &expandedGroup)
{
    m_expandedGroup = expandedGroup;

    readRecentFiles();
    readExamples();

    if (!trvExamples->currentItem())
    {
        QList<QTreeWidgetItem *> items = trvExamples->findItems(m_expandedGroup, Qt::MatchExactly);
        if (items.count() == 1)
            trvExamples->setCurrentItem(items.at(0));
    }
}

void ExamplesWidget::doRecentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    m_selectedRecentFilename.clear();
    if (current)
    {
        m_selectedRecentFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedRecentFilename.isEmpty())
        {
            problemInfo(m_selectedRecentFilename);
            return;
        }
    }

    m_infoWidget->welcome();
}

void ExamplesWidget::doRecentItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvRecentFiles->currentItem())
    {
        if (!m_selectedRecentFilename.isEmpty())
        {
            emit problemOpen(m_selectedRecentFilename);
        }
    }
}

void ExamplesWidget::doExampleItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    m_selectedExampleFilename.clear();
    if (current)
    {
        m_selectedExampleFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedExampleFilename.isEmpty())
        {
            problemInfo(m_selectedExampleFilename);
            return;
        }
    }

    m_infoWidget->welcome();
}

void ExamplesWidget::doExampleItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvExamples->currentItem())
    {
        if (!m_selectedExampleFilename.isEmpty())
        {
            emit problemOpen(m_selectedExampleFilename);
        }
    }
}

void ExamplesWidget::readRecentFiles()
{
    QSettings settings;

    trvRecentFiles->clear();

    QFont fnt = trvRecentFiles->font();
    fnt.setBold(true);

    QTreeWidgetItem *trvRecentProblems = new QTreeWidgetItem(trvRecentFiles);
    trvRecentProblems->setText(0, tr("Recent problems"));
    trvRecentProblems->setFont(0, fnt);
    trvRecentProblems->setExpanded(true);

    QStringList recentProblems = settings.value("RecentProblems").value<QStringList>();
    for (int i = 0; i < qMin(5, recentProblems.count()); i++)
    {
        QFileInfo fileInfo(recentProblems[i]);
        if (!QFile::exists(fileInfo.absoluteFilePath()))
            continue;

        QList<QIcon> icons = problemIcons(fileInfo.absoluteFilePath());

        QTreeWidgetItem *item = new QTreeWidgetItem(trvRecentProblems);
        item->setText(0, fileInfo.baseName());
        item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
        if (icons.count() == 1)
            item->setIcon(0, icons.at(0));
        else
            item->setIcon(0, icon("fields/empty"));
    }

    QTreeWidgetItem *trvRecentScripts = new QTreeWidgetItem(trvRecentFiles);
    trvRecentScripts->setText(0, tr("Recent scripts"));
    trvRecentScripts->setFont(0, fnt);
    trvRecentScripts->setExpanded(true);

    QStringList recentScripts = settings.value("RecentScripts").value<QStringList>();
    for (int i = 0; i < qMin(5, recentScripts.count()); i++)
    {
        QFileInfo fileInfo(recentScripts[i]);
        if (!QFile::exists(fileInfo.absoluteFilePath()))
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem(trvRecentScripts);
        item->setText(0, fileInfo.baseName());
        item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
        item->setIcon(0, icon("pythonlab"));
    }

    m_infoWidget->welcome();
}

void ExamplesWidget::readExamples()
{
    // clear listview
    trvExamples->clear();

    QDir dir(QString("%1/resources/examples").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readExamples(dir, trvExamples->invisibleRootItem());
}

int ExamplesWidget::readExamples(QDir dir, QTreeWidgetItem *parentItem)
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
            QFont fnt = trvExamples->font();
            fnt.setBold(true);

            QTreeWidgetItem *dirItem = new QTreeWidgetItem(parentItem);
            dirItem->setText(0, fileInfo.fileName());
            dirItem->setFont(0, fnt);
            QString fn = QString("%1.png").arg(fileInfo.absoluteFilePath());
            if (QFile::exists(fn))
                dirItem->setIcon(0, QIcon(fn));
            // expand group
            if (m_expandedGroup.isEmpty()
                    || (parentItem != trvExamples->invisibleRootItem())
                    || ((m_expandedGroup == fileInfo.fileName()) && parentItem == trvExamples->invisibleRootItem()))
                dirItem->setExpanded(true);

            // recursive read
            int numberOfProblems = readExamples(fileInfo.absoluteFilePath(), dirItem);

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
            QStringList files = JlCompress::getFileList(fileInfo.absoluteFilePath());
            // extract problem file to temp
            QString problemName = "";
            if (files.contains("problem.json"))
                problemName = "problem.json";
            else
                problemName = "problem.a2d";

            JlCompress::extractFiles(fileInfo.absoluteFilePath(), QStringList() << problemName, tempProblemDir());

            QString fn = QString("%1/%2").arg(tempProblemDir()).arg(problemName);
            this->problemInfo(fn);
            QFile::remove(fn);

            return;
        }
        else if (fileInfo.suffix() == "json")
        {
            QSharedPointer<Computation> computation = QSharedPointer<Computation>(new Computation());
            computation->readProblemFromJson(fileName);
            m_infoWidget->showProblemInfo(computation.data());

            return;
        }
        else if (fileInfo.suffix() == "a2d")
        {
            QSharedPointer<Computation> computation = QSharedPointer<Computation>(new Computation());
            computation->importProblemFromA2D(fileName);
            m_infoWidget->showProblemInfo(computation.data());

            return;
        }
        else if (fileInfo.suffix() == "py")
        {
            m_infoWidget->showPythonInfo(fileName);

            return;
        }
        else if (fileInfo.suffix() == "ui")
        {
            // templateName = "example_form.tpl";
        }

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
    else
    {
        m_infoWidget->welcome();
    }
}

QList<QIcon> ExamplesWidget::problemIcons(const QString &fileName)
{
    QList<QIcon> icons;

    if (QFile::exists(fileName))
    {
        if (QFileInfo(fileName).suffix() == "ags")
        {
            // extract problem file to temp
            QStringList files = JlCompress::getFileList(fileName);
            // extract problem file to temp
            QString problemName = "";
            if (files.contains("problem.json"))
            {
                problemName = "problem.json";

                JlCompress::extractFiles(QFileInfo(fileName).absoluteFilePath(), QStringList() << problemName, tempProblemDir());
                QString tmpFile = QString("%1/%2").arg(tempProblemDir()).arg(problemName);

                // open file
                QFile file(tmpFile);

                // json
                if (!file.exists())
                    return icons;

                if (!file.open(QIODevice::ReadOnly))
                    return icons;

                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                QJsonObject rootJson = doc.object();

                // fields
                QJsonArray fieldsJson = rootJson["fields"].toArray();
                for (int i = 0; i < fieldsJson.size(); i++)
                {
                    QJsonObject fieldJson = fieldsJson[i].toObject();
                    icons.append(icon("fields/" + fieldJson["fieldid"].toString()));
                }
            }
        }
    }

    return icons;
}

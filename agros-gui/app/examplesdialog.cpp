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

ExamplesWidget::ExamplesWidget(QWidget *parent)
    : QWidget(parent), infoWidget(infoWidget)
{
    m_selectedRecentFilename = "";
    m_selectedRecentFormFilename = "";
    m_selectedExampleFilename = "";
    m_selectedExampleFormFilename = "";
    m_expandedGroup = "";

    createActions();

    trvExamples = new QTreeWidget(this);
    trvExamples->setMinimumWidth(350);
    trvExamples->setMouseTracking(true);
    trvExamples->setColumnCount(1);
    trvExamples->setIconSize(QSize(24, 24));
    trvExamples->setHeaderHidden(true);

    connect(trvExamples, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doExampleItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvExamples, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doExampleItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    // scene - info widget
    infoWidget = new InfoWidget(this);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(trvExamples);
    layout->addWidget(infoWidget);
    layout->setStretch(1, 1);

    setLayout(layout);

    readTree();

    infoWidget->welcome();
}

ExamplesWidget::~ExamplesWidget()
{
}

void ExamplesWidget::createActions()
{
    actExamples = new QAction(icon("main_welcome"), tr("Welcome"), this);
    actExamples->setShortcut(Qt::Key_F1);
    actExamples->setCheckable(true);
}

void ExamplesWidget::doExampleItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    m_selectedExampleFilename.clear();
    if (current)
    {
        m_selectedExampleFilename = current->data(0, Qt::UserRole).toString();

        if (m_selectedExampleFilename == "welcome")
        {
            infoWidget->welcome();
        }
        else if (!m_selectedExampleFilename.isEmpty())
        {
            problemInfo(m_selectedExampleFilename);
        }
        else
        {
            infoWidget->empty();
        }
    }
    else
    {
        infoWidget->empty();
    }
}

void ExamplesWidget::doExampleItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvExamples->currentItem())
    {
        if (!m_selectedExampleFilename.isEmpty())
        {
            Q_EMIT problemOpen(m_selectedExampleFilename);
        }
    }
}

void ExamplesWidget::readTree()
{
    // clear listview
    trvExamples->clear();

    QSettings settings;

    QFont fnt = trvExamples->font();
    fnt.setBold(true);

    // welcome
    QTreeWidgetItem *trvWelcome = new QTreeWidgetItem(trvExamples);
    trvWelcome->setText(0, tr("Welcome"));
    trvWelcome->setIcon(0, icon("main_welcome_inverted"));
    trvWelcome->setData(0, Qt::UserRole, "welcome");
    trvWelcome->setFont(0, fnt);
    trvWelcome->setExpanded(true);

    // read recent
    QTreeWidgetItem *trvRecentProblems = new QTreeWidgetItem(trvExamples);
    trvRecentProblems->setText(0, tr("Recent problems"));
    trvRecentProblems->setIcon(0, icon("recent"));
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

    // read tutorials
    QDir dir(QString("%1/resources/examples").arg(Agros::dataDir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readTutorials(dir, trvExamples->invisibleRootItem());

    infoWidget->welcome();
}

int ExamplesWidget::readTutorials(QDir dir, QTreeWidgetItem *parentItem)
{
    QFont fnt = trvExamples->font();
    fnt.setBold(true);

    // read tutorials
    int countTutorials = 0;

    QFileInfoList listExamples = dir.entryInfoList();
    for (int i = 0; i < listExamples.size(); ++i)
    {
        QFileInfo fileInfo = listExamples.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if (fileInfo.isDir())
        {
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
            int numberOfProblems = readTutorials(fileInfo.absoluteFilePath(), dirItem);

            if (numberOfProblems == 0)
            {
                // remove dir from tree
                parentItem->removeChild(dirItem);
                delete dirItem;
            }

            // increase counter
            countTutorials += numberOfProblems;
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
                exampleProblemItem->setForeground(0, QBrush(QColor(Qt::blue))); // TODO: only marker - remove
            exampleProblemItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            countTutorials++;
        }
        else if (fileInfo.suffix() == "ui")
        {
            QTreeWidgetItem *exampleFormItem = new QTreeWidgetItem(parentItem);
            exampleFormItem->setIcon(0, icon("options-main"));
            exampleFormItem->setText(0, fileInfo.baseName());
            exampleFormItem->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());

            // increase counter
            countTutorials++;
        }
    }

    return countTutorials;
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

            // show info
            QSharedPointer<Problem> problem = QSharedPointer<Problem>(new Problem());
            problem->readProblemFromJson(fn);
            infoWidget->showProblemInfo(problem.data(), fileInfo.baseName());

            QFile::remove(fn);

            return;
        }
        else if (fileInfo.suffix() == "json")
        {
            // show info
            QSharedPointer<Problem> problem = QSharedPointer<Problem>(new Problem());
            problem->readProblemFromJson(fileName);
            infoWidget->showProblemInfo(problem.data());

            return;
        }
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
                    icons.append(icon("fields/" + fieldJson["fieldid"].toString(), "fields/empty"));
                }
            }
        }
    }

    return icons;
}

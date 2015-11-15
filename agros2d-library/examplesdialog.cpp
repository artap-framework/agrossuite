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

ExamplesDialog::ExamplesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Examples"));
    setModal(true);

    m_selectedFilename = "";
    m_selectedFormFilename = "";
    m_expandedGroup = "";

    lstProblems = new QTreeWidget(this);
    lstProblems->setMouseTracking(true);
    lstProblems->setColumnCount(1);
    lstProblems->setIndentation(15);
    lstProblems->setIconSize(QSize(24, 24));
    lstProblems->setHeaderHidden(true);
    lstProblems->setMinimumWidth(320);

    connect(lstProblems, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(lstProblems, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    m_infoWidget = new InfoWidgetGeneral(this);
    // connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addWidget(lstProblems);
    layoutSurface->addWidget(m_infoWidget, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    int w = 3.0/4.0 * QApplication::desktop()->screenGeometry().width();
    int h = 3.0/4.0 * QApplication::desktop()->screenGeometry().height();

    setMinimumSize(w, h);
    setMaximumSize(w, h);

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

ExamplesDialog::~ExamplesDialog()
{
}

void ExamplesDialog::doAccept()
{
    accept();
}

void ExamplesDialog::doReject()
{
    reject();
}

int ExamplesDialog::showDialog(const QString &expandedGroup)
{
    m_expandedGroup = expandedGroup;

    readProblems();

    QList<QTreeWidgetItem *> items = lstProblems->findItems(m_expandedGroup, Qt::MatchExactly);
    if (items.count() == 1)
        lstProblems->setCurrentItem(items.at(0));

    return exec();
}

void ExamplesDialog::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    m_infoWidget->clear();

    if (current)
    {
        m_selectedFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedFilename.isEmpty())
        {
            problemInfo(m_selectedFilename);
            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        }
    }
}

void ExamplesDialog::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (lstProblems->currentItem())
    {
        if (!lstProblems->currentItem()->data(0, Qt::UserRole).toString().isEmpty())
            accept();
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

void ExamplesDialog::readProblems()
{
    // clear listview
    lstProblems->clear();

    QDir dir(QString("%1/resources/examples").arg(datadir()));
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);

    readProblems(dir, lstProblems->invisibleRootItem());
}

int ExamplesDialog::readProblems(QDir dir, QTreeWidgetItem *parentItem)
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
            QFont fnt = lstProblems->font();
            fnt.setBold(true);

            QTreeWidgetItem *dirItem = new QTreeWidgetItem(parentItem);
            dirItem->setText(0, fileInfo.fileName());
            dirItem->setFont(0, fnt);
            // expand group
            if (m_expandedGroup.isEmpty()
                    || (parentItem != lstProblems->invisibleRootItem())
                    || ((m_expandedGroup == fileInfo.fileName()) && parentItem == lstProblems->invisibleRootItem()))
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

void ExamplesDialog::problemInfo(const QString &fileName)
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

QList<QIcon> ExamplesDialog::problemIcons(const QString &fileName)
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

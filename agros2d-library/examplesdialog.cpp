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

#include "solver/problem.h"
#include "solver/problem_config.h"

#include "solver/module.h"

#include "ctemplate/template.h"

#include "../3rdparty/quazip/JlCompress.h"
#include "../resources_source/classes/problem_a2d_31_xml.h"

ExamplesDialog::ExamplesDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Examples"));
    setModal(true);

    m_selectedFilename = "";
    m_selectedFormFilename = "";
    m_expandedGroup = "";

    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    lstProblems = new QTreeWidget(this);
    lstProblems->setMouseTracking(true);
    lstProblems->setColumnCount(1);
    lstProblems->setIndentation(15);
    lstProblems->setIconSize(QSize(24, 24));
    lstProblems->setHeaderHidden(true);
    lstProblems->setMinimumWidth(320);

    connect(lstProblems, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(lstProblems, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addWidget(lstProblems);
    layoutSurface->addWidget(webView, 1);

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
    webView->setHtml("");

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

        // template
        std::string info;
        ctemplate::TemplateDictionary problemInfo("info");

        // problem info
        problemInfo.SetValue("AGROS2D", "file:///" + compatibleFilename(QDir(datadir() + TEMPLATEROOT + "/panels/agros2d_logo.png").absolutePath()).toStdString());

        problemInfo.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
        problemInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(datadir()).absolutePath()).arg(TEMPLATEROOT + "/panels")).toString().toStdString());
        problemInfo.SetValue("BASIC_INFORMATION_LABEL", tr("Basic informations").toStdString());

        problemInfo.SetValue("NAME_LABEL", tr("Name:").toStdString());
        problemInfo.SetValue("NAME", fileInfo.baseName().toStdString());

        QString templateName;

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
            templateName = "example_problem.tpl";

            try
            {
                std::unique_ptr<XMLProblem::document> document_xsd = XMLProblem::document_(compatibleFilename(fileName).toStdString(), xml_schema::flags::dont_validate);
                XMLProblem::document *doc = document_xsd.get();

                problemInfo.SetValue("COORDINATE_TYPE_LABEL", tr("Coordinate type:").toStdString());
                problemInfo.SetValue("COORDINATE_TYPE", coordinateTypeString(coordinateTypeFromStringKey(QString::fromStdString(doc->problem().coordinate_type()))).toStdString());

                // geometry
                QString geometry = problemSvgGeometry(&doc->geometry());

                problemInfo.SetValue("GEOMETRY_LABEL", tr("Geometry").toStdString());
                problemInfo.SetValue("GEOMETRY_NODES_LABEL", tr("Nodes:").toStdString());
                problemInfo.SetValue("GEOMETRY_NODES", QString::number(doc->geometry().nodes().node().size()).toStdString());
                problemInfo.SetValue("GEOMETRY_EDGES_LABEL", tr("Edges:").toStdString());
                problemInfo.SetValue("GEOMETRY_EDGES", QString::number(doc->geometry().edges().edge().size()).toStdString());
                problemInfo.SetValue("GEOMETRY_LABELS_LABEL", tr("Labels:").toStdString());
                problemInfo.SetValue("GEOMETRY_LABELS", QString::number(doc->geometry().labels().label().size()).toStdString());
                problemInfo.SetValue("GEOMETRY_SVG", geometry.toStdString());

                problemInfo.SetValue("PHYSICAL_FIELD_MAIN_LABEL", tr("Physical fields").toStdString());

                // fields
                for (unsigned int i = 0; i < doc->problem().fields().field().size(); i++)
                {
                    XMLProblem::field field = doc->problem().fields().field().at(i);

                    ctemplate::TemplateDictionary *fieldInfo = problemInfo.AddSectionDictionary("FIELD_SECTION");

                    fieldInfo->SetValue("PHYSICAL_FIELD_LABEL", Module::availableModules()[QString::fromStdString(field.field_id())].toStdString());

                    // fieldInfo->SetValue("ANALYSIS_TYPE_LABEL", tr("Analysis:").toStdString());
                    // fieldInfo->SetValue("ANALYSIS_TYPE", analysisTypeString(analysisTypeFromStringKey(QString::fromStdString(field.analysis_type()))).toStdString());

                    fieldInfo->SetValue("LINEARITY_TYPE_LABEL", tr("Solver:").toStdString());
                    fieldInfo->SetValue("LINEARITY_TYPE", linearityTypeString(linearityTypeFromStringKey(QString::fromStdString(field.linearity_type()))).toStdString());

                    problemInfo.ShowSection("FIELD");
                }
            }
            catch (...)
            {
                qDebug() << "Unknown exception catched in ExampleDialog";
            }
        }
        else if (fileInfo.suffix() == "py")
        {
            templateName = "example_python.tpl";

            // python
            if (QFile::exists(fileName))
            {
                // replace current path in index.html
                QString python = readFileContent(fileName   );
                problemInfo.SetValue("PROBLEM_PYTHON", python.toStdString());
            }
        }
        else if (fileInfo.suffix() == "ui")
        {
            templateName = "example_form.tpl";
        }

        // details
        QString detailsFilename(QString("%1/%2/index.html").arg(fileInfo.absolutePath()).arg(fileInfo.baseName()));
        if (QFile::exists(detailsFilename))
        {
            // replace current path in index.html
            QString detail = readFileContent(detailsFilename);
            detail = detail.replace("{{DIR}}", QString("%1/%2").arg(QUrl::fromLocalFile(fileInfo.absolutePath()).toString()).arg(fileInfo.baseName()));
            detail = detail.replace("{{RESOURCES}}", QUrl::fromLocalFile(QString("%1/resources/").arg(QDir(datadir()).absolutePath())).toString());

            problemInfo.SetValue("PROBLEM_DETAILS", detail.toStdString());
        }

        ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/" + templateName.toStdString(), ctemplate::DO_NOT_STRIP, &problemInfo, &info);

        // setHtml(...) doesn't work
        // webView->setHtml(QString::fromStdString(info));

        // load(...) works
        writeStringContent(tempProblemDir() + "/example.html", QString::fromStdString(info));
        webView->load(QUrl::fromLocalFile(tempProblemDir() + "/example.html"));
    }
}

QString ExamplesDialog::problemSvgGeometry(XMLProblem::geometry *geometry)
{
    // SVG
    QString str;

    Point min( numeric_limits<double>::max(),  numeric_limits<double>::max());
    Point max(-numeric_limits<double>::max(), -numeric_limits<double>::max());

    // nodes
    QList<Point> points;
    for (unsigned int i = 0; i < geometry->nodes().node().size(); i++)
    {
        XMLProblem::node node = geometry->nodes().node().at(i);
        Point p(node.x(), node.y());
        points.append(p);

        min.x = qMin(min.x, p.x);
        max.x = qMax(max.x, p.x);
        min.y = qMin(min.y, p.y);
        max.y = qMax(max.y, p.y);
    }

    // bounding box
    RectPoint boundingBox(min, max);

    double size = 180;
    double stroke_width = qMax(boundingBox.width(), boundingBox.height()) / size / 2.0;

    str += QString("<svg width=\"%1px\" height=\"%2px\" viewBox=\"%3 %4 %5 %6\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\">\n").
            arg(size).
            arg(size).
            arg(boundingBox.start.x).
            arg(0).
            arg(boundingBox.width()).
            arg(boundingBox.height());

    str += QString("<g stroke=\"black\" stroke-width=\"%1\" fill=\"none\">\n").arg(stroke_width);

    // edges
    for (unsigned int i = 0; i < geometry->edges().edge().size(); i++)
    {
        XMLProblem::edge edge = geometry->edges().edge().at(i);

        Point start = points[edge.start()];
        Point end = points[edge.end()];

        if (edge.angle() > 0.0)
        {
            Point center = centerPoint(start, end, edge.angle());
            double radius = (center - start).magnitude();
            double startAngle = atan2(center.y - start.y, center.x - start.x) / M_PI*180.0 - 180.0;

            int segments = edge.angle() / 5.0;
            if (segments < 2) segments = 2;
            double theta = edge.angle() / double(segments - 1);

            for (int i = 0; i < segments-1; i++)
            {
                double arc1 = (startAngle + i*theta)/180.0*M_PI;
                double arc2 = (startAngle + (i+1)*theta)/180.0*M_PI;

                double x1 = radius * fastcos(arc1);
                double y1 = radius * fastsin(arc1);
                double x2 = radius * fastcos(arc2);
                double y2 = radius * fastsin(arc2);

                str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                        arg(center.x + x1).
                        arg(boundingBox.end.y - (center.y + y1)).
                        arg(center.x + x2).
                        arg(boundingBox.end.y - (center.y + y2));
            }
        }
        else
        {
            str += QString("<line x1=\"%1\" y1=\"%2\" x2=\"%3\" y2=\"%4\" />\n").
                    arg(start.x).
                    arg(boundingBox.end.y - start.y).
                    arg(end.x).
                    arg(boundingBox.end.y - end.y);
        }
    }

    str += "</g>\n";
    str += "</svg>\n";

    return str;
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

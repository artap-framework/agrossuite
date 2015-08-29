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

#include "optilab_variants.h"

#include <QJsonDocument>

#include "gui/common.h"
#include "util/constants.h"

#include "ctemplate/template.h"

VariantsWidget::VariantsWidget(QWidget *parent) : QWidget(parent)
{
    actRefresh = new QAction(icon("reload"), tr("Refresh"), this);
    // actDocumentSave->setShortcuts(QKeySequence::Save);
    connect(actRefresh, SIGNAL(triggered()), this, SLOT(refreshVariants()));

    actVariants = new QAction(icon("document-properties"), tr("Variants"), this);
    actVariants->setShortcut(tr("Ctrl+1"));
    actVariants->setCheckable(true);

    // problem information
    webView = new QWebView();
    webView->page()->setNetworkAccessManager(new QNetworkAccessManager());
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    // connect(webView->page(), SIGNAL(linkClicked(QUrl)), this, SLOT(linkClicked(QUrl)));

    // stylesheet
    std::string style;
    ctemplate::TemplateDictionary stylesheet("style");
    stylesheet.SetValue("FONTFAMILY", htmlFontFamily().toStdString());
    stylesheet.SetValue("FONTSIZE", (QString("%1").arg(htmlFontSize()).toStdString()));

    ctemplate::ExpandTemplate(compatibleFilename(datadir() + TEMPLATEROOT + "/panels/style_common.css").toStdString(), ctemplate::DO_NOT_STRIP, &stylesheet, &style);
    m_cascadeStyleSheet = QString::fromStdString(style);

    trvVariants = new QTreeWidget(this);
    trvVariants->setMouseTracking(true);
    trvVariants->setColumnCount(2);
    trvVariants->setIndentation(15);
    trvVariants->setIconSize(QSize(16, 16));
    trvVariants->setHeaderHidden(true);
    trvVariants->setMinimumWidth(320);
    trvVariants->setColumnWidth(0, 150);

    connect(trvVariants, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvVariants, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    // connect(trvVariants, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), optilabSingle, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    btnOpenInExternal = new QPushButton(tr("Open in external Agros2D"));
    btnOpenInExternal->setToolTip(tr("Open in external Agros2D"));
    btnOpenInExternal->setEnabled(false);
    connect(btnOpenInExternal, SIGNAL(clicked()), this, SLOT(variantOpenInExternal()));

    btnSolve = new QPushButton(tr("Solve in Agros2D"));
    btnSolve->setToolTip(tr("Solve in Agros2D"));
    btnSolve->setEnabled(false);
    connect(btnSolve, SIGNAL(clicked()), this, SLOT(variantSolve()));

    QPushButton *btnRefresh = new QPushButton(tr("Refresh"));
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refreshVariants()));

    QGridLayout *layoutButtons = new QGridLayout();
    layoutButtons->addWidget(btnRefresh, 0, 0);
    layoutButtons->addWidget(btnOpenInExternal, 0, 1);
    layoutButtons->addWidget(btnSolve, 0, 2);

    lblProblems = new QLabel("");

    QVBoxLayout *layoutControls = new QVBoxLayout();
    layoutControls->addWidget(trvVariants);
    layoutControls->addLayout(layoutButtons);
    layoutControls->addWidget(lblProblems);

    QWidget *controls = new QWidget(this);
    controls->setLayout(layoutControls);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);
    splitter->addWidget(controls);
    splitter->addWidget(webView);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 6);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    QList<int> sizes;
    sizes << 230 << 0;
    splitter->setSizes(sizes);

    QVBoxLayout *layoutSM = new QVBoxLayout();
    layoutSM->addWidget(splitter);

    setLayout(layoutSM);

    QSettings settings;
    splitter->restoreState(settings.value("VariantsWidget/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("VariantsWidget/SplitterGeometry").toByteArray());
}

VariantsWidget::~VariantsWidget()
{
    QSettings settings;
    settings.setValue("VariantsWidget/SplitterState", splitter->saveState());
    settings.setValue("VariantsWidget/SplitterGeometry", splitter->saveGeometry());
}

void VariantsWidget::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    btnSolve->setEnabled(current);
    btnOpenInExternal->setEnabled(current);

    if (current)
        loadVariant(current->data(0, Qt::UserRole).toString());
}

void VariantsWidget::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item)
        variantOpenInExternal();
}


void VariantsWidget::refreshVariants()
{
    trvVariants->setUpdatesEnabled(false);

    // save current item
    QString selectedItem;
    if (trvVariants->currentItem())
        selectedItem = trvVariants->currentItem()->data(0, Qt::UserRole).toString();

    QTime time;
    time.start();

    // clear listview
    trvVariants->clear();

    int count = 0;
    int countSolved = 0;

    QString dir = QString("%1/analyses").arg(cacheProblemDir());
    QDirIterator analysis(dir, QDir::NoDotAndDotDot | QDir::Dirs, QDirIterator::NoIteratorFlags);

    while (analysis.hasNext())
    {
        QString dirName = analysis.next();
        QTreeWidgetItem *analysisItem = new QTreeWidgetItem(trvVariants->invisibleRootItem());
        analysisItem->setText(0, QFileInfo(dirName).baseName());

        QDirIterator variant(dirName, QStringList() << "*.data", QDir::Files, QDirIterator::NoIteratorFlags);
        while (variant.hasNext())
        {
            QString fileName = variant.next();
            bool isSolved = false;

            QTreeWidgetItem *variantItem = new QTreeWidgetItem();
            if (isSolved)
                variantItem->setIcon(0, icon("browser-other"));
            else
                variantItem->setIcon(0, icon("browser-class"));
            variantItem->setText(0, QFileInfo(fileName).baseName());
            variantItem->setText(1, QFileInfo(fileName).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            variantItem->setData(0, Qt::UserRole, fileName);

            // increase counter
            count++;
            if (isSolved)
                countSolved++;

            analysisItem->addChild(variantItem);
        }
    }

    trvVariants->setUpdatesEnabled(true);

    // select first
    if (selectedItem.isEmpty() && trvVariants->topLevelItemCount() > 0)
        selectedItem = trvVariants->topLevelItem(0)->data(0, Qt::UserRole).toString();

    if (!selectedItem.isEmpty())
    {
        for (int i = 0; i < trvVariants->topLevelItemCount(); i++ )
        {
            QTreeWidgetItem *item = trvVariants->topLevelItem(i);

            if (selectedItem == item->data(0, Qt::UserRole).toString())
            {
                item->setSelected(true);
                trvVariants->setCurrentItem(item);
                // ensure visible
                trvVariants->scrollToItem(item);
            }
        }
    }

    lblProblems->setText(tr("Solutions: %1/%2").arg(countSolved).arg(count));

    qDebug() << "refresh" << time.elapsed();
}

void VariantsWidget::loadVariant(const QString &fileName)
{
    qDebug() << "loadVariant" << fileName;

    /*
    import machinery
    loader = machinery.SourceFileLoader('problem', '{0}/problem.py'.format(temp))
    loader.load_module().{0}'.format(Capcitor)
    */

    // template
    std::string output;
    ctemplate::TemplateDictionary variant("variant");

    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString content = file.readAll();
    file.close();

    QJsonDocument sd = QJsonDocument::fromJson(content.toUtf8());
    QJsonObject obj = sd.object();

    QJsonObject parameters = obj.value("parameters").toObject();
    QJsonObject variables = obj.value("variables").toObject();
    QJsonObject info = obj.value("info").toObject();

    variant.SetValue("STYLESHEET", m_cascadeStyleSheet.toStdString());
    variant.SetValue("NAME", QFileInfo(fileName).baseName().toStdString());
    if (!parameters.isEmpty())
        variant.SetValue("PARAMETER_LABEL", tr("Parameters:").toStdString());
    if (!variables.isEmpty())
        variant.SetValue("VARIABLE_LABEL", tr("Variables:").toStdString());
    if (!info.isEmpty())
        variant.SetValue("INFO_LABEL", tr("Informations:").toStdString());

    for (QJsonObject::Iterator iter = parameters.begin(); iter != parameters.end (); ++iter)
    {
        QJsonValue val = iter.value();

        if (val.isDouble())
        {
            ctemplate::TemplateDictionary *parameter = variant.AddSectionDictionary("PARAMETER_SECTION");
            parameter->SetValue("PARAMETER_LABEL", iter.key().toStdString());
            parameter->SetValue("PARAMETER_VALUE", QString::number(val.toDouble()).toStdString());
        }
    }

    for (QJsonObject::Iterator iter = variables.begin(); iter != variables.end (); ++iter)
    {
        QJsonValue val = iter.value();

        if (val.isDouble())
        {
            ctemplate::TemplateDictionary *variable = variant.AddSectionDictionary("VARIABLE_SECTION");
            variable->SetValue("VARIABLE_LABEL", iter.key().toStdString());
            variable->SetValue("VARIABLE_VALUE", QString::number(val.toDouble()).toStdString());
        }
    }

    for (QJsonObject::Iterator iter = info.begin(); iter != info.end (); ++iter)
    {
        QJsonValue val = iter.value();

        if (val.isDouble())
        {
            ctemplate::TemplateDictionary *info = variant.AddSectionDictionary("INFO_SECTION");
            info->SetValue("INFO_LABEL", iter.key().toStdString());
            info->SetValue("INFO_VALUE", QString::number(val.toDouble()).toStdString());
        }
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/optilab_variant.tpl", ctemplate::DO_NOT_STRIP, &variant, &output);
    writeStringContent(tempProblemDir() + "/variant.html", QString::fromStdString(output));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/variant.html"));


    /*
    QString fn = datadir() + "/data/pokus.a2d";
    Agros2D::scene()->readFromFile(fn);

    Agros2D::log()->printMessage(tr("Problem"), tr("Problem '%1' successfuly loaded").arg(fn));

    // solve
    Agros2D::problem()->solve();

    Agros2D::scene()->writeSolutionToFile(fn);
    */

    // Agros2D::log()->printMessage(tr("Solver"), tr("Problem was solved in %1").arg(milisecondsToTime(time.elapsed()).toString("mm:ss.zzz")));
}


void VariantsWidget::processOpenError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D");
}

void VariantsWidget::processOpenFinished(int exitCode)
{
    if (exitCode == 0)
    {
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        qDebug() << "Agros2D";
        qDebug() << errorMessage;
    }
}

void VariantsWidget::variantOpenInExternal()
{
    /*
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        // TODO: template?
        QString str;
        str += "from variant import model\n";
        str += QString("import sys; sys.path.insert(0, '%1')\n").arg(m_problemDir);
        str += "import problem\n";
        str += "p = problem.Model()\n";
        str += QString("p.load('%1')\n").arg(fileName);
        str += "p.create()\n";

        QString id = QUuid::createUuid().toString().remove("{").remove("}");
        QString tempFN = QString("%1/%2.py").arg(tempProblemDir()).arg(id);

        writeStringContent(tempFN, str);

        // run agros2d
        QProcess *process = new QProcess();
        process->setStandardOutputFile(tempProblemDir() + "/agros2d.out");
        process->setStandardErrorFile(tempProblemDir() + "/agros2d.err");
        connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processOpenError(QProcess::ProcessError)));
        connect(process, SIGNAL(finished(int)), this, SLOT(processOpenFinished(int)));

        QString command = QString("\"%1/agros2d\" -x -s \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(tempFN);

        process->start(command);
    }
    */
}

void VariantsWidget::variantSolve()
{
    /*
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        QString str = QString("variant.optilab_interface._solve_in_agros2d('%1')").arg(fileName);
        // qDebug() << str;

        currentPythonEngine()->runScript(str);
        refreshVariants();
    }
    */
}

void VariantsWidget::processSolveError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D Solver");
}

void VariantsWidget::processSolveFinished(int exitCode)
{
    if (exitCode == 0)
    {
        QApplication::processEvents();

        /*
        int index = trvVariants->currentItem()->data(0, Qt::UserRole).toInt();
        QDomNode nodeResultOld = docXML.elementsByTagName("results").at(0).childNodes().at(index);

        QString fileName = QString("%1/models/%2").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

        QDomNode nodeResultNew = readVariant(fileName);
        nodeResultOld.parentNode().appendChild(nodeResultNew);
        nodeResultOld.parentNode().replaceChild(nodeResultOld, nodeResultNew);
        */

        refreshVariants();
    }
    else
    {
        QString errorMessage = readFileContent(tempProblemDir() + "/solver.err");
        errorMessage.insert(0, "\n");
        errorMessage.append("\n");
        qDebug() << "Agros2D";
        qDebug() << errorMessage;
    }
}

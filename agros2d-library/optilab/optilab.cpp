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

#include "optilab.h"

#include "util/constants.h"
#include "gui/lineeditdouble.h"
#include "gui/common.h"
#include "gui/systemoutput.h"
#include "gui/about.h"

#include <fstream>
#include <string>

#include "pythonlab/pythonengine_agros.h"

#include "ctemplate/template.h"

#include <QJsonDocument>

OptilabWindow::OptilabWindow() : QMainWindow()
{
    setWindowTitle(tr("Optilab"));

    createActions();
    createToolBars();
    createMain();

    QSettings settings;
    splitter->restoreState(settings.value("OptilabWindow/SplitterState").toByteArray());
    splitter->restoreGeometry(settings.value("OptilabWindow/SplitterGeometry").toByteArray());
    restoreGeometry(settings.value("OptilabWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("OptilabWindow/State", saveState()).toByteArray());
}

OptilabWindow::~OptilabWindow()
{
    QSettings settings;
    settings.setValue("OptilabWindow/Geometry", saveGeometry());
    settings.setValue("OptilabWindow/State", saveState());
    settings.setValue("OptilabWindow/SplitterState", splitter->saveState());
    settings.setValue("OptilabWindow/SplitterGeometry", splitter->saveGeometry());

    removeDirectory(tempProblemDir());
}

void OptilabWindow::processOpenError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D");
}

void OptilabWindow::processOpenFinished(int exitCode)
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

void OptilabWindow::variantOpenInExternalAgros2D()
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

void OptilabWindow::variantOpenInAgros2D()
{
    /*
    QString fileName = QString("%1/models/%2.pickle").arg(m_problemDir).arg(trvVariants->currentItem()->data(0, Qt::UserRole).toString());

    if (QFile::exists(fileName))
    {
        QString str = QString("variant.optilab_interface._open_in_agros2d('%1')").arg(fileName);
        // qDebug() << str;

        currentPythonEngine()->runScript(str);
    }
    */
}

void OptilabWindow::variantSolveInExternalSolver()
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
        str += "p.solve()\n";
        str += "p.process()\n";
        str += QString("p.save('%1')\n").arg(fileName);

        QString command = QString("\"%1/agros2d_solver\" -l -c \"%2\"").
                arg(QApplication::applicationDirPath()).
                arg(str);

        SystemOutputWidget *systemOutput = new SystemOutputWidget(this);
        connect(systemOutput, SIGNAL(finished(int)), this, SLOT(processSolveFinished(int)));
        systemOutput->execute(command);
    }
    */
}

void OptilabWindow::variantSolveInAgros2D()
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

void OptilabWindow::processSolveError(QProcess::ProcessError error)
{
    qDebug() << tr("Could not start Agros2D Solver");
}

void OptilabWindow::processSolveFinished(int exitCode)
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

void OptilabWindow::createActions()
{
    actRefresh = new QAction(icon("reload"), tr("Refresh"), this);
    // actDocumentSave->setShortcuts(QKeySequence::Save);
    connect(actRefresh, SIGNAL(triggered()), this, SLOT(refreshVariants()));
}

void OptilabWindow::createToolBars()
{
    // top toolbar
#ifdef Q_WS_MAC
    int iconHeight = 24;
#endif

    QToolBar *tlbTools = addToolBar(tr("Tools"));
    tlbTools->setObjectName("Tools");
    tlbTools->setOrientation(Qt::Horizontal);
    tlbTools->setAllowedAreas(Qt::TopToolBarArea);
    tlbTools->setMovable(false);
#ifdef Q_WS_MAC
    tlbTools->setFixedHeight(iconHeight);
    tlbTools->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbTools->addAction(actRefresh);
    tlbTools->addSeparator();
}

void OptilabWindow::createMain()
{
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

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Vertical);
    splitter->addWidget(webView);
    // splitter->addWidget(optilabMulti);

    QVBoxLayout *layoutSM = new QVBoxLayout();
    layoutSM->addWidget(splitter);

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

    btnOpenInExternalAgros2D = new QPushButton(tr("Open in external Agros2D"));
    btnOpenInExternalAgros2D->setToolTip(tr("Open in external Agros2D"));
    btnOpenInExternalAgros2D->setEnabled(false);
    connect(btnOpenInExternalAgros2D, SIGNAL(clicked()), this, SLOT(variantOpenInExternalAgros2D()));

    btnSolveInExternalSolver = new QPushButton(tr("Solve in external solver"));
    btnSolveInExternalSolver->setToolTip(tr("Solve in external solver"));
    btnSolveInExternalSolver->setEnabled(false);
    connect(btnSolveInExternalSolver, SIGNAL(clicked()), this, SLOT(variantSolveInExternalSolver()));

    btnOpenInAgros2D = new QPushButton(tr("Open in Agros2D"));
    btnOpenInAgros2D->setToolTip(tr("Open in Agros2D"));
    btnOpenInAgros2D->setEnabled(false);
    connect(btnOpenInAgros2D, SIGNAL(clicked()), this, SLOT(variantOpenInAgros2D()));

    btnSolveInAgros2D = new QPushButton(tr("Solve in Agros2D"));
    btnSolveInAgros2D->setToolTip(tr("Solve in Agros2D"));
    btnSolveInAgros2D->setEnabled(false);
    connect(btnSolveInAgros2D, SIGNAL(clicked()), this, SLOT(variantSolveInAgros2D()));

    QGridLayout *layoutButtons = new QGridLayout();
    layoutButtons->addWidget(btnOpenInExternalAgros2D, 0, 0);
    layoutButtons->addWidget(btnSolveInExternalSolver, 1, 0);
    layoutButtons->addWidget(btnOpenInAgros2D, 0, 1);
    layoutButtons->addWidget(btnSolveInAgros2D, 1, 1);

    lblProblems = new QLabel("");

    QVBoxLayout *layoutLeft = new QVBoxLayout();
    layoutLeft->addWidget(trvVariants);
    layoutLeft->addLayout(layoutButtons);
    layoutLeft->addWidget(lblProblems);

    QHBoxLayout *layoutRight = new QHBoxLayout();
    // layoutRight->addWidget(tbxAnalysis);
    layoutRight->addLayout(layoutSM);
    //layoutRight->addWidget(console);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addLayout(layoutLeft);
    layout->addLayout(layoutRight, 1);

    QWidget *main = new QWidget();
    main->setLayout(layout);

    setCentralWidget(main);
}

void OptilabWindow::openProblemAgros2D()
{   
    QProcess process;

    /*
    QString command = QString("\"%1/agros2d\" -s \"%2\"").
            arg(QApplication::applicationDirPath()).
            arg(m_problem);
    process.startDetached(command);
    */
}

void OptilabWindow::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{    
    btnOpenInAgros2D->setEnabled(current);
    btnSolveInAgros2D->setEnabled(current);
    btnOpenInExternalAgros2D->setEnabled(current);
    btnSolveInExternalSolver->setEnabled(current);

    loadVariant(current->data(0, Qt::UserRole).toString());
}

void OptilabWindow::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item)
        variantOpenInAgros2D();
}

void OptilabWindow::showDialog()
{
    show();
    activateWindow();

    refreshVariants();
}

void OptilabWindow::refreshVariants()
{
    // TODO: more analyses !!
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

    QString dir = QString("%1/../analyses/1439554800").arg(cacheProblemDir());
    QDirIterator it(dir, QStringList() << "*.data", QDir::Files, QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        QString fileName = it.next();

        bool isSolved = false;

        QTreeWidgetItem *variantItem = new QTreeWidgetItem(trvVariants->invisibleRootItem());
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

void OptilabWindow::loadVariant(const QString &fileName)
{
    qDebug() << "loadVariant" << fileName;

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

    // TODO: fill template
    for (QJsonObject::Iterator iter = parameters.begin(); iter != parameters.end (); ++iter)
    {
        QJsonValue val = iter.value();

        if (val.isDouble())
        {
            qDebug() << iter.key() << val.toDouble();
        }
    }

    ctemplate::ExpandTemplate(datadir().toStdString() + TEMPLATEROOT.toStdString() + "/panels/optilab_variant.tpl", ctemplate::DO_NOT_STRIP, &variant, &output);

    // setHtml(...) doesn't work
    // webView->setHtml(QString::fromStdString(info));

    // load(...) works
    writeStringContent(tempProblemDir() + "/variant.html", QString::fromStdString(output));
    webView->load(QUrl::fromLocalFile(tempProblemDir() + "/variant.html"));
}

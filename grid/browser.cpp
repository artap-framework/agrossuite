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

#include "browser.h"
#include "gui/about.h"

#include "ssh/sshconnectionmanager.h"
#include "ssh/sshremoteprocess.h"
#include "ssh/sftpchannel.h"

BrowserWindow::BrowserWindow(int argc, char *argv[], QWidget *parent)
    : QMainWindow(parent)
{
    setWindowIcon(icon("condor"));

    lblSSHState = new QLabel(tr("Disconnected"), statusBar());

    trvNodes = new QTreeWidget(this);
    trvNodes->setMouseTracking(true);
    trvNodes->setColumnCount(8);
    trvNodes->setIndentation(15);
    trvNodes->setIconSize(QSize(16, 16));
    trvNodes->setHeaderHidden(false);
    trvNodes->setMinimumWidth(320);
    trvNodes->setColumnWidth(0, 200);
    trvNodes->setColumnWidth(1, 50);
    trvNodes->setColumnWidth(3, 200);

    QStringList headersNodes;
    headersNodes << tr("Machine") << tr("Slot") << tr("Arch") << tr("OS") << tr("Memory") << tr("State") << tr("Activity") << tr("Avg. load");
    trvNodes->setHeaderLabels(headersNodes);

    trvHistory = new QTreeWidget(this);
    trvHistory->setMouseTracking(true);
    trvHistory->setColumnCount(8);
    trvHistory->setIndentation(15);
    trvHistory->setIconSize(QSize(16, 16));
    trvHistory->setHeaderHidden(false);
    trvHistory->setMinimumWidth(320);
    trvHistory->setColumnWidth(0, 60);
    trvHistory->setColumnWidth(2, 150);
    trvHistory->setColumnWidth(3, 160);
    trvHistory->setColumnWidth(4, 160);
    trvHistory->setColumnWidth(5, 120);

    QStringList headersHistory;
    headersHistory << tr("ID") << tr("Owner") << tr("Machine") << tr("Start date") << tr("Completion date") << tr("Running time") << tr("State") << tr("Command");
    trvHistory->setHeaderLabels(headersHistory);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(trvNodes);
    layout->addWidget(trvHistory);

    QWidget *main = new QWidget();
    main->setLayout(layout);

    setCentralWidget(main);

    createActions();
    createToolBars();
    createMenus();

    //parseStatus("/home/karban/status.xml");
    //parseHistory("/home/karban/history.xml");

    QSettings settings;
    restoreState(settings.value("BrowserWindow/NodesHeaders", trvNodes->header()->saveState()).toByteArray());
    restoreGeometry(settings.value("BrowserWindow/Geometry", saveGeometry()).toByteArray());
    restoreState(settings.value("BrowserWindow/State", saveState()).toByteArray());
}

BrowserWindow::~BrowserWindow()
{
    QSettings settings;
    settings.setValue("BrowserWindow/Geometry", saveGeometry());
    settings.setValue("BrowserWindow/State", saveState());
    settings.setValue("BrowserWindow/NodesHeaders", trvNodes->header()->saveState());

    QSsh::releaseConnection(m_connection);
}

void BrowserWindow::createActions()
{
    actAbout = new QAction(icon("about"), tr("About Grid Browser"), this);
    actAbout->setMenuRole(QAction::AboutRole);
    connect(actAbout, SIGNAL(triggered()), this, SLOT(doAbout()));

    actAboutQt = new QAction(icon("help-about"), tr("About &Qt"), this);
    actAboutQt->setMenuRole(QAction::AboutQtRole);
    connect(actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    actConnect = new QAction(icon("connect"), tr("Connect"), this);
    connect(actConnect, SIGNAL(triggered()), this, SLOT(connectToHost()));

    actRefresh = new QAction(icon("reload"), tr("Read status"), this);
    actRefresh->setEnabled(false);
    connect(actRefresh, SIGNAL(triggered()), this, SLOT(refresh()));

    actExit = new QAction(icon("application-exit"), tr("E&xit"), this);
    actExit->setShortcut(tr("Ctrl+Q"));
    actExit->setMenuRole(QAction::QuitRole);
    connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

    actOptions = new QAction(icon("options"), tr("&Options"), this);
    actOptions->setMenuRole(QAction::PreferencesRole);
    connect(actOptions, SIGNAL(triggered()), this, SLOT(showSettings()));
}

void BrowserWindow::createMenus()
{
    QMenu *mnuEdit = menuBar()->addMenu(tr("E&dit"));
#ifdef Q_WS_X11
    // mnuEdit->addSeparator();
    mnuEdit->addAction(actOptions);
#endif

    QMenu *mnuHelp = menuBar()->addMenu(tr("&Help"));
#ifndef Q_WS_MAC
    mnuHelp->addSeparator();
#else
    mnuHelp->addAction(actOptions); // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actExit);    // will be added to "Agros2D" MacOSX menu
#endif
    // mnuHelp->addSeparator();
    mnuHelp->addAction(actAbout);   // will be added to "Agros2D" MacOSX menu
    mnuHelp->addAction(actAboutQt); // will be added to "Agros2D" MacOSX menu
}

void BrowserWindow::createToolBars()
{
    tlbConnection = addToolBar(tr("File"));
    tlbConnection->setObjectName("File");
    tlbConnection->setOrientation(Qt::Horizontal);
    tlbConnection->setAllowedAreas(Qt::TopToolBarArea);
    tlbConnection->setMovable(false);
#ifdef Q_WS_MAC
    tlbFile->setFixedHeight(iconHeight);
    tlbFile->setStyleSheet("QToolButton { border: 0px; padding: 0px; margin: 0px; }");
#endif
    tlbConnection->addAction(actConnect);
    tlbConnection->addAction(actRefresh);
}

void BrowserWindow::connectToHost()
{
    QSettings settings;

    QSsh::SshConnectionParameters params;
    params.host = settings.value("SSH/Host").toString();
    params.port = settings.value("SSH/Port", 22).toInt();
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePublicKey;
    params.userName = settings.value("SSH/UserName").toString();
    params.privateKeyFile = settings.value("SSH/PrivateKeyPath").toString();
    params.timeout = 10;

    m_connection = QSsh::acquireConnection(params);
    connect(m_connection, SIGNAL(connected()), SLOT(handleConnected()));
    connect(m_connection, SIGNAL(disconnected()), SLOT(handleDisconnected()));
    connect(m_connection, SIGNAL(error(QSsh::SshError)), SLOT(handleConnectionError(QSsh::SshError)));
    connect(m_connection, SIGNAL(dataAvailable(QString)), SLOT(handleShellMessage(QString)));

    m_connection->connectToHost();
}

void BrowserWindow::refresh()
{
    fnRemote = QString("/tmp/grid_history_%1.xml").arg(qrand());
    fnLocal = QString("%1/_history.xml").arg(tempProblemDir());

    m_runState = RunState_History;
    m_fileState = FileState_History;

    runCommand(QString("condor_history -xml > %1").arg(fnRemote));
}

void BrowserWindow::runCommand(const QString &command)
{
    m_process = m_connection->createRemoteProcess(command.toLatin1());

    connect(m_process.data(), SIGNAL(started()), SLOT(handleShellStarted()));
    connect(m_process.data(), SIGNAL(readyReadStandardOutput()), SLOT(handleRemoteStdout()));
    connect(m_process.data(), SIGNAL(readyReadStandardError()), SLOT(handleRemoteStderr()));
    connect(m_process.data(), SIGNAL(closed(int)), SLOT(handleChannelClosed(int)));

    m_runState = RunState_Init;
    m_process->start();
}

void BrowserWindow::parseStatus(const QString &fileName)
{
    QFile *file = new QFile(fileName);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(file);

    GridNode *node = NULL;
    m_nodes.clear();

    while (!xml.atEnd())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::EndDocument)
            break;
        else if (token == QXmlStreamReader::EndElement)
            continue;

        // new node
        if (xml.name() == "c")
        {
            m_nodes.append(GridNode());
            node = &m_nodes.last();
        }

        if (node && (xml.name() == "a"))
        {
            if (xml.attributes().value("n") == "SlotID")
            {
                xml.readNext();
                node->slotID = xml.readElementText().toInt();
            }
            else if (xml.attributes().value("n") == "Machine")
            {
                xml.readNext();
                node->machine = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "Arch")
            {
                xml.readNext();
                node->arch = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "OpSys")
            {
                xml.readNext();
                node->opSys = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "OpSysLongName")
            {
                xml.readNext();
                node->opSysName = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "Activity")
            {
                xml.readNext();
                node->activity = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "State")
            {
                xml.readNext();
                node->state = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "Memory")
            {
                xml.readNext();
                node->memory = xml.readElementText().toInt();
            }
            else if (xml.attributes().value("n") == "LoadAvg")
            {
                xml.readNext();
                node->avgLoad = xml.readElementText().toInt();
            }
        }
    }

    qSort(m_nodes);

    trvNodes->setUpdatesEnabled(false);

    foreach (GridNode node, m_nodes)
    {
        QTreeWidgetItem *nodeItem = new QTreeWidgetItem(trvNodes->invisibleRootItem());
        nodeItem->setText(0, node.machine);
        nodeItem->setText(1, QString::number(node.slotID));
        nodeItem->setText(2, node.arch);
        nodeItem->setText(3, node.opSysName);
        nodeItem->setText(4, QString::number(node.memory));
        nodeItem->setText(5, node.state);
        nodeItem->setText(6, node.activity);
        nodeItem->setText(7, QString::number(node.avgLoad));
    }

    trvNodes->setUpdatesEnabled(true);
}

void BrowserWindow::parseHistory(const QString &fileName)
{
    QFile *file = new QFile(fileName);
    file->open(QIODevice::ReadOnly | QIODevice::Text);
    QXmlStreamReader xml(file);

    History *history = NULL;
    m_history.clear();

    while (!xml.atEnd())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::EndDocument)
            break;
        else if (token == QXmlStreamReader::EndElement)
            continue;

        // new history event
        if (xml.name() == "c")
        {
            m_history.append(History());
            history = &m_history.last();
        }

        if (history && (xml.name() == "a"))
        {
            if (xml.attributes().value("n") == "ClusterId")
            {
                xml.readNext();
                history->clusterId = xml.readElementText().toInt();
            }
            else if (xml.attributes().value("n") == "ProcId")
            {
                xml.readNext();
                history->procId = xml.readElementText().toInt();
            }
            else if (xml.attributes().value("n") == "Owner")
            {
                xml.readNext();
                history->owner = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "JobStartDate")
            {
                xml.readNext();
                qint64 epoch = xml.readElementText().toLongLong() * 1000;
                history->jobStartDate = QDateTime::fromMSecsSinceEpoch(epoch);
            }
            else if (xml.attributes().value("n") == "CompletionDate")
            {
                xml.readNext();
                qint64 epoch = xml.readElementText().toLongLong() * 1000;
                history->completionDate = QDateTime::fromMSecsSinceEpoch(epoch);
            }
            else if (xml.attributes().value("n") == "LastRemoteHost")
            {
                xml.readNext();
                history->machine = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "Cmd")
            {
                xml.readNext();
                history->cmd = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "Args")
            {
                xml.readNext();
                history->args = xml.readElementText();
            }
            else if (xml.attributes().value("n") == "JobStatus")
            {
                xml.readNext();
                switch (xml.readElementText().toInt())
                {
                    case 0:
                        history->jobStatus = "Unexpanded";
                        break;
                    case 1:
                        history->jobStatus = "Idle";
                        break;
                    case 2:
                        history->jobStatus = "Running";
                        break;
                    case 3:
                        history->jobStatus = "Removed";
                        break;
                    case 4:
                        history->jobStatus = "Completed";
                        break;
                    case 5:
                        history->jobStatus = "Held";
                        break;
                    case 6:
                        history->jobStatus = "Submission error";
                        break;
                }
            }
        }
    }

    qSort(m_history);

    trvHistory->setUpdatesEnabled(false);

    foreach (History history, m_history)
    {
        QTreeWidgetItem *historyItem = new QTreeWidgetItem(trvHistory->invisibleRootItem());
        historyItem->setText(0, QString("%1.%2").arg(history.clusterId).arg(history.procId));
        historyItem->setText(1, history.owner);
        historyItem->setText(2, history.machine);
        if (history.jobStartDate.toMSecsSinceEpoch() > 0)
            historyItem->setText(3, history.jobStartDate.toString("dd.MM.yyyy hh:mm:ss"));
        if (history.completionDate.toMSecsSinceEpoch() > 0)
            historyItem->setText(4, history.completionDate.toString("dd.MM.yyyy hh:mm:ss"));
        if (history.jobStartDate.toMSecsSinceEpoch() > 0 && history.completionDate.toMSecsSinceEpoch() > 0)
            historyItem->setText(5, milisecondsToTime(history.jobStartDate.msecsTo(history.completionDate)).toString("hh:mm:ss"));
        historyItem->setText(6, history.jobStatus);
        historyItem->setText(7, QString("%1 %2").arg(history.cmd).arg(history.args));
    }

    trvHistory->setUpdatesEnabled(true);
}

void BrowserWindow::showSettings()
{
    ConfigConnectionDialog configDialog(this);
    if (configDialog.exec())
    {

    }
}

void BrowserWindow::doAbout()
{
    AboutDialog about(this);
    about.exec();
}

void BrowserWindow::handleConnectionError(QSsh::SshError error)
{
    qDebug() << "error" << error;
}

void BrowserWindow::handleDisconnected()
{
    lblSSHState->setText(tr("Disconnected"));
}

void BrowserWindow::handleConnected()
{
    actRefresh->setEnabled(true);
    lblSSHState->setText(tr("Connected"));

    // create sftp channel
    m_channel = m_connection->createSftpChannel();
    connect(m_channel.data(), SIGNAL(initialized()), this, SLOT(handleSFTPChannelInitialized()));
    m_fileState = FileState_Init;
    m_channel->initialize();
}

void BrowserWindow::handleShellStarted()
{
    // QSocketNotifier * const notifier = new QSocketNotifier(0, QSocketNotifier::Read, this);
    // connect(notifier, SIGNAL(activated(int)), SLOT(handleStdin()));
}

void BrowserWindow::handleRemoteStdout()
{
    std::cout << m_process->readAllStandardOutput().data() << std::flush;
    // buffer << m_process->readAllStandardOutput();
    // laststdOut = QString(m_process->readAllStandardOutput().data());
}

void BrowserWindow::handleRemoteStderr()
{
    // lasterrOut = QString(m_process->readAllStandardError().data());
    std::cerr << m_process->readAllStandardError().data() << std::flush;
}

void BrowserWindow::handleChannelClosed(int exitStatus)
{
    if (m_runState == RunState_History)
    {
        m_channel->downloadFile(fnRemote, fnLocal, QSsh::SftpOverwriteExisting);
        parseHistory(fnLocal);
    }

    // parseHistory();
    /*
    QString fnStatus = tempProblemFileName();
    qDebug() << m_process->readAllStandardOutput();
    writeStringContent(fnStatus, laststdOut);
    parseStatus(fnStatus);
    qDebug() << laststdOut;
    */
}

void BrowserWindow::handleSFTPChannelInitialized()
{
    qDebug() << "channel";
}

void BrowserWindow::handleShellMessage(const QString &message)
{
    qDebug() << message;
}

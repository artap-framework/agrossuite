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

#ifndef BROWSER_H
#define BROWSER_H

#include "util.h"
#include "connection.h"

#include "ssh/sshconnection.h"

struct GridNode
{
    QString machine;
    int slotID;
    QString arch;
    QString opSys;
    QString opSysName;
    QString activity;
    QString state;
    int memory;
    double avgLoad;

    bool operator<(const GridNode &other) const
    {
        return (machine < other.machine || (machine == other.machine && slotID < other.slotID));
    }

    bool operator==(const GridNode &other) const
    {
        return (machine == other.machine && slotID == other.slotID);
    }
};

struct History
{
    int clusterId;
    int procId;
    QString owner;
    QString machine;
    QDateTime jobStartDate;
    QDateTime completionDate;
    QString cmd;
    QString args;
    QString jobStatus;

    bool operator<(const History &other) const
    {
        return (completionDate > other.completionDate);
    }
};

class AGROS_LIBRARY_API BrowserWindow : public QMainWindow
{
    Q_OBJECT

    enum RunState
    {
        RunState_Init,
        RunState_Status,
        RunState_History
    };

    enum FileState
    {
        FileState_Init,
        FileState_Status,
        FileState_History
    };

public:
    BrowserWindow(int argc, char *argv[], QWidget *parent = 0);
    ~BrowserWindow();

private:
    QSsh::SshConnection *m_connection;
    QSharedPointer<QSsh::SshRemoteProcess> m_process;
    QSharedPointer<QSsh::SftpChannel> m_channel;

    RunState m_runState;
    FileState m_fileState;

    QString fnRemote;
    QString fnLocal;
    // QFile file;
    // QDataStream buffer;

    QList<GridNode> m_nodes;
    QList<History> m_history;

    QTreeWidget *trvNodes;
    QTreeWidget *trvHistory;
    QToolBar *tlbConnection;

    QLabel *lblSSHState;

    QAction *actAbout;
    QAction *actAboutQt;
    QAction *actConnect;
    QAction *actRefresh;
    QAction *actOptions;
    QAction *actExit;

    void createActions();
    void createMenus();
    void createToolBars();

    void runCommand(const QString &command);

private slots:
    void doAbout();
    void showSettings();

    void connectToHost();    
    void parseStatus(const QString &fileName);
    void parseHistory(const QString &fileName);

    void handleConnectionError(QSsh::SshError error);
    void handleConnected();
    void handleDisconnected();
    void handleShellMessage(const QString &message);

    void handleShellStarted();
    void handleRemoteStdout();
    void handleRemoteStderr();
    void handleChannelClosed(int exitStatus);

    void handleSFTPChannelInitialized();

    void refresh();
};

#endif

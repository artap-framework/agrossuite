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

#include "checkversion.h"

#include "util/util.h"
#include "util/system_utils.h"

QString esc(const QString &str)
{
#if QT_VERSION < 0x050000
    return Qt::escape(str);
#else
    return QString(str).toHtmlEscaped();
#endif
}

static CheckVersion *checkVersion = NULL;
void checkForNewVersion(bool quiet, bool isSolver)
{
    // download version
    QUrl url("http://www.agros2d.org/version/log/version.php");

    if (checkVersion == NULL)
        checkVersion = new CheckVersion(url, isSolver);

    checkVersion->run(quiet);
}

CheckVersion::CheckVersion(QUrl url, bool isSolver) : QObject(), m_url(url), m_solver(isSolver)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    delete m_manager;
}

void CheckVersion::run(bool quiet)
{
    m_quiet = quiet;

    QByteArray postData;
    postData.append(QString("OS=%1&").arg(esc(SystemUtils::operatingSystem())).toLatin1());
    postData.append(QString("PROCESSOR=%1&").arg(esc(SystemUtils::cpuType())).toLatin1());
    postData.append(QString("THREADS=%1&").arg(QString::number(SystemUtils::numberOfThreads())).toLatin1());
    postData.append(QString("MEMORY=%1&").arg(QString::number(SystemUtils::totalMemorySize())).toLatin1());
    postData.append(QString("AGROS_VERSION=%1&").arg(esc(QCoreApplication::applicationVersion())).toLatin1());

    QNetworkRequest req(m_url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));

    m_networkReply = m_manager->post(req, postData);

    connect(m_networkReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    QString text = networkReply->readAll().trimmed();

    if (!text.isEmpty())
    {
        QRegularExpression rx("^(\\d{1,1}.\\d{1,1}.\\d{1,1}.\\d{8,8})");
        if (!QString(text).contains(rx))
        {
            // be quiet
            qDebug() << text;
            return;
        }

        if (text > versionString())
        {
            QString str = tr("<b>New version available.</b><br/><br/>"
                             "Actual version: %1<br/>"
                             "Available version: %2<br/><br/>"
                             "URL: <a href=\"http://www.agros2d.org/down/\">http://www.agros2d.org/down/</a>").
                    arg(versionString()).
                    arg(text);

            // QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
        }
        else if (!m_quiet)
        {
            QString str = tr("<b>You are using actual version.</b><br/><br/>"
                             "Actual version: %1<br/>"
                             "Available version: %2<br/><br/>"
                             "URL: <a href=\"http://www.agros2d.org/down/\">http://www.agros2d.org/down/</a>").
                    arg(versionString()).
                    arg(text);

            // QMessageBox::information(QApplication::activeWindow(), tr("Actual version"), str);
        }
    }
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    qDebug() << "An error ocurred (code #" << error << ").";
}


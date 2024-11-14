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

#include "util/global.h"
#include "util/util.h"
#include "util/system_utils.h"
#include "logview.h"

#include <QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

static CheckVersion *checkVersion = nullptr;
void checkNewVersion(bool quiet, bool showActualVersion)
{
    // download version
    // QUrl url("https://www.agros2d.org/web/version/check.php");
    QUrl url("https://www.agros2d.org/web/version/version.php");

    if (checkVersion == nullptr)
       checkVersion = new CheckVersion(url);

    checkVersion->run(quiet, showActualVersion);

    // CheckVersion checkVersion(url);
    // checkVersion.run(quiet);
}

CheckVersion::CheckVersion(QUrl url) : QObject(), m_url(url)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(downloadFinished(QNetworkReply *)));
}

CheckVersion::~CheckVersion()
{
    delete m_manager;
}

void CheckVersion::run(bool quiet, bool showActualVersion)
{
    m_quiet = quiet;
    m_showActualVersion = showActualVersion;

    QUrlQuery query;
    query.addQueryItem("OS", SystemUtils::operatingSystem());
    query.addQueryItem("PROCESSOR", SystemUtils::cpuType());
    query.addQueryItem("THREADS", QString::number(SystemUtils::numberOfThreads()));
    query.addQueryItem("MEMORY", QString::number(SystemUtils::totalMemorySize()));
    query.addQueryItem("RESOLUTION", QString("%1x%2").arg(QGuiApplication::primaryScreen()->availableGeometry().width()).arg(QGuiApplication::primaryScreen()->availableGeometry().height()));
    query.addQueryItem("AGROS_VERSION", QCoreApplication::applicationVersion());

    QByteArray postData = query.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest req(m_url);
    req.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    m_networkReply = m_manager->post(req, postData);

    connect(m_networkReply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(handleError(QNetworkReply::NetworkError)));
}

void CheckVersion::downloadFinished(QNetworkReply *networkReply)
{
    QString text = networkReply->readAll().trimmed();

    if (!text.isEmpty())
    {
        QRegularExpression rx("\\d{4}\\.\\d{2}\\.\\d{2}");
        if (!QString(text).contains(rx))
        {
            // be quiet
            Agros::log()->printError(tr("Check version"), tr("Check version failed: %1").arg(text));
            qCritical() << "Check version failed: " << text;
            return;
        }

        // qInfo() << text << versionString() << (text > versionString());

        if (m_quiet)
        {
            if (text > versionString())
            {
                QString str = tr("<b>New version available.</b><br/><br/>"
                                 "Actual version: %1<br/>"
                                 "Available version: %2<br/><br/>"
                                 "URL: <a href=\"https://www.agros2d.org/web/#download\">https://www.agros2d.org/web/#download</a>").
                        arg(versionString()).
                        arg(text);

                QMessageBox::information(QApplication::activeWindow(), tr("New version"), str);
            }
            else if ((text == versionString()) && (m_showActualVersion))
            {
                QString str = tr("<b>You are using actual version.</b><br/><br/>"
                                 "Actual version: %1<br/>"
                                 "Available version: %2<br/><br/>"
                                 "URL: <a href=\"https://www.agros2d.org/web/#download\">https://www.agros2d.org/web/#download</a>").
                        arg(versionString()).
                        arg(text);

                QMessageBox::information(QApplication::activeWindow(), tr("Actual version"), str);
            }
            else
            {
                Agros::log()->printError(tr("Check version"), tr("Parse error: %1").arg(text));
            }
        }
    }
}

void CheckVersion::handleError(QNetworkReply::NetworkError error)
{
    qCritical() << "An error ocurred (code #" << error << ").";
}


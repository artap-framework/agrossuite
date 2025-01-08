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

#include "about.h"

#include <QScreen>

#include "util/util.h"
#include "util/system_utils.h"
#include "gui/checkversion.h"
#include "util/global.h"

ShortcutDialog::ShortcutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("agros"));
    setWindowTitle(tr("Shortcuts"));

    createControls();

    setMinimumSize(550, 550);
    setMaximumSize(sizeHint());
}

void ShortcutDialog::createControls()
{
    QString html = readFileContent(QString("%1/resources/templates/about/shortcuts.html").arg(Agros::dataDir()));

    auto *textShortcuts = new QLabel(html);
    textShortcuts->setWordWrap(true);
    textShortcuts->setOpenExternalLinks(true);

    auto *layout = new QVBoxLayout();
    layout->addWidget(textShortcuts);
    layout->addStretch();

    setLayout(layout);
}

// ****************************************************************************************

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowModality(Qt::ApplicationModal);
    setModal(true);

    setWindowIcon(icon("agros"));
    setWindowTitle(tr("About..."));

    createControls();

    setMinimumSize(550, 550);
    setMaximumSize(sizeHint());
}

AboutDialog::~AboutDialog()
{

}

void AboutDialog::createControls()
{
    QTabWidget *tab = new QTabWidget();
    tab->addTab(createAgros(), tr("agros"));
    tab->addTab(createDealii(), tr("deal.ii"));
    tab->addTab(createLibraries(), tr("Libraries"));
    tab->addTab(createLicense(), tr("License"));
    tab->addTab(createSysinfo(), tr("System Informations"));

    QPushButton *buttonClose = new QPushButton(tr("Close"));
    QPushButton *buttonCheckForNewVersion = new QPushButton(tr("Check version"));

    QHBoxLayout *buttonBox = new QHBoxLayout();
    buttonBox->addStretch();
    buttonBox->addWidget(buttonCheckForNewVersion);
    buttonBox->addWidget(buttonClose);

    connect(buttonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(buttonCheckForNewVersion, SIGNAL(clicked()), this, SLOT(checkVersion()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(tab);
    // layout->addStretch();
    layout->addLayout(buttonBox);

    setLayout(layout);
}

QWidget *AboutDialog::createAgros()
{
    QString html = readFileContent(QString("%1/resources/templates/about/agros.html").arg(Agros::dataDir()));

    auto *labelContent = new QLabel(html.arg(QApplication::applicationVersion()));
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    auto *labelIcon = new QLabel();
    labelIcon->setPixmap(icon("agros").pixmap(64, 64));

    auto *layoutIcon = new QGridLayout();
    layoutIcon->addWidget(labelIcon, 0, 0, 1, 1, Qt::AlignTop);
    layoutIcon->addWidget(labelContent, 0, 1);

    auto *layout = new QVBoxLayout();
    layout->addLayout(layoutIcon);
    layout->addStretch();

    auto *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createDealii()
{
    QString html = readFileContent(QString("%1/resources/templates/about/dealii.html").arg(Agros::dataDir()));

    auto *labelContent = new QLabel(html);
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createLibraries()
{
    QString html = readFileContent(QString("%1/resources/templates/about/libraries.html").arg(Agros::dataDir()));

    auto *labelContent = new QLabel(html);
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createLicense()
{
    QString html = readFileContent(QString("%1/resources/templates/about/license.html").arg(Agros::dataDir()));

    QLabel *labelContent = new QLabel(html);
    labelContent->setWordWrap(true);
    labelContent->setOpenExternalLinks(true);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(labelContent);
    layout->addStretch();

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

QWidget *AboutDialog::createSysinfo()
{
    // OS
    QGridLayout *layoutOS = new QGridLayout();
    layoutOS->addWidget(new QLabel(tr("OS:")), 0, 0);
    layoutOS->addWidget(new QLabel(SystemUtils::operatingSystem()), 0, 1);

    QGroupBox *grpOS = new QGroupBox(tr("Operating system"));
    grpOS->setLayout(layoutOS);

    // system
    QGridLayout *layoutSystem = new QGridLayout();
    layoutSystem->addWidget(new QLabel(tr("Processor:")), 10, 0);
    layoutSystem->addWidget(new QLabel(SystemUtils::cpuType()), 10, 1);
    layoutSystem->addWidget(new QLabel(tr("Number of threads:")), 11, 0);
    layoutSystem->addWidget(new QLabel(QString::number(SystemUtils::numberOfThreads())), 11, 1);
    layoutSystem->addWidget(new QLabel(tr("Memory:")), 12, 0);
    layoutSystem->addWidget(new QLabel(QString("%1 GB").arg(SystemUtils::totalMemorySize() / 1024  / 1024 / 1024)), 12, 1);
    layoutSystem->addWidget(new QLabel(tr("Screen resolution:")), 13, 0);
    layoutSystem->addWidget(new QLabel(QString("%1 x %2").
                                       arg(QGuiApplication::primaryScreen()->availableGeometry().width()).
                                       arg(QGuiApplication::primaryScreen()->availableGeometry().height())), 13, 1);
    layoutSystem->setRowStretch(20, 1);

    QGroupBox *grpSystem = new QGroupBox(tr("System"));
    grpSystem->setLayout(layoutSystem);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(grpOS);
    layout->addWidget(grpSystem);
    layout->addStretch(1);

    QWidget *widget = new QWidget();
    widget->setLayout(layout);

    return widget;
}

void AboutDialog::checkVersion()
{
    checkNewVersion(false, true);
}

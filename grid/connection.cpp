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

#include "connection.h"

ConfigConnectionDialog::ConfigConnectionDialog(QWidget *parent) : QDialog(parent)
{
    setWindowIcon(icon("options"));
    setWindowTitle(tr("Options"));

    createControls();

    load();

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void ConfigConnectionDialog::load()
{
    QSettings settings;

    txtSSHHost->setText(settings.value("SSH/Host").toString());
    txtSSHPort->setValue(settings.value("SSH/Port", 22).toInt());
    txtSSHUserName->setText(settings.value("SSH/UserName").toString());
    txtSSHPrivateKeyPath->setText(settings.value("SSH/PrivateKeyPath", "~/.ssh/id_rsa").toString());
}

void ConfigConnectionDialog::save()
{
    QSettings settings;

    settings.setValue("SSH/Host", txtSSHHost->text());
    settings.setValue("SSH/Port", txtSSHPort->value());
    settings.setValue("SSH/UserName", txtSSHUserName->text());
    settings.setValue("SSH/PrivateKeyPath", txtSSHPrivateKeyPath->text());
}

void ConfigConnectionDialog::createControls()
{
    txtSSHHost = new QLineEdit();
    txtSSHPort = new QSpinBox();
    txtSSHPort->setMinimum(1);
    txtSSHPort->setMaximum(65535);
    txtSSHUserName = new QLineEdit();
    txtSSHPrivateKeyPath = new QLineEdit();

    QGridLayout *layoutSSH = new QGridLayout();
    layoutSSH->addWidget(new QLabel("HTCondor host"), 0, 0);
    layoutSSH->addWidget(txtSSHHost, 0, 1);
    layoutSSH->addWidget(new QLabel("SSH port"), 1, 0);
    layoutSSH->addWidget(txtSSHPort, 1, 1);
    layoutSSH->addWidget(new QLabel("Username"), 2, 0);
    layoutSSH->addWidget(txtSSHUserName, 2, 1);
    layoutSSH->addWidget(new QLabel("Private key path"), 3, 0);
    layoutSSH->addWidget(txtSSHPrivateKeyPath, 3, 1);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutSSH);
    // layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);
}

void ConfigConnectionDialog::doAccept()
{
    save();
    accept();
}

void ConfigConnectionDialog::doReject()
{
    reject();
}

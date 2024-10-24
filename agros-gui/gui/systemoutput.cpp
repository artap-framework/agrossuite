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

#include "systemoutput.h"

SystemOutputWidget::SystemOutputWidget(QWidget *parent) : QDialog(parent)
{
    setModal(true);
    setWindowTitle(tr("System shell"));
    setAttribute(Qt::WA_DeleteOnClose);

    m_proc = new QProcess(this);
    m_proc->setEnvironment(QProcess::systemEnvironment());
    m_proc->setProcessChannelMode(QProcess::MergedChannels);
    m_proc->setWorkingDirectory(QApplication::applicationDirPath());

    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(updateError()));
    connect(m_proc, SIGNAL(readyReadStandardOutput()), this, SLOT(updateText()));
    connect(m_proc, SIGNAL(finished(int)), this, SLOT(finished(int)));

    m_output = new QTextEdit(this);

    QFont font = m_output->font();
    font.setPointSize(font.pointSize() - 2);
    m_output->setFont(font);

    QPalette palette = m_output->palette();
    palette.setColor(QPalette::Active, QPalette::Text, QColor(230, 230, 230));
    palette.setColor(QPalette::Active, QPalette::Base, QColor(50, 50, 50));
    m_output->setPalette(palette);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Abort | QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(close()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(breakProcess()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(m_output);
    layout->addWidget(buttonBox);

    setMinimumSize(800, 500);
    setLayout(layout);
}

SystemOutputWidget::~SystemOutputWidget()
{
    delete m_proc;
}

void SystemOutputWidget::breakProcess()
{
    m_proc->kill();
}

void SystemOutputWidget::execute(const QString &command)
{
    show();

    m_output->append(tr("<b>Working directory:</b> %1</br>").arg(m_proc->workingDirectory()));
    m_output->append(QString("<b>Command:</b> %1</br></br>").arg(command));
    QApplication::processEvents();

    m_proc->start(command);
    m_proc->waitForStarted();
    m_proc->waitForFinished();
}

void SystemOutputWidget::updateError()
{
    m_output->append(m_proc->readAllStandardError().trimmed());
    QApplication::processEvents();
}

void SystemOutputWidget::updateText()
{
    QString txt = m_proc->readAllStandardOutput().trimmed();
    txt = txt.replace("[0m", "");
    txt = txt.replace("[33m", "");
    txt = txt.replace("[30m[1m", "");
    txt = txt.replace("‘", "'");
    txt = txt.replace("’", "'");

    m_output->append(txt);
    m_output->update();
}

void SystemOutputWidget::finished(int exit)
{
    if (exit == 0)
    {
        m_output->append("<b>Completed Successfully</b>");
    }
    else
    {
        m_output->append("<b>Error</b>");
    }

    QApplication::processEvents();
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

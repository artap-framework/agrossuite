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

#include "parameterdialog.h"

#include "util/global.h"
#include "gui/lineeditdouble.h"

#include "scene.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/problem_parameter.h"

ParameterDialog::ParameterDialog(QWidget *parent)
    : QDialog(parent), m_key(""), m_value(0.0)
{
    createControls();
}

ParameterDialog::ParameterDialog(const QString &key, QWidget *parent)
    : QDialog(parent), m_key(key)
{
    m_value = Agros::problem()->config()->parameters()->number(key);

    createControls();
}

void ParameterDialog::createControls()
{
    setWindowTitle(tr("Parameter: %1").arg(m_key));
    setWindowIcon(icon("menu_parameter"));
    setMinimumWidth(450);

    lblParametersError = new QLabel();

    txtParameterName = new QLineEdit(m_key);
    connect(txtParameterName, SIGNAL(textChanged(QString)), this, SLOT(parameterNameTextChanged(QString)));
    txtParameterValue = new LineEditDouble(m_value);

    QGridLayout *layoutParametersEdit = new QGridLayout();
    layoutParametersEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutParametersEdit->addWidget(txtParameterName, 0, 1);
    layoutParametersEdit->addWidget(new QLabel(tr("Value")), 1, 0);
    layoutParametersEdit->addWidget(txtParameterValue, 1, 1);

    QPalette palette = lblParametersError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblParametersError->setPalette(palette);
    lblParametersError->setVisible(false);

    // dialog buttons
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doReject()));

    QVBoxLayout *layoutParametersWidget = new QVBoxLayout();
    layoutParametersWidget->addLayout(layoutParametersEdit, 1);
    layoutParametersWidget->addWidget(lblParametersError);
    layoutParametersWidget->addStretch();
    layoutParametersWidget->addWidget(buttonBox);

    setLayout(layoutParametersWidget);

    if (!m_key.isEmpty())
        txtParameterValue->setFocus();
}

void ParameterDialog::doAccept()
{
    if (save())
        accept();
}

void ParameterDialog::doReject()
{
    reject();
}

bool ParameterDialog::save()
{
    lblParametersError->clear();
    lblParametersError->setVisible(false);

    // check parameter name
    try
    {
        Agros::problem()->config()->checkVariableName(txtParameterName->text(), m_key);
    }
    catch (AgrosException &e)
    {
        lblParametersError->setText(e.toString());
        lblParametersError->setVisible(true);

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    }

    QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();
    parameters[txtParameterName->text()] = ProblemParameter(txtParameterName->text(), txtParameterValue->value());

    // remove old parameter
    if (!m_key.isEmpty() && txtParameterName->text() != m_key)
        parameters.remove(m_key);

    QStringList err = Agros::problem()->checkAndApplyParameters(parameters);
    if (err.isEmpty())
    {
        Agros::problem()->scene()->invalidate();
        return true;
    }
    else
    {
        QString msg;
        foreach (QString str, err)
            msg += str + "\n";

        lblParametersError->setText(msg);
        lblParametersError->setVisible(true);
    }

    return false;
}

bool ParameterDialog::remove()
{
    QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();
    parameters.remove(m_key);

    if (Agros::problem()->checkAndApplyParameters(parameters).isEmpty())
    {
        Agros::problem()->scene()->invalidate();
        return true;
    }

    return false;
}

void ParameterDialog::parameterNameTextChanged(const QString &str)
{
    lblParametersError->setVisible(false);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    QMap<QString, ProblemParameter> parameters = Agros::problem()->config()->parameters()->items();

    if (str.isEmpty())
    {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }

    if ((str != m_key) && parameters.contains(str))
    {
        lblParametersError->setText(QObject::tr("Key already exists."));
        lblParametersError->setVisible(true);

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }

    try
    {
        Agros::problem()->config()->checkVariableName(str, m_key);
    }
    catch (AgrosException &e)
    {
        lblParametersError->setText(e.toString());
        lblParametersError->setVisible(true);

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }
}

#include "util/global.h"

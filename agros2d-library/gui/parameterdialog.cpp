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

ParameterDialog::ParameterDialog(QWidget *parent)
    : QDialog(parent), m_key(""), m_value(0.0)
{
    createControls();
}

ParameterDialog::ParameterDialog(const QString &key, QWidget *parent)
    : QDialog(parent), m_key(key)
{
    ParametersType parameters = Agros2D::problem()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
    m_value = parameters[key];

    createControls();
}

void ParameterDialog::createControls()
{
    setWindowTitle(tr("Parameter: %1").arg(m_key));

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
    layoutParametersWidget->addLayout(layoutParametersEdit);
    layoutParametersWidget->addWidget(lblParametersError);
    layoutParametersWidget->addWidget(buttonBox);

    setLayout(layoutParametersWidget);
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
        Agros2D::problem()->config()->checkParameterName(txtParameterName->text());
    }
    catch (AgrosException &e)
    {
        lblParametersError->setText(e.toString());
        lblParametersError->setVisible(true);

        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return false;
    }

    ParametersType parameters = Agros2D::problem()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
    parameters[txtParameterName->text()] = txtParameterValue->value();

    // remove old parameter
    if (!m_key.isEmpty() && txtParameterName->text() != m_key)
        parameters.remove(m_key);

    if (Agros2D::problem()->checkAndApplyParameters(parameters))
    {
        Agros2D::problem()->scene()->invalidate();
        return true;
    }
}

bool ParameterDialog::remove()
{
    ParametersType parameters = Agros2D::problem()->config()->value(ProblemConfig::Parameters).value<ParametersType>();
    parameters.remove(m_key);

    if (Agros2D::problem()->checkAndApplyParameters(parameters))
    {
        Agros2D::problem()->scene()->invalidate();
        return true;
    }

    return false;
}

void ParameterDialog::parameterNameTextChanged(const QString &str)
{
    lblParametersError->setVisible(false);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    ParametersType parameters = Agros2D::problem()->config()->value(ProblemConfig::Parameters).value<ParametersType>();

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
        Agros2D::problem()->config()->checkParameterName(str);
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

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

#include "valuelineedit.h"

#include "util/global.h"
#include "util/conf.h"
#include "gui/valuetimedialog.h"
#include "gui/valuedatatabledialog.h"
#include "app/materialbrowserdialog.h"

#include "solver/field.h"
#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/problem_parameter.h"
#include "solver/problem_function.h"

#include "util/util_expr.h"

ValueLineEdit::ValueLineEdit(QWidget *parent, bool hasTimeDep, bool hasNonlin, bool isBool, QString id, QString onlyIf, QString onlyIfNot, bool isSource)
    : QWidget(parent),
      m_number(0.0),
      m_hasTimeDep(hasTimeDep),
      m_hasNonlin(hasNonlin),
      m_problem(nullptr),
      m_minimum(-numeric_limits<double>::max()),
      m_minimumSharp(-numeric_limits<double>::max()),
      m_maximum(numeric_limits<double>::max()),
      m_maximumSharp(numeric_limits<double>::max()),
      m_isBool(isBool),
      m_id(id),
      m_onlyIf(onlyIf),
      m_onlyIfNot(onlyIfNot),
      m_isSource(isSource),
      txtLineEdit(nullptr),
      chkCheckBox(nullptr)
{
    // create controls
    if (isBool)
    {
        chkCheckBox = new QCheckBox(title(), this);
        connect(chkCheckBox, SIGNAL(stateChanged(int)), this, SLOT(evaluate()));
        connect(chkCheckBox, SIGNAL(stateChanged(int)), this, SLOT(doCheckBoxStateChanged()));
    }
    else
    {
        txtLineEdit = new QLineEdit(this);
        txtLineEdit->setToolTip(tr("This textedit allows using variables."));
        txtLineEdit->setText("0");
        connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(evaluate()));
        //connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(textChanged(QString)));
        connect(txtLineEdit, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));

        btnMaterialDialog = new QPushButton(icon("up-arrow"), "");
        btnMaterialDialog->setMaximumSize(btnMaterialDialog->sizeHint());
        connect(btnMaterialDialog, SIGNAL(clicked()), this, SLOT(doOpenMaterialDialog()));

        btnDataTableDelete = new QPushButton(icon("geometry_trash"), "");
        btnDataTableDelete->setMaximumSize(btnDataTableDelete->sizeHint());
        connect(btnDataTableDelete, SIGNAL(clicked()), this, SLOT(doOpenDataTableDelete()));

        btnDataTableDialog = new QPushButton(icon("up-arrow"), "");
        btnDataTableDialog->setMaximumSize(btnDataTableDialog->sizeHint());
        connect(btnDataTableDialog, SIGNAL(clicked()), this, SLOT(doOpenDataTableDialog()));

        // timedep value
        btnEditTimeDep = new QPushButton(icon("up-arrow"), "");
        connect(btnEditTimeDep, SIGNAL(clicked()), this, SLOT(doOpenValueTimeDialog()));
    }

    lblValue = new QLabel(this);
    lblInfo = new QLabel();

    auto *layout = new QHBoxLayout();
    layout->setContentsMargins(1, 1, 1, 1);
    // layout->setMargin(0);
    if (isBool)
        layout->addWidget(chkCheckBox, 1);
    else
        layout->addWidget(txtLineEdit, 1);
    layout->addWidget(lblInfo, 1);
    layout->addWidget(lblValue, 0, Qt::AlignRight);
    if (!isBool)
    {
        layout->addWidget(btnMaterialDialog, 0, Qt::AlignRight);
        layout->addWidget(btnEditTimeDep, 0, Qt::AlignRight);
        layout->addWidget(btnDataTableDelete, 0, Qt::AlignRight);
        layout->addWidget(btnDataTableDialog, 0, Qt::AlignRight);
    }

    setLayout(layout);

    setLayoutValue();
    evaluate();
}

ValueLineEdit::~ValueLineEdit()
{
}

void ValueLineEdit::setNumber(double value)
{
    txtLineEdit->setText(QString::number(value));
    evaluate();
}

double ValueLineEdit::number()
{
    if (evaluate())
        return m_number;
    else
        return 0.0;
}

void ValueLineEdit::setValue(const Value &value)
{
    if (m_isBool)
    {
        if(value.number() == 0)
            chkCheckBox->setChecked(false);
        else if(value.number() == 1)
            chkCheckBox->setChecked(true);
        else
            assert(0);
    }
    else
    {
        m_problem = value.m_problem;
        m_table = value.table();
        txtLineEdit->setText(value.text());
    }
    setLayoutValue();
    evaluate();
}

Value ValueLineEdit::value()
{
    if (m_isBool)
        return Value(m_problem, int(chkCheckBox->isChecked()));
    else
        return Value(m_problem, txtLineEdit->text(), m_table);
}

void ValueLineEdit::doCheckBoxStateChanged()
{
    Q_EMIT enableFields(m_id, chkCheckBox->isChecked());
}

void ValueLineEdit::doEnableFields(QString id, bool checked)
{
    if (m_onlyIf == id)
    {
        assert(txtLineEdit);
        assert(!m_isBool);
        txtLineEdit->setEnabled(checked);
    }
    if (m_onlyIfNot == id)
    {
        assert(txtLineEdit);
        assert(!m_isBool);
        txtLineEdit->setEnabled(! checked);
    }
}

bool ValueLineEdit::evaluate(bool quiet)
{
    bool isOk = false;
    bool valueChanged = false;

    if (m_isBool)
    {
        m_number = int(chkCheckBox->isChecked());
        Q_EMIT evaluated(false);
        return true;
    }
    else
    {
        if (!m_hasNonlin || m_table.isEmpty())
        {
            Value val = value();

            btnEditTimeDep->setVisible(m_hasTimeDep && Agros::problem()->isTransient());
            btnMaterialDialog->setVisible(!m_isSource && !m_hasNonlin);

            if (val.isEvaluated())
            {
                if (val.number() <= m_minimumSharp)
                {
                    setValueLabel(QString("<= %1").arg(m_minimumSharp), QColor(Qt::blue), true);
                }
                else if (val.number() >= m_maximumSharp)
                {
                    setValueLabel(QString(">= %1").arg(m_maximumSharp), QColor(Qt::blue), true);
                }
                else if (val.number() < m_minimum)
                {
                    setValueLabel(QString("< %1").arg(m_minimum), QColor(Qt::blue), true);
                }
                else if (val.number() > m_maximum)
                {
                    setValueLabel(QString("> %1").arg(m_maximum), QColor(Qt::blue), true);
                }
                else if (!checkCondition(val.number()))
                {
                    setValueLabel(QString("%1").arg(m_condition), QColor(Qt::red), true);
                }
                else
                {
                    double evaluatedNumber = val.number();
                    valueChanged = (evaluatedNumber != m_number);
                    m_number = evaluatedNumber;
                    setValueLabel(QString("%1").arg(m_number, 0, 'g', 3), QApplication::palette().color(QPalette::WindowText),
                                  Agros::configComputer()->value(Config::Config_ShowResults).toBool());
                    isOk = true;
                }
            }
            else
            {
                setValueLabel(tr("error"), QColor(Qt::red), true);
                setFocus();
            }
        }
        else
        {
            // table
            isOk = true;
        }

        if (isOk)
        {
            Q_EMIT evaluated(false);
            if (valueChanged)
            {
                QString textValue = QString("%1").arg(m_number);
                Q_EMIT textChanged(textValue);
            }
            return true;
        }
        else
        {
            Q_EMIT evaluated(true);
            return false;
        }
    }
}

bool ValueLineEdit::checkCondition(double value)
{
    if (m_condition.isEmpty()) return true;

    // condition
    QString condition = m_condition;
    condition.replace(QString("value"), QString::number(value));

    // symbol table
    exprtk::symbol_table<double> parametersSymbolTable = m_problem->config()->parameters()->symbolTable();
    parametersSymbolTable.add_constant("timse", 0.0);

    exprtk::expression<double> expr;
    expr.register_symbol_table(parametersSymbolTable);

    QString error;
    if (compileExpression(condition, expr, &error))
    {
        return true;
    }
    else
    {
        QPalette palette = txtLineEdit->palette();
        palette.setColor(QPalette::Text, QColor(Qt::red));
        txtLineEdit->setPalette(palette);

        txtLineEdit->setToolTip(tr("Condition couldn't be evaluated:\n%1").arg(error.trimmed()));
        return false;
    }
}

void ValueLineEdit::setLayoutValue()
{
    if (m_isBool)
        return;

    txtLineEdit->setVisible(false);
    lblValue->setVisible(false);
    lblInfo->setVisible(false);
    btnDataTableDelete->setVisible(false);
    btnDataTableDialog->setVisible(false);

    if ((!m_hasNonlin) || (m_hasNonlin && m_table.isEmpty()))
    {
        txtLineEdit->setVisible(true);
        lblValue->setVisible(true);
    }
    if (m_hasNonlin && !m_table.isEmpty())
    {
        if (!m_labelX.isEmpty() && !m_labelY.isEmpty())
            lblInfo->setText(tr("nonlinear %1(%2)").arg(m_labelY).arg(m_labelX));
        else
            lblInfo->setText(tr("nonlinear"));
        lblInfo->setVisible(true);
        btnDataTableDelete->setVisible(true);
    }

    btnDataTableDialog->setVisible(m_hasNonlin);
    btnEditTimeDep->setVisible(m_hasTimeDep && Agros::problem()->isTransient());
    btnMaterialDialog->setVisible(!m_isSource && !m_hasNonlin);
}

void ValueLineEdit::setValueLabel(const QString &text, QColor color, bool isVisible)
{
    lblValue->setText(text);
    QPalette palette = lblValue->palette();
    palette.setColor(QPalette::WindowText, color);
    lblValue->setPalette(palette);
    lblValue->setVisible(isVisible);
}

QSize ValueLineEdit::sizeHint() const
{
    return QSize(100, 10);
}

void ValueLineEdit::focusInEvent(QFocusEvent *event)
{
    txtLineEdit->setFocus(event->reason());
}

void ValueLineEdit::doOpenValueTimeDialog()
{
    ValueTimeDialog dialog;
    dialog.setValue(Value(m_problem, txtLineEdit->text()));

    if (dialog.exec() == QDialog::Accepted)
    {
        txtLineEdit->setText(dialog.value().text());
        evaluate();
    }
}

void ValueLineEdit::doOpenDataTableDelete()
{
    m_table.clear();

    setLayoutValue();
    evaluate();
}

void ValueLineEdit::doOpenDataTableDialog()
{
    ValueDataTableDialog dataTableDialog(m_table, this, m_labelX, m_labelY, m_title);
    if (dataTableDialog.exec() == QDialog::Accepted)
        m_table = dataTableDialog.table();

    setLayoutValue();
    evaluate();
}

void ValueLineEdit::doOpenMaterialDialog()
{
    MaterialBrowserDialog materialBrowserDialog(this);
    if (materialBrowserDialog.showDialog(true) == QDialog::Accepted)
    {
        setNumber(materialBrowserDialog.constant());

        setLayoutValue();
        evaluate();
    }
}

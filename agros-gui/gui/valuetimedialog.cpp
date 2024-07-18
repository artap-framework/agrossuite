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

#include "valuetimedialog.h"

#include "util/global.h"
#include "gui/valuelineedit.h"

#include "solver/problem.h"
#include "solver/problem_config.h"
#include "solver/problem_parameter.h"

#include "util/util_expr.h"

ValueTimeDialog::ValueTimeDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Time function"));

    createControls();
    plotFunction();

    setMinimumSize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("ValueTimeDialog/Geometry", saveGeometry()).toByteArray());
}

ValueTimeDialog::~ValueTimeDialog()
{
    QSettings settings;
    settings.setValue("ValueTimeDialog/Geometry", saveGeometry());
}

void ValueTimeDialog::setValue(Value value)
{
    m_problem = value.m_problem;
    txtLineEdit->setText(value.text());
    // plot
    plotFunction();
}

void ValueTimeDialog::createControls()
{
    lblInfoError = new QLabel();

    QPalette palette = lblInfoError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblInfoError->setPalette(palette);

    txtLineEdit = new QLineEdit(this);
    connect(txtLineEdit, SIGNAL(textChanged(QString)), this, SLOT(checkExpression()));

    txtTimeTotal = new ValueLineEdit();
    txtTimeTotal->setValue(Value(m_problem, Agros::problem()->config()->value(ProblemConfig::TimeTotal).toDouble()));

    cmbPresets = new QComboBox();
    cmbPresets->addItem(tr("select a preset..."));
    cmbPresets->addItem(tr("constant"), "1.0");
    cmbPresets->addItem(tr("step"), QString("1.0*(time<%1)"));
    cmbPresets->addItem(tr("sine wave"), QString("1.0*sin(2*pi*1.0/%1*time)"));
    cmbPresets->addItem(tr("exp. step"), "1.0*(exp(-10/%1*time) - exp(-20/%1*time))");
    connect(cmbPresets, SIGNAL(currentIndexChanged(int)), this, SLOT(presetsChanged(int)));

    // chart
    auto chart = new QChart();
    chart->legend()->hide();

    // axis x
    axisX = new QValueAxis;
    axisX->setLabelFormat("%g");
    axisX->setGridLineVisible(true);
    axisX->setTitleText(tr("time"));
    chart->addAxis(axisX, Qt::AlignBottom);

    // axis y
    axisFunction = new QValueAxis;
    axisFunction->setLabelFormat("%g");
    axisFunction->setGridLineVisible(true);
    axisFunction->setTitleText(tr("function"));
    chart->addAxis(axisFunction, Qt::AlignLeft);

    // attach axis
    valueSeries = new QLineSeries();
    valueSeries->setUseOpenGL(true);
    chart->addSeries(valueSeries);
    valueSeries->attachAxis(axisX);
    valueSeries->attachAxis(axisFunction);

    chartView = new ChartView(chart);

    QGridLayout *controlsLayout = new QGridLayout();
    controlsLayout->addWidget(chartView, 0, 0, 1, 4);
    controlsLayout->setRowStretch(0, 1);
    controlsLayout->addWidget(new QLabel(tr("Function:")), 1, 0);
    controlsLayout->addWidget(txtLineEdit, 1, 1);
    controlsLayout->setColumnStretch(1, 1);
    controlsLayout->addWidget(cmbPresets, 1, 2, 1, 2);
    controlsLayout->addWidget(new QLabel(tr("Total time:")), 2, 2);
    controlsLayout->addWidget(txtTimeTotal, 2, 3);
    controlsLayout->addWidget(new QLabel(tr("Error:")), 2, 0);
    controlsLayout->addWidget(lblInfoError, 2, 1, 3, 1, Qt::AlignTop);
    controlsLayout->addWidget(new QLabel(""), 2, 1);
    controlsLayout->addWidget(new QLabel(""), 3, 1);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    // btnPlot = new QPushButton(tr("Plot"));
    // connect(btnPlot, SIGNAL(clicked()), this, SLOT(plotFunction()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    // layoutButtons->addWidget(btnPlot);
    layoutButtons->addWidget(btnOk);
    layoutButtons->addWidget(btnClose);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(controlsLayout);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void ValueTimeDialog::presetsChanged(int index)
{
    if (cmbPresets->currentIndex() > 0)
    {
        if (txtTimeTotal->value().isEvaluated())
        {
            QString preset = cmbPresets->itemData(cmbPresets->currentIndex()).toString().arg(txtTimeTotal->value().number() / 2.0);

            txtLineEdit->setText(preset);
            cmbPresets->setCurrentIndex(0);

            plotFunction();
        }
    }
}

void ValueTimeDialog::checkExpression()
{
    // symbol table
    exprtk::symbol_table<double> parametersSymbolTable = m_problem->config()->parameters()->symbolTable();
    parametersSymbolTable.add_constant("time", 0.0);

    exprtk::expression<double> expr;
    expr.register_symbol_table(parametersSymbolTable);

    QString error;
    if (compileExpression(txtLineEdit->text(), expr, &error))
    {
        plotFunction();
        lblInfoError->clear();
    }
    else
    {
        lblInfoError->setText(error.trimmed());
        txtLineEdit->setFocus();
    }
}

void ValueTimeDialog::plotFunction()
{
    // plot solution
    int count = 200;

    double totalTime = txtTimeTotal->value().number();

    // time step
    double dt = totalTime / (count - 1);

    // block signals
    valueSeries->blockSignals(true);
    valueSeries->clear();

    Value val(m_problem, txtLineEdit->text());
    if (val.isTimeDependent())
    {
        for (int i = 0; i < count; i++)
        {
            double num = val.numberAtTime(i*dt);

            valueSeries->append(i*dt, num);
        }
    }

    // unblock signals
    valueSeries->blockSignals(false);

    // fit
    chartView->fitToData();
}

void ValueTimeDialog::doAccept()
{
    accept();
}

void ValueTimeDialog::doReject()
{
    reject();
}


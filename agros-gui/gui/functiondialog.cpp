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

#include "functiondialog.h"

#include "util/global.h"
#include "gui/lineeditdouble.h"

#include "scene.h"
#include "solver/problem.h"
#include "solver/problem_config.h"

ProblemFunctionDialog::ProblemFunctionDialog(ProblemFunction *function, QWidget *parent)
    : QDialog(parent), m_function(function)
{    
}

int ProblemFunctionDialog::showDialog()
{
    createControls();
    load();

    return exec();
}

void ProblemFunctionDialog::createControls()
{
    setWindowTitle(tr("Function: %1").arg(m_function->name()));
    setWindowIcon(icon("menu_function"));

    setMinimumWidth(600);
    setMinimumHeight(400);

    lblError = new QLabel();

    txtFunctionName = new QLineEdit();
    connect(txtFunctionName, SIGNAL(textChanged(QString)), this, SLOT(functionNameTextChanged(QString)));

    txtLowerBound = new LineEditDouble(0.0, this);
    connect(txtLowerBound, SIGNAL(editingFinished()), this, SLOT(checkRange()));
    txtUpperBound = new LineEditDouble(1.0, this);
    connect(txtUpperBound, SIGNAL(editingFinished()), this, SLOT(checkRange()));

    QGridLayout *layoutEdit = new QGridLayout();
    layoutEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutEdit->addWidget(txtFunctionName, 0, 1);
    layoutEdit->addWidget(new QLabel(tr("Lower bound")), 1, 0);
    layoutEdit->addWidget(txtLowerBound, 1, 1);
    layoutEdit->addWidget(new QLabel(tr("Upper bound")), 2, 0);
    layoutEdit->addWidget(txtUpperBound, 2, 1);

    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);
    lblError->setVisible(false);

    // chart
    auto chart = new QChart();
    // chart->legend()->hide();
    // chart->setTitle(tr("Functions"));

    // axis x
    axisX = new QValueAxis;
    axisX->setLabelFormat("%g");
    axisX->setGridLineVisible(true);
    axisX->setTitleText(tr("value"));
    chart->addAxis(axisX, Qt::AlignBottom);

    // axis y
    axisFunction = new QValueAxis;
    axisFunction->setLabelFormat("%g");
    axisFunction->setGridLineVisible(true);
    axisFunction->setTitleText(tr("function"));
    chart->addAxis(axisFunction, Qt::AlignLeft);

    axisDerivative = new QValueAxis;
    axisDerivative->setLabelFormat("%g");
    axisDerivative->setGridLineVisible(true);
    axisDerivative->setTitleText(tr("derivative"));
    chart->addAxis(axisDerivative, Qt::AlignRight);

    // attach axis
    valueSeries = new QLineSeries();
    valueSeries->setUseOpenGL(true);
    valueSeries->setName(tr("Value"));
    chart->addSeries(valueSeries);
    valueSeries->attachAxis(axisX);
    valueSeries->attachAxis(axisFunction);

    // valueSeriesScatter = new QScatterSeries();
    // valueSeriesScatter->setUseOpenGL(true);
    // valueSeriesScatter->setMarkerSize(12.0);
    // chart->addSeries(valueSeriesScatter);
    // valueSeriesScatter->attachAxis(axisX);
    // valueSeriesScatter->attachAxis(axisFunction);

    derivativeSeries = new QLineSeries();
    derivativeSeries->setUseOpenGL(true);
    derivativeSeries->setName(tr("Derivative"));
    chart->addSeries(derivativeSeries);
    derivativeSeries->attachAxis(axisX);
    derivativeSeries->attachAxis(axisDerivative);

    chartView = new ChartView(chart);

    // dialog buttons
    btnOk = new QPushButton(tr("Ok"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Close"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnPlot);
    layoutButtons->addWidget(btnOk);
    layoutButtons->addWidget(btnClose);

    QVBoxLayout *layoutParametersWidget = new QVBoxLayout();
    layoutParametersWidget->addLayout(layoutEdit);
    layoutParametersWidget->addWidget(createCustomControls());
    layoutParametersWidget->addWidget(lblError);
    layoutParametersWidget->addWidget(chartView, 1);
    layoutParametersWidget->addLayout(layoutButtons);

    setLayout(layoutParametersWidget);
}

void ProblemFunctionDialog::doAccept()
{
    if (save())
        accept();
}

void ProblemFunctionDialog::doReject()
{
    reject();
}

void ProblemFunctionDialog::load()
{
    txtLowerBound->setValue(m_function->lowerBound());
    txtUpperBound->setValue(m_function->upperBound());
    txtFunctionName->setText(m_function->name());

    doPlot();
}

bool ProblemFunctionDialog::save()
{
    lblError->clear();
    lblError->setVisible(false);

    // check function name
    try
    {
        Agros::problem()->config()->checkVariableName(txtFunctionName->text(), m_function->name());

        m_function->setName(txtFunctionName->text());
        m_function->setLowerBound(txtLowerBound->value());
        m_function->setUpperBound(txtUpperBound->value());
    }
    catch (AgrosException &e)
    {
        lblError->setText(e.toString());
        lblError->setVisible(true);

        btnOk->setEnabled(false);
        return false;
    }

    return true;
}

bool ProblemFunctionDialog::checkRange()
{
    if (txtLowerBound->value() > txtUpperBound->value())
    {
        lblError->setText(tr("Lower bound is higher then upper bound."));
        lblError->setVisible(true);

        btnOk->setEnabled(false);
        return false;
    }

    return true;
}

void ProblemFunctionDialog::doPlot()
{
    // fit to chart
    chartView->fitToData();
}

void ProblemFunctionDialog::functionNameTextChanged(const QString &str)
{
    lblError->setVisible(false);
    btnOk->setEnabled(true);

    if (str.isEmpty())
    {
        btnOk->setEnabled(false);
        return;
    }

    try
    {
        Agros::problem()->config()->checkVariableName(str, m_function->name());
    }
    catch (AgrosException &e)
    {
        lblError->setText(e.toString());
        lblError->setVisible(true);

        btnOk ->setEnabled(false);
        return;
    }
}

// ******************************************************************************************************

ProblemFunctionAnalyticDialog::ProblemFunctionAnalyticDialog(ProblemFunctionAnalytic *function, QWidget *parent)
    : ProblemFunctionDialog(function, parent)
{

}

QWidget *ProblemFunctionAnalyticDialog::createCustomControls()
{
    txtExpression = new QLineEdit(this);

    QGridLayout *layoutCustom = new QGridLayout();
    layoutCustom->addWidget(new QLabel(tr("Expression")), 0, 0);
    layoutCustom->addWidget(txtExpression, 0, 1);

    QGroupBox *widget = new QGroupBox(tr("Analytic expression"));
    widget->setLayout(layoutCustom);

    return widget;
}

void ProblemFunctionAnalyticDialog::load()
{
    txtExpression->setText(function()->expression());

    ProblemFunctionDialog::load();
}

bool ProblemFunctionAnalyticDialog::save()
{
    if (ProblemFunctionDialog::save())
    {
        function()->setExpression(txtExpression->text());

        return true;
    }

    return false;
}

void ProblemFunctionAnalyticDialog::doPlot()
{
    // set expression
    function()->setExpression(txtExpression->text());

    int numberOfSteps = 100;
    double step = (txtUpperBound->value() - txtLowerBound->value()) / (numberOfSteps - 1);

    // block signals
    valueSeries->blockSignals(true);
    // valueSeriesScatter->blockSignals(true);
    derivativeSeries->blockSignals(true);

    valueSeries->clear();
    // valueSeriesScatter->clear();
    derivativeSeries->clear();

    for (int i = 0; i < numberOfSteps; i++)
    {
        double val = i * step;

        valueSeries->append(val, function()->value(val));
        // valueSeriesScatter->append(val, function()->value(val));
        derivativeSeries->append(val, function()->derivative(val));
    }

    // unblock signals
    valueSeries->blockSignals(false);
    // valueSeriesScatter->blockSignals(false);
    derivativeSeries->blockSignals(false);

    ProblemFunctionDialog::doPlot();
}

#include "util/global.h"

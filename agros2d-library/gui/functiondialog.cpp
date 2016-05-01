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

#include "qcustomplot/qcustomplot.h"

FunctionDialog::FunctionDialog(ProblemFunction *function, QWidget *parent)
    : QDialog(parent), m_function(function)
{    
}

int FunctionDialog::showDialog()
{
    createControls();
    load();

    return exec();
}

void FunctionDialog::createControls()
{
    setWindowTitle(tr("Function: %1").arg(m_function->name()));

    lblError = new QLabel();

    txtFunctionName = new QLineEdit();
    connect(txtFunctionName, SIGNAL(textChanged(QString)), this, SLOT(functionNameTextChanged(QString)));

    QGridLayout *layoutEdit = new QGridLayout();
    layoutEdit->addWidget(new QLabel(tr("Name")), 0, 0);
    layoutEdit->addWidget(txtFunctionName, 0, 1);

    QHBoxLayout *layoutCustom = new QHBoxLayout();
    layoutCustom->setMargin(0);
    layoutCustom->addWidget(createCustomControls());

    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);
    lblError->setVisible(false);

    txtLowerBound = new LineEditDouble(0.0, this);
    connect(txtLowerBound, SIGNAL(editingFinished()), this, SLOT(checkRange()));
    txtUpperBound = new LineEditDouble(1.0, this);
    connect(txtUpperBound, SIGNAL(editingFinished()), this, SLOT(checkRange()));

    // chart
    chart = new QCustomPlot(this);
    chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    chart->xAxis->setLabel(tr("value"));
    chart->yAxis->setLabel(tr("function"));
    chart->yAxis2->setLabel(tr("derivative"));
    chart->yAxis2->setVisible(true);

    graphValueLine = chart->addGraph(chart->xAxis, chart->yAxis);
    graphValueLine->setLineStyle(QCPGraph::lsLine);

    graphValueScatter = chart->addGraph(chart->xAxis, chart->yAxis);
    graphValueScatter->setLineStyle(QCPGraph::lsNone);
    graphValueScatter->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
    graphValueScatter->setPen(QPen(Qt::gray));

    graphDerivativeLine = chart->addGraph(chart->xAxis, chart->yAxis2);
    graphDerivativeLine->setLineStyle(QCPGraph::lsLine);

    // dialog buttons
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
    layoutParametersWidget->addLayout(layoutCustom);
    layoutParametersWidget->addWidget(lblError);
    layoutParametersWidget->addWidget(chart, 1);
    layoutParametersWidget->addLayout(layoutButtons);

    setLayout(layoutParametersWidget);
}

void FunctionDialog::doAccept()
{
    if (save())
        accept();
}

void FunctionDialog::doReject()
{
    reject();
}

void FunctionDialog::load()
{
    txtLowerBound->setValue(m_function->lowerBound());
    txtUpperBound->setValue(m_function->upperBound());
    txtFunctionName->setText(m_function->name());
}

bool FunctionDialog::save()
{
    lblError->clear();
    lblError->setVisible(false);

    // check function name
    try
    {
        Agros2D::problem()->config()->checkVariableName(txtFunctionName->text(), m_function->name());

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

bool FunctionDialog::checkRange()
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

void FunctionDialog::doPlot()
{
    chart->rescaleAxes();
    chart->replot(QCustomPlot::rpQueued);
}

void FunctionDialog::functionNameTextChanged(const QString &str)
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
        Agros2D::problem()->config()->checkVariableName(str, m_function->name());
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

FunctionAnalyticDialog::FunctionAnalyticDialog(ProblemFunctionAnalytic *function, QWidget *parent)
    : FunctionDialog(function, parent)
{

}

QWidget *FunctionAnalyticDialog::createCustomControls()
{
    txtExpression = new QLineEdit(this);

    QGridLayout *layoutCustom = new QGridLayout();
    layoutCustom->addWidget(new QLabel(tr("Expression")), 0, 0);
    layoutCustom->addWidget(txtExpression, 0, 1);

    QWidget *widget = new QWidget();
    widget->setLayout(layoutCustom);

    return widget;
}

void FunctionAnalyticDialog::load()
{
    FunctionDialog::load();

    txtExpression->setText(function()->expression());
}

bool FunctionAnalyticDialog::save()
{
    if (FunctionDialog::save())
    {
        function()->setExpression(txtExpression->text());

        return true;
    }

    return false;
}

void FunctionAnalyticDialog::doPlot()
{
    QVector<double> values;
    QVector<double> valuesFunction;
    QVector<double> valuesDerivative;

    int numberOfSteps = 100;
    double step = (txtUpperBound->value() - txtLowerBound->value()) / (numberOfSteps - 1);

    for (int i = 0; i < numberOfSteps; i++)
    {
        double val = i * step;

        values.append(val);
        valuesFunction.append(function()->value(val));
        valuesDerivative.append(function()->derivative(val));
    }

    graphValueLine->setData(values, valuesFunction);
    graphDerivativeLine->setData(values, valuesDerivative);

    FunctionDialog::doPlot();
}

#include "util/global.h"

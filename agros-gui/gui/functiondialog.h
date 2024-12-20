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

#ifndef FUNCTIONDIALOG_H
#define FUNCTIONDIALOG_H

#include "util/util.h"
#include "gui/other.h"
#include "gui/chart.h"
#include "solver/problem_function.h"

class LineEditDouble;

class ProblemFunctionDialog : public QDialog
{
    Q_OBJECT
public:
    ProblemFunctionDialog(ProblemFunction *function, QWidget *parent = 0);

    int showDialog();

protected:
    ProblemFunction *m_function;

    QLabel *lblError;
    QLineEdit *txtFunctionName;

    LineEditDouble *txtUpperBound;
    LineEditDouble *txtLowerBound;

    ChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisFunction;
    QValueAxis *axisDerivative;
    QLineSeries *valueSeries;
    // QScatterSeries *valueSeriesScatter;
    QLineSeries *derivativeSeries;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;

    void createControls();
    virtual QWidget *createCustomControls() = 0;

protected slots:
    void doAccept();
    void doReject();
    bool checkRange();
    virtual void doPlot();

    virtual void load();
    virtual bool save();

    void functionNameTextChanged(const QString &str);
};

class ProblemFunctionAnalyticDialog : public ProblemFunctionDialog
{
    Q_OBJECT
public:
    ProblemFunctionAnalyticDialog(ProblemFunctionAnalytic *function, QWidget *parent = 0);

protected:
    QLineEdit *txtExpression;

    virtual QWidget *createCustomControls();

    ProblemFunctionAnalytic *function() { return dynamic_cast<ProblemFunctionAnalytic *>(m_function); }

protected slots:
    virtual void doPlot();

    virtual void load();
    virtual bool save();
};

#endif // FUNCTIONDIALOG_H

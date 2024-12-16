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

#ifndef DATATABLEDIALOG_H
#define DATATABLEDIALOG_H

#include "util/util.h"
#include "gui/other.h"
#include "datatable.h"
#include <QtCharts>

class ChartView;

class ValueDataTableDialog: public QDialog
{
    Q_OBJECT

public:
    ValueDataTableDialog(DataTable table, QWidget *parent = 0, const QString &labelX = "x", const QString &labelY = "y", const QString &title = "");
    ~ValueDataTableDialog();

    DataTable table();
    void processDataTable();

private:
    // captions
    QString title;
    QString labelX;
    QString labelY;

    // charts
    ChartView *chartViewValue;
    QChart *chartValue;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QLineSeries *valueSeries;
    QScatterSeries *valueMarkersSeries;

    ChartView *chartViewDerivative;
    QChart *chartDerivative;
    QValueAxis *axisDX;
    QValueAxis *axisDY;
    QLineSeries *derivativeSeries;

    // values
    QPlainTextEdit *lstX;
    QPlainTextEdit *lstY;

    // info
    QLabel *lblLabelX;
    QLabel *lblLabelY;
    QLabel *lblInfoX;
    QLabel *lblInfoY;
    QLabel *lblInfoError;

    QCheckBox *chkDerivative;
    QCheckBox *chkMarkers;

    QGroupBox *grpInterpolation;

    QComboBox *cmbType;
    QRadioButton *radFirstDerivative;
    QRadioButton *radSecondDerivative;
    QRadioButton *radExtrapolateConstant;
    QRadioButton *radExtrapolateLinear;

    QPushButton *btnOk;
    QPushButton *btnClose;
    QPushButton *btnPlot;
    QPushButton *btnPlotAndFit;

    DataTable m_table;

    void createControls();
    void load();
    bool save();

    bool parseTable(bool addToTable = true);

    void highlightCurrentLine(QPlainTextEdit *lst);
    void gotoLine(QPlainTextEdit *lst, int lineNumber);

private slots:
    void doAccept();
    void doReject();

    void textChanged();
    void highlightCurrentLineX();
    void highlightCurrentLineY();
    void doPlot();
    void doPlotAndFit();
    void doShowDerivativeClicked();
    void doMaterialBrowser();
    void doTypeChanged();
    void doSplineDerivativeChanged();
    void doExtrapolateChanged();
};

#endif // DATATABLEDIALOG_H


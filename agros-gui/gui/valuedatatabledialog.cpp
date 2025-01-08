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

#include "valuedatatabledialog.h"

#include "gui/chart.h"

#include "app/materialbrowserdialog.h"

ValueDataTableDialog::ValueDataTableDialog(DataTable table, QWidget *parent, const QString &labelX, const QString &labelY, const QString &title)
    : QDialog(parent), labelX(labelX), labelY(labelY), m_table(table), title(title)
{
    setWindowTitle(tr("Data Table"));

    createControls();
    setMinimumSize(600, 400);

    QSettings settings;
    restoreGeometry(settings.value("DataTableDialog/Geometry", saveGeometry()).toByteArray());
    chkDerivative->setChecked(settings.value("DataTableDialog/Derivative", false).toBool());
    chkMarkers->setChecked(settings.value("DataTableDialog/Markers", true).toBool());

    chartViewDerivative->setVisible(chkDerivative->isChecked());
    chartViewDerivative->setEnabled(chkDerivative->isChecked());

    load();
    processDataTable();

    // fit to chart
    chartViewValue->fitToData();

    // fit derivative
    chartViewDerivative->fitToData();
}

ValueDataTableDialog::~ValueDataTableDialog()
{
    QSettings settings;
    settings.setValue("DataTableDialog/Geometry", saveGeometry());
    settings.setValue("DataTableDialog/Derivative", chkDerivative->isChecked());
    settings.setValue("DataTableDialog/Markers", chkMarkers->isChecked());    
}

void ValueDataTableDialog::processDataTable()
{
    lstX->blockSignals(true);
    lstY->blockSignals(true);
    for (int i = 0; i < m_table.pointsVector().size(); i++)
    {
        lstX->appendPlainText(QString::number(m_table.pointsVector().at(i)));
        lstY->appendPlainText(QString::number(m_table.valuesVector().at(i)));
    }
    lstX->blockSignals(false);
    lstY->blockSignals(false);

    // plot
    textChanged();
    doPlot();
}

bool ValueDataTableDialog::parseTable(bool addToTable)
{
    lblInfoError->setText("");

    if (lstX->toPlainText().trimmed().isEmpty() && lstY->toPlainText().trimmed().isEmpty())
    {
        chartViewValue->setEnabled(false);
        chartViewDerivative->setEnabled(false);

        return false;
    }

    bool procesOK = true;

    QStringList x = lstX->toPlainText().trimmed().split("\n");
    QStringList y = lstY->toPlainText().trimmed().split("\n");

    // check size
    if (x.size() != y.size())
    {
        btnPlot->setEnabled(false);
        btnPlotAndFit->setEnabled(false);
        btnOk->setEnabled(false);

        lblInfoError->setText((x.size() > y.size()) ? tr("Size doesn't match (%1 > %2).").arg(x.size()).arg(y.size()) :
                                                      tr("Size doesn't match (%1 < %2).").arg(x.size()).arg(y.size()));
        return false;
    }

    int count = x.size();
    auto *keys = new double[count];
    auto *values = new double[count];

    chartViewValue->setEnabled(true);
    chartViewDerivative->setEnabled(true);

    for (int i = 0; i < count; i++)
    {
        bool ok;

        // parse X
        keys[i] = x[i].toDouble(&ok);
        if (!ok)
        {
            lblInfoError->setText(tr("%1: cannot parse X number (line %2).")
                                  .arg(labelX)
                                  .arg(i+1));
            procesOK = false;
            break;
        }

        if ((i > 0) && (keys[i] < keys[i-1]))
        {
            lblInfoError->setText(tr("%1: points must be in ascending order (line %2).")
                                  .arg(labelX)
                                  .arg(i+1));

            procesOK = false;
            break;
        }

        // parse Y
        values[i] = y[i].toDouble(&ok);
        if (!ok)
        {
            lblInfoError->setText(tr("%1: cannot parse Y number (line %2).")
                                  .arg(labelY)
                                  .arg(i+1));
            procesOK = false;
            break;
        }
    }

    if (addToTable)
    {
        m_table.clear();
        m_table.setValues(keys, values, count);
    }

    delete [] keys;
    delete [] values;

    btnPlot->setEnabled(procesOK);
    btnPlotAndFit->setEnabled(procesOK);
    btnOk->setEnabled(procesOK);

    return procesOK;
}

void ValueDataTableDialog::createControls()
{
    lblLabelX = new QLabel(labelX);
    lblLabelY = new QLabel(labelY);
    lblInfoX = new QLabel();
    lblInfoY = new QLabel();
    lblInfoError = new QLabel();

    QPalette palette = lblInfoError->palette();
    palette.setColor(QPalette::WindowText, Qt::red);
    lblInfoError->setPalette(palette);

    // chart
    chartValue = new QChart();
    chartValue->legend()->hide();
    // chartValue->setTitle(tr("Nonlinear solver"));

    // axis x
    axisX = new QValueAxis;
    axisX->setLabelFormat("%g");
    axisX->setGridLineVisible(true);
    axisX->setTitleText(tr("%1").arg(labelX));
    chartValue->addAxis(axisX, Qt::AlignBottom);

    // axis y
    axisY = new QValueAxis;
    axisY->setLabelFormat("%g");
    axisY->setGridLineVisible(true);
    axisY->setTitleText(QString("%1").arg(labelY));
    chartValue->addAxis(axisY, Qt::AlignLeft);

    // attach axis
    valueSeries = new QLineSeries();
    valueSeries->setUseOpenGL(true);
    chartValue->addSeries(valueSeries);
    valueSeries->attachAxis(axisX);
    valueSeries->attachAxis(axisY);

    valueMarkersSeries = new QScatterSeries();
    valueMarkersSeries->setMarkerSize(12.0);
    chartValue->addSeries(valueMarkersSeries);
    valueMarkersSeries->attachAxis(axisX);
    valueMarkersSeries->attachAxis(axisY);

    // derivative
    chartDerivative = new QChart();
    chartDerivative->legend()->hide();

    // axis x
    axisDX = new QValueAxis;
    axisDX->setLabelFormat("%g");
    axisDX->setGridLineVisible(true);
    axisDX->setTitleText(QString("%1").arg(labelX));
    chartDerivative->addAxis(axisDX, Qt::AlignBottom);

    // axis y
    axisDY = new QValueAxis;
    axisDY->setLabelFormat("%g");
    axisDY->setGridLineVisible(true);
    axisDY->setTitleText(QString("d%1/d%2").arg(labelY).arg(labelX));
    chartDerivative->addAxis(axisDY, Qt::AlignLeft);

    // attach axis
    derivativeSeries = new QLineSeries();
    derivativeSeries->setUseOpenGL(true);
    chartDerivative->addSeries(derivativeSeries);
    derivativeSeries->attachAxis(axisDX);
    derivativeSeries->attachAxis(axisDY);

    chartViewValue = new ChartView(chartValue);
    chartViewDerivative = new ChartView(chartDerivative);

    QVBoxLayout *chartLayout = new QVBoxLayout();
    chartLayout->addWidget(chartViewValue, 1);
    chartLayout->addWidget(chartViewDerivative, 1);

    // interval
    lstX = new QPlainTextEdit();
    lstX->setMaximumWidth(100);
    lstX->setMinimumWidth(100);
    connect(lstX, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(lstX, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLineX()));
    lstY = new QPlainTextEdit();
    lstY->setMaximumWidth(100);
    lstY->setMinimumWidth(100);
    connect(lstY, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(lstY, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLineY()));

    chkMarkers = new QCheckBox(tr("Show markers"));
    connect(chkMarkers, SIGNAL(clicked()), this, SLOT(doPlot()));

    chkDerivative = new QCheckBox(tr("Derivative chart"));
    connect(chkDerivative, SIGNAL(clicked()), this, SLOT(doShowDerivativeClicked()));
    doShowDerivativeClicked();

    cmbType = new QComboBox();
    cmbType->addItem(dataTableTypeString(DataTableType_CubicSpline), DataTableType_CubicSpline);
    cmbType->addItem(dataTableTypeString(DataTableType_PiecewiseLinear), DataTableType_PiecewiseLinear);
    cmbType->addItem(dataTableTypeString(DataTableType_Constant), DataTableType_Constant);
    cmbType->setCurrentIndex(m_table.type());
    connect(cmbType, SIGNAL(currentIndexChanged(int)), this, SLOT(doTypeChanged()));

    radFirstDerivative = new QRadioButton(tr("First"));
    radSecondDerivative = new QRadioButton(tr("Second"));
    QButtonGroup *derivativeGroup = new QButtonGroup();
    derivativeGroup->addButton(radFirstDerivative);
    derivativeGroup->addButton(radSecondDerivative);
    radFirstDerivative->setChecked(m_table.splineFirstDerivatives());
    radSecondDerivative->setChecked(!m_table.splineFirstDerivatives());
    connect(radFirstDerivative, SIGNAL(clicked()), this, SLOT(doSplineDerivativeChanged()));
    connect(radSecondDerivative, SIGNAL(clicked()), this, SLOT(doSplineDerivativeChanged()));

    radExtrapolateConstant = new QRadioButton(tr("Constant"));
    radExtrapolateLinear = new QRadioButton(tr("Linear function"));
    QButtonGroup *extrapolatinGroup = new QButtonGroup();
    extrapolatinGroup->addButton(radExtrapolateConstant);
    extrapolatinGroup->addButton(radExtrapolateLinear);
    radExtrapolateConstant->setChecked(m_table.extrapolateConstant());
    radExtrapolateLinear->setChecked(!m_table.extrapolateConstant());
    connect(radExtrapolateConstant, SIGNAL(clicked()), this, SLOT(doExtrapolateChanged()));
    connect(radExtrapolateLinear, SIGNAL(clicked()), this, SLOT(doExtrapolateChanged()));

    QVBoxLayout *layoutView = new QVBoxLayout();
    layoutView->addWidget(chkMarkers);
    layoutView->addWidget(chkDerivative);
    layoutView->addStretch();

    QGroupBox *grpView = new QGroupBox(tr("View"));
    grpView->setLayout(layoutView);

    QGridLayout *layoutType = new QGridLayout();
    layoutType->addWidget(new QLabel(tr("Interpolation")), 0, 0);
    layoutType->addWidget(cmbType, 0, 1);

    QGridLayout *layoutInterpolation = new QGridLayout();
    layoutInterpolation->addWidget(new QLabel(tr("Derivative to be zero at endpoints")), 0, 0, 1, 2);
    layoutInterpolation->addWidget(radFirstDerivative, 1, 0, 1, 1);
    layoutInterpolation->addWidget(radSecondDerivative, 1, 1, 1, 1);
    layoutInterpolation->addWidget(new QLabel(tr("Extrapolate as")), 2, 0, 1, 2);
    layoutInterpolation->addWidget(radExtrapolateConstant, 3, 0, 1, 1);
    layoutInterpolation->addWidget(radExtrapolateLinear, 3, 1, 1, 1);

    grpInterpolation = new QGroupBox(tr("Spline properties"));
    grpInterpolation->setLayout(layoutInterpolation);
    grpInterpolation->setEnabled(m_table.type() == DataTableType_CubicSpline);

    QVBoxLayout *layoutSettings = new QVBoxLayout();
    layoutSettings->addLayout(layoutType);
    layoutSettings->addWidget(grpInterpolation);
    layoutSettings->addWidget(grpView);

    QGridLayout *tableLayout = new QGridLayout();
    tableLayout->addWidget(lblLabelX, 0, 0);
    tableLayout->addWidget(lblInfoX, 0, 1, 1, 1, Qt::AlignRight);
    tableLayout->addWidget(lstX, 1, 0, 1, 2);
    tableLayout->addWidget(lblLabelY, 0, 2);
    tableLayout->addWidget(lblInfoY, 0, 3, 1, 1, Qt::AlignRight);
    tableLayout->addWidget(lstY, 1, 2, 1, 2);
    tableLayout->addWidget(lblInfoError, 2, 0, 1, 4);

    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->addLayout(tableLayout, 1);
    leftLayout->addLayout(layoutSettings);

    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->addLayout(leftLayout);
    controlsLayout->addLayout(chartLayout, 1);

    // dialog buttons
    btnOk = new QPushButton(tr("Apply"));
    btnOk->setDefault(true);
    connect(btnOk, SIGNAL(clicked()), this, SLOT(doAccept()));
    btnClose = new QPushButton(tr("Cancel"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(doReject()));
    btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(doPlot()));
    btnPlotAndFit = new QPushButton(tr("Plot and fit"));
    connect(btnPlotAndFit, SIGNAL(clicked()), this, SLOT(doPlotAndFit()));
    QPushButton *btnMaterialBrowser = new QPushButton(tr("Material browser"));
    connect(btnMaterialBrowser, SIGNAL(clicked()), this, SLOT(doMaterialBrowser()));

    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch();
    layoutButtons->addWidget(btnMaterialBrowser);
    layoutButtons->addWidget(btnPlot);
    layoutButtons->addWidget(btnPlotAndFit);
    layoutButtons->addWidget(btnOk);
    layoutButtons->addWidget(btnClose);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(controlsLayout);
    layout->addLayout(layoutButtons);

    setLayout(layout);
}

void ValueDataTableDialog::textChanged()
{
    lblInfoX->setText(QString("%1").arg(lstX->toPlainText().trimmed().split("\n").size()));
    lblInfoY->setText(QString("%1").arg(lstY->toPlainText().trimmed().split("\n").size()));

    // try parse
    parseTable(false);
}

void ValueDataTableDialog::gotoLine(QPlainTextEdit *lst, int lineNumber)
{
    if (lineNumber >= lst->document()->lineCount())
        lineNumber = lst->document()->lineCount() - 1;

    const QTextBlock &block = lst->document()->findBlockByNumber(lineNumber);
    QTextCursor cursor(block);

    int linedif = lineNumber - lst->textCursor().blockNumber();
    if (linedif < 0)
        cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 0);
    else
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, 0);

    lst->blockSignals(true);
    lst->setTextCursor(cursor);
    highlightCurrentLine(lst);
    lst->blockSignals(false);
    lst->ensureCursorVisible();
}

void ValueDataTableDialog::highlightCurrentLine(QPlainTextEdit *lst)
{
    lst->blockSignals(true);

    QList<QTextEdit::ExtraSelection> selections;

    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(Qt::yellow).lighter(180);

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = lst->textCursor();
    selection.cursor.clearSelection();
    selections.append(selection);

    lst->setExtraSelections(selections);
    lst->blockSignals(false);
}

void ValueDataTableDialog::highlightCurrentLineX()
{
    highlightCurrentLine(lstX);
    gotoLine(lstY, lstX->textCursor().blockNumber());
}

void ValueDataTableDialog::highlightCurrentLineY()
{
    highlightCurrentLine(lstY);
    gotoLine(lstX, lstY->textCursor().blockNumber());
}

void ValueDataTableDialog::doPlot()
{
    parseTable();

    // block signals
    chartValue->blockSignals(true);
    chartDerivative->blockSignals(true);

    // points
    int count = m_table.size();

    std::vector<double> pointsVector = m_table.pointsVector();
    std::vector<double> valuesVector = m_table.valuesVector();

    if (chkMarkers->isChecked())
    {
        valueMarkersSeries->clear();

        for (int i = 0; i < m_table.size(); ++i)
            valueMarkersSeries->append(pointsVector[i], valuesVector[i]);
    }
    else
    {
        valueMarkersSeries->clear();
    }

    // plot
    int countSpline = count * 50;
    double keyLength = m_table.maxKey() - m_table.minKey();
    double keyStart = m_table.minKey();
    double dx = keyLength / (countSpline - 1);

    // plot
    valueSeries->clear();
    derivativeSeries->clear();
    for (int i = 0; i < countSpline; i++)
    {
        valueSeries->append(keyStart + (i * dx), m_table.value(keyStart + (i * dx)));
        derivativeSeries->append(keyStart + (i * dx), m_table.derivative(keyStart + (i * dx)));
    }

    // unblock signals
    chartValue->blockSignals(false);
    chartDerivative->blockSignals(false);
}

void ValueDataTableDialog::doPlotAndFit()
{
    doPlot();

    // fit to chart
    chartViewValue->fitToData();

    // fit derivative
    chartViewDerivative->fitToData();
}

void ValueDataTableDialog::doShowDerivativeClicked()
{
    chartViewDerivative->setVisible(chkDerivative->isChecked());
    chartViewDerivative->setEnabled(chkDerivative->isChecked());

    doPlot();
}

void ValueDataTableDialog::doTypeChanged()
{
    m_table.setType(DataTableType(cmbType->currentIndex()));
    grpInterpolation->setEnabled(m_table.type() == DataTableType_CubicSpline);
    doPlot();
}

void ValueDataTableDialog::doSplineDerivativeChanged()
{
    assert(radFirstDerivative->isChecked() != radSecondDerivative->isChecked());
    m_table.setSplineFirstDerivatives(radFirstDerivative->isChecked());
    doPlot();
}

void ValueDataTableDialog::doExtrapolateChanged()
{
    assert(radExtrapolateConstant->isChecked() != radExtrapolateLinear->isChecked());
    m_table.setExtrapolateConstant(radExtrapolateConstant->isChecked());
    doPlot();
}

void ValueDataTableDialog::doMaterialBrowser()
{
    MaterialBrowserDialog materialBrowserDialog(this);
    if (materialBrowserDialog.showDialog(true) == QDialog::Accepted)
    {
        lstX->clear();
        lstY->clear();
        for (int i = 0; i < materialBrowserDialog.x().size(); i++)
        {
            lstX->appendPlainText(QString::number(materialBrowserDialog.x().at(i)));
            lstY->appendPlainText(QString::number(materialBrowserDialog.y().at(i)));
        }

        doPlotAndFit();
    }
}

void ValueDataTableDialog::load()
{

}

bool ValueDataTableDialog::save()
{
    return parseTable();
}

void ValueDataTableDialog::doAccept()
{
    if (save())
    {
        accept();
    }
}

void ValueDataTableDialog::doReject()
{
    reject();
}

DataTable ValueDataTableDialog::table()
{
    return m_table;
}

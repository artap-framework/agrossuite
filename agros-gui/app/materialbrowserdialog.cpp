
// ***********************************************************************************************************************************************

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

#include "materialbrowserdialog.h"

#include "util/constants.h"
#include "util/global.h"
#include "gui/lineeditdouble.h"

#include "gui/lineeditdouble.h"

#include "ctemplate/template.h"
#include "qcustomplot/qcustomplot.h"

#ifdef DEAL_II_WITH_TBB
#include "tbb/mutex.h"
tbb::mutex compileExpressionMutex;
#endif

#define GENERAL "general"
#define VERSION "version"
#define NAME "name"
#define DESCRIPTION "description"

#define PROPERTIES "properties"
#define SOURCE "source"
#define TYPE "type"
#define SHORTNAME "shortname"
#define UNIT "unit"
#define INDEPENDENT_SHORTNAME "independent_shortname"
#define INDEPENDENT_UNIT "independent_unit"

#define CONSTANT "constant"
#define TABLE "table"
#define KEYS "keys"
#define VALUES "values"
#define FUNCTION "function"
#define LOWER "lower"
#define UPPER "upper"

void materialValues(const QString exprStr, double lower, double upper, int count, QVector<double> &keys, QVector<double> &values)
{
    QString str = exprStr;

#ifdef DEAL_II_WITH_TBB
    tbb::mutex::scoped_lock lock(compileExpressionMutex);
#endif

    // replace "**" with "^"
    str = str.replace("**", "^");

    // expression
    exprtk::expression<double> expr;

    // symbol table
    double t = 0.0;
    exprtk::symbol_table<double> symbolTable;
    symbolTable.add_variable("t", t);
    expr.register_symbol_table(symbolTable);

    // compile expression
    exprtk::parser<double> exprtkParser;
    if (exprtkParser.compile(str.toStdString(), expr))
    {
        double step = (upper - lower) / (count + 1);
        for (int i = 0; i < count; i++)
        {
            t = lower + i*step;

            keys.append(t);
            values.append(expr.value());
        }
    }
    else
    {
        /*
        QString str = QObject::tr("exprtk error: %1, expression: %2: ").
                arg(QString::fromStdString(exprtkParser.error())).
                arg(str);

        for (int i = 0; i < exprtkParser.error_count(); ++i)
        {
            exprtk::parser_error::type error = exprtkParser.get_error(i);

            str += QObject::tr("error: %1, position: %2, type: [%3], message: %4, expression: %5; ").
                    arg(i).
                    arg(error.token.position).
                    arg(QString::fromStdString(exprtk::parser_error::to_str(error.mode))).
                    arg(QString::fromStdString(error.diagnostic)).
                    arg(str);
        }

        qInfo() << str;
        */
    }
}

void LibraryMaterial::read(const QString &fileName)
{
    if (QFile::exists(fileName))
    {
        properties.clear();
        
        // open file
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly))
        {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            QJsonObject rootJson = doc.object();
            
            QJsonObject generalJson = rootJson[GENERAL].toObject();
            name = generalJson[NAME].toString();
            description = generalJson[DESCRIPTION].toString();
            
            QJsonArray propertiesJson = rootJson[PROPERTIES].toArray();
            for (int i = 0; i < propertiesJson.size(); i++)
            {
                Property prop;
                
                QJsonObject propertyJson = propertiesJson[i].toObject();
                
                prop.name = propertyJson[NAME].toString();
                prop.source = propertyJson[SOURCE].toString();
                prop.type = nonlinearityTypeFromStringKey(propertyJson[TYPE].toString());
                
                prop.shortname = propertyJson[SHORTNAME].toString();
                prop.unit = propertyJson[UNIT].toString();
                
                prop.independent_shortname = propertyJson[INDEPENDENT_SHORTNAME].toString();
                prop.independent_unit = propertyJson[INDEPENDENT_UNIT].toString();
                
                QJsonObject valuesJson = propertyJson[VALUES].toObject();
                
                // constant
                prop.value_constant = valuesJson[CONSTANT].toDouble();

                // table
                if (!valuesJson[TABLE].isNull())
                {
                    QJsonObject tableJson = valuesJson[TABLE].toObject();

                    QString keysTrimmed = tableJson[KEYS].toString().endsWith(";") ? tableJson[KEYS].toString().left(tableJson[KEYS].toString().count() - 1) : tableJson[KEYS].toString();
                    QString valuesTrimmed = tableJson[VALUES].toString().endsWith(";") ? tableJson[VALUES].toString().left(tableJson[VALUES].toString().count() - 1) : tableJson[VALUES].toString();

                    QStringList keysString = keysTrimmed.split(";");
                    QStringList valuesString = valuesTrimmed.split(";");
                    
                    for (int j = 0; j < keysString.size(); j++)
                    {
                        QString key = keysString[j].trimmed();
                        QString value = valuesString[j].trimmed();

                        if (!key.isEmpty() && !value.isEmpty())
                        {
                            prop.value_table_keys.append(keysString[j].toDouble());
                            prop.value_table_values.append(valuesString[j].toDouble());
                        }
                    }
                }
                
                // function
                if (!valuesJson[FUNCTION].isNull())
                {
                    QJsonObject functionJson = valuesJson[FUNCTION].toObject();

                    prop.value_function_function = functionJson[FUNCTION].toString();
                    prop.value_function_lower = functionJson[LOWER].toDouble();
                    prop.value_function_upper = functionJson[UPPER].toDouble();
                }
                
                properties.append(prop);
            }
        }
    }
}

void LibraryMaterial::write(const QString &fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << QString("Couldn't write material file '%1'.").arg(fileName);
        return;
    }

    // root object
    QJsonObject rootJson;

    // general
    QJsonObject generalJson;

    generalJson[VERSION] = 1;
    generalJson[NAME] = name;
    generalJson[DESCRIPTION] = description;

    rootJson[GENERAL] = generalJson;

    // properties
    QJsonArray propertiesJson;
    for (unsigned int i = 0; i < properties.size(); i++)
    {
        QJsonObject propertyJson;

        propertyJson[NAME] = properties[i].name;
        propertyJson[SOURCE] = properties[i].source;
        propertyJson[TYPE] = nonlinearityTypeToStringKey(properties[i].type);

        propertyJson[SHORTNAME] = properties[i].shortname;
        propertyJson[UNIT] = properties[i].unit;

        propertyJson[INDEPENDENT_SHORTNAME] = properties[i].independent_shortname;
        propertyJson[INDEPENDENT_UNIT] = properties[i].independent_unit;

        QJsonObject valuesJson;

        // constant
        valuesJson[CONSTANT] = properties[i].value_constant;

        if (properties[i].type == NonlinearityType::Table)
        {
            QStringList keysString;
            QStringList valuesString;

            QJsonObject tableJson;
            assert(properties[i].value_table_keys.size() == properties[i].value_table_values.size());
            for (int j = 0; j < properties[i].value_table_keys.size(); j++)
            {
                keysString.append(QString::number(properties[i].value_table_keys[j]));
                valuesString.append(QString::number(properties[i].value_table_values[j]));
            }

            tableJson[KEYS] = keysString.join(";");
            tableJson[VALUES] = valuesString.join(";");

            valuesJson[TABLE] = tableJson;
        }
        else if (properties[i].type == NonlinearityType::Function)
        {
            QJsonObject functionJson;
            functionJson[FUNCTION] = properties[i].value_function_function;
            functionJson[LOWER] = properties[i].value_function_lower;
            functionJson[UPPER] = properties[i].value_function_upper;

            valuesJson[FUNCTION] = functionJson;
        }
        else
            assert(0);

        propertyJson[VALUES] = valuesJson;
        propertiesJson.append(propertyJson);
    }

    rootJson[PROPERTIES] = propertiesJson;

    // save to file
    QJsonDocument doc(rootJson);
    file.write(doc.toJson(QJsonDocument::Indented));
}

MaterialEditDialog::MaterialEditDialog(const QString &fileName, QWidget *parent)
    : QDialog(parent),
    m_fileName(fileName)
{
    setWindowTitle(tr("Material editor"));
    setWindowFlags(Qt::WindowFlags() | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);

    createControls();
    
    readMaterial();
    
    QSettings settings;
    restoreGeometry(settings.value("MaterialEditDialog/Geometry", saveGeometry()).toByteArray());
}

MaterialEditDialog::~MaterialEditDialog()
{
    QSettings settings;
    settings.setValue("MaterialEditDialog/Geometry", saveGeometry());
}

int MaterialEditDialog::showDialog()
{
    return exec();
}

void MaterialEditDialog::createControls()
{    
    lstProperties = new QListWidget(this);
    lstProperties->setMouseTracking(true);
    lstProperties->setMaximumWidth(200);
    
    connect(lstProperties, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(doPropertyChanged(QListWidgetItem *, QListWidgetItem *)));
    
    txtName = new QLineEdit();
    txtDescription = new QLineEdit();
    
    QGridLayout *layoutNameAndDescription = new QGridLayout();
    layoutNameAndDescription->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutNameAndDescription->addWidget(txtName, 0, 1);
    layoutNameAndDescription->addWidget(new QLabel(tr("Description:")), 1, 0);
    layoutNameAndDescription->addWidget(txtDescription, 2, 0, 1, 3);
    
    QPushButton *btnAddProperty = new QPushButton(tr("Add..."));
    btnAddProperty->setDefault(false);
    
    QAction *actAddCustom = new QAction(tr("Custom property"), this);
    connect(actAddCustom, SIGNAL(triggered()), this, SLOT(addProperty()));
    
    // TODO: more general
    QAction *actAddThermalConductivity = new QAction(tr("Thermal conductivity"), this);
    connect(actAddThermalConductivity, SIGNAL(triggered()), this, SLOT(addPropertyThermalConductivity()));
    QAction *actAddSpecificHeat = new QAction(tr("Specific heat"), this);
    connect(actAddSpecificHeat, SIGNAL(triggered()), this, SLOT(addPropertySpecificHeat()));
    QAction *actAddDensity = new QAction(tr("Density"), this);
    connect(actAddDensity, SIGNAL(triggered()), this, SLOT(addPropertyDensity()));
    QAction *actAddMagneticPermeability = new QAction(tr("Magnetic permeability"), this);
    connect(actAddMagneticPermeability, SIGNAL(triggered()), this, SLOT(addPropertyMagneticPermeability()));
    
    QMenu *menu = new QMenu();
    menu->addAction(actAddCustom);
    menu->addSeparator();
    menu->addAction(actAddThermalConductivity);
    menu->addAction(actAddSpecificHeat);
    menu->addAction(actAddDensity);
    menu->addSeparator();
    menu->addAction(actAddMagneticPermeability);
    
    btnAddProperty->setMenu(menu);
    
    btnDeleteProperty = new QPushButton(tr("Delete"));
    btnDeleteProperty->setDefault(false);
    connect(btnDeleteProperty, SIGNAL(clicked()), this, SLOT(deleteProperty()));
    
    QGridLayout *layoutList = new QGridLayout();
    layoutList->addWidget(lstProperties, 0, 0, 1, 2);
    layoutList->addWidget(btnAddProperty, 1, 0);
    layoutList->addWidget(btnDeleteProperty, 1, 1);
    
    propertyGUI = createPropertyGUI();
    
    QHBoxLayout *layoutNonlinearProperties = new QHBoxLayout();
    layoutNonlinearProperties->addLayout(layoutList);
    layoutNonlinearProperties->addWidget(propertyGUI, 1);
    
    // table
    txtPropertyTableKeys = new QPlainTextEdit();
    txtPropertyTableValues = new QPlainTextEdit();
    
    QGridLayout *layoutTable = new QGridLayout();
    layoutTable->addWidget(new QLabel(tr("Keys:")), 0, 0);
    layoutTable->addWidget(txtPropertyTableKeys, 1, 0);
    layoutTable->addWidget(new QLabel(tr("Values:")), 0, 1);
    layoutTable->addWidget(txtPropertyTableValues, 1, 1);
    
    widNonlinearTable = new QGroupBox(tr("Table"));
    widNonlinearTable->setLayout(layoutTable);
    
    // function
    txtPropertyFunction = new QPlainTextEdit(this);
    txtPropertyFunctionLower = new LineEditDouble(0.0);
    txtPropertyFunctionUpper = new LineEditDouble(0.0);
    
    QGridLayout *layoutFunction = new QGridLayout();
    layoutFunction->addWidget(txtPropertyFunction, 0, 0, 1, 4);
    layoutFunction->addWidget(new QLabel(tr("From:")), 1, 0);
    layoutFunction->addWidget(txtPropertyFunctionLower, 1, 1);
    layoutFunction->addWidget(new QLabel(tr("To:")), 1, 2);
    layoutFunction->addWidget(txtPropertyFunctionUpper, 1, 3);
    layoutFunction->setRowStretch(0, 1);
    
    widNonlinearFunction = new QGroupBox(tr("Nonlinear function"));
    widNonlinearFunction->setLayout(layoutFunction);
    
    chartNonlinear = new QCustomPlot();
    chartNonlinear->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    chartNonlinear->setMinimumHeight(120);
    chartNonlinear->addGraph();
    chartNonlinear->graph(0)->setLineStyle(QCPGraph::lsLine);
    
    QHBoxLayout *layoutChartNonlinear = new QHBoxLayout();
    layoutChartNonlinear->addWidget(chartNonlinear);
    
    widChartNonlinear = new QGroupBox(tr("Chart"));
    widChartNonlinear->setLayout(layoutChartNonlinear);
    
    layoutNonlinearType = new QStackedLayout();
    layoutNonlinearType->addWidget(widNonlinearFunction);
    layoutNonlinearType->addWidget(widNonlinearTable);
    
    QHBoxLayout *layoutNonlinearChart = new QHBoxLayout();
    layoutNonlinearChart->addLayout(layoutNonlinearType, 2);
    layoutNonlinearChart->addWidget(widChartNonlinear, 1);
    
    QVBoxLayout *layoutNonlinear = new QVBoxLayout();
    layoutNonlinear->addLayout(layoutNonlinearProperties);
    layoutNonlinear->addLayout(layoutNonlinearChart, 1);
    
    // dialog buttons
    QPushButton *btnPlot = new QPushButton(tr("Plot"));
    connect(btnPlot, SIGNAL(clicked()), this, SLOT(drawChart()));
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnPlot, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addLayout(layoutNameAndDescription);
    layout->addLayout(layoutNonlinear, 1);
    layout->addStretch();
    layout->addWidget(buttonBox);
    
    setLayout(layout);
}

QWidget *MaterialEditDialog::createPropertyGUI()
{
    txtPropertyName = new QLineEdit();
    txtPropertyShortname = new QLineEdit();
    connect(txtPropertyShortname, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    txtPropertyUnit = new QLineEdit();
    connect(txtPropertyUnit, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    txtPropertySource = new QLineEdit();
    txtPropertyIndependentVariableShortname = new QLineEdit();
    connect(txtPropertyIndependentVariableShortname, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    txtPropertyIndependentVariableUnit = new QLineEdit();
    connect(txtPropertyIndependentVariableUnit, SIGNAL(textChanged(QString)), this, SLOT(setFunctionLabel()));
    cmbPropertyNonlinearityType = new QComboBox();
    cmbPropertyNonlinearityType->addItem(tr("Function"), LibraryMaterial::Function);
    cmbPropertyNonlinearityType->addItem(tr("Table"), LibraryMaterial::Table);
    connect(cmbPropertyNonlinearityType, SIGNAL(activated(int)), this, SLOT(doNonlinearDependenceChanged(int)));
    lblPropertyFunction = new QLabel();
    
    // constant
    txtPropertyConstant = new LineEditDouble(0.0);
    
    // table and function tab
    QGridLayout *layoutProperty = new QGridLayout();
    layoutProperty->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutProperty->addWidget(txtPropertyName, 0, 1, 1, 3);
    layoutProperty->addWidget(new QLabel(tr("Source:")), 1, 0);
    layoutProperty->addWidget(txtPropertySource, 1, 1, 1, 3);
    layoutProperty->addWidget(new QLabel(tr("Shortname:")), 2, 0);
    layoutProperty->addWidget(txtPropertyShortname, 2, 1);
    layoutProperty->addWidget(new QLabel(tr("Unit:")), 3, 0);
    layoutProperty->addWidget(txtPropertyUnit, 3, 1);
    layoutProperty->addWidget(new QLabel(tr("Constant value:")), 4, 0);
    layoutProperty->addWidget(txtPropertyConstant, 4, 1);
    layoutProperty->addWidget(new QLabel(tr("Kind of nonlinearity:")), 5, 0);
    layoutProperty->addWidget(cmbPropertyNonlinearityType, 5, 1);
    layoutProperty->addWidget(new QLabel(tr("Ind. var. shortname:")), 2, 2);
    layoutProperty->addWidget(txtPropertyIndependentVariableShortname, 2, 3);
    layoutProperty->addWidget(new QLabel(tr("Ind. var. unit:")), 3, 2);
    layoutProperty->addWidget(txtPropertyIndependentVariableUnit, 3, 3);
    layoutProperty->addWidget(new QLabel(tr("Function:")), 5, 2);
    layoutProperty->addWidget(lblPropertyFunction, 5, 3);
    layoutProperty->setRowStretch(11, 1);
    
    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutProperty);
    
    return widget;
}

void MaterialEditDialog::readMaterial()
{
    if (QFile::exists(m_fileName))
    {
        m_material.read(m_fileName);
        
        txtName->setText(m_material.name);
        txtDescription->setText(m_material.description);
        
        lstProperties->clear();
        
        // properties
        for (unsigned int i = 0; i < m_material.properties.size(); i++)
        {
            LibraryMaterial::Property prop = m_material.properties[i];
            
            // item
            QListWidgetItem *item = new QListWidgetItem(lstProperties);
            item->setText(prop.name);
            item->setData(Qt::UserRole, lstProperties->count() - 1);
            
            lstProperties->addItem(item);
        }
        
        propertyGUI->setEnabled(lstProperties->count() > 0);
        btnDeleteProperty->setEnabled(lstProperties->count() > 0);
        
        if (lstProperties->count() > 0)
        {
            lstProperties->setCurrentRow(0);
            readProperty(m_material.properties[0]);
        }
        else
        {
            // readProperty();
        }
    }
}

bool MaterialEditDialog::writeMaterial()
{
    if (lstProperties->currentItem())
        m_material.properties.replace(lstProperties->currentRow(), writeProperty());
    
    QFileInfo fileInfo(m_fileName);
    if (fileInfo.baseName() != txtName->text())
    {
        bool move = true;
        
        // rename file
        QString newFilename = fileInfo.absolutePath() + "/" + txtName->text() + ".mat";
        if (QFile::exists(newFilename))
            move = (QMessageBox::question(this, tr("File exists"), tr("Material '%1' already exists. Do you wish replace this file?").arg(newFilename)) == 0);
        
        if (move)
        {
            QFile::remove(m_fileName);
            m_fileName = newFilename;
        }
    }
    
    // general
    m_material.name = txtName->text();
    m_material.description = txtDescription->text();
    m_material.write(m_fileName);
    
    return true;
}

void MaterialEditDialog::addProperty(const QString &name, const QString &shortname, const QString &unit, LibraryMaterial::NonlinearityType nonlinearityType,
                                     const QString &indepedentShortname, const QString &indepedentUnit)
{
    bool ok = false;
    QString propName = name;
    
    if (propName.isEmpty())
        propName = QInputDialog::getText(this, tr("Add property"),
                                         tr("Name:"), QLineEdit::Normal, "", &ok);
    else
        ok = true;
    
    if (ok && !propName.isEmpty())
    {
        LibraryMaterial::Property prop;
        prop.name = propName;
        prop.source = "";
        
        prop.shortname = shortname;
        
        prop.unit = unit;
        prop.type = nonlinearityType;
        
        prop.independent_shortname = indepedentShortname;
        prop.independent_unit = indepedentUnit;
        
        prop.value_constant = 0;
        
        prop.value_function_function = "";
        prop.value_function_lower = 0;
        prop.value_function_upper = 100;
        
        m_material.properties.append(prop);
        
        // item
        QListWidgetItem *item = new QListWidgetItem(lstProperties);
        item->setText(prop.name);
        
        lstProperties->addItem(item);
        lstProperties->setCurrentItem(item);
        propertyGUI->setEnabled(lstProperties->count() > 0);
        btnDeleteProperty->setEnabled(lstProperties->count() > 0);
        
        readProperty(prop);
    }
}

void MaterialEditDialog::deleteProperty()
{
    if (lstProperties->currentItem())
    {
        if (QMessageBox::question(this, tr("Delete property"),
                                  tr("Property '%1' will be permanently deleted. Are you sure?").arg(txtPropertyName->text()),
                                  tr("&Yes"), tr("&No")) == 0)
        {
            int row = lstProperties->row(lstProperties->currentItem());
            lstProperties->takeItem(row);
            m_material.properties.removeAt(row);
            
            propertyGUI->setEnabled(lstProperties->count() > 0);
            btnDeleteProperty->setEnabled(lstProperties->count() > 0);
            
            if (lstProperties->count() > 0)
            {
                lstProperties->setCurrentRow(0);
                readProperty(m_material.properties.at(0));
            }
            else
            {
                // readProperty();
            }
        }
    }
}

void MaterialEditDialog::readProperty(LibraryMaterial::Property prop)
{
    txtPropertyName->setText(prop.name);
    txtPropertyShortname->setText(prop.shortname);
    txtPropertyUnit->setText(prop.unit);
    txtPropertySource->setText(prop.source);
    cmbPropertyNonlinearityType->setCurrentIndex(cmbPropertyNonlinearityType->findData(prop.type));
    txtPropertyIndependentVariableShortname->setText(prop.independent_shortname);
    txtPropertyIndependentVariableUnit->setText(prop.independent_unit);
    
    // clear
    txtPropertyConstant->setValue(0.0);
    txtPropertyTableKeys->clear();
    txtPropertyTableValues->clear();
    txtPropertyFunction->clear();
    txtPropertyFunctionLower->setValue(0.0);
    txtPropertyFunctionUpper->setValue(0.0);
    
    // constant
    txtPropertyConstant->setValue(prop.value_constant);
    
    if (prop.type == LibraryMaterial::Table)
    {
        // table
        for (int i = 0; i < prop.value_table_keys.size(); i++)
        {
            txtPropertyTableKeys->appendPlainText(QString::number(prop.value_table_keys[i]));
            txtPropertyTableValues->appendPlainText(QString::number(prop.value_table_values[i]));
        }
    }
    else if (prop.type == LibraryMaterial::Function)
    {
        // function
        txtPropertyFunction->setPlainText(prop.value_function_function);
        txtPropertyFunctionLower->setValue(prop.value_function_lower);
        txtPropertyFunctionUpper->setValue(prop.value_function_upper);
    }

    doNonlinearDependenceChanged(cmbPropertyNonlinearityType->currentIndex());
    
    // draw chart
    drawChart();
}

void MaterialEditDialog::setFunctionLabel()
{
    lblPropertyFunction->setText(QString("%1 (%2) = function(%3 (%4))").
                                 arg(txtPropertyShortname->text()).
                                 arg(txtPropertyUnit->text()).
                                 arg(txtPropertyIndependentVariableShortname->text()).
                                 arg(txtPropertyIndependentVariableUnit->text()));
}

void MaterialEditDialog::drawChart()
{
    // assert(0);
    
    QVector<double> keys;
    QVector<double> values;

    if (((LibraryMaterial::NonlinearityType)
         cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == LibraryMaterial::Function)
    {
        materialValues(txtPropertyFunction->toPlainText(), txtPropertyFunctionLower->value(), txtPropertyFunctionUpper->value(), 500, keys, values);
    }
    else
    {
        QStringList keysString = txtPropertyTableKeys->toPlainText().split("\n");
        QStringList valuesString = txtPropertyTableValues->toPlainText().split("\n");
        
        for (int j = 0; j < keysString.size(); j++)
        {
            if ((!keysString.at(j).isEmpty()) && (j < valuesString.count()) && (!valuesString.at(j).isEmpty()))
            {
                keys.append(keysString.at(j).toDouble());
                values.append(valuesString.at(j).toDouble());
            }
        }
    }

    chartNonlinear->graph(0)->setData(keys, values);
    chartNonlinear->rescaleAxes();
    chartNonlinear->replot(QCustomPlot::rpQueuedRefresh);
}

LibraryMaterial::Property MaterialEditDialog::writeProperty()
{
    LibraryMaterial::Property prop;
    
    // property
    prop.name = txtPropertyName->text();
    prop.source = txtPropertySource->text();
    prop.type = (LibraryMaterial::NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt();

    prop.shortname = txtPropertyShortname->text();
    prop.unit = txtPropertyUnit->text();
    
    prop.independent_shortname = txtPropertyIndependentVariableShortname->text();
    prop.independent_unit = txtPropertyIndependentVariableUnit->text();

    // constant
    prop.value_constant = txtPropertyConstant->value();
    
    // dependence
    if ((LibraryMaterial::NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()
            == LibraryMaterial::Table)
    {
        QStringList keys = txtPropertyTableKeys->toPlainText().split("\n");
        QStringList values = txtPropertyTableValues->toPlainText().split("\n");
        for (int i = 0; i < keys.size(); i++)
        {
            prop.value_table_keys.append(keys[i].toDouble());
            prop.value_table_values.append(values[i].toDouble());
        }
    }
    else if ((LibraryMaterial::NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()
             == LibraryMaterial::Function)
    {
        prop.value_function_function = txtPropertyFunction->toPlainText();
        prop.value_function_lower = txtPropertyFunctionLower->value();
        prop.value_function_upper = txtPropertyFunctionUpper->value();
    }
    else
        assert(0);
    
    return prop;
}

void MaterialEditDialog::doNonlinearDependenceChanged(int index)
{
    if (((LibraryMaterial::NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == LibraryMaterial::Function)
        layoutNonlinearType->setCurrentWidget(widNonlinearFunction);
    else if (((LibraryMaterial::NonlinearityType) cmbPropertyNonlinearityType->itemData(cmbPropertyNonlinearityType->currentIndex()).toInt()) == LibraryMaterial::Table)
        layoutNonlinearType->setCurrentWidget(widNonlinearTable);
}

void MaterialEditDialog::doPropertyChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (previous)
        m_material.properties.replace(lstProperties->row(previous), writeProperty());
    
    if (current)
        readProperty(m_material.properties.at(lstProperties->row(current)));
}

void MaterialEditDialog::doAccept()
{
    if (writeMaterial())
        accept();
}

// ***********************************************************************************************************

MaterialBrowserDialog::MaterialBrowserDialog(QWidget *parent) : QDialog(parent),
    m_select(false)
{
    setWindowTitle(tr("Material Browser"));
    setModal(true);

    webEdit = new QTextBrowser();
    webEdit->setReadOnly(true);
    webEdit->setOpenLinks(false);
    webEdit->setOpenExternalLinks(false);
    QObject::connect(webEdit, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    
    trvMaterial = new QTreeWidget(this);
    trvMaterial->setMouseTracking(true);
    trvMaterial->setColumnCount(1);
    trvMaterial->setIconSize(QSize(24, 24));
    trvMaterial->setHeaderHidden(true);
    trvMaterial->setMinimumWidth(230);
    
    connect(trvMaterial, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(doItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(trvMaterial, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(doItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
    
    // btnNew = new QPushButton();
    // btnNew->setText(tr("New"));
    // btnNew->setDefault(false);
    // connect(btnNew, SIGNAL(clicked()), this, SLOT(doNew()));
    
    btnEdit = new QPushButton();
    btnEdit->setText(tr("Edit"));
    btnEdit->setDefault(false);
    btnEdit->setEnabled(false);
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(doEdit()));
    
    btnDelete = new QPushButton();
    btnDelete->setText(tr("Delete"));
    btnDelete->setDefault(false);
    btnDelete->setEnabled(false);
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(doDelete()));
    
    QGridLayout *layoutProperties = new QGridLayout();
    layoutProperties->addWidget(trvMaterial, 0, 0, 1, 3);
    // layoutProperties->addWidget(btnNew, 2, 0);
    layoutProperties->addWidget(btnEdit, 2, 1);
    layoutProperties->addWidget(btnDelete, 2, 2);
    
    QHBoxLayout *layoutSurface = new QHBoxLayout();
    layoutSurface->addLayout(layoutProperties);
    layoutSurface->addWidget(webEdit, 1);
    
    QWidget *widget = new QWidget();
    widget->setLayout(layoutSurface);
    
    QPushButton *btnClose = new QPushButton();
    btnClose->setText(tr("Close"));
    btnClose->setDefault(true);
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    
    QHBoxLayout *layoutButtons = new QHBoxLayout();
    layoutButtons->addStretch(1);
    layoutButtons->addWidget(btnClose);
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(widget, 1);
    layout->addStretch();
    layout->addLayout(layoutButtons);
    
    setLayout(layout);
    
    readMaterials();
    
    QSettings settings;
    restoreGeometry(settings.value("MaterialBrowserDialog/Geometry", saveGeometry()).toByteArray());
}

MaterialBrowserDialog::~MaterialBrowserDialog()
{
    QSettings settings;
    settings.setValue("MaterialBrowserDialog/Geometry", saveGeometry());
}

int MaterialBrowserDialog::showDialog(bool select)
{
    m_select = select;
    return exec();
}

void MaterialBrowserDialog::doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    webEdit->setHtml("");
    btnEdit->setEnabled(false);
    btnDelete->setEnabled(false);
    
    if (current)
    {
        m_selectedFilename = current->data(0, Qt::UserRole).toString();
        if (!m_selectedFilename.isEmpty())
        {
            materialInfo(m_selectedFilename);
            if (QFileInfo(m_selectedFilename).isWritable())
            {
                // btnEdit->setEnabled(true);
                // btnDelete->setEnabled(true);
            }
        }
    }
}

void MaterialBrowserDialog::doItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (trvMaterial->currentItem())
    {
        doEdit();
    }
}

void MaterialBrowserDialog::readMaterials()
{
    // clear listview
    trvMaterial->clear();
    
    // read materials
    QDir dirSystem(QString("%1/resources/materials").arg(Agros::dataDir()));
    dirSystem.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    readMaterials(dirSystem, trvMaterial->invisibleRootItem());
    
    if (!m_selectedFilename.isEmpty())
    {
        // qDebug() << QFileInfo(m_selectedFilename).baseName();
        QList<QTreeWidgetItem *> items = trvMaterial->findItems(QFileInfo(m_selectedFilename).baseName(), Qt::MatchExactly);
        // qDebug() << items.count();
        if (items.count() >= 1)
        {
            trvMaterial->setCurrentItem(items.at(items.count() - 1));
            materialInfo(m_selectedFilename);
        }
    }
}

void MaterialBrowserDialog::readMaterials(QDir dir, QTreeWidgetItem *parentItem)
{
    QFileInfoList listExamples = dir.entryInfoList();
    for (int i = 0; i < listExamples.size(); ++i)
    {
        QFileInfo fileInfo = listExamples.at(i);
        if (fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;
        
        if (fileInfo.isDir())
        {
            QFont fnt = trvMaterial->font();
            fnt.setBold(true);
            
            QTreeWidgetItem *dirItem = new QTreeWidgetItem(parentItem);
            dirItem->setText(0, fileInfo.fileName());
            dirItem->setFont(0, fnt);
            dirItem->setExpanded(true);
            
            // recursive read
            readMaterials(fileInfo.absoluteFilePath(), dirItem);
        }
        else if (fileInfo.suffix() == "json")
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
            item->setText(0, fileInfo.baseName());
            item->setData(0, Qt::UserRole, fileInfo.absoluteFilePath());
        }
    }
}

void MaterialBrowserDialog::materialInfo(const QString &fileName)
{
    // template
    std::string info;
    ctemplate::TemplateDictionary materialInfo("info");
    
    if (QFile::exists(fileName))
    {
        LibraryMaterial material;
        material.read(fileName);
        
        materialInfo.SetValue("PANELS_DIRECTORY", QUrl::fromLocalFile(QString("%1%2").arg(QDir(Agros::dataDir()).absolutePath()).arg(TEMPLATEROOT)).toString().toStdString());
        
        materialInfo.SetValue("NAME", material.name.toStdString());
        materialInfo.SetValue("DESCRIPTION", material.description.toStdString());
        
        // properties
        for (int i = 0; i < material.properties.size(); i++)
        {
            LibraryMaterial::Property prop =  material.properties[i];
            
            ctemplate::TemplateDictionary *propSection = materialInfo.AddSectionDictionary("PROPERTIES_SECTION");
            
            propSection->SetValue("PROPERTY_LABEL", prop.name.toStdString());
            propSection->SetValue("PROPERTY_SOURCE", prop.source.toStdString());
            
            propSection->SetValue("PROPERTY_SHORTNAME_LABEL", tr("Variable:").toStdString());
            propSection->SetValue("PROPERTY_SHORTNAME", prop.shortname.toStdString());
            propSection->SetValue("PROPERTY_UNIT_LABEL", tr("Unit:").toStdString());
            propSection->SetValue("PROPERTY_UNIT", prop.unit.toStdString());
            
            propSection->SetValue("PROPERTY_INDEPENDENT_SHORTNAME_LABEL", tr("Independent variable:").toStdString());
            propSection->SetValue("PROPERTY_INDEPENDENT_SHORTNAME", prop.independent_shortname.toStdString());
            propSection->SetValue("PROPERTY_INDEPENDENT_UNIT_LABEL", tr("Independent unit:").toStdString());
            propSection->SetValue("PROPERTY_INDEPENDENT_UNIT", prop.independent_unit.toStdString());
            
            if (m_select)
                propSection->ShowSection("PROPERTY_SELECTABLE");
            
            // constant
            propSection->SetValue("PROPERTY_CONSTANT_LABEL", tr("Constant:").toStdString());
            propSection->SetValue("PROPERTY_CONSTANT", QString::number(prop.value_constant).toStdString());
            
            // nonlinearity
            QVector<double> keys;
            QVector<double> values;

            // function
            if ((prop.type == LibraryMaterial::Function) && (!prop.value_function_function.trimmed().isEmpty()))
            {
                materialValues(prop.value_function_function, prop.value_function_lower, prop.value_function_upper, 500, keys, values);
            }

            // table
            if ((prop.type == LibraryMaterial::Table) && (prop.value_table_keys.size() > 0))
            {
                for (int i = 0; i < prop.value_table_keys.size(); i++)
                {
                    keys = prop.value_table_keys;
                    values = prop.value_table_values;
                }
            }

            assert(keys.size() == values.size());
            if (keys.size() > 0)
            {
                double minKey = numeric_limits<double>::max();
                double maxKey = -numeric_limits<double>::max();
                double minValue = numeric_limits<double>::max();
                double maxValue = -numeric_limits<double>::max();
                for (int i = 0; i < keys.size(); i++)
                {
                    minKey = fmin(minKey, keys[i]);
                    maxKey = fmax(maxKey, keys[i]);
                    minValue = fmin(minValue, values[i]);
                    maxValue = fmax(maxValue, values[i]);
                }

                QPen pen;
                pen.setColor(Qt::darkGray);
                pen.setWidth(2);

                QCustomPlot customPlot;
                // create graph and assign data to it:
                customPlot.addGraph();
                customPlot.graph(0)->setData(keys, values);
                customPlot.graph(0)->setLineStyle(QCPGraph::lsLine);
                customPlot.graph(0)->setPen(pen);
                customPlot.graph(0)->setBrush(QBrush(QColor(255, 0, 0, 20)));
                // give the axes some labels:
                customPlot.xAxis->setLabel(QString("%0 (%1)").arg(prop.independent_shortname).arg(prop.independent_unit));
                customPlot.yAxis->setLabel(QString("%0 (%1)").arg(prop.shortname).arg(prop.unit));
                // set axes ranges, so we see all data:
                customPlot.xAxis->setRange(minKey, maxKey);
                customPlot.yAxis->setRange(minValue, maxValue);
                customPlot.graph(0)->rescaleAxes();
                customPlot.replot();

                QDateTime currentTime(QDateTime::currentDateTime());
                QString fn = QString("%1/%2.png").arg(tempProblemDir()).arg(currentTime.toString("yyyy-MM-dd-hh-mm-ss-zzz"));
                customPlot.savePng(fn, 500, 180);
                propSection->SetValue("PROPERTY_NONLINEAR_PNG", fn.toStdString());

                propSection->ShowSection("PROPERTY_NONLINEAR");
            }
        }
    }

    ctemplate::ExpandTemplate(compatibleFilename(Agros::dataDir() + TEMPLATEROOT + "/material.tpl").toStdString(), ctemplate::DO_NOT_STRIP, &materialInfo, &info);

    webEdit->setHtml(QString::fromStdString(info));
}

void MaterialBrowserDialog::linkClicked(const QUrl &url)
{
    QString search = "property?";
    if (url.toString().contains(search))
    {
        m_selected_x.clear();
        m_selected_y.clear();
        m_selected_constant = 0.0;

        QStringList keysString = QUrlQuery(url).queryItemValue("x").split(",");
        QStringList valuesString = QUrlQuery(url).queryItemValue("y").split(",");
        m_selected_constant = QUrlQuery(url).queryItemValue("constant").toDouble();

        for (int j = 0; j < keysString.size(); j++)
        {
            m_selected_x.append(keysString.at(j).toDouble());
            m_selected_y.append(valuesString.at(j).toDouble());
        }

        accept();
    }
}

//void MaterialBrowserDialog::doNew()
//{
//    bool ok = false;
//    QString name = QInputDialog::getText(this, tr("Add custom material"),
//                                         tr("Name:"), QLineEdit::Normal, "", &ok);
//    if (ok && !name.isEmpty())
//    {
//        QString fileName = QString("%1/materials/%2.json").arg(userDataDir()).arg(name);
//        if (QFile::exists(fileName))
//        {
//            QMessageBox::warning(this, tr("Material"), tr("Material already exists."));
//            return;
//        }

//        LibraryMaterial material;
//        material.name = name;
//        material.write(fileName);

//        // select item and edit
//        m_selectedFilename = fileName;
//        readMaterials();

//        doEdit();
//    }
//}

void MaterialBrowserDialog::doEdit()
{
    if (!btnEdit->isEnabled())
        return;

    MaterialEditDialog dialog(m_selectedFilename, this);
    if (dialog.showDialog() == QDialog::Accepted)
    {
        m_selectedFilename = dialog.fileName();
        trvMaterial->currentItem()->setText(0, QFileInfo(m_selectedFilename).baseName());
        materialInfo(m_selectedFilename);
    }
}

void MaterialBrowserDialog::doDelete()
{
    QFileInfo fileInfo(m_selectedFilename);
    if (fileInfo.exists() && fileInfo.isWritable())
    {
        if (QMessageBox::question(this, tr("Delete material"),
                                  tr("Material '%1' will be pernamently deleted. Are you sure?").arg(fileInfo.baseName()),
                                  tr("&Yes"), tr("&No")) == 0)
        {
            QFile::remove(m_selectedFilename);
            m_selectedFilename = "";

            readMaterials();
        }
    }
}


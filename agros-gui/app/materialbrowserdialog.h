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

#ifndef MATERIALBROWSERDIALOG_H
#define MATERIALBROWSERDIALOG_H

#include "util/util.h"
#include "gui/other.h"
#include "gui/chart.h"

class LineEditDouble;

struct LibraryMaterial
{
    enum NonlinearityType
    {
        Function = 1,
        Table = 2
    };

    static inline QString nonlinearityTypeToStringKey(NonlinearityType type)
    {
        if (type == LibraryMaterial::Table)
            return "table";
        else
            return "function";
    }

    static inline NonlinearityType nonlinearityTypeFromStringKey(const QString &type)
    {
        if (type == "function")
            return LibraryMaterial::Function;
        else
            return LibraryMaterial::Table;
    }

    struct Property
    {
        Property() : name(""), source(""), type(LibraryMaterial::Table), shortname(""), unit(""),
            independent_shortname(""), independent_unit(""), value_constant(0.0),
            value_table_keys(QVector<double>()), value_table_values(QVector<double>()),
            value_function_function(""), value_function_lower(0.0), value_function_upper(0.0) {}

        QString name;
        QString source;
        NonlinearityType type;

        QString shortname;
        QString unit;

        QString independent_shortname;
        QString independent_unit;

        double value_constant;

        QVector<double> value_table_keys;
        QVector<double> value_table_values;

        QString value_function_function;
        double value_function_lower;
        double value_function_upper;
    };

    QString name;
    QString description;

    QList<Property> properties;

    void read(const QString &fileName);
    void write(const QString &fileName);
};

class MaterialEditDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialEditDialog(const QString &fileName, QWidget *parent = 0);
    ~MaterialEditDialog();

    int showDialog();
    inline QString fileName() { return m_fileName; }

protected:
    void createControls();
    void readMaterial();
    bool writeMaterial();

private:
    QString m_fileName;

    QLineEdit *txtName;
    QLineEdit *txtDescription;

    QListWidget *lstProperties;
    LibraryMaterial m_material;

    // properties
    QWidget *propertyGUI;

    QLineEdit *txtPropertyName;
    QLineEdit *txtPropertyShortname;
    QLineEdit *txtPropertyUnit;
    QLineEdit *txtPropertySource;
    QLineEdit *txtPropertyIndependentVariableShortname;
    QLineEdit *txtPropertyIndependentVariableUnit;
    QComboBox *cmbPropertyNonlinearityType;
    QLabel *lblPropertyFunction;

    LineEditDouble *txtPropertyConstant;

    QPlainTextEdit *txtPropertyTableKeys;
    QPlainTextEdit *txtPropertyTableValues;

    QPlainTextEdit *txtPropertyFunction;
    LineEditDouble *txtPropertyFunctionLower;
    LineEditDouble *txtPropertyFunctionUpper;

    QGroupBox *widNonlinearTable;
    QGroupBox *widNonlinearFunction;
    QGroupBox *widChartNonlinear;
    QStackedLayout *layoutNonlinearType;

    ChartView *chartView;
    QValueAxis *axisX;
    QValueAxis *axisFunction;
    QLineSeries *valueSeries;

    QPushButton *btnDeleteProperty;

    QWidget *createPropertyGUI();

    void readProperty(LibraryMaterial::Property prop);
    LibraryMaterial::Property writeProperty();

private slots:
    void doAccept();
    void addProperty(const QString &name = "", const QString &shortname = "", const QString &unit = "", LibraryMaterial::NonlinearityType nonlinearityType = LibraryMaterial::Table,
                     const QString &indepedentShortname = "", const QString &indepedentUnit = "");
    // TODO: more general
    inline void addPropertyThermalConductivity() { addProperty("Thermal conductivity", "<i>&lambda;</i>", "W/m.K", LibraryMaterial::Table, "<i>T</i>", "K"); }
    inline void addPropertySpecificHeat() { addProperty("Specific heat", "<i>c</i><sub>p</sub>", "J/kg.K", LibraryMaterial::Table, "<i>T</i>", "K"); }
    inline void addPropertyDensity() { addProperty("Density", "<i>&rho;</i>", "kg/m<sup>3</sup>", LibraryMaterial::Table, "<i>T</i>", "K"); }
    inline void addPropertyMagneticPermeability() { addProperty("Magnetic permeability", "<i>&mu;</i><sub>r</sub>", "-", LibraryMaterial::Table, "<i>B</i>", "T"); }

    void deleteProperty();

    void doPropertyChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void doNonlinearDependenceChanged(int index);

    void setFunctionLabel();
    void drawChart();
};

class MaterialBrowserDialog : public QDialog
{
    Q_OBJECT
public:
    MaterialBrowserDialog(QWidget *parent = 0);
    ~MaterialBrowserDialog();

    int showDialog(bool select = false);

    inline QList<double> x() const { return m_selected_x; }
    inline QList<double> y() const { return m_selected_y; }
    inline double constant() const { return m_selected_constant; }

protected:
    void readMaterials();
    void readMaterials(QDir dir, QTreeWidgetItem *parentItem);
    void materialInfo(const QString &fileName);    

private:
    QTextBrowser *webEdit;
    QTreeWidget *trvMaterial;
    QPushButton *btnNew;
    QPushButton *btnEdit;
    QPushButton *btnDelete;
    QString m_selectedFilename;

    QList<double> m_selected_x;
    QList<double> m_selected_y;
    double m_selected_constant;

    bool m_select;

private slots:
    void doItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void doItemDoubleClicked(QTreeWidgetItem *item, int column);

    void linkClicked(const QUrl &url);
    // void doNew();
    void doEdit();
    void doDelete();
};

#endif // MATERIALBROWSERDIALOG_H

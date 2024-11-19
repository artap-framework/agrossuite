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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "util/util.h"
#include "gui/other.h"

struct SceneViewSettings;
class ScriptEditor;

class LineEditDouble;
class ValueLineEdit;

class ConfigComputerDialog : public QDialog
{
    Q_OBJECT
public:
    ConfigComputerDialog(QWidget *parent);

private slots:
    void doAccept();
    void doReject();

private:
    // main
    QComboBox *cmbLanguage;

    // show result in line edit value widget
    QCheckBox *chkLineEditValueShowResult;
    QCheckBox *chkCheckNewVersion;

    // log std out
    QCheckBox *chkLogStdOut;
    QCheckBox *chkApplyStyle;

    // development
    QCheckBox *chkDiscreteSaveMatrixRHS;
    QComboBox *cmbDumpFormat;

    // cache
    QSpinBox *txtCacheSize;

    // scene font
    QSpinBox *txtRulersFontSizes;
    QSpinBox *txtPostFontSizes;

    // workspace other
    QCheckBox *chkShowRulersAndAxes;

    void load();
    void save();

    void createControls();
    QWidget *createMainWidget();
    QWidget *createSolverWidget();

    void fillComboBoxPhysicField(QComboBox *cmbPhysicField);
};


#endif // OPTIONSDIALOG_H

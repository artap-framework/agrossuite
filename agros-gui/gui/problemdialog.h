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

#ifndef PROBLEMDIALOG_H
#define PROBLEMDIALOG_H

#include "util/util.h"
#include "gui/other.h"

class ProblemConfig;
class FieldInfo;
class CouplingInfo;

class LineEditDouble;
class ValueLineEdit;

class ProblemWidget;

class CouplingsWidget : public QWidget
{
    Q_OBJECT
public:
    CouplingsWidget(QWidget *parent);

    void createContent();

signals:
    void changed();

public slots:
    void save();
    void refresh();
    void itemChanged(int index);

private:
    void createComboBoxes();

    QList<QComboBox *> m_comboBoxes;
    QList<QLabel *> m_labels;
};

class ProblemWidget : public QWidget
{
    Q_OBJECT
public:
    ProblemWidget(QWidget *parent = 0);

signals:
    void changed();

public slots:
    void load();
    void save();

private:
    QDialogButtonBox *buttonBox;

    CouplingsWidget *couplingsWidget;

    QComboBox *cmbCoordinateType;
    QComboBox *cmbMeshType;
    QSpinBox *txtMeshQualitySize;

    // harmonic
    QGroupBox *grpHarmonicAnalysis;
    ValueLineEdit *txtFrequency;

    // transient
    QGroupBox *grpTransientAnalysis;
    LineEditDouble *txtTransientTimeTotal;
    QLabel* lblTransientSteps;
    QSpinBox *txtTransientSteps;
    LineEditDouble *txtTransientTolerance;
    QCheckBox *chkTransientInitialStepSize;
    LineEditDouble *txtTransientInitialStepSize;
    QLabel *lblTransientTimeTotal;
    QSpinBox *txtTransientOrder;
    QComboBox *cmbTransientMethod;
    QLabel *lblTransientTimeStep;

    // couplings
    QGroupBox *grpCouplings;

    void createControls();

    void fillComboBox();

private slots:
    void transientChanged();
};

class ProblemDialog : public QDialog
{
    Q_OBJECT
public:
    ProblemDialog(QWidget *parent = 0);
    ~ProblemDialog();

private:
    ProblemWidget *problemWidget;

private slots:
    void doAccept();
};


#endif // PROBLEMDIALOG_H

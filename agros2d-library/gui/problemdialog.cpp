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

#include "problemdialog.h"
#include "fielddialog.h"

#include "solver/plugin_interface.h"

#include "util/global.h"

#include "scene.h"
#include "scenenode.h"
#include "pythonlab/pythonengine_agros.h"

#include "solver/module.h"

#include "solver/coupling.h"
#include "solver/problem_config.h"

#include "gui/lineeditdouble.h"
#include "gui/valuelineedit.h"
#include "gui/groupbox.h"
#include "gui/common.h"

#include "pythonlab/pythoneditor.h"

CouplingsWidget::CouplingsWidget(QWidget *parent) : QWidget(parent)
{
    Agros2D::problem()->synchronizeCouplings();

    createContent();

    connect(Agros2D::problem(), SIGNAL(fieldsChanged()), this, SLOT(refresh()));
}

void CouplingsWidget::createContent()
{
    QGridLayout *layoutTable;layoutTable = new QGridLayout();
    layoutTable->setContentsMargins(0, 0, 0, 0);
    layoutTable->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTable->setColumnStretch(1, 1);

    for (int i = 0; i < couplingList()->availableCouplings().count(); i++)
    {
        QLabel *label = new QLabel(this);
        label->setVisible(false);

        QComboBox *combo = new QComboBox(this);
        combo->setVisible(false);

        foreach (QString cp, couplingTypeStringKeys())
            combo->addItem(couplingTypeString(couplingTypeFromStringKey(cp)), couplingTypeFromStringKey(cp));

        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(itemChanged(int)));

        layoutTable->addWidget(label, i, 0);
        layoutTable->addWidget(combo, i, 1);

        m_labels.append(label);
        m_comboBoxes.append(combo);
    }

    setLayout(layoutTable);
}

void CouplingsWidget::save()
{
    int row = 0;
    foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
    {
        couplingInfo->setCouplingType((CouplingType) m_comboBoxes[row]->itemData(m_comboBoxes[row]->currentIndex()).toInt());

        row++;
    }
}

void CouplingsWidget::refresh()
{
    Agros2D::problem()->synchronizeCouplings();

    setUpdatesEnabled(false);

    int row = 0;
    foreach (CouplingInfo *couplingInfo, Agros2D::problem()->couplingInfos())
    {
        m_comboBoxes[row]->blockSignals(true);
        m_comboBoxes[row]->setUpdatesEnabled(false);
        m_comboBoxes[row]->setCurrentIndex(m_comboBoxes[row]->findData(couplingInfo->couplingType()));
        m_comboBoxes[row]->setUpdatesEnabled(true);
        m_comboBoxes[row]->setVisible(true);
        m_comboBoxes[row]->blockSignals(false);

        m_labels[row]->setText(couplingInfo->name());
        m_labels[row]->setVisible(true);

        row++;
    }
    for (int i = row; i < m_comboBoxes.count(); i++)
    {
        m_comboBoxes[i]->setVisible(false);
        m_labels[i]->setVisible(false);
    }

    setUpdatesEnabled(true);
}

void CouplingsWidget::itemChanged(int index)
{
    // qDebug() << "void CouplingsWidget::itemChanged(int index)";
    emit changed();
}

// ********************************************************************************************

ProblemWidget::ProblemWidget(QWidget *parent) : QWidget(parent)
{
    createControls();

    load();

    setMinimumSize(sizeHint());
}

void ProblemWidget::createControls()
{
    // fields toolbar
    /*
    fieldsToolbar = new FieldsToobar();
    QVBoxLayout *layoutFields = new QVBoxLayout();
    layoutFields->addWidget(fieldsToolbar);
    layoutFields->addStretch();

    QGroupBox *grpFieldsToolbar = new QGroupBox(tr("Physical fields"));
    grpFieldsToolbar->setLayout(layoutFields);
    */

    // problem
    cmbCoordinateType = new QComboBox();
    // mesh type
    cmbMeshType = new QComboBox();

    // general
    QGridLayout *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Coordinate type:")), 0, 0);
    layoutGeneral->addWidget(cmbCoordinateType, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Mesh type:")), 1, 0);
    layoutGeneral->addWidget(cmbMeshType, 1, 1);

    QGroupBox *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // harmonic
    txtFrequency = new ValueLineEdit();
    txtFrequency->setMinimum(0.0);

    // harmonic analysis
    QGridLayout *layoutHarmonicAnalysis = new QGridLayout();
    layoutHarmonicAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutHarmonicAnalysis->addWidget(new QLabel(tr("Frequency (Hz):")), 0, 0);
    layoutHarmonicAnalysis->addWidget(txtFrequency, 0, 1);

    grpHarmonicAnalysis = new QGroupBox(tr("Harmonic analysis"));
    grpHarmonicAnalysis->setLayout(layoutHarmonicAnalysis);

    // transient
    cmbTransientMethod = new QComboBox();
    txtTransientOrder = new QSpinBox();
    txtTransientOrder->setMinimum(1);
    txtTransientOrder->setMaximum(3);
    txtTransientTimeTotal = new LineEditDouble(1.0);
    txtTransientTimeTotal->setBottom(0.0);
    lblTransientTimeTotal = new QLabel("Total time");
    txtTransientTolerance = new LineEditDouble(0.1);
    txtTransientTolerance->setBottom(0.0);
    chkTransientInitialStepSize = new QCheckBox(this);
    txtTransientInitialStepSize = new LineEditDouble(0.01);
    txtTransientInitialStepSize->setBottom(0);
    txtTransientSteps = new QSpinBox();
    txtTransientSteps->setMinimum(1);
    txtTransientSteps->setMaximum(10000);
    lblTransientTimeStep = new QLabel("0.0");
    lblTransientSteps = new QLabel(tr("Number of constant steps:"));
    connect(cmbTransientMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(transientChanged()));
    connect(txtTransientSteps, SIGNAL(valueChanged(int)), this, SLOT(transientChanged()));
    connect(txtTransientTimeTotal, SIGNAL(textChanged(QString)), this, SLOT(transientChanged()));
    connect(txtTransientOrder, SIGNAL(valueChanged(int)), this, SLOT(transientChanged()));

    // transient analysis
    QGridLayout *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransientAnalysis->setColumnStretch(2, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Method:")), 0, 0, 1, 2);
    layoutTransientAnalysis->addWidget(cmbTransientMethod, 0, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Order:")), 1, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientOrder, 1, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Tolerance (%):")), 2, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientTolerance, 2, 2);
    layoutTransientAnalysis->addWidget(lblTransientTimeTotal, 3, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientTimeTotal, 3, 2);
    layoutTransientAnalysis->addWidget(lblTransientSteps, 4, 0, 1, 2);
    layoutTransientAnalysis->addWidget(txtTransientSteps, 4, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial time step:")), 5, 0);
    layoutTransientAnalysis->addWidget(chkTransientInitialStepSize, 5, 1, 1, 1, Qt::AlignRight);
    layoutTransientAnalysis->addWidget(txtTransientInitialStepSize, 5, 2);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Constant time step:")), 6, 0, 1, 2);
    layoutTransientAnalysis->addWidget(lblTransientTimeStep, 6, 2);

    grpTransientAnalysis = new QGroupBox(tr("Transient analysis"));
    grpTransientAnalysis->setLayout(layoutTransientAnalysis);

    // fill combobox
    fillComboBox();

    // couplings
    couplingsWidget = new CouplingsWidget(this);
    connect(Agros2D::problem(), SIGNAL(couplingsChanged()), couplingsWidget, SLOT(refresh()));
    connect(couplingsWidget, SIGNAL(changed()), couplingsWidget, SLOT(save()));

    QVBoxLayout *layoutCouplings = new QVBoxLayout();
    layoutCouplings->addWidget(couplingsWidget);

    grpCouplings = new QGroupBox(tr("Couplings"));
    grpCouplings->setLayout(layoutCouplings);

    // problem widget
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(grpGeneral);
    // layoutArea->addWidget(grpFieldsToolbar);
    layout->addWidget(grpCouplings);
    layout->addWidget(grpHarmonicAnalysis);
    layout->addWidget(grpTransientAnalysis);
    layout->addStretch(1);

    setLayout(layout);
}

void ProblemWidget::fillComboBox()
{
    cmbCoordinateType->clear();
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Planar), CoordinateType_Planar);
    cmbCoordinateType->addItem(coordinateTypeString(CoordinateType_Axisymmetric), CoordinateType_Axisymmetric);

    cmbMeshType->clear();
    foreach (QString meshType, meshTypeStringKeys())
        cmbMeshType->addItem(meshTypeString(meshTypeFromStringKey(meshType)), meshTypeFromStringKey(meshType));

    cmbTransientMethod->clear();
    foreach (QString timeStepType, timeStepMethodStringKeys())
        cmbTransientMethod->addItem(timeStepMethodString(timeStepMethodFromStringKey(timeStepType)), timeStepMethodFromStringKey(timeStepType));
}

void ProblemWidget::load()
{
    // main
    cmbCoordinateType->setCurrentIndex(cmbCoordinateType->findData(Agros2D::problem()->config()->coordinateType()));
    if (cmbCoordinateType->currentIndex() == -1)
        cmbCoordinateType->setCurrentIndex(0);

    // mesh type
    cmbMeshType->setCurrentIndex(cmbMeshType->findData(Agros2D::problem()->config()->meshType()));

    // harmonic magnetic
    grpHarmonicAnalysis->setVisible(Agros2D::problem()->isHarmonic());
    txtFrequency->setValue(Agros2D::problem()->config()->value(ProblemConfig::Frequency).value<Value>());

    // transient
    grpTransientAnalysis->setVisible(Agros2D::problem()->isTransient());
    txtTransientSteps->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeConstantTimeSteps).toInt());
    txtTransientTimeTotal->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeTotal).toDouble());
    txtTransientTolerance->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeMethodTolerance).toDouble());
    chkTransientInitialStepSize->setChecked(Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble() > 0.0);
    txtTransientInitialStepSize->setEnabled(chkTransientInitialStepSize->isChecked());
    txtTransientInitialStepSize->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeInitialStepSize).toDouble());
    txtTransientOrder->setValue(Agros2D::problem()->config()->value(ProblemConfig::TimeOrder).toInt());
    cmbTransientMethod->setCurrentIndex(cmbTransientMethod->findData((TimeStepMethod) Agros2D::problem()->config()->value(ProblemConfig::TimeMethod).toInt()));
    if (cmbTransientMethod->currentIndex() == -1)
        cmbTransientMethod->setCurrentIndex(0);

    lblTransientTimeTotal->setText(QString("Total time (s)"));

    // couplings
    // fieldsToolbar->refresh();
    couplingsWidget->refresh();

    grpCouplings->setVisible(Agros2D::problem()->couplingInfos().count() > 0);

    transientChanged();
}

void ProblemWidget::save()
{
    // save properties
    Agros2D::problem()->config()->blockSignals(true);

    Agros2D::problem()->config()->setCoordinateType((CoordinateType) cmbCoordinateType->itemData(cmbCoordinateType->currentIndex()).toInt());
    Agros2D::problem()->config()->setMeshType((MeshType) cmbMeshType->itemData(cmbMeshType->currentIndex()).toInt());

    Agros2D::problem()->config()->setValue(ProblemConfig::Frequency, txtFrequency->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeMethod, (TimeStepMethod) cmbTransientMethod->itemData(cmbTransientMethod->currentIndex()).toInt());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeOrder, txtTransientOrder->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeMethodTolerance, txtTransientTolerance->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeConstantTimeSteps, txtTransientSteps->value());
    Agros2D::problem()->config()->setValue(ProblemConfig::TimeTotal, txtTransientTimeTotal->value());
    txtTransientInitialStepSize->setEnabled(chkTransientInitialStepSize->isChecked());
    if (chkTransientInitialStepSize->isChecked())
    {
        Agros2D::problem()->config()->setValue(ProblemConfig::TimeInitialStepSize, txtTransientInitialStepSize->value());
    }
    else
    {
        txtTransientInitialStepSize->setValue(0.0);
        Agros2D::problem()->config()->setValue(ProblemConfig::TimeInitialStepSize, 0.0);
    }

    // save couplings
    couplingsWidget->save();

    Agros2D::problem()->config()->blockSignals(false);
    Agros2D::problem()->config()->refresh();

    emit changed();
}

void ProblemWidget::transientChanged()
{
    lblTransientTimeStep->setText(QString("%1 s").arg(txtTransientTimeTotal->value() / txtTransientSteps->value()));
    lblTransientSteps->setText(tr("Approx. number of steps:"));

    switch ((TimeStepMethod) cmbTransientMethod->itemData(cmbTransientMethod->currentIndex()).toInt())
    {
    case TimeStepMethod_Fixed:
    {
        chkTransientInitialStepSize->setEnabled(false);
        txtTransientInitialStepSize->setEnabled(false);
        txtTransientTolerance->setEnabled(false);
        txtTransientSteps->setEnabled(true);

        lblTransientSteps->setText(tr("Number of steps:"));
    }
        break;
    case TimeStepMethod_BDFTolerance:
    {
        chkTransientInitialStepSize->setEnabled(true);
        txtTransientTolerance->setEnabled(true);
        txtTransientSteps->setEnabled(false);
    }
        break;
    case TimeStepMethod_BDFNumSteps:
    {
        chkTransientInitialStepSize->setEnabled(true);
        txtTransientTolerance->setEnabled(false);
        txtTransientSteps->setEnabled(true);
    }
        break;
    default:
        assert(0);
    }
}


// **************************************************************************************************************************

ProblemDialog::ProblemDialog(QWidget *parent)  : QDialog(parent)
{
    setWindowTitle(tr("Problem properties"));

    problemWidget = new ProblemWidget(this);

    // dialog buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(problemWidget);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());

    move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
         QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}


ProblemDialog::~ProblemDialog()
{
}

void ProblemDialog::doAccept()
{
    problemWidget->save();
    accept();
}

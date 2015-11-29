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

#include "physicalfield.h"

#include "util/global.h"
#include "gui/common.h"

#include "solver/problem.h"
#include "solver/solutionstore.h"
#include "solver/field.h"
#include "solver/problem.h"

void fillComboBoxComputation(QComboBox *cmbComputation)
{
    // store variable
    QString computationId = cmbComputation->itemData(cmbComputation->currentIndex()).toString();
    int itemsCount = cmbComputation->count();

    // clear combo
    cmbComputation->blockSignals(true);
    cmbComputation->clear();
    foreach (QSharedPointer<Computation> problem, Agros2D::computations())
    {
        QDateTime time = QDateTime::fromString(problem->problemDir(), "yyyy-MM-dd-hh-mm-ss-zzz");
        cmbComputation->addItem(time.toString("dd.MM.yyyy hh:mm:ss.zzz"), problem->problemDir());
    }

    // refresh only, no new computation
    if (itemsCount == cmbComputation->count())
        cmbComputation->setCurrentIndex(cmbComputation->findData(computationId));
    else if (cmbComputation->count() > 0)
        cmbComputation->setCurrentIndex(cmbComputation->count() - 1);

    if (cmbComputation->currentIndex() == -1)
        cmbComputation->setCurrentIndex(0);
    cmbComputation->blockSignals(false);
}

void fillComboBoxFieldInfo(QComboBox *cmbFieldInfo, QSharedPointer<Computation> problem)
{
    // store variable
    QString fieldId = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();

    // clear combo
    cmbFieldInfo->blockSignals(true);
    cmbFieldInfo->clear();
    foreach (FieldInfo *fieldInfo, problem->fieldInfos())
        cmbFieldInfo->addItem(fieldInfo->name(), fieldInfo->fieldId());

    cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldId));
    if (cmbFieldInfo->currentIndex() == -1)
        cmbFieldInfo->setCurrentIndex(0);
    cmbFieldInfo->blockSignals(false);
}

void fillComboBoxTimeStep(const FieldInfo* fieldInfo, QComboBox *cmbTimeStep, QSharedPointer<Computation> computation)
{
    if (!computation->isSolved())
        return;

    QList<double> times = computation->timeStepTimes();

    cmbTimeStep->blockSignals(true);

    // store variable
    int timeStep = cmbTimeStep->currentIndex();
    if (timeStep == -1)
        timeStep = times.count() - 1;

    // clear combo
    cmbTimeStep->clear();

    // qDebug() << fieldInfo->name();
    int selectedIndex = -1;
    for (int step = 0; step < times.length(); step++)
    {
        bool stepIsAvailable = computation->solutionStore()->contains(FieldSolutionID(fieldInfo->fieldId(),
                                                                                      step,
                                                                                      computation->solutionStore()->lastAdaptiveStep(fieldInfo, step)));
        if (!stepIsAvailable)
            continue;

        if (step == 0)
            cmbTimeStep->addItem(QObject::tr("Initial step"), 0.0);
        else
            cmbTimeStep->addItem(QObject::tr("%1 s (step: %2)").arg(QString::number(times[step], 'e', 2)).arg(step), step);

        if (step == timeStep)
            selectedIndex = cmbTimeStep->count() - 1;
    }

    if (selectedIndex != -1)
        cmbTimeStep->setCurrentIndex(selectedIndex);
    if (cmbTimeStep->count() > 0 && cmbTimeStep->currentIndex() == -1)
        cmbTimeStep->setCurrentIndex(0);

    cmbTimeStep->blockSignals(false);
}

void fillComboBoxAdaptivityStep(FieldInfo* fieldInfo, int timeStep, QComboBox *cmbAdaptivityStep, QSharedPointer<Computation> computation)
{
    if (!computation->isSolved())
        return;

    cmbAdaptivityStep->blockSignals(true);

    int lastAdaptiveStep = computation->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

    // store variable
    int adaptivityStep = cmbAdaptivityStep->currentIndex();
    if (adaptivityStep == -1)
        adaptivityStep = lastAdaptiveStep;

    // clear combo
    cmbAdaptivityStep->clear();

    for (int step = 0; step <= lastAdaptiveStep; step++)
    {
        cmbAdaptivityStep->addItem(QString::number(step + 1), step);
    }

    cmbAdaptivityStep->setCurrentIndex(adaptivityStep);
    cmbAdaptivityStep->blockSignals(false);
}

PhysicalFieldWidget::PhysicalFieldWidget(QWidget *parent) : QWidget(parent)
{
    cmbComputation = new QComboBox();
    connect(cmbComputation, SIGNAL(currentIndexChanged(int)), this, SLOT(doComputation(int)));

    cmbFieldInfo = new QComboBox();
    connect(cmbFieldInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldInfo(int)));

    QGridLayout *layoutComputation = new QGridLayout();
    layoutComputation->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutComputation->setColumnStretch(1, 1);
    layoutComputation->addWidget(new QLabel(tr("Computation:")), 0, 0);
    layoutComputation->addWidget(cmbComputation, 0, 1);
    layoutComputation->addWidget(new QLabel(tr("Physical field:")), 1, 0);
    layoutComputation->addWidget(cmbFieldInfo, 1, 1);

    grpComputation = new QGroupBox(tr("Postprocessor settings"), this);
    grpComputation->setLayout(layoutComputation);

    // transient
    lblTimeStep = new QLabel(tr("Time step:"));
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStep(int)));

    QGridLayout *layoutTransient = new QGridLayout();
    layoutTransient->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransient->setColumnStretch(1, 1);
    layoutTransient->addWidget(lblTimeStep, 0, 0);
    layoutTransient->addWidget(cmbTimeStep, 0, 1);

    grpTime = new QGroupBox(tr("Transient analysis"), this);
    grpTime->setVisible(false);
    grpTime->setLayout(layoutTransient);

    // adaptivity
    lblAdaptivityStep = new QLabel(tr("Adaptivity step:"));
    cmbAdaptivityStep = new QComboBox(this);

    QGridLayout *layoutAdaptivity = new QGridLayout();
    layoutAdaptivity->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdaptivity->setColumnStretch(1, 1);
    layoutAdaptivity->addWidget(lblAdaptivityStep, 0, 0);
    layoutAdaptivity->addWidget(cmbAdaptivityStep, 0, 1);

    grpAdaptivity = new QGroupBox(tr("Space adaptivity"), this);
    grpAdaptivity->setVisible(false);
    grpAdaptivity->setLayout(layoutAdaptivity);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->addWidget(grpComputation);
    layoutMain->addWidget(grpTime);
    layoutMain->addWidget(grpAdaptivity);

    setLayout(layoutMain);

    // reconnect computation slots
    connect(Agros2D::singleton(), SIGNAL(connectComputation(QSharedPointer<Computation>)), this, SLOT(connectComputation(QSharedPointer<Computation>)));
}

PhysicalFieldWidget::~PhysicalFieldWidget()
{
}

void PhysicalFieldWidget::connectComputation(QSharedPointer<Computation> computation)
{
    if (!m_computation.isNull())
    {
        disconnect(m_computation.data(), SIGNAL(meshed()), this, SLOT(updateControls()));
        disconnect(m_computation.data(), SIGNAL(solved()), this, SLOT(updateControls()));
    }

    m_computation = computation;

    if (!m_computation.isNull())
    {
        connect(m_computation.data(), SIGNAL(meshed()), this, SLOT(updateControls()));
        connect(m_computation.data(), SIGNAL(solved()), this, SLOT(updateControls()));
    }
}

FieldInfo* PhysicalFieldWidget::selectedField()
{
    if (m_computation->hasField(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString()))
        return m_computation->fieldInfo(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString());
    else
        return nullptr;
}

void PhysicalFieldWidget::selectField(const FieldInfo* fieldInfo)
{
    if (cmbFieldInfo->findData(fieldInfo->fieldId()) != -1)
    {
        cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldInfo->fieldId()));

        if (m_computation->isSolved())
        {
            fillComboBoxTimeStep(fieldInfo, cmbTimeStep, m_computation);
            doTimeStep();
        }
    }
}

int PhysicalFieldWidget::selectedTimeStep()
{
    if (cmbTimeStep->currentIndex() == -1)
        return 0;
    else
        return cmbTimeStep->itemData(cmbTimeStep->currentIndex()).toInt();
}

void PhysicalFieldWidget::selectTimeStep(int timeStep)
{
    if (cmbTimeStep->findData(timeStep) != -1)
        cmbTimeStep->setCurrentIndex(cmbTimeStep->findData(timeStep));
}

int PhysicalFieldWidget::selectedAdaptivityStep()
{
    if (cmbAdaptivityStep->currentIndex() == -1)
        return 0;
    else
        return cmbAdaptivityStep->itemData(cmbAdaptivityStep->currentIndex()).toInt();
}

void PhysicalFieldWidget::selectAdaptivityStep(int adaptivityStep)
{
    if (cmbAdaptivityStep->findData(adaptivityStep) != -1)
        cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->findData(adaptivityStep));
}

void PhysicalFieldWidget::updateControls()
{
    if (Agros2D::computations().count() > 0)
    {
        fillComboBoxComputation(cmbComputation);
    }
    else
    {
        cmbComputation->clear();
    }

    doComputation();
}

void PhysicalFieldWidget::doComputation(int index)
{
    if (Agros2D::computations().count() > 0)
    {
        // set current computation
        if (m_computation && m_computation->isMeshed())
        {
            if (cmbComputation->itemData(cmbComputation->currentIndex()).toString() != m_computation->problemDir())
                Agros2D::setCurrentComputation(cmbComputation->itemData(cmbComputation->currentIndex()).toString());

            fillComboBoxFieldInfo(cmbFieldInfo, m_computation);
        }
        else
        {
            cmbFieldInfo->clear();
        }
    }
    else
    {
        cmbFieldInfo->clear();
    }

    doFieldInfo();
}

void PhysicalFieldWidget::doFieldInfo(int index)
{
    QString fieldName = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
    if (m_computation && m_computation->hasField(fieldName))
    {
        FieldInfo *fieldInfo = m_computation->fieldInfo(fieldName);
        if (m_computation->isSolved())
        {
            fillComboBoxTimeStep(fieldInfo, cmbTimeStep, m_computation);
        }
        else
        {
            cmbTimeStep->clear();
        }

        doTimeStep();

        if ((m_currentFieldName != fieldName) || (m_currentAnalysisType != fieldInfo->analysisType()))
        {
            emit fieldChanged();
        }

        // set current field name
        m_currentFieldName = fieldName;
        m_currentAnalysisType = fieldInfo->analysisType();
    }
    else
    {
        cmbTimeStep->clear();
        doTimeStep();
    }
}

void PhysicalFieldWidget::doTimeStep(int index)
{
    if (m_computation && m_computation->isSolved())
    {
        fillComboBoxAdaptivityStep(selectedField(), selectedTimeStep(), cmbAdaptivityStep, m_computation);
        if ((cmbAdaptivityStep->currentIndex() >= cmbAdaptivityStep->count()) || (cmbAdaptivityStep->currentIndex() < 0))
        {
            cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->count() - 1);
        }
    }
    else
    {
        cmbAdaptivityStep->clear();
    }

    grpTime->setVisible(cmbTimeStep->count() > 1);
    grpAdaptivity->setVisible(cmbAdaptivityStep->count() > 1);
}


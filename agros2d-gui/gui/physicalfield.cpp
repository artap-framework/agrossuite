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

void fillComboBoxComputation(QComboBox *cmbComputation, const QString &computationId)
{
    // store variable
    int itemsCount = cmbComputation->count();

    // clear combo
    cmbComputation->blockSignals(true);
    cmbComputation->clear();
    foreach (QSharedPointer<Computation> computation, Agros2D::computations())
    {
        if (computation->isSolved())
        {
            QDateTime time = QDateTime::fromString(computation->problemDir(), "yyyy-MM-dd-hh-mm-ss-zzz");
            cmbComputation->addItem(time.toString("dd.MM.yyyy hh:mm:ss.zzz"), computation->problemDir());
        }
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

void fillComboBoxFieldInfo(QComboBox *cmbFieldInfo, QSharedPointer<Computation> problem, const QString &fieldId)
{
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

void fillComboBoxTimeStep(QComboBox *cmbTimeStep, QSharedPointer<Computation> problem, const FieldInfo* fieldInfo, int timeStep = -1)
{
    if (!problem->isSolved())
        return;

    QList<double> times = problem->timeStepTimes();

    cmbTimeStep->blockSignals(true);

    // store variable
    if (timeStep == -1)
        timeStep = times.count() - 1;

    // clear combo
    cmbTimeStep->clear();

    // qDebug() << fieldInfo->name();
    int selectedIndex = -1;
    for (int step = 0; step < times.length(); step++)
    {
        bool stepIsAvailable = problem->solutionStore()->contains(FieldSolutionID(fieldInfo->fieldId(),
                                                                                  step,
                                                                                  problem->solutionStore()->lastAdaptiveStep(fieldInfo, step)));
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

void fillComboBoxAdaptivityStep(QComboBox *cmbAdaptivityStep, QSharedPointer<Computation> problem, FieldInfo* fieldInfo, int timeStep, int adaptivityStep = -1)
{
    if (!problem->isSolved())
        return;

    cmbAdaptivityStep->blockSignals(true);

    int lastAdaptiveStep = problem->solutionStore()->lastAdaptiveStep(fieldInfo, timeStep);

    // store variable
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

PhysicalFieldWidget::PhysicalFieldWidget(QWidget *parent) : QWidget(parent),
    m_lastComputation(""), m_lastFieldId(""), m_lastFieldAnalysisType(AnalysisType_Undefined), m_lastTimeStep(-1), m_lastAdaptiveStep(-1)
{
    cmbComputation = new QComboBox();
    connect(cmbComputation, SIGNAL(currentIndexChanged(int)), this, SLOT(doComputation(int)));

    cmbFieldInfo = new QComboBox();
    connect(cmbFieldInfo, SIGNAL(currentIndexChanged(int)), this, SLOT(doFieldInfo(int)));

    // transient
    lblTimeStep = new QLabel(tr("Time step:"));
    cmbTimeStep = new QComboBox(this);
    connect(cmbTimeStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doTimeStep(int)));

    // adaptivity
    lblAdaptivityStep = new QLabel(tr("Adaptivity step:"));
    cmbAdaptivityStep = new QComboBox(this);
    connect(cmbAdaptivityStep, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStep(int)));

    QGridLayout *layoutComputation = new QGridLayout();
    layoutComputation->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutComputation->setColumnStretch(1, 1);
    layoutComputation->addWidget(new QLabel(tr("Computation:")), 0, 0);
    layoutComputation->addWidget(cmbComputation, 0, 1);
    layoutComputation->addWidget(new QLabel(tr("Physical field:")), 1, 0);
    layoutComputation->addWidget(cmbFieldInfo, 1, 1);
    layoutComputation->addWidget(lblTimeStep, 2, 0);
    layoutComputation->addWidget(cmbTimeStep, 2, 1);
    layoutComputation->addWidget(lblAdaptivityStep, 3, 0);
    layoutComputation->addWidget(cmbAdaptivityStep, 3, 1);

    setLayout(layoutComputation);
}

PhysicalFieldWidget::~PhysicalFieldWidget()
{
}

QSharedPointer<Computation> PhysicalFieldWidget::selectedComputation()
{
    if (cmbComputation->count() > 0)
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        return computations[cmbComputation->itemData(cmbComputation->currentIndex()).toString()];
    }

    return QSharedPointer<Computation>(nullptr);
}

FieldInfo* PhysicalFieldWidget::selectedField()
{
    if (selectedComputation())
    {
        if (selectedComputation()->hasField(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString()))
            return selectedComputation()->fieldInfo(cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString());
    }

    return nullptr;
}

void PhysicalFieldWidget::selectField(const FieldInfo* fieldInfo)
{
    if (cmbFieldInfo->findData(fieldInfo->fieldId()) != -1)
    {
        cmbFieldInfo->setCurrentIndex(cmbFieldInfo->findData(fieldInfo->fieldId()));

        if (selectedComputation()->isSolved())
        {
            fillComboBoxTimeStep(cmbTimeStep, selectedComputation(), fieldInfo);
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
        fillComboBoxComputation(cmbComputation, m_lastComputation);
    }
    else
    {
        m_lastComputation = "";
        m_lastFieldId = "";
        m_lastFieldAnalysisType = AnalysisType_Undefined;
        m_lastTimeStep = -1;
        m_lastAdaptiveStep = -1;

        cmbComputation->clear();
    }

    doComputation();
}

void PhysicalFieldWidget::doComputation(int index)
{
    if (Agros2D::computations().count() > 0)
    {
        // invalidate last field
        if (m_lastComputation != cmbComputation->itemData(cmbComputation->currentIndex()).toString())
        {
            m_lastComputation = cmbComputation->itemData(cmbComputation->currentIndex()).toString();

            m_lastFieldId = "";
            m_lastFieldAnalysisType = AnalysisType_Undefined;
            m_lastTimeStep = -1;
            m_lastAdaptiveStep = -1;
        }

        // set current computation
        if (selectedComputation() && selectedComputation()->isSolved())
        {
            fillComboBoxFieldInfo(cmbFieldInfo, selectedComputation(), m_lastFieldId);
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
    QString fieldId = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
    if (selectedComputation() && selectedComputation()->hasField(fieldId))
    {
        FieldInfo *fieldInfo = selectedComputation()->fieldInfo(fieldId);

        if (selectedComputation()->isSolved())
        {
            fillComboBoxTimeStep(cmbTimeStep, selectedComputation(), fieldInfo, m_lastTimeStep);
        }
        else
        {
            cmbTimeStep->clear();
        }

        doTimeStep();

        if ((m_lastFieldId != fieldId) || (m_lastFieldAnalysisType != fieldInfo->analysisType()))
        {
            // set current field name
            m_lastFieldId = cmbFieldInfo->itemData(cmbFieldInfo->currentIndex()).toString();
            m_lastFieldAnalysisType = fieldInfo->analysisType();

            emit fieldChanged();
        }
    }
    else
    {
        cmbTimeStep->clear();
        doTimeStep();
    }
}

void PhysicalFieldWidget::doTimeStep(int index)
{
    if (selectedComputation() && selectedComputation()->isSolved() && selectedField())
    {
        fillComboBoxAdaptivityStep(cmbAdaptivityStep, selectedComputation(), selectedField(), selectedTimeStep(), m_lastAdaptiveStep);
        if ((cmbAdaptivityStep->currentIndex() >= cmbAdaptivityStep->count()) || (cmbAdaptivityStep->currentIndex() < 0))
            cmbAdaptivityStep->setCurrentIndex(cmbAdaptivityStep->count() - 1);

        m_lastTimeStep = selectedTimeStep();

    }
    else
    {
        cmbAdaptivityStep->clear();
    }

    lblTimeStep->setVisible(cmbTimeStep->count() > 1);
    cmbTimeStep->setVisible(cmbTimeStep->count() > 1);

    doAdaptivityStep();
}

void PhysicalFieldWidget::doAdaptivityStep(int index)
{
    m_lastAdaptiveStep = selectedAdaptivityStep();

    lblAdaptivityStep->setVisible(cmbAdaptivityStep->count() > 1);
    cmbAdaptivityStep->setVisible(cmbAdaptivityStep->count() > 1);
}

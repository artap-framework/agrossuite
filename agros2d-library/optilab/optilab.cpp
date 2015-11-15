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

#include "optilab.h"
#include "parameter.h"
#include "study.h"
#include "util/global.h"

#include "study.h"

#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"

#include "gui/infowidget.h"

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent), m_optilab(parent)
{
    createControls();

    connect(Agros2D::problem()->studies(), SIGNAL(invalidated()), this, SLOT(refresh()));
}

OptiLabWidget::~OptiLabWidget()
{

}

void OptiLabWidget::createControls()
{
    cmbStudies = new QComboBox(this);
    connect(cmbStudies, SIGNAL(currentIndexChanged(int)), this, SLOT(studyChanged(int)));

    QVBoxLayout *layoutStudies = new QVBoxLayout();
    layoutStudies->addWidget(cmbStudies);

    QGroupBox *grpStudies = new QGroupBox(tr("Studies"));
    grpStudies->setLayout(layoutStudies);

    // parameters
    trvComputations = new QTreeWidget(this);
    trvComputations->setMouseTracking(true);
    trvComputations->setColumnCount(2);
    trvComputations->setIndentation(15);
    trvComputations->setMinimumWidth(220);
    trvComputations->setColumnWidth(0, 220);

    QStringList headers;
    headers << tr("Computation") << tr("State");
    trvComputations->setHeaderLabels(headers);

    connect(trvComputations, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(computationChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QPushButton *btnTESTSWEEP = new QPushButton(tr("TEST SWEEP"));
    connect(btnTESTSWEEP, SIGNAL(clicked()), this, SLOT(testSweep()));
    QPushButton *btnTESTOPT = new QPushButton(tr("TEST OPT."));
    connect(btnTESTOPT, SIGNAL(clicked()), this, SLOT(testOptimization()));

    QHBoxLayout *layoutParametersButton = new QHBoxLayout();
    layoutParametersButton->addWidget(btnTESTSWEEP);
    layoutParametersButton->addWidget(btnTESTOPT);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(grpStudies);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutParametersButton);

    setLayout(layout);
}

void OptiLabWidget::refresh()
{
    // fill studies
    cmbStudies->blockSignals(true);

    QString selectedItem = "";
    if (cmbStudies->currentIndex() != -1)
        selectedItem = cmbStudies->currentText();

    cmbStudies->clear();
    foreach (Study *study, Agros2D::problem()->studies()->items())
    {
        cmbStudies->addItem(studyTypeString(study->type()));
    }

    cmbStudies->blockSignals(false);

    if (cmbStudies->count() > 0)
    {
        if (!selectedItem.isEmpty())
            cmbStudies->setCurrentText(selectedItem);
        else
            cmbStudies->setCurrentIndex(0);

        studyChanged(cmbStudies->currentIndex());
    }
}

void OptiLabWidget::studyChanged(int index)
{
    // study
    Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

    QList<QSharedPointer<Computation> > computations = study->computations();

    trvComputations->blockSignals(true);

    // computations
    QString selectedItem = "";
    if (trvComputations->currentItem())
        selectedItem = trvComputations->currentItem()->data(0, Qt::UserRole).toString();

    trvComputations->clear();
    foreach (QSharedPointer<Computation> computation, computations)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvComputations);
        item->setText(0, computation->problemDir());
        item->setText(1, (computation->isSolved() ? tr("solved") : "-"));
        item->setData(0, Qt::UserRole, computation->problemDir());

        trvComputations->addTopLevelItem(item);
    }

    trvComputations->blockSignals(false);

    // select current computation
    computationSelect(selectedItem);

    // if not selected -> select first
    if (trvComputations->topLevelItemCount() > 0 && !trvComputations->currentItem())
        trvComputations->setCurrentItem(trvComputations->topLevelItem(0));
}

void OptiLabWidget::testSweep()
{
    // sweep analysis
    StudySweepAnalysis *analysis = new StudySweepAnalysis();

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // only one parameter
    // QList<double> params; params << 0.05 << 0.055 << 0.06 << 0.065;
    // analysis->setParameter(Parameter::fromList("R3", params));
    // analysis->setParameter(Parameter::fromValue("R3", 0.06));
    analysis->setParameter(Parameter::fromRandom("R3", 0.05, 0.07, 4));
    // analysis->setParameter(Parameter::fromLinspace("R3", 0.05, 0.07, 3));

    // add functionals
    analysis->addFunctional(Functional("We", Functional::Minimize, "computation.solution(\"electrostatic\").volume_integrals([0,1])[\"We\"]"));

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::testOptimization()
{
    // sweep analysis
    StudyGoldenSectionSearch *analysis = new StudyGoldenSectionSearch(1e-4);

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // only one parameter
    analysis->setParameter(Parameter("R3", 0.05, 0.07));

    // add functionals
    analysis->addFunctional(Functional("We", Functional::Minimize, "computation.solution(\"electrostatic\").volume_integrals([0,1])[\"We\"]"));

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::computationSelect(const QString &key)
{

}

void OptiLabWidget::computationChanged(QTreeWidgetItem *source, QTreeWidgetItem *dest)
{
    if (trvComputations->currentItem())
    {
        QString key = trvComputations->currentItem()->data(0, Qt::UserRole).toString();
        emit computationSelected(key);
    }
}

OptiLab::OptiLab(QWidget *parent) : QWidget(parent)
{
    actSceneModeOptiLab = new QAction(icon("optilab"), tr("OptiLab"), this);
    actSceneModeOptiLab->setShortcut(Qt::Key_F8);
    actSceneModeOptiLab->setCheckable(true);

    m_optiLabWidget = new OptiLabWidget(this);

    createControls();

    connect(m_optiLabWidget, SIGNAL(computationSelected(QString)), this, SLOT(computationSelected(QString)));
}

OptiLab::~OptiLab()
{    
}

void OptiLab::createControls()
{
    m_infoWidget = new InfoWidgetGeneral(this);

    QHBoxLayout *layoutLab = new QHBoxLayout();
    layoutLab->addWidget(m_infoWidget);

    setLayout(layoutLab);
}

void OptiLab::computationSelected(const QString &key)
{
    QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
    QSharedPointer<Computation> computation = computations[key];

    m_infoWidget->showProblemInfo(computation.data());
}

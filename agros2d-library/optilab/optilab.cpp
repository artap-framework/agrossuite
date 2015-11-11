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
#include "util/global.h"
#include "solver/problem.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"
#include "gui/infowidget.h"

OptiLabWidget::OptiLabWidget(OptiLab *parent) : QWidget(parent)
{
    createControls();
}

OptiLabWidget::~OptiLabWidget()
{

}

void OptiLabWidget::createControls()
{
    // parameters
    trvComputations = new QTreeWidget(this);
    trvComputations->setMouseTracking(true);
    trvComputations->setColumnCount(2);
    trvComputations->setIndentation(15);
    trvComputations->setMinimumWidth(220);
    trvComputations->setColumnWidth(0, 220);

    QStringList headers;
    headers << tr("Computation") << tr("Solved");
    trvComputations->setHeaderLabels(headers);

    connect(trvComputations, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(computationChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    QPushButton *btnComputationsRefresh = new QPushButton(tr("Refresh"));
    connect(btnComputationsRefresh, SIGNAL(clicked()), this, SLOT(refresh()));

    QPushButton *btnTEST = new QPushButton(tr("TEST"));
    connect(btnTEST, SIGNAL(clicked()), this, SLOT(test()));

    QHBoxLayout *layoutParametersButton = new QHBoxLayout();
    layoutParametersButton->addWidget(btnComputationsRefresh);
    layoutParametersButton->addStretch(1);
    layoutParametersButton->addWidget(btnTEST);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutParametersButton);

    setLayout(layout);
}

void OptiLabWidget::refresh()
{
    QMap<QString, QSharedPointer<ProblemComputation> > computations = Agros2D::computations();

    trvComputations->blockSignals(true);

    // computations
    QString selectedItem = "";
    if (trvComputations->currentItem())
        selectedItem = trvComputations->currentItem()->data(0, Qt::UserRole).toString();

    trvComputations->clear();
    foreach (QString key, computations.keys())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(trvComputations);
        item->setText(0, key);
        item->setText(1, (computations[key]->isSolved() ? tr("solved") : "-"));
        item->setData(0, Qt::UserRole, key);

        trvComputations->addTopLevelItem(item);
    }

    trvComputations->blockSignals(false);

    // select current computation
    computationSelect(selectedItem);

    // if not selected -> select first
    if (trvComputations->topLevelItemCount() > 0 && !trvComputations->currentItem())
        trvComputations->setCurrentItem(trvComputations->topLevelItem(0));
}

void OptiLabWidget::test()
{
    QList<double> params;
    params << 0.05 << 0.065 << 0.055 << 0.06;

    foreach (double param, params)
    {
        Agros2D::preprocessor()->config()->setParameter("R3", param);
        QSharedPointer<ProblemComputation> computation = Agros2D::preprocessor()->createComputation(true, false);
        computation->solve();

        FieldInfo *fieldInfo = computation->fieldInfo("electrostatic");
        computation->scene()->labels->setSelected(true);
        std::shared_ptr<IntegralValue> values = fieldInfo->plugin()->volumeIntegral(computation.data(),
                                                                                    fieldInfo,
                                                                                    0,
                                                                                    0);

        QMap<QString, double> integrals = values->values();
        computation->solutionStore()->setResult("We", integrals["electrostatic_energy"]);
        computation->solutionStore()->setResult("V", integrals["electrostatic_volume"]);
        computation->solutionStore()->saveRunTimeDetails();
    }

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
    QMap<QString, QSharedPointer<ProblemComputation> > computations = Agros2D::computations();
    QSharedPointer<ProblemComputation> computation = computations[key];

    m_infoWidget->showProblemInfo(computation.data());
}

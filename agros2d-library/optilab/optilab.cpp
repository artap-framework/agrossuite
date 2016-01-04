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
#include "study_sweep.h"
#include "study_genetic.h"
#include "util/global.h"

#include "study.h"

#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/plugin_interface.h"
#include "solver/solutionstore.h"

#include "gui/infowidget.h"
#include "qcustomplot.h"

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
    trvComputations->setMinimumWidth(220);
    trvComputations->setColumnWidth(0, 220);

    QStringList headers;
    headers << tr("Computation") << tr("State");
    trvComputations->setHeaderLabels(headers);

    connect(trvComputations, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(computationChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

    cmbChartX = new QComboBox(this);
    cmbChartY = new QComboBox(this);

    QGridLayout *layoutChartXYControls = new QGridLayout();
    layoutChartXYControls->addWidget(new QLabel(tr("Variable X:")), 0, 0);
    layoutChartXYControls->addWidget(cmbChartX, 0, 1);
    layoutChartXYControls->addWidget(new QLabel(tr("Variable Y:")), 1, 0);
    layoutChartXYControls->addWidget(cmbChartY, 1, 1);
    layoutChartXYControls->addWidget(new QLabel(""), 19, 0);

    QPushButton *btnTESTSWEEP = new QPushButton(tr("TEST SWEEP"));
    connect(btnTESTSWEEP, SIGNAL(clicked()), this, SLOT(testSweep()));
    QPushButton *btnTESTGENETIC = new QPushButton(tr("TEST GEN."));
    connect(btnTESTGENETIC, SIGNAL(clicked()), this, SLOT(testGenetic()));

    QHBoxLayout *layoutParametersButton = new QHBoxLayout();
    layoutParametersButton->addWidget(btnTESTSWEEP);
    layoutParametersButton->addWidget(btnTESTGENETIC);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(grpStudies);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutChartXYControls);
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
    trvComputations->clear();
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

    // fill combo boxes
    cmbChartX->clear();
    cmbChartY->clear();

    // parameters
    if (cmbStudies->currentIndex() != -1)
    {
        Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());
        foreach (Parameter parameter, study->parameters())
        {
            cmbChartX->addItem(QString("%1 (par.)").arg(parameter.name()), QString("parameter:%1").arg(parameter.name()));
            cmbChartY->addItem(QString("%1 (par.)").arg(parameter.name()), QString("parameter:%1").arg(parameter.name()));
        }
        foreach (Functional functional, study->functionals())
        {
            cmbChartX->addItem(QString("%1 (fun.)").arg(functional.name()), QString("functional:%1").arg(functional.name()));
            cmbChartY->addItem(QString("%1 (fun.)").arg(functional.name()), QString("functional:%1").arg(functional.name()));
        }
    }
}

void OptiLabWidget::studyChanged(int index)
{
    // study
    Study *study = Agros2D::problem()->studies()->items().at(cmbStudies->currentIndex());

    trvComputations->blockSignals(true);

    // computations
    QString selectedItem = "";
    if (trvComputations->currentItem())
        selectedItem = trvComputations->currentItem()->data(0, Qt::UserRole).toString();

    trvComputations->clear();

    // fill tree view
    study->fillTreeView(trvComputations);

    trvComputations->blockSignals(false);

    // select current computation
    if (!selectedItem.isEmpty())
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
    analysis->addParameter(Parameter::fromRandom("R3", 4, 0.05, 0.07));
    analysis->addParameter(Parameter::fromRandom("C", 10, 1, 5));
    // analysis->setParameter(Parameter::fromLinspace("R3", 3, 0.05, 0.07));

    // add functionals
    analysis->addFunctional(Functional("We", Functional::Minimize, "C+R3**2")); //computation.solution(\"electrostatic\").volume_integrals([0,1])[\"We\"]

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::testGenetic()
{
    // sweep analysis
    StudyGenetic *analysis = new StudyGenetic();

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // only one parameter
    analysis->addParameter(Parameter("px", -10.0, 10.0));
    analysis->addParameter(Parameter("py", -10.0, 10.0));

    // add functionals
    // analysis->addFunctional(Functional("f", Functional::Minimize, "-(sin(px) * cos(py) * exp((1 - (sqrt(px**2 + py**2)/pi))))"));
    // analysis->addFunctional(Functional("f", Functional::Minimize, "px**2 + py**2"));
    analysis->addFunctional(Functional("f", Functional::Minimize, "(px+2*py-7)**2 + (2*px+py-5)**2"));

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

    m_chart = new QCustomPlot(this);
    m_chart->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    // m_chart->xAxis->setLabel("");
    // m_chart->yAxis->setLabel("");
    m_chart->addGraph();

    m_chart->graph(0)->setLineStyle(QCPGraph::lsLine);
    m_chart->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));

    QHBoxLayout *layoutLab = new QHBoxLayout();
    layoutLab->addWidget(m_infoWidget, 1);
    layoutLab->addWidget(m_chart, 1);

    setLayout(layoutLab);
}

void OptiLab::computationSelected(const QString &key)
{
    if (key.isEmpty())
    {
        m_infoWidget->clear();
    }
    else
    {
        QMap<QString, QSharedPointer<Computation> > computations = Agros2D::computations();
        QSharedPointer<Computation> computation = computations[key];

        m_infoWidget->showProblemInfo(computation.data());
    }
}

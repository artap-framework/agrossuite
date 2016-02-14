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
#include "study_bayesopt.h"
#include "study_nlopt.h"
#include "util/global.h"

#include "study.h"
#include "logview.h"

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

    QPushButton *btnTESTSweep = new QPushButton(tr("TEST Sweep"));
    connect(btnTESTSweep, SIGNAL(clicked()), this, SLOT(testSweep()));
    QPushButton *btnTESTGenetic = new QPushButton(tr("TEST Gen."));
    connect(btnTESTGenetic, SIGNAL(clicked()), this, SLOT(testGenetic()));
    QPushButton *btnTESTBayesOpt = new QPushButton(tr("TEST BayesOpt"));
    connect(btnTESTBayesOpt, SIGNAL(clicked()), this, SLOT(testBayesOpt()));
    QPushButton *btnTESTNLoptTEAM22 = new QPushButton(tr("TEAM 22 NLopt"));
    connect(btnTESTNLoptTEAM22, SIGNAL(clicked()), this, SLOT(testNLoptTEAM22()));
    QPushButton *testBayesOptTEAM22 = new QPushButton(tr("TEAM 22 BayesOpt"));
    connect(testBayesOptTEAM22, SIGNAL(clicked()), this, SLOT(testBayesOptTEAM22()));
    QPushButton *testBayesOptTEAM25 = new QPushButton(tr("TEAM 25 BayesOpt"));
    connect(testBayesOptTEAM25, SIGNAL(clicked()), this, SLOT(testBayesOptTEAM25()));

    QHBoxLayout *layoutParametersButton1 = new QHBoxLayout();
    layoutParametersButton1->addWidget(btnTESTSweep);
    layoutParametersButton1->addWidget(btnTESTGenetic);
    layoutParametersButton1->addWidget(btnTESTNLoptTEAM22);
    layoutParametersButton1->addWidget(btnTESTBayesOpt);

    QHBoxLayout *layoutParametersButton2 = new QHBoxLayout();
    layoutParametersButton2->addWidget(testBayesOptTEAM22);
    layoutParametersButton2->addWidget(testBayesOptTEAM25);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(2, 2, 2, 3);
    layout->addWidget(grpStudies);
    layout->addWidget(trvComputations);
    layout->addLayout(layoutChartXYControls);
    layout->addLayout(layoutParametersButton1);
    layout->addLayout(layoutParametersButton2);

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
    foreach (Study *study, Agros2D::problem()->studies()->studies())
    {
        if (study->name().isEmpty())
            cmbStudies->addItem(studyTypeString(study->type()));
        else
            cmbStudies->addItem(study->name());

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
        Study *study = Agros2D::problem()->studies()->studies().at(cmbStudies->currentIndex());
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
    Study *study = Agros2D::problem()->studies()->studies().at(cmbStudies->currentIndex());

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

    // result recipes
    LocalValueRecipe *localValueRecipe = new LocalValueRecipe("V", "electrostatic", "electrostatic_potential");
    localValueRecipe->setPoint(0.02, 0.05);
    Agros2D::problem()->recipes()->addRecipe(localValueRecipe);

    SurfaceIntegralRecipe *surfaceIntegralRecipe = new SurfaceIntegralRecipe("Q", "electrostatic", "electrostatic_charge");
    surfaceIntegralRecipe->addEdge(1);
    surfaceIntegralRecipe->addEdge(12);
    Agros2D::problem()->recipes()->addRecipe(surfaceIntegralRecipe);

    VolumeIntegralRecipe *volumeIntegralRecipe = new VolumeIntegralRecipe("We", "electrostatic", "electrostatic_energy");
    Agros2D::problem()->recipes()->addRecipe(volumeIntegralRecipe);

    // parameters
    QList<double> params; params << 0.005 << 0.01 << 0.015;
    analysis->addParameter(Parameter::fromList("R1", params));
    //analysis->addParameter(Parameter::fromRandom("R2", 4, 0.05, 0.07));
    //analysis->addParameter(Parameter::fromLinspace("R3", 3, 0.05, 0.07));
    //analysis->addParameter(Parameter::fromRandom("C", 10, 1, 5));

    // functionals
    analysis->addFunctional(Functional("C", FunctionalType_Result, "2*We/U**2"));

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::testOptimization(StudyType type)
{
    Study *analysis= nullptr;
    if (type == StudyType_Genetic)
    {
        // genetic
        analysis = new StudyGenetic();
    }
    else if (type == StudyType_BayesOptAnalysis)
    {
        // BayesOpt
        analysis = new StudyBayesOptAnalysis();
    }
    else if (type == StudyType_NLoptAnalysis)
    {
        // NLopt
        analysis = new StudyNLoptAnalysis();
    }

    assert(analysis);

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // parameters
    analysis->addParameter(Parameter("px", -10.0, 10.0));
    analysis->addParameter(Parameter("py", -10.0, 10.0));

    // add functionals
    // analysis->addFunctional(Functional("f", FunctionalType_Minimize, "-(sin(px) * cos(py) * exp((1 - (sqrt(px**2 + py**2)/pi))))"));
    // analysis->addFunctional(Functional("f", FunctionalType_Minimize, "px**2 + py**2"));
    analysis->addFunctional(Functional("f", FunctionalType_Minimize, "(px+2*py-7)**2 + (2*px+py-5)**2"));

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::testTEAM22(StudyType type)
{   
    Study *analysis= nullptr;
    if (type == StudyType_Genetic)
    {
        // genetic
        analysis = new StudyGenetic();
    }
    else if (type == StudyType_BayesOptAnalysis)
    {
        // BayesOpt
        analysis = new StudyBayesOptAnalysis();
    }
    else if (type == StudyType_NLoptAnalysis)
    {
        // NLopt
        analysis = new StudyNLoptAnalysis();
    }

    assert(analysis);

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // parameters
    analysis->addParameter(Parameter("R2", 2.6, 3.4));
    analysis->addParameter(Parameter("h2", 0.204*2, 1.1*2));
    analysis->addParameter(Parameter("d2", 0.1, 0.4));
    // QString func = "abs(2*Wm - 180e6) / 180e6 + 1.0/(22*3e-6**2) * (0 ";
    QString func = "abs(2*Wm - 180e6) / 180e6";

    // analysis->addParameter(Parameter("R1", 1.0, 1.5));
    // analysis->addParameter(Parameter("R2", 1.6, 5.0));
    // analysis->addParameter(Parameter("h1", 0.1*2, 1.8*2));
    // analysis->addParameter(Parameter("h2", 0.1*2, 1.8*2));
    // analysis->addParameter(Parameter("d1", 0.1, 0.8));
    // analysis->addParameter(Parameter("d2", 0.1, 0.8));
    // analysis->addParameter(Parameter("J1", 10e6, 30e6));
    // analysis->addParameter(Parameter("J2", -30e6, -10e6));
    // QString func = "abs(2*Wm - 180e6) / 180e6 + 1.0/(22*200e-6**2) * (0 ";

    // result recipes

    /*int N = 11;
    double step = 10.0 / (N-1);
    for (int i = 0; i < N; i++)
    {
        Point point(i*step, 10.0);
        LocalValueRecipe *localValueP = new LocalValueRecipe(QString("B%1").arg(i), "magnetic", "magnetic_flux_density_real");
        localValueP->setPoint(point);
        localValueP->setComponent(PhysicFieldVariableComp_Magnitude);
        Agros2D::problem()->recipes()->addRecipe(localValueP);

        // qDebug() << point.toString();
        func += QString("+ %1**2 ").arg(QString("B%1").arg(i));
    }

    N = 11;
    step = 10.0 / N;
    for (int i = 0; i < N; i++)
    {
        Point point(10.0, i*step);
        LocalValueRecipe *localValueP = new LocalValueRecipe(QString("B%1").arg(i+N+1), "magnetic", "magnetic_flux_density_real");
        localValueP->setPoint(point);
        localValueP->setComponent(PhysicFieldVariableComp_Magnitude);
        Agros2D::problem()->recipes()->addRecipe(localValueP);

        // qDebug() << point.toString();
        func += QString("+ %1**2 ").arg(QString("B%1").arg(i+N+1));
    }
    func += ")";
    */

    VolumeIntegralRecipe *volumeIntegralRecipe = new VolumeIntegralRecipe("Wm", "magnetic", "magnetic_energy");
    Agros2D::problem()->recipes()->addRecipe(volumeIntegralRecipe);

    qDebug() << func;

    // add functionals
    analysis->addFunctional(Functional("OF", FunctionalType_Minimize, func));

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::testTEAM25(StudyType type)
{
    Study *analysis= nullptr;
    if (type == StudyType_Genetic)
    {
        // genetic
        analysis = new StudyGenetic();
    }
    else if (type == StudyType_BayesOptAnalysis)
    {
        // BayesOpt
        analysis = new StudyBayesOptAnalysis();
    }
    else if (type == StudyType_NLoptAnalysis)
    {
        // NLopt
        analysis = new StudyNLoptAnalysis();
    }

    assert(analysis);

    // add to list
    Agros2D::problem()->studies()->addStudy(analysis);

    // parameters
    analysis->addParameter(Parameter("R1", 5e-3, 9e-3));
    analysis->addParameter(Parameter("L2", 12.6e-3, 18e-3));
    analysis->addParameter(Parameter("L3", 14e-3, 45e-3));
    analysis->addParameter(Parameter("L4", 4e-3, 19e-3));

    QString func = "0 ";

    // result recipes
    double R = 9.5e-3 + 2.25e-3;

    int N = 10;
    double step = 45.0/(N-1)/180.0*M_PI;
    for (int i = 0; i < N; i++)
    {
        Point point(R*cos(i*step), R*sin(i*step));
        LocalValueRecipe *localValuePx = new LocalValueRecipe(QString("B%1x").arg(i), "magnetic", "magnetic_flux_density_real");
        localValuePx->setPoint(point);
        localValuePx->setComponent(PhysicFieldVariableComp_X);
        Agros2D::problem()->recipes()->addRecipe(localValuePx);

        LocalValueRecipe *localValuePy = new LocalValueRecipe(QString("B%1y").arg(i), "magnetic", "magnetic_flux_density_real");
        localValuePy->setPoint(point);
        localValuePy->setComponent(PhysicFieldVariableComp_Y);
        Agros2D::problem()->recipes()->addRecipe(localValuePy);

        double B0 = 0.35; // 4253 Amps
        // double B0 = 1.5; // 17500 Amps
        func += QString("+ ((%1-%3)**2 + (%2-%4)**2)").
                arg(QString("B%1x").arg(i)).
                arg(QString("B%1y").arg(i)).
                arg(B0*cos(i*step)).
                arg(B0*sin(i*step));
    }

    // add functionals
    analysis->addFunctional(Functional("W", FunctionalType_Minimize, func));

    // solve
    analysis->solve();

    refresh();
}

void OptiLabWidget::testGenetic()
{
    testOptimization(StudyType_Genetic);
}

void OptiLabWidget::testBayesOpt()
{
    testOptimization(StudyType_BayesOptAnalysis);
}

void OptiLabWidget::testNLoptTEAM22()
{
    testTEAM22(StudyType_NLoptAnalysis);
}

void OptiLabWidget::testBayesOptTEAM22()
{
    testTEAM22(StudyType_BayesOptAnalysis);
}

void OptiLabWidget::testBayesOptTEAM25()
{
    testTEAM25(StudyType_BayesOptAnalysis);
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

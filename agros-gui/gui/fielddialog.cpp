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

#include "fielddialog.h"

#include "solver/plugin_interface.h"
#include "solver/plugin_solver_interface.h"

#include "util/global.h"
#include "util/constants.h"

#include "scene.h"
#include "scenenode.h"

#include "solver/problem.h"
#include "solver/module.h"
#include "solver/problem_config.h"

#include "gui/lineeditdouble.h"
#include "gui/valuelineedit.h"
#include "gui/common.h"

FieldWidget::FieldWidget(FieldInfo *fieldInfo, QWidget *parent)
    : QWidget(parent), m_fieldInfo(fieldInfo)
{
    createContent();
    load();
}

void FieldWidget::createContent()
{
    // equations
    equationImage = new QLabel();
    fieldImage = new QLabel();

    cmbAdaptivityType = new QComboBox();
    cmbAnalysisType = new QComboBox();
    cmbLinearityType = new QComboBox();
    cmbLinearSolver = new QComboBox();

    connect(cmbAdaptivityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityChanged(int)));
    connect(cmbAnalysisType, SIGNAL(currentIndexChanged(int)), this, SLOT(doAnalysisTypeChanged(int)));
    connect(cmbLinearityType, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearityTypeChanged(int)));
    connect(cmbLinearSolver, SIGNAL(currentIndexChanged(int)), this, SLOT(doLinearSolverChanged(int)));

    // mesh
    txtNumberOfRefinements = new QSpinBox(this);
    txtNumberOfRefinements->setMinimum(0);
    txtNumberOfRefinements->setMaximum(10);
    txtPolynomialOrder = new QSpinBox(this);
    txtPolynomialOrder->setMinimum(1);
    txtPolynomialOrder->setMaximum(DEALII_MAX_ORDER);

    // table
    auto *layoutGeneral = new QGridLayout();
    layoutGeneral->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutGeneral->setColumnStretch(1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Analysis:")), 0, 0);
    layoutGeneral->addWidget(cmbAnalysisType, 0, 1);
    layoutGeneral->addWidget(new QLabel(tr("Solver:")), 1, 0);
    layoutGeneral->addWidget(cmbLinearityType, 1, 1);
    layoutGeneral->addWidget(new QLabel(tr("Matrix solver:")), 2, 0);
    layoutGeneral->addWidget(cmbLinearSolver, 2, 1);

    auto *grpGeneral = new QGroupBox(tr("General"));
    grpGeneral->setLayout(layoutGeneral);

    // mesh
    auto *layoutMesh = new QGridLayout();
    layoutMesh->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutMesh->setColumnStretch(1, 1);
    layoutMesh->addWidget(new QLabel(tr("Number of refinements:")), 0, 0);
    layoutMesh->addWidget(txtNumberOfRefinements, 0, 1);
    layoutMesh->addWidget(new QLabel(tr("Polynomial order:")), 1, 0);
    layoutMesh->addWidget(txtPolynomialOrder, 1, 1);
    layoutMesh->addWidget(new QLabel(tr("Space adaptivity:")), 2, 0);
    layoutMesh->addWidget(cmbAdaptivityType, 2, 1);
    layoutMesh->setRowStretch(50, 1);

    auto *grpMesh = new QGroupBox(tr("Mesh parameters"));
    grpMesh->setLayout(layoutMesh);

    // left
    auto *layoutLeft = new QVBoxLayout();
    layoutLeft->addWidget(grpGeneral);
    layoutLeft->addStretch();

    // right
    auto *layoutRight = new QVBoxLayout();
    layoutRight->addWidget(grpMesh);
    layoutRight->addStretch();

    // both
    auto *layoutPanel = new QHBoxLayout();
    layoutPanel->addLayout(layoutLeft);
    layoutPanel->addLayout(layoutRight);

    // equation
    auto *layoutEquation = new QHBoxLayout();
    layoutEquation->addWidget(equationImage);
    layoutEquation->addStretch(1);
    layoutEquation->addWidget(fieldImage);

    // tabs
    auto *tabWidget = new QTabWidget(this);
    tabWidget->addTab(createSolverWidget(), tr("Solver"));
    tabWidget->addTab(createAdaptivityWidget(), tr("Space adaptivity"));
    tabWidget->addTab(createTransientAnalysisWidget(), tr("Transient analysis"));
    tabWidget->addTab(createLinearSolverWidget(), tr("Matrix solver"));

    auto *grpEquation = new QGroupBox(tr("Partial differential equation"));
    grpEquation->setLayout(layoutEquation);

    auto *layoutProblem = new QVBoxLayout();
    layoutProblem->addWidget(grpEquation);
    layoutProblem->addLayout(layoutPanel);
    layoutProblem->addWidget(tabWidget);

    // fill combobox
    fillComboBox();

    setLayout(layoutProblem);
}

QWidget *FieldWidget::createSolverWidget()
{
    // linearity
    chkNonlinearResidual = new QCheckBox(this);
    connect(chkNonlinearResidual, SIGNAL(stateChanged(int)), this, SLOT(doNonlinearResidual(int)));
    txtNonlinearResidual = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearResidualNorm).toDouble());
    txtNonlinearResidual->setBottom(0.0);

    chkNonlinearRelativeChangeOfSolutions = new QCheckBox(this);
    connect(chkNonlinearRelativeChangeOfSolutions, SIGNAL(stateChanged(int)), this, SLOT(doNonlinearRelativeChangeOfSolutions(int)));
    txtNonlinearRelativeChangeOfSolutions = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());
    txtNonlinearRelativeChangeOfSolutions->setBottom(0.0);

    auto *layoutSolverConvergence = new QGridLayout();
    layoutSolverConvergence->setColumnMinimumWidth(0, columnMinimumWidth());

    layoutSolverConvergence->addWidget(new QLabel(tr("Relative change of solutions (%):")), 0, 0);
    layoutSolverConvergence->addWidget(chkNonlinearRelativeChangeOfSolutions, 0, 1);
    layoutSolverConvergence->addWidget(txtNonlinearRelativeChangeOfSolutions, 0, 2);
    layoutSolverConvergence->addWidget(new QLabel(tr("Residual:")), 1, 0);
    layoutSolverConvergence->addWidget(chkNonlinearResidual, 1, 1);
    layoutSolverConvergence->addWidget(txtNonlinearResidual, 1, 2);

    auto *grpSolverConvergence = new QGroupBox(tr("Convergence (all selected conditions have to be satisfied)"));
    grpSolverConvergence->setLayout(layoutSolverConvergence);

    cmbNonlinearDampingType = new QComboBox();
    connect(cmbNonlinearDampingType, SIGNAL(currentIndexChanged(int)), this, SLOT(doNonlinearDampingChanged(int)));
    txtNonlinearDampingCoeff = new LineEditDouble(m_fieldInfo->defaultValue(FieldInfo::NonlinearDampingCoeff).toDouble());
    txtNonlinearDampingCoeff->setBottom(0.0);
    txtNonlinearDampingStepsForFactorIncrease = new QSpinBox(this);
    txtNonlinearDampingStepsForFactorIncrease->setMinimum(1);
    txtNonlinearDampingStepsForFactorIncrease->setMaximum(5);
    txtNonlinearDampingRatioForFactorDecrease = new LineEditDouble();

    auto *layoutSolverDamping = new QGridLayout();

    layoutSolverDamping->addWidget(new QLabel(tr("Damping type:")), 0, 0);
    layoutSolverDamping->addWidget(cmbNonlinearDampingType, 0, 1);
    layoutSolverDamping->addWidget(new QLabel(tr("Factor:")), 1, 0);
    layoutSolverDamping->addWidget(txtNonlinearDampingCoeff, 1, 1);
    layoutSolverDamping->addWidget(new QLabel(tr("Min. residual ratio for factor decrease:")), 0, 2);
    layoutSolverDamping->addWidget(txtNonlinearDampingRatioForFactorDecrease, 0, 3);
    layoutSolverDamping->addWidget(new QLabel(tr("Min. steps for factor increase:")), 1, 2);
    layoutSolverDamping->addWidget(txtNonlinearDampingStepsForFactorIncrease, 1, 3);

    auto *grpSolverDamping = new QGroupBox(tr("Damping"));
    grpSolverDamping->setLayout(layoutSolverDamping);

    chkNewtonReuseJacobian = new QCheckBox(tr("Reuse Jacobian if possible"));
    connect(chkNewtonReuseJacobian, SIGNAL(toggled(bool)), this, SLOT(doNewtonReuseJacobian(bool)));
    txtNewtonSufficientImprovementFactorForJacobianReuse = new LineEditDouble();
    txtNewtonMaximumStepsWithReusedJacobian = new QSpinBox(this);
    txtNewtonMaximumStepsWithReusedJacobian->setMinimum(0);
    txtNewtonMaximumStepsWithReusedJacobian->setMaximum(100);

    // Newton's solver
    auto *layoutNewtonSolverReuse = new QGridLayout();
    layoutNewtonSolverReuse->addWidget(chkNewtonReuseJacobian, 0, 0);
    layoutNewtonSolverReuse->addWidget(new QLabel(tr("Max. residual ratio for Jacobian reuse:")), 1, 0);
    layoutNewtonSolverReuse->addWidget(txtNewtonSufficientImprovementFactorForJacobianReuse, 1, 1);
    layoutNewtonSolverReuse->addWidget(new QLabel(tr("Max. steps with the same Jacobian:")), 2, 0);
    layoutNewtonSolverReuse->addWidget(txtNewtonMaximumStepsWithReusedJacobian, 2, 1);
    layoutNewtonSolverReuse->setRowStretch(50, 1);

    QWidget *widgetNewtonSolver = new QWidget(this);
    widgetNewtonSolver->setLayout(layoutNewtonSolverReuse);

    // Picard's solver
    chkPicardAndersonAcceleration = new QCheckBox(tr("Use Anderson acceleration"));
    connect(chkPicardAndersonAcceleration, SIGNAL(stateChanged(int)), this, SLOT(doPicardAndersonChanged(int)));
    txtPicardAndersonBeta = new LineEditDouble(0.2);
    txtPicardAndersonBeta->setBottom(0.0);
    txtPicardAndersonBeta->setTop(1.0);
    txtPicardAndersonNumberOfLastVectors = new QSpinBox(this);
    txtPicardAndersonNumberOfLastVectors->setMinimum(1);
    txtPicardAndersonNumberOfLastVectors->setMaximum(5);

    auto *layoutPicardSolver = new QGridLayout();
    layoutPicardSolver->addWidget(chkPicardAndersonAcceleration, 0, 0, 1, 2);
    layoutPicardSolver->addWidget(new QLabel(tr("Anderson beta:")), 1, 0);
    layoutPicardSolver->addWidget(txtPicardAndersonBeta, 1, 1);
    layoutPicardSolver->addWidget(new QLabel(tr("Num. of last used iter.:")), 2, 0);
    layoutPicardSolver->addWidget(txtPicardAndersonNumberOfLastVectors, 2, 1);
    layoutPicardSolver->setRowStretch(50, 1);

    QWidget *widgetPicardSolver = new QWidget(this);
    widgetPicardSolver->setLayout(layoutPicardSolver);

    QTabWidget *tab = new QTabWidget(this);
    tab->addTab(widgetNewtonSolver, tr("Newton's solver"));
    tab->addTab(widgetPicardSolver, tr("Picard's solver"));

    auto *layoutSolver = new QVBoxLayout();
    layoutSolver->addWidget(grpSolverConvergence);
    layoutSolver->addWidget(grpSolverDamping);
    layoutSolver->addWidget(tab);

    QWidget *widSolver = new QWidget(this);
    widSolver->setLayout(layoutSolver);

    return widSolver;
}

QWidget *FieldWidget::createAdaptivityWidget()
{
    txtAdaptivitySteps = new QSpinBox(this);
    txtAdaptivitySteps->setMinimum(1);
    txtAdaptivitySteps->setMaximum(100);
    txtAdaptivitySteps->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivitySteps).toInt());
    chkAdaptivityTolerance = new QCheckBox(this);
    connect(chkAdaptivityTolerance, SIGNAL(stateChanged(int)), this, SLOT(doAdaptivityTolerance(int)));
    txtAdaptivityTolerance = new LineEditDouble(1.0);
    txtAdaptivityTolerance->setBottom(0.0);
    cmbAdaptivityEstimator = new QComboBox();
    foreach (QString type, adaptivityEstimatorStringKeys())
        if (adaptivityEstimatorFromStringKey(type) != AdaptivityEstimator_Undefined)
            cmbAdaptivityEstimator->addItem(adaptivityEstimatorString(adaptivityEstimatorFromStringKey(type)),
                                            adaptivityEstimatorFromStringKey(type));
    cmbAdaptivityStrategy = new QComboBox();
    foreach (QString type, adaptivityStrategyStringKeys())
        if (adaptivityStrategyFromStringKey(type) != AdaptivityStrategy_Undefined)
            cmbAdaptivityStrategy->addItem(adaptivityStrategyString(adaptivityStrategyFromStringKey(type)),
                                           adaptivityStrategyFromStringKey(type));
    connect(cmbAdaptivityStrategy, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStrategyChanged(int)));

    cmbAdaptivityStrategyHP = new QComboBox();
    foreach (QString type, adaptivityStrategyHPStringKeys())
        if (adaptivityStrategyHPFromStringKey(type) != AdaptivityStrategyHP_Undefined)
            cmbAdaptivityStrategyHP->addItem(adaptivityStrategyHPString(adaptivityStrategyHPFromStringKey(type)),
                                             adaptivityStrategyHPFromStringKey(type));
    connect(cmbAdaptivityStrategyHP, SIGNAL(currentIndexChanged(int)), this, SLOT(doAdaptivityStrategyHPChanged(int)));

    txtAdaptivityFineFraction = new QSpinBox(this);
    txtAdaptivityFineFraction->setMinimum(0);
    txtAdaptivityFineFraction->setMaximum(100);
    txtAdaptivityFineFraction->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivityFinePercentage).toInt());
    txtAdaptivityCoarseFraction = new QSpinBox(this);
    txtAdaptivityCoarseFraction->setMinimum(0);
    txtAdaptivityCoarseFraction->setMaximum(100);
    txtAdaptivityCoarseFraction->setValue(m_fieldInfo->defaultValue(FieldInfo::AdaptivityCoarsePercentage).toInt());
    txtAdaptivityBackSteps = new QSpinBox(this);
    txtAdaptivityBackSteps->setMinimum(0);
    txtAdaptivityBackSteps->setMaximum(100);
    txtAdaptivityRedoneEach = new QSpinBox(this);
    txtAdaptivityRedoneEach->setMinimum(1);
    txtAdaptivityRedoneEach->setMaximum(100);

    // control
    auto *layoutAdaptivityControl = new QGridLayout();
    layoutAdaptivityControl->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutAdaptivityControl->addWidget(new QLabel(tr("Maximum steps:")), 0, 0);
    layoutAdaptivityControl->addWidget(txtAdaptivitySteps, 0, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Tolerance (%):")), 1, 0);
    layoutAdaptivityControl->addWidget(chkAdaptivityTolerance, 1, 1, 1, 1, Qt::AlignRight);
    layoutAdaptivityControl->addWidget(txtAdaptivityTolerance, 1, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Error estimator:")), 2, 0);
    layoutAdaptivityControl->addWidget(cmbAdaptivityEstimator, 2, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Control strategy:")), 3, 0);
    layoutAdaptivityControl->addWidget(cmbAdaptivityStrategy, 3, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Percentage to be refined:")), 4, 0);
    layoutAdaptivityControl->addWidget(txtAdaptivityFineFraction, 4, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("Percentage to be coarsened:")), 5, 0);
    layoutAdaptivityControl->addWidget(txtAdaptivityCoarseFraction, 5, 2);
    layoutAdaptivityControl->addWidget(new QLabel(tr("<i>hp</i>-adaptivity strategy:")), 6, 0);
    layoutAdaptivityControl->addWidget(cmbAdaptivityStrategyHP, 6, 2);
    layoutAdaptivityControl->setRowStretch(50, 1);

    auto *grpControl = new QGroupBox(tr("Control"), this);
    grpControl->setLayout(layoutAdaptivityControl);

    // transient
    auto *layoutAdaptivityTransient = new QGridLayout();
    layoutAdaptivityTransient->addWidget(new QLabel(tr("Steps back in transient:")), 0, 0);
    layoutAdaptivityTransient->addWidget(txtAdaptivityBackSteps, 0, 1);
    layoutAdaptivityTransient->addWidget(new QLabel(tr("Redone each trans. step:")), 1, 0);
    layoutAdaptivityTransient->addWidget(txtAdaptivityRedoneEach, 1, 1);
    layoutAdaptivityTransient->setRowStretch(50, 1);

    auto *grpTransient = new QGroupBox(tr("Transient analysis"), this);
    grpTransient->setLayout(layoutAdaptivityTransient);

    auto *layoutAdaptivity = new QVBoxLayout();
    layoutAdaptivity->addWidget(grpControl);
    layoutAdaptivity->addWidget(grpTransient);
    layoutAdaptivity->addStretch(1);

    QWidget *widAdaptivity = new QWidget(this);
    widAdaptivity->setLayout(layoutAdaptivity);

    return widAdaptivity;
}

QWidget *FieldWidget::createTransientAnalysisWidget()
{
    // transient
    txtTransientInitialCondition = new LineEditDouble(0.0);
    txtTransientTimeSkip = new LineEditDouble(0.0);
    txtTransientTimeSkip->setBottom(0.0);

    // transient analysis
    auto *layoutTransientAnalysis = new QGridLayout();
    layoutTransientAnalysis->setColumnMinimumWidth(0, columnMinimumWidth());
    layoutTransientAnalysis->setColumnStretch(1, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Initial condition:")), 0, 0);
    layoutTransientAnalysis->addWidget(txtTransientInitialCondition, 0, 1);
    layoutTransientAnalysis->addWidget(new QLabel(tr("Time skip (s):")), 1, 0);
    layoutTransientAnalysis->addWidget(txtTransientTimeSkip, 1, 1);
    layoutTransientAnalysis->setRowStretch(50, 1);

    QWidget *widTransientAnalysis = new QWidget(this);
    widTransientAnalysis->setLayout(layoutTransientAnalysis);

    return widTransientAnalysis;
}

QWidget *FieldWidget::createLinearSolverWidget()
{
    cmbIterLinearSolverDealIIMethod = new QComboBox();
    cmbIterLinearSolverDealIIPreconditioner = new QComboBox();
    txtIterLinearSolverToleranceAbsolute = new LineEditDouble(1e-15);
    txtIterLinearSolverToleranceAbsolute->setBottom(0.0);
    txtIterLinearSolverIters = new QSpinBox();
    txtIterLinearSolverIters->setMinimum(1);
    txtIterLinearSolverIters->setMaximum(10000);
    cmbExternalLinearSolverCommand = new QComboBox();
    txtExternalLinearSolverWorkingDirectory = new QLineEdit();
    txtExternalLinearSolverExecutable = new QLineEdit();

    auto *iterSolverDealIILayout = new QGridLayout();
    iterSolverDealIILayout->addWidget(new QLabel(tr("Method:")), 0, 0);
    iterSolverDealIILayout->addWidget(cmbIterLinearSolverDealIIMethod, 0, 1);
    iterSolverDealIILayout->addWidget(new QLabel(tr("Preconditioner:")), 1, 0);
    iterSolverDealIILayout->addWidget(cmbIterLinearSolverDealIIPreconditioner, 1, 1);
    iterSolverDealIILayout->addWidget(new QLabel(tr("Absolute tolerance:")), 0, 2);
    iterSolverDealIILayout->addWidget(txtIterLinearSolverToleranceAbsolute, 0, 3);
    iterSolverDealIILayout->addWidget(new QLabel(tr("Maximum number of iterations:")), 1, 2);
    iterSolverDealIILayout->addWidget(txtIterLinearSolverIters, 1, 3);

    auto *iterSolverDealIIGroup = new QGroupBox(tr("deal.II"));
    iterSolverDealIIGroup->setLayout(iterSolverDealIILayout);

    auto *externalSolverLayout = new QGridLayout();
    externalSolverLayout->addWidget(new QLabel(tr("Solver:")), 0, 0);
    externalSolverLayout->addWidget(cmbExternalLinearSolverCommand, 0, 1);
    externalSolverLayout->addWidget(new QLabel(tr("Working directory:")), 1, 0);
    externalSolverLayout->addWidget(txtExternalLinearSolverWorkingDirectory, 1, 1);
    externalSolverLayout->addWidget(new QLabel(tr("Executable:")), 2, 0);
    externalSolverLayout->addWidget(txtExternalLinearSolverExecutable, 2, 1);

    auto *externalSolverGroup = new QGroupBox(tr("External"));
    externalSolverGroup->setLayout(externalSolverLayout);

    auto *layoutLinearSolver = new QVBoxLayout();
    layoutLinearSolver->addWidget(iterSolverDealIIGroup);
    layoutLinearSolver->addWidget(externalSolverGroup);
    layoutLinearSolver->addStretch();

    QWidget *widLinearSolver = new QWidget(this);
    widLinearSolver->setLayout(layoutLinearSolver);

    return widLinearSolver;
}

void FieldWidget::fillComboBox()
{
    cmbNonlinearDampingType->clear();
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Automatic), DampingType_Automatic);
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Fixed), DampingType_Fixed);
    cmbNonlinearDampingType->addItem(dampingTypeString(DampingType_Off), DampingType_Off);

    cmbAdaptivityType->clear();
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_None), AdaptivityMethod_None);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_H), AdaptivityMethod_H);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_P), AdaptivityMethod_P);
    cmbAdaptivityType->addItem(adaptivityTypeString(AdaptivityMethod_HP), AdaptivityMethod_HP);

    foreach(LinearityType linearityType, m_fieldInfo->availableLinearityTypes())
    {
        cmbLinearityType->addItem(linearityTypeString(linearityType), linearityType);
    }

    QMapIterator<AnalysisType, QString> it(m_fieldInfo->analyses());
    while (it.hasNext())
    {
        it.next();
        cmbAnalysisType->addItem(it.value(), it.key());
    }

    cmbLinearSolver->clear();
    foreach (QString solver, matrixSolverTypeStringKeys())
        cmbLinearSolver->addItem(matrixSolverTypeString(matrixSolverTypeFromStringKey(solver)), matrixSolverTypeFromStringKey(solver));

    cmbIterLinearSolverDealIIMethod->clear();
    foreach (QString method, iterLinearSolverDealIIMethodStringKeys())
        cmbIterLinearSolverDealIIMethod->addItem(iterLinearSolverDealIIMethodString(iterLinearSolverDealIIMethodFromStringKey(method)), iterLinearSolverDealIIMethodFromStringKey(method));

    cmbIterLinearSolverDealIIPreconditioner->clear();
    foreach (QString type, iterLinearSolverDealIIPreconditionerStringKeys())
        cmbIterLinearSolverDealIIPreconditioner->addItem(iterLinearSolverDealIIPreconditionerString(iterLinearSolverDealIIPreconditionerFromStringKey(type)), iterLinearSolverDealIIPreconditionerFromStringKey(type));

    // read from files
    cmbExternalLinearSolverCommand->clear();
    foreach (PluginSolverInterface *solver, Agros::solvers())
    {
        cmbExternalLinearSolverCommand->addItem(solver->name(), solver->name());
    }
    if (cmbExternalLinearSolverCommand->count() > 0)
        cmbExternalLinearSolverCommand->setCurrentIndex(0);
}

void FieldWidget::load()
{
    // analysis type
    cmbAnalysisType->setCurrentIndex(cmbAnalysisType->findData(m_fieldInfo->analysisType()));
    if (cmbAnalysisType->currentIndex() == -1)
        cmbAnalysisType->setCurrentIndex(0);
    // adaptivity
    cmbAdaptivityType->setCurrentIndex(cmbAdaptivityType->findData(m_fieldInfo->adaptivityType()));
    txtAdaptivitySteps->setValue(m_fieldInfo->value(FieldInfo::AdaptivitySteps).toInt());
    chkAdaptivityTolerance->setChecked(m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble() > 0);
    txtAdaptivityTolerance->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTolerance).toDouble());
    txtAdaptivityFineFraction->setValue(m_fieldInfo->value(FieldInfo::AdaptivityFinePercentage).toInt());
    txtAdaptivityCoarseFraction->setValue(m_fieldInfo->value(FieldInfo::AdaptivityCoarsePercentage).toInt());
    cmbAdaptivityEstimator->setCurrentIndex(cmbAdaptivityEstimator->findData((AdaptivityEstimator) m_fieldInfo->value(FieldInfo::AdaptivityEstimator).toInt()));
    cmbAdaptivityStrategy->setCurrentIndex(cmbAdaptivityStrategy->findData((AdaptivityStrategy) m_fieldInfo->value(FieldInfo::AdaptivityStrategy).toInt()));
    cmbAdaptivityStrategyHP->setCurrentIndex(cmbAdaptivityStrategyHP->findData((AdaptivityStrategyHP) m_fieldInfo->value(FieldInfo::AdaptivityStrategyHP).toInt()));
    txtAdaptivityBackSteps->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTransientBackSteps).toInt());
    txtAdaptivityRedoneEach->setValue(m_fieldInfo->value(FieldInfo::AdaptivityTransientRedoneEach).toInt());
    // matrix solver
    cmbLinearSolver->setCurrentIndex(cmbLinearSolver->findData(m_fieldInfo->matrixSolver()));
    //mesh
    txtNumberOfRefinements->setValue(m_fieldInfo->value(FieldInfo::SpaceNumberOfRefinements).toInt());
    txtPolynomialOrder->setValue(m_fieldInfo->value(FieldInfo::SpacePolynomialOrder).toInt());
    // transient
    txtTransientInitialCondition->setValue(m_fieldInfo->value(FieldInfo::TransientInitialCondition).toDouble());
    txtTransientTimeSkip->setValue(m_fieldInfo->value(FieldInfo::TransientTimeSkip).toDouble());
    // linearity
    cmbLinearityType->setCurrentIndex(cmbLinearityType->findData(m_fieldInfo->linearityType()));
    txtNonlinearResidual->setValue(m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble());
    chkNonlinearResidual->setChecked(m_fieldInfo->value(FieldInfo::NonlinearResidualNorm).toDouble() > 0);
    chkNonlinearRelativeChangeOfSolutions->setChecked(m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble() > 0);
    txtNonlinearRelativeChangeOfSolutions->setValue(m_fieldInfo->value(FieldInfo::NonlinearRelativeChangeOfSolutions).toDouble());
    cmbNonlinearDampingType->setCurrentIndex(cmbNonlinearDampingType->findData((DampingType) m_fieldInfo->value(FieldInfo::NonlinearDampingType).toInt()));
    txtNonlinearDampingCoeff->setValue(m_fieldInfo->value(FieldInfo::NonlinearDampingCoeff).toDouble());
    txtNewtonSufficientImprovementFactorForJacobianReuse->setValue(m_fieldInfo->value(FieldInfo::NewtonJacobianReuseRatio).toDouble());
    txtNonlinearDampingRatioForFactorDecrease->setValue(m_fieldInfo->value(FieldInfo::NonlinearDampingFactorDecreaseRatio).toDouble());
    txtNewtonMaximumStepsWithReusedJacobian->setValue(m_fieldInfo->value(FieldInfo::NewtonMaxStepsReuseJacobian).toInt());
    txtNonlinearDampingStepsForFactorIncrease->setValue(m_fieldInfo->value(FieldInfo::NonlinearStepsToIncreaseDampingFactor).toInt());
    chkNewtonReuseJacobian->setChecked((m_fieldInfo->value(FieldInfo::NewtonReuseJacobian)).toBool());
    chkPicardAndersonAcceleration->setChecked(m_fieldInfo->value(FieldInfo::PicardAndersonAcceleration).toBool());
    txtPicardAndersonBeta->setValue(m_fieldInfo->value(FieldInfo::PicardAndersonBeta).toDouble());
    txtPicardAndersonNumberOfLastVectors->setValue(m_fieldInfo->value(FieldInfo::PicardAndersonNumberOfLastVectors).toInt());
    // linear solver
    txtIterLinearSolverToleranceAbsolute->setValue(m_fieldInfo->value(FieldInfo::LinearSolverIterToleranceAbsolute).toDouble());
    txtIterLinearSolverIters->setValue(m_fieldInfo->value(FieldInfo::LinearSolverIterIters).toInt());
    cmbIterLinearSolverDealIIMethod->setCurrentIndex((IterSolverDealII) cmbIterLinearSolverDealIIMethod->findData(m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIMethod).toInt()));
    cmbIterLinearSolverDealIIPreconditioner->setCurrentIndex((PreconditionerDealII) cmbIterLinearSolverDealIIPreconditioner->findData(m_fieldInfo->value(FieldInfo::LinearSolverIterDealIIPreconditioner).toInt()));
    // external solver
    cmbExternalLinearSolverCommand->setCurrentIndex(cmbExternalLinearSolverCommand->findData(m_fieldInfo->value(FieldInfo::LinearSolverExternalName).toString()));
    if (cmbExternalLinearSolverCommand->currentIndex() == -1)
        cmbExternalLinearSolverCommand->setCurrentIndex(0);
    txtExternalLinearSolverWorkingDirectory->setText(m_fieldInfo->value(FieldInfo::LinearSolverExternalWorkingDirectory).toString());
    txtExternalLinearSolverExecutable->setText(m_fieldInfo->value(FieldInfo::LinearSolverExternalExecutable).toString());

    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

bool FieldWidget::save()
{
    // analysis type
    m_fieldInfo->setAnalysisType((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt());
    // adaptivity
    m_fieldInfo->setAdaptivityType((AdaptivityMethod) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivitySteps, txtAdaptivitySteps->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTolerance, chkAdaptivityTolerance->isChecked() ? txtAdaptivityTolerance->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::AdaptivityFinePercentage, txtAdaptivityFineFraction->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityCoarsePercentage, txtAdaptivityCoarseFraction->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityEstimator, (AdaptivityEstimator) cmbAdaptivityEstimator->itemData(cmbAdaptivityEstimator->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityStrategy, (AdaptivityStrategy) cmbAdaptivityStrategy->itemData(cmbAdaptivityStrategy->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityStrategyHP, (AdaptivityStrategyHP) cmbAdaptivityStrategyHP->itemData(cmbAdaptivityStrategyHP->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTransientBackSteps, txtAdaptivityBackSteps->value());
    m_fieldInfo->setValue(FieldInfo::AdaptivityTransientRedoneEach, txtAdaptivityRedoneEach->value());
    // matrix solver
    m_fieldInfo->setMatrixSolver((MatrixSolverType) cmbLinearSolver->itemData(cmbLinearSolver->currentIndex()).toInt());
    //mesh
    m_fieldInfo->setValue(FieldInfo::SpaceNumberOfRefinements, txtNumberOfRefinements->value());
    m_fieldInfo->setValue(FieldInfo::SpacePolynomialOrder, txtPolynomialOrder->value());
    // transient
    m_fieldInfo->setValue(FieldInfo::TransientInitialCondition, txtTransientInitialCondition->value());
    m_fieldInfo->setValue(FieldInfo::TransientTimeSkip, txtTransientTimeSkip->value());
    // linearity
    m_fieldInfo->setLinearityType((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::NonlinearResidualNorm, chkNonlinearResidual->isChecked() ? txtNonlinearResidual->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::NonlinearRelativeChangeOfSolutions, chkNonlinearRelativeChangeOfSolutions->isChecked() ? txtNonlinearRelativeChangeOfSolutions->value() : 0.0);
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingCoeff, txtNonlinearDampingCoeff->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingType, (DampingType) cmbNonlinearDampingType->itemData(cmbNonlinearDampingType->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::NewtonReuseJacobian, chkNewtonReuseJacobian->isChecked());
    m_fieldInfo->setValue(FieldInfo::NewtonJacobianReuseRatio, txtNewtonSufficientImprovementFactorForJacobianReuse->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearDampingFactorDecreaseRatio, txtNonlinearDampingRatioForFactorDecrease->value());
    m_fieldInfo->setValue(FieldInfo::NewtonMaxStepsReuseJacobian, txtNewtonMaximumStepsWithReusedJacobian->value());
    m_fieldInfo->setValue(FieldInfo::NonlinearStepsToIncreaseDampingFactor, txtNonlinearDampingStepsForFactorIncrease->value());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonAcceleration, chkPicardAndersonAcceleration->isChecked());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonBeta, txtPicardAndersonBeta->value());
    m_fieldInfo->setValue(FieldInfo::PicardAndersonNumberOfLastVectors, txtPicardAndersonNumberOfLastVectors->value());
    // linear solver
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterToleranceAbsolute, txtIterLinearSolverToleranceAbsolute->value());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterIters, txtIterLinearSolverIters->value());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterDealIIMethod, cmbIterLinearSolverDealIIMethod->itemData(cmbIterLinearSolverDealIIMethod->currentIndex()).toInt());
    m_fieldInfo->setValue(FieldInfo::LinearSolverIterDealIIPreconditioner, cmbIterLinearSolverDealIIPreconditioner->itemData(cmbIterLinearSolverDealIIPreconditioner->currentIndex()).toInt());
    // external solver
    m_fieldInfo->setValue(FieldInfo::LinearSolverExternalName, cmbExternalLinearSolverCommand->itemData(cmbExternalLinearSolverCommand->currentIndex()).toString());
    m_fieldInfo->setValue(FieldInfo::LinearSolverExternalWorkingDirectory, txtExternalLinearSolverWorkingDirectory->text());
    m_fieldInfo->setValue(FieldInfo::LinearSolverExternalExecutable, txtExternalLinearSolverExecutable->text());

    return true;
}

void FieldWidget::refresh()
{
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

FieldInfo *FieldWidget::fieldInfo()
{
    return m_fieldInfo;
}

void FieldWidget::doAnalysisTypeClicked()
{
    doAnalysisTypeChanged(cmbAnalysisType->currentIndex());
}

void FieldWidget::doAnalysisTypeChanged(int index)
{
    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();

    // initial condition
    txtTransientInitialCondition->setEnabled(analysisType == AnalysisType_Transient);

    // time steps skip
    bool otherFieldIsTransient = false;
    foreach (FieldInfo* otherFieldInfo, Agros::problem()->fieldInfos())
        if (otherFieldInfo->analysisType() == AnalysisType_Transient && otherFieldInfo->fieldId() != m_fieldInfo->fieldId())
            otherFieldIsTransient = true;

    txtTransientTimeSkip->setEnabled(!(m_fieldInfo->analysisType() == AnalysisType_Transient) && otherFieldIsTransient);

    LinearityType previousLinearityType = (LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt();
    cmbLinearityType->clear();

    int idx = 0, nextIndex = 0;
    foreach(LinearityType linearityType, m_fieldInfo->availableLinearityTypes((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt()))
    {
        cmbLinearityType->addItem(linearityTypeString(linearityType), linearityType);

        if (linearityType == previousLinearityType)
            nextIndex = idx;
        idx++;
    }
    cmbLinearityType->setCurrentIndex(nextIndex);

    doShowEquation();
    doAdaptivityChanged(cmbAdaptivityType->currentIndex());
}

void FieldWidget::doShowEquation()
{
    QPixmap pixmapEquation(QString("%1/resources/images/equations/%2_equation_%3.png").
                   arg(Agros::dataDir()).
                   arg(m_fieldInfo->fieldId()).
                   arg(analysisTypeToStringKey((AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt())));

    equationImage->setPixmap(pixmapEquation);
    equationImage->setMask(pixmapEquation.mask());

    QPixmap pixmapField = icon(QString("fields/%1").arg(m_fieldInfo->fieldId())).pixmap(32, 32);

    fieldImage->setPixmap(pixmapField);
    // fieldImage->setMask(pixmapField.mask());
}

void FieldWidget::doAdaptivityChanged(int index)
{
    txtAdaptivitySteps->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    chkAdaptivityTolerance->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    txtAdaptivityTolerance->setEnabled(((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None) && chkAdaptivityTolerance->isChecked());
    cmbAdaptivityEstimator->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    cmbAdaptivityStrategy->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    cmbAdaptivityStrategyHP->setEnabled((AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() == AdaptivityMethod_HP);

    AnalysisType analysisType = (AnalysisType) cmbAnalysisType->itemData(cmbAnalysisType->currentIndex()).toInt();
    txtAdaptivityBackSteps->setEnabled(Agros::problem()->isTransient() && analysisType != AnalysisType_Transient && (AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);
    txtAdaptivityRedoneEach->setEnabled(Agros::problem()->isTransient() && analysisType != AnalysisType_Transient && (AdaptivityMethod) cmbAdaptivityType->itemData(index).toInt() != AdaptivityMethod_None);

    doAdaptivityStrategyChanged(cmbAdaptivityStrategy->currentIndex());
}

void FieldWidget::doAdaptivityStrategyChanged(int index)
{
    bool enabled = (((AdaptivityMethod) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt() != AdaptivityMethod_None) &&
                    (((AdaptivityStrategy) cmbAdaptivityStrategy->itemData(index).toInt() == AdaptivityStrategy_FixedFractionOfCells) ||
                     ((AdaptivityStrategy) cmbAdaptivityStrategy->itemData(index).toInt() == AdaptivityStrategy_FixedFractionOfTotalError)));

    txtAdaptivityFineFraction->setEnabled(enabled);
    txtAdaptivityCoarseFraction->setEnabled(enabled);
}

void FieldWidget::doAdaptivityStrategyHPChanged(int index)
{
}

void FieldWidget::doLinearityTypeChanged(int index)
{
    chkNonlinearResidual->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearResidual->setEnabled(((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear) && chkNonlinearResidual->isChecked());
    chkNonlinearRelativeChangeOfSolutions->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    txtNonlinearRelativeChangeOfSolutions->setEnabled(((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear) && chkNonlinearRelativeChangeOfSolutions->isChecked());

    cmbNonlinearDampingType->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() != LinearityType_Linear);
    doNonlinearDampingChanged(cmbNonlinearDampingType->currentIndex());

    txtNewtonMaximumStepsWithReusedJacobian->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    chkNewtonReuseJacobian->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    txtNewtonSufficientImprovementFactorForJacobianReuse->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Newton);
    doNewtonReuseJacobian(chkNewtonReuseJacobian->isChecked());

    chkPicardAndersonAcceleration->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    txtPicardAndersonBeta->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    txtPicardAndersonNumberOfLastVectors->setEnabled((LinearityType) cmbLinearityType->itemData(index).toInt() == LinearityType_Picard);
    doPicardAndersonChanged(chkPicardAndersonAcceleration->checkState());
}

void FieldWidget::doLinearSolverChanged(int index)
{
    MatrixSolverType solverType = (MatrixSolverType) cmbLinearSolver->itemData(cmbLinearSolver->currentIndex()).toInt();
    bool isIterative = isMatrixSolverIterative(solverType);

    txtIterLinearSolverToleranceAbsolute->setEnabled(isIterative);
    txtIterLinearSolverIters->setEnabled(isIterative);
    cmbIterLinearSolverDealIIMethod->setEnabled(solverType == SOLVER_DEALII);
    cmbIterLinearSolverDealIIPreconditioner->setEnabled(solverType == SOLVER_DEALII);
    cmbExternalLinearSolverCommand->setEnabled(solverType == SOLVER_PLUGIN);
    txtExternalLinearSolverWorkingDirectory->setEnabled(solverType == SOLVER_PLUGIN);
    txtExternalLinearSolverExecutable->setEnabled(solverType == SOLVER_PLUGIN);
}

void FieldWidget::doNonlinearDampingChanged(int index)
{
    txtNonlinearDampingCoeff->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                         ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() != DampingType_Off));
    txtNonlinearDampingStepsForFactorIncrease->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                                          ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() == DampingType_Automatic));
    txtNonlinearDampingRatioForFactorDecrease->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) &&
                                                          ((DampingType) cmbNonlinearDampingType->itemData(index).toInt() == DampingType_Automatic));

}

void FieldWidget::doNewtonReuseJacobian(bool checked)
{
    txtNewtonMaximumStepsWithReusedJacobian->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Newton) &&
                                                        (chkNewtonReuseJacobian->isChecked()));
    txtNewtonSufficientImprovementFactorForJacobianReuse->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Newton) &&
                                                                     (chkNewtonReuseJacobian->isChecked()));
}

void FieldWidget::doPicardAndersonChanged(int index)
{
    txtPicardAndersonBeta->setEnabled((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Picard && chkPicardAndersonAcceleration->isChecked());
    txtPicardAndersonNumberOfLastVectors->setEnabled((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() == LinearityType_Picard && chkPicardAndersonAcceleration->isChecked());
}

void FieldWidget::doAdaptivityTolerance(int state)
{
    txtAdaptivityTolerance->setEnabled(((AdaptivityMethod) cmbAdaptivityType->itemData(cmbAdaptivityType->currentIndex()).toInt() != AdaptivityMethod_None) && chkAdaptivityTolerance->isChecked());
}

void FieldWidget::doNonlinearResidual(int state)
{
    txtNonlinearResidual->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) && chkNonlinearResidual->isChecked());
}

void FieldWidget::doNonlinearRelativeChangeOfSolutions(int state)
{
    txtNonlinearRelativeChangeOfSolutions->setEnabled(((LinearityType) cmbLinearityType->itemData(cmbLinearityType->currentIndex()).toInt() != LinearityType_Linear) && chkNonlinearRelativeChangeOfSolutions->isChecked());
}

// ********************************************************************************************

FieldDialog::FieldDialog(FieldInfo *fieldInfo, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(fieldInfo->name());
    setWindowIcon(icon("fields/" + fieldInfo->fieldId()));

    fieldWidget = new FieldWidget(fieldInfo, this);

    // dialog buttons
    QPushButton *btnDeleteField = new QPushButton(tr("Delete field"));
    btnDeleteField->setDefault(false);
    btnDeleteField->setEnabled(Agros::problem()->hasField(fieldInfo->fieldId()));
    connect(btnDeleteField, SIGNAL(clicked()), this, SLOT(deleteField()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->addButton(btnDeleteField, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doAccept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

    auto *layout = new QVBoxLayout();
    layout->addWidget(fieldWidget);
    layout->addStretch();
    layout->addWidget(buttonBox);

    setLayout(layout);

    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());

    if (QApplication::activeWindow())
        move(QApplication::activeWindow()->pos().x() + (QApplication::activeWindow()->width() - width()) / 2.0,
             QApplication::activeWindow()->pos().y() + (QApplication::activeWindow()->height() - height()) / 2.0);
}

FieldDialog::~FieldDialog()
{
}

void FieldDialog::doAccept()
{
    fieldWidget->save();
    accept();
}

void FieldDialog::deleteField()
{
    if (QMessageBox::question(this, tr("Delete"), tr("Physical field '%1' will be pernamently deleted. Are you sure?").
                              arg(fieldWidget->fieldInfo()->name()), tr("&Yes"), tr("&No")) == 0)
    {
        Agros::problem()->removeField(fieldWidget->fieldInfo());
        accept();
    }
}

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

#include "study_methoddialog.h"

#include "util/global.h"
#include "gui/lineeditdouble.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "optilab/study.h"

#include "qcustomplot/qcustomplot.h"

// *****************************************************************************************************

StudySweepDialog::StudySweepDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudySweepDialog::createStudyControls()
{
    txtNumSamples = new QSpinBox(this);
    txtNumSamples->setMinimum(1);
    txtNumSamples->setMaximum(10000);

    cmbInitMethod = new QComboBox(this);
    foreach (QString key, study()->initMethodStringKeys())
        cmbInitMethod->addItem(study()->initMethodString(study()->initMethodFromStringKey(key)), key);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Number of samples:")), 0, 0);
    layoutInitialization->addWidget(txtNumSamples, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Initial strategy:")), 1, 0);
    layoutInitialization->addWidget(cmbInitMethod, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Sweep analysis"), this);
    grpInitialization->setLayout(layoutInitialization);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);

    return layoutMain;
}

void StudySweepDialog::load()
{
    StudyDialog::load();

    txtNumSamples->setValue(study()->value(Study::Sweep_num_samples).toInt());
    cmbInitMethod->setCurrentIndex(cmbInitMethod->findData(study()->value(Study::Sweep_init_method).toString()));
}

void StudySweepDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::Sweep_num_samples, txtNumSamples->value());
    study()->setValue(Study::Sweep_init_method, cmbInitMethod->itemData(cmbInitMethod->currentIndex()).toString());
}

StudyNSGA3Dialog::StudyNSGA3Dialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

// *****************************************************************************************************

QLayout *StudyNSGA3Dialog::createStudyControls()
{
    txtPopSize = new QSpinBox(this);
    txtPopSize->setMinimum(1);
    txtPopSize->setMaximum(10000);
    txtPopSize->setSingleStep(1);

    txtNGen = new QSpinBox(this);
    txtNGen->setMinimum(1);
    txtNGen->setMaximum(1000);

    txtPCross = new LineEditDouble(0.0);
    txtPCross->setBottom(0.6);
    txtPCross->setTop(1.0);
    // txtPMut = new LineEditDouble(0.0);
    // txtPMut->setBottom(1e-3);
    txtEtaC = new LineEditDouble(0.0);
    txtEtaC->setBottom(5);
    txtEtaC->setTop(30);
    txtEtaM = new LineEditDouble(0.0);
    txtEtaM->setBottom(5);
    txtEtaM->setTop(30);

    chkUseSurrogateFunction = new QCheckBox(tr("Use surrogate function (Gaussian process)"));

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Estimated population size:")), 0, 0);
    layoutInitialization->addWidget(txtPopSize, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Maximum number of generations:")), 1, 0);
    layoutInitialization->addWidget(txtNGen, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout();
    layoutConfig->addWidget(new QLabel(tr("Probability of crossover (0.6 - 1.0):")), 0, 0);
    layoutConfig->addWidget(txtPCross, 0, 1);
    // layoutConfig->addWidget(new QLabel(tr("Probability of mutation (0.6 - 1.0):")), 1, 0);
    // layoutConfig->addWidget(txtPMut, 1, 1);
    layoutConfig->addWidget(new QLabel(tr("Distribution index for crossover (5 - 30):")), 2, 0);
    layoutConfig->addWidget(txtEtaC, 2, 1);
    layoutConfig->addWidget(new QLabel(tr("Distribution index for mutation (5 - 30):")), 3, 0);
    layoutConfig->addWidget(txtEtaM, 3, 1);
    layoutConfig->addWidget(chkUseSurrogateFunction, 4, 0, 1, 2);

    QGroupBox *grpConfig = new QGroupBox(tr("Config"), this);
    grpConfig->setLayout(layoutConfig);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpConfig);

    return layoutMain;
}

void StudyNSGA3Dialog::load()
{
    StudyDialog::load();

    txtPopSize->setValue(study()->value(Study::NSGA3_popsize).toInt());
    txtNGen->setValue(study()->value(Study::NSGA3_ngen).toInt());
    txtPCross->setValue(study()->value(Study::NSGA3_pcross).toDouble());
    // txtPMut->setValue(study()->value(Study::NSGA3_pmut).toDouble());
    txtEtaC->setValue(study()->value(Study::NSGA3_eta_c).toDouble());
    txtEtaM->setValue(study()->value(Study::NSGA3_eta_m).toDouble());
    chkUseSurrogateFunction->setChecked(study()->value(Study::NSGA3_use_surrogate).toBool());
}

void StudyNSGA3Dialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::NSGA3_popsize, txtPopSize->value());
    study()->setValue(Study::NSGA3_ngen, txtNGen->value());
    study()->setValue(Study::NSGA3_pcross, txtPCross->value());
    // study()->setValue(Study::NSGA3_pmut, txtPMut->value());
    study()->setValue(Study::NSGA3_eta_c, txtEtaC->value());
    study()->setValue(Study::NSGA3_eta_m, txtEtaM->value());
    study()->setValue(Study::NSGA3_use_surrogate, chkUseSurrogateFunction->isChecked());
}

// *****************************************************************************************************

StudyNSGA2Dialog::StudyNSGA2Dialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyNSGA2Dialog::createStudyControls()
{
    txtPopSize = new QSpinBox(this);
    txtPopSize->setMinimum(4);
    txtPopSize->setMaximum(4*1000);
    txtPopSize->setSingleStep(4);

    txtNGen = new QSpinBox(this);
    txtNGen->setMinimum(1);
    txtNGen->setMaximum(1000);

    txtPCross = new LineEditDouble(0.0);
    txtPCross->setBottom(0.6);
    txtPCross->setTop(1.0);
    txtPMut = new LineEditDouble(0.0);
    txtPMut->setBottom(1e-3);
    txtEtaC = new LineEditDouble(0.0);
    txtEtaC->setBottom(5);
    txtEtaC->setTop(20);
    txtEtaM = new LineEditDouble(0.0);
    txtEtaM->setBottom(5);
    txtEtaM->setTop(20);

    radCrowdParameters = new QRadioButton(tr("Crowd over the parameters"));
    radCrowdObjective = new QRadioButton(tr("Crowd over the objective function"));

    QButtonGroup *crowdGroup = new QButtonGroup(this);
    crowdGroup->addButton(radCrowdParameters);
    crowdGroup->addButton(radCrowdObjective);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Population size:")), 0, 0);
    layoutInitialization->addWidget(txtPopSize, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Maximum number of generations:")), 1, 0);
    layoutInitialization->addWidget(txtNGen, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout();
    layoutConfig->addWidget(new QLabel(tr("Probability of crossover (0.6 - 1.0):")), 0, 0);
    layoutConfig->addWidget(txtPCross, 0, 1);
    layoutConfig->addWidget(new QLabel(tr("Probability of mutation (0.6 - 1.0):")), 1, 0);
    layoutConfig->addWidget(txtPMut, 1, 1);
    layoutConfig->addWidget(new QLabel(tr("Distribution index for crossover (5 - 20):")), 2, 0);
    layoutConfig->addWidget(txtEtaC, 2, 1);
    layoutConfig->addWidget(new QLabel(tr("Distribution index for mutation (5 - 20):")), 3, 0);
    layoutConfig->addWidget(txtEtaM, 3, 1);
    layoutConfig->addWidget(new QLabel(tr("Crowding:")), 4, 0);
    layoutConfig->addWidget(radCrowdParameters, 4, 1);
    layoutConfig->addWidget(radCrowdObjective, 5, 1);

    QGroupBox *grpConfig = new QGroupBox(tr("Config"), this);
    grpConfig->setLayout(layoutConfig);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpConfig);

    return layoutMain;
}

void StudyNSGA2Dialog::load()
{
    StudyDialog::load();

    txtPopSize->setValue(study()->value(Study::NSGA2_popsize).toInt());
    txtNGen->setValue(study()->value(Study::NSGA2_ngen).toInt());
    txtPCross->setValue(study()->value(Study::NSGA2_pcross).toDouble());
    txtPMut->setValue(study()->value(Study::NSGA2_pmut).toDouble());
    txtEtaC->setValue(study()->value(Study::NSGA2_eta_c).toDouble());
    txtEtaM->setValue(study()->value(Study::NSGA2_eta_m).toDouble());
    if (study()->value(Study::NSGA2_crowdobj).toBool())
        radCrowdObjective->setChecked(true);
    else
        radCrowdParameters->setChecked(true);
}

void StudyNSGA2Dialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::NSGA2_popsize, txtPopSize->value());
    study()->setValue(Study::NSGA2_ngen, txtNGen->value());
    study()->setValue(Study::NSGA2_pcross, txtPCross->value());
    study()->setValue(Study::NSGA2_pmut, txtPMut->value());
    study()->setValue(Study::NSGA2_eta_c, txtEtaC->value());
    study()->setValue(Study::NSGA2_eta_m, txtEtaM->value());
    study()->setValue(Study::NSGA2_crowdobj, radCrowdObjective->isChecked());
}

// *****************************************************************************************************

StudyNLoptDialog::StudyNLoptDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyNLoptDialog::createStudyControls()
{
    txtXRelTol = new LineEditDouble(0.0);
    txtXAbsTol = new LineEditDouble(0.0);
    txtFRelTol = new LineEditDouble(0.0);
    txtFAbsTol = new LineEditDouble(0.0);

    txtNIterations = new QSpinBox(this);
    txtNIterations->setMinimum(0);
    txtNIterations->setMaximum(10000);

    cmbAlgorithm = new QComboBox(this);
    foreach (QString key, study()->algorithmStringKeys())
        cmbAlgorithm->addItem(study()->algorithmString(study()->algorithmFromStringKey(key)), key);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Algorithm:")), 0, 0);
    layoutInitialization->addWidget(cmbAlgorithm, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of iterations:")), 1, 0);
    layoutInitialization->addWidget(txtNIterations, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout();
    layoutConfig->addWidget(new QLabel(tr("Relative tolerance")), 0, 0);
    layoutConfig->addWidget(new QLabel(QString("|&Delta;<i>x</i><sub>i</sub>|/|<i>x</i><sub>i</sub>|:")), 0, 1);
    layoutConfig->addWidget(txtXRelTol, 0, 2);
    layoutConfig->addWidget(new QLabel(QString("|&Delta;<i>f</i>|/|<i>f</i>|:")), 0, 3);
    layoutConfig->addWidget(txtFRelTol, 0, 4);
    layoutConfig->addWidget(new QLabel(tr("Absolute tolerance")), 1, 0);
    layoutConfig->addWidget(new QLabel(QString("|&Delta;<i>x</i><sub>i</sub>|:")), 1, 1);
    layoutConfig->addWidget(txtXAbsTol, 1, 2);
    layoutConfig->addWidget(new QLabel(QString("|&Delta;<i>f</i>|:")), 1, 3);
    layoutConfig->addWidget(txtFAbsTol, 1, 4);

    QGroupBox *grpConfig = new QGroupBox(tr("Config"), this);
    grpConfig->setLayout(layoutConfig);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpConfig);

    return layoutMain;
}

void StudyNLoptDialog::load()
{
    StudyDialog::load();

    txtXRelTol->setValue(study()->value(Study::NLopt_xtol_rel).toDouble());
    txtXAbsTol->setValue(study()->value(Study::NLopt_xtol_abs).toDouble());
    txtFRelTol->setValue(study()->value(Study::NLopt_ftol_rel).toDouble());
    txtFAbsTol->setValue(study()->value(Study::NLopt_ftol_abs).toDouble());
    txtNIterations->setValue(study()->value(Study::NLopt_n_iterations).toDouble());
    cmbAlgorithm->setCurrentIndex(cmbAlgorithm->findData(study()->value(Study::NLopt_algorithm).toString()));
}

void StudyNLoptDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::NLopt_xtol_rel, txtXRelTol->value());
    study()->setValue(Study::NLopt_xtol_abs, txtXAbsTol->value());
    study()->setValue(Study::NLopt_ftol_rel, txtFRelTol->value());
    study()->setValue(Study::NLopt_ftol_abs, txtFAbsTol->value());
    study()->setValue(Study::NLopt_n_iterations, txtNIterations->value());
    study()->setValue(Study::NLopt_algorithm, cmbAlgorithm->itemData(cmbAlgorithm->currentIndex()).toString());
}


// *****************************************************************************************************

StudyCMAESDialog::StudyCMAESDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyCMAESDialog::createStudyControls()
{
    txtFTarget = new LineEditDouble(0.0);

    txtMaxIter = new QSpinBox(this);
    txtMaxIter->setMinimum(0);
    txtMaxIter->setMaximum(10000);

    txtMaxEval = new QSpinBox(this);
    txtMaxEval->setMinimum(0);
    txtMaxEval->setMaximum(10000);

    cmbAlgorithm = new QComboBox(this);
    foreach (QString key, study()->algorithmStringKeys())
        cmbAlgorithm->addItem(study()->algorithmString(study()->algorithmFromStringKey(key)), key);

    cmbSurrogate = new QComboBox(this);
    foreach (QString key, study()->surrogateStringKeys())
        cmbSurrogate->addItem(study()->surrogateString(key), key);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Algorithm:")), 0, 0);
    layoutInitialization->addWidget(cmbAlgorithm, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Surrogate function:")), 1, 0);
    layoutInitialization->addWidget(cmbSurrogate, 1, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of iterations:")), 2, 0);
    layoutInitialization->addWidget(txtMaxIter, 2, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of evaluations:")), 3, 0);
    layoutInitialization->addWidget(txtMaxEval, 3, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout();
    layoutConfig->addWidget(new QLabel(tr("Absolute tolerance")), 0, 0, 1, 2);
    layoutConfig->addWidget(new QLabel(QString("|&Delta;<i>f</i>|:")), 1, 0);
    layoutConfig->addWidget(txtFTarget, 1, 1);

    QGroupBox *grpConfig = new QGroupBox(tr("Config"), this);
    grpConfig->setLayout(layoutConfig);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpConfig);

    return layoutMain;
}

void StudyCMAESDialog::load()
{
    StudyDialog::load();

    txtFTarget->setValue(study()->value(Study::CMAES_ftarget).toDouble());
    txtMaxIter->setValue(study()->value(Study::CMAES_maxiter).toInt());
    txtMaxEval->setValue(study()->value(Study::CMAES_maxeval).toInt());
    cmbAlgorithm->setCurrentIndex(cmbAlgorithm->findData(study()->value(Study::CMAES_algorithm).toString()));
    cmbSurrogate->setCurrentIndex(cmbSurrogate->findData(study()->value(Study::CMAES_surrogate).toString()));
}

void StudyCMAESDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::CMAES_ftarget, txtFTarget->value());
    study()->setValue(Study::CMAES_maxiter, txtMaxIter->value());
    study()->setValue(Study::CMAES_maxeval, txtMaxEval->value());
    study()->setValue(Study::CMAES_algorithm, cmbAlgorithm->itemData(cmbAlgorithm->currentIndex()).toString());
    study()->setValue(Study::CMAES_surrogate, cmbSurrogate->itemData(cmbSurrogate->currentIndex()).toString());
}

// *****************************************************************************************************

StudyBayesOptDialog::StudyBayesOptDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{
}

QLayout *StudyBayesOptDialog::createStudyControls()
{
    txtNInitSamples = new QSpinBox(this);
    txtNInitSamples->setMinimum(1);
    txtNInitSamples->setMaximum(1000);

    txtNIterations = new QSpinBox(this);
    txtNIterations->setMinimum(0);
    txtNIterations->setMaximum(10000);

    txtNIterRelearn = new QSpinBox(this);
    txtNIterRelearn->setMinimum(1);
    txtNIterRelearn->setMaximum(10000);

    cmbInitMethod = new QComboBox(this);
    foreach (QString key, study()->initMethodStringKeys())
        cmbInitMethod->addItem(study()->initMethodString(study()->initMethodFromStringKey(key)), key);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Number of initial samples:")), 0, 0);
    layoutInitialization->addWidget(txtNInitSamples, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Initial strategy:")), 1, 0);
    layoutInitialization->addWidget(cmbInitMethod, 1, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of iterations:")), 2, 0);
    layoutInitialization->addWidget(txtNIterations, 2, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of iterations between re-learning:")), 3, 0);
    layoutInitialization->addWidget(txtNIterRelearn, 3, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    cmbSurrogateNameMethod = new QComboBox(this);
    foreach (QString key, study()->surrogateStringKeys())
        cmbSurrogateNameMethod->addItem(study()->surrogateString(study()->surrogateFromStringKey(key)), key);

    txtSurrogateNoise = new LineEditDouble(0.0);
    txtSurrogateNoise->setBottom(0.0);

    QGridLayout *layoutSurrogate = new QGridLayout();
    layoutSurrogate->addWidget(new QLabel(tr("Function:")), 0, 0);
    layoutSurrogate->addWidget(cmbSurrogateNameMethod, 0, 1);
    layoutSurrogate->addWidget(new QLabel(tr("Noise:")), 1, 0);
    layoutSurrogate->addWidget(txtSurrogateNoise, 1, 1);

    QGroupBox *grpSurrogate = new QGroupBox(tr("Surrogate model parameters"), this);
    grpSurrogate->setLayout(layoutSurrogate);

    cmbHPLearningMethod = new QComboBox(this);
    foreach (QString key, study()->learningTypeStringKeys())
        cmbHPLearningMethod->addItem(study()->learningTypeString(study()->learningTypeFromStringKey(key)), key);

    cmbHPScoreFunction = new QComboBox(this);
    foreach (QString key, study()->scoreTypeStringKeys())
        cmbHPScoreFunction->addItem(study()->scoreTypeString(study()->scoreTypeFromStringKey(key)), key);

    QGridLayout *layoutKernelParameters = new QGridLayout();
    layoutKernelParameters->addWidget(new QLabel(tr("Learning method:")), 0, 0);
    layoutKernelParameters->addWidget(cmbHPLearningMethod, 0, 1);
    layoutKernelParameters->addWidget(new QLabel(tr("Score function:")), 1, 0);
    layoutKernelParameters->addWidget(cmbHPScoreFunction, 1, 1);

    QGroupBox *grpKernelParameters = new QGroupBox(tr("Kernel parameters"), this);
    grpKernelParameters->setLayout(layoutKernelParameters);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpSurrogate);
    layoutMain->addWidget(grpKernelParameters);

    return layoutMain;
}

void StudyBayesOptDialog::load()
{
    StudyDialog::load();

    txtNInitSamples->setValue(study()->value(Study::BayesOpt_n_init_samples).toInt());
    txtNIterations->setValue(study()->value(Study::BayesOpt_n_iterations).toInt());
    txtNIterRelearn->setValue(study()->value(Study::BayesOpt_n_iter_relearn).toInt());
    cmbInitMethod->setCurrentIndex(cmbInitMethod->findData(study()->value(Study::BayesOpt_init_method).toString()));
    cmbSurrogateNameMethod->setCurrentIndex(cmbSurrogateNameMethod->findData(study()->value(Study::BayesOpt_surr_name).toString()));
    txtSurrogateNoise->setValue(study()->value(Study::BayesOpt_surr_noise).toDouble());
    cmbHPLearningMethod->setCurrentIndex(cmbHPLearningMethod->findData(study()->value(Study::BayesOpt_l_type).toString()));
    cmbHPScoreFunction->setCurrentIndex(cmbHPScoreFunction->findData(study()->value(Study::BayesOpt_sc_type).toString()));
}

void StudyBayesOptDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::BayesOpt_n_init_samples, txtNInitSamples->value());
    study()->setValue(Study::BayesOpt_n_iterations, txtNIterations->value());
    study()->setValue(Study::BayesOpt_n_iter_relearn, txtNIterRelearn->value());
    study()->setValue(Study::BayesOpt_init_method, cmbInitMethod->itemData(cmbInitMethod->currentIndex()).toString());
    study()->setValue(Study::BayesOpt_surr_name, cmbSurrogateNameMethod->itemData(cmbSurrogateNameMethod->currentIndex()).toString());
    study()->setValue(Study::BayesOpt_surr_noise, txtSurrogateNoise->value());
    study()->setValue(Study::BayesOpt_l_type, cmbHPLearningMethod->itemData(cmbHPLearningMethod->currentIndex()).toString());
    study()->setValue(Study::BayesOpt_sc_type, cmbHPScoreFunction->itemData(cmbHPScoreFunction->currentIndex()).toString());
}

// *****************************************************************************************************

StudyLimboDialog::StudyLimboDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyLimboDialog::createStudyControls()
{
    txtNInitSamples = new QSpinBox(this);
    txtNInitSamples->setMinimum(1);
    txtNInitSamples->setMaximum(1000);

    txtNIterations = new QSpinBox(this);
    txtNIterations->setMinimum(0);
    txtNIterations->setMaximum(10000);

    txtHPIterRelearn = new QSpinBox(this);
    txtHPIterRelearn->setMinimum(1);
    txtHPIterRelearn->setMaximum(10000);

    txtHPNoise = new LineEditDouble(0.0);
    txtHPNoise->setBottom(0.0);

    cmbMean = new QComboBox(this);
    foreach (QString key, study()->meanStringKeys())
        cmbMean->addItem(study()->meanString(study()->meanFromStringKey(key)), key);
    connect(cmbMean, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    cmbGP = new QComboBox(this);
    foreach (QString key, study()->gpStringKeys())
        cmbGP->addItem(study()->gpString(study()->gpFromStringKey(key)), key);
    connect(cmbGP, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    cmbAcqui = new QComboBox(this);
    foreach (QString key, study()->acquiStringKeys())
        cmbAcqui->addItem(study()->acquiString(study()->acquiFromStringKey(key)), key);
    connect(cmbAcqui, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    lblError = new QLabel("");
    QPalette palette = lblError->palette();
    palette.setColor(QPalette::WindowText, QColor(Qt::red));
    lblError->setPalette(palette);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Number of initial samples:")), 0, 0);
    layoutInitialization->addWidget(txtNInitSamples, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Number of iterations:")), 1, 0);
    layoutInitialization->addWidget(txtNIterations, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutOptimizer = new QGridLayout();
    layoutOptimizer->addWidget(new QLabel(tr("Number of iterations between re-learning:")), 0, 0);
    layoutOptimizer->addWidget(txtHPIterRelearn, 0, 1);
    layoutOptimizer->addWidget(new QLabel(tr("Noise:")), 1, 0);
    layoutOptimizer->addWidget(txtHPNoise, 1, 1);
    layoutOptimizer->addWidget(new QLabel(tr("Mean value:")), 2, 0);
    layoutOptimizer->addWidget(cmbMean, 2, 1);
    layoutOptimizer->addWidget(new QLabel(tr("Acquisition function:")), 3, 0);
    layoutOptimizer->addWidget(cmbAcqui, 3, 1);
    layoutOptimizer->addWidget(new QLabel(tr("Likelihood optimizer:")), 4, 0);
    layoutOptimizer->addWidget(cmbGP, 4, 1);
    layoutOptimizer->addWidget(new QLabel(tr("Kernel function:")), 5, 0);
    layoutOptimizer->addWidget(new QLabel(tr("Squared exponential covariance function")), 5, 1);

    QGroupBox *grpOptimizer = new QGroupBox(tr("Bayesian optimizer"), this);
    grpOptimizer->setLayout(layoutOptimizer);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpOptimizer);
    layoutMain->addStretch();
    layoutMain->addWidget(lblError);

    return layoutMain;
}

void StudyLimboDialog::currentIndexChanged(int index)
{
    lblError->clear();

    QString mean = cmbMean->itemData(cmbMean->currentIndex()).toString();
    QString gp = cmbGP->itemData(cmbGP->currentIndex()).toString();
    QString acqui = cmbAcqui->itemData(cmbAcqui->currentIndex()).toString();

    // unsupported combinations
    if (((mean == "data") && (gp == "mean_lf") && (acqui == "gpucb"))
            || ((mean == "data") && (gp == "mean_lf") && (acqui == "ei"))
            || ((mean == "data") && (gp == "kernel_mean_lf") && (acqui == "gpucb"))
            || ((mean == "data") && (gp == "kernel_mean_lf") && (acqui == "ei"))
            || ((mean == "constant") && (gp == "mean_lf") && (acqui == "gpucb"))
            || ((mean == "constant") && (gp == "mean_lf") && (acqui == "ei"))
            || ((mean == "constant") && (gp == "kernel_mean_lf") && (acqui == "gpucb"))
            || ((mean == "constant") && (gp == "kernel_mean_lf") && (acqui == "ei")))
    {
        lblError->setText(tr("Unsupported combination:<br/>   mean = %1,<br/>   gp = %2,<br/>   acqui = %3")
                          .arg(cmbMean->currentText())
                          .arg(cmbGP->currentText())
                          .arg(cmbAcqui->currentText()));
    }
}

void StudyLimboDialog::load()
{
    StudyDialog::load();

    txtNInitSamples->setValue(study()->value(Study::LIMBO_init_randomsampling_samples).toInt());
    txtNIterations->setValue(study()->value(Study::LIMBO_stop_maxiterations_iterations).toInt());
    txtHPIterRelearn->setValue(study()->value(Study::LIMBO_bayes_opt_boptimizer_hp_period).toInt());
    txtHPNoise->setValue(study()->value(Study::LIMBO_bayes_opt_boptimizer_noise).toDouble());
    cmbMean->setCurrentIndex(cmbMean->findData(study()->value(Study::LIMBO_mean).toString()));
    cmbAcqui->setCurrentIndex(cmbAcqui->findData(study()->value(Study::LIMBO_acqui).toString()));
    cmbGP->setCurrentIndex(cmbGP->findData(study()->value(Study::LIMBO_gp).toString()));
}

void StudyLimboDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::LIMBO_init_randomsampling_samples, txtNInitSamples->value());
    study()->setValue(Study::LIMBO_stop_maxiterations_iterations, txtNIterations->value());
    study()->setValue(Study::LIMBO_bayes_opt_boptimizer_hp_period, txtHPIterRelearn->value());
    study()->setValue(Study::LIMBO_bayes_opt_boptimizer_noise, txtHPNoise->value());
    study()->setValue(Study::LIMBO_mean, cmbMean->itemData(cmbMean->currentIndex()).toString());
    study()->setValue(Study::LIMBO_acqui, cmbAcqui->itemData(cmbAcqui->currentIndex()).toString());
    study()->setValue(Study::LIMBO_gp, cmbGP->itemData(cmbGP->currentIndex()).toString());
}

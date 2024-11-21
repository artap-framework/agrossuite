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

StudyPagmoDialog::StudyPagmoDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyPagmoDialog::createStudyControls()
{
    cmbAlgorithm = new QComboBox(this);
    foreach (QString key, study()->algorithmStringKeys())
        cmbAlgorithm->addItem(study()->algorithmString(key), key);

    txtPopSize = new QSpinBox(this);
    txtPopSize->setMinimum(4);
    txtPopSize->setMaximum(4*1000);
    txtPopSize->setSingleStep(4);

    txtNGen = new QSpinBox(this);
    txtNGen->setMinimum(1);
    txtNGen->setMaximum(1000);

    txtEliteCount = new QSpinBox(this);
    txtEliteCount->setMinimum(2);
    txtEliteCount->setMaximum(1000);

    txtCrossoverFraction = new LineEditDouble(0.0);
    txtCrossoverFraction->setBottom(0);
    txtCrossoverFraction->setTop(1.0);
    txtMutationRate = new LineEditDouble(0.0);
    txtMutationRate->setBottom(0);
    txtMutationRate->setTop(1.0);

    QGridLayout *layoutInitialization = new QGridLayout();
    layoutInitialization->addWidget(new QLabel(tr("Algorithm:")), 0, 0);
    layoutInitialization->addWidget(cmbAlgorithm, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Population size:")), 1, 0);
    layoutInitialization->addWidget(txtPopSize, 1, 1);
    layoutInitialization->addWidget(new QLabel(tr("Maximum number of generations:")), 2, 0);
    layoutInitialization->addWidget(txtNGen, 2, 1);
    layoutInitialization->addWidget(new QLabel(tr("Elite count:")), 3, 0);
    layoutInitialization->addWidget(txtEliteCount, 3, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout();
    layoutConfig->addWidget(new QLabel(tr("Probability of crossover (0.0 - 1.0):")), 0, 0);
    layoutConfig->addWidget(txtCrossoverFraction, 0, 1);
    layoutConfig->addWidget(new QLabel(tr("Mutation rate (0.0 - 1.0):")), 1, 0);
    layoutConfig->addWidget(txtMutationRate, 1, 1);

    QGroupBox *grpConfig = new QGroupBox(tr("Config"), this);
    grpConfig->setLayout(layoutConfig);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpConfig);

    return layoutMain;
}

void StudyPagmoDialog::load()
{
    StudyDialog::load();

    // cmbAlgorithm->setCurrentIndex(cmbAlgorithm->findData(study()->value(Study::Pagmo_algorithm).toString()));
    // txtPopSize->setValue(study()->value(Study::Pagmo_popsize).toInt());
    // txtNGen->setValue(study()->value(Study::Pagmo_ngen).toInt());
    // txtEliteCount->setValue(study()->value(Study::Pagmo_elite_count).toInt());
    // txtCrossoverFraction->setValue(study()->value(Study::Pagmo_crossover_fraction).toDouble());
    // txtMutationRate->setValue(study()->value(Study::Pagmo_mutation_rate).toDouble());
}

void StudyPagmoDialog::save()
{
    StudyDialog::save();

    // study()->setValue(Study::Pagmo_algorithm, cmbAlgorithm->currentData().toString());
    // study()->setValue(Study::Pagmo_popsize, txtPopSize->value());
    // study()->setValue(Study::Pagmo_ngen, txtNGen->value());
    // study()->setValue(Study::Pagmo_elite_count, txtEliteCount->value());
    // study()->setValue(Study::Pagmo_crossover_fraction, txtCrossoverFraction->value());
    // study()->setValue(Study::Pagmo_mutation_rate, txtMutationRate->value());
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

StudyModelDialog::StudyModelDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyModelDialog::createStudyControls()
{
    chkClearSolution->setEnabled(false);
    chkSolveProblem->setEnabled(false);

    txtName = new QLineEdit();
    txtDescription = new QPlainTextEdit();
    txtDescription->setMinimumWidth(300);
    txtDescription->setMinimumHeight(200);

    auto *layoutNames = new QGridLayout();
    layoutNames->addWidget(new QLabel(tr("Name:")), 0, 0);
    layoutNames->addWidget(txtName, 0, 1);
    layoutNames->addWidget(new QLabel(tr("Description:")), 1, 0);
    layoutNames->addWidget(txtDescription, 1, 1);
    layoutNames->setColumnStretch(1, 1);

    auto *groupNames = new QGroupBox(tr("Model"), this);
    groupNames->setLayout(layoutNames);

    QVBoxLayout *layoutMain = new QVBoxLayout();
    layoutMain->addWidget(groupNames);

    return layoutMain;
}

void StudyModelDialog::load()
{
    StudyDialog::load();

    chkClearSolution->setChecked(false);
    chkSolveProblem->setChecked(true);

    txtName->setText(m_study->value(Study::Model_Name).toString());
    txtDescription->setPlainText(m_study->value(Study::Model_Description).toString());
}

void StudyModelDialog::save()
{
    StudyDialog::save();

    m_study->setValue(Study::Model_Name, txtName->text());
    m_study->setValue(Study::Model_Description, txtDescription->toPlainText());
}

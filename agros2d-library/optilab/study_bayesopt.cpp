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

#include "study_bayesopt.h"
#include "bayesopt/include/prob_distribution.hpp"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "gui/lineeditdouble.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

const QString N_INIT_SAMPLES = "n_init_samples";
const QString N_ITERATIONS = "n_iterations";
const QString N_ITER_RELEARN = "n_iter_relearn";
const QString INIT_METHOD = "init_method";

BayesOptProblem::BayesOptProblem(StudyBayesOpt *study, bayesopt::Parameters par)
    : ContinuousModel(study->parameters().count(), par), m_study(study)
{
    vectord lowerBound(m_study->parameters().count());
    vectord upperBound(m_study->parameters().count());

    // set bounding box
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];

        lowerBound.insert_element(i, parameter.lowerBound());
        upperBound.insert_element(i, parameter.upperBound());
    }

    setBoundingBox(lowerBound, upperBound);
}

double BayesOptProblem::evaluateSample(const vectord& x)
{   
    // computation
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
    m_study->addComputation(computation);

    // set parameters
    vectord query(m_study->parameters().count());
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->setParameter(parameter.name(), x[i]);        

        query[i] = (x[i] - parameter.lowerBound()) / (parameter.upperBound() - parameter.lowerBound());
    }

    // evaluate step
    try
    {
        SolutionUncertainty solutionUncertainty;
        bayesopt::ProbabilityDistribution *pd = nullptr;
        if (m_study->computationSets().count() > 1)
        {
            pd = getPrediction(query);

            solutionUncertainty.uncertainty = -evaluateCriteria(query);
            solutionUncertainty.lowerBound = pd->getMean() - 2.0*pd->getStd();
            solutionUncertainty.upperBound = pd->getMean() + 2.0*pd->getStd();
        }

        m_study->evaluateStep(computation, solutionUncertainty);
        double value = m_study->evaluateSingleGoal(computation);

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        return value;
    }
    catch (AgrosException &e)
    {
        qDebug() << e.toString();

        return numeric_limits<double>::max();
    }
}

StudyBayesOpt::StudyBayesOpt() : Study()
{    
}

int StudyBayesOpt::estimatedNumberOfSteps() const
{
    return value(Study::BayesOpt_n_init_samples).toInt() + value(Study::BayesOpt_n_iterations).toInt();
}

void StudyBayesOpt::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    // parameters
    bayesopt::Parameters par = initialize_parameters_to_default();
    par.n_init_samples = value(BayesOpt_n_init_samples).toInt();
    par.n_iterations = value(BayesOpt_n_iterations).toInt();
    par.n_iter_relearn = value(BayesOpt_n_iter_relearn).toInt();
    par.init_method = value(BayesOpt_init_method).toInt();
    par.surr_name = value(BayesOpt_surr_name).toString().toStdString();
    par.l_type = (learning_type) value(BayesOpt_l_type).toInt();
    par.sc_type = (score_type) value(BayesOpt_sc_type).toInt();
    par.noise = value(Study::BayesOpt_surr_noise).toDouble();
    par.random_seed = 0;
    par.verbose_level = -1;

    BayesOptProblem bayesOptProblem(this, par);

    // init BayesOpt problem
    addComputationSet(tr("Initialization"));
    bayesOptProblem.initializeOptimization();

    // steps
    addComputationSet(tr("Steps"));
    for (int i = 0; i < bayesOptProblem.getParameters()->n_iterations; i++)
    {
        if (isAborted())
            break;

        // step
        bayesOptProblem.stepOptimization();
    }

    // sort computations
    // QString parameterName = m_functionals[0].name();
    // m_computationSets.last().sort(parameterName);

    // vectord result = bayesOptProblem.getFinalResult();

    m_isSolving = false;

    emit solved();
}

void StudyBayesOpt::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[BayesOpt_n_init_samples] = 5;
    m_settingDefault[BayesOpt_n_iterations] = 10;
    m_settingDefault[BayesOpt_n_iter_relearn] = 5;
    m_settingDefault[BayesOpt_init_method] = 1; // 1-LHS, 2-Sobol
    m_settingDefault[BayesOpt_surr_name] = "sGaussianProcessML";
    m_settingDefault[BayesOpt_surr_noise] = 1e-10;
    m_settingDefault[BayesOpt_l_type] = L_EMPIRICAL;
    m_settingDefault[BayesOpt_sc_type] = SC_MAP;
}

void StudyBayesOpt::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[BayesOpt_n_init_samples] = "BayesOpt_n_init_samples";
    m_settingKey[BayesOpt_n_iterations] = "BayesOpt_n_iterations";
    m_settingKey[BayesOpt_n_iter_relearn] = "BayesOpt_n_iter_relearn";
    m_settingKey[BayesOpt_init_method] = "BayesOpt_init_method";
    m_settingKey[BayesOpt_surr_name] = "BayesOpt_surr_name";
    m_settingKey[BayesOpt_surr_noise] = "BayesOpt_surr_noise";
    m_settingKey[BayesOpt_l_type] = "BayesOpt_l_type";
    m_settingKey[BayesOpt_sc_type] = "BayesOpt_sc_type";
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
    cmbInitMethod->addItem(tr("Latin Hypercube Sampling (LHS)"), 1);
    cmbInitMethod->addItem(tr("Sobol Sequences"), 2);
    // cmbInitMethod->addItem(tr("Uniform Sampling"), 3);

    QGridLayout *layoutInitialization = new QGridLayout(this);
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
    cmbSurrogateNameMethod->addItem(tr("Gaussian process (hyperparameters are known)"), "sGaussianProcess");
    cmbSurrogateNameMethod->addItem(tr("Gaussian process (hyperparameters are estimated using maximum likelihood estimates)"), "sGaussianProcessML");
    cmbSurrogateNameMethod->addItem(tr("Gaussian process with a Normal prior on the mean function parameters"), "sGaussianProcessNormal");
    cmbSurrogateNameMethod->addItem(tr("Student's t process with a Jeffreys prior"), "sStudentTProcessJef");
    cmbSurrogateNameMethod->addItem(tr("Student's t process with a Normal prior on the mean function parameters)"), "sStudentTProcessNIG");

    txtSurrogateNoise = new LineEditDouble(0.0);
    txtSurrogateNoise->setBottom(0.0);

    QGridLayout *layoutSurrogate = new QGridLayout(this);
    layoutSurrogate->addWidget(new QLabel(tr("Function:")), 0, 0);
    layoutSurrogate->addWidget(cmbSurrogateNameMethod, 0, 1);
    layoutSurrogate->addWidget(new QLabel(tr("Noise:")), 1, 0);
    layoutSurrogate->addWidget(txtSurrogateNoise, 1, 1);

    QGroupBox *grpSurrogate = new QGroupBox(tr("Surrogate model parameters"), this);
    grpSurrogate->setLayout(layoutSurrogate);

    cmbHPLearningMethod = new QComboBox(this);
    cmbHPLearningMethod->addItem(tr("Fixed"), L_FIXED);
    cmbHPLearningMethod->addItem(tr("Emperical"), L_EMPIRICAL);
    cmbHPLearningMethod->addItem(tr("MCMC"), L_MCMC);

    cmbHPScoreFunction = new QComboBox(this);
    cmbHPScoreFunction->addItem(tr("Leave one out cross-validation"), SC_LOOCV);
    cmbHPScoreFunction->addItem(tr("Maximum total likelihood"), SC_MTL);
    cmbHPScoreFunction->addItem(tr("Posterior maximum likelihood "), SC_ML);
    cmbHPScoreFunction->addItem(tr("Maximum a posteriori"), SC_MAP);

    QGridLayout *layoutKernelParameters = new QGridLayout(this);
    layoutKernelParameters->addWidget(new QLabel(tr("Learning method:")), 0, 0);
    layoutKernelParameters->addWidget(cmbHPLearningMethod, 0, 1);
    layoutKernelParameters->addWidget(new QLabel(tr("Score function:")), 1, 0);
    layoutKernelParameters->addWidget(cmbHPScoreFunction, 1, 1);

    QGroupBox *grpKernelParameters = new QGroupBox(tr("Kernel parameters"), this);
    grpKernelParameters->setLayout(layoutKernelParameters);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
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
    cmbInitMethod->setCurrentIndex(cmbInitMethod->findData(study()->value(Study::BayesOpt_init_method).toInt()));
    cmbSurrogateNameMethod->setCurrentIndex(cmbSurrogateNameMethod->findData(study()->value(Study::BayesOpt_surr_name).toString()));
    txtSurrogateNoise->setValue(study()->value(Study::BayesOpt_surr_noise).toDouble());
    cmbHPLearningMethod->setCurrentIndex(cmbHPLearningMethod->findData(study()->value(Study::BayesOpt_l_type).toInt()));
    cmbHPScoreFunction->setCurrentIndex(cmbHPScoreFunction->findData(study()->value(Study::BayesOpt_sc_type).toInt()));
}

void StudyBayesOptDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::BayesOpt_n_init_samples, txtNInitSamples->value());
    study()->setValue(Study::BayesOpt_n_iterations, txtNIterations->value());
    study()->setValue(Study::BayesOpt_n_iter_relearn, txtNIterRelearn->value());
    study()->setValue(Study::BayesOpt_init_method, cmbInitMethod->itemData(cmbInitMethod->currentIndex()).toInt());
    study()->setValue(Study::BayesOpt_surr_name, cmbSurrogateNameMethod->itemData(cmbSurrogateNameMethod->currentIndex()).toString());
    study()->setValue(Study::BayesOpt_surr_noise, txtSurrogateNoise->value());
    study()->setValue(Study::BayesOpt_l_type, (learning_type) cmbHPLearningMethod->itemData(cmbHPLearningMethod->currentIndex()).toInt());
    study()->setValue(Study::BayesOpt_sc_type, (score_type) cmbHPScoreFunction->itemData(cmbHPScoreFunction->currentIndex()).toInt());
}

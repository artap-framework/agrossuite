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

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

const QString N_INIT_SAMPLES = "n_init_samples";
const QString N_ITERATIONS = "n_iterations";
const QString N_ITER_RELEARN = "n_iter_relearn";
const QString INIT_METHOD = "init_method";

BayesOptProblem::BayesOptProblem(StudyBayesOptAnalysis *study, bayesopt::Parameters par)
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
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->setParameter(parameter.name(), x[i]);
    }

    // solve
    computation->solve();

    // TODO: better error handling
    if (!computation->isSolved())
    {
        return numeric_limits<double>::max();
    }

    // evaluate functionals
    m_study->evaluateFunctionals(computation);

    // TODO: more functionals !!!
    assert(m_study->functionals().size() == 1);
    QString parameterName = m_study->functionals()[0].name();

    double value = computation->results()->resultValue(parameterName);
    computation->saveResults();

    m_study->updateParameters(m_study->parameters(), computation.data());
    m_study->updateChart();

    return value;
}

StudyBayesOptAnalysis::StudyBayesOptAnalysis() : Study()
{    
}

void StudyBayesOptAnalysis::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    // parameters
    bayesopt::Parameters par = initialize_parameters_to_default();
    par.n_init_samples = value(BayesOpt_n_init_samples).toInt();
    par.n_iterations = value(BayesOpt_n_iterations).toInt();
    par.n_iter_relearn = value(BayesOpt_n_iter_relearn).toInt();
    par.init_method = value(BayesOpt_init_method).toInt();
    par.noise = 1e-10;
    par.random_seed = 0;
    par.verbose_level = 1;

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
    QString parameterName = m_functionals[0].name();
    m_computationSets.last().sort(parameterName);

    // vectord result = bayesOptProblem.getFinalResult();

    m_isSolving = false;

    emit solved();
}

void StudyBayesOptAnalysis::setDefaultValues()
{
    m_settingDefault.clear();

    m_settingDefault[BayesOpt_n_init_samples] = 5;
    m_settingDefault[BayesOpt_n_iterations] = 10;
    m_settingDefault[BayesOpt_n_iter_relearn] = 5;
    m_settingDefault[BayesOpt_init_method] = 1; // 1-LHS, 2-Sobol
}

void StudyBayesOptAnalysis::setStringKeys()
{
    m_settingKey[BayesOpt_n_init_samples] = "BayesOpt_n_init_samples";
    m_settingKey[BayesOpt_n_iterations] = "BayesOpt_n_iterations";
    m_settingKey[BayesOpt_n_iter_relearn] = "BayesOpt_n_iter_relearn";
    m_settingKey[BayesOpt_init_method] = "BayesOpt_init_method";
}

// *****************************************************************************************************

StudyBayesOptAnalysisDialog::StudyBayesOptAnalysisDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QWidget *StudyBayesOptAnalysisDialog::createStudyControls()
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
    cmbInitMethod->addItem(tr("Uniform Sampling"), 3);

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

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->addWidget(grpInitialization);

    QWidget *widget = new QWidget(this);
    widget->setLayout(layoutMain);

    return widget;
}

void StudyBayesOptAnalysisDialog::load()
{
    txtNInitSamples->setValue(study()->value(Study::BayesOpt_n_init_samples).toInt());
    txtNIterations->setValue(study()->value(Study::BayesOpt_n_iterations).toInt());
    txtNIterRelearn->setValue(study()->value(Study::BayesOpt_n_iter_relearn).toInt());
    cmbInitMethod->setCurrentIndex(cmbInitMethod->findData(study()->value(Study::BayesOpt_init_method).toInt()));
}

void StudyBayesOptAnalysisDialog::save()
{
    study()->setValue(Study::BayesOpt_n_init_samples, txtNInitSamples->value());
    study()->setValue(Study::BayesOpt_n_iterations, txtNIterations->value());
    study()->setValue(Study::BayesOpt_n_iter_relearn, txtNIterRelearn->value());
    study()->setValue(Study::BayesOpt_init_method, cmbInitMethod->itemData(cmbInitMethod->currentIndex()).toInt());
}

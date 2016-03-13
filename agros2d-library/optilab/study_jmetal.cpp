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

#include "study_jmetal.h"

#include "study.h"
#include "parameter.h"

#include "gui/lineeditdouble.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include "agros_jmetal.h"

class JMetalProblemSolver : public JMetalSolver
{
public:
    JMetalProblemSolver(Study *study) : JMetalSolver(), m_study(study)
    {
        numberOfVariables = m_study->parameters().count();
        numberOfObjectives = m_study->functionals().count();

        lowerLimit = new double[m_study->parameters().count()];
        upperLimit = new double[m_study->parameters().count()];

        // set bounding box
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];

            lowerLimit[i] = parameter.lowerBound();
            upperLimit[i] = parameter.upperBound();
        }

        // m_study->setValue(Study::JMetal_mutationProbability, 1.0 / m_study->parameters().count());
    }

    virtual void evaluate(std::vector<double> &x, std::vector<double> &of)
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

        // evaluate step
        try
        {
            m_study->evaluateStep(computation);
            QList<double> values = m_study->evaluateMultiGoal(computation);

            if (m_study->value(Study::General_ClearSolution).toBool())
                computation->clearSolution();

            for (int i = 0; i < values.count(); i++)
                of[i] = values[i];
        }
        catch (AgrosException &e)
        {
            qDebug() << e.toString();
        }
    }

    // parameters
    virtual JMetalSolver::Algorithm algorithm() { return (JMetalSolver::Algorithm) m_study->value(Study::JMetal_algorithm).toInt(); }
    virtual int populationSize() { return m_study->value(Study::JMetal_populationSize).toInt(); }
    virtual int archiveSize() { return m_study->value(Study::JMetal_archiveSize).toInt(); }
    virtual int maxEvaluations() { return m_study->value(Study::JMetal_maxEvaluations).toInt(); }

    virtual double crossoverProbability() { return m_study->value(Study::JMetal_crossoverProbability).toDouble(); }
    virtual double crossoverDistributionIndex() { return m_study->value(Study::JMetal_crossoverDistributionIndex).toDouble(); }
    virtual double crossoverWeightParameter() { return m_study->value(Study::JMetal_crossoverWeightParameter).toDouble(); }

    virtual double mutationProbability() { return m_study->value(Study::JMetal_mutationProbability).toDouble(); }
    virtual double mutationPerturbation() { return m_study->value(Study::JMetal_mutationPerturbation).toDouble(); }
    virtual double mutationDistributionIndex() { return m_study->value(Study::JMetal_mutationDistributionIndex).toDouble(); }

    Study *m_study;
};

StudyJMetal::StudyJMetal() : Study()
{  
}

int StudyJMetal::estimatedNumberOfSteps() const
{
    JMetalSolver::Algorithm algorithm = (JMetalSolver::Algorithm) value(Study::JMetal_algorithm).toInt();

    if (algorithm == JMetalSolver::OMOPSO)
        return value(JMetal_maxEvaluations).toInt() * (value(JMetal_populationSize).toInt() + 1);
    else if (algorithm == JMetalSolver::SMPSO || algorithm == JMetalSolver::SMPSOhv)
        return value(JMetal_maxEvaluations).toInt() * (value(JMetal_populationSize).toInt() + 1);
    else if (algorithm == JMetalSolver::NSGAII || algorithm == JMetalSolver::ssNSGAII)
        return value(JMetal_maxEvaluations).toInt();
    else if (algorithm == JMetalSolver::GDE3)
        return value(JMetal_maxEvaluations).toInt() * (value(JMetal_populationSize).toInt() + 1);
    else if (algorithm == JMetalSolver::paes)
        return value(JMetal_maxEvaluations).toInt();
    else if (algorithm == JMetalSolver::SMSEMOA || algorithm == JMetalSolver::FastSMSEMOA)
        return value(JMetal_maxEvaluations).toInt();
    else
        assert(0);
}

void StudyJMetal::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    addComputationSet(tr("Steps"));

    JMetalProblemSolver problem(this);
    problem.run();

    m_isSolving = false;

    // sort computations
    // QString parameterName = m_functionals[0].name();
    // m_computationSets.last().sort(parameterName);

    emit solved();
}

void StudyJMetal::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[JMetal_algorithm] = JMetalSolver::SMPSOhv;
    m_settingDefault[JMetal_populationSize] = 10;
    m_settingDefault[JMetal_archiveSize] = 10;
    m_settingDefault[JMetal_maxEvaluations] = 30;

    m_settingDefault[JMetal_crossoverProbability] = 0.9;
    m_settingDefault[JMetal_crossoverDistributionIndex] = 20.0;
    m_settingDefault[JMetal_crossoverWeightParameter] = 0.5;

    m_settingDefault[JMetal_mutationProbability] = 1.0;
    m_settingDefault[JMetal_mutationPerturbation] = 0.5;
    m_settingDefault[JMetal_mutationDistributionIndex] = 20.0;
}

void StudyJMetal::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[JMetal_algorithm] = "JMetal_algorithm";
    m_settingKey[JMetal_populationSize] = "JMetal_populationSize";
    m_settingKey[JMetal_archiveSize] = "JMetal_archiveSize";
    m_settingKey[JMetal_maxEvaluations] = "JMetal_maxEvaluations";

    m_settingKey[JMetal_crossoverProbability] = "JMetal_crossoverProbability";
    m_settingKey[JMetal_crossoverDistributionIndex] = "JMetal_crossoverDistributionIndex";
    m_settingKey[JMetal_crossoverWeightParameter] = "JMetal_crossoverWeightParameter";

    m_settingKey[JMetal_mutationProbability] = "JMetal_mutationProbability";
    m_settingKey[JMetal_mutationPerturbation] = "JMetal_mutationPerturbation";
    m_settingKey[JMetal_mutationDistributionIndex] = "JMetal_mutationDistributionIndex";
}

// *****************************************************************************************************

StudyJMetalDialog::StudyJMetalDialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

QLayout *StudyJMetalDialog::createStudyControls()
{    
    lblAlgorithmCrossover = new QLabel("-");
    lblAlgorithmMutation = new QLabel("-");
    lblAlgorithmSelection = new QLabel("-");

    cmbAlgorithm = new QComboBox(this);
    cmbAlgorithm->addItem(tr("OMOPSO (particle swarm optimization)"), JMetalSolver::OMOPSO);
    cmbAlgorithm->addItem(tr("SMPSO (particle swarm optimization)"), JMetalSolver::SMPSO);
    cmbAlgorithm->addItem(tr("SMPSOhv (particle swarm optimization (indicator based))"), JMetalSolver::SMPSOhv);
    cmbAlgorithm->addItem(tr("NSGAII (Genetic algorithm)"), JMetalSolver::NSGAII);
    cmbAlgorithm->addItem(tr("ssNSGAII (Genetic algorithm (steady state))"), JMetalSolver::ssNSGAII);
    cmbAlgorithm->addItem(tr("GDE3 (differential evolution)"), JMetalSolver::GDE3);
    cmbAlgorithm->addItem(tr("paes (evolution strategy)"), JMetalSolver::paes);
    cmbAlgorithm->addItem(tr("SMSEMOA (multiobjective selection based on dominated hypervolume)"), JMetalSolver::SMSEMOA);
    cmbAlgorithm->addItem(tr("FastSMSEMOA (fast multiobjective selection based on dominated hypervolume)"), JMetalSolver::FastSMSEMOA);
    connect(cmbAlgorithm, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

    txtPopulationSize = new QSpinBox(this);
    txtPopulationSize->setMinimum(1);
    txtPopulationSize->setMaximum(10000);

    txtArchiveSize = new QSpinBox(this);
    txtArchiveSize->setMinimum(1);
    txtArchiveSize->setMaximum(10000);

    txtMaxEvaluations = new QSpinBox(this);
    txtMaxEvaluations->setMinimum(1);
    txtMaxEvaluations->setMaximum(10000);

    txtCrossoverProbability = new LineEditDouble(0.0);
    txtCrossoverProbability->setBottom(0.0);
    txtCrossoverProbability->setTop(1.0);
    txtCrossoverDistributionIndex = new LineEditDouble(0.0);
    txtCrossoverDistributionIndex->setBottom(5.0);
    txtCrossoverDistributionIndex->setTop(20.0);
    txtCrossoverWeightParameter = new LineEditDouble(0.0);
    txtCrossoverWeightParameter->setBottom(0.0);
    txtCrossoverWeightParameter->setTop(2.0);

    txtMutationProbability = new LineEditDouble(0.0); // 1.0 / m_study->parameters().count()
    txtMutationProbability->setBottom(0.0);
    txtMutationProbability->setTop(1.0);
    txtMutationPerturbation = new LineEditDouble(0.0);
    txtMutationPerturbation->setBottom(0.0);
    txtMutationPerturbation->setTop(1.0);
    txtMutationDistributionIndex = new LineEditDouble(0.0);
    txtMutationDistributionIndex->setBottom(5.0);
    txtMutationDistributionIndex->setTop(20.0);

    QGridLayout *layoutInitialization = new QGridLayout(this);
    layoutInitialization->addWidget(new QLabel(tr("Algorithm:")), 0, 0);
    layoutInitialization->addWidget(cmbAlgorithm, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Crossover method:")), 1, 0);
    layoutInitialization->addWidget(lblAlgorithmCrossover, 1, 1);
    layoutInitialization->addWidget(new QLabel(tr("Mutation method:")), 2, 0);
    layoutInitialization->addWidget(lblAlgorithmMutation, 2, 1);
    layoutInitialization->addWidget(new QLabel(tr("Selection method:")), 3, 0);
    layoutInitialization->addWidget(lblAlgorithmSelection, 3, 1);
    layoutInitialization->addWidget(new QLabel(tr("Population / swarm size:")), 4, 0);
    layoutInitialization->addWidget(txtPopulationSize, 4, 1);
    layoutInitialization->addWidget(new QLabel(tr("Archive size:")), 5, 0);
    layoutInitialization->addWidget(txtArchiveSize, 5, 1);
    layoutInitialization->addWidget(new QLabel(tr("Maximum number of evalutions:")), 6, 0);
    layoutInitialization->addWidget(txtMaxEvaluations, 6, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutCrossover = new QGridLayout(this);
    layoutCrossover->addWidget(new QLabel(tr("Probability of crossover (0.0 - 1.0):")), 0, 0);
    layoutCrossover->addWidget(txtCrossoverProbability, 0, 1);
    layoutCrossover->addWidget(new QLabel(tr("Distribution index for crossover (5 - 20):")), 1, 0);
    layoutCrossover->addWidget(txtCrossoverDistributionIndex, 1, 1);
    layoutCrossover->addWidget(new QLabel(tr("Weight coefficient - differential evolution (0 - 2.0):")), 3, 0);
    layoutCrossover->addWidget(txtCrossoverWeightParameter, 3, 1);

    QGroupBox *grpCrossover = new QGroupBox(tr("Crossover"), this);
    grpCrossover->setLayout(layoutCrossover);

    QGridLayout *layoutMutation = new QGridLayout(this);
    layoutMutation->addWidget(new QLabel(tr("Probability of mutation (0.0 - 1.0, 1/num. of params.):")), 0, 0);
    layoutMutation->addWidget(txtMutationProbability, 0, 1);
    layoutMutation->addWidget(new QLabel(tr("Perturbation (0.0 - 1.0):")), 1, 0);
    layoutMutation->addWidget(txtMutationPerturbation, 1, 1);
    layoutMutation->addWidget(new QLabel(tr("Distribution index for mutation (5 - 20):")), 2, 0);
    layoutMutation->addWidget(txtMutationDistributionIndex, 2, 1);

    QGroupBox *grpMutation = new QGroupBox(tr("Mutation"), this);
    grpMutation->setLayout(layoutMutation);

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->addWidget(grpInitialization);
    layoutMain->addWidget(grpCrossover);
    layoutMain->addWidget(grpMutation);

    currentIndexChanged(cmbAlgorithm->currentIndex());

    return layoutMain;
}

void StudyJMetalDialog::currentIndexChanged(int index)
{
    JMetalSolver::Algorithm algorithm = (JMetalSolver::Algorithm) cmbAlgorithm->itemData(cmbAlgorithm->currentIndex()).toInt();

    // TODO: more general
    if (algorithm == JMetalSolver::OMOPSO)
    {
        txtPopulationSize->setEnabled(true);
        txtArchiveSize->setEnabled(true);
        txtMaxEvaluations->setEnabled(true);

        txtCrossoverProbability->setEnabled(false);
        txtCrossoverDistributionIndex->setEnabled(false);
        txtCrossoverWeightParameter->setEnabled(false);

        txtMutationProbability->setEnabled(true);
        txtMutationPerturbation->setEnabled(true);
        txtMutationDistributionIndex->setEnabled(false);

        lblAlgorithmCrossover->setText(tr("-"));
        lblAlgorithmMutation->setText(tr("uniform and nonuniform mutation"));
        lblAlgorithmSelection->setText(tr("-"));
    }
    else if (algorithm == JMetalSolver::SMPSO || algorithm == JMetalSolver::SMPSOhv)
    {
        txtPopulationSize->setEnabled(true);
        txtArchiveSize->setEnabled(true);
        txtMaxEvaluations->setEnabled(true);

        txtCrossoverProbability->setEnabled(false);
        txtCrossoverDistributionIndex->setEnabled(false);
        txtCrossoverWeightParameter->setEnabled(false);

        txtMutationProbability->setEnabled(true);
        txtMutationPerturbation->setEnabled(false);
        txtMutationDistributionIndex->setEnabled(true);

        lblAlgorithmCrossover->setText(tr("-"));
        lblAlgorithmMutation->setText(tr("polynomial mutation"));
        lblAlgorithmSelection->setText(tr("-"));
    }
    else if (algorithm == JMetalSolver::NSGAII || algorithm == JMetalSolver::ssNSGAII)
    {
        txtPopulationSize->setEnabled(true);
        txtArchiveSize->setEnabled(false);
        txtMaxEvaluations->setEnabled(true);

        txtCrossoverProbability->setEnabled(true);
        txtCrossoverDistributionIndex->setEnabled(true);
        txtCrossoverWeightParameter->setEnabled(false);

        txtMutationProbability->setEnabled(true);
        txtMutationPerturbation->setEnabled(false);
        txtMutationDistributionIndex->setEnabled(true);

        lblAlgorithmCrossover->setText(tr("SBX crossover"));
        lblAlgorithmMutation->setText(tr("polynomial mutation"));
        lblAlgorithmSelection->setText(tr("binary tournament"));
    }
    else if (algorithm == JMetalSolver::GDE3)
    {
        txtPopulationSize->setEnabled(true);
        txtArchiveSize->setEnabled(false);
        txtMaxEvaluations->setEnabled(true);

        txtCrossoverProbability->setEnabled(true);
        txtCrossoverDistributionIndex->setEnabled(false);
        txtCrossoverWeightParameter->setEnabled(true);

        txtMutationProbability->setEnabled(false);
        txtMutationPerturbation->setEnabled(false);
        txtMutationDistributionIndex->setEnabled(false);

        lblAlgorithmCrossover->setText(tr("differential evolution crossover"));
        lblAlgorithmMutation->setText(tr("-"));
        lblAlgorithmSelection->setText(tr("differential evolution selection"));
    }
    else if (algorithm == JMetalSolver::paes)
    {
        txtPopulationSize->setEnabled(true);
        txtArchiveSize->setEnabled(false);
        txtMaxEvaluations->setEnabled(true);

        txtCrossoverProbability->setEnabled(true);
        txtCrossoverDistributionIndex->setEnabled(true);
        txtCrossoverWeightParameter->setEnabled(false);

        txtMutationProbability->setEnabled(false);
        txtMutationPerturbation->setEnabled(false);
        txtMutationDistributionIndex->setEnabled(false);

        lblAlgorithmCrossover->setText(tr("-"));
        lblAlgorithmMutation->setText(tr("polynomial mutation"));
        lblAlgorithmSelection->setText(tr("-"));
    }
    else if (algorithm == JMetalSolver::SMSEMOA || algorithm == JMetalSolver::FastSMSEMOA)
    {
        txtPopulationSize->setEnabled(true);
        txtArchiveSize->setEnabled(false);
        txtMaxEvaluations->setEnabled(true);

        txtCrossoverProbability->setEnabled(true);
        txtCrossoverDistributionIndex->setEnabled(true);
        txtCrossoverWeightParameter->setEnabled(false);

        txtMutationProbability->setEnabled(true);
        txtMutationPerturbation->setEnabled(false);
        txtMutationDistributionIndex->setEnabled(true);

        lblAlgorithmCrossover->setText(tr("SBX crossover"));
        lblAlgorithmMutation->setText(tr("polynomial mutation"));
        lblAlgorithmSelection->setText(tr("random selection"));
    }
    else
        assert(0);
}

void StudyJMetalDialog::load()
{    
    StudyDialog::load();

    cmbAlgorithm->setCurrentIndex(cmbAlgorithm->findData(study()->value(Study::JMetal_algorithm).toInt()));
    txtPopulationSize->setValue(study()->value(Study::JMetal_populationSize).toInt());
    txtArchiveSize->setValue(study()->value(Study::JMetal_archiveSize).toInt());
    txtMaxEvaluations->setValue(study()->value(Study::JMetal_maxEvaluations).toInt());

    txtCrossoverProbability->setValue(study()->value(Study::JMetal_crossoverProbability).toDouble());
    txtCrossoverDistributionIndex->setValue(study()->value(Study::JMetal_crossoverDistributionIndex).toDouble());
    txtCrossoverWeightParameter->setValue(study()->value(Study::JMetal_crossoverWeightParameter).toDouble());

    txtMutationProbability->setValue(study()->value(Study::JMetal_mutationProbability).toDouble());
    txtMutationPerturbation->setValue(study()->value(Study::JMetal_mutationPerturbation).toDouble());
    txtMutationDistributionIndex->setValue(study()->value(Study::JMetal_mutationDistributionIndex).toDouble());
}

void StudyJMetalDialog::save()
{
    StudyDialog::save();

    study()->setValue(Study::JMetal_algorithm, (JMetalSolver::Algorithm) cmbAlgorithm->itemData(cmbAlgorithm->currentIndex()).toInt());
    study()->setValue(Study::JMetal_populationSize, txtPopulationSize->value());
    study()->setValue(Study::JMetal_archiveSize, txtArchiveSize->value());
    study()->setValue(Study::JMetal_maxEvaluations, txtMaxEvaluations->value());

    study()->setValue(Study::JMetal_crossoverProbability, txtCrossoverProbability->value());
    study()->setValue(Study::JMetal_crossoverDistributionIndex, txtCrossoverDistributionIndex->value());
    study()->setValue(Study::JMetal_crossoverWeightParameter, txtCrossoverWeightParameter->value());

    study()->setValue(Study::JMetal_mutationProbability, txtMutationProbability->value());
    study()->setValue(Study::JMetal_mutationPerturbation, txtMutationPerturbation->value());
    study()->setValue(Study::JMetal_mutationDistributionIndex, txtMutationDistributionIndex->value());
}


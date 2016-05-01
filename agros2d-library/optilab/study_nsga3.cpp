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

#include "study_nsga3.h"

#include "study.h"
#include "parameter.h"

#include "gui/lineeditdouble.h"
#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include "alg_nsgaiii.h"
#include "alg_individual.h"
#include "alg_population.h"
#include "problem_base.h"

#include "../../3rdparty/Eigen/Core"
#include "../../3rdparty/limbo/model/gp.hpp"
#include "../../3rdparty/limbo/model/gp/kernel_mean_lf_opt.hpp"
#include "../../3rdparty/limbo/model/gp/kernel_lf_opt.hpp"
#include "../../3rdparty/limbo/model/gp/mean_lf_opt.hpp"
#include "../../3rdparty/limbo/model/gp/no_lf_opt.hpp"
#include "../../3rdparty/limbo/kernel/squared_exp_ard.hpp"
#include "../../3rdparty/limbo/mean/data.hpp"
#include "../../3rdparty/limbo/mean/constant.hpp"
#include "../../3rdparty/limbo/mean/function_ard.hpp"

struct Params {
    // struct opt_rprop : public limbo::defaults::opt_rprop {};
    struct opt_rprop { BO_PARAM(int, iterations, 100); }; // default 300
    // struct opt_parallelrepeater : public limbo::defaults::opt_parallelrepeater {};
    struct opt_parallelrepeater { BO_PARAM(int, repeats, 10); }; // default 10
};

class NSGA3Problem : public NSGA3ProblemBase
{
public:
    NSGA3Problem(StudyNSGA3 *study) : NSGA3ProblemBase("NSGA3"), m_study(study), relearn(30), countComputation(0), countSurrogate(0)
    {
        lbs_.resize(m_study->parameters().count());
        ubs_.resize(m_study->parameters().count());

        // set bounding box
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];

            lbs_[i] = parameter.lowerBound();
            ubs_[i] = parameter.upperBound();
        }
    }

    virtual std::size_t num_variables() const
    {
        return m_study->parameters().count();
    }

    virtual std::size_t num_objectives() const
    {
        return m_study->functionals().count();
    }

    virtual bool Evaluate(CIndividual *indv) const
    {
        CIndividual::TDecVec &x = indv->vars();
        CIndividual::TObjVec &f = indv->objs();

        if (m_study->isAborted())
            return false;

        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);
        // m_study->addComputation(computation);

        // training patterns
        Eigen::VectorXd samples(m_study->parameters().count());
        // Gaussian process parameters
        Eigen::VectorXd mu;
        double sigma;

        // set parameters
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            computation->config()->parameters()->set(parameter.name(), x[i]);
            computation->scene()->cacheGeometryConstraints();

            // training patterns
            samples[i] = x[i];
        }

        // evaluate step
        try
        {
            bool useSurrogate = m_study->value(Study::NSGA3_use_surrogate).toBool() && (gp.nb_samples() > relearn - 1);

            if (useSurrogate)
            {
                // estimate values
                std::tie(mu, sigma) = gp.query(samples);
                double lik = gp.get_lik();
                qDebug() << "sigma = " << sigma << ", lik = " << lik;

                // qDebug() << "surrogate function";
                // for (int i = 0; i < m_study->functionals().count(); i++)
                //     qDebug() << i << " : surr : mu = " << mu[i] << ", sigma = " << sigma << ", lik = " << gp.get_lik() << ", use = " << (sigma < 1e-4);
                // surrogate function
                if (sigma < 1e-4)
                {
                    // weight functionals
                    int totalWeight = 0;
                    foreach (Functional functional, m_study->functionals())
                        totalWeight += functional.weight();

                    QList<double> values;
                    double totalValue = 0.0;

                    // set objective functions
                    for (int i = 0; i < m_study->functionals().count(); i++)
                    {
                        f[i] = mu[i];
                        values.append(mu[i]);

                        totalValue += ((double) m_study->functionals()[i].weight() / totalWeight) * mu[i];
                    }

                    Agros2D::computations().remove(computation->problemDir());

                    countSurrogate++;

                    // update chart
                    emit m_study->updateChart(values, totalValue, SolutionUncertainty());
                }
                else
                {
                    // force computation
                    useSurrogate = false;
                }
            }

            // real computation and model improvement
            if (!useSurrogate)
            {
                // qDebug() << "real computation";
                m_study->evaluateStep(computation);
                QList<double> values = m_study->evaluateMultiGoal(computation);

                if (m_study->value(Study::General_ClearSolution).toBool())
                    computation->clearSolution();

                // penalty
                double totalPenalty = 0.0;
                for (int i = 0; i < m_study->parameters().count(); i++)
                {
                    Parameter parameter = m_study->parameters()[i];
                    if (parameter.penaltyEnabled())
                        totalPenalty += parameter.penalty(x[i]);
                }

                // set objective functions
                countComputation++;
                for (int i = 0; i < values.count(); i++)
                    f[i] = values[i] + totalPenalty;

                // add computation
                m_study->addComputation(computation);

                if (m_study->value(Study::NSGA3_use_surrogate).toBool())
                {
                    // estimate values
                    if (gp.nb_samples() > 1)
                    {
                        std::tie(mu, sigma) = gp.query(samples);
                         for (int i = 0; i < values.count(); i++)
                             qDebug() << i << " : value = " << values[i] << ", mu = " << mu[i] << ", sigma = " << sigma;
                    }

                    // add samples and observations
                    Eigen::VectorXd observations(m_study->functionals().count());
                    for (int i = 0; i < values.count(); i++)
                        observations[i] = values[i];

                    m_samples.push_back(samples);
                    m_observations.push_back(observations);

                    // refresh model
                    if (m_observations.size() % relearn == 0)
                        gp.compute(m_samples, m_observations, 1e-10);                                         
                }
            }

            // add set
            int popCount = m_study->computationsCount();
            if (popCount > 1 && popCount < m_study->estimatedNumberOfSteps())
            {
                if (popCount % m_study->currentPopulationSize() == 0)
                    m_study->addComputationSet(QObject::tr("Step %1").arg(popCount / m_study->currentPopulationSize()));
            }

            return true;
        }
        catch (AgrosSolverException &e)
        {
            qDebug() << e.toString();

            // opt.set_force_stop(2);
        }

        return false;
    }

    mutable int countComputation;
    mutable int countSurrogate;
private:
    StudyNSGA3 *m_study;

    mutable limbo::model::GP<Params, limbo::kernel::SquaredExpARD<Params>, limbo::mean::Data<Params>, limbo::model::gp::KernelLFOpt<Params> > gp;
    // mutable limbo::model::GP<Params, limbo::kernel::SquaredExpARD<Params>, limbo::mean::Data<Params>, limbo::model::gp::NoLFOpt<Params> > gp;
    // mutable limbo::model::GP<Params, limbo::kernel::SquaredExpARD<Params>, limbo::mean::FunctionARD<Params, limbo::mean::Data<Params> >, limbo::model::gp::MeanLFOpt<Params> > gp;

    // parameters
    int relearn;
    mutable std::vector<Eigen::VectorXd> m_samples;
    mutable std::vector<Eigen::VectorXd> m_observations;


};

StudyNSGA3::StudyNSGA3() : Study()
{
}

int StudyNSGA3::estimatedNumberOfSteps() const
{
    return currentPopulationSize() * (value(NSGA3_ngen).toInt() + 1);
}

int StudyNSGA3::currentPopulationSize() const
{
    int estSize = (value(NSGA3_popsize).toInt() + m_functionals.count() - 1);
    while (estSize % 4) estSize++;

    return estSize;
}

void StudyNSGA3::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    addComputationSet(QObject::tr("Initilization"));

    // nsga3
    NSGA3Problem nsga3Problem(this);

    // solutions
    CPopulation solutions;

    NSGA3 nsga3(value(NSGA3_pcross).toDouble(),
                value(NSGA3_pmut).toDouble(),
                value(NSGA3_eta_c).toDouble(),
                value(NSGA3_eta_m).toDouble(),
                value(NSGA3_popsize).toInt(),
                value(NSGA3_ngen).toInt());
    nsga3.Solve(&solutions, nsga3Problem);

    qDebug() << "countComputation = " << nsga3Problem.countComputation << ", countSurrogate = " << nsga3Problem.countSurrogate << " (" <<
                100.0 * (double) nsga3Problem.countSurrogate / (nsga3Problem.countComputation + nsga3Problem.countSurrogate) << " %)";

    m_isSolving = false;

    // sort computations
    // QString parameterName = m_functionals[0].name();
    // m_computationSets.last().sort(parameterName);

    emit solved();
}

void StudyNSGA3::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[NSGA3_popsize] = 4;
    m_settingDefault[NSGA3_ngen] = 3;
    m_settingDefault[NSGA3_pcross] = 1.0;
    // m_settingDefault[NSGA3_pmut] = 0.2;
    m_settingDefault[NSGA3_eta_c] = 30.0;
    m_settingDefault[NSGA3_eta_m] = 20.0;
    m_settingDefault[NSGA3_use_surrogate] = false;
}

void StudyNSGA3::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[NSGA3_popsize] = "NSGA3_popsize";
    m_settingKey[NSGA3_ngen] = "NSGA3_ngen";
    m_settingKey[NSGA3_pcross] = "NSGA3_pcross";
    // m_settingKey[NSGA3_pmut] = "NSGA3_pmut";
    m_settingKey[NSGA3_eta_c] = "NSGA3_eta_c";
    m_settingKey[NSGA3_eta_m] = "NSGA3_eta_m";
    m_settingKey[NSGA3_use_surrogate] = "NSGA3_use_surrogate";
}

// *****************************************************************************************************

StudyNSGA3Dialog::StudyNSGA3Dialog(Study *study, QWidget *parent)
    : StudyDialog(study, parent)
{

}

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

    QGridLayout *layoutInitialization = new QGridLayout(this);
    layoutInitialization->addWidget(new QLabel(tr("Estimated population size:")), 0, 0);
    layoutInitialization->addWidget(txtPopSize, 0, 1);
    layoutInitialization->addWidget(new QLabel(tr("Maximum number of generations:")), 1, 0);
    layoutInitialization->addWidget(txtNGen, 1, 1);

    QGroupBox *grpInitialization = new QGroupBox(tr("Initialization"), this);
    grpInitialization->setLayout(layoutInitialization);

    QGridLayout *layoutConfig = new QGridLayout(this);
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

    QVBoxLayout *layoutMain = new QVBoxLayout(this);
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


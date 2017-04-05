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
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

BayesOptProblem::BayesOptProblem(StudyBayesOpt *study, bayesopt::Parameters par)
    : ContinuousModel(study->parameters().count(), par), m_study(study), m_steps(0)
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

double BayesOptProblem::evaluateSample(const vectord &x)
{   
    // computation
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

    // set parameters
    vectord query(m_study->parameters().count());
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->parameters()->set(parameter.name(), x[i]);

        query[i] = (x[i] - parameter.lowerBound()) / (parameter.upperBound() - parameter.lowerBound());
    }

    // evaluate step
    try
    {
        // compute uncertainty
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

        // design of experiments
        if (m_study->value(Study::General_DoE).toBool())
        {
            // base point for DoE
            QVector<double> init(m_study->parameters().count());
            for (int i = 0; i < m_study->parameters().count(); i++)
                init[i] = x[i];

            // DoE
            m_study->doeCompute(computation, init, value);
        }

        if (m_study->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        // add computation
        m_study->addComputation(computation);

        // penalty
        double totalPenalty = 0.0;
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            if (parameter.penaltyEnabled())
                totalPenalty += parameter.penalty(x[i]);
        }

        m_steps++;
        qInfo() << "BayesOpt: step " << m_steps << "/" << m_study->estimatedNumberOfSteps();

        return value + totalPenalty;
    }
    catch (AgrosSolverException &e)
    {
        qDebug() << e.toString();

        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            qInfo() << parameter.name() << " = " << x[i];
        }

        return 0; // numeric_limits<double>::max();
    }
}

bool BayesOptProblem::checkReachability(const vectord &x)
{
    return true;
    // TODO: fix

    if (!m_study->value(Study::General_SolveProblem).toBool())
        return true;

    // computation
    QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

    // set parameters
    for (int i = 0; i < m_study->parameters().count(); i++)
    {
        Parameter parameter = m_study->parameters()[i];
        computation->config()->parameters()->set(parameter.name(), x[i]);
    }

    // check geometry
    try
    {
        // invalidate scene (parameter update)
        computation->clearSolution();
        computation->scene()->invalidate();
        computation->scene()->loopsInfo()->processPolygonTriangles(true);
        computation->scene()->invalidate();

        computation->scene()->checkGeometryResult();
        computation->scene()->checkGeometryAssignement();

        return true;
    }
    catch (AgrosGeometryException& e)
    {
        qDebug() << e.toString();

        return false;
    }
}

StudyBayesOpt::StudyBayesOpt() : Study()
{    
    // learningTypeList
    learningTypeList.insert(L_FIXED, "fixed");
    learningTypeList.insert(L_EMPIRICAL, "emperical");
    learningTypeList.insert(L_MCMC, "mcmc");

    // scoreTypeList
    scoreTypeList.insert(SC_LOOCV, "loocv");
    scoreTypeList.insert(SC_MTL, "mtl");
    scoreTypeList.insert(SC_ML, "ml");
    scoreTypeList.insert(SC_MAP, "map");

    // surrogateList
    surrogateList.append("sGaussianProcess");
    surrogateList.append("sGaussianProcessML");
    surrogateList.append("sGaussianProcessNormal");
    surrogateList.append("sStudentTProcessJef");
    surrogateList.append("sStudentTProcessNIG");

    // init method
    initMethodList.insert(1, "lhs");
    initMethodList.insert(2, "sobol");
}

QString StudyBayesOpt::learningTypeString(learning_type learningType) const
{
    switch (learningType)
    {
    case L_FIXED:
        return QObject::tr("Fixed");
    case L_EMPIRICAL:
        return QObject::tr("Emperical");
    case L_MCMC:
        return QObject::tr("MCMC");
    default:
        std::cerr << "learning_type type '" + QString::number(learningType).toStdString() + "' is not implemented. learningTypeString(learning_type learningType)" << endl;
        throw;
    }
}

QString StudyBayesOpt::scoreTypeString(score_type scoreType) const
{
    switch (scoreType)
    {
    case SC_LOOCV:
        return QObject::tr("Leave one out cross-validation");
    case SC_MTL:
        return QObject::tr("Maximum total likelihood");
    case SC_ML:
        return QObject::tr("Posterior maximum likelihood");
    case SC_MAP:
        return QObject::tr("Maximum a posteriori");
    default:
        std::cerr << "score_type type '" + QString::number(scoreType).toStdString() + "' is not implemented. scoreTypeString(score_type scoreType)" << endl;
        throw;
    }
}

QString StudyBayesOpt::surrogateString(const QString &surrogateType) const
{
    if (surrogateType == "sGaussianProcess")
        return QObject::tr("Gaussian process (hyperparameters are known)");
    else if (surrogateType == "sGaussianProcessML")
        return QObject::tr("Gaussian process (hyperparameters are estimated using maximum likelihood estimates)");
    else if (surrogateType == "sGaussianProcessNormal")
        return QObject::tr("Gaussian process with a Normal prior on the mean function parameters");
    else if (surrogateType == "sStudentTProcessJef")
        return QObject::tr("Student's t process with a Jeffreys prior");
    else if (surrogateType == "sStudentTProcessNIG")
        return QObject::tr("Student's t process with a Normal prior on the mean function parameters)");
    else
    {
        std::cerr << "surrogate '" + surrogateType.toStdString() + "' is not implemented. surrogateString(const QString &surrogateType)" << endl;
        throw;
    }
}

QString StudyBayesOpt::initMethodString(int method) const
{
    if (method == 1)
        return QObject::tr("Latin Hypercube Sampling (LHS)");
    else if (method == 2)
        return QObject::tr("Sobol Sequences");
    else
    {
        std::cerr << "init method '" + QString::number(method).toStdString() + "' is not implemented. initMethodString(int method)" << endl;
        throw;
    }
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
    par.init_method = initMethodFromStringKey(value(BayesOpt_init_method).toString());
    par.surr_name = surrogateFromStringKey(value(BayesOpt_surr_name).toString()).toStdString();
    par.l_type = learningTypeFromStringKey(value(BayesOpt_l_type).toString());
    par.sc_type = scoreTypeFromStringKey(value(BayesOpt_sc_type).toString());
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
        try
        {
            bayesOptProblem.stepOptimization();
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    // sort computations
    // QString parameterName = m_functionals[0].name();
    // m_computationSets.last().sort(parameterName);

    // vectord result = bayesOptProblem.getFinalResult();

    m_isSolving = false;
}

void StudyBayesOpt::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[BayesOpt_n_init_samples] = 5;
    m_settingDefault[BayesOpt_n_iterations] = 10;
    m_settingDefault[BayesOpt_n_iter_relearn] = 5;
    m_settingDefault[BayesOpt_init_method] = initMethodToStringKey(1); // 1-LHS, 2-Sobol
    m_settingDefault[BayesOpt_surr_name] = surrogateToStringKey("sGaussianProcessML");
    m_settingDefault[BayesOpt_surr_noise] = 1e-10;
    m_settingDefault[BayesOpt_l_type] = learningTypeToStringKey(L_EMPIRICAL);
    m_settingDefault[BayesOpt_sc_type] = scoreTypeToStringKey(SC_MAP);
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


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

#include "study_cmaes.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include "surrogatestrategy.h"
#include "surrogates/rsvm_surr_strategy.hpp"

using namespace libcmaes;

class CMAESProblem
{
public:
    CMAESProblem(StudyCMAES *study) : m_study(study),
        m_lowerBound(new double[study->parameters().count()]),
        m_upperBound(new double[study->parameters().count()])
    {
        // set bounding box
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];

            m_lowerBound[i] = parameter.lowerBound();
            m_upperBound[i] = parameter.upperBound();
        }
    }
    ~CMAESProblem()
    {
        delete [] m_lowerBound;
        delete [] m_upperBound;
    }

    double objectiveFunction(const double *x, const int N)
    {
        if (m_study->isAborted())
        {
            // opt.set_force_stop(1);
            return numeric_limits<double>::max();
        }

        // computation
        QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);

        // set parameters
        for (int i = 0; i < m_study->parameters().count(); i++)
        {
            Parameter parameter = m_study->parameters()[i];
            computation->config()->parameters()->set(parameter.name(), x[i]);
        }

        // evaluate step
        try
        {
            m_study->evaluateStep(computation);
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
            // qInfo() << totalPenalty;

            return value + totalPenalty;
        }
        catch (AgrosSolverException &e)
        {
            qDebug() << e.toString();

            // opt.set_force_stop(2);
            return numeric_limits<double>::max();
        }
    }

    double *lowerBound() const { return m_lowerBound; }
    double *upperBound() const { return m_upperBound; }

private:
    StudyCMAES *m_study;
    // double m_penaltyLambda;
    double *m_lowerBound;
    double *m_upperBound;
};

static CMAESProblem *cmaesProblem = nullptr;

FitFunc objFunctionWrapper = [](const double *x, const int n)
{
    return cmaesProblem->objectiveFunction(x, n);
};

CSurrFunc ftrain = [](const std::vector<Candidate> &c, const dMat &cov)
{
    // do nothing
    return 0;
};

SurrFunc fpredict = [](std::vector<Candidate> &c, const dMat &cov)
{
    // fill up with real fvalue.
    for (size_t i=0;i<c.size();i++)
        c.at(i).set_fvalue(objFunctionWrapper(c.at(i).get_x_ptr(),c.at(i).get_x_size()));
    return 0;
};

StudyCMAES::StudyCMAES() : Study()
{
    surrogateList.append("none");
    surrogateList.append("simple");
    surrogateList.append("rsvm");
    surrogateList.append("acm");

    algorithmList.insert(CMAES_DEFAULT, "CMAES_DEFAULT");
    algorithmList.insert(aCMAES, "aCMAES");
    algorithmList.insert(sepCMAES, "sepCMAES");
    algorithmList.insert(sepaCMAES, "sepaCMAES");
    algorithmList.insert(VD_CMAES, "VD_CMAES");

    // restart strategies
    // algorithmList.insert(IPOP_CMAES, "IPOP_CMAES");
    // algorithmList.insert(BIPOP_CMAES, "BIPOP_CMAES");
    // algorithmList.insert(aIPOP_CMAES, "aIPOP_CMAES");
    // algorithmList.insert(aBIPOP_CMAES, "aBIPOP_CMAES");
    // algorithmList.insert(sepIPOP_CMAES, "sepIPOP_CMAES");
    // algorithmList.insert(sepBIPOP_CMAES, "sepBIPOP_CMAES");
    // algorithmList.insert(sepaIPOP_CMAES, "sepaIPOP_CMAES");
    // algorithmList.insert(sepaBIPOP_CMAES, "sepaBIPOP_CMAES");
    // algorithmList.insert(VD_IPOP_CMAES, "VD_IPOP_CMAES");
    // algorithmList.insert(VD_BIPOP_CMAES, "VD_BIPOP_CMAES");

    cmaesProblem = new CMAESProblem(this);
}

int StudyCMAES::estimatedNumberOfSteps() const
{
    return value(Study::CMAES_maxiter).toInt();
}

QString StudyCMAES::surrogateString(const QString &surrogate) const
{
    if (surrogate == "none")
        return QObject::tr("None");
    else if (surrogate == "simple")
        return QObject::tr("Simple");
    else if (surrogate == "rsvm")
        return QObject::tr("Ranking SVM (support vector machine)");
    else if (surrogate == "acm")
        return QObject::tr("ACM (assisted covariance matrix)");
    else
    {
        std::cerr << "surrogate '" + surrogate.toStdString() + "' is not implemented. surrogateString(const QString &surrogate)" << endl;
        throw;
    }
}

QString StudyCMAES::algorithmString(int algorithm) const
{
    switch (algorithm)
    {
    case CMAES_DEFAULT:
        return QObject::tr("CMA-ES");
    case aCMAES:
        return QObject::tr("Active CMA-ES");
    case sepCMAES:
        return QObject::tr("Separable CMA-ES (linear time)");
    case sepaCMAES:
        return QObject::tr("Active separable CMA-ES (linear time)");
    case VD_CMAES:
        return QObject::tr("VD-CMA-ES (linear time)");
    default:
        std::cerr << "algorithm '" + QString::number(algorithm).toStdString() + "' is not implemented. algorithmString(int algorithm)" << endl;
        throw;
    }

    // restart strategies
    // case IPOP_CMAES:
    //     return QObject::tr("IPOP-CMA-ES");
    // case BIPOP_CMAES:
    //     return QObject::tr("BIPOP-CMA-ES");
    // case aIPOP_CMAES:
    //     return QObject::tr("Active IPOP-CMA-ES");
    // case aBIPOP_CMAES:
    //     return QObject::tr("Active BIPOP-CMA-ES");
    // case sepIPOP_CMAES:
    //     return QObject::tr("sep-IPOP-CMA-ES");
    // case sepBIPOP_CMAES:
    //     return QObject::tr("sep-BIPOP-CMA-ES");
    // case sepaIPOP_CMAES:
    //     return QObject::tr("Active sep-IPOP-CMA-ES");
    // case sepaBIPOP_CMAES:
    //     return QObject::tr("Active sep-BIPOP-CMA-ES");
    // case VD_IPOP_CMAES:
    //     return QObject::tr("VD-IPOP-CMA-ES");
    // case VD_BIPOP_CMAES:
    //     return QObject::tr("VD-BIPOP-CMA-ES");
}

void StudyCMAES::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    addComputationSet(tr("Steps"));

    // inital guess
    std::vector<double> initialGuess(m_parameters.count());

    // set bounding box
    for (int i = 0; i < m_parameters.count(); i++)
    {
        Parameter parameter = m_parameters[i];

        initialGuess[i] = (parameter.lowerBound() + parameter.upperBound()) / 2.0;
    }

    cmaesProblem = new CMAESProblem(this);

    // -1 for automatically decided lambda, 0 is for random seeding of the internal generator.
    double sigma = -1.0;
    double lambda = -1.0;

    // solutions
    CMASolutions cmasols;

    if (value(CMAES_surrogate).toString() == "none")
    {
        GenoPheno<> gp(cmaesProblem->lowerBound(), cmaesProblem->upperBound(), m_parameters.count());

        CMAParameters<> cmaparams(initialGuess, sigma, lambda, 0, gp);
        cmaparams.set_algo(algorithmFromStringKey(value(Study::CMAES_algorithm).toString()));
        cmaparams.set_max_fevals(value(CMAES_maxeval).toInt());
        cmaparams.set_max_iter(value(CMAES_maxiter).toInt());
        cmaparams.set_ftarget(value(CMAES_ftarget).toDouble());

        if (value(CMAES_surrogate).toString() == "simple")
        {
            ESOptimizer<SimpleSurrogateStrategy<CMAStrategy, ACovarianceUpdate>, CMAParameters<> > optim(objFunctionWrapper, cmaparams);
            optim.set_exploit(true);
            optim.optimize();

            cmasols = optim.get_solutions();
        }
        else if (value(CMAES_surrogate).toString() == "acm")
        {
            ESOptimizer<ACMSurrogateStrategy<CMAStrategy, ACovarianceUpdate>, CMAParameters<> > optim(objFunctionWrapper, cmaparams);
            optim.set_exploit(true);
            optim.optimize();

            cmasols = optim.get_solutions();
        }
        else if (value(CMAES_surrogate).toString() == "rsvm")
        {
            ESOptimizer<RSVMSurrogateStrategy<CMAStrategy, ACovarianceUpdate>, CMAParameters<> > optim(objFunctionWrapper, cmaparams);
            optim.set_exploit(true);
            optim.optimize();

            cmasols = optim.get_solutions();
        }
        else
            assert(0);
    }
    else
    {
        // genotype / phenotype transform associated to bounds.
        GenoPheno<pwqBoundStrategy> gp(cmaesProblem->lowerBound(), cmaesProblem->upperBound(), m_parameters.count());

        CMAParameters<GenoPheno<pwqBoundStrategy> > cmaparams(initialGuess, sigma, lambda, 0, gp);
        cmaparams.set_algo(algorithmFromStringKey(value(Study::CMAES_algorithm).toString()));
        cmaparams.set_max_fevals(value(CMAES_maxeval).toInt());
        cmaparams.set_max_iter(value(CMAES_maxiter).toInt());
        cmaparams.set_ftarget(value(CMAES_ftarget).toDouble());

        cmasols = cmaes<GenoPheno<pwqBoundStrategy> >(objFunctionWrapper, cmaparams);
    }
    m_isSolving = false;

    // status
    if (cmasols.run_status() > 7)
    {
        // qInfo() << "optimization took " << cmasols.elapsed_time() / 1000.0 << " seconds, status: " << QString::fromStdString(cmasols.status_msg());
        emit solved();
    }
    else
    {
        // qInfo() << "err: " << cmasols.run_status();
    }

    /*
    if (result > 0)
    {
    if (result == nlopt::SUCCESS)
        Agros2D::log()->printMessage(tr("NLopt"), tr("Successful"));
    else if (result == nlopt::FTOL_REACHED)
        Agros2D::log()->printMessage(tr("NLopt"), tr("Functional tolerance reached"));
    else if (result == nlopt::XTOL_REACHED)
        Agros2D::log()->printMessage(tr("NLopt"), tr("Parameter tolerance reached"));
    else if (result == nlopt::MAXEVAL_REACHED)
        Agros2D::log()->printMessage(tr("NLopt"), tr("Maximum iterations reached"));
    }
    */
}

void StudyCMAES::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[CMAES_maxiter] = 100;
    m_settingDefault[CMAES_maxeval] = 100;
    m_settingDefault[CMAES_ftarget] = 1e-3;
    m_settingDefault[CMAES_surrogate] = "rsvm";
    m_settingDefault[CMAES_algorithm] = algorithmToStringKey(aCMAES);
}

void StudyCMAES::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[CMAES_maxiter] = "CMAES_maxiter";
    m_settingKey[CMAES_maxeval] = "CMAES_maxeval";
    m_settingKey[CMAES_ftarget] = "CMAES_ftarget";
    m_settingKey[CMAES_surrogate] = "CMAES_surrogate";
    m_settingKey[CMAES_algorithm] = "CMAES_algorithm";
}


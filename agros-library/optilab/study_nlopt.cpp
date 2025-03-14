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

#include "study_nlopt.h"

#include "study.h"
#include "parameter.h"
#include "logview.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

class NLoptProblem
{
public:
    NLoptProblem(StudyNLopt *study) :  m_study(study), m_steps(0),
        opt((nlopt::algorithm) study->algorithmFromStringKey(study->value(Study::NLopt_algorithm).toString()), study->parameters().count())
    {

    }

    double evaluatePoint(const std::vector<double> &x)
    {
        return 0;
    }

    double objectiveFunction(const std::vector<double> &x, std::vector<double> &grad, void *data)
    {
        if (m_study->isAborted())
        {
            opt.set_force_stop(1);
            return numeric_limits<double>::max();
        }

        // computation
        QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

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
            if (!m_study->isAborted())
            {
                if (m_study->value(Study::General_ClearSolution).toBool())
                    computation->clearSolution();

                // add computation
                m_study->addComputation(computation);

                m_steps++;
                // qInfo() << "NLOpt: step " << m_steps << "/" << m_study->estimatedNumberOfSteps();
            }

            return value;
        }
        catch (AgrosOptilabEvaluationException &e)
        {
            computation.clear();
            // opt.set_force_stop(2);
            return numeric_limits<double>::max();
        }
    }

    nlopt::opt opt;

private:
    StudyNLopt *m_study;

    int m_steps;
};

double objFunctionWrapper(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
    NLoptProblem *obj = static_cast<NLoptProblem *>(data);
    return obj->objectiveFunction(x, grad, data);
}

StudyNLopt::StudyNLopt() : Study()
{
    algorithmList.insert(nlopt::GN_DIRECT_L, "gn_direct_l");
    algorithmList.insert(nlopt::GN_DIRECT_L_RAND, "gn_direct_l_rand");
    algorithmList.insert(nlopt::GN_MLSL, "gn_mlsl");
    algorithmList.insert(nlopt::GN_CRS2_LM, "gn_crs2_lm");
    algorithmList.insert(nlopt::GN_ISRES, "gn_isres");
    algorithmList.insert(nlopt::GN_ESCH, "gn_esch");
    algorithmList.insert(nlopt::LN_BOBYQA, "ln_bobyqa");
    algorithmList.insert(nlopt::LN_COBYLA, "ln_cobyla");
    algorithmList.insert(nlopt::LN_NELDERMEAD, "ln_neldermead");
    algorithmList.insert(nlopt::LN_SBPLX, "ln_sbplx");
    algorithmList.insert(nlopt::LN_PRAXIS, "ln_praxis");
    algorithmList.insert(nlopt::LN_AUGLAG_EQ, "ln_auglag_eq");
}

int StudyNLopt::estimatedNumberOfSteps() const
{
    return value(Study::NLopt_n_iterations).toInt();
}

QString StudyNLopt::algorithmString(int algorithm) const
{
    switch (algorithm)
    {
    case nlopt::GN_DIRECT_L:
        return QObject::tr("Global - DIviding RECTangles (locally biased)");
    case nlopt::GN_DIRECT_L_RAND:
        return QObject::tr("Global - DIviding RECTangles (locally biased, randomized)");
    case nlopt::GN_MLSL:
        return QObject::tr("Global - Multi-Level Single-Linkage");
    case nlopt::GN_CRS2_LM:
        return QObject::tr("Global - Controlled Random Search (local mutation)");
    case nlopt::GN_ISRES:
        return QObject::tr("Global - Improved Stochastic Ranking Evolution Strategy");
    case nlopt::GN_ESCH:
        return QObject::tr("Global - ESCH (evolutionary algorithm)");
    case nlopt::LN_BOBYQA:
        return QObject::tr("Local - BOBYQA");
    case nlopt::LN_COBYLA:
        return QObject::tr("Local - COBYLA");
    case nlopt::LN_NELDERMEAD:
        return QObject::tr("Local - Nelder-Mead Simplex");
    case nlopt::LN_SBPLX:
        return QObject::tr("Local - Sbplx");
    case nlopt::LN_PRAXIS:
        return QObject::tr("Local - PRincipal AXIS");
    case nlopt::LN_AUGLAG_EQ:
        return QObject::tr("Local - Augmented Lagrangian method");
    default:
        std::cerr << "algorithm '" + QString::number(algorithm).toStdString() + "' is not implemented. algorithmString(int algorithm)" << endl;
        throw;
    }
}

void StudyNLopt::solve()
{
    // start computation
    Study::solve();

    m_isSolving = true;

    addComputationSet(tr("Steps"));

    std::vector<double> initialGuess(m_parameters.count());
    std::vector<double> lowerBound(m_parameters.count());
    std::vector<double> upperBound(m_parameters.count());

    // set bounding box
    for (int i = 0; i < m_parameters.count(); i++)
    {
        Parameter parameter = m_parameters[i];

        lowerBound[i] = parameter.lowerBound();
        upperBound[i] = parameter.upperBound();
        initialGuess[i] = (parameter.lowerBound() + parameter.upperBound()) / 2.0;
    }

    NLoptProblem nLoptProblem(this);
    nLoptProblem.opt.set_min_objective(objFunctionWrapper, &nLoptProblem);
    nLoptProblem.opt.set_lower_bounds(lowerBound);
    nLoptProblem.opt.set_upper_bounds(upperBound);
    nLoptProblem.opt.set_xtol_rel(value(NLopt_xtol_rel).toDouble());
    nLoptProblem.opt.set_ftol_rel(value(NLopt_ftol_rel).toDouble());
    nLoptProblem.opt.set_xtol_abs(value(NLopt_xtol_abs).toDouble());
    nLoptProblem.opt.set_ftol_abs(value(NLopt_ftol_abs).toDouble());
    nLoptProblem.opt.set_maxeval(value(NLopt_n_iterations).toInt());

    try
    {
        double minimum = 0.0; // numeric_limits<double>::max();
        nlopt::result result = nLoptProblem.opt.optimize(initialGuess, minimum);

        m_isSolving = false;
        m_abort = false;

        if (result > 0)
        {
            if (result == nlopt::SUCCESS)
                Agros::log()->printMessage(tr("NLopt"), tr("Successful"));
            else if (result == nlopt::FTOL_REACHED)
                Agros::log()->printMessage(tr("NLopt"), tr("Functional tolerance reached"));
            else if (result == nlopt::XTOL_REACHED)
                Agros::log()->printMessage(tr("NLopt"), tr("Parameter tolerance reached"));
            else if (result == nlopt::MAXEVAL_REACHED)
                Agros::log()->printMessage(tr("NLopt"), tr("Maximum iterations reached"));
            else
                Agros::log()->printError(tr("NLopt"), tr("Uknown error: ") + QString::number(result));
        }
        else
        {
            qCritical() << "err: " << result;
        }
    }
    catch (nlopt::forced_stop &e)
    {
        Agros::log()->printError(tr("NLopt"), e.what());
        m_isSolving = false;
        m_abort = false;
    }
}

void StudyNLopt::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[NLopt_xtol_rel] = 1e-6;
    m_settingDefault[NLopt_xtol_abs] = 1e-12;
    m_settingDefault[NLopt_ftol_rel] = 1e-6;
    m_settingDefault[NLopt_ftol_abs] = 1e-12;
    m_settingDefault[NLopt_n_iterations] = 100;
    m_settingDefault[NLopt_algorithm] = algorithmToStringKey(nlopt::LN_BOBYQA);
}

void StudyNLopt::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[NLopt_xtol_rel] = "NLopt_xtol_rel";
    m_settingKey[NLopt_xtol_abs] = "NLopt_xtol_abs";
    m_settingKey[NLopt_ftol_rel] = "NLopt_ftol_rel";
    m_settingKey[NLopt_ftol_abs] = "NLopt_ftol_abs";
    m_settingKey[NLopt_n_iterations] = "NLopt_n_iterations";
    m_settingKey[NLopt_algorithm] = "NLopt_algorithm";
}


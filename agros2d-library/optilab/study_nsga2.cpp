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

#include "study_nsga2.h"

#include "study.h"
#include "parameter.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"
#include "solver/solutionstore.h"
#include "solver/plugin_interface.h"

#include "scene.h"

#include "nsga2/exception.h"

// TODO: encapsulate
static StudyNSGA2 *localStudy = nullptr;
nsga2::NSGA2 *localNSGA2 = nullptr;

void objectiveFunction(double *xreal, double *xbin, int **gene, double *obj, double *constr)
{    
    if (localStudy->isAborted())
    {
        return;
    }

    // qDebug() << "opt.get_population()" << opt.get_population();

    // computation
    QSharedPointer<Computation> computation = Agros2D::problem()->createComputation(true);

    // set parameters
    for (int i = 0; i < localStudy->parameters().count(); i++)
    {
        Parameter parameter = localStudy->parameters()[i];
        computation->config()->parameters()->set(parameter.name(), xreal[i]);
    }

    // evaluate step
    try
    {
        // multiobjective optimization
        localStudy->evaluateStep(computation);
        QList<double> values = localStudy->evaluateMultiGoal(computation);

        if (localStudy->value(Study::General_ClearSolution).toBool())
            computation->clearSolution();

        for (int i = 0; i < values.count(); i++)
            obj[i] = values[i];

        // add computation
        localStudy->addComputation(computation);
    }
    catch (AgrosSolverException &e)
    {
        qDebug() << e.toString();

        // opt.set_force_stop(2);
        return;
    }
}

void popPopulation(nsga2::population& pop)
{
    localStudy->addComputationSet(QObject::tr("Step %1").arg(pop.generation));

    pop.evaluate();
}

StudyNSGA2::StudyNSGA2() : Study()
{
    // study
    localStudy = this;    
}

int StudyNSGA2::estimatedNumberOfSteps() const
{
    return value(NSGA2_popsize).toInt() * value(NSGA2_ngen).toInt();
}

void StudyNSGA2::solve()
{
    m_computationSets.clear();
    m_isSolving = true;

    nsga2::individual_config conf;

    // set bounding box
    for (int i = 0; i < m_parameters.count(); i++)
    {
        Parameter parameter = m_parameters[i];

        conf.limits_realvar.push_back(make_pair(parameter.lowerBound(), parameter.upperBound()));
    }

    int seed = time(0);

    // nsga2
    localNSGA2 = new nsga2::NSGA2();
    localNSGA2->set_seed(seed);
    localNSGA2->set_nreal(m_parameters.count());
    localNSGA2->set_nbin(0);
    localNSGA2->set_nobj(m_functionals.count());
    localNSGA2->set_ncon(0); // add a constraint due to possible simulation failures
    localNSGA2->set_popsize(value(NSGA2_popsize).toInt());
    localNSGA2->set_ngen(value(NSGA2_ngen).toInt());
    localNSGA2->set_pcross_real(value(NSGA2_pcross).toDouble());
    localNSGA2->set_pmut_real(value(NSGA2_pmut).toDouble());
    localNSGA2->set_eta_c(value(NSGA2_eta_c).toDouble());
    localNSGA2->set_eta_m(value(NSGA2_eta_m).toDouble());
    localNSGA2->set_epsilon_c(1e-14);
    localNSGA2->set_limits_realvar(conf.limits_realvar);
    localNSGA2->set_function(&objectiveFunction);
    localNSGA2->set_popfunction(&popPopulation);
    localNSGA2->set_crowdobj(value(NSGA2_crowdobj).toBool());
    // localNSGA2->set_crowdobj(false); // crowd over the parameters, not the objective functions
    // localNSGA2->set_crowdobj(true); // crowd over objective function

    //localNSGA2->set_nbits(0);
    localNSGA2->set_pcross_bin(0.0);
    localNSGA2->set_pmut_bin(0.0);
    // localNSGA2->set_limits_binvar(limits_binvar);
    // localNSGA2->set_custom_report_function(&update_generation);
    // localNSGA2->set_nreport(10);
    localNSGA2->set_backup_filename(""); // no backup

    try
    {
        localNSGA2->initialize();
        localNSGA2->evolve();

        m_isSolving = false;

        // sort computations
        // QString parameterName = m_functionals[0].name();
        // m_computationSets.last().sort(parameterName);

        emit solved();
    }
    catch (nsga2::nsga2exception &e)
    {
        Agros2D::log()->printError(tr("NSGA2"), e.what());
        m_isSolving = false;
    }

    delete localNSGA2;
    localNSGA2 = nullptr;
}

void StudyNSGA2::setDefaultValues()
{
    Study::setDefaultValues();

    m_settingDefault[NSGA2_popsize] = 4;
    m_settingDefault[NSGA2_ngen] = 3;
    m_settingDefault[NSGA2_pcross] = 0.6;
    m_settingDefault[NSGA2_pmut] = 0.2;
    m_settingDefault[NSGA2_eta_c] = 10.0;
    m_settingDefault[NSGA2_eta_m] = 20.0;
    m_settingDefault[NSGA2_crowdobj] = false;
}

void StudyNSGA2::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[NSGA2_popsize] = "NSGA2_popsize";
    m_settingKey[NSGA2_ngen] = "NSGA2_ngen";
    m_settingKey[NSGA2_pcross] = "NSGA2_pcross";
    m_settingKey[NSGA2_pmut] = "NSGA2_pmut";
    m_settingKey[NSGA2_eta_c] = "NSGA2_eta_c";
    m_settingKey[NSGA2_eta_m] = "NSGA2_eta_m";
    m_settingKey[NSGA2_crowdobj] = "NSGA2_crowdobj";
}

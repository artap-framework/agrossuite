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

// hack to redefinition of serialize function
#define PAGMO_S11N_HPP
#include "study_pagmo2.h"

#include "study.h"
#include "parameter.h"
#include "logview.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include <cmath>
#include <initializer_list>
#include <iostream>
#include <utility>
#include <boost/math/constants/constants.hpp>

#include <pagmo/problem.hpp>
#include <pagmo/types.hpp>

#include <pagmo/algorithm.hpp>
#include <pagmo/population.hpp>
#include <pagmo/algorithms/pso.hpp>
#include <pagmo/algorithms/bee_colony.hpp>
#include <pagmo/algorithms/gaco.hpp>
#include <pagmo/algorithms/de.hpp>
#include <pagmo/algorithms/gwo.hpp>
#include <pagmo/algorithms/sga.hpp>
#include <pagmo/algorithms/simulated_annealing.hpp>
#include <pagmo/algorithms/moead.hpp>
#include <pagmo/algorithms/maco.hpp>
#include <pagmo/algorithms/nspso.hpp>
#include <pagmo/algorithms/nsga2.hpp>

static StudyPagmo *localStudy = nullptr;
static int localSteps = 0;


struct ProblemPagmo
{
	// Implementation of the objective function.
	pagmo::vector_double fitness(const pagmo::vector_double &dv) const
	{
		if (localStudy->isAborted())
		{
			return {numeric_limits<double>::max()};
		}

		// computation
		QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

		// set parameters
		for (int i = 0; i < localStudy->parameters().count(); i++)
		{
			Parameter parameter = localStudy->parameters()[i];
			computation->config()->parameters()->set(parameter.name(), dv[i]);
		}

		// evaluate step
		try
		{
			localStudy->evaluateStep(computation);

			if (localStudy->isSingleObjective(localStudy->value(Study::Pagmo_algorithm).toString()))
			{
				double value = localStudy->evaluateSingleGoal(computation);
				if (!localStudy->isAborted())
				{
					if (localStudy->value(Study::General_ClearSolution).toBool())
						computation->clearSolution();

					// add computation
					localStudy->addComputation(computation);

					localSteps++;
					// qInfo() << "NLOpt: step " << m_steps << "/" << localStudy->estimatedNumberOfSteps();
				}
				return {value};
			}
			else if (localStudy->isMultiObjective(localStudy->value(Study::Pagmo_algorithm).toString()))
			{
				QList<double> value = localStudy->evaluateMultiGoal(computation);
				if (!localStudy->isAborted())
				{
					if (localStudy->value(Study::General_ClearSolution).toBool())
						computation->clearSolution();

					// add computation
					localStudy->addComputation(computation);

					localSteps++;
					// qInfo() << "NLOpt: step " << m_steps << "/" << localStudy->estimatedNumberOfSteps();
				}

				pagmo::vector_double valueOut(value.count());
				for (int i = 0; i < value.count(); i++)
					valueOut[i] = value[i];
				return valueOut;
			}
			else
			{
				assert(0);
			}
		}
		catch (AgrosOptilabEvaluationException &e)
		{
			computation.clear();
			// opt.set_force_stop(2);
			return {numeric_limits<double>::max()};
		}
	}
	// Implementation of the box bounds.
	std::pair<pagmo::vector_double, pagmo::vector_double> get_bounds() const
	{
		// qInfo() << localStudy->bounds();
		return localStudy->bounds();
	}

	pagmo::vector_double::size_type get_nx() const
	{
		return localStudy->parameters().count();
	}

	pagmo::vector_double::size_type get_nobj() const
	{
		return localStudy->goalFunctions().count();
	}
};


StudyPagmo::StudyPagmo() : Study()
{
	// study
	localStudy = this;
	localSteps = 0;
}

int StudyPagmo::estimatedNumberOfSteps() const
{
	return value(Pagmo_popsize).toInt() * value(Pagmo_ngen).toInt();
}

QStringList StudyPagmo::algorithmStringKeys()
{
	QStringList list;
	// single objective
	list.append("de");
	list.append("gwo");
	list.append("pso");
	list.append("sga");
	list.append("sa");
	list.append("bee");
	// multi objective
	list.append("nsga2");
	list.append("moead");
	list.append("maco");
	list.append("nspso");

	return list;
}

QString StudyPagmo::algorithmString(const QString &algorithm)
{
	if (algorithm == "de")
		return tr("Differential Evolution (SO)");
	else if (algorithm == "gwo")
		return tr("Grey Wolf Optimizer (SO)");
	else if (algorithm == "pso")
		return tr("Particle Swarm Optimization (SO)");
	else if (algorithm == "sga")
		return tr("Simple Genetic Algorithm (SO)");
	else if (algorithm == "sa")
		return tr("Simulated Annealing (SO)");
	else if (algorithm == "bee")
		return tr("Artificial Bee Colony (SO)");
	else if (algorithm == "nsga2")
		return tr("Non-dominated Sorting Genetic Algorithm II (MO)");
	else if (algorithm == "moead")
		return tr("Multi-Objective Evolutionary Algorithm based on Decomposition (MO)");
	else if (algorithm == "maco")
		return tr("Multi-objective Ant Colony Optimization (MO)");
	else if (algorithm == "nspso")
		return tr("Non-dominated Sorting Particle Swarm Optimization (MO)");
	return "";
}


bool StudyPagmo::isSingleObjective(const QString &algorithm)
{
    if (algorithm == "de")
		return true;
	else if (algorithm == "gwo")
	 	return true;
	else if (algorithm == "pso")
		return true;
	else if (algorithm == "sga")
		return true;
	else if (algorithm == "sa")
	 	return true;
	else if (algorithm == "bee")
		return true;
	else if (algorithm == "nsga2")
		return false;
	else if (algorithm == "moead")
		return false;
	else if (algorithm == "maco")
		return false;
	else if (algorithm == "nspso")
		return false;

	assert(0);
	return false;
}

bool StudyPagmo::isMultiObjective(const QString &algorithm)
{
	if (algorithm == "de")
		return false;
	else if (algorithm == "gwo")
		return false;
	else if (algorithm == "pso")
		return true;
	else if (algorithm == "sga")
		return false;
	else if (algorithm == "sa")
		return false;
	else if (algorithm == "bee")
		return false;
	else if (algorithm == "nsga2")
		return true;
	else if (algorithm == "moead")
		return true;
	else if (algorithm == "maco")
		return true;
	else if (algorithm == "nspso")
		return true;

	assert(0);
	return false;
}

void StudyPagmo::solve()
{
	if ((goalFunctions().count() == 1) && !isSingleObjective(value(Study::Pagmo_algorithm).toString()))
	{
		m_hasError = true;
		Agros::log()->printError(tr("Study"), tr("Single objective optimization is not implemented for algorithm '%1'.").arg(value(Study::Pagmo_algorithm).toString()));
	}
	else if ((goalFunctions().count() > 1) && !isMultiObjective(value(Study::Pagmo_algorithm).toString()))
	{
		m_hasError = true;
		Agros::log()->printError(tr("Study"), tr("Multi objective optimization is not implemented for algorithm '%1'.").arg(value(Study::Pagmo_algorithm).toString()));
	}

	// start computation
	Study::solve();

    m_isSolving = true;

	try
	{
		// Construct a pagmo::problem
		pagmo::problem p{ProblemPagmo{}};

		// 2 - Instantiate a pagmo algorithm
		pagmo::algorithm algo;

		auto algorithmName = value(Study::Pagmo_algorithm).toString();

		if (algorithmName == "gaco")
			algo = pagmo::algorithm{pagmo::gaco((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "de")
			algo = pagmo::algorithm{pagmo::de((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "gwo")
			algo = pagmo::algorithm{pagmo::gwo((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "pso")
			algo = pagmo::algorithm{pagmo::pso((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "sga")
			algo = pagmo::algorithm{pagmo::sga((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "sa")
			algo = pagmo::algorithm{pagmo::simulated_annealing((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "bee")
			algo = pagmo::algorithm{pagmo::bee_colony((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "nsga2")
			algo = pagmo::algorithm{pagmo::nsga2((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "moead")
			algo = pagmo::algorithm{pagmo::moead((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "maco")
			algo = pagmo::algorithm{pagmo::maco((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else if (algorithmName == "nspso")
			algo = pagmo::algorithm{pagmo::nspso((unsigned int) value(Study::Pagmo_ngen).toInt())};
		else
			Agros::log()->printError(tr("Study"), tr("Unknown algorithm: %1").arg(algorithmName));

		// 3 - Instantiate a population
		pagmo::population pop{p, (unsigned int) value(Study::Pagmo_popsize).toInt()};

		// 4 - Evolve the population
		pop = algo.evolve(pop);

		// 5 - Output the population
		// std::cout << "The population: \n" << pop;

		// Print p to screen.
		// std::cout << p << '\n';

		addComputationSet(tr("Values"));

		// remove empty computation sets
		localStudy->removeEmptyComputationSets();

		m_hasError = false;
	}
	catch (std::exception &e)
	{
		m_hasError = true;

		Agros::log()->printError(tr("Study pagmo"), e.what());
	}

	m_isSolving = false;
}

std::pair<std::vector<double>, std::vector<double> > StudyPagmo::bounds()
{
	std::vector<double> lowerBound(m_parameters.count());
	std::vector<double> upperBound(m_parameters.count());

	// set bounding box
	for (int i = 0; i < m_parameters.count(); i++)
	{
		Parameter parameter = m_parameters[i];

		lowerBound[i] = parameter.lowerBound();
		upperBound[i] = parameter.upperBound();
	}

	return std::pair(lowerBound, upperBound);
}

void StudyPagmo::setDefaultValues()
{
    Study::setDefaultValues();

	m_settingDefault[Pagmo_algorithm] = "gwo";
	m_settingDefault[Pagmo_popsize] = 10;
	m_settingDefault[Pagmo_ngen] = 20;
}

void StudyPagmo::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[Pagmo_algorithm] = "Pagmo_algorithm";
	m_settingKey[Pagmo_popsize] = "Pagmo_popsize";
	m_settingKey[Pagmo_ngen] = "Pagmo_ngen";
}


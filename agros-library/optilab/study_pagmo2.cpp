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

#include <pagmo/problem.hpp>
#include <pagmo/types.hpp>

#include <pagmo/algorithm.hpp>
#include <pagmo/algorithms/pso.hpp>
#include <pagmo/population.hpp>

static StudyPagmo *localStudy = nullptr;
static int localSteps = 0;


struct ProblemPagmo
{
	// Implementation of the objective function.
	// pagmo::vector_double fitness(const pagmo::vector_double &dv) const
	// {
	// 	return {dv[0] * dv[3] * (dv[0] + dv[1] + dv[2]) + dv[2]};
	// }
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
		qInfo() << localStudy->bounds();
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
	return value(OpenGA_popsize).toInt() * value(OpenGA_ngen).toInt();
}

QStringList StudyPagmo::algorithmStringKeys()
{
	QStringList list;
	// list << algorithmToString(EA::GA_MODE::SOGA);
	// list << algorithmToString(EA::GA_MODE::NSGA_III);

	return list;
}

QString StudyPagmo::algorithmString(const QString &algorithm)
{
	// const EA::GA_MODE alg = stringToAlgorithm(algorithm);
	// if (alg == EA::GA_MODE::SOGA)
	// 	return QObject::tr("Single objective GA");
	// if (alg == EA::GA_MODE::NSGA_III)
	// 	return QObject::tr("NSGA III");

	// assert(0);
	return "";
}

void StudyPagmo::solve()
{
	// start computation
	Study::solve();

    m_isSolving = true;
	qInfo() << "Pagmo";

	// Construct a pagmo::problem from our example problem.
	pagmo::problem p{ProblemPagmo{}};

	// Compute the value of the objective function
	// in the point (1, 2, 3, 4).
	// std::cout << "Value of the objfun in (1, 2, 3, 4): " << p.fitness({0.002, 0.002, 0.002, 0.002, 0.002})[0] << '\n';

	// Fetch the lower/upper bounds for the first variable.
	std::cout << "Lower bounds: [" << p.get_lb()[0] << "]\n";
	std::cout << "Upper bounds: [" << p.get_ub()[0] << "]\n\n";

	// 2 - Instantiate a pagmo algorithm
	pagmo::algorithm algo{pagmo::pso(10)};

	// 3 - Instantiate a population
	pagmo::population pop{p, 10};

	// 4 - Evolve the population
	pop = algo.evolve(pop);

	// 5 - Output the population
	std::cout << "The population: \n" << pop;

	// Print p to screen.
	std::cout << p << '\n';

	qInfo() << "Pagmo - OK";

	addComputationSet(tr("Initial values"));


	// remove empty computation sets
	// localStudy->removeEmptyComputationSets();

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

	// m_settingDefault[OpenGA_algorithm] = algorithmToString(EA::GA_MODE::SOGA);
	// m_settingDefault[OpenGA_algorithm] = algorithmToString(EA::GA_MODE::NSGA_III);
	m_settingDefault[OpenGA_popsize] = 10;
	m_settingDefault[OpenGA_ngen] = 20;
	m_settingDefault[OpenGA_elite_count] = 5;
	m_settingDefault[OpenGA_mutation_rate] = 0.4;
	m_settingDefault[OpenGA_crossover_fraction] = 0.7;
}

void StudyPagmo::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[OpenGA_algorithm] = "OpenGA_algorithm";
	m_settingKey[OpenGA_popsize] = "OpenGA_popsize";
	m_settingKey[OpenGA_ngen] = "OpenGA_ngen";
	m_settingKey[OpenGA_elite_count] = "OpenGA_elite_count";
	m_settingKey[OpenGA_mutation_rate] = "OpenGA_mutation_rate";
	m_settingKey[OpenGA_crossover_fraction] = "OpenGA_crossover_fraction";
}


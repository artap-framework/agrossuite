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

#include "study_openga.h"

#include "study.h"
#include "parameter.h"
#include "logview.h"

#include "util/global.h"
#include "solver/problem.h"
#include "solver/problem_result.h"

#include "openga/openGA.hpp"

static StudyOpenGA *localStudy = nullptr;
static int localSteps = 0;

QString algorithmToString(EA::GA_MODE algorithm)
{
	if (algorithm == EA::GA_MODE::SOGA)
		return "soga";
	if (algorithm == EA::GA_MODE::NSGA_III)
		return "nsga_iii";

	assert(0);
	return "";
}

EA::GA_MODE stringToAlgorithm(const QString &algorithm)
{
	if (algorithm == "soga")
		return EA::GA_MODE::SOGA;
	if (algorithm == "nsga_iii")
		return EA::GA_MODE::NSGA_III;

	assert(0);
	return EA::GA_MODE::SOGA;
}

struct MySolution
{
	MySolution()
	{
		auto n = localStudy->parameters().size();
		for (int i = 0; i < n; i++)
			x.push_back(0.0);
	}

	MySolution(const MySolution& other)
	{
		for (int i = 0; i < other.x.size(); i++)
			x.push_back(other.x[i]);
	}

	std::string to_string() const
	{
		std::string str = "[";
		for (int i = 0; i < x.size(); i++)
			str += std::to_string(x[i]) + ((i == x.size()-1) ? "" : ", ");
		str += "]";

		return str;
	}

	std::vector<double> x;
};

struct MyMiddleCost
{
	MyMiddleCost()
	{
		auto n = localStudy->goalFunctions().size();
		for (int i = 0; i < n; i++)
			cost_f.push_back(0.0);
	}

	MyMiddleCost(const MyMiddleCost& other)
	{
		for (int i = 0; i < other.cost_f.size(); i++)
			cost_f.push_back(other.cost_f[i]);
	}

	std::vector<double> cost_f;
};

typedef EA::Genetic<MySolution, MyMiddleCost> GA_Type;
typedef EA::GenerationType<MySolution, MyMiddleCost> Generation_Type;
static GA_Type ga_obj;

void init_genes(MySolution& p, const std::function<double(void)> &rnd01)
{
	for (int i = 0; i < localStudy->parameters().size(); i++)
	{
		Parameter parameter = localStudy->parameters()[i];
		p.x[i] = parameter.lowerBound() + (parameter.upperBound() - parameter.lowerBound()) * rnd01();
	}
}

bool eval_solution(const MySolution& p, MyMiddleCost &c)
{
	if (localStudy->isAborted())
	{
		ga_obj.user_request_stop = true;
	    return false;
	}

	// computation
	QSharedPointer<Computation> computation = Agros::problem()->createComputation(true);

	// set parameters
	for (int i = 0; i < localStudy->parameters().count(); i++)
	{
	    Parameter parameter = localStudy->parameters()[i];
	    computation->config()->parameters()->set(parameter.name(), p.x[i]);
	}

	// evaluate step
	try
	{
	    localStudy->evaluateStep(computation);
		QList<double> values = localStudy->evaluateMultiGoal(computation);

	    if (localStudy->value(Study::General_ClearSolution).toBool())
	        computation->clearSolution();

	    // add computation
	    localStudy->addComputation(computation);

		// set values
		for (int i = 0; i < values.count(); i++)
			c.cost_f[i] = values[i];

	    localSteps++;
	    // qInfo() << "OpenGA: step " << localSteps << "/" << localStudy->estimatedNumberOfSteps();

		return true;
	}
	catch (AgrosSolverException &e)
	{
	    qDebug() << e.toString();
	    return false;
	}
}

MySolution mutate(const MySolution& X_base, const std::function<double(void)> &rnd01, double shrink_scale)
{
	MySolution X_new;

	// mutation radius
    const double mu = 0.2 * shrink_scale;

	bool out_of_range;
	do
	{
		X_new = X_base;

		out_of_range = false;
		for (int i = 0; i < X_new.x.size(); i++)
		{
			X_new.x[i] += mu * (rnd01() - rnd01());

			// check limits
			Parameter parameter = localStudy->parameters()[i];
			if (X_new.x[i] < parameter.lowerBound() || X_new.x[i] > parameter.upperBound())
				out_of_range = true;

			// qInfo() << parameter.lowerBound() << X_new.x[i] << parameter.upperBound() << in_range;

			if (out_of_range)
			{
				// qInfo() << "mutate out of range";
				break;
			}
		}
	}
	while (out_of_range);

	// qInfo() << "Mutation: " << X_base.to_string().c_str() << " -> " << X_new.to_string().c_str();
	return X_new;
}

MySolution crossover(const MySolution& X1, const MySolution& X2, const std::function<double(void)> &rnd01)
{
	MySolution X_new;

	bool out_of_range;
	do
	{
		out_of_range = false;
		for (int i = 0; i < X_new.x.size(); i++)
		{
			const double r = rnd01();
			X_new.x[i] = r * X1.x[i] + (1.0 - r) * X2.x[i];

			// check limits
			Parameter parameter = localStudy->parameters()[i];
			if (X_new.x[i] < parameter.lowerBound() || X_new.x[i] > parameter.upperBound())
				out_of_range = true;

			// qInfo() << parameter.lowerBound() << X_new.x[i] << parameter.upperBound() << in_range;

			if (out_of_range)
			{
				//  qInfo() << "crossover out of range";
				break;
			}
		}
	}
	while (out_of_range);

	// qInfo() << "Crossover: " << X1.to_string().c_str() << " + " << X2.to_string().c_str() << " = " << X_new.to_string().c_str();
	return X_new;
}

double calculate_so_total_fitness(const GA_Type::thisChromosomeType &X)
{
	double out = 0.0;
	for (int i = 0; i < X.middle_costs.cost_f.size(); i++)
		out += X.middle_costs.cost_f[i];
	// qInfo() << "Total fitness: " << out;
	return out;
}

std::vector<double> calculate_mo_objectives(const GA_Type::thisChromosomeType &X)
{
	return X.middle_costs.cost_f;
}

void so_report_generation(int generation_number, const EA::GenerationType<MySolution, MyMiddleCost> &last_generation, const MySolution& best_genes)
{
	localStudy->addComputationSet(QObject::tr("Generation %1").arg(generation_number));
	std::cout
		<<"Generation ["<<generation_number<<"], "
		<<"Best="<<last_generation.best_total_cost<<", "
		<<"Average="<<last_generation.average_cost<<", "
		<<"Best genes=("<<best_genes.to_string()<<")"<<", "
		<<"Exe_time="<<last_generation.exe_time
		<<std::endl;
}

void mo_report_generation(int generation_number, const EA::GenerationType<MySolution,MyMiddleCost> &last_generation, const std::vector<unsigned int>& pareto_front)
{
	localStudy->addComputationSet(QObject::tr("Generation %1").arg(generation_number));
	// (void) last_generation;
	//
	// std::cout<<"Generation ["<< generation_number <<"], ";
	// std::cout<<"Pareto-Front {";
	// for (unsigned int i=0; i<pareto_front.size(); i++)
	// {
	// 	std::cout << (i>0 ? "," : "");
	// 	std::cout << pareto_front[i];
	// }
	// std::cout << "}" << std::endl;
}

StudyOpenGA::StudyOpenGA() : Study()
{
	// study
	localStudy = this;
	localSteps = 0;
}

int StudyOpenGA::estimatedNumberOfSteps() const
{
	return value(OpenGA_popsize).toInt() * value(OpenGA_ngen).toInt();
}

QStringList StudyOpenGA::algorithmStringKeys()
{
	QStringList list;
	list << algorithmToString(EA::GA_MODE::SOGA);
	list << algorithmToString(EA::GA_MODE::NSGA_III);

	return list;
}

QString StudyOpenGA::algorithmString(const QString &algorithm)
{
	const EA::GA_MODE alg = stringToAlgorithm(algorithm);
	if (alg == EA::GA_MODE::SOGA)
		return QObject::tr("Single objective GA");
	if (alg == EA::GA_MODE::NSGA_III)
		return QObject::tr("NSGA III");

	assert(0);
	return "";
}

void StudyOpenGA::solve()
{
    m_computationSets.clear();
    m_isSolving = true;
	localSteps = 0;

    addComputationSet(tr("Initial values"));

	EA::Chronometer timer;
	timer.tic();

	EA::GA_MODE alg = stringToAlgorithm(m_settingDefault[OpenGA_algorithm].toString());

	ga_obj.problem_mode = alg;
	ga_obj.dynamic_threading = false;
	ga_obj.multi_threading = false;
	ga_obj.idle_delay_us = 1; // switch between threads quickly
	ga_obj.verbose = false;
	ga_obj.population = value(OpenGA_popsize).toInt();
	ga_obj.generation_max = value(OpenGA_ngen).toInt();
	ga_obj.elite_count = value(OpenGA_elite_count).toInt();
	if (alg == EA::GA_MODE::SOGA)
	{
		ga_obj.calculate_SO_total_fitness = calculate_so_total_fitness;
		ga_obj.SO_report_generation = so_report_generation;
	}
	else
	{
		ga_obj.calculate_MO_objectives = calculate_mo_objectives;
		ga_obj.MO_report_generation = mo_report_generation;
	}
	ga_obj.init_genes = init_genes;
	ga_obj.eval_solution = eval_solution;
	ga_obj.mutate = mutate;
	ga_obj.mutation_rate = value(OpenGA_mutation_rate).toDouble();
	ga_obj.crossover = crossover;
	ga_obj.crossover_fraction = value(OpenGA_crossover_fraction).toDouble();
	ga_obj.solve();

	// remove empty computation sets
	localStudy->removeEmptyComputationSets();

	m_isSolving = false;
}

void StudyOpenGA::setDefaultValues()
{
    Study::setDefaultValues();

	m_settingDefault[OpenGA_algorithm] = algorithmToString(EA::GA_MODE::SOGA);
	// m_settingDefault[OpenGA_algorithm] = algorithmToString(EA::GA_MODE::NSGA_III);
	m_settingDefault[OpenGA_popsize] = 10;
	m_settingDefault[OpenGA_ngen] = 20;
	m_settingDefault[OpenGA_elite_count] = 5;
	m_settingDefault[OpenGA_mutation_rate] = 0.4;
	m_settingDefault[OpenGA_crossover_fraction] = 0.7;
}

void StudyOpenGA::setStringKeys()
{
    Study::setStringKeys();

    m_settingKey[OpenGA_algorithm] = "OpenGA_algorithm";
	m_settingKey[OpenGA_popsize] = "OpenGA_popsize";
	m_settingKey[OpenGA_ngen] = "OpenGA_ngen";
	m_settingKey[OpenGA_elite_count] = "OpenGA_elite_count";
	m_settingKey[OpenGA_mutation_rate] = "OpenGA_mutation_rate";
	m_settingKey[OpenGA_crossover_fraction] = "OpenGA_crossover_fraction";
}


#include "alg_nsgaiii.h"
#include "problem_base.h"
#include "alg_individual.h"
#include "alg_reference_point.h"
#include "alg_population.h"

#include "alg_initialization.h"
#include "alg_crossover.h"
#include "alg_mutation.h"
#include "alg_environmental_selection.h"

#include <vector>
#include <fstream>

using namespace std;

NSGA3::NSGA3(double pc, double pm, double eta_c, double eta_m, int pop_size, int gen_num):
	name_("NSGAIII"),
    gen_num_(gen_num),
    pc_(pc), // default setting in NSGA-III (IEEE tEC 2014)
    pm_(pm),
    eta_c_(eta_c), // default setting
    eta_m_(eta_m) // default setting
{
    obj_division_p_.push_back(pop_size);
}

// ----------------------------------------------------------------------
void NSGA3::Solve(CPopulation *solutions, const NSGA3ProblemBase &problem)
{   
	CIndividual::SetTargetProblem(problem);
	
	vector<CReferencePoint> rps;
	GenerateReferencePoints(&rps, problem.num_objectives(), obj_division_p_); 
	size_t PopSize = rps.size();
	while (PopSize%4) PopSize += 1;

	CPopulation pop[2]={CPopulation(PopSize)};
	CSimulatedBinaryCrossover SBX(pc_, eta_c_);
	CPolynomialMutation PolyMut(1.0/problem.num_variables(), eta_m_);

	int cur = 0, next = 1;
	RandomInitialization(&pop[cur], problem);
	for (size_t i=0; i<PopSize; i+=1)
	{
		problem.Evaluate(&pop[cur][i]);
	}

	for (size_t t=0; t<gen_num_; t+=1)
	{
		pop[cur].resize(PopSize*2);

		for (size_t i=0; i<PopSize; i+=2)
		{
			int father = rand()%PopSize,
				mother = rand()%PopSize;

			SBX(&pop[cur][PopSize+i], &pop[cur][PopSize+i+1], pop[cur][father], pop[cur][mother]);

			PolyMut(&pop[cur][PopSize+i]);
			PolyMut(&pop[cur][PopSize+i+1]);

			problem.Evaluate(&pop[cur][PopSize+i]);
			problem.Evaluate(&pop[cur][PopSize+i+1]);
		}

		EnvironmentalSelection(&pop[next], &pop[cur], rps, PopSize);

		std::swap(cur, next);
	}

	*solutions = pop[cur];
}

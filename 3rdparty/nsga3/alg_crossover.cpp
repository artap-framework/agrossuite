
#include "alg_crossover.h"
#include "alg_individual.h"
#include "aux_math.h"
#include "problem_base.h"

#include <cmath>
#include <algorithm>
#include <cstddef>
using std::size_t;

// ----------------------------------------------------------------------
// The implementation was adapted from the code of function realcross() in crossover.c
// http://www.iitk.ac.in/kangal/codes/nsga2/nsga2-gnuplot-v1.1.6.tar.gz
//
// ref: http://www.slideshare.net/paskorn/simulated-binary-crossover-presentation#
// ----------------------------------------------------------------------
double CSimulatedBinaryCrossover::get_betaq(double rand, double alpha, double eta) const
{
	double betaq = 0.0;
	if (rand <= (1.0/alpha))
	{
		betaq = std::pow((rand*alpha),(1.0/(eta+1.0)));
	}
	else
	{
		betaq = std::pow((1.0/(2.0 - rand*alpha)),(1.0/(eta+1.0)));
	}
	return betaq;
}
// ----------------------------------------------------------------------
bool CSimulatedBinaryCrossover::operator()(CIndividual *child1, 
										   CIndividual *child2, 
										   const CIndividual &parent1, 
										   const CIndividual &parent2, 
										   double cr, 
										   double eta) const
{
	*child1 = parent1;
	*child2 = parent2;

	if (MathAux::random(0.0, 1.0) > cr) return false; // not crossovered

	CIndividual::TDecVec &c1 = child1->vars(), &c2 = child2->vars();
	const CIndividual::TDecVec &p1 = parent1.vars(), &p2 = parent2.vars();						

	for (size_t i=0; i<c1.size(); i+=1)
	{
		if (MathAux::random(0.0, 1.0) > 0.5) continue; // these two variables are not crossovered
		if (std::fabs(p1[i]-p2[i]) <= MathAux::EPS) continue; // two values are the same
		
		double y1 = std::min(p1[i], p2[i]),
			   y2 = std::max(p1[i], p2[i]);

		double lb = CIndividual::TargetProblem().lower_bounds()[i],
			   ub = CIndividual::TargetProblem().upper_bounds()[i];

		double rand = MathAux::random(0.0, 1.0);

		// child 1
		double beta = 1.0 + (2.0*(y1-lb)/(y2-y1)),
			   alpha = 2.0 - std::pow(beta, -(eta+1.0));
		double betaq = get_betaq(rand, alpha, eta);
		
		c1[i] = 0.5*((y1+y2)-betaq*(y2-y1));

		// child 2
		beta = 1.0 + (2.0*(ub-y2)/(y2-y1));
		alpha = 2.0 - std::pow(beta, -(eta+1.0));
		betaq = get_betaq(rand, alpha, eta);

		c2[i] = 0.5*((y1+y2)+betaq*(y2-y1));

		// boundary checking
		c1[i] = std::min(ub, std::max(lb, c1[i]));
		c2[i] = std::min(ub, std::max(lb, c2[i]));

		if (MathAux::random(0.0, 1.0) <= 0.5)
		{
			std::swap(c1[i], c2[i]);
		}
	}

	return true;
}// CSimulatedBinaryCrossover
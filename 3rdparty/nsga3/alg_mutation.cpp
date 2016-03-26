
#include "alg_mutation.h"
#include "alg_individual.h"
#include "aux_math.h"
#include "problem_base.h"

#include <cstddef>
#include <algorithm>
using std::size_t;

// ----------------------------------------------------------------------
// The implementation was adapted from the code of function real_mutate_ind() in mutation.c in
// http://www.iitk.ac.in/kangal/codes/nsga2/nsga2-gnuplot-v1.1.6.tar.gz
//
// ref: http://www.slideshare.net/paskorn/simulated-binary-crossover-presentation#
// ---------------------------------------------------------------------
bool CPolynomialMutation::operator()(CIndividual *indv, double mr, double eta) const
{
    //int j;
    //double rnd, delta1, delta2, mut_pow, deltaq;
    //double y, yl, yu, val, xy;

	bool mutated = false;

	CIndividual::TDecVec &x = indv->vars();

    for (size_t i=0; i<x.size(); i+=1)
    {
		if (MathAux::random(0.0, 1.0) <= mr)
        {
			mutated = true;

            double y = x[i],
			       lb = CIndividual::TargetProblem().lower_bounds()[i],
			       ub = CIndividual::TargetProblem().upper_bounds()[i];

            double delta1 = (y-lb)/(ub-lb),
                   delta2 = (ub-y)/(ub-lb);
            
			double mut_pow = 1.0/(eta+1.0);

			double rnd = MathAux::random(0.0, 1.0), deltaq = 0.0;
            if (rnd <= 0.5)
            {
                double xy = 1.0-delta1;
                double val = 2.0*rnd+(1.0-2.0*rnd)*(pow(xy,(eta+1.0)));
                deltaq =  pow(val,mut_pow) - 1.0;
            }
            else
            {
                double xy = 1.0-delta2;
                double val = 2.0*(1.0-rnd)+2.0*(rnd-0.5)*(pow(xy,(eta+1.0)));
                deltaq = 1.0 - (pow(val,mut_pow));
            }

            y = y + deltaq*(ub-lb);
			y = std::min(ub, std::max(lb, y));

            x[i] = y;
        }
    }

	return mutated;
}// CPolynomialMutation
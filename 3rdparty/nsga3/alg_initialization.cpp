#include "alg_initialization.h"
#include "problem_base.h"
#include "alg_individual.h"
#include "alg_population.h"
#include "aux_math.h"

#include <cstddef>
using std::size_t;

CRandomInitialization RandomInitialization;

void CRandomInitialization::operator()(CIndividual *indv, const NSGA3ProblemBase &prob) const
{
	CIndividual::TDecVec &x = indv->vars();
	x.resize(prob.num_variables());

	for (size_t i=0; i<x.size(); i+=1)
	{
		x[i] = MathAux::random(prob.lower_bounds()[i], prob.upper_bounds()[i]);
	}
}
// ----------------------------------------------------------------------
void CRandomInitialization::operator()(CPopulation *pop, const NSGA3ProblemBase &prob) const
{
	for (size_t i=0; i<pop->size(); i+=1)
	{
		(*this)( &(*pop)[i], prob );
	}
}
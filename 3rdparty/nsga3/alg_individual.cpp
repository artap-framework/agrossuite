
#include "alg_individual.h"
#include "problem_base.h"

using std::size_t;


const NSGA3ProblemBase * CIndividual::target_problem_ = 0;
// ----------------------------------------------------------------------
CIndividual::CIndividual(std::size_t num_vars, std::size_t num_objs):
	variables_(num_vars), 
	objectives_(num_objs),
	converted_objectives_(num_objs)
{
	if (target_problem_ != 0)
	{
		variables_.resize(target_problem_->num_variables());
		objectives_.resize(target_problem_->num_objectives());
		converted_objectives_.resize(target_problem_->num_objectives());
	}
}
// ----------------------------------------------------------------------
const NSGA3ProblemBase & CIndividual::TargetProblem() { return *target_problem_; }
// ----------------------------------------------------------------------
std::ostream & operator << (std::ostream &os, const CIndividual &indv)
{
	for (size_t i=0; i<indv.vars().size(); i+=1)
	{
		os << indv.vars()[i] << ' ';
	}

	os << " => ";
	for (size_t f=0; f<indv.objs().size(); f+=1)
	{
		os << indv.objs()[f] << ' ';
	}

	return os;
}
// ----------------------------------------------------------------------

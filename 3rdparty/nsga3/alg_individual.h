#ifndef INDIVIDUAL__
#define INDIVIDUAL__

#include <vector>
#include <ostream>

// ----------------------------------------------------------------------
//		CIndividual
// ----------------------------------------------------------------------

class NSGA3ProblemBase;

class CIndividual
{
public:
	typedef double TGene;
	typedef std::vector<TGene> TDecVec;
	typedef std::vector<double> TObjVec;

	explicit CIndividual(std::size_t num_vars = 0, std::size_t num_objs = 0);

	TDecVec & vars() { return variables_; }
	const TDecVec & vars() const { return variables_; }

	TObjVec & objs() { return objectives_; }
	const TObjVec & objs() const { return objectives_; }

	TObjVec & conv_objs() { return converted_objectives_; }
	const TObjVec & conv_objs() const { return converted_objectives_; }

	// if a target problem is set, memory will be allocated accordingly in the constructor
	static void SetTargetProblem(const NSGA3ProblemBase &p) { target_problem_ = &p; }
	static const NSGA3ProblemBase & TargetProblem();

private:
	TDecVec variables_;
	TObjVec objectives_;
	TObjVec converted_objectives_;

	static const NSGA3ProblemBase *target_problem_;
};

std::ostream & operator << (std::ostream &os, const CIndividual &indv);

#endif

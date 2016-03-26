#ifndef BASE_PROBLEM__
#define BASE_PROBLEM__

#include <string>
#include <vector>

// ----------------------------------------------------------------------
//		BProblem: the base class of problems (e.g. ZDT and DTLZ)
// ----------------------------------------------------------------------
class CIndividual;

class NSGA3ProblemBase
{
public:
    explicit NSGA3ProblemBase(const std::string &name):name_(name) {}
    virtual ~NSGA3ProblemBase() {}

	virtual std::size_t num_variables() const = 0;
	virtual std::size_t num_objectives() const = 0;
	virtual bool Evaluate(CIndividual *indv) const = 0;

	const std::string & name() const { return name_; }
	const std::vector<double> & lower_bounds() const { return lbs_; }
	const std::vector<double> & upper_bounds() const { return ubs_; }

protected:
	std::string name_;

	std::vector<double> lbs_, // lower bounds of variables 
		                ubs_; // upper bounds of variables
};

#endif

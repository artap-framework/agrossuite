#ifndef INITIALIZATION__
#define INITIALIZATION__


// ----------------------------------------------------------------------
//		CRandomInitialization
// ----------------------------------------------------------------------

class CIndividual;
class CPopulation;
class NSGA3ProblemBase;

class CRandomInitialization
{
public:
	void operator()(CPopulation *pop, const NSGA3ProblemBase &prob) const;
	void operator()(CIndividual *indv, const NSGA3ProblemBase &prob) const;
};

extern CRandomInitialization RandomInitialization;

#endif
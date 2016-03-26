#ifndef POPULATION__
#define POPULATION__

#include "alg_individual.h"

#include <vector>

class CPopulation
{
public:
    CPopulation(std::size_t s = 0) : individuals_(std::vector<CIndividual>(s)) {}

	CIndividual & operator[](std::size_t i) { return individuals_[i]; }
	const CIndividual & operator[](std::size_t i) const { return individuals_[i]; }

	std::size_t size() const { return individuals_.size(); }
	void resize(size_t t) { individuals_.resize(t); }
	void push_back(const CIndividual &indv) { individuals_.push_back(indv); }
	void clear() { individuals_.clear(); }

private:
	std::vector<CIndividual> individuals_;
};

#endif

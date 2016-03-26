#ifndef COMPARATOR__
#define COMPARATOR__

class CIndividual;

// ----------------------------------------------------------------------------------
//			BComparator : the base class of comparison operators
// ----------------------------------------------------------------------------------
class BComparator
{
public:
	virtual ~BComparator() {}

	virtual bool	operator()(const CIndividual &l, const CIndividual &r) const = 0;
};



// ----------------------------------------------------------------------------------
//			CParetoDominate
// ----------------------------------------------------------------------------------

class CParetoDominate : public BComparator 
{
public:
	virtual	bool	operator()(const CIndividual &l, const CIndividual &r) const;
};


extern CParetoDominate ParetoDominate;
#endif
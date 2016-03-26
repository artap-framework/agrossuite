#ifndef MUTATION__
#define MUTATION__

// ----------------------------------------------------------------------------------
//		CPolynomialMutation : polynomial mutation
// ----------------------------------------------------------------------------------

class CIndividual;

class CPolynomialMutation
{
public:
	explicit CPolynomialMutation(double mr = 0.0, double eta = 20):mr_(mr), eta_(eta) {}

	void SetMutationRate(double mr) { mr_ = mr; }
	double MutationRate() const { return mr_; }
	void SetDistributionIndex(double eta) { eta_ = eta; }
	double DistributionIndex() const { return eta_; }

	bool operator()(CIndividual *c, double mr, double eta) const;
	bool operator()(CIndividual *c) const
	{
		return operator()(c, mr_, eta_);
	}

private:
	double mr_, // mutation rate
		   eta_; // distribution index
};

#endif
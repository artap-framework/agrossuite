#ifndef REFERENCE_POINT__
#define REFERENCE_POINT__

#include <vector>
#include <utility>
#include "alg_nondominated_sort.h"

// ----------------------------------------------------------------------------------
//		CReferencePoint
//
// Reference points play very important roles in NSGA-III. Individuals in the population
// are associated with reference points, and the survivors in the environmental selection
// are determined based on the niche count of the reference points.
//
// Check Algorithms 1-4 in the orignal paper for the usage of reference points.
// ----------------------------------------------------------------------------------

class CReferencePoint
{
public:
	explicit CReferencePoint(std::size_t s):position_(s), member_size_(0) {}

	const std::vector<double> & pos() const { return position_; }
	std::vector<double> & pos() { return position_; }

	std::size_t MemberSize() const { return member_size_; }
	bool HasPotentialMember() const { return !potential_members_.empty(); }
	void clear();
	void AddMember();
	void AddPotentialMember(std::size_t member_ind, double distance);
	int FindClosestMember() const;
	int RandomMember() const;
	void RemovePotentialMember(std::size_t member_ind);

private:
	std::vector<double> position_;

	// pair<indices of individuals in the population, distance>
	// note. only the data of individuals in the last considered front
	// will be stored.
	std::vector< std::pair<std::size_t, double> > potential_members_; 
	std::size_t member_size_; 
};

// ----------------------------------------------------------------------------------
// GenerateReferencePoints():
//
// Given the number of objectives (M) and the number of divisions (p), generate the set of 
// reference points. Check Section IV-B and equation (3) in the original paper.

void GenerateReferencePoints(std::vector<CReferencePoint> *rps, std::size_t M, const std::vector<std::size_t> &p);
// ----------------------------------------------------------------------------------
// Associate():
//
// Associate individuals in the population with reference points.
// Check Algorithm 3 in the original paper.
class CPopulation;
void Associate(std::vector<CReferencePoint> *prps, const CPopulation &pop, const CNondominatedSort::TFronts &fronts);
// ----------------------------------------------------------------------------------

#endif
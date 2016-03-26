#include "alg_comparator.h"
#include "alg_nondominated_sort.h"
#include "alg_population.h"

using namespace std;

CNondominatedSort NondominatedSort(ParetoDominate);
// ----------------------------------------------------------------------

std::vector< CNondominatedSort::TFrontMembers > CNondominatedSort::operator()(const CPopulation &pop) const
{
	CNondominatedSort::TFronts fronts;
	size_t num_assigned_individuals = 0;
	size_t rank = 1;
	vector<size_t> indv_ranks(pop.size(), 0);

	while (num_assigned_individuals < pop.size())
	{
		CNondominatedSort::TFrontMembers cur_front;

		for (size_t i=0; i<pop.size(); i+=1)
		{
			if (indv_ranks[i] > 0) continue; // already assigned a rank

			bool be_dominated = false;
			for (size_t j=0; j<cur_front.size(); j+=1)
			{
				if ( dominate(pop[ cur_front[j] ], pop[i]) ) // i is dominated
				{
					be_dominated = true;
					break;
				}
				else if ( dominate(pop[i], pop[ cur_front[j] ]) ) // i dominates a member in the current front
				{
					cur_front.erase(cur_front.begin()+j);
					j -= 1;
				}
			}
			if (!be_dominated)
			{
				cur_front.push_back(i);
			}
		}

		for (size_t i=0; i<cur_front.size(); i+=1)
		{
			indv_ranks[ cur_front[i] ] = rank;
		}
		fronts.push_back(cur_front);
		num_assigned_individuals += cur_front.size();
		
		rank += 1;
	}

	return fronts;

}// CNondominatedSort::operator()
// ----------------------------------------------------------------------
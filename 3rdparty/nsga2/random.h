#ifndef _RANDOM_GEN_H_
#define _RANDOM_GEN_H_

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace nsga2 {

class random_gen {
public:
    random_gen(uint32_t seed=0);
    virtual ~random_gen();
    
    double realu();
    double real(double low, double high);
    int integer(int low,       int high);
    
    void set_seed(uint32_t seed);
    int get_seed() const;
    
private:
    int seed;
    // generator of numbers from boost
    boost::random::mt19937 gen;
    // distributions
    boost::random::uniform_01<double> u01d;
    boost::random::uniform_int_distribution<int> uintd;
};
}

#endif /* _RANDOM_GEN_H_ */

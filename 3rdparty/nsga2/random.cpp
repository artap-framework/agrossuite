#include "nsga2/random.h"

//#include <iostream>

using namespace nsga2;

random_gen rgen; // global common random generator

random_gen::random_gen(uint32_t s) {
    seed = s;
    gen.seed(seed);
} 

random_gen::~random_gen() {
}

double random_gen::realu() {
    return u01d(gen);
}

double random_gen::real(double low, double high) { 
    return (low + (high-low)*realu());
}

int random_gen::integer(int low,       int high) { 
    uintd.param(boost::random::uniform_int_distribution<int>::param_type(low,high));
    int r = uintd(gen);
    // std::cout << "Random integer between " << low
    // 	      << " and " << high << ": " << r << std::endl;
    return r;
}

void random_gen::set_seed(uint32_t s) {
    seed = s;
    gen.seed(seed);
}

int random_gen::get_seed() const {
    return seed;
}

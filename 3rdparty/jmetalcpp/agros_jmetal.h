#ifndef __AGROS_JMETAL__
#define __AGROS_JMETAL__

#include <vector>

class JMetalSolver
{
public:
    JMetalSolver();
    ~JMetalSolver();

    enum Algorithm
    {
        OMOPSO,
        SMPSO,
        SMPSOhv,
        NSGAII,
        ssNSGAII,
        GDE3,
        paes,
        SMSEMOA,
        FastSMSEMOA
    };

    // initialization
    int numberOfVariables;
    int numberOfObjectives;
    double *lowerLimit;
    double *upperLimit;

    // parameters
    virtual JMetalSolver::Algorithm algorithm() = 0;
    virtual int populationSize() = 0;
    virtual int archiveSize() = 0;
    virtual int maxEvaluations() = 0;

    virtual double crossoverProbability() = 0;
    virtual double crossoverDistributionIndex() = 0;
    virtual double crossoverWeightParameter() = 0;

    virtual double mutationProbability() = 0;
    virtual double mutationPerturbation() = 0;
    virtual double mutationDistributionIndex() = 0;

    virtual void evaluate(std::vector<double> &x, std::vector<double> &of) = 0;
    void run();
};

#endif

#include "agros_jmetal.h"

#include <Problem.h>
#include <Solution.h>
#include <SBXCrossover.h>
#include <DifferentialEvolutionCrossover.h>
#include <DifferentialEvolutionSelection.h>
#include <RandomSelection.h>
#include <PolynomialMutation.h>
#include <NonUniformMutation.h>
#include <UniformMutation.h>
#include <BinaryTournament2.h>
#include <QualityIndicator.h>

#include <ssNSGAII.h>
#include <NSGAII.h>
#include <OMOPSO.h>
#include <SMPSO.h>
#include <SMPSOhv.h>
#include <paes.h>
#include <GDE3.h>
#include <MOEAD.h>
#include <SMSEMOA.h>
#include <FastSMSEMOA.h>

#include <iostream>
#include <map>
#include <assert.h>
#include <string.h>
#include <time.h>

class JMetalProblem : public Problem
{
public:
    JMetalProblem(JMetalSolver *solver);
    virtual ~JMetalProblem();

    void evaluate(Solution *solution);

    virtual void createAlgorithm() = 0;

    virtual void executeOptimization();

    inline SolutionSet *solutioSet() const { return m_solutioSet; }
    inline Algorithm *algorithm() const { return m_algorithm; }

    inline double executionTime() const { return m_executionTime; }
    inline int numberOfExecutions() const { return m_numberOfExecutions; }

protected:
    JMetalSolver *m_solver;

    std::vector<double> evaluationX;
    std::vector<double> evaluationOF;

    SolutionSet *m_solutioSet;
    Algorithm *m_algorithm;

    double m_executionTime;

    int m_numberOfExecutions;
    std::map<std::string, void *> m_parameters;

    Crossover *crossover;
    Mutation *mutation;
    Selection *selection;

    // parameters
    int populationSize;
    int archiveSize;
    int maxEvaluations;

    double crossoverProbability;
    double crossoverDistributionIndex;
    double crossoverWeightParameter;

    double mutationProbability;
    double mutationPerturbation;
    double mutationDistributionIndex;
};

JMetalProblem::JMetalProblem(JMetalSolver *solver) : Problem(),
    m_algorithm(nullptr), m_solutioSet(nullptr), crossover(nullptr), mutation(nullptr), selection(nullptr), m_numberOfExecutions(0)
{
    m_solver = solver;

    numberOfVariables_ = solver->numberOfVariables;
    numberOfObjectives_ = solver->numberOfObjectives;
    numberOfConstraints_ = 0;
    problemName_ = "JMetal";

    solutionType_ = new RealSolutionType(this);

    lowerLimit_ = solver->lowerLimit;
    upperLimit_ = solver->upperLimit;

    evaluationX.resize(numberOfVariables_);
    evaluationOF.resize(numberOfObjectives_);

    populationSize = solver->populationSize();
    archiveSize = solver->archiveSize();
    maxEvaluations = solver->maxEvaluations();

    crossoverProbability = solver->crossoverProbability();
    crossoverDistributionIndex = solver->crossoverDistributionIndex();
    crossoverWeightParameter = solver->crossoverWeightParameter();

    mutationProbability = solver->mutationProbability();
    mutationPerturbation = solver->mutationPerturbation();
    mutationDistributionIndex = solver->mutationDistributionIndex();
}

JMetalProblem::~JMetalProblem()
{
    if (m_algorithm)
    {
        delete m_algorithm;
        m_algorithm = nullptr;
    }

    if (m_solutioSet)
    {
        delete m_solutioSet;
        m_solutioSet = nullptr;
    }

    delete solutionType_;
    solutionType_ = nullptr;

    if (crossover)
    {
        delete crossover;
        crossover = nullptr;
    }
    if (mutation)
    {
        delete mutation;
        mutation = nullptr;
    }
    if (selection)
    {
        delete selection;
        selection = nullptr;
    }
}

void JMetalProblem::evaluate(Solution *solution)
{
    m_numberOfExecutions++;

    // std::cout << "getLocation = " << solution->getLocation() << ", getRank = " << solution->getRank() << ", executions = " << m_numberOfExecutions << std::endl;
    // << ", iteration = " << dynamic_cast<ssNSGAII *>(m_algorithm)->iteration << std::endl;

    // variables
    Variable **variables = solution->getDecisionVariables();
    for (int i = 0; i < numberOfVariables_; i++)
        evaluationX[i] = variables[i]->getValue();

    // objectives
    for (int i = 0; i < numberOfObjectives_; i++)
        evaluationOF[i] = 0;

    // evaluate
    m_solver->evaluate(evaluationX, evaluationOF);

    // set objectives
    for (int i = 0; i < numberOfObjectives_; i++)
        solution->setObjective(i, evaluationOF[i]);

    // double x = solution->getDecisionVariables()[0]->getValue();
    // double y = solution->getDecisionVariables()[1]->getValue();
    // double OF = (1. - x)*(1. - x) + (y - x*x)*(y - x*x)*100;
    // solution->setObjective(0, OF);
}

void JMetalProblem::executeOptimization()
{
    // Execute the Algorithm
    clock_t t_ini = clock();
    m_solutioSet = m_algorithm->execute();

    clock_t t_fin = clock();
    m_executionTime = (double) (t_fin - t_ini);
    m_executionTime = m_executionTime / CLOCKS_PER_SEC;
}

// ********************************************************************************************************

class StudyJMetalOMOPSO : public JMetalProblem
{
public:
    StudyJMetalOMOPSO(JMetalSolver *solver)
        : JMetalProblem(solver), uniformMutation(nullptr), nonUniformMutation(nullptr)
    {
    }

    virtual ~StudyJMetalOMOPSO()
    {
        if (uniformMutation)
        {
            delete uniformMutation;
            uniformMutation = nullptr;
        }
        if (nonUniformMutation)
        {
            delete nonUniformMutation;
            nonUniformMutation = nullptr;
        }
    }

    virtual void createAlgorithm()
    {
        m_algorithm = new OMOPSO(this);

        // Algorithm parameters
        m_algorithm->setInputParameter("swarmSize", &populationSize);
        m_algorithm->setInputParameter("archiveSize", &archiveSize);
        m_algorithm->setInputParameter("maxIterations", &maxEvaluations);

        m_parameters.clear();
        m_parameters["probability"] =  &mutationProbability;
        m_parameters["perturbation"] = &mutationPerturbation;
        uniformMutation = new UniformMutation(m_parameters);

        m_parameters.clear();
        m_parameters["probability"] =  &mutationProbability;
        m_parameters["perturbation"] =  &mutationPerturbation;
        m_parameters["maxIterations"] = &maxEvaluations;
        nonUniformMutation = new NonUniformMutation(m_parameters);

        // Add the operators to the algorithm
        m_algorithm->addOperator("uniformMutation", uniformMutation);
        m_algorithm->addOperator("nonUniformMutation", nonUniformMutation);
    }

protected:
    Mutation *uniformMutation;
    Mutation *nonUniformMutation;
};

class StudyJMetalSMPSO : public JMetalProblem
{
public:
    StudyJMetalSMPSO(JMetalSolver *solver)
        : JMetalProblem(solver)
    {
    }

    virtual void createAlgorithm()
    {
        if (m_solver->algorithm() == JMetalSolver::SMPSO)
            m_algorithm = new SMPSO(this);
        else if (m_solver->algorithm() == JMetalSolver::SMPSOhv)
            m_algorithm = new SMPSOhv(this);
        else
            assert(0);

        // Algorithm parameters
        m_algorithm->setInputParameter("swarmSize", &populationSize);
        m_algorithm->setInputParameter("archiveSize", &archiveSize);
        m_algorithm->setInputParameter("maxIterations", &maxEvaluations);

        // Mutation operator
        m_parameters["probability"] = &mutationProbability;
        m_parameters["distributionIndex"] = &mutationDistributionIndex;
        mutation = new PolynomialMutation(m_parameters);

        // Add the operators to the algorithm
        m_algorithm->addOperator("mutation", mutation);
    }
};

class StudyJMetalNSGAII : public JMetalProblem
{
public:
    StudyJMetalNSGAII(JMetalSolver *solver)
        : JMetalProblem(solver)
    {
    }

    virtual void createAlgorithm()
    {
        if (m_solver->algorithm() == JMetalSolver::NSGAII)
            m_algorithm = new NSGAII(this);
        else if (m_solver->algorithm() == JMetalSolver::ssNSGAII)
            m_algorithm = new ssNSGAII(this);
        else
            assert(0);

        // Algorithm parameters
        m_algorithm->setInputParameter("populationSize", &populationSize);
        m_algorithm->setInputParameter("maxEvaluations", &maxEvaluations);

        // Mutation and Crossover for Real codification
        m_parameters.clear();
        m_parameters["probability"] =  &crossoverProbability;
        m_parameters["distributionIndex"] = &crossoverDistributionIndex;
        crossover = new SBXCrossover(m_parameters);

        m_parameters.clear();
        m_parameters["probability"] = &mutationProbability;
        m_parameters["distributionIndex"] = &mutationDistributionIndex;
        mutation = new PolynomialMutation(m_parameters);

        // Selection Operator
        m_parameters.clear();
        selection = new BinaryTournament2(m_parameters);

        // Add the operators to the algorithm
        m_algorithm->addOperator("crossover", crossover);
        m_algorithm->addOperator("mutation", mutation);
        m_algorithm->addOperator("selection", selection);
    }
};

class StudyJMetalGDE3 : public JMetalProblem
{
public:
    StudyJMetalGDE3(JMetalSolver *solver)
        : JMetalProblem(solver)
    {
    }

    virtual void createAlgorithm()
    {
        m_algorithm = new GDE3(this);

        // Algorithm parameters
        m_algorithm->setInputParameter("populationSize", &populationSize);
        m_algorithm->setInputParameter("maxIterations", &maxEvaluations);

        // Crossover operator
        m_parameters.clear();
        m_parameters["CR"] =  &crossoverProbability;
        m_parameters["F"] = &crossoverWeightParameter;
        crossover = new DifferentialEvolutionCrossover(m_parameters);

        // Selection operator
        m_parameters.clear();
        selection = new DifferentialEvolutionSelection(m_parameters) ;

        // Add the operators to the algorithm
        m_algorithm->addOperator("crossover", crossover);
        m_algorithm->addOperator("selection", selection);
    }
};

class StudyJMetalPAES : public JMetalProblem
{
public:
    StudyJMetalPAES(JMetalSolver *solver)
        : JMetalProblem(solver)
    {
    }

    virtual void createAlgorithm()
    {
        m_algorithm = new paes(this);

        biSections = 5;

        m_algorithm->setInputParameter("archiveSize", &archiveSize);
        m_algorithm->setInputParameter("biSections", &biSections);
        m_algorithm->setInputParameter("maxEvaluations", &maxEvaluations);

        m_parameters.clear();
        m_parameters["probability"] =  &mutationProbability;
        m_parameters["distributionIndex"] = &mutationDistributionIndex;
        mutation = new PolynomialMutation(m_parameters);
        m_algorithm->addOperator("mutation", mutation);
    }

protected:
    int biSections;
};

class StudyJMetalMOEAD : public JMetalProblem
{
public:
    StudyJMetalMOEAD(JMetalSolver *solver)
        : JMetalProblem(solver)
    {
    }

    virtual void createAlgorithm()
    {
        m_algorithm = new MOEAD(this);

        // crParameter = 1.0;

        // Algorithm parameters
        m_parameters.clear();
        m_algorithm->setInputParameter("populationSize", &populationSize);
        m_algorithm->setInputParameter("maxEvaluations", &maxEvaluations);

        // Directory with the files containing the weight vectors used in
        // Q. Zhang,  W. Liu,  and H Li, The Performance of a New Version of MOEA/D
        // on CEC09 Unconstrained MOP Test Instances Working Report CES-491, School
        // of CS & EE, University of Essex, 02/2009.
        // http://dces.essex.ac.uk/staff/qzhang/MOEAcompetition/CEC09final/code/ZhangMOEADcode/moead0305.rar
        dataDirectory = "metaheuristics/moead/Weight";
        m_algorithm->setInputParameter("dataDirectory", &dataDirectory);

        m_parameters.clear();
        m_parameters["probability"] =  &mutationProbability;
        m_parameters["distributionIndex"] = &mutationDistributionIndex;
        mutation = new UniformMutation(m_parameters);

        // Crossover operator
        m_parameters.clear();
        m_parameters["CR"] =  &crossoverProbability;
        m_parameters["F"] = &crossoverWeightParameter;
        crossover = new DifferentialEvolutionCrossover(m_parameters);

        // Add the operators to the algorithm
        m_algorithm->addOperator("crossover", crossover);
        m_algorithm->addOperator("mutation", mutation);
    }

protected:
    string dataDirectory;
};

class StudyJMetalSMSEMOA : public JMetalProblem
{
public:
    StudyJMetalSMSEMOA(JMetalSolver *solver)
        : JMetalProblem(solver)
    {
    }

    virtual void createAlgorithm()
    {
        offset = 100;

        if (m_solver->algorithm() == JMetalSolver::SMSEMOA)
            m_algorithm = new SMSEMOA(this);
        else if (m_solver->algorithm() == JMetalSolver::FastSMSEMOA)
            m_algorithm = new FastSMSEMOA(this);
        else
            assert(0);

        m_algorithm->setInputParameter("populationSize", &populationSize);
        m_algorithm->setInputParameter("maxEvaluations", &maxEvaluations);
        m_algorithm->setInputParameter("offset", &offset);

        // Mutation
        m_parameters.clear();
        m_parameters["probability"] =  &crossoverProbability;
        m_parameters["distributionIndex"] = &crossoverDistributionIndex;
        crossover = new SBXCrossover(m_parameters);

        //Crossover
        m_parameters.clear();
        m_parameters["probability"] = &mutationProbability;
        m_parameters["distributionIndex"] = &mutationDistributionIndex;
        mutation = new PolynomialMutation(m_parameters);

        // Selection Operator
        m_parameters.clear();
        selection = new RandomSelection(m_parameters);

        // Add the operators to the algorithm
        m_algorithm->addOperator("crossover", crossover);
        m_algorithm->addOperator("mutation", mutation);
        m_algorithm->addOperator("selection", selection);
    }

protected:
    double offset;
};

// *******************************************************************************************

JMetalSolver::JMetalSolver()
{
}

JMetalSolver::~JMetalSolver()
{
    if (lowerLimit)
        delete [] lowerLimit;
    if (upperLimit)
        delete [] upperLimit;
}

void JMetalSolver::run()
{
    JMetalProblem *problem = nullptr;
    switch (algorithm())
    {
    case OMOPSO:
        problem = new StudyJMetalOMOPSO(this);
        break;
    case SMPSO:
    case SMPSOhv:
        problem = new StudyJMetalSMPSO(this);
        break;
    case NSGAII:
    case ssNSGAII:
        problem = new StudyJMetalNSGAII(this);
        break;
    case GDE3:
        problem = new StudyJMetalGDE3(this);
        break;
    case paes:
        problem = new StudyJMetalPAES(this);
        break;
    case SMSEMOA:
    case FastSMSEMOA:
        problem = new StudyJMetalSMSEMOA(this);
        break;
    default:
        break;
    }

    assert(problem);

    problem->createAlgorithm();
    problem->executeOptimization();

    // Print the results
    cout << "Total execution time: " << problem->executionTime() << "s, size = " << problem->solutioSet()->size() <<
            ", executions = " <<  problem->numberOfExecutions() << endl;
    problem->solutioSet()->printObjectives();

    // delete problem;
}

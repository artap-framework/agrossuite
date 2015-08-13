from variant.optimization import Optimization
from variant.genetic.population_creator import ImplicitPopulationCreator
from variant.genetic.selector import ImplicitSingleCriteriaSelector, ImplicitMultiCriteriaSelector
from variant.genetic.mutation import ImplicitMutation
from variant.genetic.crossover import ImplicitCrossover

from time import time

class Genetic(Optimization):
    """Genetic optimization method class."""

    def __init__(self, model_class, parameters, functionals):
        """Initialization of genetic optimization object."""

        Optimization.__init__(self, model_class, parameters, functionals)

        self._current_population_index = 0
        self._cache = list()

        self.population_size = len(self.parameters.parameters)*10

        self.selection_ratio = 4.0/5.0
        self.elitism_ratio = 1.0/10.0
        self.crossover_ratio = 6.0/5.0

        self.mutation_probability = 1e-3
        self.mutation_ratio = 1.0/10.0

        self.population_creator = ImplicitPopulationCreator(self.parameters, self.model_class)

        if not self.functionals.multicriteria():
            self.selector = ImplicitSingleCriteriaSelector(self.functionals)
        else:
            self.selector = ImplicitMultiCriteriaSelector(self.functionals)

        self.crossover = ImplicitCrossover(self.model_class)
        self.mutation = ImplicitMutation(self.parameters)

    def _functional_as_key(self, individual):
        return self.functionals.evaluate(individual)

    def population(self, population=None, sorted=False):
        """Find and return population (list of individuals).
        
        Args:
            population: Population number (default means last population).
        """

        individuals = []
        if population == None:
            individuals = self._cache
        else:
            for model in self.model_dictionary.models():
                if model.info['population'] == population:
                    individuals.append(model)

        if not sorted:
            return individuals
        else:
            direction = self.functionals.functional().direction_sign()
            return sorted(population, key=self._functional_as_key, reverse=bool(direction != 1))

    def create_population(self):
        """Create new population."""

        if bool(self._current_population_index):
            # selection
            last_population = self.population()
            selections = min(int(self.selection_ratio * self.population_size),
                                len(last_population))
            selected = self.selector.select(last_population, selections)
            self.record('selections', selections)
    
            # elitism
            """
            elitists = max(int(self.elitism_ratio * self.population_size), 1)
            elite = sorted(selected, key=self._functional_as_key,
                            reverse=bool(self.functionals.functional().direction_sign() != 1))[0:elitists]
            self.record('elitists', elitists)
            """

            # crossover
            crossbreeds = max(int(self.crossover_ratio * (selections )),#+ elitists
                                int(self.population_size/2))
            descendants = self.crossover.cross(selected, crossbreeds)
            self.record('crossbreeds', crossbreeds)
    
            # mutation
            descendants = self.mutation.mutate(descendants, self.mutation_probability, self.mutation_ratio)
    
            for individual in descendants:
                individual.clear()

            population = descendants #+ elite
        else:
            population = self.population_creator.create(self.population_size)

        # preconditioning
        """
        minimum_population_size = len(population)
        create = self.population_creator.create
    
        iterations = 100
        number = 0
        unsuitables = 0
        for i in range(iterations):
            population_size = len(population)
            population += create(int(minimum_individuals-population_size))
            population = self.preconditioning(population)
            number += population_size - len(population)
            unsuitables += number

            if (len(population) >= minimum_individuals or
                number >= self.population_size):
                clear = False

            self.record('unsuitables', unsuitables)
        """
        return population

    def optimize(self, populations, resume=False,
            save=False, data_file=None, problem_file=None):

        """Run optimization.

        Args:
            populations: Expected number of populations.
            resume: Resume in optimization.
            save: Save models after solution (default is True).
            data_file: Name of dictionary data file (standard extension is .opt).
            problem_file: Name of Python script with model class definition.
        """

        """
        if (resume and not data_file):
            raise RuntimeError('Data file is not defined.')
        """

        if resume:
            self.resume(data_file)
            if (self.sensitivity and (len(self.model_dictionary.models()) >= self.sensitivity_individuals)):
                self.sensitivity_analysis()

        if ((save and not data_file) or (save and not problem_file)):
            raise RuntimeError('Data file and problem file is not defined.')

        for index in range(self._current_population_index, populations):
            start = time()
            self.record_attributes()

            self._current_population_index = index
            population = self.create_population()
            self.record('individuals', len(population))
            last_model_index = len(self.model_dictionary.models())
            for individual in population:
                individual.info['population'] = index
                self.model_dictionary.add_model(individual)

            # TODO: Save dictionary during solution.
            self.model_dictionary.solve()
            if save:
                self.model_dictionary.save(data_file, problem_file)
            self._cache = self.model_dictionary.models()[last_model_index:]

            stop = False
            if (self.sensitivity and (len(self.model_dictionary.models()) >= self.sensitivity_individuals)):
                self.sensitivity_analysis()
                if sum(self.model_dictionary.description['dulls']) >= len(self.parameters.parameters):
                    self.record(message='Nothing for optimization!')
                    stop = True

            if self.criterions.validate(self.find_best(self.population())):
                self.record(message='Stopping criterion reached.')
                stop = True

            self.record('time', time()-start)
            if stop: break

    def resume(self, data_file):
        if (data_file and not self.model_dictionary.models()):
            self.model_dictionary.load(data_file)

        if self.model_dictionary.models():
            last_population = 0
            for model in self.model_dictionary.models():
                population = model.info['population']
                if population > last_population:
                    last_population = population

            for model in self.model_dictionary.models():
                if model.info['population'] == last_population:
                    if not model.solved():
                        model.solved()
                    self._cache.append(model)

            self._current_population_index = last_population + 1

if __name__ == '__main__':
    from variant.test_functions import HolderTableFunction
    from variant.parameter import Parameters, ContinuousParameter
    from variant.functional import Functionals, Functional
    from variant.criterion import ValueCriterion

    import pythonlab

    # optimization parameters
    parameters = Parameters([ContinuousParameter('x', -10, 10),
                             ContinuousParameter('y', -10, 10)])

    functionals = Functionals([Functional("F", "min")])
    genetic = Genetic(HolderTableFunction, parameters, functionals)

    # stopping criterion
    #genetic.criterions.add_criterion(ValueCriterion('Exact soluton'), [Functional('F'), -19.2085, 1])

    # sensitivity analysis
    genetic.sensitivity = True
    genetic.sensitivity_threshold = 0.01
    genetic.sensitivity_individuals = 500

    data_file = 'holder_table_function.opt'
    problem_file = '{0}resources/python/variant/test_functions/holder_table_function.py'.format(pythonlab.datadir())

    genetic.population_size = 100
    genetic.optimize(10, resume=False, save=True,
                     data_file=data_file, problem_file=problem_file)

    superstar = genetic.find_best()
    print('Fmin = {0}, parameters: {1}'.format(superstar.variables['F'], superstar.parameters))

    F = []
    population = []
    for individual in genetic.model_dictionary.models():
        F.append(individual.variables['F'])
        population.append(individual.info['population'])
    pythonlab.chart(population, F, '# population', 'F')
from copy import deepcopy
import random as rnd

class Crossover:
    """General class for individuals crossovering."""
    def __init__(self, model_class):
        """Initialization of crossover.

        Args:
            model_class: Class described optimized model.
        """

        self.model_class = model_class

    def cross(self, population):
        """Return list of individuals gained by crossing.

        Args:
            population: List of individuals.
        """
        pass

class ImplicitCrossover(Crossover):
    """Implicit crossover generator.

    Crossbreeds are created equally from mother and father.
    """

    def cross(self, population, number):
        """ Create new crossbreeds.

        Args:
            population: List of individuals.
            number: Number of new individuals creates by crossing.
        """

        indices = []
        for index in range(len(population)):
            if 'priority' in population[index].info:
                indices += [index] * population[index].info['priority']
            else:
                indices += [index]

        crossbreeds = []
        while len(crossbreeds) < number:
            mother = population[rnd.choice(indices)]
            father = population[rnd.choice(indices)]

            while mother == father:
                father = population[rnd.choice(indices)]

            son = self.model_class()
            son._data = deepcopy(mother._data)
            mother_gens = dict(mother.parameters.items())
            father_gens = {}

            while len(mother_gens) > len(father_gens):
                key = rnd.choice(list(mother_gens.keys()))
                son.parameters[key] = father.parameters[key]

                father_gens[key] = father.parameters[key]
                del(mother_gens[key])

            #son.info['mother'] = mother.info['index']
            #son.info['father'] = father.info['index']
            crossbreeds.append(son)
        return crossbreeds

if __name__ == '__main__':
    from variant.test_functions.quadratic_function import QuadraticFunction
    from variant.dictionary import ModelDictionary

    md = ModelDictionary()
    variants = range(1, 10)
    for x in variants:
        model = QuadraticFunction()
        model.parameters['x'] = x
        model.parameters['a'] = x/2.0
        model.parameters['b'] = x/3.0
        model.parameters['c'] = 2*x
        model.info['priority'] = 1
        md.add_model(model)

    md.solve(save=False)

    crossover = ImplicitCrossover(QuadraticFunction)
    crossbreeds = crossover.cross(md.models(), 2)
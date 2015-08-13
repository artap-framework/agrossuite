import random as rnd

class Mutation:
    """General class for individuals mutation."""

    def __init__(self, parameters):
        """Initialization of mutation creator.

        Args:
            parameters: Optimized parameters.
        """
        self._parameters = parameters

    def mutate(self, population):
        """Return list of individual with mutated parameters.

        Args:
            population: List of individuals.
        """
        pass

class ImplicitMutation(Mutation):
    """Implicit individuals mutation.

    All model parameters are mutated with defined probability and
    its values are changed within +-ratio*(max - min).
    """

    def mutate(self, population, probability, ratio):
        """Return list of individual with mutated parameters.

        Args:
            population: List of individuals.
            probability: Probability of parameter mutation.
            ratio: Maximum ration for parameter change.
        """

        for individual in population:
            for name, parameter in self._parameters.parameters.items():
                if not (rnd.random() <= probability):
                    continue

                original = individual.parameters[name]
                individual.parameters[name] = parameter.perturbation(original, ratio)

        return population

if __name__ == '__main__':
    from variant.test_functions.quadratic_function import QuadraticFunction

    from variant.dictionary import ModelDictionary
    from variant.parameter import Parameters, ContinuousParameter

    md = ModelDictionary()
    variants = range(1, 100)
    for x in variants:
        model = QuadraticFunction()
        model.parameters['x'] = x
        md.add_model(model)

    md.solve(save=False)

    parameters = Parameters([ContinuousParameter('x', 1, 100)])
    mutation = ImplicitMutation(parameters)
    mutants = mutation.mutate(md.models(), 0.05, 0.1)
class PopulationCreator:
    """General class for population creator."""

    def __init__(self, parameters, model_class):
        """Initialization of population creator.
        
        Args:
            parameters: Optimization parameters.
            model_class: Model class inherited from ModelBase.
        """

        self._parameters = parameters
        self._model_class = model_class

    def create(self, number):
        """Return new population as list of individuals.

        Args:
            number: Number of individuals in population.
        """
        pass

class ImplicitPopulationCreator(PopulationCreator):
    """Implicit population creator.
    
    All model parameters are taken randomly.
    """

    def create(self, number):
        population = []
        for index in range(number):
            individual = self._model_class()
            for parameter in self._parameters.parameters.values():
                individual.parameters[parameter.name] = parameter.random_value()
            population.append(individual)

        return population
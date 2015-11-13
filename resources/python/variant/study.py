from variant.dictionary import ModelDictionary
from variant.parameter import Parameters
from variant.functional import Functionals
from variant.criterion import Criterions
from variant.calculator import Calculators

from variant.calculator import SpearmanRankCorrelation

import logging
logging.basicConfig(format='%(asctime)s: %(name)s: %(message)s',
                    datefmt='%H:%M:%S', level=logging.INFO)

class Study:
    """General class for study."""

    def __init__(self, model_class, parameters=None, functionals=None,
                 calculators=None, criterions=None):
        """Initialization of study."""

        self._model_class = model_class
        self.model_dictionary = ModelDictionary()

        self._parameters = Parameters()
        if parameters: self.parameters = parameters

        self._functionals = Functionals()
        if functionals: self.functionals = functionals

        self._calculators = Calculators()
        if calculators: self.calculators = calculators

        self._calculators = Criterions()
        if criterions: self.criterions = criterions

    @property
    def model_class(self):
        """Model class."""
        return self._model_class

    @model_class.setter
    def model_class(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def parameters(self):
        """Analysed parameters (instance of Functionals class)."""
        return self._parameters

    @parameters.setter
    def parameters(self, value):
        if isinstance(value, Parameters):
            self._parameters = value
        else:
            raise TypeError('Parameters must be instance of Parameters class.')

    @property
    def functionals(self):
        """Analysed functionals (instance of Functionals class)."""
        return self._functionals

    @functionals.setter
    def functionals(self, value):
        if isinstance(value, Functionals):
            self._functionals = value
        else:
            raise TypeError('Functionals must be instance of Functionals class.')

    @property
    def calculators(self):
        """Calculators (instance of Calculators class)."""
        return self._calculators

    @calculators.setter
    def calculators(self, value):
        if isinstance(value, Calculators):
            self._calculators = value
        else:
            raise TypeError('Calculators must be instance of Calculators class.')

    @property
    def criterions(self):
        """Calculators (instance of Calculators class)."""
        return self._calculators

    @criterions.setter
    def criterions(self, value):
        if isinstance(value, Criterions):
            self._criterions = value
        else:
            raise TypeError('Calculators must be instance of Calculators class.')

    def record(self, key=None, value=None, message=None,
               verbose=False):

        """Method provide bassic logging functionality.

        Key and value arguments are saved to description.

        Args:
            key: Name of recorded value (default is None).
            value: Recorded value (default is None).
            message: Logging message (default is None).
            verbose: Print all recorded data.
        """

        if (key and not key in self.model_dictionary.description):
            self.model_dictionary.description[key] = list()

        if (key and value):
            self.model_dictionary.description[key].append(value)

        if not hasattr(self, 'loger'):
            self.loger = logging.getLogger(self.__class__.__name__)

        if (message and key and value):
            self.loger.info('{0}: {1} = {2}'.format(message, key, value))
        if (message and not key and not value):
            self.loger.info(message)
        if (not message and key and value and verbose):
            self.loger.info('{0} = {1}'.format(key, value))

    def record_attributes(self):
        """Save all attributes (int, float or bool) to description."""

        for name in dir(self):
            attribute = getattr(self, name)
            if (isinstance(attribute, (int, float, bool)) and
                not name.startswith('_')):
                self.record(name, attribute)

class SensitivityAnalysis(Study):
    def analyse(self, models, save=False,
                correlation_calculator=SpearmanRankCorrelation):

        """Run analysis and return dictionary with results."""

        for functional in self.functionals.functionals.values():
            for parameter in self.parameters.parameters.values():
                name = '{0}({1},{2})'.format(correlation_calculator.__name__, functional.name, parameter.name)
                if not name in self.calculators.calculators:
                    self.calculators.add_calculator(correlation_calculator(name), [functional, parameter])

        for index in range(int(models)):
            model = self._model_class()
            for parameter in self.parameters.parameters.values():
                model.parameters[parameter.name] = parameter.random_value()
            self.model_dictionary.add_model(model)

        self.model_dictionary.solve(save=save)
        results = self.calculators.calculate(self.model_dictionary.models())
        for key, value in results.items():
            self.record(key, value)

        return results

if __name__ == '__main__':
    from variant.test_functions import QuadraticFunction
    from variant.parameter import ContinuousParameter
    from variant.functional import Functional

    parameters = Parameters([ContinuousParameter('a', -10, 10),
                             ContinuousParameter('b', -10, 10),
                             ContinuousParameter('c', -10, 10),
                             ContinuousParameter('x', -10, 10)])

    functionals = Functionals([Functional('F')])
    sensitivity = SensitivityAnalysis(QuadraticFunction, parameters, functionals)
    sensitivity.analyse(500)
    print(sensitivity.model_dictionary.description)
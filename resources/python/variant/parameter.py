import random as rnd

class Parameters:
    """General class collected parameters."""

    def __init__(self, parameters=None):
        """Initialization of parameters object.
        
        Args:
            parameters: List of Parameter objects (default is None).
        """

        self._parameters = dict()
        if isinstance(parameters, list):
            for parameter in parameters:
                self.add_parameter(parameter)

    @property
    def parameters(self):
        """Return dictionary of parameters."""
        return self._parameters

    @parameters.setter
    def parameters(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    def parameter(self, name=None):
        """Return parameter with specific name.

        Args:
            name: Name of parameter (default is None).

        Returns:
            If name is None, return first parameter in directory.
        """

        if not name:
            return list(self._parameters.values())[0]
        else:
            return self._parameters[name]

    def add_parameter(self, parameter):
        """Add new parameter.

        Args:
            parameter: Model object inherited from Parameter class.
        """

        if parameter.name in self._parameters:
            raise NameError('Parameter with name "{0}" alredy exist.'.format(parameter.name))

        if (parameter.__class__.__base__ == Parameter):
            self._parameters[parameter.name] = parameter
        else:
            raise TypeError('Parameter must be instance or inherited object from Parameter class.')

    def remove_parameter(self, name):
        """Remove existing parameter from collection.

        Args:
            name: Parameter name.
        """
        del self._parameters[name]

    def multidimensional(self):
        """If two and more parameters are defined return True, otherwise return False."""

        if len(self._parameters) >= 2:
            return True
        else:
            return False


    def evaluate(self, model):
        """Evaluate parameters of model.
        
        Args:
            model: Model object inherited from ModelBase class.

        Returns:
            For one parameter returns one value,
            for more parameters dictionary.
        """

        if not self.multidimensional():
            return model.variables[self.parameter().name]
        else:
            parameters = dict()
            for parameter in self.parameters.values():
                parameters[parameter.name] = model.variables[parameter.name]

            return parameters

class Parameter:
    """General class for study parameter.

    Attributes:
        name: Criterion name.
        weights: Dictionary of parameter weigths.
    """

    def __init__(self, name):
        self.name = name
        self.weights = dict()

    def continues(self):
        """Return True if parameter is continue."""
        pass
        
    def random_value(self):
        """Return random value within bounds."""
        pass

    def perturbation(self, original, ratio):
        """Change original value within +-ratio*(max - min).

        Args:
            original: Original value.
            ration: Maximum ration for parameter value change.
        """
        pass

    def evaluate(self, model):
        """Evaluate parameter of model.
        
        Args:
            model: Model object inherited from ModelBase class.
        """
        return model.parameters[self.name]

class ContinuousParameter(Parameter):
    """Class for parameters with continue interval."""

    def __init__(self, name, minimum, maximum):
        """Initialization of parameter object.
        
        Args:
            name: Parameter name.
            minimum: Minimum of parameter value interval.
            max: maximum of parameter value interval.
        """

        Parameter.__init__(self, name)
        self.minimum = minimum
        self.maximum = maximum
        
    def continuous(self):
        return True

    def random_value(self):
        """Return random value within bounds."""
        return self.minimum + rnd.random() * (self.maximum - self.minimum)
        
    def perturbation(self, original, ratio):
        """Change original value within +-ratio*(max - min).

        Args:
            original: Original value.
            ration: Maximum ration for parameter value change.
        """

        if (self.minimum > original or  self.maximum < original):
            raise ValueError('Original must be in the parameter range from {0} to {1}.'.format(self.minimum, self.maximum))

        difference =  ratio * self.interval()
        value_min = max(self.minimum, original - difference)
        value_max = min(self.maximum, original + difference)

        return value_min + rnd.random() * (value_max - value_min)
        
    def interval(self):
        """Return interval length."""
        return self.maximum - self.minimum

class DiscreteParameter(Parameter):
    def __init__(self, name, options):
        """Initialization of parameter object.
        
        Args:
            name: Parameter name.
            options: List of parameter values.
        """

        Parameter.__init__(self, name)
        self.options = options

    def continuous(self):
        return False

    def random_value(self):
        """Return random value from options."""
        return rnd.choice(self.options)
        
    def perturbation(self, original, ratio):
        """Approximate change original value within +-ratio*(max - min).

        Args:
            original: Original value.
            ration: Maximum ration for change.
        """

        max_shift = int(len(self.options) * ratio)
        if max_shift == 0: max_shift = 1

        original_index = self.options.index(original)
        min_index = max(0, original_index - max_shift)
        max_index = min(len(self.options) - 1, original_index + max_shift)

        new_index = original_index
        while new_index == original_index:
            new_index = rnd.randrange(min_index, max_index + 1)

        return self.options[new_index]

if __name__ == '__main__':
    cp = ContinuousParameter("cp", 2, 5.2)
    dp = DiscreteParameter("dp", [4, 6, 77, 44, 99, 11])

    print([cp.random_value(), cp.random_value(), cp.random_value()])
    print([cp.perturbation(2.3, 0.2), cp.perturbation(2.3, 0.2), cp.perturbation(2.3, 0.2)])

    print([dp.random_value(), dp.random_value(), dp.random_value()])
    print([dp.perturbation(99,0.1), dp.perturbation(99,0.1), dp.perturbation(99,0.5)])
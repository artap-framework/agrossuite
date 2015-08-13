class Functionals:
    """General class collected functionals."""

    def __init__(self, functionals=None):
        """Initialization of functionals object.
        
        Args:
            functionals: List of Functional objects (default is None).
        """

        self._functionals = dict()
        if isinstance(functionals, list):
            for functional in functionals:
                self.add_functional(functional)

    @property
    def functionals(self):
        """Return dictionary of functionals."""
        return self._functionals

    @functionals.setter
    def functionals(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    def functional(self, name=None):
        """Return functional with specific name.

        Args:
            name: Name of functional (default is None).

        Returns:
            If name is None, return first functional in directory.
        """

        if not name:
            return list(self._functionals.values())[0]
        else:
            return self._functionals[name]

    def add_functional(self, functional):
        """Add new functional.

        Args:
            functional: Model object inherited from Functional class.
        """

        if functional.name in self._functionals:
            raise NameError('Functional with name "{0}" alredy exist.'.format(functional.name))

        if (functional.__class__ == Functional):
            self._functionals[functional.name] = functional
        else:
            raise TypeError('Functional must be instance of Functional class.')

    def remove_functional(self, name):
        """Remove existing functional from collection.

        Args:
            name: Functional name.
        """
        del self._functionals[name]

    def multicriteria(self):
        """If two and more functionals are defined return True, otherwise return False."""

        if len(self._functionals) >= 2:
            return True
        else:
            return False

    def evaluate(self, model):
        """Evaluate functionals of model.
        
        Args:
            model: Model object inherited from ModelBase class.

        Returns:
            For one functional returns one value,
            for more functionals dictionary.
        """

        if not self.multicriteria():
            return model.variables[self.functional().name]
        else:
            functionals = dict()
            for functional in self.functionals.values():
                functionals[functional.name] = model.variables[functional.name]

            return functionals

class Functional:
    """General class for optimization functional."""

    def __init__(self, name, direction=None):
        """Initialization of functional object.

        Args:
            name: Functional name.
            direction: Direction for optimization ("min" or "max").
        """

        self.name = name
        self._direction = str()
        self.direction = direction

    @property
    def direction(self):
        """Direction of functional ("min" or "max")."""
        return self._direction

    @direction.setter
    def direction(self, value):
        if value in ["min", "max", None]:
            self._direction = value
        else:
            raise ValueError('Direction "{0}" is not defined. Allowed values are "min" and "max".'.format(value))
        
    def direction_sign(self):
        """Return sign of functional direction."""
        if self.direction == "min":
            return 1
        elif self.direction == "max":
            return -1
        else:
            return 0

    def evaluate(self, model):
        """Evaluate functionals of model.
        
        Args:
            model: Model object inherited from ModelBase class.
        """
        return model.variables[self.name]
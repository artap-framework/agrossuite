import warnings
import inspect

def operators(operator):
    import operator as op
    return {
        '==' : op.eq,
        '!=' : op.ne,
        '>' : op.gt,
        '>=' : op.ge,
        '<' : op.lt,
        '<=' : op.le,
        '<<' : op.le,
        }[operator]

class Criterions:
    """General class collected criterions."""

    def __init__(self):
        """Initialization of criterions object."""

        self._criterions = dict()
        self._arguments = dict()

    @property
    def criterions(self):
        """Return dictionary of criterions."""
        return self._criterions

    @criterions.setter
    def criterions(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def arguments(self):
        """Return dictionary of criterions arguments."""
        return self._arguments

    @arguments.setter
    def arguments(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    def add_criterion(self, criterion, arguments):
        """Add new criterion.

        Args:
            criterion: Object inherited from Criterion class.
            arguments: List of Parameter and Functional objects used as criterion arguments.
        """

        if criterion.name in self._criterions:
            raise NameError('Criterion with name "{0}" alredy exist.'.format(criterion.name))

        specification = inspect.getargspec(criterion.validate)
        required_arguments = specification.args
        number_of_defaults = 0
        if specification.defaults:
            number_of_defaults = len(specification.defaults)

        self._arguments[criterion.name] = arguments
        """
        if len(required_arguments)-number_of_defaults == len(arguments)+1:
            self._arguments[criterion.name] = arguments
        else:
            raise RuntimeError('Number of inputs is not coincident with criterion arguments!')
        """
        if (criterion.__class__.__base__ == Criterion):
            self._criterions[criterion.name] = criterion
        else:
            raise TypeError('Criterion must be instance or inherited object from Criterion class.')

    def remove_criterion(self, name):
        """Remove existing criterion from collection.

        Args:
            name: Criterion name.
        """

        del self._criterions[name]
        del self._arguments[name]

    def validate(self, model):
        """Validate all criterions for model and return result.

        Args:
            model: Object inherited from ModelBase class.
        """

        result = False
        for name, criterion in self._criterions.items():
            arguments = []
            for argument in self._arguments[name]:
                if (hasattr(argument, 'evaluate') and
                    inspect.ismethod(getattr(argument, 'evaluate'))):
                    arguments.append(argument.evaluate(model))
                else:
                    arguments.append(argument)

            if not criterion.validate(*arguments):
                return False
            else:
                result = True

        return result

class Criterion:
    """General class for model data criterion."""

    def __init__(self, name):
        """Initialization of criterion."""
        self.name = name

    def validate(self, model):
        warnings.warn("Method validate() should be overrided!", RuntimeWarning)

class ValueCriterion(Criterion):
    """Criterion tested if two values are equal."""

    def validate(self, x, y, places=3):
        """Return True if round(x - y, places) == 0, otherwise False."""
        return round(x - y, places) == 0

class RangeCriterion(Criterion):
    """Criterion tested if value be in interval."""

    def validate(self, x, a, b):
        """Return True if a <= x <=b, otherwise False."""
        return (a <= x <= b)

class RelationshipCriterion(Criterion):
    """Criterion tested if x and y are in specific relation."""

    def validate(self, x, y, operator):
        return operators(operator)(x, y)

class RelativeDifferenceCriterion(Criterion):
    """Criterion tested relative difference of value.

    If relative difference of value in last two steps
    exceed defined value return True, otherwise return False.
    """

    def __init__(self, name):
        """Initialization of criterion."""

        Criterion.__init__(self, name)
        self.values = list()
        self.differences = list()

    def validate(self, value, difference):
        if not self.values:
            self.values.append(value)
            return False

        if (abs(value) > 0 or abs(self.values[-1]) > 0):
            self.differences.append(abs(value-self.values[-1])/max(abs(value), abs(self.values[-1])))
        else:
            self.differences.append(0)

        return self.differences[-1]

if __name__ == '__main__':
    from variant.test_functions import QuadraticFunction
    from variant.parameter import Parameter
    from variant.functional import Functional

    model = QuadraticFunction()
    model.parameters['x'] = 2
    model.solve()

    criterions = Criterions()
    criterions.add_criterion(ValueCriterion('3=3'), [3, 3])
    criterions.add_criterion(ValueCriterion('x=2'), [Parameter('x'), 2])
    criterions.add_criterion(RangeCriterion('0<=F<=10'), [Functional('F'), 5, 10])
    criterions.add_criterion(RelationshipCriterion('x>a'), [Parameter('x'), Parameter('a'), '>'])

    print(criterions.validate(model))
from math import sqrt

import warnings
import inspect

class Calculators:
    """General class collected calculators."""

    def __init__(self):
        """Initialization of calculators object."""

        self._calculators = dict()
        self._arguments = dict()

    @property
    def calculators(self):
        """Return dictionary of calculators."""
        return self._calculators

    @calculators.setter
    def calculators(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    @property
    def arguments(self):
        """Return dictionary of calculators arguments."""
        return self._arguments

    @arguments.setter
    def arguments(self, value):
        raise RuntimeError('Object cannot be overwrite!')

    def add_calculator(self, calculator, arguments):
        """Add new calculator.

        Args:
            calculator: Object inherited from Calculator class.
            arguments: List of Parameter and Functional objects used as calculator arguments.
        """

        if calculator.name in self._calculators:
            raise NameError('Calculator with name "{0}" alredy exist.'.format(calculator.name))

        specification = inspect.getargspec(calculator.calculate)
        required_arguments = specification.args
        number_of_defaults = 0
        if specification.defaults:
            number_of_defaults = len(specification.defaults)

        if len(required_arguments)-number_of_defaults == len(arguments)+1:
            self._arguments[calculator.name] = arguments
        else:
            raise RuntimeError('Number of inputs is not coincident with calculator arguments!')

        if (calculator.__class__.__base__ == Calculator):
            self._calculators[calculator.name] = calculator
        else:
            raise TypeError('Calculator must be instance or inherited object from Calculator class.')

    def remove_calculator(self, name):
        """Remove existing calculator from collection.

        Args:
            name: Calculator name.
        """

        del self._calculators[name]
        del self._arguments[name]

    def calculate(self, models):
        """Evaluate all calculators for models and return results.

        Args:
            model: List of models for calculation.
        """

        results = {}
        for name, calculator in self._calculators.items():
            arguments = []
            for argument in self._arguments[name]:
                if (hasattr(argument, 'evaluate') and
                    inspect.ismethod(getattr(argument, 'evaluate'))):

                    values = []
                    for model in models:
                        values.append(argument.evaluate(model))
                    arguments.append(values)
                else:
                    arguments.append(argument)

            results[calculator.name] = calculator.calculate(*arguments)

        return results

class Calculator:
    """General class for calculator."""

    def __init__(self, name):
        """Initialization of calculator."""
        self.name = name

    def calculate(self):
        warnings.warn("Method calculate() should be overrided!", RuntimeWarning)

class PearsonCorrelation(Calculator):
    """Calculator for Pearson product-moment correlation coefficient."""

    def calculate(self, X, Y):
        """Calculate and return correlation coefficient.

        Args:
            X: List of first variable values.
            Y: List of second variable values.
        """

        assert len(X) == len(Y)

        X_mean = sum(X)/float(len(X))
        X_dev = 0.0
        for Xi in X:
            X_dev += (Xi-X_mean)**2

        Y_mean = sum(Y)/float(len(Y))
        Y_dev = 0.0
        for Yi in Y:
            Y_dev += (Yi-Y_mean)**2

        score = 0.0
        for i in range(len(X)):
            score += (X[i]-X_mean) * (Y[i]-Y_mean)
        return score/(sqrt(X_dev) * sqrt(Y_dev))

class SpearmanRankCorrelation(Calculator):
    """Calculator for Spearman's rank correlation coefficient."""

    def calculate(self, X, Y):
        """Calculate and return correlation coefficient.

        Args:
            X: List of first variable values.
            Y: List of second variable values.
        """

        assert len(X) == len(Y)

        Xs = sorted(X)
        ix = []
        for Xi in X:
            n = X.count(Xi)
            if n == 1:
                ix.append(Xs.index(Xi)+1)
            else:
                ix.append(Xs.index(Xi)+1+1/n)

        Ys = sorted(Y)
        iy = []
        for Yi in Y:
            n = Y.count(Yi)
            if n == 1:
                iy.append(Ys.index(Yi)+1)
            else:
                iy.append(Ys.index(Yi)+1+1/n)

        diff = 0
        for i in range(len(X)):
            diff += (ix[i] - iy[i])**2

        return 1 - 6*diff/(len(X)*(len(X)**2-1))

class FirstOrderSensitivityIndex(Calculator):
    """Calculator for first-order sensitivity index."""

    def calculate(self, X, Y, subsets=10):
        assert len(X) == len(Y)

        Xs = sorted(X)
        Ys = []
        for Xsi in Xs:
            Ys.append(Y[X.index(Xsi)])

        s = max(1, int(len(Ys)/float(subsets)))
        Ys_sub = [Ys[i:i+s] for i in range(0, len(Ys), s)]

        EY_sub = []
        for Ys_subi in Ys_sub:
            EY_sub.append(sum(Ys_subi)/float(len(Ys_subi)))
        EEY_sub = sum(EY_sub)/float(len(EY_sub))

        VY_sub = 0.0
        for EY_subi in EY_sub:
            VY_sub += (EY_subi-EEY_sub)**2

        EY = sum(Y)/float(len(Y))
        VY = 0.0
        for Yi in Y:
            VY += (Yi-EY)**2
        VY = VY/len(Y)

        return VY_sub/VY

if __name__ == '__main__':
    from variant.dictionary import ModelDictionary
    from variant.test_functions import HolderTableFunction

    import random as rnd

    md = ModelDictionary()

    minimum = -50
    maximum = +50
    for i in range(1000):
        model = HolderTableFunction()
        model.parameters['x'] = minimum + rnd.random() * (maximum - minimum)
        model.parameters['y'] = minimum + rnd.random() * (maximum - minimum)
        md.add_model(model)

    md.solve()
    """
    sobol = FirstOrderSensitivityIndex('Sobol')
    print(sobol.calculate(md.parameter('x'), md.variable('F')))
    print(sobol.calculate(md.parameter('y'), md.variable('F')))
    """

    print(PearsonCorrelation('F(x)').calculate([3400,2600,2200,2400,2900,2100], [24,20,17,19,26,20])) #0.78
    print(SpearmanRankCorrelation('F(x)').calculate([3400,2600,2200,2400,2900,2100], [24,20,17,19,26,20])) #0.7
    print(SpearmanRankCorrelation('F(x)').calculate([106, 86, 100, 101, 99, 103, 97, 113, 112, 110],
                                                    [7, 0, 27, 50, 28, 29, 20, 12, 6, 17])) # -0.175

    """
    print(FirstOrderSensitivityIndex('F(x)').calculate([106, 86, 100, 101, 99, 103, 97, 113, 112, 110],
                                                       [7, 0, 27, 50, 28, 29, 20, 12, 6, 17], 5))
    """
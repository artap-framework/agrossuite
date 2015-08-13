from variant.calculator import SpearmanRankCorrelation
from variant.study import Study

class Optimization(Study):
    """General class for optimization method."""

    def __init__(self, model_class, parameters, functionals, calculators=None):
        Study.__init__(self, model_class, parameters, functionals, calculators)

        self.sensitivity = False
        self.sensitivity_calculator = SpearmanRankCorrelation
        self.sensitivity_threshold = 0.01
        self.sensitivity_individuals = 100

    def find_best(self, models=None):
        """Find and return best model.

        Args:
            models: List of models (search in all models).
        """

        if not models:
            models = self.model_dictionary.models()

        if not self.functionals.multicriteria():
            direction = self.functionals.functional().direction_sign()
            evaluate = self.functionals.evaluate

            superstar = models[-1]
            superstar_score = evaluate(superstar)
            for model in models:
                score = evaluate(model)
                if direction * score < direction * superstar_score:
                    superstar = model
                    superstar_score = evaluate(superstar)
            return superstar
        else:
            # TODO: Pareto front
            pass

    def sensitivity_analysis(self):
        """Sensitivity analysis of model parameters.

        If calculated weight is below sensitivity threshold, remove optimized parameter.
        """

        dulls = []
        for parameter in self.parameters.parameters.values():
            X = self.model_dictionary.parameter(parameter.name)
            for functional in self.functionals.functionals.values():
                name = '{0}({1},{2})'.format(self.sensitivity_calculator.__name__, functional.name, parameter.name)
                calculator = self.sensitivity_calculator(name)

                Y = self.model_dictionary.variable(functional.name)
                value = calculator.calculate(X, Y)
                weight = abs(value)
                parameter.weights[functional.name] = weight
                self.record(name, value)

                if (weight <= self.sensitivity_threshold and
                    len(self.model_dictionary.models()) >= self.sensitivity_individuals):
                    dulls.append(parameter.name)

        self.record('dulls', len(dulls))
        for dull in dulls:
            self.parameters.remove_parameter(dull)

    def optimize(self):
        pass
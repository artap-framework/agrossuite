from variant.model import ModelBase
from math import exp, sqrt, cos, pi

class AckleysFunction(ModelBase):
    """F(x,y) = -20 * exp(-0.2 * sqrt(0.5 * (x**2 + y**2)))
                -exp(0.5 * (cos(2*pi*x) + cos(2*pi*y))) + e + 20
    """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-5, 5], 'y' : [-5, 5]}
        self.info['minimum'] = {'x' : 0, 'y' : 0}

    def create(self):
        pass

    def solve(self):
            x = self.parameters['x']
            y = self.parameters['y']
            self.variables['F'] = -20 * exp(-0.2  *sqrt(0.5 * (x**2 + y**2))) -\
                                   exp(0.5 * (cos(2*pi*x) + cos(2*pi*y))) + exp(1) + 20

if __name__ == '__main__':
    model = AckleysFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0}'.format(model.variables['F']))
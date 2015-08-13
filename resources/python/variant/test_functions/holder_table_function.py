from variant import ModelBase
from math import sin, cos, exp, sqrt, pi

class HolderTableFunction(ModelBase):
    """ F(x,y) = -(sin(x) * cos(y) * exp((1 - (sqrt(x**2 + y**2)(pi))))) """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-10, 10], 'y' : [-10, 10]}
        self.info['minimum'] = {'x' : 8.05502, 'y' : 9.66459}

    def create(self):
        pass

    def solve(self):
        x = self.parameters['x']
        y = self.parameters['y']
        self.variables['F'] = -abs(sin(x) * cos(y) * \
                              exp(abs(1 - (sqrt(x**2 + y**2)/(pi)))))

if __name__ == '__main__':
    model = HolderTableFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0:1.4f}'.format(model.variables['F']))
from variant.model import ModelBase
from math import sin, sqrt

class EggholderFunction(ModelBase):
    """F(x,y) = -(y + 47) * sin(sqrt(abs(y + x/2.0 + 47)))
                -x*sin(sqrt(abs(x - (y + 47))))
    """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-512, 512], 'y' : [-512, 512]}
        self.info['minimum'] = {'x' : 512, 'y' : 404.2319}

    def create(self):
        pass

    def solve(self):
            x = self.parameters['x']
            y = self.parameters['y']
            self.variables['F'] = -(y + 47) * sin(sqrt(abs(y + x/2.0 + 47))) -\
                                  x*sin(sqrt(abs(x - (y + 47))))

if __name__ == '__main__':
    model = EggholderFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0}'.format(model.variables['F']))
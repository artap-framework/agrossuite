from variant.model import ModelBase

class RosenbrockFunction(ModelBase):
    """F(x,y) = (a - x)**2 + b(y - x**2)**2"""

    def declare(self):
        self.parameters.declare('a', float, 1)
        self.parameters.declare('b', float, 100)
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-1e2, 1e2], 'y' : [-1e2, 1e2]}
        self.info['minimum'] = {'x' : self.parameters['a'], 'y' : self.parameters['a']**2}

    def create(self):
        pass

    def solve(self):
            a = self.parameters['a']
            b = self.parameters['b']
            x = self.parameters['x']
            y = self.parameters['y']
            self.variables['F'] = (a - x)**2 + b * (y - x**2)**2

if __name__ == '__main__':
    model = RosenbrockFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0}'.format(model.variables['F']))
from variant.model import ModelBase

class GoldsteinPriceFunction(ModelBase):
    """F(x,y) = (1 + (x + y + 1)**2
                (19 - 14*x + 3*x**2 - 14*y + 6*x*y + 3*y**2))
                (30 + (2*x - 3*y)**2
                (18 - 32*x + 12*x**2 + 48*y - 36*x*y + 27*y**2))
    """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-2, 2], 'y' : [-2, 2]}
        self.info['minimum'] = {'x' : 0, 'y' : -1}

    def create(self):
        pass

    def solve(self):
            x = self.parameters['x']
            y = self.parameters['y']
            self.variables['F'] = (1 + (x + y + 1)**2 *\
                                  (19 - 14*x + 3*x**2 - 14*y + 6*x*y + 3*y**2)) *\
                                  (30 + (2*x - 3*y)**2 *\
                                  (18 - 32*x + 12*x**2 + 48*y - 36*x*y + 27*y**2))

if __name__ == '__main__':
    model = GoldsteinPriceFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0}'.format(model.variables['F']))
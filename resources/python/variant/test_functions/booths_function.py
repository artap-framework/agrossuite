from variant import ModelBase

class BoothsFunction(ModelBase):
    """ F(x,y) = (x + 2y - 7)**2 + (2x + y - 5)**2 """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-10, 10], 'y' : [-10, 10]}
        self.info['minimum'] = {'x' : 1, 'y' : 3}

    def create(self):
        pass

    def solve(self):
        x = self.parameters['x']
        y = self.parameters['y']
        self.variables['F'] = (x + 2 * y - 7)**2 + (2 * x + y - 5)**2

if __name__ == '__main__':
    model = BoothsFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0}'.format(model.variables['F']))
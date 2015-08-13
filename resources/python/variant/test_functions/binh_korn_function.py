from variant import ModelBase

class BinhKornFunction(ModelBase):
    """F1(x,y) = 4 * x**2 + 4 * y**2;
       F2(x,y) = (x - 5)**2 + (y - 5)**2
    """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F1', float)
        self.variables.declare('F2', float)

        self.info['domain'] = {'x' : [0, 5], 'y' : [0, 3]}

    def create(self):
        pass

    def solve(self):
        x = self.parameters['x']
        y = self.parameters['y']
        self.variables['F1'] = 4 * x**2 + 4 * y**2
        self.variables['F2'] = (x - 5)**2 + (y - 5)**2

if __name__ == '__main__':
    model = BinhKornFunction()
    model.parameters['x'] = 2.5
    model.parameters['y'] = 1.5
    model.solve()
    print('F1 = {0}, F2 = {1}'.format(model.variables['F1'],
                                      model.variables['F2']))
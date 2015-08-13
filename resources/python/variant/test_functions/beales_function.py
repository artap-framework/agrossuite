from variant.model import ModelBase

class BealesFunction(ModelBase):
    """F(x,y) = (1.5 - x +x*y)**2 + (2.25 - x + x*y**2)**2
                + (2.625 - x + x*y**3)**2
    """

    def declare(self):
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

        self.info['domain'] = {'x' : [-4.5, 4.5], 'y' : [-4.5, 4.5]}
        self.info['minimum'] = {'x' : 3, 'y' : 0.5}

    def create(self):
        pass

    def solve(self):
            x = self.parameters['x']
            y = self.parameters['y']
            self.variables['F'] = (1.5 - x +x*y)**2 + (2.25 - x + x*y**2)**2 +\
                                  (2.625 - x + x*y**3)**2

if __name__ == '__main__':
    model = BealesFunction()
    model.parameters['x'] = model.info['minimum']['x']
    model.parameters['y'] = model.info['minimum']['y']
    model.solve()
    print('F = {0}'.format(model.variables['F']))
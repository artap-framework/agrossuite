from variant import ModelBase
from variant import Parameters, ContinuousParameter
from variant import Functionals, Functional
from variant.genetic import Genetic

from mpl_toolkits.mplot3d import Axes3D

import pythonlab
import numpy as np
import pylab as pl

class RosenbrockFunction(ModelBase):
    def declare(self):
        self.parameters.declare('a', float, 1)
        self.parameters.declare('b', float, 100)
        self.parameters.declare('x', float)
        self.parameters.declare('y', float)
        self.variables.declare('F', float)

    def create(self):
        pass

    def solve(self):
        self.variables['F'] = (self.parameters['a'] - self.parameters['x'])**2 +\
                               self.parameters['b'] * (self.parameters['y'] - self.parameters['x']**2)**2

if __name__ == '__main__':
    """ optimization """
    parameters = Parameters([ContinuousParameter('x', -5.0, 5.0),
                             ContinuousParameter('y', -5.0, 5.0)])
    
    functionals = Functionals([Functional("F", "min")])
    genetic = Genetic(RosenbrockFunction, parameters, functionals)

    genetic.population_size = 100

    genetic.selection_ratio = 4.0/5.0
    genetic.elitism_ratio = 1.0/10.0
    genetic.crossover_ratio = 3.0/2.0

    genetic.mutation_probability = 1e-1
    genetic.mutation_ratio = 1.0/2.0

    genetic.optimize(10, save=False)

    star = genetic.find_best()
    print('Fmin = {0}, prameters: {1}'.format(star.variables['F'], star.parameters))

    # results
    x = genetic.model_dictionary.parameter_values('x')
    y = genetic.model_dictionary.parameter_values('y')
    F = genetic.model_dictionary.variable_values('F')
    populations = genetic.model_dictionary.info_values('population')

    pl.figure()
    pl.plot(populations, F, 'o', ms=7.5, c=(0.75,0,0))
    pl.xlabel('# population')
    pl.ylabel('F')
    pl.yscale('log')

    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=60, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)

    pl.figure()
    pl.plot(x, y, 'o', ms=7.5, c=(0.75,0,0))
    pl.plot(star.parameters['a'], star.parameters['a']**2, 'o', ms=15, c=(0,0,0.75))
    pl.xlabel('x')
    pl.ylabel('y')

    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=60, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)

    pl.figure()
    X = np.arange(-5.0, 5.0, 1)
    Y = np.arange(-5.0, 5.0, 1)
    X, Y = np.meshgrid(X, Y)
    Fs = (1 - X)**2 + 100*(Y - X**2)**2

    axes = pl.axes(projection='3d')
    axes.plot_wireframe(X, Y, Fs, color=(0,0,0))

    axes.scatter(x, y, F, 'o', c=(0.75,0,0), s=30)
    axes.scatter(star.parameters['a'], star.parameters['a']**2, 0.0, 's',
                 c=(0,0,0.75), s=120)

    axes.set_xlabel('x')
    axes.set_ylabel('y')
    axes.set_zlabel('F')
    axes.set_zscale('log')
    axes.zaxis.set_ticks([0, max(F)])
    axes.view_init(elev=30, azim=80)

    file = pythonlab.tempname("png")
    pl.savefig(file, dpi=70, bbox_inches='tight')
    pl.close()
    pythonlab.image(file)
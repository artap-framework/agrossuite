from variant.test_functions.ackleys_function import AckleysFunction
from variant.test_functions.beales_function import BealesFunction
from variant.test_functions.booths_function import BoothsFunction
from variant.test_functions.eggholder_function import EggholderFunction
from variant.test_functions.goldstein_price_function import GoldsteinPriceFunction
from variant.test_functions.holder_table_function import HolderTableFunction
from variant.test_functions.rosenbrock_function import RosenbrockFunction

from variant.parameter import Parameters, ContinuousParameter
from variant.functional import Functionals, Functional
from variant.genetic import Genetic

import matplotlib as mpl
import pylab as pl

pl.rcParams['figure.figsize'] = 6, 4
pl.rcParams['legend.fontsize'] = 12
pl.rcParams['font.size'] = 14
pl.rcParams['text.usetex'] = True
pl.rcParams['axes.grid'] = False

model_class = BoothsFunction
model = model_class()
model.parameters['x'] = model.info['minimum']['x']
model.parameters['y'] = model.info['minimum']['y']
model.solve()

parameters = Parameters([ContinuousParameter('x', model.info['domain']['x'][0], model.info['domain']['x'][1]),
                         ContinuousParameter('y', model.info['domain']['y'][0], model.info['domain']['y'][1])])
    
functionals = Functionals([Functional("F", "min")])
genetic = Genetic(model_class, parameters, functionals)

genetic.population_size = 20
genetic.selection_ratio = 4.0/5.0
genetic.elitism_ratio = 1.0/10.0
genetic.crossover_ratio = 4.0/3.0
genetic.mutation_probability = 1e-1
genetic.mutation_ratio = 2.0/3.0

number_of_populations = 20
genetic.optimize(number_of_populations, save=False)
star = genetic.find_best()
print('Optimization: Fmin = {0} ({1})'.format(star.variables['F'], star.parameters))
print('Solution: Fmin = {0} ({1})'.format(model.variables['F'], model.parameters))

x = genetic.model_dictionary.parameter_values('x')
y = genetic.model_dictionary.parameter_values('y')
F = genetic.model_dictionary.variable_values('F')
populations = genetic.model_dictionary.info_values('population')

pl.figure()
pl.plot(populations, F, 'o', ms=7.5, c=(0.75,0,0))
pl.xlabel('$\#\,\mathrm{population}$')
pl.ylabel('$F$')
pl.savefig('convergence.pdf', bbox_inches='tight')

pl.figure()
pl.plot(range(number_of_populations), genetic.model_dictionary.description['time'], '-', c=(0.75,0,0))
pl.xlabel('$\#\,\mathrm{population}$')
pl.ylabel('$t_\mathrm{e}\,\mathrm{(s)}$')
pl.savefig('elapsed_time.pdf', bbox_inches='tight')

pl.figure()
pl.scatter(x, y, s=60, c=F, cmap=mpl.cm.jet)
pl.plot(model.info['minimum']['x'], model.info['minimum']['y'], 'o', ms=20, c=(0.75,0,0))
pl.xlabel('$x$')
pl.ylabel('$y$')
pl.xlim(model.info['domain']['x'])
pl.ylim(model.info['domain']['y'])
pl.savefig('domain.pdf', bbox_inches='tight')

pl.figure()
models = genetic.model_dictionary.models()
for index in range(number_of_populations):
    color=mpl.cm.jet(index/float(number_of_populations-1), 1)

    population = genetic.population(population=index)
    for model in population:
        if ('mother' in model.info and 'father' in model.info):
            mother = models[model.info['mother']]
            father = models[model.info['father']]
            pl.plot([mother.parameters['x'], model.parameters['x']],
                    [mother.parameters['y'], model.parameters['y']],
                    '--', color=color)
            pl.plot([father.parameters['x'], model.parameters['x']],
                    [father.parameters['y'], model.parameters['y']],
                    '-.', color=color)

    for model in population:
        pl.plot(model.parameters['x'], model.parameters['y'], 'o', ms=7.5, color=color)

pl.plot(model.info['minimum']['x'], model.info['minimum']['y'], 'o', ms=20, c=(0.75,0,0))

pl.xlabel('$x$')
pl.ylabel('$y$')
pl.xlim(model.info['domain']['x'])
pl.ylim(model.info['domain']['y'])
pl.savefig('map.pdf', bbox_inches='tight')
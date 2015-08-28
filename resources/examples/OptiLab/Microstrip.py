import agros2d as a2d
import pythonlab

from variant import ModelBase
from variant import Parameters, ContinuousParameter
from variant import Functionals, Functional
from variant.genetic import Genetic
from variant.criterion import RangeCriterion
from variant.calculator import SpearmanRankCorrelation, PearsonCorrelation
from variant.study import SensitivityAnalysis

from math import sqrt, log, pi

class Microstrip(ModelBase):
    def declare(self):
        self.parameters.declare('U', float, 1)
        self.parameters.declare('epsr', float, 2.6285)
        self.parameters.declare('I', float, 1)
        self.parameters.declare('Z0', float, 75)

        self.parameters.declare('W', float)
        self.parameters.declare('ts', float, 0.5e-4)
        self.parameters.declare('tg', float, 0.5e-4)
        self.parameters.declare('b', float)
        self.parameters.declare('h', float)

        self.variables.declare('Z0', float)
        self.variables.declare('Z0e', float)
        self.variables.declare('F', float)

    def create(self):
        self.problem = a2d.problem(clear = True)
        self.problem.coordinate_type = "planar"
        self.problem.mesh_type = "triangle"

        # electrostic field        
        self.electrostatic = a2d.field("electrostatic")
        self.electrostatic.analysis_type = "steadystate"
        self.electrostatic.matrix_solver = "mumps"
        self.electrostatic.number_of_refinements = 0
        self.electrostatic.polynomial_order = 2
        self.electrostatic.adaptivity_type = "disabled"
        self.electrostatic.solver = "linear"

        self.electrostatic.add_boundary("Source electrode", "electrostatic_potential", {"electrostatic_potential" : self.parameters['U']})
        self.electrostatic.add_boundary("Ground electrode", "electrostatic_potential", {"electrostatic_potential" : 0})
        self.electrostatic.add_boundary("Zero charge", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        self.electrostatic.add_material("Dielectric substrate", {"electrostatic_permittivity" : self.parameters['epsr']})
        self.electrostatic.add_material("Air", {"electrostatic_permittivity" : 1})

        # magnetic field
        self.magnetic = a2d.field("magnetic")
        self.magnetic.analysis_type = "steadystate"
        self.magnetic.matrix_solver = "mumps"
        self.magnetic.number_of_refinements = 0
        self.magnetic.polynomial_order = 2
        self.magnetic.adaptivity_type = "disabled"
        self.magnetic.solver = "linear"

        self.magnetic.add_boundary("A = 0", "magnetic_potential", {"magnetic_potential_real" : 0})
        self.magnetic.add_material("Dielectric substrate", {"magnetic_permeability" : 1})
        self.magnetic.add_material("Conductor (source)", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0,
                                                          "magnetic_total_current_prescribed" : True, "magnetic_total_current_real" : self.parameters['I']})
        self.magnetic.add_material("Conductor (ground)", {"magnetic_permeability" : 1, "magnetic_conductivity" : 0,
                                                          "magnetic_total_current_prescribed" : True, "magnetic_total_current_real" : -self.parameters['I']})
        self.magnetic.add_material("Air", {"magnetic_permeability" : 1})

        geometry = a2d.geometry
        W = self.parameters['W']
        ts = self.parameters['ts']
        tg = self.parameters['tg']
        b = self.parameters['b']
        h = self.parameters['h']

        # dielectric substrate
        geometry.add_edge(-b/2.0, -h/2.0, +b/2.0, -h/2.0, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_edge(b/2.0, -h/2.0, b/2.0, h/2.0)
        geometry.add_edge(b/2.0, h/2.0, W/2.0, h/2.0)
        geometry.add_edge(W/2.0, h/2.0, -W/2.0, h/2.0, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_edge(-W/2.0, h/2.0, -b/2.0, h/2.0)
        geometry.add_edge(-b/2.0, h/2.0, -b/2.0, -h/2.0)
        geometry.add_label(0, 0, materials={"electrostatic" : "Dielectric substrate", "magnetic" : "Dielectric substrate"})

        # source electrode
        geometry.add_edge(W/2.0, h/2.0, W/2.0, h/2.0+ts, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_edge(W/2.0, h/2.0+ts, -W/2.0, h/2.0+ts, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_edge(-W/2.0, h/2.0+ts, -W/2.0, h/2.0, boundaries={"electrostatic" : "Source electrode"})
        geometry.add_label(0, h/2.0+ts/2.0, materials={"magnetic" : "Conductor (source)"})

        # ground electrode
        geometry.add_edge(-b/2.0, -h/2.0, -b/2.0, -h/2.0-tg, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_edge(-b/2.0, -h/2.0-tg, b/2.0, -h/2.0-tg, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_edge(b/2.0, -h/2.0-tg, b/2.0, -h/2.0, boundaries={"electrostatic" : "Ground electrode"})
        geometry.add_label(0, -h/2.0-tg/2.0, materials={"magnetic" : "Conductor (ground)"})

        # boundary
        geometry.add_circle(0, 0, 30*b, boundaries={"electrostatic" : "Zero charge", "magnetic" : "A = 0"})
        geometry.add_label(0, 30*b/2.0, materials={"electrostatic" : "Air", "magnetic" : "Air"})

        # disable view
        a2d.view.mesh.disable()
        a2d.view.post2d.disable()

    def solve(self):
        self.problem.solve()

        # store geometry
        self.info['_geometry'] = a2d.geometry.export_svg_image()

        # compute variables
        C = 2 * self.electrostatic.volume_integrals()['We'] / self.parameters['U']**2
        L = 2 * self.magnetic.volume_integrals()['Wm'] / self.parameters['I']**2
        self.variables['Z0'] = sqrt(L/C)
        self.variables['F'] = abs(self.parameters['Z0'] - self.variables['Z0'])

        # analytical solution
        """
        epsr = self.parameters['epsr']
        W = self.parameters['W']
        b = self.parameters['b']
        epse = (epsr + 1)/2.0 + (epsr - 1)/2.0 * 1/(sqrt(1 + 12*b/W))
        if (W/b <= 1):
            self.variables['Z0e'] = 60.0/sqrt(epse) * log(8*b/W + W/(4*b))
        if (W/b > 1):
            self.variables['Z0e'] = 120.0*pi/(sqrt(epse)*(W/b + 1.393 + 0.667*log(W/b + 1.444)))
        """

if __name__ == '__main__':
    for i in range(10):
        parameters = Parameters([ContinuousParameter('W', 1e-4, 1e-3),
                                 ContinuousParameter('ts', 0.25e-4, 2e-4),
                                 ContinuousParameter('tg', 0.25e-4, 2e-4),
                                 ContinuousParameter('b', 1e-3, 2e-2),
                                 ContinuousParameter('h', 1e-4, 1e-3)])
            
        functionals = Functionals([Functional('F', 'min')])
    
        # optimization
        genetic = Genetic(Microstrip, parameters, functionals)
        genetic.criterions.add_criterion(RangeCriterion('StoppingCriterion'), [Functional('F'), 0, 0.1])
    
        genetic.sensitivity = True
        genetic.sensitivity_calculator = SpearmanRankCorrelation #PearsonCorrelation
        genetic.sensitivity_threshold = 0.1
        genetic.sensitivity_individuals = 1
    
        genetic.population_size = 50
        genetic.optimize(20)
    
        star = genetic.find_best(genetic.model_dictionary.models())
        genetic.record('star.parameters', star.parameters)
        genetic.record('star.variables', star.variables)

        print('Z0 = {0} Ohm (required 75 Ohm)'.format(star.variables['Z0']))
        print('Best variant parameters: {0}'.format(star.parameters))

        if genetic.sensitivity:
            data_file_name = 'optimization-{0}-{1}.opt'.format(i, genetic.sensitivity_calculator.__name__)
        else:
            data_file_name = 'optimization-{0}.opt'.format(i)

        genetic.model_dictionary.save(data_file=data_file_name, problem_file='Microstrip.py')
    
    """
    F = genetic.model_dictionary.variable('F')
    pythonlab.chart(range(len(F)), F)

    print('S(F,W) : {0}'.format(genetic.model_dictionary.description['SpearmanRankCorrelation(F,W)']))
    print('S(F,ts) : {0}'.format(genetic.model_dictionary.description['SpearmanRankCorrelation(F,ts)']))
    print('S(F,tg) : {0}'.format(genetic.model_dictionary.description['SpearmanRankCorrelation(F,tg)']))
    print('S(F,b) : {0}'.format(genetic.model_dictionary.description['SpearmanRankCorrelation(F,b)']))
    print('S(F,h) : {0}'.format(genetic.model_dictionary.description['SpearmanRankCorrelation(F,h)']))
    """

    # sensitivity analysis
    """
    sensitivity = SensitivityAnalysis(Microstrip, parameters, functionals)
    sensitivity.analyse(500, correlation_calculator=PearsonCorrelation)
    sensitivity.model_dictionary.save(data_file='Microstrip-sensitivity.opt', problem_file='Microstrip.py')

    F = sensitivity.model_dictionary.variable('F')
    W = sensitivity.model_dictionary.parameter('W')
    ts = sensitivity.model_dictionary.parameter('ts')
    tg = sensitivity.model_dictionary.parameter('tg')
    b = sensitivity.model_dictionary.parameter('b')
    h = sensitivity.model_dictionary.parameter('h')

    pythonlab.chart(W, F)
    pythonlab.chart(ts, F)
    pythonlab.chart(tg, F)
    pythonlab.chart(b, F)
    pythonlab.chart(h, F)
    """
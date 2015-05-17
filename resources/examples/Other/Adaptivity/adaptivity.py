import agros2d
import pythonlab

def model_sparkgap(type, tolerance, estimator, strategy):
    # problem
    problem = agros2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    # electrostatic
    electrostatic = agros2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.number_of_refinements = 0
    electrostatic.polynomial_order = 1
    electrostatic.solver = "linear"
    
    electrostatic.adaptivity_type = type
    electrostatic.adaptivity_parameters['tolerance'] = tolerance
    electrostatic.adaptivity_parameters['steps'] = 50
    electrostatic.adaptivity_parameters['estimator'] = estimator
    electrostatic.adaptivity_parameters['strategy'] = strategy

    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1000})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Border", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    
    # geometry
    geometry = agros2d.geometry
    geometry.add_edge(0.2, 1, 0, 0.5, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0, 0.5, 0, 0.25, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -0.25, 0, -1, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -1, 1.5, 0.5, angle = 90, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(1.5, 0.5, 0, 2, angle = 90, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, 1, 0.2, 1, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(0, 2, 0, 1, boundaries = {"electrostatic" : "Border"})
    geometry.add_edge(0, -0.25, 0.25, 0, angle = 90, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(0.25, 0, 0, 0.25, angle = 90, boundaries = {"electrostatic" : "Ground"})
    
    geometry.add_label(0.879551, 0.764057, area = 0.06, materials = {"electrostatic" : "Air"})
    agros2d.view.zoom_best_fit()
    
    problem.solve()
    
    info = electrostatic.adaptivity_info()

    return info['dofs'], info['error'], problem.elapsed_time()

def model_capacitor(type, tolerance, estimator, strategy):
    # startup script
    R1 = 0.01
    R2 = 0.03
    R3 = 0.05
    R4 = 0.06
    L = 0.08
    RB = 0.2
    
    U = 10
    eps1 = 3
    eps2 = 4
    
    # problem
    problem = agros2d.problem(clear = True)
    problem.coordinate_type = "axisymmetric"
    problem.mesh_type = "triangle"
    
    # electrostatic
    electrostatic = agros2d.field("electrostatic")
    electrostatic.analysis_type = "steadystate"
    electrostatic.matrix_solver = "umfpack"
    electrostatic.number_of_refinements = 0
    electrostatic.polynomial_order = 1
    electrostatic.solver = "linear"
        
    electrostatic.adaptivity_type = type
    electrostatic.adaptivity_parameters['tolerance'] = tolerance
    electrostatic.adaptivity_parameters['steps'] = 50
    electrostatic.adaptivity_parameters['estimator'] = estimator
    electrostatic.adaptivity_parameters['strategy'] = strategy

    # boundaries
    electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : U})
    electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
    electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
    
    # materials
    electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : eps1, "electrostatic_charge_density" : 0})
    electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : eps2, "electrostatic_charge_density" : 0})
    
    # geometry
    geometry = agros2d.geometry
    geometry.add_edge(0, RB, 0, L, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(R1, L, R1, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
    geometry.add_edge(R1, 0, R2, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(R2, 0, R2, L)
    geometry.add_edge(R2, L, R3, L)
    geometry.add_edge(R3, 0, R2, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(R3, L, R3, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(R4, 0, R4, L, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(R3, L, R4, L, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
    geometry.add_edge(R4, 0, RB, 0, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(RB, 0, 0, RB, angle = 90, segments = 8, boundaries = {"electrostatic" : "Neumann"})
    geometry.add_edge(R1, L, R2, L)
    geometry.add_edge(R1, L, 0, L, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
    
    geometry.add_label((R1+R2)/2.0, L/2.0, materials = {"electrostatic" : "Dielectric 1"})
    geometry.add_label((R2+R3)/2.0, L/2.0, materials = {"electrostatic" : "Dielectric 2"})
    geometry.add_label(R1, RB-R1, materials = {"electrostatic" : "Air"})
    agros2d.view.zoom_best_fit()
    
    problem.solve()
    
    info = electrostatic.adaptivity_info()

    return info['dofs'], info['error'], problem.elapsed_time()

def model_waveguide(type, tolerance, estimator, strategy):
    # problem
    problem = agros2d.problem(clear = True)
    problem.coordinate_type = "planar"
    problem.mesh_type = "triangle"
    problem.frequency = 1.6e+10
    
    # rf_te
    rf_te = agros2d.field("rf_te")
    rf_te.analysis_type = "harmonic"
    rf_te.matrix_solver = "umfpack"
    rf_te.number_of_refinements = 0
    rf_te.polynomial_order = 1
    rf_te.solver = "linear"

    rf_te.adaptivity_type = type
    rf_te.adaptivity_parameters['tolerance'] = tolerance
    rf_te.adaptivity_parameters['steps'] = 50
    rf_te.adaptivity_parameters['estimator'] = estimator
    rf_te.adaptivity_parameters['strategy'] = strategy
    
    # boundaries
    rf_te.add_boundary("Perfect electric conductor", "rf_te_electric_field", {"rf_te_electric_field_real" : 0, "rf_te_electric_field_imag" : 0})
    rf_te.add_boundary("Matched boundary", "rf_te_impedance", {"rf_te_impedance" : 377})
    rf_te.add_boundary("Source", "rf_te_electric_field", {"rf_te_electric_field_real" : { "expression" : "cos(y/0.01143*pi/2)" }, "rf_te_electric_field_imag" : 0})
    
    # materials
    rf_te.add_material("Air", {"rf_te_permittivity" : 1, "rf_te_permeability" : 1, "rf_te_conductivity" : 0, "rf_te_current_density_external_real" : 0, "rf_te_current_density_external_imag" : 0})
    
    # geometry
    geometry = agros2d.geometry
    geometry.add_edge(0.17, -0.01143, 0.17, 0.01143, boundaries = {"rf_te" : "Matched boundary"})
    geometry.add_edge(0, 0.01143, 0, -0.01143, boundaries = {"rf_te" : "Source"})
    geometry.add_edge(0.076, 0.01143, 0.076, 0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.076, 0.0045, 0.081, 0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.081, 0.0045, 0.081, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.081, -0.0045, 0.081, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.081, -0.0045, 0.076, -0.0045, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.076, -0.0045, 0.076, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0, -0.01143, 0.076, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.081, -0.01143, 0.17, -0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.17, 0.01143, 0.081, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    geometry.add_edge(0.076, 0.01143, 0, 0.01143, boundaries = {"rf_te" : "Perfect electric conductor"})
    
    geometry.add_label(0.0367388, 0.0025708, materials = {"rf_te" : "Air"})
    agros2d.view.zoom_best_fit()

    problem.solve()
    
    info = rf_te.adaptivity_info()

    return info['dofs'], info['error'], problem.elapsed_time()

types = ['h-adaptivity', 'p-adaptivity', 'hp-adaptivity']
#estimators = ['kelly', 'gradient']
strategies = ['fixed_fraction_of_cells', 'fixed_fraction_of_total_error', 'balanced_error_and_cost', 'global_refinement']

#types = ['h-adaptivity']
estimators = ['kelly']
#strategies = ['balanced_error_and_cost']

tolerance = 0.5
model = model_sparkgap
#tolerance = 0.5
#model = model_capacitor
#tolerance = 10
#model = model_waveguide

dofs = {}
errors = {}
times = {}
for type in types:
    dofs[type] = {}
    errors[type] = {}
    times[type] = {}
    for estimator in estimators:
        dofs[type][estimator] = {}
        errors[type][estimator] = {}
        times[type][estimator] = {}
        for strategy in strategies:
            dofs[type][estimator][strategy], errors[type][estimator][strategy], times[type][estimator][strategy] = model(type, tolerance, estimator, strategy)
            print(type + ' - ' + estimator + ' - ' + strategy + ' : dofs = ' + str(dofs[type][estimator][strategy][-1]) + ', error = ' + str(errors[type][estimator][strategy][-1]) + ', time = ' + str(times[type][estimator][strategy]))

# chart
import pylab as pl
import numpy as np

pl.close()
for type in types:
    for estimator in estimators:
        for strategy in strategies:
            pl.semilogy(dofs[type][estimator][strategy], errors[type][estimator][strategy], label=type + ' - ' + estimator + ' - ' + strategy)
pl.grid(True)
pl.xlabel("DOFs (-)")
pl.ylabel("error (%)")
pl.legend()

#pl.ylim([1e-1, 1e2])            
fn_chart = pythonlab.tempname("png")
pl.savefig(fn_chart, dpi=60)

# show in console
pythonlab.image(fn_chart)

nm = []
tm = []
dm = []
em = []
for type in types:
    for estimator in estimators:
        for strategy in strategies:
            nm.append(type + ' - ' + estimator + ' - ' + strategy) 
            tm.append(times[type][estimator][strategy])
            dm.append(dofs[type][estimator][strategy][-1])
            em.append(errors[type][estimator][strategy][-1])

fig = pl.figure()
pl.bar(np.arange(len(nm)), tm)
pos = np.arange(len(nm)) + 0.5
pl.xticks(pos, nm, rotation=80)
pl.ylabel("time (s)")
fig.tight_layout()
pl.grid(True)

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)

fig = pl.figure()
pl.bar(np.arange(len(nm)), dm)
pos = np.arange(len(nm)) + 0.5
pl.xticks(pos, nm, rotation=80)
pl.ylabel("dofs (-)")
fig.tight_layout()
pl.grid(True)

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)

fig = pl.figure()
pl.bar(np.arange(len(nm)), em)
pos = np.arange(len(nm)) + 0.5
pl.xticks(pos, nm, rotation=80)
pl.ylabel("error (%)")
fig.tight_layout()
pl.grid(True)

chart_file = pythonlab.tempname("png")
pl.savefig(chart_file, dpi=60)
pl.close()

pythonlab.image(chart_file)
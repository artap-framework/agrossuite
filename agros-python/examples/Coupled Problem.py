from agrossuite import agros

# model
problem = agros.problem(clear=True)
problem.coordinate_type = "planar"
problem.mesh_type = "triangle"

# fields
current = problem.field("current")
current.analysis_type = "steadystate"
current.number_of_refinements = 0
current.polynomial_order = 3
current.solver = "linear"

current.add_boundary("Neumann", "current_inward_current_flow", {"current_inward_current_flow": 0})
current.add_boundary("U", "current_potential", {"current_potential": 0.01})
current.add_boundary("Ground", "current_potential", {"current_potential": 0})

current.add_material("Mild steel", {"current_conductivity": 6e6})
current.add_material("Aluminium", {"current_conductivity": 33e6})
current.solver = "linear"

heat = problem.field("heat")
heat.analysis_type = "steadystate"
heat.number_of_refinements = 1
heat.polynomial_order = 3
heat.solver = "linear"

heat.add_boundary("Zero flux", "heat_heat_flux", {"heat_convection_external_temperature": 0, "heat_convection_heat_transfer_coefficient": 0, "heat_heat_flux": 0, "heat_radiation_ambient_temperature": 0, "heat_radiation_emissivity": 0})
heat.add_boundary("Convection", "heat_heat_flux", {"heat_convection_external_temperature": 20, "heat_convection_heat_transfer_coefficient": 20, "heat_heat_flux": 0, "heat_radiation_ambient_temperature": 0, "heat_radiation_emissivity": 0})

heat.add_material("Mild steel", {"heat_conductivity": 400, "heat_volume_heat": 0})
heat.add_material("Aluminium", {"heat_conductivity": 250, "heat_volume_heat": 0})

elasticity = problem.field("elasticity")
elasticity.analysis_type = "steadystate"
elasticity.number_of_refinements = 1
elasticity.polynomial_order = 3
elasticity.solver = "linear"

elasticity.add_boundary("Fixed", "elasticity_fixed_fixed", {"elasticity_displacement_x": 0, "elasticity_displacement_y": 0})
elasticity.add_boundary("Free", "elasticity_free_free", {"elasticity_force_x": 0, "elasticity_force_y": 0})

elasticity.add_material("Mild steel", {"elasticity_alpha": 15e-6, "elasticity_volume_force_x": 0, "elasticity_volume_force_y": 0, "elasticity_poisson_ratio": 0.33, "elasticity_temperature_difference": 0, "elasticity_temperature_reference": 20, "elasticity_young_modulus": 208e9})
elasticity.add_material("Aluminium", {"elasticity_alpha": 23e-6, "elasticity_volume_force_x": 0, "elasticity_volume_force_y": 0, "elasticity_poisson_ratio": 0.33, "elasticity_temperature_difference": 0, "elasticity_temperature_reference": 20, "elasticity_young_modulus": 70e9})

elasticity.solver = "linear"

# coupling
problem.set_coupling_type("current", "heat", "weak")
problem.set_coupling_type("heat", "elasticity", "weak")

# geometry
geometry = problem.geometry()

# edges
geometry.add_edge(0, 0.01, 0.191, 0.02, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.205, 0.01, 0.191, 0.02, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Zero flux"})
geometry.add_edge(0.205, 0.01, 0.204, 0.002, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0, 0.01, 0, 0, boundaries={"current": "U", "elasticity": "Fixed", "heat": "Zero flux"})
geometry.add_edge(0, 0, 0, -0.01, boundaries={"current": "U", "elasticity": "Fixed", "heat": "Zero flux"})
geometry.add_edge(0.204, 0.002, 0.194, -0.002, boundaries={"current": "Ground", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.194, -0.002, 0.194, -0.008, boundaries={"current": "Ground", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.194, -0.002, 0, 0, boundaries={})
geometry.add_edge(0.13, 0.014, 0.086, 0.002, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Zero flux"})
geometry.add_edge(0.13, 0.014, 0.12, 0.004, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Zero flux"})
geometry.add_edge(0.086, 0.002, 0.12, 0.004, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Zero flux"})
geometry.add_edge(0.084, -0.014, 0.104, -0.004, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.104, -0.004, 0.128, -0.004, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.128, -0.004, 0.13, -0.026, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.13, -0.026, 0.194, -0.008, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})
geometry.add_edge(0.084, -0.014, 0, -0.01, boundaries={"current": "Neumann", "elasticity": "Free", "heat": "Convection"})

# labels
geometry.add_label(0.173639, 0.0106815, materials={"current": "Mild steel", "elasticity": "Mild steel", "heat": "Mild steel"})
geometry.add_label(0.160202, -0.00535067, materials={"current": "Aluminium", "elasticity": "Aluminium", "heat": "Aluminium"})
geometry.add_label(0.116793, 0.00774503, materials={"current": "none", "elasticity": "none", "heat": "none"})

problem.save("pokus.ags")


# solve problem
computation = problem.computation()
computation.solve()
"""
# solution
solution_current = computation.solution("current")
solution_heat = computation.solution("heat")
solution_elasticity = computation.solution("elasticity")

# point value
local_values_current = solution_current.local_values(0.155787, 0.00713725)
print(local_values_current)
local_values_heat = solution_heat.local_values(0.155787, 0.00713725)
print(local_values_heat)
local_values_elasticity = solution_elasticity.local_values(0.155787, 0.00713725)
print(local_values_elasticity)
"""
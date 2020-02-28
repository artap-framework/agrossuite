#!/usr/bin/env python3

import scipy.io as sio

mat_stiffness_object = sio.loadmat('transient_solver-heat_matrix_stiffness.mat')
matrix_stiffness = mat_stiffness_object["matrix_stiffness"]
mat_mass_object = sio.loadmat('transient_solver-heat_matrix_mass.mat')
matrix_mass = mat_mass_object["matrix_mass"]
rhs_object = sio.loadmat('transient_solver-heat_rhs.mat')
rhs = rhs_object["rhs"]
sln_object = sio.loadmat('transient_solver-heat_solutions.mat')
slns = sln_object["slns"]

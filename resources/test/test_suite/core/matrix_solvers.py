import pythonlab
import agros2d

import scipy.io as sio
import numpy as np

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

class TestMatrixSolvers(Agros2DTestCase):
    @classmethod
    def setUpClass(self):
        # store state
        self.save_matrix_and_rhs = agros2d.options.save_matrix_and_rhs
        self.dump_format = agros2d.options.dump_format
        
        # dump format
        agros2d.options.save_matrix_and_rhs = True
        agros2d.options.dump_format = "matlab_mat"
        
        # read reference matrix and rhs from file
        self.reference_mat, self.reference_rhs, self.reference_sln = self.read_system(pythonlab.datadir("/resources/test/test_suite/core/matrix_solvers/matrix.mat"), 
                                                                                      pythonlab.datadir("/resources/test/test_suite/core/matrix_solvers/rhs.mat"), 
                                                                                      pythonlab.datadir("/resources/test/test_suite/core/matrix_solvers/sln.mat"))

    @classmethod
    def tearDownClass(self):
        # restore state
        agros2d.options.save_matrix_and_rhs = self.save_matrix_and_rhs
        agros2d.options.dump_format = self.dump_format

    @classmethod
    def model(self, solver, matrix_solver=""):
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"

        electrostatic = problem.field("electrostatic")
        electrostatic.analysis_type = "steadystate"
        electrostatic.matrix_solver = solver
        electrostatic.number_of_refinements = 1
        electrostatic.polynomial_order = 2
        electrostatic.adaptivity_type = "disabled"
        electrostatic.solver = "linear"

        electrostatic.matrix_solver_parameters['external_solver'] = matrix_solver
                
        electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1e9})
        electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 1})
        electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 20})
        electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : 4, "electrostatic_charge_density" : 30})
        
        geometry = problem.geometry()
        geometry.add_edge(0, 0.2, 0, 0.08, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.01, 0.08, 0.01, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})
        geometry.add_edge(0.01, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.03, 0, 0.03, 0.08)
        geometry.add_edge(0.03, 0.08, 0.05, 0.08)
        geometry.add_edge(0.05, 0, 0.03, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.05, 0.08, 0.05, 0, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.06, 0, 0.06, 0.08, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.05, 0.08, 0.06, 0.08, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Ground"})
        geometry.add_edge(0.06, 0, 0.2, 0, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.2, 0, 0, 0.2, angle = 90, boundaries = {"electrostatic" : "Neumann"})
        geometry.add_edge(0.01, 0.08, 0.03, 0.08)
        geometry.add_edge(0.01, 0.08, 0, 0.08, refinements = {"electrostatic" : 1}, boundaries = {"electrostatic" : "Source"})

        geometry.add_label(0.019, 0.021, materials = {"electrostatic" : "Dielectric 1"})
        geometry.add_label(0.0379, 0.051, materials = {"electrostatic" : "Dielectric 2"})
        geometry.add_label(0.0284191, 0.123601, materials = {"electrostatic" : "Air"})
        
        computation = problem.computation()
        computation.solve()
        solution = computation.solution("electrostatic")

        return solution.filename_matrix(), solution.filename_rhs(), solution.filename_sln()
    
    @classmethod    
    def analyse_system(self, filename_matrix, filename_rhs, filename_sln):
        import pylab as pl

        # read matrix and rhs from file
        matrix, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
              
        # size of the matrix
        print(("Matrix size: " + str(len(rhs))))
        print(("Number of nonzeros: " + str(matrix.getnnz()) + " (" + str(round(float(matrix.getnnz()) / (len(rhs)**2) * 100.0, 3)) + " %)"))
        
        # visualize matrix sparsity pattern
        fig = pl.figure()
        pl.spy(matrix, markersize=1)
        fn_pattern = pythonlab.tempname("png")
        pl.savefig(fn_pattern, dpi=60)
        pl.close(fig)   
        pythonlab.image(fn_pattern)

    @classmethod        
    def read_system(self, filename_matrix, filename_rhs, filename_sln):
        mat_object = sio.loadmat(filename_matrix)
        matrix = mat_object["matrix"]
        rhs_object = sio.loadmat(filename_rhs)
        rhs = rhs_object["rhs"]
        sln_object = sio.loadmat(filename_sln)
        sln = sln_object["sln"]
        
        return matrix, rhs, sln

    def test_umfpack(self):
        # UMFPACK
        filename_matrix, filename_rhs, filename_sln = self.model("umfpack")
        mat, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), mat.todense(), rtol=1e-15, atol=1e-15), 
                        "UMFPACK matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, rhs, rtol=1e-15, atol=1e-10), 
                        "UMFPACK rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, sln, rtol=1e-15, atol=1e-6), 
                        "UMFPACK sln failed.")

    def test_dealii(self):
        # deal.II
        filename_matrix, filename_rhs, filename_sln = self.model("dealii")
        mat, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
                                
        self.assertTrue(np.allclose(self.reference_mat.todense(), mat.todense(), rtol=1e-15, atol=1e-15), 
                        "dealii matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, rhs, rtol=1e-15, atol=1e-10), 
                        "dealii rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, sln, rtol=1e-6), 
                        "dealii sln failed.")                                                   

    def test_external_umfpack(self):
        # UMFPACK
        filename_matrix, filename_rhs, filename_sln = self.model("external", "solver_UMFPACK.ext")
        mat, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), mat.todense(), rtol=1e-15, atol=1e-15), 
                        "UMFPACK (external) matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, rhs, rtol=1e-15, atol=1e-10), 
                        "UMFPACK (external) rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, sln, rtol=1e-6), 
                        "UMFPACK (external) sln failed.")

    def test_external_mumps(self):
        # MUMPS
        filename_matrix, filename_rhs, filename_sln = self.model("external", "solver_MUMPS.ext")
        mat, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), mat.todense(), rtol=1e-15, atol=1e-15), 
                        "MUMPS matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, rhs, rtol=1e-15, atol=1e-10), 
                        "MUMPS rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, sln, rtol=1e-6), 
                        "MUMPS sln failed.")

    def test_external_paralution(self):
        # PARALUTION
        filename_matrix, filename_rhs, filename_sln = self.model("external", "solver_PARALUTION.ext")
        mat, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), mat.todense(), rtol=1e-15, atol=1e-15), 
                        "PARALUTION matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, rhs, rtol=1e-15, atol=1e-10), 
                        "PARALUTION rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, sln, rtol=1e-6), 
                        "PARALUTION sln failed.")

    def test_external_viennacl(self):
        # ViennaCL
        filename_matrix, filename_rhs, filename_sln = self.model("external", "solver_ViennaCL.ext")
        mat, rhs, sln = self.read_system(filename_matrix, filename_rhs, filename_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), mat.todense(), rtol=1e-15, atol=1e-15), 
                        "ViennaCL matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, rhs, rtol=1e-15, atol=1e-10), 
                        "ViennaCL rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, sln, rtol=1e-5), 
                        "ViennaCL sln failed.")

if __name__ == '__main__':
    import unittest as ut

    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestMatrixSolvers))
    suite.run(result)
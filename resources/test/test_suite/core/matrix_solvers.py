import pythonlab
import agros2d

import scipy.io as sio
import numpy as np

from test_suite.scenario import Agros2DTestCase
from test_suite.scenario import Agros2DTestResult

# TODO: add more iter methods (deal.II)
# TODO: add MUMPS

class TestInternalMatrixSolvers(Agros2DTestCase):
    @classmethod
    def setUpClass(self): 
        # store state
        self.save_matrix_and_rhs = agros2d.options.save_matrix_and_rhs
        self.dump_format = agros2d.options.dump_format
        
        # dump format
        agros2d.options.save_matrix_and_rhs = True
        agros2d.options.dump_format = "matlab_mat"
        
        # read reference matrix and rhs from file
        self.reference_mat, self.reference_rhs, self.reference_sln = self.read_system(pythonlab.datadir("/resources/test/test_suite/core/matrix_solvers_matrix.mat"), 
                                                                                      pythonlab.datadir("/resources/test/test_suite/core/matrix_solvers_rhs.mat"), 
                                                                                      pythonlab.datadir("/resources/test/test_suite/core/matrix_solvers_sln.mat"))

    @classmethod
    def tearDownClass(self):
        # restore state
        agros2d.options.save_matrix_and_rhs = self.save_matrix_and_rhs
        agros2d.options.dump_format = self.dump_format

    @classmethod
    def model(self, solver):
        # problem
        problem = agros2d.problem(clear = True)
        problem.coordinate_type = "axisymmetric"
        problem.mesh_type = "triangle"
        
        # fields
        # electrostatic
        electrostatic = agros2d.field("electrostatic")
        electrostatic.analysis_type = "steadystate"
        electrostatic.matrix_solver = solver
        electrostatic.number_of_refinements = 1
        electrostatic.polynomial_order = 2
        electrostatic.adaptivity_type = "disabled"
        electrostatic.solver = "linear"
        
        # boundaries
        electrostatic.add_boundary("Source", "electrostatic_potential", {"electrostatic_potential" : 1e9})
        electrostatic.add_boundary("Ground", "electrostatic_potential", {"electrostatic_potential" : 0})
        electrostatic.add_boundary("Neumann", "electrostatic_surface_charge_density", {"electrostatic_surface_charge_density" : 0})
        
        # materials
        electrostatic.add_material("Air", {"electrostatic_permittivity" : 1, "electrostatic_charge_density" : 1})
        electrostatic.add_material("Dielectric 1", {"electrostatic_permittivity" : 3, "electrostatic_charge_density" : 20})
        electrostatic.add_material("Dielectric 2", {"electrostatic_permittivity" : 4, "electrostatic_charge_density" : 30})
        
        # geometry
        geometry = agros2d.geometry
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
        
        agros2d.view.zoom_best_fit()
        problem.solve()
        
        return electrostatic.filename_matrix(), electrostatic.filename_rhs(), electrostatic.filename_sln()
    
    @classmethod    
    def analyse_system(self, filename_matrix, filename_rhs, filename_sln):
        import pylab as pl
   
        # read matrix and rhs from file
        mat_object = sio.loadmat(filename_matrix)
        matrix = mat_object["matrix"]
        rhs_object = sio.loadmat(filename_rhs)
        rhs = rhs_object["rhs"]
        sln_object = sio.loadmat(filename_sln)
        sln = sln_object["sln"]
              
        # size of the matrix
        print(("Matrix size: " + str(len(rhs))))
        print(("Number of nonzeros: " + str(matrix.getnnz()) + " (" + str(round(float(matrix.getnnz()) / (len(rhs)**2) * 100.0, 3)) + " %)"))
        
        # visualize matrix sparsity pattern
        fig = pl.figure()
        pl.spy(matrix, markersize=1)
        fn_pattern = pythonlab.tempname("png")
        pl.savefig(fn_pattern, dpi=60)
        pl.close(fig)   
        # show in console
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
        filename_umfpack_matrix, filename_umfpack_rhs, filename_umfpack_sln = self.model("umfpack")
        umfpack_mat, umfpack_rhs, umfpack_sln = self.read_system(filename_umfpack_matrix, 
                                                                 filename_umfpack_rhs, 
                                                                 filename_umfpack_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), umfpack_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "UMFPACK matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, umfpack_rhs, rtol=1e-15, atol=1e-10), 
                        "UMFPACK rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, umfpack_sln, rtol=1e-15, atol=1e-6), 
                        "UMFPACK sln failed.")       

    def test_dealii_iter(self):
        # deal.II - iterative
        filename_dealii_iterative_matrix, filename_dealii_iterative_rhs, filename_dealii_iterative_sln = self.model("dealii")
        dealii_iterative_mat, dealii_iterative_rhs, dealii_iterative_sln = self.read_system(filename_dealii_iterative_matrix, 
                                                                                            filename_dealii_iterative_rhs,
                                                                                            filename_dealii_iterative_sln)
                                
        self.assertTrue(np.allclose(self.reference_mat.todense(), dealii_iterative_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "dealii iterative matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, dealii_iterative_rhs, rtol=1e-15, atol=1e-10), 
                        "dealii iterative rhs failed.")
        self.assertTrue(np.allclose(self.reference_sln, dealii_iterative_sln, rtol=1e-6), 
                        "dealii iterative sln failed.")                                                   

    def test_external(self):
        # external
        filename_external_matrix, filename_external_rhs, filename_external_sln = self.model("external")
        external_mat, external_rhs, external_sln = self.read_system(filename_external_matrix, 
                                                                    filename_external_rhs, 
                                                                    filename_external_sln)
        
        self.assertTrue(np.allclose(self.reference_mat.todense(), external_mat.todense(), rtol=1e-15, atol=1e-15), 
                        "EXTERNAL matrix failed.")
        self.assertTrue(np.allclose(self.reference_rhs, external_rhs, rtol=1e-15, atol=1e-10), 
                        "EXTERNAL rhs failed.")        
        self.assertTrue(np.allclose(self.reference_sln, external_sln, rtol=1e-6), 
                        "EXTERNAL rhs failed.")        

if __name__ == '__main__':        
    import unittest as ut
    
    suite = ut.TestSuite()
    result = Agros2DTestResult()
    suite.addTest(ut.TestLoader().loadTestsFromTestCase(TestInternalMatrixSolvers))
    suite.run(result)
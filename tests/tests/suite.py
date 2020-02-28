import inspect
import types

import tests.fields
import tests.coupled_problems
import tests.adaptivity
import tests.particle_tracing
import tests.core
import tests.script
import tests.optilab

def __get_tests__(object):
    tests = list()
    if isinstance(object, list):
        for member in object:
            tests += __get_tests__(member)

    if isinstance(object, types.ModuleType):
        for name, obj in inspect.getmembers(object):
            for sub_name, sub_obj in inspect.getmembers(obj):
                if isinstance(sub_obj, types.ModuleType): continue
                test_or_benchmark = sub_name.startswith('Test') or sub_name.startswith('Benchmark')
                if inspect.isclass(sub_obj) and test_or_benchmark:
                    tests.append(sub_obj)

    for name, obj in inspect.getmembers(object):
        test_or_benchmark = name.startswith('Test') or name.startswith('Benchmark')
        if inspect.isclass(obj) and test_or_benchmark:
            tests.append(obj)

    return tests
__tests__ = dict()

# fields
__tests__["fields"] = __get_tests__(tests.fields)

# coupled
__tests__["coupled"] = __get_tests__(tests.coupled_problems)

# nonlin
__tests__["nonlin"] = [
tests.fields.heat.TestHeatNonlinPlanarNewton,
tests.fields.heat.TestHeatNonlinPlanarPicard,
tests.fields.magnetic_steady.TestMagneticNonlinPlanar,
tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar,
tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric,
tests.fields.flow.TestFlowPlanar,
tests.fields.flow.TestFlowAxisymmetric
]

# adaptivity
__tests__["adaptivity"] = __get_tests__(tests.adaptivity)

# tracing
__tests__["tracing"] = __get_tests__(tests.particle_tracing)

# script
__tests__["script"] = __get_tests__(tests.script)

# core
__tests__["core"] = __get_tests__(tests.core.matrix_solvers) + \
                    __get_tests__(tests.core.mesh_generator)

# complete 
__tests__["complete"] = __tests__["fields"] + __tests__["coupled"] + __tests__["nonlin"] + \
                        __tests__["adaptivity"] + __tests__["tracing"] + \
                        __tests__["script"] + __tests__["core"]

# deal.II 
__tests__["deal.II"] = [
# electrostatic field
tests.fields.electrostatic.TestElectrostaticPlanar, 
tests.fields.electrostatic.TestElectrostaticAxisymmetric,
# current field
tests.fields.current.TestCurrentPlanar,
tests.fields.current.TestCurrentAxisymmetric,
# elasticity
tests.fields.elasticity.TestElasticityPlanar,
tests.fields.elasticity.TestElasticityAxisymmetric,
# incompressible flow
tests.fields.flow.TestFlowPlanar,
tests.fields.flow.TestFlowAxisymmetric,
# acoustic field
tests.fields.acoustic.TestAcousticHarmonicPlanar,
tests.fields.acoustic.TestAcousticHarmonicAxisymmetric,
# heat transfer
tests.fields.heat.TestHeatPlanar,
tests.fields.heat.TestHeatAxisymmetric,
tests.fields.heat.TestHeatNonlinPlanarNewton,
tests.fields.heat.TestHeatNonlinPlanarPicard,
tests.fields.heat.TestHeatTransientAxisymmetric,
# magnetic field
tests.fields.magnetic_steady.TestMagneticPlanar,
tests.fields.magnetic_steady.TestMagneticAxisymmetric,
tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanar,
tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetric,
#tests.fields.magnetic_harmonic.TestMagneticHarmonicPlanarTotalCurrent,
#tests.fields.magnetic_harmonic.TestMagneticHarmonicAxisymmetricTotalCurrent,
tests.fields.magnetic_steady.TestMagneticNonlinPlanar,
tests.fields.magnetic_steady.TestMagneticNonlinAxisymmetric,
#tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinPlanar,
#tests.fields.magnetic_harmonic.TestMagneticHarmonicNonlinAxisymmetric,
# rf te
tests.fields.rf_te.TestRFTEHarmonicPlanar,
tests.fields.rf_te.TestRFTEHarmonicAxisymmetric,
# rf tm
tests.fields.rf_tm.TestRFTMHarmonicPlanar,
tests.fields.rf_tm.TestRFTMHarmonicAxisymmetric,
# math coeff
tests.fields.math_coeff.TestMathCoeffPlanar,
tests.fields.math_coeff.TestMathCoeffAxisymmetric,
# adaptivity
tests.adaptivity.adaptivity.TestAdaptivityElectrostatic,
tests.adaptivity.adaptivity.TestAdaptivityAcoustic,
tests.adaptivity.adaptivity.TestAdaptivityElasticityBracket,
tests.adaptivity.adaptivity.TestAdaptivityMagneticProfileConductor,
#tests.adaptivity.adaptivity.TestAdaptivityRF_TE,
tests.adaptivity.adaptivity.TestAdaptivityHLenses,
#tests.adaptivity.adaptivity.TestAdaptivityPAndHCoupled,
# particle tracing
tests.particle_tracing.particle_tracing.TestParticleTracingPlanar,
#tests.particle_tracing.particle_tracing.TestParticleTracingAxisymmetric,
# coupled fields
tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakWeak,
#tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1WeakHard,
#tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardWeak,
#tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic1HardHard,
tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Weak,
#tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic2Hard,
tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakWeak,
#tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic3WeakHard,
tests.coupled_problems.basic_coupled_problems.TestCoupledProblemsBasic4Weak,
tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakWeak,
#tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsWeakHard,
#tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardWeak,
#tests.coupled_problems.unrealistic_coupled_problems.TestCoupledProblemsManyDomainsHardHard,
# core
tests.core.matrix_solvers.TestMatrixSolversInternal,
# optilab
tests.optilab.studies.TestNSGA2Sphere,
tests.optilab.studies.TestNSGA3Sphere,
tests.optilab.studies.TestLimboSphere,
tests.optilab.studies.TestBayesOptBooth,
tests.optilab.studies.TestNLoptBooth
] + __tests__["script"]

__tests__["optilab"] = __get_tests__(tests.optilab)

def all_tests():
    global __tests__
    return __tests__

def test(name):
    global __tests__
    return __tests__[name]

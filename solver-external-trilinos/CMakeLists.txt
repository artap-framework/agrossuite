PROJECT(solver_TRILINOS C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

# SET(DEBUG_FLAGS "-g")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsupported-friend")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
ENDIF()

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../3rdparty/")

#SET(TRILINOS_ROOT ${CMAKE_SOURCE_DIR}/trilinos/build)
FIND_PACKAGE(MPI REQUIRED)
FIND_PACKAGE(TRILINOS REQUIRED)

#Print info about the Trilinos installation found
IF(TRILINOS_FOUND)
   MESSAGE("\nFound Trilinos!  Here are the details: ")
   MESSAGE("   TRILINOS_DIR = ${TRILINOS_DIR}")
   MESSAGE("   TRILINOS_VERSION = ${TRILINOS_VERSION}")
   MESSAGE("   TRILINOS_PACKAGE_LIST = ${TRILINOS_PACKAGE_LIST}")
   MESSAGE("   TRILINOS_LIBRARIES = ${TRILINOS_LIBRARIES}")
   MESSAGE("   TRILINOS_INCLUDE_DIRS = ${TRILINOS_INCLUDE_DIRS}")
   MESSAGE("   TRILINOS_TPL_LIST = ${TRILINOS_TPL_LIST}")
   MESSAGE("   TRILINOS_TPL_INCLUDE_DIRS = ${TRILINOS_TPL_INCLUDE_DIRS}")
   MESSAGE("   TRILINOS_TPL_LIBRARIES = ${TRILINOS_TPL_LIBRARIES}")
   # Will Tramonto work with a Shared version of TRILINOS?
   MESSAGE("   TRILINOS_BUILD_SHARED_LIBS = ${TRILINOS_BUILD_SHARED_LIBS}")
   #These last outputs are temporary
   MESSAGE("   TRILINOS_CXX_COMPILER = ${TRILINOS_CXX_COMPILER}")
   MESSAGE("   TRILINOS_C_COMPILER = ${TRILINOS_C_COMPILER}")
   MESSAGE("   TRILINOS_Fortran_COMPILER = ${TRILINOS_Fortran_COMPILER}")
   MESSAGE("   TRILINOS_CXX_COMPILER_FLAGS = ${TRILINOS_CXX_COMPILER_FLAGS}")
   MESSAGE("   TRILINOS_C_COMPILER_FLAGS = ${TRILINOS_C_COMPILER_FLAGS}")
   MESSAGE("   TRILINOS_Fortran_COMPILER_FLAGS =
     ${TRILINOS_Fortran_COMPILER_FLAGS}")
   MESSAGE("   TRILINOS_LINKER = ${TRILINOS_LINKER}")
   MESSAGE("   TRILINOS_EXTRA_LD_FLAGS = ${TRILINOS_EXTRA_LD_FLAGS}")
   MESSAGE("   TRILINOS_AR = ${TRILINOS_AR}")
   # Add MPI_EXEC, MPI_EXEC_MAX_NUMPROCS, MPI_EXEC_NUMPROC_FLAGS
   # These are not available in Trilinos 10.2.0, but should be in 10.4.0
   MESSAGE("End of Trilinos details\n")
ELSE()
  # We should never reach this because FIND_PACKAGE(TRILINOS now uses REQUIRED
  MESSAGE(FATAL_ERROR "Could not find Trilinos 10.2 or greater!")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../3rdparty/matio)
INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${EPETRA_INCLUDE_PATH} ${TEUCHOS_INCLUDE_PATH} ${AMESOS_INCLUDE_PATH} ${AZTECOO_INCLUDE_PATH} ${ML_INCLUDE_PATH})
message("Trilinos include dir:" ${TRILINOS_INCLUDE_DIR})

SET(SOURCES main.cpp)

file(WRITE "${CMAKE_HOME_DIRECTORY}/../libs/${PROJECT_NAME}.ext"
"Trilinos\n\
solver_TRILINOS\n\
<b>-v</b> or <b>--verbose</b> (<i>int</i>) : verbose mode 0 .. disabled, 1 .. simple verbose, 2 .. details, 3 .. file output<br/>\
<b>-g</b> or <b>--multigrid</b> : use algebraic multigrid<br/>\
<b>-t</b> or <b>--rel_tol</b> (<i>double</i>)<br/>\
<b>-l</b> or <b>--solver</b> (<i>string</i>) : Non-Multigrid solvers: Amesos_Klu, Amesos_Paraklete, AztecOO. Multigrid solver variants: AztecOO_cg, AztecOO_cg_condnum, AztecOO_gmres, AztecOO_gmres_condnum, AztecOO_cgs, AztecOO_tfqmr, AztecOO_bicgstab, AztecOO_lu, AztecOOML_cg, AztecOOML_cg_condnum, AztecOOML_gmres, AztecOOML_gmres_condnum, AztecOOML_cgs, AztecOOML_tfqmr, AztecOOML_bicgstab<br/>\
<b>-c</b> or <b>--preconditioner</b> (<i>string</i>) : Multigrid preconditioner set of parameters: SA, NSSA, maxwell, DD, RefMaxwell, DD-ML, DD-ML-LU<br/>\
<b>-e</b> or <b>--aggregationType</b> (<i>string</i>) : Multigrid aggregation type: Uncoupled, Coupled, MIS, Uncoupled-MIS, METIS, ParMETIS<br/>\
<b>-o</b> or <b>--smootherType</b> (<i>string</i>) : Multigrid smoother type: Aztec, IFPACK, Jacobi, ML_symmetric_Gauss-Seidel, symmetric_Gauss-Seidel, ML_Gauss-Seidel, Gauss-Seidel, block_Gauss-Seidel, symmetric_block_Gauss-Seidel, Chebyshev, MLS, Hiptmair, Amesos-KLU, Amesos-Superlu, Amesos-UMFPACK, Amesos-Superludist, Amesos-MUMPS, user-defined, SuperLU, IFPACK-Chebyshev, self, do-nothing, IC, ICT, ILU, ILUT, Block_Chebyshev, IFPACK-Block_Chebyshev, line_Jacobi, line_Gauss-Seidel, SILU<br/>\
<b>-z</b> or <b>--coarseType</b> (<i>string</i>) : Multigrid coarser type: Aztec, IFPACK, Jacobi, ML_symmetric_Gauss-Seidel, symmetric_Gauss-Seidel, ML_Gauss-Seidel, Gauss-Seidel, block_Gauss-Seidel, symmetric_block_Gauss-Seidel, Chebyshev, MLS, Hiptmair, Amesos-KLU, Amesos-Superlu, Amesos-UMFPACK, Amesos-Superludist, Amesos-MUMPS, user-defined, SuperLU, IFPACK-Chebyshev, self, do-nothing, IC, ICT, ILU, ILUT, Block_Chebyshev, IFPACK-Block_Chebyshev, line_Jacobi, line_Gauss-Seidel, SILU<br/>\
<b>-x</b> or <b>--max_iter</b> (<i>int</i>)")

#<b>-a</b> or <b>--abs_tol</b> (<i>double</i>)

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MPI_LIBRARIES} ${TRILINOS_LIBRARY} ${NOX_EPETRA_LIBRARY} ${NOX_LIBRARY} ${EPETRA_LIBRARY} ${TEUCHOS_LIBRARY} ${TEUCHOS_LIBRARY_PARAMETER_LIST} ${AMESOS_LIBRARY} ${AZTECOO_LIBRARY} ${ML_LIBRARY})  # slinkuje AMESOS atd. podobne
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENABLE_TESTING()
INCLUDE(CTest)

add_test(NAME "TRILINOS-electrostatic-Amesos_Klu-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -t 1e-2 -l Amesos_Klu -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-electrostatic-Amesos_Klu-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 2 -t 1e-2 -l Amesos_Klu -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOO-AZ_tfqmr_Prec-dom_decomp-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -t 1e-2 -l AztecOO_tfqmr -c AZ_dom_decomp  -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#bad
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_Prec_AMG-prec-SA_AggregType-Uncoupled_Smoother-Chebyshev_Coarse-Amesos-KLU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -t 1e-2 -l AztecOOML_tfqmr -c SA -e Uncoupled -o Chebyshev -z Amesos-KLU -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_Prec_AMG-prec-maxwell_AggregType-Uncoupled_Smoother-Chebyshev_Coarse-Amesos-KLU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -t 1e-2 -l AztecOOML_tfqmr -c maxwell -e Uncoupled -o Chebyshev -z Amesos-KLU -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#bad
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_Prec_AMG-prec-SA_AggregType-Uncoupled_Smoother-Chebyshev_Coarse-SILU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -t 1e-2 -l AztecOOML_tfqmr -c SA -e Uncoupled -o Chebyshev -z SILU -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# OK tests
add_test(NAME "TRILINOS-acoustic-Amesos_Klu-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -t 1e-2 -l Amesos_Klu -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOO-AZ_tfqmr_Prec-dom_decomp-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -t 1e-2 -l AztecOO_tfqmr -c AZ_dom_decomp -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOO-AZ_tfqmr_Prec-dom_decomp-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 2 -t 1e-2 -l AztecOO_tfqmr -c AZ_dom_decomp -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr_Prec_AMG-prec-SA_AggregType-Uncoupled_Smoother-Chebyshev_Coarse-Amesos-KLU-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -t 1e-2 -l AztecOOML_tfqmr -c SA -e Uncoupled -o Chebyshev -z Amesos-KLU -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr_Prec_AMG-prec-maxwell_AggregType-Uncoupled_Smoother-Chebyshev_Coarse-Amesos-KLU-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -t 1e-2 -l AztecOOML_tfqmr -c maxwell -e Uncoupled -o Chebyshev -z Amesos-KLU -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr_Prec_AMG-prec-SA_AggregType-Uncoupled_Smoother-Chebyshev_Coarse-SILU-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -t 1e-2 -l AztecOOML_tfqmr -c SA -e Uncoupled -o Chebyshev -z SILU -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_smooth_symmetric-block-Gauss-Seidel_coarse_symmetric-block-Gauss-Seidel" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -o "symmetric_block_Gauss-Seidel" -z "symmetric_block_Gauss-Seidel" -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
# AMG with default sets
# SA
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-SA-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c SA -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-SA-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c SA -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# NSSA
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-NSSA-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c NSSA -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-NSSA-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c NSSA -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# maxwell - bad
#add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-maxwell-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c maxwell -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-maxwell-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c maxwell -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# DD
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-DD-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-DD-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# RefMaxwell
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-RefMaxwell-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c RefMaxwell -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-RefMaxwell-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c RefMaxwell -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# DD-ML
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-DD-ML-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD-ML -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-DD-ML-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD-ML -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# DD-ML-LU
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-DD-ML-LU-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD-ML-LU -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-DD-ML-LU-default" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD-ML-LU -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# quickest + MPI
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-AZ_tfqmr_AMG-Prec-DD-default_MPI" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c DD -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOOML-AZ_tfqmr-Prec-RefMaxwell-default_MPI" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 2 -g -l AztecOOML_tfqmr -c RefMaxwell -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#
#add_test(NAME "TRILINOS-electrostatic-Amesos_Klu-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l Amesos_Klu -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-electrostatic-Amesos_Klu-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 2 -l Amesos_Klu -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-Amesos_Klu-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l Amesos_Klu -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_dom_decomp" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_dom_decomp" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOO-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 2 -l AztecOO -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# bad
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_none" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_none -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# bad
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_none" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c  AZ_none -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_Jacobi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_Jacobi -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# bad
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_Jacobi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_Jacobi -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
# bad
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_Neumann" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_Neumann -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# bad
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_Neumann" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_Neumann -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
# bad
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_ls" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_ls -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# bad
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_ls" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_ls -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
# bad
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_sym_GS" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_sym_GS -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# bad
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_sym_GS" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO -c AZ_sym_GS -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)

#
#add_test(NAME "TRILINOS-electrostatic-AztecOO-seq-prec_dom_decomp-solv_cgs" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO_cgs -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOO-seq-prec_dom_decomp-solv_cgs" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -l AztecOO_cgs -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_SA" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_SA" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_NSSA" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c NSSA -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_NSSA" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c NSSA -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_maxwell" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c maxwell -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_maxwell" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c maxwell -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_DD" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c DD -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_DD" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c DD -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_RefMaxwell" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c RefMaxwell -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_RefMaxwell" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c RefMaxwell -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_DD-ML" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c DD-ML -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_DD-ML" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c DD-ML -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_DD-ML-LU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c DD-ML-LU -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_DD-ML-LU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c DD-ML-LU -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_prec_SA_Uncoupled" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -e Uncoupled -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_prec_SA_Uncoupled" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -e Uncoupled -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_smooth_symmetric-block-Gauss-Seidel" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -o "symmetric_block_Gauss-Seidel" -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_smooth_symmetric-block-Gauss-Seidel" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -o "symmetric_block_Gauss-Seidel" -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-electrostatic-AztecOOML-seq_coarse_SILU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -z SILU -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_coarse_SILU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -z SILU -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
#add_test(NAME "TRILINOS-acoustic-AztecOOML-seq_coarse_SILU_agreg_Uncoupled_smoother_SILU" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 2 -g -l AztecOOML -c SA -z SILU -e Uncoupled -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)

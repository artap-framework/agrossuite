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
<b>-v</b> or <b>--variant_solver</b> (<i>int</i>) : 0 - Amesos (default), 1 - AztecOO<br/>")
#<b>-c</b> or <b>--preconditioner</b> (<i>string</i>) : Jacobi, MultiColoredGS, MultiColoredSGS, ILU, MultiColoredILU, MultiElimination, FSAI<br/>\
#<b>-l</b> or <b>--solver</b> (<i>string</i>) : BiCGStab, CG, GMRES, FGMRES, CR, IDR<br/>\
#<b>-x</b> or <b>--max_iter</b> (<i>int</i>)<br/>\
#<b>-t</b> or <b>--rel_tol</b> (<i>double</i>)<br/>\
#<b>-a</b> or <b>--abs_tol</b> (<i>double</i>)


ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MPI_LIBRARIES} ${TRILINOS_LIBRARY} ${NOX_EPETRA_LIBRARY} ${NOX_LIBRARY} ${EPETRA_LIBRARY} ${TEUCHOS_LIBRARY} ${TEUCHOS_LIBRARY_PARAMETER_LIST} ${AMESOS_LIBRARY} ${AZTECOO_LIBRARY})  # slinkuje AMESOS atd. podobne
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENABLE_TESTING()
INCLUDE(CTest)

add_test(NAME "TRILINOS-electrostatic-Amesos_Klu-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 0 -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-electrostatic-Amesos_Klu-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 0 -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-acoustic-Amesos_Klu-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 0 -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOO-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 0 -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-electrostatic-AztecOO-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 4 ../libs/solver_TRILINOS -v 0 -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
add_test(NAME "TRILINOS-acoustic-AztecOO-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -v 0 -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)

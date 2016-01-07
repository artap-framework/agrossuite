PROJECT(solver_PETSC C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

SET(DEBUG_FLAGS "-g")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsupported-friend")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
ENDIF()

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../3rdparty/")

FIND_PACKAGE(PETSc REQUIRED)
FIND_PACKAGE(MPI REQUIRED)
FIND_PACKAGE(LAPACK REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../3rdparty/matio)
INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${PETSC_INCLUDE_DIR})

SET(SOURCES main.cpp)

file(WRITE "${CMAKE_HOME_DIRECTORY}/../libs/${PROJECT_NAME}.ext"
"PETSC\n\
solver_PETSC\n\
")

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MPI_LIBRARIES} ${PETSC_LIBRARIES} ${LAPACK_LIBRARIES} gfortran)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENABLE_TESTING()
INCLUDE(CTest)

add_test(NAME "PETSc-electrostatic-KSPSolve-PCNONE-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_PETSC -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln -l chebyshev)
add_test(NAME "PETSc-acoustic-KSPSolve-PCNONE-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_PETSC -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln -l chebyshev)
add_test(NAME "PETSc-electrostatic-KSPSolve-PCHYPRE-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_PETSC -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln -c hypre)
add_test(NAME "PETSc-acoustic-KSPSolve-PCHYPRE-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_PETSC -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln -c hypre)
add_test(NAME "PETSc-electrostatic-KSPSolve-PCJACOBI-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_PETSC -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln -c jacobi -l chebyshev)
add_test(NAME "PETSc-acoustic-KSPSolve-PCJACOBI-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND  ../libs/solver_PETSC -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln -c jacobi -l chebyshev)
add_test(NAME "PETSc-electrostatic-KSPSolve-PCSOR-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND  ../libs/solver_PETSC -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln -c sor -l chebyshev)
add_test(NAME "PETSc-acoustic-KSPSolve-PCSOR-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_PETSC -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln -c sor -l chebyshev)

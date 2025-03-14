CMAKE_MINIMUM_REQUIRED(VERSION 3.13.4)
PROJECT(solver_plugin_MUMPS C CXX)

SET(CMAKE_BUILD_TYPE "Release")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

# Qt
IF(MSVC)
  SET(CMAKE_PREFIX_PATH "c:/Qt/6.5.3/msvc2019_64")
ENDIF()

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-deprecated -Ofast")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsupported-friend")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
ENDIF()

# MUMPS
IF(MSVC)
ELSE()
  FIND_PACKAGE(MUMPS REQUIRED)
ENDIF()

INCLUDE_DIRECTORIES(${MUMPS_INCLUDE_DIR})

SET(SOURCES main.cpp ../agros-library/solver/plugin_solver_interface.cpp)

# deal.II
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/build/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/boost-1.70.0/include/")

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../3rdparty/")

FIND_PACKAGE(deal.II HINTS "${CMAKE_SOURCE_DIR}/../dealii/build" REQUIRED)
deal_ii_initialize_cached_variables()
# message("MUMPS deal.II: ${DEAL_II_VERSION}")

# Handle Qt.
SET(CMAKE_AUTOMOC TRUE)

find_package(Qt6 COMPONENTS Core REQUIRED)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
if (MSVC)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/usr/lib
					         LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/usr/lib
) 
endif()

DEAL_II_SETUP_TARGET(${PROJECT_NAME})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_TARGET} ${MUMPS_LIBRARIES} ${LAPACK_LIBRARIES} ${BLAS_LIBRARY} Qt6::Core)
# SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../libs)

# ENABLE_TESTING()
# INCLUDE(CTest)

# add_test(NAME "MUMPS-electrostatic-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_MUMPS -v 1 -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# add_test(NAME "MUMPS-electrostatic-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 3 ../libs/solver_MUMPS -v 1 -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)
# add_test(NAME "MUMPS-acoustic-seq" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_MUMPS -v 1 -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)
# add_test(NAME "MUMPS-acoustic-mpi" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND mpirun -np 3 ../libs/solver_MUMPS -v 1 -m ../resources_source/linear_solvers/acoustic.matrix -p ../resources_source/linear_solvers/acoustic.matrix_pattern -r ../resources_source/linear_solvers/acoustic.rhs -q  ../resources_source/linear_solvers/acoustic.sln)

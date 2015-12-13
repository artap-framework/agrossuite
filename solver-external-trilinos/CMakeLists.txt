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

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../3rdparty/matio)
INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${AZTECOO_INCLUDE_PATH} ${AMESOS_INCLUDE_PATH} ${EPETRA_INCLUDE_PATH})
#message(${TRILINOS_INCLUDE_DIR})

SET(SOURCES main.cpp)

file(WRITE "${CMAKE_HOME_DIRECTORY}/../libs/${PROJECT_NAME}.ext"
"Trilinos\n\
solver_TRILINOS\n\
<b>-c</b> or <b>--preconditioner</b> (<i>string</i>) : Jacobi, MultiColoredGS, MultiColoredSGS, ILU, MultiColoredILU, MultiElimination, FSAI<br/>\
<b>-l</b> or <b>--solver</b> (<i>string</i>) : BiCGStab, CG, GMRES, FGMRES, CR, IDR<br/>\
<b>-x</b> or <b>--max_iter</b> (<i>int</i>)<br/>\
<b>-t</b> or <b>--rel_tol</b> (<i>double</i>)<br/>\
<b>-a</b> or <b>--abs_tol</b> (<i>double</i>)")

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MPI_LIBRARIES} ${AZTECOO_LIBRARIES} ${AMESOS_LIBRARY} ${TEUCHOS_LIBRARY} ${EPETRA_LIBRARY})  # slinkuje AMESOS atd. podobne
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENABLE_TESTING()
INCLUDE(CTest)

add_test(NAME "TRILINOS-electrostatic-" WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY} COMMAND ../libs/solver_TRILINOS -m ../resources_source/linear_solvers/electrostatic.matrix -p ../resources_source/linear_solvers/electrostatic.matrix_pattern -r ../resources_source/linear_solvers/electrostatic.rhs -q  ../resources_source/linear_solvers/electrostatic.sln)

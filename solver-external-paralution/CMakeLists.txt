PROJECT(solver_PARALUTION C CXX)
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

SET(PARALUTION_ROOT ${CMAKE_SOURCE_DIR}/paralution/build)
find_package(PARALUTION REQUIRED)
INCLUDE_DIRECTORIES(${PARALUTION_INCLUDE_DIR})
message(${PARALUTION_INCLUDE_DIR})

SET(SOURCES main.cpp)

file(WRITE "${CMAKE_HOME_DIRECTORY}/../libs/${PROJECT_NAME}.ext"
"PARALUTION\n\
solver_PARALUTION\n\
<b>-c</b> or <b>--preconditioner</b> (<i>string</i>) : Jacobi, MultiColoredGS, MultiColoredSGS, ILU, MultiColoredILU, MultiElimination, FSAI<br/>\
<b>-l</b> or <b>--solver</b> (<i>string</i>) : BiCGStab, CG, GMRES, FGMRES, CR, IDR<br/>\
<b>-x</b> or <b>--max_iter</b> (<i>int</i>)<br/>\
<b>-t</b> or <b>--rel_tol</b> (<i>double</i>)<br/>\
<b>-a</b> or <b>--abs_tol</b> (<i>double</i>)")

ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PARALUTION_LIBRARY})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/../libs)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

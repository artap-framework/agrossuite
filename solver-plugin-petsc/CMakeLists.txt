PROJECT(solver_plugin_PETSc C CXX)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# FIND_PACKAGE(MPI REQUIRED)
# SET(PETSC_DIR "/home/karban/Projects/3rdparty/petsc/") # arch-linux-c-opt
# SET(PETSC_ARCH "arch-linux-c-opt")
# FIND_PACKAGE(PETSc REQUIRED)
# FIND_PACKAGE(LAPACK REQUIRED)
set(PETSC_LIBRARIES "/home/karban/Projects/3rdparty/petsc/arch-linux-c-debug/lib/libpetsc.so")
INCLUDE_DIRECTORIES("/home/karban/Projects/3rdparty/petsc/include")
INCLUDE_DIRECTORIES("/home/karban/Projects/3rdparty/petsc/arch-linux-c-debug/include")

# SET(DEBUG_FLAGS "-g")
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}")

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-deprecated -Ofast")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsupported-friend")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
ENDIF()

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../3rdparty/")

# INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/../3rdparty/matio)
# INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})

SET(SOURCES main.cpp ../agros-library/solver/plugin_solver_interface.cpp)

# deal.II
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/build/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/boost-1.70.0/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/tbb-2018_U2/include/")

FIND_PACKAGE(deal.II HINTS "${CMAKE_SOURCE_DIR}/../dealii/build" REQUIRED)
# HACK
# string(REPLACE "/lib/lib" "/lib" DEAL_II_LIBRARIES ${DEAL_II_LIBRARIES})

# Handle Qt.
SET(CMAKE_AUTOMOC TRUE)

FIND_PACKAGE(Qt5Core REQUIRED)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})

# ${LAPACK_LIBRARIES}
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_LIBRARIES} ${PETSC_LIBRARIES} Qt5::Core)

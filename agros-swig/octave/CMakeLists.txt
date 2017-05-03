PROJECT( wrapper_octave )
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Octave
find_package(Octave REQUIRED)

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga3)

SET(SOURCES ../src/extension.cpp
    ../src/swig_field.cpp
    ../src/swig_geometry.cpp
    ../src/swig_particletracing.cpp
    ../src/swig_problem.cpp
    ../src/swig_study.cpp
    ../src/swig_agros.cpp)
SET(HEADERS ../src/swig_field.h
    ../src/swig_geometry.h
    ../src/swig_particletracing.h
    ../src/swig_problem.h
    ../src/swig_study.h
    ../src/swig_agros.h)

if(NOT ${Octave_FOUND})
  message(FATAL_ERROR "Unable to find Octave include directory")
else()
    set(OCTAVE_RELEASE_NAME)
    message(STATUS "[OCTAVE] - version ${OCTAVE_VERSION_STRING}")
    message(STATUS "[OCTAVE] - flags ${OCTAVE_CXXFLAGS}")
    message(STATUS "[OCTAVE] - include directory ${OCTAVE_INCLUDE_DIRS}")
    message(STATUS "[OCTAVE] - library ${OCTAVE_LIBRARY_RELEASE}")
    message(STATUS "[OCTAVE] - octave - mkoctfile ${MKOCTFILE_EXECUTABLE}")

    INCLUDE_DIRECTORIES(${OCTAVE_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SET(CMAKE_SWIG_FLAGS "-DSWIG_OCTAVE ${OCTAVE_CXXFLAGS}")

    SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES CPLUSPLUS ON)
    SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES SWIG_FLAGS "-includeall")
    SWIG_ADD_MODULE(${PROJECT_NAME} octave ../agros.i ${SOURCES} ${HEADERS})
    SWIG_LINK_LIBRARIES(${PROJECT_NAME} ${OCTAVE_LIBRARY_RELEASE} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns)
#  ${PLUGINS} ${AGROS_LIBRARY}
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME _${PROJECT_NAME})
endif()


PROJECT(${AGROS_PYTHON})

set(PYTHON "python3")
set(PACKAGE_VERSION 1.0)

# cython
# message("Cython files - Agros2D")
# message("\tcython agros2d.pyx")
add_custom_target(CYTHON_AGROS2D ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
    COMMAND C:/Python34/Scripts/cython ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)
ELSE()
  ADD_CUSTOM_COMMAND(
    COMMAND cython3 ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros2d.cpp)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/util)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros2d-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga3)


SET(SOURCES extension.cpp
    pythonlab/pyfield.cpp
    pythonlab/pygeometry.cpp
    pythonlab/pyparticletracing.cpp
    pythonlab/pyproblem.cpp
    pythonlab/pystudy.cpp
    pythonlab/pyagros.cpp)
SET(HEADERS pythonlab/pyfield.h
    pythonlab/pygeometry.h
    pythonlab/pyparticletracing.h
    pythonlab/pyproblem.h
    pythonlab/pystudy.h
    pythonlab/pyagros.h)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} )
ADD_DEPENDENCIES(${PROJECT_NAME} CYTHON_AGROS2D)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${QT_QTMAIN_LIBRARY} ${AGROS_LIBRARY})

QT5_USE_MODULES(${PROJECT_NAME} Core Network Xml XmlPatterns)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME agros2d_python)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

PROJECT(${AGROS_PYTHON})

set(PYTHON "python3")
set(PACKAGE_VERSION 1.0)

# Python
set(Python_ADDITIONAL_VERSIONS 3.4)
FIND_PACKAGE(PythonLibs 3.4 REQUIRED)
IF(MSVC)
  get_filename_component(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} PATH)
ENDIF()
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

# cython
# message("Cython files - agros")
# message("\tcython agros.pyx")
add_custom_target(CYTHON_AGROS ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.cpp)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
    COMMAND C:/Python34/Scripts/cython ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.cpp)
ELSE()
  ADD_CUSTOM_COMMAND(
    COMMAND cython3 ARGS -3 --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/agros.cpp)
ENDIF()

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
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libcmaes/src)

# set static build
IF (AGROS_BUILD_STATIC)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DAGROS_BUILD_STATIC=${AGROS_BUILD_STATIC}")
ENDIF (AGROS_BUILD_STATIC)

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

# other files
MACRO(ADD_INFO_FILES)
   FOREACH(_it ${ARGN})
     IF(NOT IS_DIRECTORY ${_it})
       GET_FILENAME_COMPONENT(name ${_it} NAME)
       IF(NOT ${_it} MATCHES "^/\\\\..*$;~$")
         SET_SOURCE_FILES_PROPERTIES(${_it} PROPERTIES HEADER_FILE_ONLY TRUE)
       ENDIF()
     ENDIF()
   ENDFOREACH()
ENDMACRO()

set(info_python ../resources_source/python/agros.pyx
         ../resources_source/python/pygeometry.pxi
         ../resources_source/python/pyfield.pxi
         ../resources_source/python/pyproblem.pxi
         ../resources_source/python/pycomputation.pxi
         ../resources_source/python/pysolution.pxi
         ../resources_source/python/pystudy.pxi
         ../resources_source/python/pyparticletracing.pxi
)
add_info_files(${info_python})
add_custom_target(other_python SOURCES ${info_python})

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS} )
ADD_DEPENDENCIES(${PROJECT_NAME} CYTHON_AGROS)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PLUGINS} ${AGROS_LIBRARY} ${PYTHON_LIBRARIES})

QT5_USE_MODULES(${PROJECT_NAME} Core Network)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME agros_python)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

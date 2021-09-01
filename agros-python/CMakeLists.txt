PROJECT(${AGROS_PYTHON})

set(PYTHON "python3")
set(PACKAGE_VERSION 1.0)

# deal.ii
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
REMOVE_DEFINITIONS(-DDEAL_II_WITH_CXX14)

# Python
# set(Python_ADDITIONAL_VERSIONS REQUIRED)
FIND_PACKAGE(PythonLibs REQUIRED)

IF(MSVC)
  get_filename_component(PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS} PATH)
ENDIF()
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

# cython
# message("Cython files - agros")
# message("\tcython3 _agros.pyx")
add_custom_target(CYTHON_AGROS ALL DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.cpp)

IF(WIN32)
  ADD_CUSTOM_COMMAND(
    COMMAND C:/Python36/Scripts/cython ARGS --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.cpp)
ELSE()
  ADD_CUSTOM_COMMAND(
    COMMAND cython3 ARGS --cplus ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.pyx
    DEPENDS ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.pyx
    OUTPUT  ${CMAKE_HOME_DIRECTORY}/resources_source/python/_agros.cpp)
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)

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
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${PLUGINS} ${AGROS_LIBRARY} ${PYTHON_LIBRARIES} Qt5::Core Qt5::Network)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES PREFIX "" OUTPUT_NAME _agros)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

add_custom_command(
    TARGET ${AGROS_PYTHON} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy "../libs/_agros.so" "../agrossuite/_agros.so"
)

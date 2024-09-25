PROJECT(${AGROS_PYTHON})

# deal.ii
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
REMOVE_DEFINITIONS(-DDEAL_II_WITH_CXX14)

IF(MSVC)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG_INIT} -DAGROS_LIBRARY_DLL")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} -DAGROS_LIBRARY_DLL")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_INIT} /bigobj")
ENDIF()

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/src/api)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nsga2)

# python wrapper
file(COPY ${CMAKE_HOME_DIRECTORY}/tests.sh DESTINATION ${PROJECT_BINARY_DIR}/..)
file(COPY ${CMAKE_HOME_DIRECTORY}/agros-python/agrossuite DESTINATION ${PROJECT_BINARY_DIR}/../agros-python)
file(COPY ${CMAKE_HOME_DIRECTORY}/agros-python/tests DESTINATION ${PROJECT_BINARY_DIR}/../agros-python)
file(COPY ${CMAKE_HOME_DIRECTORY}/agros-python/tests-tutorials DESTINATION ${PROJECT_BINARY_DIR}/../agros-python)
file(COPY ${CMAKE_HOME_DIRECTORY}/agros-python/examples DESTINATION ${PROJECT_BINARY_DIR}/../agros-python)

SET(SOURCES extension.cpp
    pythonlab/pyfield.cpp
    pythonlab/pygeometry.cpp
        pythonlab/pyproblem.cpp
    pythonlab/pystudy.cpp
    pythonlab/pyagros.cpp)
SET(HEADERS pythonlab/pyfield.h
    pythonlab/pygeometry.h
        pythonlab/pyproblem.h
    pythonlab/pystudy.h
    pythonlab/pyagros.h)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${AGROS_LIBRARY} Qt6::Core Qt6::Network)

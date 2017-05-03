PROJECT( wrapper_csharp )
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# csharp
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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
#INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})

set(CMAKE_SWIG_FLAGS -namespace org.agros)
set(CMAKE_SWIG_OUTDIR "org")

SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(${PROJECT_NAME} csharp ../agros.i ${SOURCES} ${HEADERS})
SWIG_LINK_LIBRARIES(${PROJECT_NAME} ${AGROS_LIBRARY} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME _${PROJECT_NAME})


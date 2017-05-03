PROJECT( wrapper_matlab )
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# matlab
set(Matlab_ROOT_DIR /opt/matlab-R2015a/)
#set(Matlab_ROOT_DIR /opt/matlab-R2017a-zcu/)
#set(MATLAB_ADDITIONAL_VERSIONS "R2017a=9.2")
find_package(Matlab REQUIRED)

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

if(NOT ${Matlab_FOUND})
  message(FATAL_ERROR "Unable to find Matlab include directory")
else()
    set(MATLAB_RELEASE_NAME)
    # matlab_get_release_name_from_version(${Matlab_VERSION_STRING} MATLAB_RELEASE_NAME)
    message(STATUS "[MATLAB] - version ${Matlab_VERSION_STRING}")
    message(STATUS "[MATLAB] - release ${MATLAB_RELEASE_NAME}")
    message(STATUS "[MATLAB] - include directory ${Matlab_INCLUDE_DIRS}")
    message(STATUS "[MATLAB] - mex library ${Matlab_MEX_LIBRARY}")
    message(STATUS "[MATLAB] - mex extension ${Matlab_MEX_EXTENSION}")
    message(STATUS "[MATLAB] - mx library ${Matlab_MX_LIBRARY}")
    message(STATUS "[MATLAB] - matlab ${Matlab_MAIN_PROGRAM}")
    
    INCLUDE_DIRECTORIES(${Matlab_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    add_definitions(-DMATLAB_MEX_FILE -DWITH_DEEPBIND=ON) #define matlab macros
    SET(USE_SWIG_FLAGS "")

    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--exclude-libs,ALL")
    swig_module_initialize(agrosMEX matlab)

    SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES CPLUSPLUS ON)
    SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES SWIG_FLAGS "-includeall")
    SWIG_ADD_MODULE(${PROJECT_NAME} matlab ../agros.i ${SOURCES} ${HEADERS})
    SWIG_LINK_LIBRARIES(${PROJECT_NAME} ${Matlab_LIBRARIES} ${QT_LIBRARIES} ${AGROS_LIBRARY} ${CURL_LIBRARIES} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns)
    # ${PLUGINS} ${AGROS_LIBRARY}
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME _${PROJECT_NAME})
endif()


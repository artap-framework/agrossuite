PROJECT( wrapper_java )
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Java
find_package(Java REQUIRED COMPONENTS Runtime Development)
find_package(JNI REQUIRED)
include(UseJava)

INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/agros-library)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/libdxfrw)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/poly2tri)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/quazip)

# optimization
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/bayesopt/include)
INCLUDE_DIRECTORIES(${CMAKE_HOME_DIRECTORY}/3rdparty/nlopt2/src/api)
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
INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})

set(CMAKE_SWIG_FLAGS -package org.agros)
set(CMAKE_SWIG_OUTDIR "org/agros")

SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(../agros.i PROPERTIES SWIG_FLAGS "-includeall")
SWIG_ADD_MODULE(${PROJECT_NAME} java ../agros.i ${SOURCES} ${HEADERS})
SWIG_LINK_LIBRARIES(${PROJECT_NAME} ${AGROS_LIBRARY} Qt5::Core Qt5::Network Qt5::Xml Qt5::XmlPatterns)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME _${PROJECT_NAME})


# JAVA
set(EXAMPLE_JAVA_FILES
    org/agros/main.java
    org/agros/agros.java
    org/agros/agrosJNI.java
    org/agros/Field.java
    org/agros/Geometry.java
    org/agros/SwigProblemBase.java
    org/agros/problem.java
    org/agros/Computation.java
  )

#set(CMAKE_JNI_TARGET TRUE)
#add_jar(example_swig_jar ${EXAMPLE_JAVA_FILES}
#        VERSION 1.2.0
#        ENTRY_POINT example)
#install_jar(example_swig_jar ${CMAKE_SWIG_OUTDIR}/example)
#install_jni_symlink(example_swig_jar .)

#set(JAVA_BUNDLE "${CMAKE_HOME_DIRECTORY}/java_bundle.sh")
#set(JAVA_BUNDLE_SO_FILES "${CMAKE_HOME_DIRECTORY}/libexample_swig.so")
#set(MANIFEST_FILE "${CMAKE_HOME_DIRECTORY}/MANIFEST")
#ADD_CUSTOM_COMMAND(TARGET example_swig
#                   POST_BUILD
#                   DEPEND example_swig_jar
#                   COMMAND ${JAVA_BUNDLE} "example_swig_jar.jar" "${Java_JAVA_EXECUTABLE}" "example.NativeLib" "${CMAKE_STRIP}" -so ${JAVA_BUNDLE_SO_FILES} -txt ${JAVA_BUNDLE_TXT_FILES}
#                   COMMAND ${JAVA_ARCHIVE} -uvf "example_swig_jar.jar" "NATIVE"
#                   COMMAND ${CMAKE_COMMAND} -E echo "Class-Path: ." > ${MANIFEST_FILE}
#                   COMMAND ${JAVA_ARCHIVE} -uvmf "MANIFEST" "example_swig_jar.jar"
#                   COMMAND ${CMAKE_COMMAND} -E remove ${MANIFEST_FILE}
#                   )

#ADD_CUSTOM_COMMAND(TARGET example_swig
#                   POST_BUILD
#                   COMMAND ${JAVA_COMPILE} "example_test.java")

#enable_testing()
#ADD_TEST(example-test "java" "example_test")

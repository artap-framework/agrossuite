CMAKE_MINIMUM_REQUIRED(VERSION 3.13.4)
PROJECT(agros)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debugging symbols for all files all the time:
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "-Wno-deprecated -Wno-deprecated-declarations -Wno-deprecated-copy -Wno-missing-declarations -Wno-return-type -Wno-unused-variable -Wno-unused-parameter -Wno-suggest-override -Wno-reorder ${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
    ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/dealii/cmake /usr/lib/x86_64-linux-gnu/cmake/Qt6)

SET(DEAL_II_DIR "dealii/build")
FIND_PACKAGE(deal.II HINTS ${DEAL_II_DIR})
# deal_ii_initialize_cached_variables()
message("deal.II: ${DEAL_II_VERSION}")
message("deal.II: ${DEAL_II_TARGET}")

# Allow to override the default values in CMake.vars:
IF(MSVC)
    INCLUDE(CMake.vars.Windows OPTIONAL)
ENDIF()
INCLUDE(CMake.vars OPTIONAL)

# set static build
SET(PLUGINS)

# Handle Qt.
SET(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Link Zlib
IF(MSVC)
    INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
ELSE() 
 FIND_PACKAGE(ZLIB REQUIRED)
ENDIF()

# Link OpenGL when needed.
IF(MSVC)
    FIND_PACKAGE(OpenGL REQUIRED)
ENDIF()

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 COMPONENTS Core5Compat REQUIRED)
find_package(Qt6 COMPONENTS Xml REQUIRED)

# date
string(TIMESTAMP RESULT %y%m%d)
string(SUBSTRING ${RESULT} 0 2 YEAR)
string(SUBSTRING ${RESULT} 2 2 MONTH)
string(SUBSTRING ${RESULT} 4 2 DAY)
SET(VERSION_YEAR 20${YEAR})
SET(VERSION_MONTH ${MONTH})
SET(VERSION_DAY ${DAY})

# Naming of projects built.
SET(AGROS_LIBRARY agros_library)
SET(AGROS_GENERATOR agros_generator)
SET(AGROS_GUI agros_gui)
SET(AGROS_PYTHON agros_python)
SET(CTEMPLATE_LIBRARY agros_3rdparty_ctemplate)
SET(DXFLIB_LIBRARY agros_3rdparty_dxflib)
SET(QUAZIP_LIBRARY agros_3rdparty_quazip)
SET(MATIO_LIBRARY agros_3rdparty_matio)
SET(TRIANGLE_LIBRARY agros_3rdparty_triangle)
SET(NLOPTLIB_LIBRARY agros_3rdparty_nlopt2)
SET(BAYESOPTLIB_LIBRARY agros_3rdparty_bayesopt)
SET(PAGMO2_LIBRARY agros_3rdparty_pagmo2)

# curl
find_package(CURL REQUIRED)

# Output paths.
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/usr/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/usr/lib)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/usr/bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/usr/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/usr/lib)

IF(MSVC)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
ENDIF(MSVC)

# Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on Unicode
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

# Configure CCache if available
find_program(CCACHE_PROGRAM ccache)
IF(CCACHE_PROGRAM)
    message(NOTICE "Selected CCache: ${CCACHE_PROGRAM}")

    set(ENV{CCACHE_BASEDIR} "${PROJECT_SOURCE_DIR}")
    set(ENV{CCACHE_DIR} "${PROJECT_SOURCE_DIR}/ccache")
    set(ENV{CCACHE_COMPILERCHECK} "content")

    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM} base_dir=${PROJECT_SOURCE_DIR})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM} base_dir=${PROJECT_SOURCE_DIR})

    # set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    # set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
ENDIF()

# Create file with preprocessor definitions exposing the build settings to the source code.
configure_file(
        agros-library/util/agros_version.h.cmake
        ${CMAKE_BINARY_DIR}/agros-library/util/agros_version.h)
configure_file(
        resources_source/resources.rc.cmake
        ${CMAKE_BINARY_DIR}/resources_source/resources.rc)

# copy resources
file(COPY ${PROJECT_SOURCE_DIR}/resources_source DESTINATION ${PROJECT_BINARY_DIR})
file(COPY ${PROJECT_SOURCE_DIR}/resources DESTINATION ${PROJECT_BINARY_DIR}/usr/share/agrossuite)
file(COPY ${PROJECT_SOURCE_DIR}/CMake.vars.Windows DESTINATION ${PROJECT_BINARY_DIR})

# deploy
file(COPY ${PROJECT_SOURCE_DIR}/deploy.sh DESTINATION ${PROJECT_BINARY_DIR})

# add_dependencies(RoE copy_resources_source)

IF(MSVC)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/agros-library/util)
ENDIF()
# Include OUR header files location
include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/ctemplate)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/libdxfrw)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/qsingleapplication)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/quazip)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/matio)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/pagmo2/include)

# Subdirectories.
ADD_SUBDIRECTORY(3rdparty)

# deal.ii
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
# REMOVE_DEFINITIONS(-DDEAL_II_WITH_CXX17)

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/build/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/boost-1.70.0/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/tbb-2018_U2/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/kokkos-3.7.00/core/src/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/kokkos-3.7.00/tpls/desul/include/")

# Include external directories.
INCLUDE_DIRECTORIES(${AMESOS_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${AZTECOO_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${EPETRA_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${IFPACK_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${LOCA_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${ML_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${NOX_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${TEUCHOS_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${KOMPLEX_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${LOCA_EPETRA_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${NOX_EPETRA_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${EPETRAEXT_INCLUDE_PATH})

message("CMake: Start agros")
message("=====================")
ADD_SUBDIRECTORY(agros-library)
ADD_SUBDIRECTORY(solver-plugin-mumps)

ADD_SUBDIRECTORY(agros-generator)
# GUI
ADD_SUBDIRECTORY(agros-gui)
# Python interface
ADD_SUBDIRECTORY(agros-python)

message("\nagros Configuration results")
message("---------------------")
message(" Version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_SUB}")

message(" Build version: ${CMAKE_BUILD_TYPE}")
message("---------------------")
message("\n")

# pixmap
FILE(GLOB PIXMAP_FILES "resources/images/agros.png")
# desktop
FILE(GLOB DESKTOP_FILES "agros.desktop")
# resources
SET(RESOURCES_FILES "resources")

# share
INSTALL(FILES ${PIXMAP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps)
INSTALL(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
INSTALL(DIRECTORY ${RESOURCES_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/agros)

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

set(info resources_source/modules/acoustic.json
         resources_source/modules/current.json
         resources_source/modules/elasticity.json
         resources_source/modules/electrostatic.json
         resources_source/modules/flow.json
         resources_source/modules/heat.json
         resources_source/modules/magnetic.json
         resources_source/modules/rf_te.json
         resources_source/modules/rf_tm.json

         resources_source/couplings/current-heat.json
         resources_source/couplings/heat-elasticity.json
         resources_source/couplings/magnetic-heat.json

         resources_source/generator/filter_cpp.tpl
         resources_source/generator/filter_h.tpl
         resources_source/generator/force_cpp.tpl
         resources_source/generator/force_h.tpl
         resources_source/generator/localvalue_h.tpl
         resources_source/generator/localvalue_cpp.tpl
         resources_source/generator/surfaceintegral_cpp.tpl
         resources_source/generator/surfaceintegral_h.tpl
         resources_source/generator/volumeintegral_cpp.tpl
         resources_source/generator/volumeintegral_h.tpl
         resources_source/generator/interface_h.tpl
         resources_source/generator/module_CMakeLists_txt.tpl
         resources_source/generator/weakform_cpp.tpl
         resources_source/generator/weakform_h.tpl
         resources_source/generator/plugins_CMakeLists_txt.tpl
         resources_source/generator/interface_cpp.tpl
         resources_source/generator/doc_index.tpl
         resources_source/generator/equations.tpl
         resources_source/generator/equations.tex

         resources/templates/empty.tpl
         resources/templates/material.tpl
         resources/templates/problem.tpl
         resources/templates/welcome.tpl         

         agros-python/agrossuite/src/__init__.py
         agros-python/agrossuite/src/_agros.pyx
         agros-python/agrossuite/src/pycomputation.pxi
         agros-python/agrossuite/src/pyfield.pxi
         agros-python/agrossuite/src/pygeometry.pxi
         agros-python/agrossuite/src/pyproblem.pxi
         agros-python/agrossuite/src/pysolution.pxi
         agros-python/agrossuite/src/pystudy.pxi
         agros-python/agrossuite/src/tools.py
)
add_info_files(${info})
add_custom_target(other SOURCES ${info})


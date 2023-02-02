# Set project's name
PROJECT( agros )
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/dealii/cmake)

# This is necessary for Visual Studio - without this for some reason the linking hangs.
IF(MSVC)
#  SET (CMAKE_EXE_LINKER_FLAGS "/machine:X64")
#  SET (CMAKE_SHARED_LINKER_FLAGS "/machine:X64")
#  SET (CMAKE_MODULE_LINKER_FLAGS "/machine:X64")
#  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN64")

#  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LTCG")
#  SET(CMAKE_SHARED_LINKER_FLAGS	 "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
#  ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
ENDIF(MSVC)

SET(CMAKE_AGROS_DIRECTORY "${CMAKE_HOME_DIRECTORY}/")

# Allow to override the default values in CMake.vars:
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    # SET(AGROS_UMFPACK ON)
    INCLUDE(CMake.vars.Linux OPTIONAL)
ELSE()
    SET(AGROS_UMFPACK ON)
    INCLUDE(CMake.vars.Windows OPTIONAL)
ENDIF()
INCLUDE(CMake.vars OPTIONAL)

# set static build
SET(PLUGINS)

# Handle Qt.
SET(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Link OpenGL and Zlib when needed.
IF(MSVC)
    # FIND_PACKAGE(OpenGL REQUIRED)
    FIND_PACKAGE(ZLIB REQUIRED)
        INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
ENDIF()

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Network REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core5Compat)

# git revision
#execute_process(COMMAND git rev-list HEAD --count OUTPUT_VARIABLE NUMBER_OF_COMMITS)
#STRING(REGEX REPLACE "(\r?\n)+$" "" VERSION_GIT "${NUMBER_OF_COMMITS}")

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
SET(POLY2TRI_LIBRARY agros_3rdparty_poly2tri)
SET(QCUSTOMPLOT_LIBRARY agros_3rdparty_qcustomplot)
SET(QUAZIP_LIBRARY agros_3rdparty_quazip)
SET(QTSINGLEAPPLICATION_LIBRARY agros_3rdparty_qtsingleapplication)
SET(MATIO_LIBRARY agros_3rdparty_matio)
SET(TRIANGLE_LIBRARY agros_3rdparty_triangle)
SET(NLOPTLIB_LIBRARY agros_3rdparty_nlopt2)
SET(BAYESOPTLIB_LIBRARY agros_3rdparty_bayesopt)
SET(NSGA2_LIBRARY agros_3rdparty_nsga2)
SET(QTAWESOME_LIBRARY agros_3rdparty_qtawesome)
SET(SPDLOG_LIBRARY agros_3rdparty_spdlog)

# curl
find_package(CURL REQUIRED)

# Output paths.
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_AGROS_DIRECTORY})
SET(LIBRARY_OUTPUT_PATH ${CMAKE_AGROS_DIRECTORY}/libs)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_AGROS_DIRECTORY}/libs)

IF(MSVC)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_AGROS_DIRECTORY})
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_AGROS_DIRECTORY})
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_AGROS_DIRECTORY}/libs)
  SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_AGROS_DIRECTORY}/libs)
ENDIF(MSVC)

# Include current dir
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# Turn on Unicode
ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# Enable debugging symbols for all files all the time:
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_BUILD_TYPE RelWithDebInfo)
  SET(RELEASE_FLAGS "-DNDEBUG -g -O3 -Ofast")
  SET(CMAKE_CXX_FLAGS "-Wno-deprecated -Wno-deprecated-declarations -Wno-missing-declarations -Wno-return-type ${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
  ADD_DEFINITIONS(-DQT_NO_DEBUG_OUTPUT)
ENDIF()

# Create file with preprocessor definitions exposing the build settings to the source code.
configure_file(
  agros-library/util/agros_version.h.cmake
  agros-library/util/agros_version.h)
configure_file(
  resources_source/resources.rc.cmake
  resources_source/resources.rc)

# Include OUR header files location
INCLUDE(IncludeSubdirs.cmake)

# Subdirectories.
ADD_SUBDIRECTORY(3rdparty)

# deal.ii
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
REMOVE_DEFINITIONS(-DDEAL_II_WITH_CXX14)

SET(DEAL_II_BUILD_TYPE "Release")

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/build/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/boost-1.70.0/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/dealii/bundled/tbb-2018_U2/include/")

FIND_PACKAGE(deal.II HINTS "dealii/build" REQUIRED)

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
ADD_SUBDIRECTORY(solver-plugin-umfpack)
IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    ADD_SUBDIRECTORY(solver-plugin-mumps)
    # ADD_SUBDIRECTORY(solver-plugin-petsc)
ENDIF()

ADD_SUBDIRECTORY(agros-generator)
# Python interface
ADD_SUBDIRECTORY(agros-python)
# GUI
ADD_SUBDIRECTORY(agros-gui)

message("\nagros Configuration results")
message("---------------------")
message(" Version: ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_SUB}")

message(" Build version: Release")
message("---------------------")
message("\n")

# pixmap
FILE(GLOB PIXMAP_FILES "resources/images/agros.xpm")
# desktop
FILE(GLOB DESKTOP_FILES "agros.desktop")
# resources
SET(RESOURCES_FILES "resources")

# share
INSTALL(FILES ${PIXMAP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps)
INSTALL(FILES ${DESKTOP_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
INSTALL(DIRECTORY ${RESOURCES_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/agros)
INSTALL(FILES dealii/build/lib/libdeal_II.so.9.4.0 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libagros_deal_II.so)

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
         resources_source/modules/math_coeff.json
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
)
add_info_files(${info})
add_custom_target(other SOURCES ${info})


CMAKE_MINIMUM_REQUIRED(VERSION 3.13.4)
PROJECT(solver_plugin_amgcl C CXX)

SET(CMAKE_BUILD_TYPE "Release")
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/../dealii/cmake)

# Qt
IF(MSVC)
  SET(CMAKE_PREFIX_PATH "c:/Qt/6.5.3/msvc2019_64")
ENDIF()

# Set global compiler parameters.
IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wno-deprecated -Ofast")
ENDIF()
IF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unsupported-friend")
  SET(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
ENDIF()

SET(SOURCES main.cpp ../agros-library/solver/plugin_solver_interface.cpp)

# deal.II
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/build/include/")
SET(Boost_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/3rdparty/boost/include")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/3rdparty/boost/include/")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../dealii/bundled/boost-1.70.0/include/")

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/../3rdparty/")

FIND_PACKAGE(deal.II HINTS "${CMAKE_SOURCE_DIR}/../dealii/build" REQUIRED)
deal_ii_initialize_cached_variables()

# Handle Qt.
SET(CMAKE_AUTOMOC TRUE)

find_package(Qt6 COMPONENTS Core REQUIRED)

ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_DLL)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})
if (MSVC)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/usr/lib
					         LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/usr/lib
) 
endif()

DEAL_II_SETUP_TARGET(${PROJECT_NAME})

find_package(OpenMP)
if (OPENMP_FOUND)
  message("AMGCL - OpenMP found")

  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_TARGET} Qt6::Core OpenMP::OpenMP_CXX)
  # TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_TARGET} Qt6::Core OpenMP::OpenMP_CXX OpenCL::OpenCL)
else()
  message("AMGCL - OpenMP not found")

  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${DEAL_II_TARGET} Qt6::Core)
endif()








#find_package(Boost REQUIRED)
#
## Ensure all targets are available
#include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/VexCLBoostTargets.cmake")
#
##----------------------------------------------------------------------------
## Generic target
##----------------------------------------------------------------------------
#add_library(Common INTERFACE)
#add_library(VexCL::Common ALIAS Common)
#
#target_compile_features(Common INTERFACE
#        cxx_auto_type
#        cxx_nullptr
#        cxx_rvalue_references
#        cxx_right_angle_brackets
#        cxx_static_assert
#        cxx_variadic_templates
#        cxx_decltype
#)
#
#if (VEXCL_AMD_SI_WORKAROUND)
#  target_compile_definitions(Common INTERFACE VEXCL_AMD_SI_WORKAROUND)
#endif()
#
#target_include_directories(Common INTERFACE
#        ${Boost_INCLUDE_DIRS}
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:include>
#)
#
#
#target_link_libraries(Common INTERFACE
#        Boost::filesystem
#        Boost::system
#        Boost::thread
#)
#
#if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
#  target_link_libraries(Common INTERFACE
#          Boost::chrono
#          Boost::date_time
#  )
#endif()
#
#target_compile_options(Common INTERFACE
#        # g++
#        $<$<CXX_COMPILER_ID:GNU>:$<BUILD_INTERFACE:-Wall>>
#        $<$<CXX_COMPILER_ID:GNU>:-Wno-missing-braces>
#        $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>
#        $<$<CXX_COMPILER_ID:GNU>:-Wno-ignored-attributes>
#        $<$<CXX_COMPILER_ID:GNU>:-Wno-unused-local-typedefs>
#        $<$<CXX_COMPILER_ID:GNU>:-Wno-variadic-macros>
#        # Clang
#        $<$<CXX_COMPILER_ID:Clang>:$<BUILD_INTERFACE:-Wall>>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-braces>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-deprecated-declarations>
#        $<$<CXX_COMPILER_ID:Clang>:-Wno-ignored-attributes>
#        # MSVC
#        $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
#        $<$<CXX_COMPILER_ID:MSVC>:/wd4003>
#        $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
#)
#
#target_compile_definitions(Common INTERFACE
#        # MSVC
#        $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
#        $<$<CXX_COMPILER_ID:MSVC>:_VARIADIC_MAX=10>
#)
#
##----------------------------------------------------------------------------
## Find VexCL backends
##----------------------------------------------------------------------------
#find_package(OpenCL)
#
## Ensure all targets are available
#include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/VexCLOpenCLTarget.cmake")
#
#if(OpenCL_FOUND)
#  set(CL_HPP_TARGET_OPENCL_VERSION 120 CACHE STRING "Target OpenCL version")
#  set(CL_HPP_MINIMUM_OPENCL_VERSION 120 CACHE STRING "Minimum OpenCL version")
#
#  add_library(OpenCL INTERFACE)
#  add_library(VexCL::OpenCL ALIAS OpenCL)
#
#  target_link_libraries(OpenCL INTERFACE Common OpenCL::OpenCL)
#  target_compile_definitions(OpenCL INTERFACE VEXCL_BACKEND_OPENCL)
#  target_compile_definitions(OpenCL INTERFACE CL_TARGET_OPENCL_VERSION=${CL_HPP_TARGET_OPENCL_VERSION})
#  target_compile_definitions(OpenCL INTERFACE CL_HPP_TARGET_OPENCL_VERSION=${CL_HPP_TARGET_OPENCL_VERSION})
#  target_compile_definitions(OpenCL INTERFACE CL_HPP_MINIMUM_OPENCL_VERSION=${CL_HPP_MINIMUM_OPENCL_VERSION})
#
#  find_file(OPENCL_HPP CL/opencl.hpp HINTS ${OpenCL_INCLUDE_DIRS})
#  message(STATUS " -- ${OPENCL_HPP} --")
#  if(OPENCL_HPP)
#    target_compile_definitions(OpenCL INTERFACE VEXCL_HAVE_OPENCL_HPP)
#  endif()
#
#  target_compile_options(Common INTERFACE
#          $<$<CXX_COMPILER_ID:GNU>:-Wno-catch-value>
#  )
#
#  message(STATUS "Found VexCL::OpenCL")
#
#  if ("${Boost_VERSION}" VERSION_GREATER_EQUAL "1.61.0")
#    add_library(Compute INTERFACE)
#    add_library(VexCL::Compute ALIAS Compute)
#
#    target_link_libraries(Compute INTERFACE Common OpenCL::OpenCL)
#    target_compile_definitions(Compute INTERFACE VEXCL_BACKEND_COMPUTE)
#
#    message(STATUS "Found VexCL::Compute")
#  endif()
#endif()
#
##find_package(CUDA)
##if(CUDA_FOUND)
##  add_library(CUDA INTERFACE)
##  add_library(VexCL::CUDA ALIAS CUDA)
##
##  target_include_directories(CUDA INTERFACE "${CUDA_INCLUDE_DIRS}")
##  target_link_libraries(CUDA INTERFACE Common "${CUDA_CUDA_LIBRARY}")
##  target_compile_definitions(CUDA INTERFACE VEXCL_BACKEND_CUDA)
##
##  message(STATUS "Found VexCL::CUDA")
##endif()
#
#find_path(Boost_DLL NAMES boost/dll PATHS ${Boost_INCLUDE_DIRS})
#if (Boost_DLL)
#  if(OpenCL_INCLUDE_DIR)
#    add_library(JIT INTERFACE)
#    target_include_directories(JIT INTERFACE "${OpenCL_INCLUDE_DIR}")
#  else()
#    include(CheckIncludeFile)
#    check_include_file("CL/cl_platform.h" HAVE_OpenCL_PLATFORM_H)
#    if(HAVE_OpenCL_PLATFORM_H)
#      add_library(JIT INTERFACE)
#    endif()
#  endif()
#
#  if(NOT TARGET JIT)
#    message(WARNING "The JIT interface requires OpenCL headers to be available."
#            "You can download them from https://github.com/KhronosGroup/OpenCL-Headers"
#            "Set OpenCL_INCLUDE_DIR to the location of the headers."
#            "For now, disabling the JIT target.")
#  endif()
#endif()
#
#
#if(TARGET JIT)
#  add_library(VexCL::JIT ALIAS JIT)
#
#  set(VEXCL_JIT_COMPILER_FLAGS "" CACHE STRING "VexCL JIT compiler flags")
#  target_compile_definitions(JIT INTERFACE VEXCL_JIT_COMPILER_FLAGS=${VEXCL_JIT_COMPILER_FLAGS})
#
#  find_package(OpenMP)
#
#  # Have to check several OPENMP_FOUND due to bug in
#  # one version of CMake and the docs (fixed in patch release)
#  # OpenMP is missing on macOS llvm default, for example
#  if(OpenMP_FOUND OR OPENMP_FOUND OR OpenMP_CXX_FOUND)
#
#    # CMake 3.9 FindOpenMP allows correct linking with Clang in more cases
#    if(TARGET OpenMP::OpenMP_CXX)
#      target_link_libraries(JIT INTERFACE OpenMP::OpenMP_CXX Common)
#    else()
#      # Clang may need -fopenmp=libiomp5 instead, can't be detected here without CMake 3.9
#      target_link_libraries(JIT INTERFACE
#              $<$<CXX_COMPILER_ID:GNU>:${OpenMP_CXX_FLAGS}>
#              $<$<CXX_COMPILER_ID:Clang>:${OpenMP_CXX_FLAGS}>
#              $<$<CXX_COMPILER_ID:Intel>:${OpenMP_CXX_FLAGS}>
#      )
#      target_compile_options(JIT INTERFACE ${OpenMP_CXX_FLAGS})
#    endif()
#
#    set(VEXCL_OMP_FLAGS "${OpenMP_CXX_FLAGS}")
#
#    # We only need to add libraries to link to if this is using a preprocessor only OpenMP flag
#    if("${OpenMP_CXX_FLAGS}" MATCHES ".*X(clang|preprocessor).*")
#      foreach(item ${OpenMP_CXX_LIBRARIES})
#        set(VEXCL_OMP_FLAGS "${VEXCL_OMP_FLAGS} ${item}")
#      endforeach()
#    endif()
#
#    # Pass the required flags to code
#    target_compile_definitions(JIT INTERFACE VEXCL_OMP_FLAGS=${VEXCL_OMP_FLAGS})
#  endif()
#
#  target_link_libraries(JIT INTERFACE Common ${CMAKE_DL_LIBS})
#  target_compile_definitions(JIT INTERFACE VEXCL_BACKEND_JIT)
#
#  message(STATUS "Found VexCL::JIT")
#endif()
#
#include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/VexCLTools.cmake")

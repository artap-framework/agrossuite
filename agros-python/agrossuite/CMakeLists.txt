cmake_minimum_required(VERSION 3.15)

project(agrossuite)
ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB)
enable_testing()
if(MSVC)
SET(CMAKE_PREFIX_PATH "c:/Qt/6.5.3/msvc2019_64")
endif()
find_package(PythonExtensions REQUIRED)
find_package(Cython REQUIRED)

add_subdirectory(src)
